/*[]----------------------------------------------------------------------[]*/
/*| shutter control                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"mdl_def.h"
#include	"prm_tbl.h"


// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH321800(S) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
//char	pcard_shtter_ctl;
//void	Pcard_shut_close( void );
//// MH321800(E) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)

/*[]----------------------------------------------------------------------[]*/
/*| �w��������䏈���x                                                      |*/
/*|                                                                        |*/
/*|�����V���b�^�[�ƃR�C���V���b�^�[�������ɓ��삵�Ȃ��悤�ɐ��䂷��B      |*/
/*| �@���R�F�d���e�ʂ̌��O                                                 |*/
/*|��������@                                                              |*/
/*|	�V���b�^�[���삳����Ƃ��i�J�ł��ł��j�A                             |*/
/*|		���V���b�^�[�����쒆�Ȃ�A�����ۗ�����B                         |*/
/*|		���V���b�^�[�����쒆�܂��͓���֎~���Ȃ�A�����ۗ�����B         |*/
/*|	�Ƃ���B                                                               |*/
/*|���ϐ�                                                                  |*/
/*|�@�V���b�^�[����v���@BYTE  SHT_REQ�@|d7|d6|d5|d4|d3|d2|d1|d0|          |*/
/*|                                                                        |*/
/*|		d1d0 �� �@�@���V���b�^�[����v���@=0:�Ȃ��A=1:�v���A=2:�J�v��    |*/
/*|		d3d2 �� �R�C���V���b�^�[����v���@=0:�Ȃ��A=1:�v���A=2:�J�v��    |*/
/*|                                                                        |*/
/*|�@�V���b�^�[������  BYTE  SHT_CTRL�@|d7|d6|d5|d4|d3|d2|d1|d0|         |*/
/*|                                                                        |*/
/*|		d1d0 �� �@�@���V���b�^�[�����ԁ@=0:��~�A=1:���쒆�A=2:����֎~��|*/
/*|		d3d2 �� �R�C���V���b�^�[�����ԁ@=0:��~�A=1:���쒆�A=2:����֎~��|*/
/*|                                                                        |*/
/*|		���쒆�Ƃ́A�V���b�^�[����̃p���X�d������i��200ms�j�������B    |*/
/*|		����֎~���Ƃ́A�p���X�o�͂���������������ɐ݂�������֎~���     |*/
/*|		�i��200ms�j�������B                                                |*/
/*|		�V���b�^�[�J����̓\���m�C�h���x�㏸�h�~�̂���3sec�̓���֎~������ |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : shtctrl( void )                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       : 2005-12-20 T.Hashimoto                                  |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	shtctrl( void )
{
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	//----- ����������� -----
//	if( SHT_REQ.BIT.READ ){											// ����v�����肩?
//		if( !SHT_CTRL.BIT.READ && (SHT_CTRL.BIT.COIN != 1) ){		// ��������삵�Ă��Ȃ�
//			// ����������䒆�łȂ��A��ݼ���������삵�Ă��Ȃ�
//
//			if( SHT_REQ.BIT.READ == 1 ){
//				//���������ظ���
//				_di();
//				SHT_REQ.BIT.READ = 0;								// �v���𗎂Ƃ�
//				SHT_CTRL.BIT.READ = 1;								// ���Cذ�ް��������쒆
//				CP_RED_SHUT_CLOSE = 1;								// �������
//				CP_RED_SHUT_OPEN = 0;
//				_ei();
//				inc_dct( READ_SHUT_CT, 1 );							// ���춳��+1
//				LagTim20ms( LAG20_RD_SHTCTL, 11, TpuReadShut );		// 200ms�`220ms�߰ďo��
//				LedReq( RD_SHUTLED, LED_OFF );						// ���Cذ�ް�޲��LED OFF
//				READ_SHT_flg = 1;									// ��������
//			}
//			else if( SHT_REQ.BIT.READ == 2 ){
//				//��������Jظ���
//				_di();
//				SHT_REQ.BIT.READ = 0;								// �v���𗎂Ƃ�
//				SHT_CTRL.BIT.READ = 1;								// ���Cذ�ް��������쒆
//				CP_RED_SHUT_OPEN = 1;								// ������J
//				CP_RED_SHUT_CLOSE = 0;
//				_ei();
//				inc_dct( READ_SHUT_CT, 1 );							// ���춳��+1
//				LagTim20ms( LAG20_RD_SHTCTL, 11, TpuReadShut );		// 200ms�`220ms�߰ďo��
//				LedReq( RD_SHUTLED, LED_ONOFF );					// ���Cذ�ް�޲��LED�_��
//				READ_SHT_flg = 2;									// ������J���
//			}
//		}
//	}
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)

}

/*[]----------------------------------------------------------------------[]*/
/*| ذ�ް���������                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : rd_shutter( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| ����������Ԃ�ذ�ް�̏�Ԃ𔻒f�����������                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	rd_shutter( void )
{
	if( PPrmSS[S_P01][1] != 0 ){									// ������J��?
		if( GT_Settei_flg == 1 && OPECTL.Ope_Mnt_flg == 0){			// ���Cذ�ް���߈ُ�̏ꍇ�ŁA�������>�����������łȂ��ꍇ��
			return;                                                 // ���������݂����Ȃ�
		}
		read_sht_opn();												// ذ�ް������J�ELED ON
	}else{
		if(( RD_mod != 11 )&&( RD_mod != 10 )){						// ���ގ��o����?
			start_rdsht_cls();										// 500sec��ڲ��ɼ��������
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ������̋N�����ިڲ��ɍs��                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : read_sht_opn( void )                                    |*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2005-12-20                                              |*/
/*| Update       : 2005-12-20 T.Hashimoto                                  |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void start_rdsht_cls( void )
{
	LagTim20ms( LAG20_RD_SHTCLS, 26, read_sht_cls );				// 500ms���
}

/*[]----------------------------------------------------------------------[]*/
/*| ذ�ް������J����                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : read_sht_opn( void )                                    |*/
/*| PARAMETER    : unsigned char : �G���[����(0:��/1:�J)                   |*/
/*| RETURN VALUE : unsigned char : �G���[����(0:�G���[�Ȃ�/1:�G���[����)   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Ise                                                     |*/
/*| Date         : 2009-06-03                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
CERTIFY_ERR_ARM certify_opn_err[] =		/* �V���b�^�[�J����۔���p�e�[�u�� */
{
	{mod_read,  1},						/* ���C���[�_�[�ʐM�s�� */
	{mod_read,  2},						/* ���C���[�_�[�^�C�v�G���[ */
	{mod_read, 10},						/* ���C���[�_�[�����l�܂� */
	{mod_read, 11},						/* ���C�v�����^�����l�܂� */
};
uchar read_sht_jdg(uchar opn_cls)
{
	uchar ret = 0;						/* �G���[���茋�� */
	int i, loop;						/* �G���[�Ώۃe�[�u���Q�Ɨp�ϐ� */

	if(opn_cls)							/* ����������J���� */
	{
		if(OPECTL.op_faz < 2)			/* ����t�F�[�Y�����Z���ȊO */
		{
			loop = sizeof(certify_opn_err) / sizeof(certify_opn_err[0]);
			for(i = 0; i < loop; i++)	/* �G���[�Ώۂ̔�����Ԃ��擾 */
			{
				/* �Ώۂ̃G���[�������� */
				if(ERR_CHK[certify_opn_err[i].kind][certify_opn_err[i].code])
				{
					ret = 1;			/* �G���[���� */
					break;				/* �G���[���菈���𔲂��� */
				}
			}
		}
	}

	return ret;							/* �����I�� */
}

