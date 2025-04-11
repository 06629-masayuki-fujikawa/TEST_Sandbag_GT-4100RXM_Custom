/*[]----------------------------------------------------------------------[]*/
/*| ۯ����u�Ď�����                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : R.Hara(NT4500EX���ް���IF�ՒʐM���߂Ƃ��č�蒼����)     |*/
/*| Date        : 2005.02.01                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"Message.h"
#include	"tbl_rkn.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"mem_def.h"
#include	"flp_def.h"
#include	"ope_def.h"
#include	"LKcom.h"
#include	"LKmain.h"
#include	"prm_tbl.h"
#include	"common.h"
#include	"ntnet.h"
#include	"mdl_def.h"
#include	"IFM.h"
#include	"ntnet_def.h"

static	void	Lk_InCount( uchar uc_prm );
static	void	Lk_OutCount( uchar uc_prm, uchar mode );
static	void	Lk_AllMove( uchar lk_kind, uchar mov_req, uchar req_kind );
static	uchar	Mnt_Lk_Move( uchar mov_req );

static	void	SyuseiFuseiSet( ushort no, uchar type );

char		ope_RegistFuseiData(ushort no, flp_com *flp, char BFrs);
flp_com		flp_com_frs;

char		ope_LockTimeOutData(ushort no, flp_com *flp);

struct	FLPCTL_rec	FLPCTL;											// �ׯ��(ۯ����u)����ܰ��ر
ushort	lkreccnt;													// �ω��ް���
ushort	lkrecrpt;													// �ω��ް��Ǐo����
ushort	lktimoutno;													// ��ѱ�ĊĎ���������ۯ����u��

t_flp_DownLockErrInfo	flp_DownLockErrInfo[LOCK_MAX];				// ���~ۯ���ϰ���ر(�N�����I�[��0)
uchar	flp_f_DownLock_RetryDownCtrl;								// 1=���g���C����ɂ�鉺�~�w��
																	// FlpSet()�ւ̎w�����̂ݗL��
static void	flp_flag_clr( uchar, uchar );

#define	CAR_SENSOR_ON	0x01
#define	LAG_TIME_OVER	0x02
#define	CAR_SENSOR_OFF	0x04
static	uchar	InOutCountFunc( uchar mode,	ushort	evt );

static	uchar	Aida_Flap_Chk( ushort );
static	void	flp_DownLockRetry_RecvSts( ushort );		// ��ԕω��ɂ�郊�g���C����
static	void	flp_DownLock_ForCarSensorOff( ushort no );
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
extern void LongTermParkingRel( ulong LockNo , uchar knd, flp_com *flp);
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)

/*[]----------------------------------------------------------------------[]*/
/*| Flaper Management Main                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : fcmain                                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	fcmain( void )
{
	ushort	evt;
	char	wt_flg = 0;


	if( LKcom_InitFlg == 0 ){
		// lkcom����Ƽ�ٖ�����
		return;
	}

	if( ( evt = FlpMsg() ) == 0 ){
		return;
	}

	if( ( FLPCTL.Room_no < 1 ) || ( LOCK_MAX < FLPCTL.Room_no ) ) {
		return;
	}
	Lock_Kind = LkKind_Get( FLPCTL.Room_no );					// ���u��ʁi�ׯ�߁^ۯ����u�j���擾
	if( Lock_Kind == LK_KIND_ERR ){
		return;
	}

	if( _is_ntnet_remote() && (prm_get(COM_PRM,S_NTN,121,1,1) == 0) ){	// ���uNT-NET�ݒ� & �����`���̂Ƃ��̂݃`�F�b�N
		switch( evt / 100 ){
			case 1:		// Car sensor ON
			case 2:		// Car sensor OFF
				if( (prm_get(COM_PRM,S_NTN,120,3,1)!=0) && (NT_pcars_timer == -1) ) {
					NT_pcars_timer = (short)(prm_get(COM_PRM,S_NTN,120,3,1) * 50);
				}
				break;
		}
	}

	// �Ԏ����ׯ���ް���ܰ��ر��۰��
	memcpy( &FLPCTL.flp_work, &FLAPDT.flp_data[ FLPCTL.Room_no - 1 ], sizeof( FLPCTL.flp_work ) );

	switch( FLPCTL.flp_work.mode ){
		case FLAP_CTRL_MODE1:	// ���~�ς�(�ԗ��Ȃ�)
			wt_flg = flp_faz1( evt );
			break;
		case FLAP_CTRL_MODE2:	// ���~�ς�(�ԗ�����FT���Ē�)
			wt_flg = flp_faz2( evt );
			break;
		case FLAP_CTRL_MODE3:	// �㏸���쒆
			wt_flg = flp_faz3( evt );
			break;
		case FLAP_CTRL_MODE4:	// �㏸�ς�(���Ԓ�)
			wt_flg = flp_faz4( evt );
			break;
		case FLAP_CTRL_MODE5:	// ���~���쒆
			wt_flg = flp_faz5( evt );
			break;
		case FLAP_CTRL_MODE6:	// ���~�ς�(�ԗ�����LT���Ē�)
			wt_flg = flp_faz6( evt );
			break;
		default:
			if( FLPCTL.Room_no > 0 && FLPCTL.Room_no <= INT_CAR_START_INDEX ){
				ex_errlg( ERRMDL_IFFLAP, ERR_FLAP_STATUSNG, 2, 0 );	// �ׯ�ߏ�ԋK��O
			}else if( FLPCTL.Room_no > INT_CAR_START_INDEX && FLPCTL.Room_no <= BIKE_START_INDEX ){
				ex_errlg( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_STATENG, 2, 0 );	// �t���b�v�^���b�N��ԋK��O
			} else {
				ex_errlg( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_STATENG, 2, 0 );	// �t���b�v�^���b�N��ԋK��O
			}
			break;
	}
	if( wt_flg != 0 ) {
		nmisave( &FLAPDT.flp_data[ FLPCTL.Room_no - 1 ], &FLPCTL.flp_work, sizeof( FLPCTL.flp_work ) );
// MH810100(S) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X (RT���Z�f�[�^�Ή�)
//		if( wt_flg == 1 ){
//			Make_Log_Enter( FLPCTL.Room_no );		// ���Ƀf�[�^�쐬(LOG�ɕۑ�����`��)
//			Log_regist( LOG_ENTER );				// ���ɗ���o�^
//		}
// MH810100(E) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X (RT���Z�f�[�^�Ή�)
	}

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| ���~�ς�(�ԗ��Ȃ�)�̂Ƃ��̲���ď���                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : flp_faz1( evt, aflp )                                   |*/
/*| PARAMETER    : evt = ����ć�                                           |*/
/*| RETURN VALUE : ret = �ׯ���ް��������� 0:�Ȃ��A-1:����                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	flp_faz1( ushort evt )
{
	char	ret;

	ret = 0;
	switch( evt / 100 ){
		case 1:		// Car sensor ON
			flp_flag_clr( FLAP_CTRL_MODE2, CAR_SENSOR_ON );
			ret = (char)-1;
			break;
		case 2:		// Car sensor OFF
			/* �o�ɏ��o�^ */
			IoLog_write(IOLOG_EVNT_OUT_GARAGE, (ushort)LockInfo[FLPCTL.Room_no - 1].posi, 0, 0);
			break;
	}

	switch( evt % 100 ){
		case 50:	// ��ԕω�
			if(( FLPCTL.flp_work.nstat.bits.b02 == 1 )&&			// �㏸����
			   ( FLPCTL.flp_work.nstat.bits.b01 == 1 )){			// �㏸�ς�(ۯ��ς�)
				FmvTimer( FLPCTL.Room_no, -1 );						// MT stop
			}
			else if(( FLPCTL.flp_work.nstat.bits.b02 == 0 )&&		// ���~����
					( FLPCTL.flp_work.nstat.bits.b01 == 0 )){		// ���~�ς�(ۯ��J�ς�)
				FmvTimer( FLPCTL.Room_no, -1 );						// MT stop
				if( ( Aida_Flap_Chk((ushort)(FLPCTL.Room_no-1))) &&
					( FLPCTL.flp_work.nstat.bits.b00 == 0 ) &&
					( FLPCTL.flp_work.nstat.bits.b05 == 1 ) ){
					// �p�c���ׯ�߂ŉ��~ۯ���
					
					// �i��������j
					// ���ԑ҂��i�ԂȂ��^���~�ρj�ŉ��~�ݻ��OFF�����ꍇ�AIF�Ղ������I�ɉ��~������s����
					// ����ł����~�ݻ��ON���Ȃ��ꍇ�A���~ۯ��ƂȂ�B�p�c���ׯ�ߎ��͂P����ײ�iҲ݂��牺�~�v���j���s���B
					FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
				}
			}
			flp_DownLock_ForCarSensorOff( FLPCTL.Room_no );
			break;
		case 21:	// Payment complete
			break;
		case 23:	// �ׯ�ߏ㏸�w��(�C�����Z�p)
			if( syusei[FLPCTL.Room_no-1].ot_car == 2 ){			// ���2�F�o�Ɂ`���̓���
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 3 ){	// ���3�F���̓��Ɂ`�ׯ�ߏ㏸��ђ�
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 4 ){	// ���4�F�ׯ�ߏ㏸�`���̐��Z
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 5 ){	// ���5�F׸���ѱ��߁`�Đ��Z
				SyuseiFuseiSet( FLPCTL.Room_no, 1 );
			}
			break;
		case 24:	// �ׯ�߉��~�w��(�C�����Z�p)
			break;
		case 25:	// Mentenance Flaper Up
			Mnt_Lk_Move( ON );										// ����ݽ����u�ׯ�ߏ㏸�iۯ��j�v�v��
			ret = (char)-1;
			break;
		case 26:	// Mentenance Flaper Down
			Mnt_Lk_Move( OFF );										// ����ݽ����u�ׯ�߉��~�iۯ��J�j�v�v��
			ret = (char)-1;
			break;
		case 33:	// Flap move management timer over
			NTNET_Snd_Data05_Sub();
			ret = (char)-1;
			break;
		case 43:	// �ׯ�ߏ㏸(ۯ����u��)��ϰ
			break;
		case 42:	// Lag time timer over
			break;
		case 45:	// ���~ۯ���ϰ��ѱ��
			(void)flp_DownLock_DownSend( (ushort)(FLPCTL.Room_no-1) );
			break;
		default:
			break;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| ���~�ς�(�ԗ�����FT���Ē�)�̂Ƃ��̲���ď���                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : flp_faz2( evt )                                         |*/
/*| PARAMETER    : evt = ����ć�                                           |*/
/*| RETURN VALUE : ret = �ׯ���ް��������� 0:�Ȃ��A-1:����                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	flp_faz2( ushort evt )
{
	char	ret;
	uchar		snd20_flg;

	snd20_flg = 0;
	ret = 0;

	switch( evt / 100 ){
		case 1:		// Car sensor ON
			flp_flag_clr( FLAP_CTRL_MODE2, CAR_SENSOR_ON );
			ret = (char)-1;
			break;
		case 2:		// Car sensor OFF
			snd20_flg = InOutCountFunc( FLAP_CTRL_MODE2, (ushort)(evt / 100));
			/* �o�ɏ��o�^ */
			IoLog_write(IOLOG_EVNT_OUT_GARAGE, (ushort)LockInfo[FLPCTL.Room_no - 1].posi, 0, 0);
			// TF4800�ɍ��킹�ăt���b�v(���b�N)�㏸�^�C�}�[���ɏo�ɂ����ꍇ�A�t���b�v���オ���Ă���΁A���~������B
			if( FLPCTL.flp_work.nstat.bits.b01 == 1 ){				// �t���b�v�㏸���H
				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
			}
			ret = (char)-1;
			break;
	}

	switch( evt % 100 ){
		case 50:	// ��ԕω�
			if(( FLPCTL.flp_work.nstat.bits.b02 == 1 )&&			// �㏸����
			   ( FLPCTL.flp_work.nstat.bits.b01 == 1 )){			// �㏸�ς�(ۯ��ς�)
				FmvTimer( FLPCTL.Room_no, -1 );						// MT stop
			}
			else if(( FLPCTL.flp_work.nstat.bits.b02 == 0 )&&		// ���~����
					( FLPCTL.flp_work.nstat.bits.b01 == 0 )){		// ���~�ς�(ۯ��J�ς�)
				FmvTimer( FLPCTL.Room_no, -1 );						// MT stop
				flp_DownLockRetry_RecvSts( FLPCTL.Room_no );		// ��ԕω��ɂ�郊�g���C����
			}
			break;
		case 21:	// Payment complete
			if(Carkind_Param(FLP_ROCK_INTIME, (char)LockInfo[FLPCTL.Room_no - 1].ryo_syu,1,1)){	// ۯ����u�J�n��ϰ�N�����ɐ��Z����
				if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){			// Car ON?
					if( FLPCTL.flp_work.nstat.bits.b01 == 0 ){		// ���~�ς�?
						FLPCTL.flp_work.mode = FLAP_CTRL_MODE6;
					}else{
						FLPCTL.flp_work.mode = FLAP_CTRL_MODE5;
					}
					FlpSet( FLPCTL.Room_no, 2 );					// Flaper DOWN
					FLPCTL.flp_work.timer = LagTimer( FLPCTL.Room_no );	// Lag time start
				}else{
					FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
					FlpSet( FLPCTL.Room_no, 2 );					// Flaper DOWN(IF�ՂƂ̃Y���h�~�A�O�̂���)
					FLPCTL.flp_work.timer = -1;						// Lag time stop
				}
				snd20_flg = InOutCountFunc( FLAP_CTRL_MODE2, (ushort)(evt % 100));
				ret = (char)-1;
			}
			break;
		case 23:	// �ׯ�ߏ㏸�w��(�C�����Z�p)
			if( syusei[FLPCTL.Room_no-1].ot_car == 2 ){			// ���2�F�o�Ɂ`���̓���
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 3 ){	// ���3�F���̓��Ɂ`�ׯ�ߏ㏸��ђ�
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 4 ){	// ���4�F�ׯ�ߏ㏸�`���̐��Z
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 5 ){	// ���5�F׸���ѱ��߁`�Đ��Z
				SyuseiFuseiSet( FLPCTL.Room_no, 1 );
			}
			break;
		case 24:	// �ׯ�߉��~�w��(�C�����Z�p)
			break;
		case 25:	// Mentenance Flaper Up
			Mnt_Lk_Move( ON );										// ����ݽ����u�ׯ�ߏ㏸�iۯ��j�v�v��
			ret = (char)-1;
			break;
		case 26:	// Mentenance Flaper Down
			Mnt_Lk_Move( OFF );										// ����ݽ����u�ׯ�߉��~�iۯ��J�j�v�v��
			ret = (char)-1;
			break;
		case 33:	// Flap move management timer over
			NTNET_Snd_Data05_Sub();
			ret = (char)-1;
			break;
		case 42:	// Lag time timer over
			break;
		case 43:	// �ׯ�ߏ㏸(ۯ����u��)��ϰ
			if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){ 				// �ԗ�����H
				// �ԗ�����
				if( FLPCTL.flp_work.nstat.bits.b01 == 1 ){			// �㏸�ς݁iۯ��ς݁j�H
					// �㏸�ς݁iۯ��ς݁j
					if( LockInfo[FLPCTL.Room_no-1].lok_syu <=  6 ){			// ۯ����u�̎�
						if( FLPCTL.flp_work.nstat.bits.b02 == 0 ){			// ���~�w���iۯ��J�w���j
							// ۯ��J�w���ς݂�ۯ���ԁ��܂�ۯ��J�̌��ʂ���M���Ă��Ȃ�
							// �]���āAۯ�����ѱ�̂�ۯ��𑗐M����
							FLPCTL.flp_work.mode = FLAP_CTRL_MODE3;			// ��ԁ��㏸���쒆
							FlpSet( FLPCTL.Room_no, 1 );					// �ׯ�ߏ㏸�iۯ��j�v��
						}else{												// �㏸�w���iۯ��w���j
							FLPCTL.flp_work.mode = FLAP_CTRL_MODE4;			// ��ԁ��㏸�ς�(���Ԓ�)
							snd20_flg = InOutCountFunc( FLAP_CTRL_MODE2, (ushort)(evt % 100));
						}
					}else{													// �ׯ�߂̎�
						FLPCTL.flp_work.mode = FLAP_CTRL_MODE4;				// ��ԁ��㏸�ς�(���Ԓ�)
						snd20_flg = InOutCountFunc( FLAP_CTRL_MODE2, (ushort)(evt % 100));
					}
				}
				else{
					// ���~�ς݁iۯ��J�ς݁j
					FLPCTL.flp_work.mode = FLAP_CTRL_MODE3;			// ��ԁ��㏸���쒆
					FlpSet( FLPCTL.Room_no, 1 );					// �ׯ�ߏ㏸�iۯ��j�v��
				}
			}
			else{
				// �ԗ��Ȃ�
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;				// ��ԁ����~�ς�(�ԗ��Ȃ�)
			}
			ret = (char)-1;
			break;
		case 44:	// ���ɔ�����ϰ
			if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){ 				// �ԗ�����H
				snd20_flg = InOutCountFunc( FLAP_CTRL_MODE2, (ushort)(evt % 100));
			}else{
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;					// ��ԁ����~�ς�(�ԗ��Ȃ�)
				FLPCTL.flp_work.timer = -1;								// �ׯ�ߏ㏸(ۯ����u��)��ϰ�į��
			}
			ret = (char)-1;
			break;
		case 45:	// ���~ۯ���ϰ��ѱ��
			(void)flp_DownLock_DownSend( (ushort)(FLPCTL.Room_no-1) );
			break;
		default:
			break;
	}
	if( snd20_flg == 1 ){
		ret = 1;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| �㏸���쒆�̂Ƃ��̲���ď���                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : flp_faz3( evt, aflp )                                   |*/
/*| PARAMETER    : evt = ����ć�                                           |*/
/*| RETURN VALUE : ret = �ׯ���ް��������� 0:�Ȃ��A-1:����                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	flp_faz3( ushort evt )
{
	char	ret;
	uchar	snd20_flg;
	uchar	fp_car_ot = 0;

	snd20_flg = 0;

	ret = 0;
	switch( evt / 100 ){
		case 1:		// Car sensor ON
			flp_flag_clr( FLAP_CTRL_MODE2, CAR_SENSOR_ON );
			ret = (char)-1;
			break;
		case 2:		// Car sensor OFF
			if(( prm_get( COM_PRM,S_TYP,73,1,1 ) == 1 )&&					// �p�c�޲��s���o�ɂȂ�
			   ( LockInfo[FLPCTL.Room_no-1].lok_syu == LK_TYPE_AIDA2 )){	// �Ԏ��ݒ肪�p�c�޲�
				// �p�c�޲����͕s���o�ɂȂ��Ƃ���
				break;
			}
// MH810100(S) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X (RT���Z�f�[�^�Ή�)
//			if (FLPCTL.flp_work.lag_to_in.BIT.FUKUG){
//				// �U�֐��Z�E�������ɒ��̏o�ɂ͕s���o�ɑO�ɉ��̍ē��ɂ𑗂�
//				Make_Log_Enter_frs(FLPCTL.Room_no, (void*)&FLPCTL.flp_work);	// �����Ƀf�[�^
//				Log_regist( LOG_ENTER );										// ���ɗ���o�^
//			}
// MH810100(E) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X (RT���Z�f�[�^�Ή�)
			// �s���o���ް��o�^
			if( fusei.kensuu >= LOCK_MAX ){
				fusei.kensuu = LOCK_MAX - 1;
			}
			memcpy( &fusei.fus_d[fusei.kensuu].iyear,
			        &FLAPDT.flp_data[FLPCTL.Room_no-1].year, 6 );			// In Time
			memcpy( &fusei.fus_d[fusei.kensuu].oyear, &CLK_REC.year, 6 );	// Out Time
			fusei.fus_d[fusei.kensuu].t_iti = FLPCTL.Room_no;

			FLPCTL.flp_work.mode = FLAP_CTRL_MODE5;
			fusei.fus_d[fusei.kensuu].kyousei = 0;							// �����׸�OFF
			IoLog_write(IOLOG_EVNT_OUT_ILLEGAL_START, (ushort)LockInfo[FLPCTL.Room_no - 1].posi, 0, 0);
			FlpSet( FLPCTL.Room_no, 2 );									// Flaper DOWN
			FLPCTL.flp_work.nstat.bits.b06 = 1;								// �s��
			fusei.kensuu ++;												// �s���E��������+1
			InOutCountFunc( FLAP_CTRL_MODE3, (ushort)(evt / 100));

			fp_car_ot = 1;									// �ԗ��Ȃ����ׯ�߉��~�v������
			ret = (char)-1;
			break;
	}

	switch( evt % 100 ){
		case 50:	// ��ԕω�
			if( FLPCTL.flp_work.nstat.bits.b01 == 1 ){				// �㏸�ς�(ۯ��ς�)
				if( fp_car_ot == 0 ){								// �ԗ��Ȃ����ׯ�߉��~�v���Ȃ�
					FLPCTL.flp_work.mode = FLAP_CTRL_MODE4;
				}
				IFM_LockTable.ucFlapWait = 0;		// ���슮��
				FmvTimer( FLPCTL.Room_no, -1 );						// MT stop
				if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){			// Car ON?

					if(	( !prm_get( COM_PRM,S_TYP,62,1,2 ) ) ||			// �C�����Z�Ȃ�
						( FLPCTL.flp_work.lag_to_in.BIT.SYUUS == 0 ) ){	// �C�����Z��łȂ�
						snd20_flg = InOutCountFunc( FLAP_CTRL_MODE3, (ushort)(evt % 100));
					}
					FLPCTL.flp_work.lag_to_in.BIT.SYUUS = 0;
				}
				ret = (char)-1;
			}
			break;
		case 21:	// Payment complete
			if(Carkind_Param(FLP_ROCK_INTIME, (char)LockInfo[FLPCTL.Room_no - 1].ryo_syu,1,1)){		// ۯ����u�J�n��ϰ�N�����ɐ��Z����
				if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){			// Car ON?
					if( FLPCTL.flp_work.nstat.bits.b01 == 0 ){		// ���~�ς�?
						FLPCTL.flp_work.mode = FLAP_CTRL_MODE6;
					}else{
						FLPCTL.flp_work.mode = FLAP_CTRL_MODE5;
					}
					FlpSet( FLPCTL.Room_no, 2 );					// Flaper DOWN
					FLPCTL.flp_work.timer = LagTimer( FLPCTL.Room_no );	// Lag time start
				}else{
					FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
					FlpSet( FLPCTL.Room_no, 2 );					// Flaper DOWN(IF�ՂƂ̃Y���h�~�A�O�̂���)
					FLPCTL.flp_work.timer = -1;						// Lag time stop
				}
				InOutCountFunc( FLAP_CTRL_MODE3, (ushort)(evt % 100));
				ret = (char)-1;
			}
			break;
		case 23:	// �ׯ�ߏ㏸�w��(�C�����Z�p)
			if( syusei[FLPCTL.Room_no-1].ot_car == 2 ){			// ���2�F�o�Ɂ`���̓���
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 3 ){	// ���3�F���̓��Ɂ`�ׯ�ߏ㏸��ђ�
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 4 ){	// ���4�F�ׯ�ߏ㏸�`���̐��Z
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 5 ){	// ���5�F׸���ѱ��߁`�Đ��Z
				SyuseiFuseiSet( FLPCTL.Room_no, 1 );
				memcpy( &FLPCTL.flp_work.year, &syusei[FLPCTL.Room_no - 1].iyear, 6 );
				syusei[FLPCTL.Room_no - 1].sei = 2;				// �C�����Ԏ��F���޽��эl��
				ret = 1;
				FLPCTL.flp_work.lag_to_in.BIT.SYUUS = 1;
				FLPCTL.flp_work.lag_to_in.BIT.LAGIN = 0;
			}
			break;
		case 24:	// �ׯ�߉��~�w��(�C�����Z�p)
			break;
		case 25:	// Mentenance Flaper Up
			NTNET_Snd_Data05_Sub();									// NT-NETۯ����䌋���ް��쐬
			ret = (char)-1;
			break;
		case 26:	// Mentenance Flaper Down
			NTNET_Snd_Data05_Sub();									// NT-NETۯ����䌋���ް��쐬
			ret = (char)-1;
			break;
		case 33:	// Flap move management timer over
			// �װ�o�^
			FLPCTL.flp_work.mode = FLAP_CTRL_MODE4;
			NTNET_Snd_Data05_Sub();
			ret = (char)-1;
			break;
		case 43:	// �ׯ�ߏ㏸(ۯ����u��)��ϰ
			if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){				// Car ON?
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE3;
			}else{
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
			}
			ret = (char)-1;
			break;
		case 42:	// Lag time timer over
			break;
		default:
			break;
	}
	if (FLPCTL.flp_work.mode != FLAP_CTRL_MODE3) {
	// �U�֌��̕���������������U�֏����N���A
		FLPCTL.flp_work.lag_to_in.BIT.FUKUG = 0;
		FLPCTL.flp_work.lag_to_in.BIT.FURIK = 0;
		FLPCTL.flp_work.lag_to_in.BIT.SYUUS = 0;
	}
	if( snd20_flg == 1 ){
		ret = 1;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| �㏸�ς�(���Ԓ�)�̂Ƃ��̲���ď���                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : flp_faz4( evt )                                         |*/
/*| PARAMETER    : evt = ����ć�                                           |*/
/*| RETURN VALUE : ret = �ׯ���ް��������� 0:�Ȃ��A-1:����                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	flp_faz4( ushort evt )
{
	char	ret;
	ret = 0;
	switch( evt / 100 ){
		case 1:		// Car sensor ON
			break;
		case 2:		// Car sensor OFF
			if(!((( FLPCTL.flp_work.nstat.bits.b01 == 0 )&&			// ���~�ς�(ۯ��J�ς�)
			      ( FLPCTL.flp_work.nstat.bits.b07 == 1 ))||		// ����ݽ�׸�ON
			     ( FLPCTL.flp_work.nstat.bits.b03 == 1 )))			// �����o��
			{
				// �s���o��
				if(( prm_get( COM_PRM,S_TYP,73,1,1 ) == 1 )&&					// �p�c�޲��s���o�ɂȂ�
				   ( LockInfo[FLPCTL.Room_no-1].lok_syu == LK_TYPE_AIDA2 )){	// �Ԏ��ݒ肪�p�c�޲�
					// �p�c�޲����͕s���o�ɂȂ��Ƃ���
					break;
				}
			}
			ope_RegistFuseiData(FLPCTL.Room_no, &FLPCTL.flp_work, 0);
			InOutCountFunc( FLAP_CTRL_MODE4, (ushort)(evt / 100));
			ret = (char)-1;
			break;
	}

	switch( evt % 100 ){
		case 50:	// ��ԕω�
			if(( FLPCTL.flp_work.nstat.bits.b02 == 1 )&&			// �㏸����
			   ( FLPCTL.flp_work.nstat.bits.b01 == 1 )){			// �㏸�ς�(ۯ��ς�)
				FmvTimer( FLPCTL.Room_no, -1 );						// MT stop
			}
			else if(( FLPCTL.flp_work.nstat.bits.b02 == 0 )&&		// ���~����
					( FLPCTL.flp_work.nstat.bits.b01 == 0 )){		// ���~�ς�(ۯ��J�ς�)
				FmvTimer( FLPCTL.Room_no, -1 );						// MT stop
			}
			break;
		case 21:	// Payment complete
			if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){				// Car ON?
				if( FLPCTL.flp_work.nstat.bits.b01 == 0 ){			// ���~�ς�?
					FLPCTL.flp_work.mode = FLAP_CTRL_MODE6;
				}else{
					FLPCTL.flp_work.mode = FLAP_CTRL_MODE5;
				}
				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
				FLPCTL.flp_work.timer = LagTimer( FLPCTL.Room_no );	// Lag time start
			}else{
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN(IF�ՂƂ̃Y���h�~�A�O�̂���)
				FLPCTL.flp_work.timer = -1;							// Lag time stop
			}
			InOutCountFunc( FLAP_CTRL_MODE4, (ushort)(evt % 100));
			ret = (char)-1;
			break;
		case 23:	// �ׯ�ߏ㏸�w��(�C�����Z�p)
			if( syusei[FLPCTL.Room_no-1].ot_car == 2 ){			// ���2�F�o�Ɂ`���̓���
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 3 ){	// ���3�F���̓��Ɂ`�ׯ�ߏ㏸��ђ�
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 4 ){	// ���4�F�ׯ�ߏ㏸�`���̐��Z
				SyuseiFuseiSet( FLPCTL.Room_no, 0 );

			}else if( syusei[FLPCTL.Room_no-1].ot_car == 5 ){	// ���5�F׸���ѱ��߁`�Đ��Z
				SyuseiFuseiSet( FLPCTL.Room_no, 1 );
				memcpy( &FLPCTL.flp_work.year, &syusei[FLPCTL.Room_no - 1].iyear, 6 );
				syusei[FLPCTL.Room_no - 1].sei = 2;				// �C�����Ԏ��F���޽��эl��
				ret = 1;
				FLPCTL.flp_work.lag_to_in.BIT.SYUUS = 1;
				FLPCTL.flp_work.lag_to_in.BIT.LAGIN = 0;
			}
			break;
		case 24:	// �ׯ�߉��~�w��(�C�����Z�p)
			break;
		case 25:	// Mentenance Flaper Up
			Mnt_Lk_Move( ON );										// ����ݽ����u�ׯ�ߏ㏸�iۯ��j�v�v��
			ret = (char)-1;
			break;
		case 26:	// Mentenance Flaper Down
			/* �����o�ɊJ�n���o�^ */
			IoLog_write(IOLOG_EVNT_FORCE_STA, (ushort)LockInfo[FLPCTL.Room_no - 1].posi, 0, 0);
			Mnt_Lk_Move( OFF );										// ����ݽ����u�ׯ�߉��~�iۯ��J�j�v�v��
			if( FLPCTL.Room_no >= LOCK_START_NO ){
				ope_anm( AVM_FORCE_ROCK );							// ���Z�������ųݽ(���b�N)
			}else{
				ope_anm( AVM_FORCE_FLAP );							// ���Z�������ųݽ(�t���b�v)
			}
			ret = (char)-1;
			break;
		case 33:	// Flap move management timer over
			NTNET_Snd_Data05_Sub();
			ret = (char)-1;
			break;
		case 43:	// �ׯ�ߏ㏸(ۯ����u��)��ϰ
			break;
		case 42:	// Lag time timer over
			break;
			// ���Ԓ��A�����e�ɂ�鉺�~�ł����g���C����
		case 45:	// ���~ۯ���ϰ��ѱ��
			(void)flp_DownLock_DownSend( (ushort)(FLPCTL.Room_no-1) );
			break;
		default:
			break;
	}

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| ���~���쒆�̂Ƃ��̲���ď���                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : flp_faz5( evt )                                         |*/
/*| PARAMETER    : evt = ����ć�                                           |*/
/*| RETURN VALUE : ret = �ׯ���ް��������� 0:�Ȃ��A-1:����                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	flp_faz5( ushort evt )
{
	char	ret;
	struct	CAR_TIM		wok_tm;
	uchar	snd20_flg;
	/* �C�����@��flp_faz3()��fp_car_ot�ɂ��C���Ɠ��l�Ƃ��� */
	uchar	fp_car_in = 0;

	snd20_flg = 0;

	ret = 0;
	switch( evt / 100 ){
		case 1:		// Car sensor ON
			flp_flag_clr( FLAP_CTRL_MODE2, CAR_SENSOR_ON );
			fp_car_in = 1;											// �ԗ�����̏����ς�
			ret = (char)-1;
			break;
		case 2:		// Car sensor OFF
			Flp_LagExtCnt[FLPCTL.Room_no-1] = 0;					// ���O�^�C�������񐔃N���A
			flp_flag_clr( FLAP_CTRL_MODE1, CAR_SENSOR_OFF );
			IoLog_write(IOLOG_EVNT_OUT_GARAGE, (ushort)LockInfo[FLPCTL.Room_no - 1].posi, 0, 0);
// MH322914(S) K.Onodera 2016/12/07 ���~���b�N��Ԃŏo�ɂ����ꍇ�A�ݒ�ɂăt���b�v���~�����g���C����[���ʃo�ONo.1336](GW872707)
//			if(	(Aida_Flap_Chk((ushort)(FLPCTL.Room_no-1))) &&
//				( FLPCTL.flp_work.nstat.bits.b05 == 1 ) ){
//				// �p�c���ׯ�߂ŉ��~ۯ����̏ꍇ
//				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
//			}
			if(	( prm_get( COM_PRM,S_TYP,119,1,1 ) == 1 ) &&		// �t���b�v���~�����g���C����
				( FLPCTL.flp_work.nstat.bits.b05 == 1 )		){		// ���~���b�N����
				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
			}
			if( prm_get( COM_PRM,S_TYP,119,1,1 ) == 2 ){ 			// �t���b�v���~�𖳏����Ń��g���C����
				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
			}
// MH322914(E) K.Onodera 2016/12/07 ���~���b�N��Ԃŏo�ɂ����ꍇ�A�ݒ�ɂăt���b�v���~�����g���C����[���ʃo�ONo.1336](GW872707)
			InOutCountFunc( FLAP_CTRL_MODE5, (ushort)(evt / 100));
			ret = (char)-1;
			break;
	}

	switch( evt % 100 ){
		case 50:	// ��ԕω�
			if( FLPCTL.flp_work.nstat.bits.b01 == 0 ){				// ���~�ς�(ۯ��J�ς�)
				IFM_LockTable.ucFlapWait = 0;		// ���슮��
				if( fp_car_in == 0 ){								// �ԗ�����̏����ς݂łȂ�
					if( FLPCTL.flp_work.nstat.bits.b06 == 1 ){		// �s��
						FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
						FLPCTL.flp_work.nstat.bits.b06 = 0;			// �s���ر
					}else{
						if( FLPCTL.flp_work.nstat.bits.b00 ){
							FLPCTL.flp_work.mode = FLAP_CTRL_MODE6;
						}
					}
				}
				FmvTimer( FLPCTL.Room_no, -1 );						// MT stop
				ret = (char)-1;
				flp_DownLockRetry_RecvSts( FLPCTL.Room_no );		// ��ԕω��ɂ�郊�g���C����
			}
			flp_DownLock_ForCarSensorOff( FLPCTL.Room_no );
			break;
		case 21:	// Payment complete
// �s��C��(S) K.Onodera 2016/10/13 #1582 �U�֌��̃t���b�v���㏸���ɓd����؂�ƁA�U�֐�̃t���b�v�����~���Ȃ��Ȃ�
			// �㏸�ς݁H
			if( FLPCTL.flp_work.nstat.bits.b01 == 1 ){
				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
			}
// �s��C��(E) K.Onodera 2016/10/13 #1582 �U�֌��̃t���b�v���㏸���ɓd����؂�ƁA�U�֐�̃t���b�v�����~���Ȃ��Ȃ�
			break;
		case 23:	// �ׯ�ߏ㏸�w��(�C�����Z�p)
			if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){				// Car ON?
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE3;
				FlpSet( FLPCTL.Room_no, 1 );						// Flaper UP

				// 01-0039�C=0�ݒ�̎��́i�ԈႦ�j���Z���ɏo�ɶ��Ă��Ă��邽�߁A�������ɓ��ɶ��Ă��s��

				snd20_flg = InOutCountFunc( FLAP_CTRL_MODE5, (ushort)(evt % 100));
				memcpy( &FLPCTL.flp_work.year, &syusei[FLPCTL.Room_no-1].iyear, 6 );
				memset( &FLPCTL.flp_work.s_year, 0, 6 );
				syusei[FLPCTL.Room_no - 1].sei = 2;					// �C�����Ԏ��F���޽��эl��

				FLPCTL.flp_work.timer = -1;							// ׸���� ��ϰ�į��
				FLPCTL.flp_work.lag_to_in.BIT.SYUUS = 1;
			}else{
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
			}
			ret = (char)-1;
			break;
		case 24:	// �ׯ�߉��~�w��(�C�����Z�p)
