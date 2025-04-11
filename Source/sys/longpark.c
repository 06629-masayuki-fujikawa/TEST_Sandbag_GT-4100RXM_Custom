// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
/*[]----------------------------------------------------------------------[]*/
/*| �������ԃ`�F�b�N�@�\                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2018-09-06                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]*/
#include	<string.h>
#include	"system.h"
#include	"Message.h"
#include	"mem_def.h"
#include	"flp_def.h"
#include	"ntnet_def.h"
#include	"ope_def.h"
#include	"prm_tbl.h"
#include	"LKmain.h"
#include	"AppServ.h"

void LongTermParkingCheck( void );
void LongTermParkingCheck_r10( void );
void LongTermParkingCheck_Resend( void );
void LongTermParkingCheck_Resend_flagset( void );
void LongTermParkingRel( ulong LockNo , uchar knd, flp_com *flp);

extern	void LongTermParkingCheck_r10_prmcng( void );
extern	void LongTermParkingCheck_r10_defset( void );
static short LongTermParking_stay_hour_check(flp_com *p, short hours);

#define ARMLOG_CHK_COUNT 50 // �A���[�����O�̘A���o�^�̃}�X�N����
//[]----------------------------------------------------------------------[]
///	@brief		�������ԃ`�F�b�N(�A���[���f�[�^�ɂ�錟�o)
//[]----------------------------------------------------------------------[]
///	@return		
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date		Create	:	2018/09/06<br>
///				Update	:	
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
void	LongTermParkingCheck( void )
{
	ushort	wHour;
	ushort	i,j;
	ulong	ulwork;
	uchar	prm_wk;

	prm_wk = (uchar)prm_get(COM_PRM, S_TYP, 135, 1, 5);				// �������Ԍ��o����
	if((prm_wk == 0) || (prm_wk == 2)){// �������Ԍ��o(�A���[���f�[�^�ɂ��ʒm)
		wHour = (ushort)prm_get(COM_PRM,  S_TYP, 135, 4, 1);		// ��������
		if(wHour == 0){
			wHour = LONG_PARK_TIME1_DEF;	// �ݒ肪0�̏ꍇ�͋����I��48���ԂƂ���
		}
		// �����O�ɃA���[�����O��FROM�������ݒ��̏ꍇ��U������(��Ăɔ��������ꍇ�Ƀ��O�o�^�̃p���N��h������)
		if (AppServ_IsLogFlashWriting(eLOG_ALARM) != 0){	// �A���[�����O��FROM�������ݒ��H
			return;
		}
		if(prm_get( COM_PRM, S_PRN, 33, 1, 1 ) == 0 ){	// 18-0033�E�������ԃA���[���W���[�i���� ����
			if(NG == PriJrnExeCheck()){					// �W���[�i���v�����^�󎚒�
				// �W�v�󎚂����s����Ă���ꍇ�͒����Ԉ󎚂ł��Ȃ��P�[�X������̂ł��̂��߂̃K�[�h����
				return;
			}
		}
		for( i = 0,j = 0; i < LOCK_MAX; i++ ){
			WACDOG;													// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
			if ((LockInfo[i].lok_syu != 0) && (LockInfo[i].ryo_syu != 0) ) {// �Ԏ��L��
				if((FLAPDT.flp_data[i].mode >= FLAP_CTRL_MODE2) && (FLAPDT.flp_data[i].mode <= FLAP_CTRL_MODE4) ) {//�u�o�ɏ������v�܂��́u��ԁv�ȊO�ŗL��
					ulwork = (ulong)(( LockInfo[i].area * 10000L ) + LockInfo[i].posi );	// �����擾
					if(LongTermParking_stay_hour_check( &FLAPDT.flp_data[i], (short)wHour ) != 0 ) {// �w�莞�Ԉȏ���
						if(LongParkingFlag[i] == 0){					// �������ԏ�ԁF�������ԏ�ԂȂ�
							LongParkingFlag[i] = 1;						// �������ԏ�Ԃ���
							alm_chk2(ALMMDL_MAIN, ALARM_LONG_PARKING, 1, 2, 1, &ulwork);	// A0031�o�^ �������Ԍ��o
							j++;// ���[�v���̃A���[���o�^�����J�E���g�A�b�v
						}
					}else{// �w�莞�Ԗ���
						if(LongParkingFlag[i] != 0){					// �������ԏ�ԁF�������ԏ�Ԃ���
							// �����ύX�ɂ�蒷�����ԏ�ԁu����v���u�Ȃ��v�ɕω�����ꍇ�͉�������
							LongParkingFlag[i] = 0;						// �������ԏ�ԂȂ�
							alm_chk2(ALMMDL_MAIN, ALARM_LONG_PARKING, 0, 2, 1, &ulwork);	// A0031���� �������Ԍ��o
							j++;// ���[�v���̃A���[���o�^�����J�E���g�A�b�v
						}
					}
				}
			}
			// �{�֐���10�b���ƂɃR�[�������B�A���[��50���󎚂���̂ɖ�4.5�b�̂��߂���2�{�̎��Ԃ����Ă��܂�
			if(prm_get( COM_PRM, S_PRN, 33, 1, 1 ) == 0 ){// 18-0033�E�������ԃA���[���W���[�i���� ����
				if(j >= ARMLOG_CHK_COUNT){// ���[�v���ł̃A���[���o�^�������B
					break;// �W���[�i���󎚃o�b�t�@�o�^���ӂ�}���̂��ߔ�����
				}
			}
			// �A���[�����O��FROM�������ݒ��ɂȂ������U������(��Ăɔ��������ꍇ�Ƀ��O�o�^�̃p���N��h������)
			if (AppServ_IsLogFlashWriting(eLOG_ALARM) != 0){	// �A���[�����O��FROM�������ݒ��H
				break;
			}
		}
	}
}
//[]----------------------------------------------------------------------[]
///	@brief		�������ԃ`�F�b�N(�������ԃf�[�^�ɂ�錟�o)
//[]----------------------------------------------------------------------[]
///	@return		
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date		Create	:	2018/09/06<br>
///				Update	:	
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
void	LongTermParkingCheck_r10( void )
{
	ushort	wHour,wHour2;
	ushort	i;
	uchar	prm_wk;

	prm_wk = (uchar)prm_get(COM_PRM, S_TYP, 135, 1, 5);				// �������Ԍ��o����
	if(prm_wk == 2){	// �������Ԍ��o(�������ԃf�[�^�ɂ��ʒm)
		wHour = (ushort)prm_get(COM_PRM,  S_TYP, 135, 4, 1);		// �������Ԍ��o����1
		if(wHour == 0){
			wHour = LONG_PARK_TIME1_DEF;	// �ݒ肪0�̏ꍇ�͋����I��48����(2��)�Ƃ���
		}
		wHour2 = (ushort)prm_get(COM_PRM,  S_TYP, 136, 4, 1);		// �������Ԍ��o����2

		// �����O�ɒ������ԃ��O��FROM�������ݒ��̏ꍇ��U������(��Ăɔ��������ꍇ�Ƀ��O�o�^�̃p���N��h������)
		if (AppServ_IsLogFlashWriting(eLOG_LONGPARK_PWEB) != 0){	// �������ԃ��O��FROM�������ݒ��H
			return;
		}
		for( i = 0; i < LOCK_MAX; i++ ){
			WACDOG;													// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
			if ((LockInfo[i].lok_syu != 0) && (LockInfo[i].ryo_syu != 0) ) {// �Ԏ��L��
				if((FLAPDT.flp_data[i].mode >= FLAP_CTRL_MODE2) && (FLAPDT.flp_data[i].mode <= FLAP_CTRL_MODE4) ) {//�u�o�ɏ������v�܂��́u��ԁv�ȊO�ŗL��

					// �������Ԍ��o����1�̃`�F�b�N
					if(LongTermParking_stay_hour_check( &FLAPDT.flp_data[i], (short)wHour ) != 0 ) {	// �w�莞�Ԉȏ���
						if(FLAPDT.flp_data[i].flp_state.BIT.b00 == 0){									// �������ԏ�ԁF�������ԏ�ԂȂ�
							Make_Log_LongParking_Pweb( i+1, wHour, LONGPARK_LOG_SET, LONGPARK_LOG_NON);	// ��������1�F���� �������ԃf�[�^���O����
							Log_regist( LOG_LONGPARK );	// �������ԃf�[�^���O�o�^
							FLAPDT.flp_data[i].flp_state.BIT.b00 = 1;	// �������ԏ�Ԃ���
						}
					}
					// �������ԃf�[�^�̏ꍇ�A�����ύX�ɂ�钷�����ԏ�ԁu����v���u�Ȃ��v�ł͉������Ȃ�

					if(wHour2 != 0){// ���o����2�̐ݒ肪�L���ȏꍇ
						// �������Ԍ��o����2�̃`�F�b�N
						if(LongTermParking_stay_hour_check( &FLAPDT.flp_data[i], (short)wHour2 ) != 0 ) {	// �w�莞�Ԉȏ���
							if(FLAPDT.flp_data[i].flp_state.BIT.b01 == 0){									// �������ԏ�ԁF�������ԏ�ԂȂ�
								Make_Log_LongParking_Pweb( i+1, wHour2, LONGPARK_LOG_SET, LONGPARK_LOG_NON);	// ��������2�F���� �������ԃf�[�^���O����
								Log_regist( LOG_LONGPARK );	// �������ԃf�[�^���O�o�^
								FLAPDT.flp_data[i].flp_state.BIT.b01 = 1;	// �������ԏ�Ԃ���
							}
						}
						// �������ԃf�[�^�̏ꍇ�A�����ύX�ɂ�钷�����ԏ�ԁu����v���u�Ȃ��v�ł͉������Ȃ�
					}
				}
			}
			
			// �������ԃ��O��FROM�������ݒ��ɂȂ������U������(��Ăɔ��������ꍇ�Ƀ��O�o�^�̃p���N��h������)
			if (AppServ_IsLogFlashWriting(eLOG_LONGPARK_PWEB) != 0){	// �������ԃ��O��FROM�������ݒ��H
				break;
			}
		}
	}
}
//[]----------------------------------------------------------------------[]
///	@brief		�������Ԃ̐ݒ肪�ύX���ꂽ���̊m�F(�������ԃf�[�^�ɂ�錟�o)
//[]----------------------------------------------------------------------[]
///	@return		
///	@author		A.Iiizumi
///	@note		�������Ԃ̌��o���Ԃ��ύX���ꂽ�ꍇ�A���󌟏o���Ă��钷�����ԏ�Ԃ���U��������
///				ParkingWeb�̗�������폜���邽�߁B
///				������ALongTermParkingCheck_r10()�֐��ɂōČ��o����B
//[]----------------------------------------------------------------------[]
///	@date		Create	:	2018/09/06<br>
///				Update	:	
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
void	LongTermParkingCheck_r10_prmcng( void )
{
	ushort	wHour,wHour2;
	uchar	prm_wk;

	prm_wk = (uchar)prm_get(COM_PRM, S_TYP, 135, 1, 5);				// �������Ԍ��o����
	if(prm_wk == 2){	// �������Ԍ��o(�������ԃf�[�^�ɂ��ʒm)
		// �����O�ɒ������ԃ��O��FROM�������ݒ��̏ꍇ��U������(��Ăɔ��������ꍇ�Ƀ��O�o�^�̃p���N��h������)
		if (AppServ_IsLogFlashWriting(eLOG_LONGPARK_PWEB) != 0){	// �������ԃ��O��FROM�������ݒ��H
			return;
		}
		wHour = (ushort)prm_get(COM_PRM,  S_TYP, 135, 4, 1);		// �������Ԍ��o����1
		wHour2 = (ushort)prm_get(COM_PRM,  S_TYP, 136, 4, 1);		// �������Ԍ��o����2
		if(wHour == 0){
			wHour = LONG_PARK_TIME1_DEF;	// �ݒ肪0�̏ꍇ�͋����I��48����(2��)�Ƃ���
		}

		if( LongPark_Prmcng.time1 != wHour ){		// ���o����1 �ݒ�ω�����
			LongPark_Prmcng.f_prm_cng = 1;			// �ݒ�ύX���o�t���O�Z�b�g
		}
		
		if( LongPark_Prmcng.time2 != wHour2 ){		// ���o����2 �ݒ�ω�����
			if(LongPark_Prmcng.time2 == 0){
				// ���o����2�����g�p���g�p�ƂȂ����ꍇ�͐ݒ�̕ύX���L�����邾���Œ������ԏ�Ԃ͉������Ȃ��B
				// ���̏ꍇ�A�ݒ�ǉ��ɂ�錟�o���s�������ŁA����������ĉ�������K�v�͂Ȃ�����
				LongPark_Prmcng.time2 = wHour2; // ���o����2�̐ݒ��ԍX�V
			}else{
				LongPark_Prmcng.f_prm_cng = 1;// �ݒ�ύX���o�t���O�Z�b�g
			}
		}
		
		if(LongPark_Prmcng.f_prm_cng == 0){
			LongPark_Prmcng.cng_count = 0;// �Ԏ������p�̃J�E���^�̃N���A
			return;// �ݒ�ύX���o�Ȃ����͏������Ȃ�
		}
		// LongPark_Prmcng.cng_count��LOCK_MAX�̎��͍X�V���I����Ă���͂��Ȃ̂ŏ�Ԃ��X�V����
		// �ȉ��u��d�|�C���g�@�v�œd�����������Ƃ��̑΍�
		if(LongPark_Prmcng.cng_count >= LOCK_MAX ){
			LongPark_Prmcng.time1 = wHour; // ���o����1�̐ݒ��ԍX�V
			LongPark_Prmcng.time2 = wHour2; // ���o����2�̐ݒ��ԍX�V
			LongPark_Prmcng.f_prm_cng = 0;//�����I�����t���O�N���A
			LongPark_Prmcng.cng_count = 0;// �Ԏ������p�̃J�E���^�̃N���A
			return;
		}
		
		while( LongPark_Prmcng.cng_count < LOCK_MAX){
			WACDOG;													// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
			if ((LockInfo[LongPark_Prmcng.cng_count].lok_syu != 0) && (LockInfo[LongPark_Prmcng.cng_count].ryo_syu != 0)) {// �Ԏ��L��
				// ParkingWeb�ł͒������ԃf�[�^�ɂ�錟�o�����͓��Ɏ��ԂƎԎ����݂̂ōs���Ă��邽�ߋ��������ł�
				// �K���g�p���Ă��錟�o����1�̃f�[�^�ŉ�����o�^����
				if((FLAPDT.flp_data[LongPark_Prmcng.cng_count].flp_state.BIT.b00 != 0)||	// ��������1���o���� �܂���
				   (FLAPDT.flp_data[LongPark_Prmcng.cng_count].flp_state.BIT.b01 != 0)){	// ��������2���o����
				   // ��������1�F���� �������ԃf�[�^���O����
					Make_Log_LongParking_Pweb( LongPark_Prmcng.cng_count+1, wHour, LONGPARK_LOG_RESET, LONGPARK_LOG_NON);
					Log_regist( LOG_LONGPARK );	// �������ԃf�[�^���O�o�^
					FLAPDT.flp_data[LongPark_Prmcng.cng_count].flp_state.BIT.b00 = 0;	// ��������1����
					FLAPDT.flp_data[LongPark_Prmcng.cng_count].flp_state.BIT.b01 = 0;	// ��������2����
				}
			}
			LongPark_Prmcng.cng_count++;			// �Ԏ������p�̃J�E���^�̉��Z
			// ��d�|�C���g�@
			if(LongPark_Prmcng.cng_count >= LOCK_MAX ){
				LongPark_Prmcng.time1 = wHour;		// ���o����1�̐ݒ��ԍX�V
				LongPark_Prmcng.time2 = wHour2;		// ���o����2�̐ݒ��ԍX�V
				LongPark_Prmcng.f_prm_cng = 0;		// �����I�����t���O�N���A
				LongPark_Prmcng.cng_count = 0;		// �Ԏ������p�̃J�E���^�̃N���A
				break;
			}
			
			// �������ԃ��O��FROM�������ݒ��ɂȂ������U������(��Ăɔ��������ꍇ�Ƀ��O�o�^�̃p���N��h������)
			if (AppServ_IsLogFlashWriting(eLOG_LONGPARK_PWEB) != 0){	// �������ԃ��O��FROM�������ݒ��H
				break;
			}
		}
	}
}
//[]----------------------------------------------------------------------[]
///	@brief		�������ԃ`�F�b�N�̐ݒ��Ԃ̍X�V����(�������ԃf�[�^�ɂ�錟�o)
//[]----------------------------------------------------------------------[]
///	@return		
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date		Create	:	2018/09/06<br>
///				Update	:	
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
void	LongTermParkingCheck_r10_defset( void )
{
	ushort	wHour,wHour2;

	wHour = (ushort)prm_get(COM_PRM,  S_TYP, 135, 4, 1);		// �������Ԍ��o����1
	wHour2 = (ushort)prm_get(COM_PRM,  S_TYP, 136, 4, 1);		// �������Ԍ��o����2
	if(wHour == 0){
		wHour = LONG_PARK_TIME1_DEF;	// �ݒ肪0�̏ꍇ�͋����I��48����(2��)�Ƃ���
	}
	LongPark_Prmcng.time1 = wHour;		// ���o����1�̐ݒ��ԍX�V
	LongPark_Prmcng.time2 = wHour2;		// ���o����2�̐ݒ��ԍX�V
}
//[]----------------------------------------------------------------------[]
///	@brief		�������Ԏ��ԃ`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]  flp_com *p   : �擪�ԍ�
///	@param[in]  short   hours: ���Ԏ��� 
///	@return     ret          : 0:�w�莞�Ԗ��� 1:�w�莞�ԓ��B
///	@author     A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date		Create	:	2018/05/16<br>
///				Update	:	
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
short LongTermParking_stay_hour_check(flp_com *p, short hours)
{
	short	year, mon, day, hour, min;
	short	add_day, add_hour;

	year = p->year;		/* ���ɔN */
	mon = p->mont;		/* ���Ɍ� */
	day = p->date;		/* ���ɓ� */
	hour = p->hour;		/* ���Ɏ� */
	min = p->minu;		/* ���ɕ� */

	// ���Ԏ���(��)����A���Ɋ��Z����
	add_day = hours / 24;
	add_hour = hours % 24;

	// ���ɓ�������add_hour���Ԍ�̓������擾����
	hour += add_hour;
	if(hour >= 24){
		day++;//1�����Z
		hour = hour % 24;
		for ( ; ; ) {
			if (day <= medget(year, mon)) {// �����ȉ�
				break;
			}
			// �Y�����̓����������Č������Z
			day -= medget(year, mon);
			if (++mon > 12) {
				mon = 1;
				year++;
			}
		}
	}
	// ���ɓ�������add_day����̓������擾����
	day += add_day;
	for ( ; ; ) {
		if (day <= medget(year, mon)) {// �����ȉ�
			break;
		}
		// �Y�����̓����������Č������Z
		day -= medget(year, mon);
		if (++mon > 12) {
			mon = 1;
			year++;
		}
	}

	if(CLK_REC.year > year){
		return 1;	//���ݔN���������ԔN���߂���(�w�莞�ԓ��B)
	}
	if(CLK_REC.year < year){
		return 0;	//���ݔN���������ԔN���ߋ�(�w�莞�Ԗ���)
	}
	// �ȉ� �N�͓���
	if(CLK_REC.mont > mon){
		return 1;	//���݌����������Ԍ����߂���(�w�莞�ԓ��B)
	}
	if(CLK_REC.mont < mon){
		return 0;	//���݌����������Ԍ����ߋ�(�w�莞�Ԗ���)
	}
	// �ȉ� ���͓���
	if(CLK_REC.date > day){
		return 1;	//���ݓ����������ԓ����߂���(�w�莞�ԓ��B)
	}
	if(CLK_REC.date < day){
		return 0;	//���ݓ����������ԓ����ߋ�(�w�莞�Ԗ���)
	}
	// �ȉ� ���͓���
	if(CLK_REC.hour > hour){
		return 1;	//���ݎ����������Ԏ����߂���(�w�莞�ԓ��B)
	}
	if(CLK_REC.hour < hour){
		return 0;	//���ݎ����������Ԏ����ߋ�(�w�莞�Ԗ���)
	}
	// �ȉ� ���͓���
	if(CLK_REC.minu > min){
		return 1;	//���ݕ����������ԕ����߂���(�w�莞�ԓ��B)
	}
	if(CLK_REC.minu < min){
		return 0;	//���ݕ����������ԕ����ߋ�(�w�莞�Ԗ���)
	}
	return 1;// �N����������v(�w�莞�ԓ��B)
}
//[]----------------------------------------------------------------------[]
///	@brief		�������ԃ`�F�b�N(�h�A���ɃZ���^�[�ɑ΂��đ��M���鏈��)
//[]----------------------------------------------------------------------[]
///	@return		
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date		Create	:	2018/09/06<br>
///				Update	:	
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
void	LongTermParkingCheck_Resend( void )
{
	ushort	wHour;
	ulong	ulwork;
	uchar	prm_wk;

	prm_wk = (uchar)prm_get(COM_PRM, S_TYP, 135, 1, 5);				// �������Ԍ��o����
	if((prm_wk == 0) || (prm_wk == 2)){
		wHour = (ushort)prm_get(COM_PRM,  S_TYP, 135, 4, 1);		// ��������
		if(wHour == 0){
			wHour = LONG_PARK_TIME1_DEF;// �ݒ肪0�̏ꍇ�͋����I��48���ԂƂ���
		}

		if( isDefToErrAlmTbl(1, ALMMDL_MAIN, ALARM_LONG_PARKING) == FALSE ){
			// �A���[���f�[�^�̑��M���x���̐ݒ�i34-0037�B�j�ő��M���郌�x���łȂ��ꍇ�̓`�F�b�N���Ȃ�
			f_LongParkCheck_resend = 0;//�t���O�N���A
			LongParkCheck_resend_count = 0;
			return;
		}
		// �h�A���ɒ������ԃ`�F�b�N���s�����o���ł���΃Z���^�[�ɑ΂��čđ����邽�߂Ƀ��O�ɓo�^����

		if(f_LongParkCheck_resend == 0){// �v���Ȃ�
			return;
		}
		// �A���[�����O��FROM�������ݒ��ɂȂ������U������
		if (AppServ_IsLogFlashWriting(eLOG_ALARM) != 0){	// �A���[�����O��FROM�������ݒ��H
			return;
		}
		while( LongParkCheck_resend_count < LOCK_MAX ){
			WACDOG;													// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
			if ((LockInfo[LongParkCheck_resend_count].lok_syu != 0) && (LockInfo[LongParkCheck_resend_count].ryo_syu != 0) ) {// �Ԏ��L��
				//�u�o�ɏ������v�܂��́u��ԁv�ȊO�ŗL��
				if((FLAPDT.flp_data[LongParkCheck_resend_count].mode >= FLAP_CTRL_MODE2) && (FLAPDT.flp_data[LongParkCheck_resend_count].mode <= FLAP_CTRL_MODE4) ) {
					ulwork = (ulong)(( LockInfo[LongParkCheck_resend_count].area * 10000L ) + LockInfo[LongParkCheck_resend_count].posi );	// �����擾
					if(LongTermParking_stay_hour_check( &FLAPDT.flp_data[LongParkCheck_resend_count], (short)wHour ) != 0 ) {// �w�莞�Ԉȏ���
						// A0031�o�^ �������Ԍ��o
						memcpy( &Arm_work.Date_Time, &CLK_REC, sizeof( date_time_rec ) );	// ��������
						Arm_work.Armsyu = ALMMDL_MAIN;										// �װю��
						Arm_work.Armcod = ALARM_LONG_PARKING;								// �װѺ���
						Arm_work.Armdtc = 1;												// �װє���/����
											
						Arm_work.Armlev = (uchar)getAlmLevel( ALMMDL_MAIN, ALARM_LONG_PARKING );// �װ�����
						Arm_work.ArmDoor = ERR_LOG_RESEND_F;								// �đ���ԂƂ���
						Arm_work.Arminf = 2;												// �t���ް�(bin)����
						Arm_work.ArmBinDat = ulwork;										// bin �A���[�������

						Log_regist( LOG_ALARM );											// �װ�۸ޓo�^
					}
				}
			}
			LongParkCheck_resend_count++;
			if(LongParkCheck_resend_count >= LOCK_MAX ){
				LongParkCheck_resend_count = 0;
				f_LongParkCheck_resend = 0;//�����I�����t���O�N���A
				break;
			}
			// �A���[�����O��FROM�������ݒ��ɂȂ������U������(��Ăɔ��������ꍇ�Ƀ��O�o�^�̃p���N��h������)
			if (AppServ_IsLogFlashWriting(eLOG_ALARM) != 0){	// �A���[�����O��FROM�������ݒ��H
				break;
			}
		}
	}

}
//[]----------------------------------------------------------------------[]
///	@brief		�������ԃ`�F�b�N�t���O�Z�b�g����(�h�A���ɃZ���^�[�ɑ΂��đ��M���鏈��)
//[]----------------------------------------------------------------------[]
///	@return		
///	@author		A.Iiizumi
///	@note		�h�A�ɂ��Z���^�[�֑��M����A���[�����Z�b�g������t���O�̔��菈��
//[]----------------------------------------------------------------------[]
///	@date		Create	:	2018/09/06<br>
///				Update	:	
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
void	LongTermParkingCheck_Resend_flagset( void )
{
	uchar	prm_wk;
	
	prm_wk = (uchar)prm_get(COM_PRM, S_TYP, 135, 1, 5);				// �������Ԍ��o����
	if((prm_wk == 0) || (prm_wk == 2)){
		if(f_LongParkCheck_resend == 0){
			// �t���O�����󂯕t������́A�������ԃA���[���f�[�^�̑��M�������I����܂Ŏ󂯕t���Ȃ��悤�ɂ���
			f_LongParkCheck_resend = 1;
			LongParkCheck_resend_count = 0;
		}
	}else{
		f_LongParkCheck_resend = 0;
		LongParkCheck_resend_count = 0;
	}
}
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