/*[]----------------------------------------------------------------------[]*/
/*| ذ�ް������J����                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : read_sht_opn( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       : 2005-12-20 T.Hashimoto                                  |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	read_sht_opn( void )
{
	//****************************
	LagCan20ms( LAG20_RD_SHTCLS );								// ��������ިڲ���~�߂�
	//****************************
	if( GT_Settei_flg == 1 && OPECTL.Ope_Mnt_flg == 0){			// ���Cذ�ް���߈ُ�̏ꍇ�ŁA�������>�����������łȂ��ꍇ��
		return;													// ���������݂����Ȃ�
	}
	if(read_sht_jdg(1))												// �V���b�^�[�J�s�\�ȃG���[������
	{
		return;														// ���������݂����Ȃ�
	}
// MH321800(S) hosoda IC�N���W�b�g�Ή� (�A���[�����)
// MH810103 GG119202(S) �݂Ȃ����ψ������̓���
//	if (ERR_CHK[mod_ec][ERR_EC_ALARM_TRADE_WAON] != 0) {			// WAON���ψُ킪�������Ă���
	if (ERR_CHK[mod_ec][ERR_EC_ALARM_TRADE_WAON] != 0 ||			// WAON���ψُ킪�������Ă���
		ERR_CHK[mod_ec][ERR_EC_SETTLE_ABORT] != 0) {				// ���Ϗ������s���������Ă���
// MH810103 GG119202(E) �݂Ȃ����ψ������̓���
		return;														// �V���b�^�[�I�[�v�������Ȃ�
	}
// MH321800(E) hosoda IC�N���W�b�g�Ή� (�A���[�����)

	if(( READ_SHT_flg != 2 )||( SHT_REQ.BIT.READ == 1 )){
		// ��������J�łȂ� or �v������
		SHT_REQ.BIT.READ = 2;										// ���Cذ�ް������Jظ���
		shtctrl();
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ذ�ް���������                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : read_sht_cls( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       : 2005-12-20 T.Hashimoto                                  |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	read_sht_cls( void )
{
	if(( READ_SHT_flg != 1 )||( SHT_REQ.BIT.READ == 2 )){
		// ��������J�łȂ� or �J�v������
		SHT_REQ.BIT.READ = 1;										// ���Cذ�ް�������ظ���
		shtctrl();
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ذ�ް������߰Đ���                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : TpuReadShut( void )                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	TpuReadShut( void )
{
	short	T;

	CP_RED_SHUT_OPEN = 0;											// ��ɲ��OFF(�߰ďo��OFF)
	CP_RED_SHUT_CLOSE = 0;

	if( 1 == SHT_CTRL.BIT.READ && OPECTL.Ope_Mnt_flg != 7 ){		// ���܂œ��쒆?
		SHT_CTRL.BIT.READ = 2;										// ����֎~��ԂƂ���
		if( READ_SHT_flg == 2 ){
			// �J���
			//T = 71;												// 1500ms�`1700ms���Ă���
			T = 151;												// 3000ms�`3200ms���Ă���
		}else{
			// ���
			T = 11;													// 200ms�`220ms���Ă���
		}
		LagTim20ms( LAG20_RD_SHTCTL, T, TpuReadShut );
	}else{
		SHT_CTRL.BIT.READ = 0;										// ��~��ԂƂ���
	}
	SHTTER_CTRL = 1;												// shtctrl()�̌Ăяo��
}

/*[]----------------------------------------------------------------------[]*/
/*| ��ݓ�����������߰Đ���                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : TpuCoinShut( void )                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hara                                                    |*/
/*| Date         : 2005-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	TpuCoinShut( void )
{
	if( 1 == SHT_CTRL.BIT.COIN ){									// ���܂œ��쒆?
		SHT_CTRL.BIT.COIN = 2;										// ����֎~��ԂƂ���
		LagTim20ms( LAG20_CN_SHTCTL, 11, TpuCoinShut );				// 200ms�`220ms���Ă���
	}else{
		SHT_CTRL.BIT.COIN = 0;										// ��~��ԂƂ���
	}
	SHTTER_CTRL = 1;												// shtctrl()�̌Ăяo��
}

/*[]----------------------------------------------------------------------[]*/
/*| ذ�ް���������                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : shutter_err_chk( void )                                 |*/
/*| PARAMETER    : count : ��ײ��                                        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| �������Ԃ���莞�ԓ��쒆�̂܂܂ɂȂ����ꍇ�A�ُ�Ɣ��f���āA��������� |*/
/*| �׸ނ�ر���A�����������s�Ȃ��B�i�ҋ@�E�x�Ɛ�p�����j			   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka                                                 |*/
/*| Date         : 2011-02-01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]*/
void	shutter_err_chk( uchar *count )
{

	if(( SHT_REQ.BIT.READ != 0 || SHT_CTRL.BIT.READ != 0 ) ||	// ���Cذ�ް����������쒆
	   ( SHT_REQ.BIT.COIN != 0 || SHT_CTRL.BIT.COIN != 0 )){	// ��ݼ���������쒆
	   	(*count)++;
		if( *count == RD_ERR_COUNT_MAX ){						// ��ײ�񐔏��
			_di();
			CP_RED_SHUT_OPEN = 0;											// ��ɲ��OFF(�߰ďo��OFF)
			CP_RED_SHUT_CLOSE = 0;
			_ei();
			Lagtim( OPETCBNO, 18, RD_SOLENOID_WAIT );			// ��ɲ�ޓ������ϰ����
		}else if( (*count) > RD_ERR_COUNT_MAX ){				// ��ײ�񐔏���𒴂��Ă���ꍇ�i����4��ځj
			_di();
			READ_SHT_flg = 0;									// ذ�ް�������Ԃ�������
			COIN_SHT_flg = 0;									// ��ݼ������Ԃ�������
			SHT_REQ.BYTE = 0;									// �v����Ԃ�ر
			SHT_CTRL.BYTE = 0;									// �����Ԃ�ر
			_ei();
			rd_shutter();										// ���Cذ�ް����������
			*count = 0;											// ���Đ���ر
		}else{													// ��ײ���ް���Ă��Ȃ�
			Lagtim( OPETCBNO, 18, RD_ERR_INTERVAL );			// �����Ԋu�p����ϰ����
		}
	}
}
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH321800(S) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
//void	Pcard_shut_close( void )
//{
//	uchar	ist;
//
//	if( (SHT_CTRL.BIT.READ == 2)&&(SHT_CTRL.BIT.COIN != 1) ){	// ��~��ԂłȂ�
//		if( READ_SHT_flg == 2 ){
//			LagCan20ms( LAG20_RD_SHTCTL );
//			ist = _di2();										// ���荞�݋֎~
//			SHT_CTRL.BIT.READ = 0;
//			_ei2( ist );										// ���荞�݃X�e�[�^�X�����ɖ߂�
//		}
//	}
//	read_sht_cls();
//	pcard_shtter_ctl = 1;
//}
//// MH321800(E) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