#if	SYUSEI_PAYMENT		// SYUSEI_PAYMENT
			if( FLPCTL.flp_work.nstat.bits.b00 == 0 ){				// Car OFF?
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
				ret = -1;
			}
#endif	// SYUSEI_PAYMENT
			break;
		case 25:	// Mentenance Flaper Up
			NTNET_Snd_Data05_Sub();									// NT-NETۯ����䌋���ް��쐬
			ret = (char)-1;
			break;
		case 26:	// Mentenance Flaper Down
			NTNET_Snd_Data05_Sub();									// NT-NETۯ����䌋���ް��쐬
			ret = (char)-1;
			break;
		case 33:	// Flap move management timer over
			// �װ�o�^
			if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){				// Car ON?
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE6;
			}else{
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
			}
			NTNET_Snd_Data05_Sub();
			ret = (char)-1;
			break;
		case 43:	// �ׯ�ߏ㏸(ۯ����u��)��ϰ
			break;
		case 42:	// Lag time timer over
			Flp_LagExtCnt[FLPCTL.Room_no-1] = 0;					// ���O�^�C�������񐔃N���A
			if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){				// Car ON?
				snd20_flg = InOutCountFunc( FLAP_CTRL_MODE5, (ushort)(evt % 100));
				IoLog_write(IOLOG_EVNT_RENT_GARAGE, (ushort)LockInfo[FLPCTL.Room_no - 1].posi, 0, 0);		/* �ē��ɏ��o�^ */

				if (FLPCTL.flp_work.lag_to_in.BIT.FUKUG) {
					// �U�֐��Z
					FLPCTL.flp_work.mode = FLAP_CTRL_MODE3;
					FlpSet( FLPCTL.Room_no, 1 );						// Flaper UP
					FLPCTL.flp_work.nstat.bits.b06 = 0;					// �s���ر
					FLPCTL.flp_work.nstat.bits.b07 = 0;					// ����ݽ�׸�OFF
					FLPCTL.flp_work.nstat.bits.b09 = 0;					// NT-NET�׸�OFF

					FLPCTL.flp_work.nstat.bits.b14 = 0;					// ���u����̑���w����ʃN���A
					FLPCTL.flp_work.nstat.bits.b15 = 0;					// ���u����̃t���b�v����v���N���A
					ret = (char)-1;
					break;	// ���Ɏ����Auketuke�Apasswd�Abk_syu�͂��̂܂�
				}
				
				switch( Carkind_Param(FLP_ROCK_INTIME, (char)LockInfo[FLPCTL.Room_no - 1].ryo_syu,1,3) ){	// ׸���ьo�ߌ�̓��Ɏ��ԁH
				
					case	0:		// �u���ݎ��ԁv����Ɏ��ԂƂ���
						memcpy( &FLPCTL.flp_work.year, &CLK_REC, 6 );
						break;

					case	1:		// �u���ݎ���-1���v����Ɏ��ԂƂ���
						wok_tm.year = CLK_REC.year;					// ����	�N
						wok_tm.mon  = CLK_REC.mont;					//		��
						wok_tm.day  = CLK_REC.date;					//		��
						wok_tm.hour = CLK_REC.hour;					//		��
						wok_tm.min  = CLK_REC.minu;					//		��
						wok_tm.week = CLK_REC.week;					//		�j��
						ec66( &wok_tm,(long)1 );					// ���t�X�V�����i���ݎ�����-1�����Z�j
						FLPCTL.flp_work.year = wok_tm.year;			// ����	�N
						FLPCTL.flp_work.mont = wok_tm.mon;			//		��
						FLPCTL.flp_work.date = wok_tm.day;			//		��
						FLPCTL.flp_work.hour = wok_tm.hour;			//		��
						FLPCTL.flp_work.minu = wok_tm.min;			//		��
						break;

					case	2:		// �u���Z���ԁv����Ɏ��ԂƂ���
						memcpy( &FLPCTL.flp_work.year, &FLPCTL.flp_work.s_year, 6 );
						break;

					default:		// ���̑��i���ݎ��Ԃ���Ɏ��ԂƂ���j
						memcpy( &FLPCTL.flp_work.year, &CLK_REC, 6 );
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
						break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
				}
				flp_flag_clr( FLAP_CTRL_MODE3, LAG_TIME_OVER );
			}else{
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
			}
			ret = (char)-1;
			break;
		case 45:	// ���~ۯ���ϰ��ѱ��
			(void)flp_DownLock_DownSend( (ushort)(FLPCTL.Room_no-1) );
			break;
		default:
			break;
	}
	if (FLPCTL.flp_work.mode == FLAP_CTRL_MODE1) {
	// ��Ԃɕω�
		if (FLPCTL.flp_work.lag_to_in.BIT.FURIK) {
			// �U�֒��Ȃ�U�֌��Ԏ��o�ɏ���ʒm
			queset(OPETCBNO, CAR_FURIKAE_OUT, 0, 0);			// �U�֌��̎Ԃ��o��
			FLPCTL.flp_work.lag_to_in.BIT.FUKUG = 0;
			FLPCTL.flp_work.lag_to_in.BIT.FURIK = 0;
			FLPCTL.flp_work.lag_to_in.BIT.SYUUS = 0;			// <- �Ƃ肠�����N���A���Ă���
		}
	}
	if( snd20_flg == 1 ){
		ret = 1;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| ���~�ς�(�ԗ�����LT���Ē�)�̂Ƃ��̲���ď���                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : flp_faz6( evt )                                         |*/
/*| PARAMETER    : evt = ����ć�                                           |*/
/*| RETURN VALUE : ret = �ׯ���ް��������� 0:�Ȃ��A-1:����                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	flp_faz6( ushort evt )
{
	char	ret;
	struct	CAR_TIM		wok_tm;
	uchar	snd20_flg;

	snd20_flg = 0;

	ret = 0;
	switch( evt / 100 ){
		case 1:		// Car sensor ON
			flp_flag_clr( FLAP_CTRL_MODE2, CAR_SENSOR_ON );
			ret = (char)-1;
			break;
		case 2:		// Car sensor OFF
			Flp_LagExtCnt[FLPCTL.Room_no-1] = 0;					// ���O�^�C�������񐔃N���A
			flp_flag_clr( FLAP_CTRL_MODE1, CAR_SENSOR_OFF );
			IoLog_write(IOLOG_EVNT_OUT_GARAGE, (ushort)LockInfo[FLPCTL.Room_no - 1].posi, 0, 0);
// MH322914(S) K.Onodera 2016/12/07 ���~���b�N��Ԃŏo�ɂ����ꍇ�A�ݒ�ɂăt���b�v���~�����g���C����[���ʃo�ONo.1336](GW872707)
//			if(	(Aida_Flap_Chk((ushort)(FLPCTL.Room_no-1))) &&
//				( FLPCTL.flp_work.nstat.bits.b05 == 1 ) ){
//				// �p�c���ׯ�߂ŉ��~ۯ����̏ꍇ
//				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
//			}
			if(	( prm_get( COM_PRM,S_TYP,119,1,1 ) == 1 ) &&		// �t���b�v���~�����g���C����
				( FLPCTL.flp_work.nstat.bits.b05 == 1 )		){		// ���~���b�N����
				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
			}
			if( prm_get( COM_PRM,S_TYP,119,1,1 ) == 2 ){			// �t���b�v���~�𖳏����Ń��g���C����
				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
			}
// MH322914(E) K.Onodera 2016/12/07 ���~���b�N��Ԃŏo�ɂ����ꍇ�A�ݒ�ɂăt���b�v���~�����g���C����[���ʃo�ONo.1336](GW872707)
			InOutCountFunc( FLAP_CTRL_MODE6, (ushort)(evt / 100));

			ret = (char)-1;
			break;
	}

	switch( evt % 100 ){
		case 50:	// ��ԕω�
			if(( FLPCTL.flp_work.nstat.bits.b02 == 1 )&&			// �㏸����
			   ( FLPCTL.flp_work.nstat.bits.b01 == 1 )){			// �㏸�ς�(ۯ��ς�)
				FmvTimer( FLPCTL.Room_no, -1 );						// MT stop
			}
			else if(( FLPCTL.flp_work.nstat.bits.b02 == 0 )&&		// ���~����
					( FLPCTL.flp_work.nstat.bits.b01 == 0 )){		// ���~�ς�(ۯ��J�ς�)
				FmvTimer( FLPCTL.Room_no, -1 );						// MT stop
				flp_DownLockRetry_RecvSts( FLPCTL.Room_no );		// ��ԕω��ɂ�郊�g���C����
			}
			break;
		case 21:	// Payment complete
			break;
		case 23:	// �ׯ�ߏ㏸�w��(�C�����Z�p)
			if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){				// Car ON?
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE3;
				FlpSet( FLPCTL.Room_no, 1 );						// Flaper UP

				snd20_flg = InOutCountFunc( FLAP_CTRL_MODE6, (ushort)(evt % 100));
				memcpy( &FLPCTL.flp_work.year, &syusei[FLPCTL.Room_no-1].iyear, 6 );
				memset( &FLPCTL.flp_work.s_year, 0, 6 );
				syusei[FLPCTL.Room_no - 1].sei = 2;					// �C�����Ԏ��F���޽��эl��

				FLPCTL.flp_work.timer = -1;							// ׸���� ��ϰ�į��
				FLPCTL.flp_work.lag_to_in.BIT.SYUUS = 1;
			}else{
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
			}
			ret = (char)-1;
			break;
		case 24:	// �ׯ�߉��~�w��(�C�����Z�p)
#if	SYUSEI_PAYMENT		// SYUSEI_PAYMENT
			if( FLPCTL.flp_work.nstat.bits.b00 == 0 ){				// Car OFF?
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
				FlpSet( FLPCTL.Room_no, 2 );						// Flaper DOWN
				ret = -1;
			}
#endif	// SYUSEI_PAYMENT
			break;
		case 25:	// Mentenance Flaper Up
			Mnt_Lk_Move( ON );										// ����ݽ����u�ׯ�ߏ㏸�iۯ��j�v�v��
			ret = (char)-1;
			break;
		case 26:	// Mentenance Flaper Down
			if( Mnt_Lk_Move( OFF ) == OK ){							// ����ݽ����u�ׯ�߉��~�iۯ��J�j�v�v��
				// ���~ۯ���������ݽ�ŉ��~�������ꍇ�A׸���ς��ċN������
				if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){			// Car ON?
					FLPCTL.flp_work.timer = LagTimer( FLPCTL.Room_no );	// Lag time start
				}
			}
			ret = (char)-1;
			break;
		case 33:	// Flap move management timer over
			NTNET_Snd_Data05_Sub();
			ret = (char)-1;
			break;
		case 43:	// �ׯ�ߏ㏸(ۯ����u��)��ϰ
			break;
		case 42:	// Lag time timer over
			Flp_LagExtCnt[FLPCTL.Room_no-1] = 0;					// ���O�^�C�������񐔃N���A
			if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){				// Car ON?
				snd20_flg = InOutCountFunc( FLAP_CTRL_MODE6, (ushort)(evt % 100));
				IoLog_write(IOLOG_EVNT_RENT_GARAGE, (ushort)LockInfo[FLPCTL.Room_no - 1].posi, 0, 0);		/* �ē��ɏ��o�^ */
				if (FLPCTL.flp_work.lag_to_in.BIT.FUKUG) {
					// �U�֐��Z
					FLPCTL.flp_work.mode = FLAP_CTRL_MODE3;
					FlpSet( FLPCTL.Room_no, 1 );						// Flaper UP
					FLPCTL.flp_work.nstat.bits.b06 = 0;					// �s���ر
					FLPCTL.flp_work.nstat.bits.b07 = 0;					// ����ݽ�׸�OFF
					FLPCTL.flp_work.nstat.bits.b09 = 0;					// NT-NET�׸�OFF
					FLPCTL.flp_work.nstat.bits.b14 = 0;					// ���u����̑���w����ʃN���A
					FLPCTL.flp_work.nstat.bits.b15 = 0;					// ���u����̃t���b�v����v���N���A
					ret = (char)-1;
					break;	// ���Ɏ����Auketuke�Apasswd�Abk_syu�͂��̂܂�
				}

				switch( Carkind_Param(FLP_ROCK_INTIME, (char)LockInfo[FLPCTL.Room_no - 1].ryo_syu,1,3) ){	// ׸���ьo�ߌ�̓��Ɏ��ԁH
				
					case	0:		// �u���ݎ��ԁv����Ɏ��ԂƂ���
						memcpy( &FLPCTL.flp_work.year, &CLK_REC, 6 );
						break;

					case	1:		// �u���ݎ���-1���v����Ɏ��ԂƂ���
						wok_tm.year = CLK_REC.year;					// ����	�N
						wok_tm.mon  = CLK_REC.mont;					//		��
						wok_tm.day  = CLK_REC.date;					//		��
						wok_tm.hour = CLK_REC.hour;					//		��
						wok_tm.min  = CLK_REC.minu;					//		��
						wok_tm.week = CLK_REC.week;					//		�j��
						ec66( &wok_tm,(long)1 );					// ���t�X�V�����i���ݎ�����-1�����Z�j
						FLPCTL.flp_work.year = wok_tm.year;			// ����	�N
						FLPCTL.flp_work.mont = wok_tm.mon;			//		��
						FLPCTL.flp_work.date = wok_tm.day;			//		��
						FLPCTL.flp_work.hour = wok_tm.hour;			//		��
						FLPCTL.flp_work.minu = wok_tm.min;			//		��
						break;

					case	2:		// �u���Z���ԁv����Ɏ��ԂƂ���
						memcpy( &FLPCTL.flp_work.year, &FLPCTL.flp_work.s_year, 6 );
						break;

					default:		// ���̑��i���ݎ��Ԃ���Ɏ��ԂƂ���j
						memcpy( &FLPCTL.flp_work.year, &CLK_REC, 6 );
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
						break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
				}
				flp_flag_clr( FLAP_CTRL_MODE3, LAG_TIME_OVER );
			}else{
				FLPCTL.flp_work.mode = FLAP_CTRL_MODE1;
			}
			ret = (char)-1;
			break;
		case 45:	// ���~ۯ���ϰ��ѱ��
			if( 1 == flp_DownLock_DownSend( (ushort)(FLPCTL.Room_no-1) ) ){
				// ���~ۯ����ɉ��~�������ꍇ�A׸���ς��ċN������
				if( FLPCTL.flp_work.nstat.bits.b00 == 1 ){			// Car ON?
					FLPCTL.flp_work.timer = LagTimer( FLPCTL.Room_no );	// Lag time start
				}
			}
			break;
		default:
			break;
	}
	if (FLPCTL.flp_work.mode == FLAP_CTRL_MODE1) {
	// ��Ԃɕω�
		if (FLPCTL.flp_work.lag_to_in.BIT.FURIK) {
			// �U�֒��Ȃ�U�֌��Ԏ��o�ɏ���ʒm
			queset(OPETCBNO, CAR_FURIKAE_OUT, 0, 0);			// �U�֌��̎Ԃ��o��
			FLPCTL.flp_work.lag_to_in.BIT.FUKUG = 0;
			FLPCTL.flp_work.lag_to_in.BIT.FURIK = 0;
			FLPCTL.flp_work.lag_to_in.BIT.SYUUS = 0;
		}
	}
	if( snd20_flg == 1 ){
		ret = 1;
	}
	return( ret );
}


/*[]----------------------------------------------------------------------[]*/
/*| ү���ގ捞����                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FlpMsg( void )                                          |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ����ć�                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005.02.01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	FlpMsg( void )
{
	MsgBuf		*msb;

	union {
		unsigned short comd;
		unsigned char cc[2];
	} sc;

	ushort	ret = 0;
	ushort	dat;

	LkTimChk();

	if( lkreccnt == 0 ){
		if( (msb = GetMsg( FLPTCBNO )) == NULL ){
			return( 0 );
		}
		ret = 0;
		sc.comd = msb->msg.command;
		FLPCTL.Comd_knd = sc.cc[0];									// Command kind
		FLPCTL.Comd_cod = sc.cc[1];									// Command code
		FLPCTL.Ment_flg = 0;										// ����ݽ�׸޸ر

		switch( sc.comd ){
			case LK_DATAREC:										// �ް���M
				if( lkreccnt == 0 ){
					if( LKcom_RcvDataGet( &lkreccnt ) == 2 ){
						lkrecrpt = 0;
						ret = 99;									// �ԗ��y��ۯ���ԕω��L��
					}else{
						lkreccnt = 0;								// �ω��ް����ر
						lkrecrpt = 0;
						ret = 0;
					}
				}else{
					ret = 0;
				}
				break;
			case CAR_PAYCOM_SND:									// ���Z����
				memcpy( &FLPCTL.Room_no, msb->msg.data, 2 );		// �Ԏ������
				syusei[FLPCTL.Room_no - 1].ot_car = 1;
				ret = 21;
				break;
			case CAR_SVSLUG_SND:									// �ׯ�ߏ㏸(ۯ����u��)��ϰ,׸������ѱ���
				memcpy( &FLPCTL.Room_no, msb->msg.data, 2 );		// �Ԏ������
				dat = FLAPDT.flp_data[ FLPCTL.Room_no - 1 ].mode;
				if(( dat == FLAP_CTRL_MODE5 )||( dat == FLAP_CTRL_MODE6 )){
					ret = 42;										// ׸������ѱ��
					syusei[FLPCTL.Room_no - 1].ot_car = 5;
				}else{
					ret = 43;										// �ׯ�ߏ㏸(ۯ����u��)��ϰ��ѱ��
					syusei[FLPCTL.Room_no - 1].ot_car = 4;
				}
				break;
			case CAR_INCHK_SND:										// ���ɔ�����ѱ���
				memcpy( &FLPCTL.Room_no, msb->msg.data, 2 );		// �Ԏ������
				ret = 44;											// ���ɔ�����ѱ��
				break;
			case CAR_FLPMOV_SND:									// �ׯ�ߔ���Ď���ϰ��ѱ���
				memcpy( &FLPCTL.Room_no, msb->msg.data, 2 );		// �Ԏ������
				ret = 33;
				break;
			case FLAP_UP_SND:										// �ׯ�ߏ㏸�w��(����ݽ)
			case FLAP_UP_SND_NTNET:									// �ׯ�ߏ㏸�w��(NTNET)
			case FLAP_UP_SND_MAF:									// �ׯ�ߏ㏸�w��(MAF)
				memcpy( &FLPCTL.Room_no, msb->msg.data, 2 );		// �Ԏ������
				if( sc.comd == FLAP_UP_SND ){
					FLPCTL.Ment_flg = 0;							// ����ݽ�ŏ㏸�w��
				}else if( sc.comd == FLAP_UP_SND_MAF ){				// �ׯ�ߏ㏸�w��(MAF)
					FLPCTL.Ment_flg = 2;							// MAF�ŏ㏸�w��
				}else{
					FLPCTL.Ment_flg = 1;							// NTNET�ŏ㏸�w��
				}
				ret = 25;
				break;
			case FLAP_DOWN_SND:										// �ׯ�߉��~�w��(����ݽ)
			case FLAP_DOWN_SND_NTNET:								// �ׯ�߉��~�w��(NTNET)
			case FLAP_DOWN_SND_MAF:									// �ׯ�ߏ㏸�w��(MAF)
				memcpy( &FLPCTL.Room_no, msb->msg.data, 2 );		// �Ԏ������
				if( sc.comd == FLAP_DOWN_SND ){
					FLPCTL.Ment_flg = 0;							// ����ݽ�ŉ��~�w��
				}else if( sc.comd == FLAP_DOWN_SND_MAF ){			// �ׯ�߉��~�w��(MAF)
					FLPCTL.Ment_flg = 2;							// MAF�ŉ��~�w��
				}else{
					FLPCTL.Ment_flg = 1;							// NTNET�ŉ��~�w��
				}
				ret = 26;
				break;
			case FLAP_UP_SND_SS:									// �ׯ�ߏ㏸�w��(�C�����Z�p)
				memcpy( &FLPCTL.Room_no, msb->msg.data, 2 );		// �Ԏ������
				ret = 23;
				break;
			case FLAP_DOWN_SND_SS:									// �ׯ�߉��~�w��(�C�����Z�p)
				memcpy( &FLPCTL.Room_no, msb->msg.data, 2 );		// �Ԏ������
				ret = 24;
				break;
			case LK_SND_P_CHK:
				memset( MntLockTest, 0xff, sizeof( MntLockTest ));
			case LK_SND_CLS:										// IF�Ղ֐����ް����M�v��(��)
			case LK_SND_OPN:										// IF�Ղ֐����ް����M�v��(�J)
			case LK_SND_STS:										// IF�Ղ֐����ް����M�v��(��ԗv��)
			case LK_SND_TEST:										// IF�Ղ֐����ް����M�v��(�J��ý�)
			case LK_SND_CNT:										// IF�Ղ֐����ް����M�v��(���춳�ĸر)
				memcpy( &FLPCTL.Room_no, msb->msg.data, 2 );		// �Ԏ������
				if( ( FLPCTL.Room_no >= 1 )&&( LOCK_MAX >= FLPCTL.Room_no ) ){
					LKopeApiLKCtrl( FLPCTL.Room_no, (uchar)( FLPCTL.Comd_cod & 0x0f ) );	// �����ް����M�v��
				}
				break;
			case LK_SND_A_STS:										// IF�Ղ֐����ް����M�v��(�S��ԗv��)
			case LK_SND_MNT:										// IF�Ղ֐����ް����M�v��(��Ï��v��)
			case LK_SND_A_TEST:										// IF�Ղ֐����ް����M�v��(�S�J��ý�)
			case LK_SND_A_CNT:										// IF�Ղ֐����ް����M�v��(�S���춳�ĸر)
			case LK_SND_VER:										// IF�Ղ֐����ް����M�v��(�ް�ޮݗv��)
			case LK_SND_ERR:										// IF�Ղ֐����ް����M�v��(�װ��ԗv��)
			case LK_SND_A_LOOP_DATA:								// ���[�v�f�[�^�v��
				LKopeApiLKCtrl_All( msb->msg.data[0], (uchar)( FLPCTL.Comd_cod & 0x0f ) );	// �����ް����M�v��
				break;
			case FLAP_A_UP_SND_MNT:									// �S�ׯ�ߏ㏸�v��(����ݽ)
				Lk_AllMove( LK_KIND_FLAP, 1, 0 );
				break;
			case FLAP_A_DOWN_SND_MNT:								// �S�ׯ�߉��~�v��(����ݽ)
				Lk_AllMove( LK_KIND_FLAP, 0, 0 );
				break;
			case LOCK_A_CLS_SND_MNT:								// �Sۯ����u�v��(����ݽ)
				Lk_AllMove( LK_KIND_LOCK, 1, 0 );
				break;
			case LOCK_A_OPN_SND_MNT:								// �Sۯ����u�J�v��(����ݽ)
				Lk_AllMove( LK_KIND_LOCK, 0, 0 );
				break;
			case INT_FLAP_A_UP_SND_MNT:								// �S�����ׯ�ߏ㏸�v��(����ݽ)
				Lk_AllMove( LK_KIND_INT_FLAP, 1, 0 );
				break;
			case INT_FLAP_A_DOWN_SND_MNT:							// �S�����ׯ�߉��~�v��(����ݽ)
				Lk_AllMove( LK_KIND_INT_FLAP, 0, 0 );
				break;
			case FLAP_A_UP_SND_NTNET:								// �S�ׯ�ߏ㏸�v��(NTNET)
				memcpy( &dat, msb->msg.data, 2 );					// ��ʃZ�b�g
				Lk_AllMove( LK_KIND_FLAP, 1, (dat==1?2:1) );
				break;
			case FLAP_A_DOWN_SND_NTNET:								// �S�ׯ�߉��~�v��(NTNET)
				memcpy( &dat, msb->msg.data, 2 );					// ��ʃZ�b�g
				Lk_AllMove( LK_KIND_FLAP, 0, (dat==1?2:1) );
				break;
			case LOCK_A_CLS_SND_NTNET:								// �Sۯ����u�v��(NTNET)
				memcpy( &dat, msb->msg.data, 2 );					// ��ʃZ�b�g
				Lk_AllMove( LK_KIND_LOCK, 1, (dat==1?2:1) );
				break;
			case LOCK_A_OPN_SND_NTNET:								// �Sۯ����u�J�v��(NTNET)
				memcpy( &dat, msb->msg.data, 2 );					// ��ʃZ�b�g
				Lk_AllMove( LK_KIND_LOCK, 0, (dat==1?2:1) );
				break;
			case INT_FLAP_A_UP_SND_NTNET:							// �S�����ׯ�ߏ㏸�v��(NTNET)
				memcpy( &dat, msb->msg.data, 2 );					// ��ʃZ�b�g
				Lk_AllMove( LK_KIND_INT_FLAP, 1, (dat==1?2:1) );
				break;
			case INT_FLAP_A_DOWN_SND_NTNET:							// �S�����ׯ�߉��~�v��(NTNET)
				memcpy( &dat, msb->msg.data, 2 );					// ��ʃZ�b�g
				Lk_AllMove( LK_KIND_INT_FLAP, 0, (dat==1?2:1) );
				break;
			case DOWNLOCKOVER_SND:									// ���~ۯ���ѱ���
				memcpy( &FLPCTL.Room_no, msb->msg.data, 2 );		// �Ԏ������
				ret = 45;											// 
				break;
			case LK_SND_FORCE_LOOP_ON:								// �������[�vON
			case LK_SND_FORCE_LOOP_OFF:								// �������[�vOFF
				memcpy( &FLPCTL.Room_no, msb->msg.data, 2 );		// �Ԏ������
				if( ( FLPCTL.Room_no >= 1 )&&( LOCK_MAX >= FLPCTL.Room_no ) ){
					LKopeApiLKCtrl( FLPCTL.Room_no, (uchar)( FLPCTL.Comd_cod & 0xff ) );	// �����ް����M�v��
				}
				break;
			default:
				ex_errlg( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_EVENTOUTOFRULE, 2, 0 );	// �K��O����Ĕ���
				break;
		}
		FreeBuf( msb );
	}
	if(( lkreccnt )||( ret == 99 )){
		/* ��M�����ް��͕����Ԏ����ް����܂�ł��邽�߁A������������	*/
		/* ������1�Ԏ����ɱ���ق֖߂�A���̑�������ւ̕��S�����炷		*/
		for( ; lkrecrpt < lkreccnt; ){
			ret = 0;
			if( LKcom_RcvDataAnalys( lkrecrpt ) != 2 ){
				lkrecrpt++;
				continue;
			}
			if( LockBuf[lkrecrpt].LockNoBuf == 0xffff ){			// ����ȍ~�ް��Ȃ�
				lkreccnt = 0;										// �ω��ް����ر
				lkrecrpt = 0;
				break;
			}
			if( LockBuf[lkrecrpt].LockNoBuf == 0xfffe ){			// ��ԕω��Ȃ�
				lkrecrpt++;
				continue;
			}
			FLPCTL.Room_no = LockBuf[lkrecrpt].LockNoBuf + 1;		// ���Ԉʒu�ԍ�(1�`324)
			dat = FLAPDT.flp_data[LockBuf[lkrecrpt].LockNoBuf].nstat.word ^
					FLAPDT.flp_data[LockBuf[lkrecrpt].LockNoBuf].ostat.word;
			if( dat & 0x0001 ){										// �ԗ��L���ω�
				if( FLAPDT.flp_data[LockBuf[lkrecrpt].LockNoBuf].nstat.bits.b00 == 1 ){
					/*** �ԗ��Ȃ�=>���� ***/
					ret += 100;
					syusei[FLPCTL.Room_no - 1].ot_car = 3;
					//���ɑ䐔���Ĉʒu�ړ�
				}else{
					/*** �ԗ�����=>�Ȃ� ***/
					ret += 200;
					syusei[FLPCTL.Room_no - 1].ot_car = 2;
					//�o�ɑ䐔���Ĉʒu�ړ�
				}
			}
			if( dat & 0x0036 ){										// ۯ����u��ԕω�
				ret += 50;
			}
			else{
				// ���̏����ɂ�LKcom_RcvDataAnalys()�ŏ�ԕω�����iret==2�j�ŕԂ��Ȃ���������Ă��Ȃ��ׁA���񂩂ǂ����̔���͏ȗ�
				if( LockBuf[lkrecrpt].lock_cnd == 4 ){				// ��Ԃ����~���b�N�����Ŏ�M�����ꍇ
					ret += 50;
				}
				else if( (dat & 0x0001) == 0 &&						// �ԗ��L���ω��Ȃ�
						 LockBuf[lkrecrpt].lock_cnd == 1 ){			// ��Ԃ��㏸�ς݂Ŏ�M�����ꍇ
					ret += 50;
				}
			}
			lkrecrpt++;
			break;
		}
		if( lkrecrpt == lkreccnt ){
			lkreccnt = 0;											// �ω��ް����ر
			lkrecrpt = 0;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| ���ɔ��莞�ԋN��(500msec��ϰ�g�p)                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : InChkTimer                                              |*/
/*| PARAMETER    : no  = �Ԏ���1�`324                                      |*/
/*| RETURN VALUE : ret = ���ɔ��莞��                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2007-09-03                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	InChkTimer( void )
{
	short	tim;

	if( Lock_Kind == LK_KIND_FLAP || Lock_Kind == LK_KIND_INT_FLAP ){// ���u��ʁH
		tim = (short)Carkind_Param(IN_CHK_TIME,(char)LockInfo[FLPCTL.Room_no - 1].ryo_syu,3,4);					// �b�̐ݒ���i�[����
	}else{
		tim = (short)Carkind_Param(IN_CHK_TIME,(char)LockInfo[FLPCTL.Room_no - 1].ryo_syu,3,1);					// �b�̐ݒ���i�[����
	}

	tim = tim * 2;												// ����500ms�P�ʂɕϊ�

	if( tim == 0 ){					// �ݒ�0,��ʈُ펞�͍ŏ��l�Ƃ���
		tim = 1;
	}

	return( tim );
}
/*[]----------------------------------------------------------------------[]*/
/*| �ׯ�ߏ㏸(ۯ����u��)��ϰ�N��(500msec��ϰ�g�p)                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LockTimer                                               |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret = �ׯ�ߏ㏸(ۯ����u��)��ϰ                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
long	LockTimer( void )
{
	long	tim;
// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH322914 (s) kasiyama 2016/07/12 ���b�N�^�C�����T�[�r�X�^�C���܂ŉ�������[���ʃo�ONo.1211](MH341106)
//	long			svstime;			// ���޽��ѐݒ�l
//	uchar			syu;				// �Ԏ�
//// MH322914 (e) kasiyama 2016/07/12 ���b�N�^�C�����T�[�r�X�^�C���܂ŉ�������[���ʃo�ONo.1211](MH341106)
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)

	if( Lock_Kind == LK_KIND_FLAP || Lock_Kind == LK_KIND_INT_FLAP ){// ���u��ʁH
		// �ׯ��
		tim = Carkind_Param(FLAP_UP_TIMER, (char)(LockInfo[FLPCTL.Room_no - 1].ryo_syu), 5,1);
	}
	else{
		// ۯ����u
		tim = Carkind_Param(ROCK_CLOSE_TIMER, (char)(LockInfo[FLPCTL.Room_no - 1].ryo_syu), 5,1);
	}
	tim = (long)(((tim/100)*60) + (tim%100));

// MH322914 (s) kasiyama 2016/07/12 ���b�N�^�C�����T�[�r�X�^�C���܂ŉ�������[���ʃo�ONo.1211](MH341106)
// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	if( prm_get( COM_PRM, S_TYP, 68, 1, 5 ) == 1 ) {
//		// ���b�N�^�C�}�[�ƃT�[�r�X�^�C�����������Ԃ̎��Ƀ��b�N�^�C�����T�[�r�X�^�C���܂ŉ�������
//		if( prm_get( COM_PRM,S_STM,1,1,1 ) == 0 ){	// �S���ʻ��޽��сH
//			svstime = CPrmSS[S_STM][2];				// ���ʻ��޽��ѐݒ�l�擾
//		}else{
//			syu = LockInfo[FLPCTL.Room_no-1].ryo_syu;
//			svstime = CPrmSS[S_STM][5+(3*(syu-1))];	// ��ʖ����޽��ѐݒ�l�擾
//		}
//		svstime *= 60;	/* �T�[�r�X�^�C���F�b	*/
//		if(svstime == tim) {
//			// �T�[�r�X�^�C���I���Ń��b�N����悤�ɂ���
//			tim += 60 - CLK_REC.seco;
//		}
//	}
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
// MH322914 (e) kasiyama 2016/07/12 ���b�N�^�C�����T�[�r�X�^�C���܂ŉ�������[���ʃo�ONo.1211](MH341106)

	tim *= 2;														// 500ms�P�ʂɕϊ�

	if( tim == 0 ){													// �ݒ�0���͍ŏ��l�Ƃ���
		tim = 1;
	}
	return( tim );
}

/*[]----------------------------------------------------------------------[]*/
/*| ׸���ыN��(500msec��ϰ�g�p)                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LagTimer                                                |*/
/*| PARAMETER    : no  = �Ԏ���1�`324                                      |*/
/*| RETURN VALUE : ret = ׸����                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
long	LagTimer( ushort no )
{
	long	tim;
	uchar	syu;

	tim = 0;														// ���O�^�C���l������
	if( prm_get( COM_PRM,S_STM,1,1,1 ) == 0 ){						// �S�Ԏ틤��?
		/*** ׸���т͑S�Ԏ틤�� ***/
		tim = (long)CPrmSS[S_STM][4];
	}else{
		/*** ׸���т͎Ԏ했�ňقȂ� ***/
		syu = SvsTime_Syu[no-1];									// �Ԏ����̐��Z���̗������get
		if(( 1 <= syu )&&( syu <= 12 )){
			tim = (long)CPrmSS[S_STM][7+((syu-1)*3)];
		}
	}
	tim = tim * 60 * 2;												// ����500ms�P�ʂɕϊ�
	if( tim == 0 ){													// �ݒ肪�O,��ʔ͈͊O
		tim = 1200L;												// �����I��10���ɂ���
	}

	return( tim );
}

/*[]----------------------------------------------------------------------[]*/
/*| �ׯ�ߔ㏸/���~�J�n                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FlpSet( no, updw )                                      |*/
/*| PARAMETER    : no   = �Ԏ���1�`324                                     |*/
/*|              : updw = 1:�㏸(��), 2:���~(�J)                           |*/
/*|              : wait = 0:�ʏ펞����,1:����҂����                      |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	FlpSet( ushort no, uchar updw )
{
	short	tim;

	if( updw == 2 ){
		FLPCTL.flp_work.nstat.bits.b02 = 0;							// ���~����
	}else{
		FLPCTL.flp_work.nstat.bits.b02 = 1;							// �㏸����
	}
	IFM_LockTable.ucFlapWait = 1;									// �t���b�v���쒆
	LKopeApiLKCtrl( no, updw );										// �ׯ��(ۯ����u)����w��

	if( Aida_Flap_Chk((ushort)(no-1)) ) {							// ���u��ʂ��p�c���t���b�v
		flp_DownLock_FlpSet( (ushort)(no-1) );						// �t���b�v����w�����{��̉��~���b�N�ی쏈��
	}

	if( Lock_Kind == LK_KIND_FLAP || Lock_Kind == LK_KIND_INT_FLAP ){// ���u��ʁH
		// �ׯ��
		tim = 300;													// �ׯ�ߓ���Ď���ϰ�N��(150s)
	}
	else{
		// ۯ����u
		tim = 60;													// ۯ����u����Ď���ϰ�N��(30s)
	}
	FmvTimer( no, tim );											// ����Ď���ϰ�N��
}

/*[]----------------------------------------------------------------------[]*/
/*| �ׯ�ߔ���Ď���ϰ�N��(500ms��ϰ�g�p)                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FmvTimer( no, tim )                                     |*/
/*| PARAMETER    : no = �Ԏ���1�`324                                       |*/
/*|              : tim = ��ϰ�l(500ms�P��)                                 |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	FmvTimer( ushort no, short tim )
{
	FLPCTL.Flp_mv_tm[no-1] = tim;									 // �ׯ�ߔ���Ď���ϰ���

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| �ׯ�ߏ㏸(ۯ����u��)��ϰ�A׸������ϰ�A                                 |*/
/*| �ׯ�ߔ���Ď���ϰ���Z(500ms��ϰ�g�p)                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SrvTimMng                                               |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	SrvTimMng( void )
{
	short	i;

	for( i = 0; i < LOCK_MAX; i++ ){
		WACDOG;																// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		// �ׯ�ߏ㏸(ۯ����u��)��ϰ�A׸������ϰ�Ď�
		if( FLAPDT.flp_data[i].timer > 0 ){
			FLAPDT.flp_data[i].timer--;
		}
		// �ׯ�ߔ�(ۯ����u)����Ď���ϰ�Ď�
		if( FLPCTL.Flp_mv_tm[i] > 0 ){
			FLPCTL.Flp_mv_tm[i]--;
		}
		// ���ɔ�����ϰ
		if( FLAPDT.flp_data[i].in_chk_cnt > 0){
			FLAPDT.flp_data[i].in_chk_cnt--;
		}
		/* �t���b�v���~���b�N�������ی쏈�� */
		if( (flp_DownLockErrInfo[i].f_info.BIT.EXEC == 1) &&				// ���g���C���쒆
			(flp_DownLockErrInfo[i].TimerCount != 0) )						// �^�C���A�E�g���Ă��Ȃ�
		{
			flp_DownLockErrInfo[i].TimerCount--;							// �^�C�}�[�J�E���g
		}
	}

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| �ׯ�ߏ㏸���~�ُ�(ۯ����u�J�ُ�)������ϰ�Ď�����(5s��ϰ�g�p)         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LkErrTimMng                                             |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-09-27                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LkErrTimMng( void )
{
	short	i;

	for( i = 0; i < LOCK_MAX; i++ ){
		WACDOG;																// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		// �㏸ۯ�(�ُ�)�װ������ϰ
		if( FLPCTL.Flp_uperr_tm[i] > 0 ){
			FLPCTL.Flp_uperr_tm[i]--;
		}
		// ���~ۯ�(�J�ُ�)�װ������ϰ
		if( FLPCTL.Flp_dwerr_tm[i] > 0 ){
			FLPCTL.Flp_dwerr_tm[i]--;
		}
	}

	return;
}


/*[]----------------------------------------------------------------------[]*/
/*| �ׯ�ߏ㏸(ۯ����u��)��ϰ�A׸������ϰ�A�ׯ�ߔ���Ď���ϰ�̊Ď�����    |*/
/*| ��ѱ�Ă������͏���ү���ޓo�^����B                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LkTimChk                                                |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-09-21                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	LkTimChk( void )
{
	ushort	no;
	t_flp_DownLockErrInfo *pInfo = &flp_DownLockErrInfo[lktimoutno];

	//���ɔ��莞�ԊĎ�
	if( FLAPDT.flp_data[lktimoutno].in_chk_cnt == 0){	// ���ɔ�����ѱ��
		
		no = lktimoutno + 1;
		queset( FLPTCBNO, CAR_INCHK_SND, sizeof( no ), &no );
		FLAPDT.flp_data[lktimoutno].in_chk_cnt = -1;				// ��ѱ��
		/* ���ɏ��o�^ */
		IoLog_write(IOLOG_EVNT_ENT_GARAGE, (ushort)LockInfo[lktimoutno].posi, 0, 0);
	}
	// �ׯ�ߏ㏸(ۯ����u��)��ϰ�A׸������ϰ�Ď�
	if( FLAPDT.flp_data[lktimoutno].timer == 0 ){					// ��ѱ��?

		if( (FLAPDT.flp_data[lktimoutno].lag_to_in.BYTE < _FLP_FURIKAE )||
			(FLAPDT.flp_data[lktimoutno].lag_to_in.BIT.SYUUS) ){

			FLAPDT.flp_data[lktimoutno].lag_to_in.BIT.SYUUS = 0;
		no = lktimoutno + 1;
		queset( FLPTCBNO, CAR_SVSLUG_SND, sizeof( no ), &no );
		FLAPDT.flp_data[lktimoutno].timer = -1;						// ��ϰ�į��
		} else {
			// �U�֐��Z���ɂ��A�^�C���A�E�g�ʒm/�^�C�}�[��~�͍s��Ȃ�
			FLAPDT.flp_data[lktimoutno].timer = 1;
		}
	}

	/* �t���b�v���~���b�N�������ی쏈�� */
	if( (pInfo->f_info.BIT.EXEC == 1) &&							// ���g���C���쒆
		(pInfo->TimerCount == 0) &&									// �^�C���A�E�g���Ă���
		(pInfo->f_info.BIT.TO_MSG == 0) )							// �^�C���A�E�g���b�Z�[�W�����M
	{
		no = lktimoutno + 1;										// �Ώۂ̎Ԏ��ԍ����
		queset( FLPTCBNO, DOWNLOCKOVER_SND, sizeof( no ), &no );	// ���~���b�N�^�C���I�[�o�[�ʒm
		pInfo->f_info.BIT.TO_MSG = 1;								// �^�C���A�E�g���b�Z�[�W���M�ς�
	}

	// �ׯ�ߔ�(ۯ����u)����Ď���ϰ�Ď�
	if( FLPCTL.Flp_mv_tm[lktimoutno] == 0 ){						// ��ѱ��?
		no = lktimoutno + 1;
		queset( FLPTCBNO, CAR_FLPMOV_SND, sizeof( no ), &no );
		FLPCTL.Flp_mv_tm[lktimoutno] = -1;							// ��ϰ�į��
	}

	// �㏸ۯ�(�ُ�)�װ������ϰ�Ď�
	if( FLPCTL.Flp_uperr_tm[lktimoutno] == 0 ){						// ��ѱ��?
		FLPCTL.Flp_uperr_tm[lktimoutno] = -1;						// ��ϰ�į��
		if( FLAPDT.flp_data[lktimoutno].nstat.bits.b02 == 1 ){		// �㏸����(����ُ�)
			if( FLAPDT.flp_data[lktimoutno].nstat.bits.b04 == 1 ){	// �㏸ۯ�(��ۯ�)
				ErrBinDatSet( (ulong)lktimoutno, 1 );
				if (lktimoutno < INT_CAR_START_INDEX ) {
					err_chk( ERRMDL_IFFLAP, ERR_FLAPCLOSEFAIL, 1, 2, 1 );
				}else if( lktimoutno < BIKE_START_INDEX ){
					err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_LOCKCLOSEFAIL, 1, 2, 1 );
				} else {
					err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_LOCKCLOSEFAIL, 1, 2, 1 );
				}
			}
		}
	}
	// ���~ۯ�(�J�ُ�)�װ������ϰ�Ď�
	if( FLPCTL.Flp_dwerr_tm[lktimoutno] == 0 ){						// ��ѱ��?
		FLPCTL.Flp_dwerr_tm[lktimoutno] = -1;						// ��ϰ�į��
		if( FLAPDT.flp_data[lktimoutno].nstat.bits.b02 == 0 ){		// ���~����(�J����ُ�)
			if( FLAPDT.flp_data[lktimoutno].nstat.bits.b05 == 1 ){	// ���~ۯ�(�Jۯ�)
				ErrBinDatSet( (ulong)lktimoutno, 1 );
				if (lktimoutno < INT_CAR_START_INDEX ) {
					err_chk( ERRMDL_IFFLAP, ERR_FLAPOPENFAIL, 1, 2, 1 );
				}else if( lktimoutno < BIKE_START_INDEX ){
					err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_LOCKOPENFAIL, 1, 2, 1 );
				} else {
					err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_LOCKOPENFAIL, 1, 2, 1 );
				}
			}
		}
	}

	lktimoutno++;
	if( lktimoutno >= LOCK_MAX ){
		lktimoutno = 0;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| NT-NETۯ����䌋���ް��쐬����(���삳���Ȃ��������p)                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data05_Sub                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-09-12                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data05_Sub( void )
{
	uchar	CarCnd, LockCnd;
	void	(*_ntnet_snd_data05)(ulong, uchar, uchar);		/* �֐��|�C���^ */

	_ntnet_snd_data05 = NTNET_Snd_Data05;		// �ʏ��NT-NET�ւ̑��M�|�C���^���Z�b�g

	CarCnd = 0;
	LockCnd = 0;

	if( FLPCTL.Ment_flg == 0 && FLPCTL.flp_work.nstat.bits.b09 == 0 ){	// NT-NET?
		return;
	}

	if( FLPCTL.flp_work.nstat.bits.b08 == 0 ){						// �ڑ��L��?
		if( FLPCTL.flp_work.nstat.bits.b00 ){						// �ԗ��L��?
			CarCnd = 1;												// �ԗ��L��
		}else{
			CarCnd = 2;												// �ԗ�����
		}
		if( FLPCTL.flp_work.nstat.bits.b01 ){						// �㏸�ς�?
			if( FLPCTL.flp_work.nstat.bits.b04 ){					// �㏸ۯ�?
				LockCnd = 3;										// ۯ��ُ�
			}else{
				LockCnd = 1;										// ۯ��ς�
			}
		}else{
			if( FLPCTL.flp_work.nstat.bits.b05 ){					// ���~ۯ�?
				LockCnd = 4;										// ۯ��J�ُ�
			}else{
				LockCnd = 2;										// ۯ��J�ς�
			}
		}
	}else{
		CarCnd = 0;													// �ڑ�����
		LockCnd = 6;												// �w��ۯ����u�ڑ�����
	}

	_ntnet_snd_data05( (ulong)( LockInfo[FLPCTL.Room_no - 1].area*10000L + LockInfo[FLPCTL.Room_no - 1].posi ),
					  CarCnd, LockCnd );							// NT-NETۯ����䌋���ް��쐬
	FLPCTL.flp_work.nstat.bits.b09 = 0;								// NT-NET�׸�OFF
}

/*[]----------------------------------------------------------------------[]*/
/*| ���Ɏ��̑䐔���ď���                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lk_InCount                                              |*/
/*| PARAMETER    : uc_prm : ���Ă���^���Ȃ��̐ݒ�                         |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| ���o�ɑ䐔���Ă�CarCount�̈�ōs���AT�W�v�󎚎��ɏW�v�ر�։��Z����B   |*/
/*| �󎚒����䐔���Ă��A�󎚊�������T�W�v�ر��ر�������ݸނ�CarCount�̈�  |*/
/*| ��T�W�v�̈���r���󎚒��ɑ������䐔�����߁A�������䐔��CarCount�̈�  |*/
/*| ���X�V����B����ɂ��󎚒��̑䐔���Ă͎���T�W�v�ɔ��f�����B        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2006-02-28                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static	void	Lk_InCount( uchar uc_prm )
{
uchar	set1_39_5;
ulong	ul_lkNo_w = 0;												// �ڋq�pۯ����u��

	set1_39_5 = (uchar)prm_get(COM_PRM,S_SYS,39,1,2);

	if( uc_prm == 1 ){												// ���ݑ䐔1���Ă���ݒ�?
		if( set1_39_5 == 2 ){										// �{�̂ɂăJ�E���g����
			PPrmSS[S_P02][6]++;										// ���ݑ䐔1
		}
		CarCount.In_car_cnt[0]++;									// ���ɑ䐔1
	}else if( uc_prm == 2 ){										// ���ݑ䐔2���Ă���ݒ�?
		if( set1_39_5 == 2 ){										// �{�̂ɂăJ�E���g����
			PPrmSS[S_P02][10]++;									// ���ݑ䐔2
		}
		CarCount.In_car_cnt[1]++;									// ���ɑ䐔2
	}else if( uc_prm == 3 ){										// ���ݑ䐔3���Ă���ݒ�?
		if( set1_39_5 == 2 ){										// �{�̂ɂăJ�E���g����
			PPrmSS[S_P02][14]++;									// ���ݑ䐔3
		}
		CarCount.In_car_cnt[2]++;									// ���ɑ䐔3
	}
	if( uc_prm ){													// ��ʖ����Ă���ݒ�?
		if( set1_39_5 == 2 ){										// �{�̂ɂăJ�E���g����
			PPrmSS[S_P02][2]++;										// ���ݑ䐔
			if( _is_ntnet_remote() && (prm_get(COM_PRM,S_NTN,121,1,1) != 0) ){	// ���uNT-NET�ݒ� & ���ԏ�Z���^�[�`���̂Ƃ��̂݃`�F�b�N
				if( (prm_get(COM_PRM,S_NTN,120,3,1)!=0) && (NT_pcars_timer == -1) ) {
					NT_pcars_timer = (short)(prm_get(COM_PRM,S_NTN,120,3,1) * 50);
				}
			}
		}
		CarCount.In_car_Tcnt++;										// �����ɑ䐔
	}
	//���j�^�[�o�^
	ul_lkNo_w = (ulong)( LockInfo[FLPCTL.Room_no - 1].area*10000L +
					  LockInfo[FLPCTL.Room_no - 1].posi );	// �ڋq�pۯ����u��
					  
	wmonlg(OPMON_CAR_IN,NULL,ul_lkNo_w);								// ����o�^�i���Ɂj
}


/*[]----------------------------------------------------------------------[]*/
/*| �o�Ɏ��̑䐔���ď���                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lk_OutCount                                             |*/
/*| PARAMETER    : uc_prm : ���Ă���^���Ȃ��̐ݒ�                         |*/
/*| 		     : mode : 0:�ݎ��ް����M���� 1:�ݎ��ް����M�Ȃ�            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2006-02-28                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static	void	Lk_OutCount( uchar uc_prm, uchar mode )
{

uchar	set1_39_5;
ulong	ul_lkNo_w = 0;												// �ڋq�pۯ����u��

	set1_39_5 = (uchar)prm_get(COM_PRM,S_SYS,39,1,2);

	if( uc_prm == 1 ){												// ���ݑ䐔1���Ă���ݒ�?
		if( set1_39_5 == 2 ){										// �{�̂ɂăJ�E���g����
			if( PPrmSS[S_P02][6] > 0 ){
				PPrmSS[S_P02][6]--;										// ���ݑ䐔1
			}
		}
		CarCount.Out_car_cnt[0]++;									// �o�ɑ䐔1
	}else if( uc_prm == 2 ){										// ���ݑ䐔2���Ă���ݒ�?
		if( set1_39_5 == 2 ){										// �{�̂ɂăJ�E���g����
			if( PPrmSS[S_P02][10] > 0 ){
				PPrmSS[S_P02][10]--;									// ���ݑ䐔2
			}
		}
		CarCount.Out_car_cnt[1]++;									// �o�ɑ䐔2
	}else if( uc_prm == 3 ){										// ���ݑ䐔3���Ă���ݒ�?
		if( set1_39_5 == 2 ){										// �{�̂ɂăJ�E���g����
			if( PPrmSS[S_P02][14] > 0 ){
				PPrmSS[S_P02][14]--;									// ���ݑ䐔3
			}
		}
		CarCount.Out_car_cnt[2]++;									// �o�ɑ䐔3
	}
	if( uc_prm ){													// ��ʖ����Ă���ݒ�?
		if( set1_39_5 == 2 ){										// �{�̂ɂăJ�E���g����
			if( PPrmSS[S_P02][2] > 0 ){
				PPrmSS[S_P02][2]--;										// ���ݑ䐔-1
				if( _is_ntnet_remote() && (prm_get(COM_PRM,S_NTN,121,1,1) != 0) ){	// ���uNT-NET�ݒ� & ���ԏ�Z���^�[�`���̂Ƃ��̂݃`�F�b�N
					if( (prm_get(COM_PRM,S_NTN,120,3,1)!=0) && (NT_pcars_timer == -1) ) {
						NT_pcars_timer = (short)(prm_get(COM_PRM,S_NTN,120,3,1) * 50);
					}
				}
			}
		}
		CarCount.Out_car_Tcnt++;									// ���o�ɑ䐔
	}
	//���j�^�[�o�^
	ul_lkNo_w = (ulong)( LockInfo[FLPCTL.Room_no - 1].area*10000L +
					  LockInfo[FLPCTL.Room_no - 1].posi );	// �ڋq�pۯ����u��
					  
	wmonlg(OPMON_CAR_OUT,NULL,ul_lkNo_w);								// ����o�^�i�o�Ɂj

}

/*[]----------------------------------------------------------------------[]*/
/*| ۯ����u��ʎ擾����                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LkKind_Get                                              |*/
/*| PARAMETER    : no : ���uNo.                                            |*/
/*| RETURN VALUE : ۯ����u���                                             |*/
/*| 				LK_KIND_FLAP = �ׯ��	                               |*/
/*| 				LK_KIND_LOCK = ۯ����u	                               |*/
/*|					LK_KIND_INT_FLAP = �����ׯ��						   |*/
/*| 				LK_KIND_ERR  = �s��	                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| ���Ұ��Ŏw�肳�ꂽۯ����u���ׯ�߂Ȃ̂�ۯ����u�Ȃ̂����肵�o�͂���      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Motohashi                                             |*/
/*| Date         : 2006-09-11                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
uchar	LkKind_Get( ushort no )
{
	uchar	kind = LK_KIND_ERR;

	if( ( no >= FLAP_START_NO ) && ( no <= FLAP_END_NO ) ){
		// �ׯ��
		kind = LK_KIND_FLAP;
	}
	else if( ( no >= INT_FLAP_START_NO ) && ( no <= INT_FLAP_END_NO ) ){
		// �ׯ�ߑ��u
		kind = LK_KIND_INT_FLAP;
	}
	else if( ( no >= LOCK_START_NO ) && ( no <= LOCK_END_NO ) ){
		// ۯ����u
		kind = LK_KIND_LOCK;
	}
	return( kind );
}

/*[]----------------------------------------------------------------------[]*/
/*| �Sۯ����u���쏈��                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lk_AllMove                                              |*/
/*|																		   |*/
/*| PARAMETER    :	lk_kind : ���u���                                     |*/
/*| 					LK_KIND_FLAP = �ׯ��	                           |*/
/*| 					LK_KIND_LOCK = ۯ����u	                           |*/
/*| 					LK_KIND_INT_FLAP = �����ׯ�ߑ��u                   |*/
/*|																		   |*/
/*| 				mov_req : �v������	                                   |*/
/*| 					�O = �ׯ�߉��~�^ۯ��J	                           |*/
/*| 					�P = �ׯ�ߏ㏸�^ۯ���	                           |*/
/*|																		   |*/
/*| 				req_kind : �v����	                                   |*/
/*| 					�O = ����ݽ����			                           |*/
/*| 					�P = NT-NET(���u����)	                           |*/
/*| RETURN VALUE : �Ȃ�	                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| �u�S�ׯ�ߏ㏸�^���~�v�A�u�Sۯ����u�J�^�v�v���ɑ΂��鏈�����s��       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Motohashi                                             |*/
/*| Date         : 2006-09-28                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static	void	Lk_AllMove( uchar lk_kind, uchar mov_req, uchar req_kind )
{
	ushort		s_no;			// ����Ώۑ��uNo.(start)
	ushort		e_no;			// ����Ώۑ��uNo.(end)
	uchar		mtype;			// �ׯ�߁^ۯ����
	ushort		lk_ofs;			// ���uNo.�̾��(���uNo.-1)
	ushort		lk_cnt = 0;		// ����Ώۑ��u��
	flp_com		*lk_data;		// ���Ԉʒu����߲���
	uchar		force_exec=0;	// �����o���׸�

	// ���u��ʂ𔻒f������ΏۂƂ��鑕�uNo.���
	if( lk_kind == LK_KIND_FLAP ){
		if( m_mode_chk_all( LK_KIND_FLAP ) == ON ){
			// ���ꂩ������ق��蓮Ӱ�ނ̏ꍇ����m�f�Ƃ���
			return;
		}
		s_no = FLAP_START_NO;
		e_no = FLAP_END_NO;
		mtype = _MTYPE_FLAP;
	}
	else if( lk_kind == LK_KIND_INT_FLAP ){
		if( m_mode_chk_all( LK_KIND_INT_FLAP ) == ON ){
			// ���ꂩ������ق��蓮Ӱ�ނ̏ꍇ����m�f�Ƃ���
			return;
		}
		s_no = INT_FLAP_START_NO;
		e_no = INT_FLAP_END_NO;
		mtype = _MTYPE_INT_FLAP;		
	}
	else{
		if( m_mode_chk_all( LK_KIND_LOCK ) == ON ){
			// ���ꂩ������ق��蓮Ӱ�ނ̏ꍇ����m�f�Ƃ���
			return;
		}
		s_no = LOCK_START_NO;
		e_no = LOCK_END_NO;
		mtype = _MTYPE_LOCK;
	}
	// �ΏۂƂ���S�Ă̑��u�ɑ΂��������s��
	for( lk_ofs = (s_no-1) ; lk_ofs < e_no ; lk_ofs++ ){
		WACDOG;										// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if (LockInfo[lk_ofs].lok_syu != 0 ) {		// �ڑ�����H

			lk_cnt++;								// ����Ώۑ��u���ā{1

			lk_data = &FLAPDT.flp_data[lk_ofs];		// ���Ԉʒu����߲������

			if( mov_req == 0 ){						// �v������H
				if(FLPCTL.flp_work.mode == FLAP_CTRL_MODE4)		// ���݃t���b�v���㏸�ς�
				{
					/* �����o�ɊJ�n���o�^ */
					IoLog_write(IOLOG_EVNT_FORCE_STA, (ushort)LockInfo[lk_ofs].posi, 0, 0);
					force_exec = 1;					// �����o���׸�ON
				}
				// �ׯ�߉��~�^ۯ��J
				lk_data->nstat.bits.b02 = 0;		// ���~�����׸�ON
				Flapdt_sub_clear(lk_ofs);			// ���~�ڍ״ر�ر
			}
			else{
				// �ׯ�ߏ㏸�^ۯ���
				lk_data->nstat.bits.b02 = 1;		// �㏸�����׸�ON
			}

			if( req_kind == 0 ){					// �v�����H
				// ����ݽ����
				lk_data->nstat.bits.b07 = 1;		// ����ݽ�׸�ON
				lk_data->nstat.bits.b09 = 0;		// NT-NET�׸�OFF
			}
			else{
				// NT-NET(���u����)
				lk_data->nstat.bits.b07 = 1;		// ����ݽ�׸�ON
				lk_data->nstat.bits.b09 = 1;		// NT-NET�׸�ON
				lk_data->nstat.bits.b15 = 1;		// ���u������ׯ��/ۯ����삠��
				if( req_kind == 2 ){
					lk_data->nstat.bits.b14 = 1;	// MAF����̗v��
				}else{
					lk_data->nstat.bits.b14 = 0;	// NT-NET����̗v��
				}
			}
		}
	}
	if( lk_cnt != 0 ){								// ����ΏۂƂ��鑕�u����H
		if( mov_req == 0 ){
			// �ׯ�߉��~�^ۯ��J
			LKopeApiLKCtrl_All( mtype, 6 );			// �S���u�i���~/�J�j�����ް����M�v��
			if( force_exec ){
				if( lk_kind == LK_KIND_LOCK ){
					ope_anm( AVM_FORCE_ROCK );			// ���~�ųݽ(���b�N)
				}else{
					ope_anm( AVM_FORCE_FLAP );			// �J�ųݽ(�t���b�v)
				}
			}
		}
		else{
			// �ׯ�ߏ㏸�^ۯ���
			LKopeApiLKCtrl_All( mtype, 5 );			// �S���u�i�㏸/�j�����ް����M�v��
		}
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| ��ۯ����u����v������                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Mnt_Lk_Move                                             |*/
/*|																		   |*/
/*| PARAMETER	�F	mov_req : �v������	                                   |*/
/*| 					ON  = �ׯ�ߏ㏸�^ۯ���	                           |*/
/*| 					OFF = �ׯ�߉��~�^ۯ��J	                           |*/
/*|																		   |*/
/*| RETURN VALUE : ���쌋��	                                               |*/
/*| 					OK = �v��������s		                           |*/
/*| 					NG = �v�����얳��		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| ����ݽ����ɂ��ʂ��ׯ�߁i�㏸�^���~�j�^ۯ����u�i�J�^�j�������s�� |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Motohashi                                             |*/
/*| Date         : 2006-10-04                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static	uchar	Mnt_Lk_Move( uchar mov_req )
{
	uchar	ret = NG;
	uchar	m_mode = OFF;


	m_mode = m_mode_chk_psl( FLPCTL.Room_no );					// �蓮Ӱ�ޏ�Ԏ擾
	
	if( mov_req == ON ){
		// �ׯ�ߏ㏸�iۯ��j�v��
		if( ( FLPCTL.Flp_mv_tm[ FLPCTL.Room_no - 1 ] == -1 )	// ���쒆�ȊO?
					&&											//   ����
			( m_mode == OFF )									// �蓮Ӱ�ޒ��łȂ�
		){

			if(FLPCTL.Ment_flg != 0) {
				if(FLPCTL.flp_work.nstat.bits.b01 == 1 &&		// �㏸�ς�
					FLPCTL.flp_work.nstat.bits.b04 == 0 &&		// �㏸���b�N�ł͂Ȃ�
					FLPCTL.flp_work.nstat.bits.b08 == 0 &&		// �ڑ�����
					FLPCTL.flp_work.nstat.bits.b12 == 0) {		// �㏸�ُ�ł͂Ȃ�
					FLPCTL.flp_work.nstat.bits.b09 = 1;
					// �v�����얳��
					NTNET_Snd_Data05_Sub();						// NT-NETۯ����䌋���ް��쐬
					return NG;
				}
			}
			FlpSet( FLPCTL.Room_no, 1 );						// �ׯ�ߏ㏸�iۯ��j�v��
			ret = OK;
		}
	}
	else{
		// �ׯ�߉��~�iۯ��J�j�v��
		if( ( FLPCTL.Flp_mv_tm[ FLPCTL.Room_no - 1 ] == -1 )	// ���쒆�ȊO?
					&&											//   ����
			( m_mode == OFF )									// �蓮Ӱ�ޒ��łȂ�
		){
			if(FLPCTL.Ment_flg != 0) {
				if(FLPCTL.flp_work.nstat.bits.b01 == 0 &&		// ���~�ς�
					FLPCTL.flp_work.nstat.bits.b05 == 0 &&		// ���~���b�N�ł͂Ȃ�
					FLPCTL.flp_work.nstat.bits.b08 == 0 &&		// �ڑ�����
					FLPCTL.flp_work.nstat.bits.b13 == 0) {		// ���~�ُ�ł͂Ȃ�
					FLPCTL.flp_work.nstat.bits.b09 = 1;			// NTNET�t���OON
					FLPCTL.Ment_flg = 0;						// ����ݽ�׸޸ر
					// �v�����얳��
					NTNET_Snd_Data05_Sub();						// NT-NETۯ����䌋���ް��쐬
					return NG;
				}
			}
			FlpSet( FLPCTL.Room_no, 2 );						// �ׯ�߉��~�iۯ��J�j�v��
			ret = OK;
		}
	}
	if( ret == OK ){
		// �v��������s
		if( FLPCTL.Ment_flg == 0 ){								// ����ݽ
			FLPCTL.flp_work.nstat.bits.b07 = 1;					// ����ݽ�׸�ON
			FLPCTL.flp_work.nstat.bits.b09 = 0;					// NT-NET�׸�OFF
		}else{													// NT-NET
			FLPCTL.flp_work.nstat.bits.b07 = 1;					// ����ݽ�׸�ON
			FLPCTL.flp_work.nstat.bits.b09 = 1;					// NT-NET�׸�ON
			FLPCTL.flp_work.nstat.bits.b15 = 1;					// ���u������ׯ��/ۯ����삠��
			if( FLPCTL.Ment_flg == 2 ){
				FLPCTL.flp_work.nstat.bits.b14 = 1;				// MAF����̗v��
			}else{
				FLPCTL.flp_work.nstat.bits.b14 = 0;				// NT-NET����̗v��
			}
		}
	}
	else{
		// �v�����얳��
		NTNET_Snd_Data05_Sub();									// NT-NETۯ����䌋���ް��쐬
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| �ׯ��/ۯ����u�蓮Ӱ�����������i�ʁj                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : m_mode_chk_psl                                          |*/
/*|																		   |*/
/*| PARAMETER	�F	no : ���uNo.							               |*/
/*|																		   |*/
/*| RETURN VALUE : ��������	                                               |*/
/*| 					OFF = �蓮Ӱ�ނłȂ�		                       |*/
/*| 					ON  = �蓮Ӱ�ޒ�		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| �w�肳�ꂽ�ׯ�ߑ��u�̎蓮Ӱ�ޏ�Ԃ��������߂�l�Ƃ��ďo�͂��� 		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Motohashi                                             |*/
/*| Date         : 2006-10-17                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
uchar	m_mode_chk_psl( ushort no )
{
	uchar	mode = OFF;
	uchar	tno;
	
	if( (no >= 1) && (no <= LOCK_MAX) ){			// ���uNo.����

		tno = LockInfo[no - 1].if_oya;				// �ڑ�����Ă��������No.�擾

		if( no <= INT_FLAP_END_NO ){
			// �ׯ�ߑ��u
			mode = flp_m_mode[tno-1];				// �蓮Ӱ�ޏ�Ԏ擾
		}
		else{
			// ۯ����u
			mode = lok_m_mode[tno-1];				// �蓮Ӱ�ޏ�Ԏ擾
		}
	}
	return( mode );
}

/*[]----------------------------------------------------------------------[]*/
/*| �ׯ��/ۯ����u�蓮Ӱ�����������i�S�āj                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : m_mode_chk_all                                          |*/
/*|																		   |*/
/*| PARAMETER	�F	kind : ���u���							               |*/
/*|						LK_KIND_FLAP���ׯ�ߑ��u							   |*/
/*|						LK_KIND_LOCK��ۯ����u							   |*/
/*| 					LK_KIND_INT_FLAP = �����ׯ�ߑ��u                   |*/
/*|																		   |*/
/*| RETURN VALUE : ��������	                                               |*/
/*| 					OFF = �蓮Ӱ�ނłȂ�		                       |*/
/*| 					ON  = �蓮Ӱ�ޒ�		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| �w�肳�ꂽ���u��ʂ̎蓮Ӱ�ޏ�Ԃ��������߂�l�Ƃ��ďo�͂��� 		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Motohashi                                             |*/
/*| Date         : 2006-10-17                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
uchar	m_mode_chk_all( uchar kind )
{
	uchar	mode = OFF;
	uchar	tno;
	uchar	type;
	
	if( kind == LK_KIND_FLAP ){
		// �ׯ�ߑ��u������
		for ( tno = 0 ; tno < FLAP_IF_MAX ; tno++ ){	// �S���������
			if( flp_m_mode[tno] == ON ){
				mode = ON;
				break;
			}
		}
	}
	else{
		// ۯ����u������
		for ( tno = 0 ; tno < LOCK_IF_MAX ; tno++ ){	// �S���������
			type = LKcom_Search_Ifno( (uchar)(tno+1) );
			if( lok_m_mode[tno] == ON && 
			  (( kind == LK_KIND_LOCK && !type ) ||
			   ( kind == LK_KIND_INT_FLAP && type ))){
				mode = ON;
				break;
			}
		}
	}
	return( mode );
}
/*[]----------------------------------------------------------------------------------------------[]*/
/*| �ڍג��~�ر�ر����																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: Flapdt_sub_clear( no )														   |*/
/*| PARAMETER	: no	; ���������p���Ԉʒu�ԍ�(1�`324)										   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: H.Sekiguchi																	   |*/
/*| Date		: 2006-10-24																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void Flapdt_sub_clear(ushort no){
	struct CAR_TIM wk_time1,wk_time2;
	uchar	i;
	uchar	buf_size;
	ulong	posi;

	posi = LockInfo[no].posi;				//�����ԍ���蒓�Ԉʒu���擾

	for(i = 0 ; i < 10 ; i++){
		if(FLAPDT_SUB[i].WPlace == posi){		//���Ԉʒu������
			/*���Ɏ���*/
			wk_time1.year = FLAPDT.flp_data[no].year ;
			wk_time1.mon = FLAPDT.flp_data[no].mont ;
			wk_time1.day = FLAPDT.flp_data[no].date ;
			wk_time1.hour = FLAPDT.flp_data[no].hour ;
			wk_time1.min = FLAPDT.flp_data[no].minu ;
			/*�ڍג��~�����ް�*/
			wk_time2.year = FLAPDT_SUB[i].TInTime.Year;
			wk_time2.mon = FLAPDT_SUB[i].TInTime.Mon;
			wk_time2.day = FLAPDT_SUB[i].TInTime.Day;
			wk_time2.hour = FLAPDT_SUB[i].TInTime.Hour;
			wk_time2.min = FLAPDT_SUB[i].TInTime.Min;
			
			if(0 == ec64(&wk_time1,&wk_time2)){		//���Ɏ���������
				if(i < 9){
					buf_size = (uchar)(9 - i);
					memset(&FLAPDT_SUB[i],0,sizeof(flp_com_sub));			// �ڍג��~�ر(���Z����)�ر
					memmove(&FLAPDT_SUB[i],&FLAPDT_SUB[i+1],sizeof(flp_com_sub)*buf_size);
				}
				memset(&FLAPDT_SUB[9],0,sizeof(flp_com_sub));							// �I�[�ڍג��~�ر�ر
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------------------------------[]*/
/*| �U�֐��Z�J�n����																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: ope_Furikae_start(void)														   |*/
/*| PARAMETER	: ushort; �U�֌��Ԏ�No															   |*/
/*| RETURN VALUE: char	; 1:���� 0:���s															   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: 																				   |*/
/*| Date		: 2007-04-04																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
char	ope_Furikae_start(ushort no)
{
	if( FLAPDT.flp_data[no - 1].lag_to_in.BIT.SYUUS == 1 ){
		return 0;
	}
	if ((FLAPDT.flp_data[no - 1].mode == FLAP_CTRL_MODE5 ||
		 FLAPDT.flp_data[no - 1].mode == FLAP_CTRL_MODE6) &&
		FLAPDT.flp_data[no - 1].timer != -1) {
		// �U�֐��Z�J�nOK
		FLAPDT.flp_data[no - 1].lag_to_in.BIT.FURIK = 1;
		memcpy(&flp_com_frs, &FLAPDT.flp_data[no - 1], sizeof(flp_com));

		return 1;
	}
	return 0;
}

/*[]----------------------------------------------------------------------------------------------[]*/
/*| �U�֐��Z�I������																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: ope_Furikae_stop																   |*/
/*| PARAMETER	: ushort; �U�֌��Ԏ�No / char	; 0:���~ 1:����									   |*/
/*| RETURN VALUE: char	; 1:���� 0:���s															   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: 																				   |*/
/*| Date		: 2007-04-04																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
char	ope_Furikae_stop(ushort no, char bOK)
{

	if (((FLAPDT.flp_data[no - 1].mode == FLAP_CTRL_MODE5 || (FLAPDT.flp_data[no - 1].mode == FLAP_CTRL_MODE6)) &&
		(FLAPDT.flp_data[no - 1].lag_to_in.BIT.FURIK))) {
		// ���[�h��4�܂���5�A�����O�^�C�}��~��(�U�֒�)�Ȃ�U�֐���
		FLAPDT.flp_data[no - 1].lag_to_in.BIT.FUKUG = 0;
		FLAPDT.flp_data[no - 1].lag_to_in.BIT.FURIK = 0;
		FLAPDT.flp_data[no - 1].lag_to_in.BIT.SYUUS = 0;

		if (bOK) {
			// �U�֊���
			FLAPDT.flp_data[no - 1].lag_to_in.BIT.FUKUG = 1;
			FLAPDT.flp_data[no - 1].timer = 0;				// ���O�^�C�}�[�^�C���A�E�g����
		}
	} else {
		if (bOK) {
			// �ԂȂ��A�������͕ʂ̎Ԃ�����ꍇ�͐U�֎��s -> �ē���
			flp_com_frs.lag_to_in.BIT.FURIK = 0;
			flp_com_frs.lag_to_in.BIT.SYUUS = 0;
			flp_com_frs.lag_to_in.BIT.FUKUG = 1;
			if(( prm_get( COM_PRM,S_TYP,73,1,1 ) == 1 )&&		// �p�c�޲��s���o�ɂȂ�
			   ( LockInfo[no-1].lok_syu == LK_TYPE_AIDA2 )){	// �Ԏ��ݒ肪�p�c�޲�
				// �p�c�޲����͕s���o�ɂȂ��Ƃ���
			}
			else {
// MH810100(S) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X (RT���Z�f�[�^�Ή�)
//				Make_Log_Enter_frs(no, (void*)&flp_com_frs);	// �����Ƀf�[�^
//				Log_regist( LOG_ENTER );						// ���ɗ���o�^
// MH810100(E) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X (RT���Z�f�[�^�Ή�)
				ope_RegistFuseiData(no, &flp_com_frs, 1);		// �s���o��
			}
		}
	}
	return 1;
}


/*[]----------------------------------------------------------------------------------------------[]*/
/*| ���d�� �U�֐��Z����																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: ope_Furikae_fukuden(void)														   |*/
/*| PARAMETER	: void	;																		   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: 																				   |*/
/*| Date		: 2007-04-04																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	ope_Furikae_fukuden(void)
{
	int	i;

// �U�֐��Z�r���̂��̂��N���A����
	for (i = 0; i < LOCK_MAX; i++) {
		WACDOG;																// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if (FLAPDT.flp_data[i].lag_to_in.BIT.FURIK) {
			FLAPDT.flp_data[i].lag_to_in.BIT.FUKUG = 0;
			FLAPDT.flp_data[i].lag_to_in.BIT.FURIK = 0;
			FLAPDT.flp_data[i].lag_to_in.BIT.SYUUS = 0;
		}
	}
}


/*[]----------------------------------------------------------------------------------------------[]*/
/*| �s���o�ɓo�^����																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: ope_RegistFuseiData(void)														   |*/
/*| PARAMETER	: flp_com	�o�^�Ԏ��f�[�^														   |*/
/*|          	: ushort	�o�^�Ԏ��ԍ�														   |*/
/*|          	: uchar		�U�֐��Z�ɂ��s���o�ɓo�^�Ȃ�1										   |*/
/*| RETURN VALUE: char	; 0�F�s���o��															   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: 																				   |*/
/*| Date		: 2007-04-17																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
char	ope_RegistFuseiData(ushort no, flp_com *flp, char BFrs)
{
	if( fusei.kensuu >= LOCK_MAX ){
		fusei.kensuu = LOCK_MAX - 1;
	}
	fusei.fus_d[fusei.kensuu].iyear = flp->year;	// In Time
	fusei.fus_d[fusei.kensuu].imont = flp->mont;
	fusei.fus_d[fusei.kensuu].idate = flp->date;
	fusei.fus_d[fusei.kensuu].ihour = flp->hour;
	fusei.fus_d[fusei.kensuu].iminu = flp->minu;

	memcpy( &fusei.fus_d[fusei.kensuu].oyear, &CLK_REC.year, 6 );	// Out Time
	fusei.fus_d[fusei.kensuu].t_iti = no;

	if((( flp->nstat.bits.b01 == 0 )&&			// ���~�ς�(ۯ��J�ς�)
	    ( flp->nstat.bits.b07 == 1 ))||			// ����ݽ�׸�ON
	   ( flp->nstat.bits.b03 == 1 )){			// �����o��
		// �����o��
		fusei.fus_d[fusei.kensuu].kyousei = 1;	// �����׸�ON
		if( flp->nstat.bits.b15 ){				// // ���u������ׯ��/ۯ����삠��H
			fusei.fus_d[fusei.kensuu].kyousei = 11;
			flp->nstat.bits.b15 = 0;
		}
		flp_flag_clr( FLAP_CTRL_MODE1, CAR_SENSOR_OFF );
		flp->nstat.bits.b06 = 0;				// �s���ر
// MH322916(S) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�
		// �������Ԃ̏ꍇ�A�����o�ɂɂ��A���[�������͉��~�w�����󂯂����ł͂Ȃ�
		// �����o�ɂ������������_�ōs��Ȃ��Ǝ��̎��v���i�Ō댟�m���邽�߂����ŉ���
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
//		LongTermParkingRel( no );// �������ԉ���(�����o��)
		LongTermParkingRel( no, LONGPARK_LOG_KYOUSEI_FUSEI, flp);// �������ԉ���(�����o��)
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
// MH322916(E) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�
	}
	else{
		// �s���o��
		flp->mode = FLAP_CTRL_MODE5;
		fusei.fus_d[fusei.kensuu].kyousei = 0;	// �����׸�OFF
		IoLog_write(IOLOG_EVNT_OUT_ILLEGAL_START, (ushort)LockInfo[FLPCTL.Room_no - 1].posi, 0, 0);
		if (!BFrs) {
			FlpSet( no, 2 );						// Flaper DOWN
		}
		flp->nstat.bits.b06 = 1;				// �s��
// MH322916(S) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
//		LongTermParkingRel( no );// �������ԉ���(�s���o��)
		LongTermParkingRel( no, LONGPARK_LOG_KYOUSEI_FUSEI, flp);// �������ԉ���(�s���o��)
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
// MH322916(E) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�
	}
	fusei.kensuu ++;							// �s���E��������+1

	return 0;
}

/*[]----------------------------------------------------------------------------------------------[]*/
/*| �t���b�v�㏸�A���b�N����ϰ���o�ɓo�^����													   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: ope_LockTimeOutData(void)														   |*/
/*| PARAMETER	: flp_com	�o�^�Ԏ��f�[�^														   |*/
/*|          	: ushort	�o�^�Ԏ��ԍ�														   |*/
/*| RETURN VALUE: char	; 0�F�t���b�v�㏸�A���b�N�o��											   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: 																				   |*/
/*| Date		: 2007-04-17																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
char	ope_LockTimeOutData(ushort no, flp_com *flp)
{
	if( locktimeout.kensuu >= LOCK_MAX ){
		locktimeout.kensuu = LOCK_MAX - 1;
	}
	locktimeout.fus_d[locktimeout.kensuu].iyear = flp->year;	// In Time
	locktimeout.fus_d[locktimeout.kensuu].imont = flp->mont;
	locktimeout.fus_d[locktimeout.kensuu].idate = flp->date;
	locktimeout.fus_d[locktimeout.kensuu].ihour = flp->hour;
	locktimeout.fus_d[locktimeout.kensuu].iminu = flp->minu;

	memcpy( &locktimeout.fus_d[locktimeout.kensuu].oyear, &CLK_REC.year, 6 );	// Out Time
	locktimeout.fus_d[locktimeout.kensuu].t_iti = no;

	locktimeout.kensuu ++;							// �t���b�v�㏸�A���b�N����ϓ��o�Ɍ���+1

	return 0;
}


/*[]----------------------------------------------------------------------------------------------[]*/
/*| �V�C�����Z�p�֐��i�C�����Z�J�n�j                                                               |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: ope_SyuseiStart(void)	                                                           |*/
/*| PARAMETER	: flp_com	�o�^�Ԏ��f�[�^														   |*/
/*|          	: ushort	�o�^�Ԏ��ԍ�														   |*/
/*| RETURN VALUE: char	; 0�F�t���b�v�㏸�A���b�N�o��											   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: 																				   |*/
/*| Date		: 2007-04-17																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	ope_SyuseiStart(ushort no)
{
	FLAPDT.flp_data[OPECTL.Pr_LokNo - 1].lag_to_in.BIT.SYUUS = 1;	
	memcpy(&flp_com_frs, &FLAPDT.flp_data[no - 1], sizeof(flp_com));
}

/*[]----------------------------------------------------------------------[]*/
/*| �V�C�����Z(�s���o�ɓo�^)                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SyuseiFuseiSet(void)                                    |*/
/*| PARAMETER    : no : �t���b�v��                                         |*/
/*|              : type : 1=�C�����Ԏ���׸���ѱ��ߌ�̏ꍇ                 |*/
/*|              :          �EFLAPDT�̓��Ɏ����`���ݎ���                   |*/
/*|              :          �E���ԗ����͋���0�~                            |*/
/*|              :        0=���̑��̏C���ł̕s���o��                       |*/
/*|              :          �E�C���ޯ̧�̓��Ɏ����`�o�Ɏ���                |*/
/*|              :          �E�C���ޯ̧�ł̒��ԗ���                        |*/
/*| RETURN VALUE :                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Akiba                                                 |*/
/*| Date         : 2008-03-07                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	SyuseiFuseiSet( ushort no, uchar type )
{

	if( fusei.kensuu >= LOCK_MAX ){
		fusei.kensuu = LOCK_MAX - 1;
	}

	if( type == 1 ){
		memcpy( &fusei.fus_d[fusei.kensuu].iyear,&FLAPDT.flp_data[no-1].year,6 );	// In Time
		memcpy( &fusei.fus_d[fusei.kensuu].oyear,&CLK_REC.year,6 );					// Out Time
	}else{ 
		memcpy( &fusei.fus_d[fusei.kensuu].iyear,&syusei[no-1].iyear,6 );			// In Time
		memcpy( &fusei.fus_d[fusei.kensuu].oyear,&syusei[no-1].oyear,6 );			// Out Time
	}

	fusei.fus_d[fusei.kensuu].t_iti = no;
	fusei.fus_d[fusei.kensuu].kyousei = (uchar)(type+2);	// �����׸�OFF(�J�E���g�Ȃ��j

	fusei.kensuu ++;							// �s���E��������+1

	return;
}

/*[]----------------------------------------------------------------------------------------------[]*/
/*| ���~�w�����M�\�`�F�b�N�i�������񃊃g���C�����p�j                                             |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: �Ԏ����f�[�^�i�ړI�Ԏ��̍ŐV�f�[�^�Ƃ��� flp_work ���Q�Ƃ���				   |*/
/*| RETURN VALUE: uchar	; 1=���~���b�N���g���C�������s�\�i���~�w�����M�\�j					   |*/
/*|						  0=���~�w�����M�s��													   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: okuda																			   |*/
/*| Date		: 2008/10/16																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
uchar	flp_DownLock_DownSendEnableCheck( void )
{
	flp_com	*p_flp_com = &FLPCTL.flp_work;
	uchar ret = 0;

	if( (p_flp_com->nstat.bits.b02 == 0) &&						  // ���~����i���~/�㏸�w���̂����Ō�ɑ��M�������́j
		(p_flp_com->nstat.bits.b05 == 1) )						  // ���~���b�N�G���[������
	{
		ret = 1;
	}
	return	ret;
}

uchar	flp_DownLock_DownSendEnableCheck2( ushort no )
{
	flp_com	*p_flp_com = &FLAPDT.flp_data[no];
	uchar ret = 0;

	if( (p_flp_com->nstat.bits.b02 == 0) &&						  // ���~����i���~/�㏸�w���̂����Ō�ɑ��M�������́j
		(p_flp_com->nstat.bits.b05 == 1) )						  // ���~���b�N�G���[������
	{
		ret = 1;
	}
	return	ret;
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�G���[�o�^����																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: DownLockErr()																				   |*/
/*| PARAMETER	: no	: �Ԏ�No�i0�`�j																		   |*/
/*| 			: ErrNo	: �G���[No (���W���[���R�[�h11�̉�2��)												   |*/
/*| 			: knd	: 1=�����C0=����																	   |*/
/*| RETURN VALUE: void																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: okuda																						   |*/
/*| Date		: 2008/10/16																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2008 AMANO Corp.------[]*/
void	flp_DownLock_ErrSet( ushort no, char ErrNo, char knd )
{
	uchar	edit=0;														// �G���[��ԕω��F1=����
	ulong	err_sub;
	t_flp_DownLockErrInfo* pInfo = &flp_DownLockErrInfo[no];
	
	switch( ErrNo ){
	case ERR_FLAPLOCK_DOWNRETRY:										// E1641:���g���C����J�n�G���[
		if( pInfo->f_info.BIT.RETRY_START_ERR != knd ){					// �ω�����
		    pInfo->f_info.BIT.RETRY_START_ERR = knd;					// �V���save
			edit = 1;
		}
		break;

	case ERR_FLAPLOCK_DOWNRETRYOVER:									// E1640:���g���C�I�[�o�[�G���[
		if( pInfo->f_info.BIT.RETRY_OVER_ERR != knd ){					// �ω�����
		    pInfo->f_info.BIT.RETRY_OVER_ERR = knd;						// �V���save
			edit = 1;
		}
		break;

	default:
		break;
	}

	if( edit == 1 ){													// �ω�����
		err_sub = flp_ErrBinDataEdit( no );								// �G���[�o�^�p�Ԏ��ԍ��쐬
		if( no < INT_CAR_START_INDEX )
			err_chk2( ERRMDL_IFFLAP, ErrNo, knd, 2, 1, &err_sub );		// IF�Ց��u�փG���[�o�^
		else
			err_chk2( ERRMDL_FLAP_CRB, ErrNo, knd, 2, 1, &err_sub );		// CRR����u�փG���[�o�^
	}
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�G���[���̕ҏW																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: no	: �Ԏ�No�i0�`�j																		   |*/
/*| RETURN VALUE: �G���[�o�^�p�̋����																	   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: okuda																						   |*/
/*| Date		: 2008-10-06																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2008 AMANO Corp.------[]*/
ulong	flp_ErrBinDataEdit( ushort no )
{
	ulong	ul;

	ul = (ulong)LockInfo[no].area * 10000;
	ul += (ulong)LockInfo[no].posi;
	return (ul);
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	���~���b�N�^�C�}�[�G���A�̏�����																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: void																						   |*/
/*| RETURN VALUE: void																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Namioka																					   |*/
/*| Date		: 2008-10-06																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2008 AMANO Corp.------[]*/
void	flp_DownLock_Initial( void )
{
	memset( flp_DownLockErrInfo, 0, sizeof( flp_DownLockErrInfo ));
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	���g���C�p�����^�C�}�[�J�n																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| �ŏ��ł����񃊃g���C���ł�Call�����																	   |*/
/*|	���g���C�@�\�J�n���f�������ōs��																		   |*/
/*|	flp_DownLock_DownSendEnableCheck() or flp_DownLock_DownSendEnableCheck2()�̎��s�\�`�F�b�N���{���Call	   |*/
/*|	���邱��																								   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: no	  : �Ԏ�No�i0�`323�j																   |*/
/*| RETURN VALUE: void																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: okuda																						   |*/
/*| Date		: 2008/10/16																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2008 AMANO Corp.------[]*/
void	flp_DownLock_RetryTimerStart( ushort no )
{
	ushort	prm1 = (ushort)prm_get(COM_PRM, S_TYP, 74, 2, 3);		// n���^�C�}�[�lget
	ushort	prm2 = (ushort)prm_get(COM_PRM, S_TYP, 74, 2, 1);		// m�񃊃g���C��get

	/* n��m�񃊃g���C�^�C�}�[�N���J�n�\�`�F�b�N */
	if( (prm1 && prm2) &&											// n��m�񃊃g���C�������s�ݒ肠��
		( Aida_Flap_Chk( no )) )				// ���u��ʂ��p�c���t���b�v
		/*(flp_DownLock_DownSendEnableCheck()) )*/					// ���~�w�����M�\(�����ł͏o���Ȃ�)
	{
		flp_DownLockErrInfo[no].TimerCount = (prm1 * 60 * 2);		// �^�C�}�l(x500ms)set
		flp_DownLockErrInfo[no].f_info.BIT.TO_MSG = 0;				// �^�C���A�E�g���b�Z�[�W�����M

		if( flp_DownLockErrInfo[no].f_info.BIT.EXEC == 0 ){			// �ŏ��̋N���i���܂Ń��g���C������s���Ŗ����j
			flp_DownLockErrInfo[no].RetryCount = 0;					// ���g���C�񐔃N���A
			flp_DownLock_ErrSet( no, ERR_FLAPLOCK_DOWNRETRY, 1 );	// ���g���C�J�n�G���[����
			flp_DownLockErrInfo[no].f_info.BIT.EXEC = 1;			// �^�C�}�[�X�^�[�g
		}
	}
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	���g���C�����~																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: no	: �Ԏ�No�i0�`323�j																	   |*/
/*| RETURN VALUE: void																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: okuda																						   |*/
/*| Date		: 2008/10/16																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2008 AMANO Corp.------[]*/
void	flp_DownLock_RetryStop( ushort no )
{
	if( flp_DownLockErrInfo[no].f_info.BIT.EXEC == 1 ){				// ���g���C���쒆
		flp_DownLockErrInfo[no].f_info.BIT.EXEC = 0;				// �����~�i�^�C�}�[���X�g�b�v�j
		flp_DownLock_ErrSet( no, ERR_FLAPLOCK_DOWNRETRY, 0 );		// ���g���C�J�n�G���[����
	}
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	���g���C�� ���~�w�����M����																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	���M�\�󋵂����m�F���A�\�ł���Ή��~�w���𑗐M�������^�C�}�[���ċN������							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: no	: �Ԏ�No�i0�`323�j																	   |*/
/*| RETURN VALUE: 1 = �t���b�v���~�w�����{�A0=�����{														   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: okuda																						   |*/
/*| Date		: 2008/10/16																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2008 AMANO Corp.------[]*/
uchar	flp_DownLock_DownSend( ushort no )
{
	ushort	prm2 = (ushort)prm_get(COM_PRM, S_TYP, 74, 2, 1);		// m�񃊃g���C��get
	uchar	ret = 0;

	if( flp_DownLockErrInfo[no].f_info.BIT.EXEC == 1 ){				// ���g���C���쒆�ł���
		if( flp_DownLock_DownSendEnableCheck() ){					// ���~�w�����M�\
			if( flp_DownLockErrInfo[no].RetryCount < prm2 ){		// ���g���C�I�[�o�[�łȂ�
				++flp_DownLockErrInfo[no].RetryCount;

				/* ���g���C����ɂ�鉺�~�w�� */
				flp_f_DownLock_RetryDownCtrl = 1;					// ���g���C����ɂ�鉺�~�w��(ON)
				FlpSet( (ushort)(no+1), 2 );						// Flaper DOWN
																	// ���̊ԂɁ@flp_DownLock_FlpSet() ��Call�����
				flp_f_DownLock_RetryDownCtrl = 0;					// ���g���C����ɂ�鉺�~�w��(OFF)
				ret = 1;
			}
			else{													// ���g���C�I�[�o�[
				flp_DownLock_ErrSet( no, ERR_FLAPLOCK_DOWNRETRYOVER, 1 );	// ���g���C�I�[�o�[�G���[����
				flp_DownLock_RetryStop( no );						// ���g���C����I��
			}
		}
	}
	return	ret;
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�㏸/���~�t���b�v�w�����M���Call����鏈��	�i FlpSet()����Call����� �j								   |*/
/*| �i�t���b�v���u�̏ꍇ�̂�Call�����j																	   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: no	: �Ԏ�No�i0�`323�j																	   |*/
/*| RETURN VALUE: void																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: okuda																						   |*/
/*| Date		: 2008/10/16																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2008 AMANO Corp.------[]*/
void	flp_DownLock_FlpSet( ushort no )
{
	/* ���~�w�����M�� */
	if( FLPCTL.flp_work.nstat.bits.b02 == 0 ){						// ���~����w����
		if( FLPCTL.flp_work.nstat.bits.b05 == 1 ){					// ���~���b�N�G���[������
			// n��m�񃊃g���C�^�C���A�E�g�ɂ�鉺�~�w���ł͂Ȃ��ꍇ��
			// ��������V����n��m�񃊃g���C���J�n����B
			// �i���g���C��=0��A�^�C�}�[�ċN���j
			if( flp_f_DownLock_RetryDownCtrl == 0 ){				// n��m�񃊃g���C�^�C���A�E�g�ɂ�鉺�~�w���ł͂Ȃ�
				flp_DownLockErrInfo[no].RetryCount = 0;				// ���g���C�񐔃N���A
			}
			flp_DownLock_RetryTimerStart( no );						// n��m�񃊃g���C�^�C�}�[�J�n�i���̓��X�^�[�g�j
		}
	}
	/* �㏸�w�����M�� */
	else{
		flp_DownLock_RetryStop( no );								// ���g���C����I��
	}
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	���~���b�N�G���[��������Call����鏈��	�i lk_err_chk()����Call����� �j								   |*/
/*| �i�t���b�v���u�Ŋ����~���b�N�G���[�o�^�̏ꍇ�̂�Call�����j											   |*/
/*|	���ΏێԎ��̏��� FLPCTL.flp_work �ł͖����̂Œ��ӁB													   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: no	: �Ԏ�No�i0�`323�j																	   |*/
/*|               ErrNo	: �װ�ԍ��i��2���j																	   |*/
/*|               knd	: 0:���� 1:����																		   |*/
/*| RETURN VALUE: void																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: okuda																						   |*/
/*| Date		: 2008/10/16																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2008 AMANO Corp.------[]*/
void	flp_DownLock_lk_err_chk( ushort no, char ErrNo, char kind )
{
	if( ErrNo == ERR_LOCKOPENFAIL ){								// ���~���b�N�i�̎�����Call����Ȃ����j
		if( kind == 1 ){											// ����
			if( flp_DownLock_DownSendEnableCheck2( no ) ){			// n��m�񃊃g���C���s�\�`�F�b�N
				flp_DownLock_RetryTimerStart( no );					// n��m�񃊃g���C�^�C�}�[�J�n
			}
		}
		else{														// ����
			flp_DownLock_RetryStop( no );							// ���g���C����I��
			flp_DownLock_ErrSet( no, ERR_FLAPLOCK_DOWNRETRYOVER, 0 );	// ���g���C�I�[�o�[�G���[����
		}
	}
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�ԗ����Ɏ��E���O�^�C���A�b�v���ȂǂŃN���A�����G���A���܂Ƃ߂Ė{�֐��ŃN���A��������					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: mode	�J�ڐ�i�ύX��j��mode																   |*/
/*| 			: type	1:CarSenserON	2:LagTimeOver 3:CarSensorOFF										   |*/
/*| RETURN VALUE: void																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: namioka																					   |*/
/*| Date		: 2009/08/26																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2009 AMANO Corp.------[]*/
void	flp_flag_clr( uchar	mode, uchar	type )
{
	
	flp_com *p = &FLPCTL.flp_work;
	
	if(( type & (CAR_SENSOR_ON | LAG_TIME_OVER)) ){
		p->mode = mode;									// ��ԊǗ��G���A�̍X�V
		p->nstat.bits.b06 = 0;							// �s���ر
		p->nstat.bits.b07 = 0;							// ����ݽ�׸�OFF
		p->nstat.bits.b09 = 0;							// NT-NET�׸�OFF
		p->uketuke = 0;									// ��t�����s�ςݸر
		p->passwd = 0;									// �߽ܰ�޸ر
		p->bk_syu = 0;									// ���(���~,�C���p)�ر
		p->lag_to_in.BYTE = OFF;						// ׸���ϰ��ѱ�Ăɂ��ē����׸�OFF
		p->issue_cnt = 0;								// ���ԏؖ������s�񐔏�����
		p->nstat.bits.b14 = 0;							// ���u����̑���w����ʃN���A
		p->nstat.bits.b15 = 0;							// ���u����̃t���b�v����v���N���A
	}
	switch( type ){
		case	CAR_SENSOR_ON:							// CarSensorON�̏ꍇ
			memcpy( &p->year, &CLK_REC, 6 );			// ���ݎ������Z�b�g
			p->timer = LockTimer();						// �ׯ�ߏ㏸(ۯ����u��)��ϰ����
			// ���ɶ��Đݒ肪ۯ��A�ׯ�ߏ㏸���̏ꍇ�͓��ɔ�����ϰ���N�����Ȃ�
			if( Carkind_Param(TYUU_DAISUU_MODE,(char)LockInfo[FLPCTL.Room_no - 1].ryo_syu,1,4) != 2){
				p->in_chk_cnt = InChkTimer();			// ���ɔ�����ϰ�׸�ON
				// ���ɔ��莞�ԃ`�F�b�N
				if( p->timer <= p->in_chk_cnt ){		// ���b�N���Ԃ����ɔ��莞�Ԃ�菬������
					p->in_chk_cnt = (short)p->timer;	// ���b�N���u�i�ׯ�ߏ㏸�j���Ԃɍ��킹��B
				}
			}else{
				p->in_chk_cnt = -1;						// ��~��Ԃ��Z�b�g
				/* ���ɏ��o�^ */
				IoLog_write(IOLOG_EVNT_ENT_GARAGE, (ushort)LockInfo[FLPCTL.Room_no - 1].posi, 0, 0);
			}
			break;
		case	CAR_SENSOR_OFF:
			p->mode = mode;								// ��ԁF���~�ς݁i�ԗ��Ȃ��j��
			p->timer = -1;								// �ׯ�ߏ㏸(ۯ����u��)��ϰ�į��
			p->in_chk_cnt = -1;							// ���ɔ�����ϰ�į��
			break;
		case	LAG_TIME_OVER:							// LagTimeUP�̏ꍇ
			p->lag_to_in.BYTE = ON;
			FlpSet( FLPCTL.Room_no, 1 );				// Flaper UP
			break;
		default:
			break;
	}
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	���o�ɶ��ď���																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: mode	����̪���																			   |*/
/*| 			: evt	�e��������																			   |*/
/*| RETURN VALUE: ret	0:�����ް����M���Ȃ� 1:�����ް����M����												   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: namioka																					   |*/
/*| Date		: 2009/09/11																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2009 AMANO Corp.------[]*/
uchar	InOutCountFunc( uchar mode, ushort	evt )
{
	uchar ret = 0;
	uchar inparam;
	uchar outparam;
	uchar uc_prm;
	flp_com *p = &FLPCTL.flp_work;
	
	outparam = (uchar)Carkind_Param(TYUU_DAISUU_MODE,(char)LockInfo[FLPCTL.Room_no - 1].ryo_syu,1,3);			// �o�ɃJ�E���g
	inparam = (uchar)Carkind_Param(TYUU_DAISUU_MODE,(char)LockInfo[FLPCTL.Room_no - 1].ryo_syu,1,4);			// ���ɃJ�E���g
	uc_prm = (uchar)prm_get( COM_PRM,S_SHA,(short)(2+((LockInfo[FLPCTL.Room_no - 1].ryo_syu-1)*6)),1,1 );	// ��ʖ����Ă���ݒ�get

	switch( mode ){
		case FLAP_CTRL_MODE1:	// ���~�ς�(�ԗ��Ȃ�)
			break;
		case FLAP_CTRL_MODE2:	// ���~�ς�(�ԗ�����FT���Ē�)
			switch( evt ){
				case	2:		// Car sensor OFF
					if( inparam != 2 ){
						if(p->in_chk_cnt == -1){						// ���ɔ�����ϰ��ѱ���
							//�o�ɶ���
							Lk_OutCount( uc_prm, 0 );					// �o�ɑ䐔���ď���
							//�t���b�v�㏸�A���b�N����ϓ��o�ɏ��o�^
							if( inparam == 1 ){							// ���ɶ������ݸ�(ۯ��E�ׯ�ߏ㏸��ɑ��M����)
								ret = 1;
							}
							if( inparam != 2 ){							// ���ɶ������ݸ�(ۯ��E�ׯ�ߏ㏸��ɶ��ĈȊO)
								ope_LockTimeOutData( FLPCTL.Room_no, p );
							}
						}
					}
					flp_flag_clr( FLAP_CTRL_MODE1, CAR_SENSOR_OFF );	// �׸޸ر
					break;
				case	21:		// Payment complete
					if( outparam == 1 ){								// �o�ɶ������ݸ�(�o�Ɏ��ɶ���)
						if( inparam == 2 ){								// ���ɶ������ݸ�(ۯ��E�ׯ�ߏ㏸��ɶ���)
							Lk_InCount( uc_prm );
							ret = 1;
						}else{
							if(p->in_chk_cnt > 0){						// ���ɔ���^�C�}�[�ғ���
								Lk_InCount( uc_prm );
								ret = 1;
							}else{
								if( inparam == 1 ){						// ���ɶ������ݸ�(ۯ��E�ׯ�ߏ㏸��ɶ��Ă���)
									ret = 1;
								}
							}
						}
					}else{												// �o�ɶ������ݸ�(���Z���ɶ���)
						if(p->in_chk_cnt > 0){							// ���ɔ�����ϰ�ғ���
							Lk_InCount( uc_prm );						// ���ɑ䐔���ď���
							ret = 1;
						}
						else{
							if( inparam == 2 ){							// ���ɶ������ݸ�(ۯ��E�ׯ�ߏ㏸��ɶ��Ă���)
								Lk_InCount( uc_prm );					// ���ɑ䐔���ď���
								ret = 1;
							}
							else if( inparam == 1 ){					// ���ɶ������ݸ�(���ɔ��莞�Ԍo�ߌ�ɶ���)
								ret = 1;
							}
						}
						Lk_OutCount( uc_prm, 0 );						// �o�ɑ䐔���ď���
					}
					break;
				case	43:		// �ׯ�ߏ㏸(ۯ����u��)��ϰ
					if( inparam == 1 ){									// ���ɶ������ݸ�(���ɔ��莞�Ԍo�ߌ�ɶ���)
						ret = 1;										// �����ް����M
					}
					if( inparam == 2 ){									// ���ɶ������ݸ�(ۯ��E�ׯ�ߏ㏸��ɶ��Ă���)
						Lk_InCount( uc_prm );							// ���ɑ䐔���ď���
						ret = 1;										// �����ް����M
					}
					break;
				case	44:		// ���ɔ�����ϰ
					if( inparam == 0 ){									// ���ɶ������ݸ�(���ɔ��莞�Ԍo�ߌ�ɶ���)
						ret = 1;
					}
					if( inparam != 2 ){									// ���ɶ������ݸ�(ۯ��E�ׯ�ߏ㏸��ɶ��ĈȊO)
						Lk_InCount( uc_prm );							// ���ɑ䐔���ď���
					}
					break;
			}
			break;
		case FLAP_CTRL_MODE3:				// �㏸���쒆
			switch( evt ){
				case	2:		// Car sensor OFF
					Lk_OutCount( uc_prm, 0 );							// �o�ɑ䐔���ď���
					break;
				case	50:		// ��ԕω�
					if( inparam == 1 ){									// ���ɶ������ݸ�(���ɔ��莞�Ԍo�ߌ�ɶ���)
						ret = 1;
					}
					if( inparam == 2 ){									// ���ɶ������ݸ�(ۯ��E�ׯ�ߏ㏸��ɶ��ĈȊO)
						Lk_InCount( uc_prm );							// ���ɑ䐔���ď���
						ret = 1;
					}
					break;
				case	21:		// Payment complete
					if( outparam == 0 ){								// �o�ɶ������ݸ�(���Z�������ɶ���)
						Lk_OutCount( uc_prm, 0 );						// �o�ɑ䐔���ď���
					}
					break;
			}
			break;
		case FLAP_CTRL_MODE4:				// �㏸�ς�(���Ԓ�)
			switch( evt ){
				case	2:		// Car sensor OFF
					Lk_OutCount( uc_prm, 0 );							// �o�ɑ䐔���ď���
					break;
				case	21:		// Payment complete
					if( outparam == 0 ){								// �o�ɶ������ݸ�(���Z�������ɶ���)
						Lk_OutCount( uc_prm, 0 );						// �o�ɑ䐔���ď���
					}
					break;
			}
			break;
		case FLAP_CTRL_MODE5:				// ���~���쒆
		case FLAP_CTRL_MODE6:				// ���~�ς�(�ԗ�����LT���Ē�)
			switch( evt ){
				case	2:		// Car sensor OFF
					if( outparam == 1 ){								// �o�ɶ������ݸ�(�o�Ɏ��ɶ���)
						Lk_OutCount( uc_prm, 0 );						// �o�ɑ䐔���ď���
					}
					break;
				case	23:		// �ׯ�ߏ㏸�w��(�C�����Z�p)
					if( outparam == 0 ){								// �o�ɶ������ݸ�(���Z�������ɶ���)
						Lk_OutCount( uc_prm, 0 );						// �o�ɑ䐔���ď���
					}
					ret = 1;
					break;
				case 42:		// Lag time timer over
					if( outparam == 1 ){								// �o�Ɏ��ɶ���(�o�ɂ��Ă��Ȃ����߂����Ŷ��Ă���)
						Lk_OutCount( uc_prm, 1 );						// �o�ɑ䐔���ď���
					}
					if( inparam != 2 ){									// ���ɶ������ݸ�(ۯ��E�ׯ�ߏ㏸��ɶ��ĈȊO)
						Lk_InCount( uc_prm );							// ���ɑ䐔���ď���
						if( inparam == 0 ){								// ���ɶ������ݸ�(���ɔ��莞�Ԍo�ߌ�ɶ���)
							ret = 1;
						}
					}
					break;
			}
			break;
		default:			// ���̑���̪��ށi���肦�Ȃ��j
			break;
	}
	return ret;
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�p�c���t���b�v���菈��																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: index	�F�Ώۂ̎Ԏ����C���f�b�N�X 0�`423													   |*/
/*| RETURN VALUE: ret	�F0 �p�c���t���b�v�ȊO�̑��u  1 �p�c���t���b�v										   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: namioka																					   |*/
/*| Date		: 2010/11/22																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2010 AMANO Corp.------[]*/
uchar	Aida_Flap_Chk( ushort index )
{
	uchar ret = 0;
	
	return ret;
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	��ԗv����M���̉��~���b�N���g���C�^�C�}�[����															   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: no	�F�Ώۂ̎Ԏ����C���f�b�N�X 0�`423													   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: namioka																					   |*/
/*| Date		: 2010/11/22																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2010 AMANO Corp.------[]*/
void	flp_DownLockRetry_RecvSts( ushort no )
{
	
	if( FLPCTL.flp_work.nstat.bits.b00 == 1 &&								// �ԗ�����
		FLPCTL.flp_work.nstat.bits.b02 == 0 &&								// �Ō�ɑ��M�����̂��A���~�w��
		FLPCTL.flp_work.nstat.bits.b05 == 1 &&								// ���~���b�N������
		DownLockFlag[ no - 1 ] == 1			&&								// �N�������珉��̏�ԗv��
		Aida_Flap_Chk((ushort)(no - 1))){								 	// ���u��ʂ��p�c���t���b�v
		FlpSet( no, 2 );													// Flaper DOWN
		DownLockFlag[ no - 1 ]++; 
	}
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�G���[���������Ă���t���b�v�^���b�N���u����������													   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: no		:�����C���f�b�N�X																   |*/
/*|				: errinfo	:�Ԏ��ԍ��i�[�A�h���X															   |*/
/*| RETURN VALUE: ret   �F0x00=�G���[�Ԏ��Ȃ�	0x00<>�G���[�Ԏ�����										   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: t.hashimoto																				   |*/
/*| Date		: 2013/02/27																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2013 AMANO Corp.------[]*/
uchar	flp_err_search( ushort sno, ulong* errinfo )
{
	uchar	ret_ecd;
	BITS	f_info;
	t_flp_DownLockErrInfo* pInfo;

	if( LockInfo[sno].lok_syu == 0 ){					// �ڑ�����?
		return( 0 );
	}

	ret_ecd = 0;
	// --- �G���[�`�F�b�N ---
	f_info = FLAPDT.flp_data[sno].nstat;
	if( f_info.word & 0x0030 ){							// �G���[���������ǂ���
		if( f_info.bits.b04 ){							// E1638:�㏸���b�N������
			ret_ecd |= 0x01;
		}
		if( f_info.bits.b05 ){							// E1639:�������b�N������
			ret_ecd |= 0x02;
		}
	}

	pInfo = &flp_DownLockErrInfo[sno];
	if( pInfo->f_info.BYTE & 0x03 ){
		if( pInfo->f_info.BIT.RETRY_OVER_ERR == 1 ){	// E1640:���g���C�I�[�o�[�G���[
			ret_ecd |= 0x04;
		}
		if( pInfo->f_info.BIT.RETRY_START_ERR == 1 ){	// E1641:���g���C����J�n�G���[
			ret_ecd |= 0x08;
		}
	}
	if( ret_ecd ){
		*errinfo = flp_ErrBinDataEdit( sno );
	}

	return( ret_ecd );
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�ԗ��Ȃ����̃t���b�v���~����																			   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| PARAMETER	: no		:�����C���f�b�N�X																   |*/
/*| RETURN VALUE: none																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Nagai																					   |*/
/*| Date		: 2015/01/20																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2015 AMANO Corp.------[]*/
static void flp_DownLock_ForCarSensorOff( ushort no )
{
	if( FLPCTL.flp_work.nstat.bits.b00 == 0 &&								// �ԗ��Ȃ�
		FLPCTL.flp_work.nstat.bits.b01 == 1 &&								// �㏸�ς�
		DownLockFlag[ no - 1 ] == 1){										// �N�������珉��̏�ԗv��
// MH322914 (s) kasiyama 2016/07/07 �����e�i���X�≓�u����t���b�v�㏸���s���ƃt���b�v�̏㏸��ɏ���Ƀt���b�v�����~���Ă��܂�(���ʉ��PNo.1252)
//		FlpSet( no, 2 );													// Flaper DOWN
		// �����e�i���X�E���u�̎��͍s��Ȃ�
		if( FLPCTL.flp_work.nstat.bits.b07 != 1){
			FlpSet( no, 2 );													// Flaper DOWN
		}
// MH322914 (e) kasiyama 2016/07/07 �����e�i���X�≓�u����t���b�v�㏸���s���ƃt���b�v�̏㏸��ɏ���Ƀt���b�v�����~���Ă��܂�(���ʉ��PNo.1252)
		DownLockFlag[ no - 1 ]++;
	}
}
