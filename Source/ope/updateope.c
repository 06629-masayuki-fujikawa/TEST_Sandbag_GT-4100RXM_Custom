//[]----------------------------------------------------------------------[]
///	@file		update_ope.c
///	@brief		�^�p�ʊǗ�����
/// @date		2012/04/24
///	@author		A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]

/*--- Include -------------------*/
#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"AppServ.h"
#include	"fla_def.h"
#include	"updateope.h"

/*--- Pragma --------------------*/

/*--- Prototype -----------------*/
void update_sw_area_init(void);
void get_from_swdata(void);
unsigned char chk_wave_data_onfrom(unsigned short, unsigned char *);
void wave_data_swupdate(void);
void parm_data_swupdate(void);
unsigned char wave_data_swchk(void);
unsigned char parm_data_swchk(void);
void update_flag_set(unsigned char);
void update_flag_clear(unsigned char);
unsigned char chk_prog_data_onfrom(unsigned char *dat);

/*--- Define --------------------*/

/*--- Gloval Value --------------*/

/*--- Extern --------------------*/


//[]----------------------------------------------------------------------[]
///	@brief			�^�p�ʏ�񏉊�������
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		RSW=4(�X�[�p�[�C�j�V�����C�Y�̎��̂݃R�[������)
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void update_sw_area_init(void)
{
	// �X�V�t���O�̏�����(�v���O�����X�V�t���O,�����f�[�^�X�V�t���O,���ʃp�����[�^�X�V�t���O)
	update_flag_clear(PROGRAM_UPDATE_FLAG | WAVEDATA_UPDATE_FLAG | PARAM_UPDATE_FLAG);
	// �^�p�ʏ��o�b�N�A�b�v(�X�V���̃��J�o���p)
	BootInfo.sw_bakup.wave = 0;
	BootInfo.sw_bakup.parm = 0;
	// �t���b�V���ɏ����ꂽ�^�p�ʏ��
	BootInfo.sw_flash.wave = 0;
	BootInfo.sw_flash.parm = 0;
	// �u�[�g�v���O�������ł̂ݎg�p����G���[���g���C�J�E���^�ł��邪�A�O�̂��ߏ���������
	BootInfo.err_count = 0;// �u�[�g�v���O�����Ŕ��������G���[�̉�

	// �^�p�ʏ��
	// ���ʃp�����[�^�̉^�p�ʂ������I�ɖ�1�ɂ���
	BootInfo.sw.parm = OPERATE_AREA1;
	FLT_write_parm_swdata(OPERATE_AREA1);								// FROM�̉^�p�ʂ���1�Ƃ���
	// NOTE:�����f�[�^�̏ꍇ�A�C���X�g�[���������_�ŏ������ނ̂ł����ł͉������Ȃ�
	// �v���O�����͓���FROM�œ��삷�邽�߁A�O��FROM�Ƀ_�E�����[�h�v���O���������FROM�ɑ΂���
	// �X�V��������@�\�̂ݓ��ڂ���d�l
}

//[]----------------------------------------------------------------------[]
///	@brief			FROM�̉^�p�ʏ��擾����
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void get_from_swdata(void)
{
	BootInfo.sw_flash.wave = FLT_get_wave_swdata();
	BootInfo.sw_flash.parm = FLT_get_parm_swdata();
}

//[]----------------------------------------------------------------------[]
///	@brief			�����f�[�^���w�肵���^�p�ʂɊi�[����Ă��邩�`�F�b�N����
//[]----------------------------------------------------------------------[]
///	@param[in]		sw :�^�p�ʏ�� OPERATE_AREA1(��1)�܂���OPERATE_AREA2(��2)
///	@param[in]		*dat :�o�[�W���������i�[���ė~�����|�C���^(�o�[�W�����f�[�^��8Byte)
///					      NULL���w�肵���ꍇ�ƃ`�F�b�N���ʂ��f�[�^�����̏ꍇ�͊i�[���Ȃ�
///	@return			0:�f�[�^�L 1:�f�[�^����
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char chk_wave_data_onfrom(unsigned short sw, unsigned char *dat)
{
	int i,j;
	int f_ok = 1;
	
	if(OPERATE_AREA1 == sw){						// �^�p��1
		FLT_read_wave_sum_version(0,swdata_write_buf);
	} else {										// �^�p��2
		FLT_read_wave_sum_version(1,swdata_write_buf);
	}
	// �����O�X�̈�̊m�F
	for(i = LENGTH_OFFSET; i<(LENGTH_OFFSET+LENGTH_SIZE);i++){
		if(swdata_write_buf[i] != 0xFF){
			f_ok = 0;//�f�[�^�L
			break;
		}
	}
	if(f_ok == 1){
		return 1;									// �u�����N�Ȃ̂Ńf�[�^�Ȃ�
	}
	// �`�F�b�N�T���̈�̊m�F
	if((0xFF == swdata_write_buf[SUM_OFFSET])&&(0xFF == swdata_write_buf[SUM_OFFSET+1])) {
		return 1;									// �u�����N�Ȃ̂Ńf�[�^�Ȃ�
	}
	// �o�[�W������ASCII�̕����R�[�h�͈͊O�Ȃ�L���f�[�^�Ȃ�
	for(i = VER_OFFSET; i<(VER_OFFSET+VER_SIZE);i++){
		if((swdata_write_buf[i] < 0x20 )||(swdata_write_buf[i] > 0x7A)) {
			return 1;// ASCII�̃X�y�[�X����'z'�͈̔͊O(�����R�[�h���ΏۊO)
		}
	}
	// �����ɗ������_�Ńf�[�^����
	if(dat != NULL){
		for(i = VER_OFFSET,j = 0; i<(VER_OFFSET+VER_SIZE); i++, j++){
			dat[j] = swdata_write_buf[i];
		}
	}
	return 0;// �f�[�^�L
}
//[]----------------------------------------------------------------------[]
///	@brief			�����f�[�^�^�p�ʍX�V����
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void wave_data_swupdate(void)
{
// MH810100(S) Y.Yamauchi 2020/03/05 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	if((BootInfo.sw.wave != OPERATE_AREA1)&&(BootInfo.sw.wave != OPERATE_AREA2)) {
//		memset(BootInfo.f_wave_update,0x00,sizeof(BootInfo.f_wave_update));// �O�̂��ߍX�V�t���O�N���A
//		return;// �X�V�����͓���RAM�̖ʏ�񂪐���ł��邱�Ƃ��O��A�ُ펞�͍X�V���Ȃ�
//	}
//	
//	if(0 == memcmp(BootInfo.f_wave_update,wave_update_flag,sizeof(BootInfo.f_wave_update))) {// �X�V�t���O�L
//		if((BootInfo.sw_bakup.wave != OPERATE_AREA1)&&(BootInfo.sw_bakup.wave != OPERATE_AREA2)) {
//			// �^�p�ʏ��o�b�N�A�b�v���Ȃ��ꍇ
//			BootInfo.sw_bakup.wave = BootInfo.sw.wave;// FROM�X�V�r���œd���f���������l�����ĉ^�p�ʏ����o�b�N�A�b�v
//		}
//		if((BootInfo.sw_flash.wave != OPERATE_AREA1)&&(BootInfo.sw_flash.wave != OPERATE_AREA2)) {
//			// �O��FROM�ɉ^�p�ʏ�񂪖���(FROM�X�V���̓d���f�̃P�[�X)
//			if(BootInfo.sw.wave == OPERATE_AREA1) {		// ���݂̉^�p�ʂ�1�̏ꍇ
//				FLT_write_wave_swdata(OPERATE_AREA2);
//				BootInfo.sw_flash.wave = OPERATE_AREA2;
//			} else {									// ���݂̉^�p�ʂ�2�̏ꍇ
//				FLT_write_wave_swdata(OPERATE_AREA1);
//				BootInfo.sw_flash.wave = OPERATE_AREA1;
//			}
//		} else {
//			// �O��FROM�Ƀf�[�^�L
//			if(BootInfo.sw_bakup.wave == BootInfo.sw_flash.wave) {
//				// FROM�Ɖ^�p�ʏ��o�b�N�A�b�v����v�������͐��FROM�̍X�V���s�Ȃ�
//				if(BootInfo.sw.wave == OPERATE_AREA1) {		// ���݂̉^�p�ʂ�1�̏ꍇ
//					FLT_write_wave_swdata(OPERATE_AREA2);
//					BootInfo.sw_flash.wave = OPERATE_AREA2;
//				} else {									// ���݂̉^�p�ʂ�2�̏ꍇ
//					FLT_write_wave_swdata(OPERATE_AREA1);
//					BootInfo.sw_flash.wave = OPERATE_AREA1;
//				}
//			}
//		}
//
//		if(BootInfo.sw.wave == BootInfo.sw_bakup.wave) {// ���݂̉^�p�ʃo�b�N�A�b�v�̉^�p�ʈ�v
//			if(BootInfo.sw.wave == OPERATE_AREA1) {		// ���݂̉^�p�ʂ�1�̏ꍇ
//				BootInfo.sw.wave = OPERATE_AREA2;
//			} else {									// ���݂̉^�p�ʂ�2�̏ꍇ
//				BootInfo.sw.wave = OPERATE_AREA1;
//			}
//		}
//	}
// MH810100(E) Y.Yamauchi 2020/03/05 �Ԕԃ`�P�b�g���X(�����e�i���X)
	memset(BootInfo.f_wave_update,0x00,sizeof(BootInfo.f_wave_update));// �X�V�t���O�N���A
}


//[]----------------------------------------------------------------------[]
///	@brief			���ʃp�����[�^�^�p�ʍX�V����
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void parm_data_swupdate(void)
{
	if((BootInfo.sw.parm != OPERATE_AREA1)&&(BootInfo.sw.parm != OPERATE_AREA2)) {
		memset(BootInfo.f_parm_update,0x00,sizeof(BootInfo.f_parm_update));// �O�̂��ߍX�V�t���O�N���A
		return;// �X�V�����͓���RAM�̖ʏ�񂪐���ł��邱�Ƃ��O��A�ُ펞�͍X�V���Ȃ�
	}
	
	if(0 == memcmp(BootInfo.f_parm_update,parm_update_flag,sizeof(BootInfo.f_parm_update))) {// �X�V�t���O�L
		if((BootInfo.sw_bakup.parm != OPERATE_AREA1)&&(BootInfo.sw_bakup.parm != OPERATE_AREA2)) {
			// �^�p�ʏ��o�b�N�A�b�v���Ȃ��ꍇ
			BootInfo.sw_bakup.parm = BootInfo.sw.parm;// FROM�X�V�r���œd���f���������l�����ĉ^�p�ʏ����o�b�N�A�b�v
		}
		if((BootInfo.sw_flash.parm != OPERATE_AREA1)&&(BootInfo.sw_flash.parm != OPERATE_AREA2)) {
			// �O��FROM�ɉ^�p�ʏ�񂪖���(FROM�X�V���̓d���f�̃P�[�X)
			if(BootInfo.sw.parm == OPERATE_AREA1) {		// ���݂̉^�p�ʂ�1�̏ꍇ
				FLT_write_parm_swdata(OPERATE_AREA2);
			} else {									// ���݂̉^�p�ʂ�2�̏ꍇ
				FLT_write_parm_swdata(OPERATE_AREA1);
			}
		} else {
			// �O��FROM�Ƀf�[�^�L
			if(BootInfo.sw_bakup.parm == BootInfo.sw_flash.parm) {
				// FROM�Ɖ^�p�ʏ��o�b�N�A�b�v����v�������͐��FROM�̍X�V���s�Ȃ�
				if(BootInfo.sw.parm == OPERATE_AREA1) {		// ���݂̉^�p�ʂ�1�̏ꍇ
					FLT_write_parm_swdata(OPERATE_AREA2);
				} else {									// ���݂̉^�p�ʂ�2�̏ꍇ
					FLT_write_parm_swdata(OPERATE_AREA1);
				}
			}
		}

		if(BootInfo.sw.parm == BootInfo.sw_bakup.parm) {// ���݂̉^�p�ʃo�b�N�A�b�v�̉^�p�ʈ�v
			if(BootInfo.sw.parm == OPERATE_AREA1) {		// ���݂̉^�p�ʂ�1�̏ꍇ
				BootInfo.sw.parm = OPERATE_AREA2;
			} else {									// ���݂̉^�p�ʂ�2�̏ꍇ
				BootInfo.sw.parm = OPERATE_AREA1;
			}
		}
		_flt_DestroyParamRamSum();						// RAM��̃p�����[�^�T�����N���A
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
		if(!remotedl_work_update_get()){
			// �蓮�Ŗʐؑւ̏ꍇ�̂�
			mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
		}
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
	}
	memset(BootInfo.f_parm_update,0x00,sizeof(BootInfo.f_parm_update));// �X�V�t���O�N���A
}


//[]----------------------------------------------------------------------[]
///	@brief			�����f�[�^�^�p�ʃ`�F�b�N����
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return         0:���� 
///                 1:FROM������Ȃ̂�FROM�ɍX�V���ĕ���
///                 2:�^�p�ʏ��ُ�(��1�Ƀf�[�^�L���^�p��1�ɍX�V���ĕ���)
///                 3:�^�p�ʏ��ُ�(��2�Ƀf�[�^�L���^�p��2�ɍX�V���ĕ���)
///                 4:�f�[�^�̎��̂����݂��Ȃ�(�C���X�g�[������Ă��Ȃ�)
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char wave_data_swchk(void)
{
// MH810100(S) Y.Yamauchi 2020/03/05 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	unsigned char ret;
//
//	if(( 0 != chk_wave_data_onfrom(OPERATE_AREA1,NULL)) && ( 0 != chk_wave_data_onfrom(OPERATE_AREA2,NULL)) ) {
//		// �f�[�^�̎��̂����݂��Ȃ�(����C���X�g�[�������Ă��Ȃ�)
//		ret = 4;
//	} else if((BootInfo.sw.wave == OPERATE_AREA1)||(BootInfo.sw.wave == OPERATE_AREA2)) {// SRAM�^�p�ʐ���
//		ret = 0;
//	} else if((BootInfo.sw_flash.wave == OPERATE_AREA1)||(BootInfo.sw_flash.wave == OPERATE_AREA2)) {
//		// FROM�^�p�ʂ�����Ȃ��ߍX�V
//		BootInfo.sw.wave = BootInfo.sw_flash.wave;
//		ret = 1;
//	} else if( 0 == chk_wave_data_onfrom(OPERATE_AREA1,NULL)) {
//		// �^�p��1�Ɏ��ۂɃf�[�^�L
//		FLT_write_wave_swdata(OPERATE_AREA1);
//		BootInfo.sw_flash.wave = OPERATE_AREA1;
//		BootInfo.sw.wave = OPERATE_AREA1;
//		ret = 2;
//	} else if( 0 == chk_wave_data_onfrom(OPERATE_AREA2,NULL)) {
//		// �^�p��2�Ɏ��ۂɃf�[�^�L
//		FLT_write_wave_swdata(OPERATE_AREA2);
//		BootInfo.sw_flash.wave = OPERATE_AREA2;
//		BootInfo.sw.wave = OPERATE_AREA2;
//		ret = 3;
//	} else {// �f�[�^�̎��̂����݂��Ȃ�
//		ret = 4;
//	}
//	if( BootInfo.sw.wave == OPERATE_AREA1 ){
//		SodiacSoundAddress = FROM1_SA27;
//	}else{
//		SodiacSoundAddress = FROM1_SA28;
//	}
//	memset( SOUND_VERSION, 0, sizeof( SOUND_VERSION ));
//	chk_wave_data_onfrom( BootInfo.sw.wave, SOUND_VERSION );
//	return ret;
	return 4;
// MH810100(E) Y.Yamauchi 2020/03/05 �Ԕԃ`�P�b�g���X(�����e�i���X)
}

//[]----------------------------------------------------------------------[]
///	@brief			���ʃp�����[�^�^�p�ʃ`�F�b�N����
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@return         0:���� 
///                 1:FROM������Ȃ̂�FROM�ɍX�V���ĕ���
///                 2:�^�p�ʏ��ُ���^�p��1�ɍX�V���ĕ���
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char parm_data_swchk(void)
{
	unsigned char ret;

	if((BootInfo.sw.parm == OPERATE_AREA1)||(BootInfo.sw.parm == OPERATE_AREA2)) {
		// SRAM�^�p�ʐ���
		ret = 0;
	} else if((BootInfo.sw_flash.parm == OPERATE_AREA1)||(BootInfo.sw_flash.parm == OPERATE_AREA2)) {
		// SRAM�ُ�ł��邪FROM�^�p�ʂ�����Ȃ��ߍX�V
		BootInfo.sw.parm = BootInfo.sw_flash.parm;
		ret = 1;
	} else {
		// �^�p�ʂ��ُ�ȏꍇ�͕K���f�[�^�����݂���^�p��1�ɋ����Z�b�g
		FLT_write_parm_swdata(OPERATE_AREA1);
		BootInfo.sw_flash.parm = OPERATE_AREA1;
		BootInfo.sw.parm = OPERATE_AREA1;
		ret = 2;
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�f�[�^�X�V�t���O�������ݏ���
//[]----------------------------------------------------------------------[]
///	@param[in]		update	: 0�r�b�g��ON�X�V �v���O���� 1�r�b�g��ON �����f�[�^�X�V 2�r�b�g��ON ���ʃp�����[�^�X�V
///	@return			void
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void update_flag_set(unsigned char update)
{
	if(PROGRAM_UPDATE_FLAG & update) {							// �v���O�����X�V�t���O�v���L
		memcpy(BootInfo.f_prog_update,prog_update_flag,sizeof(BootInfo.f_prog_update));
	}

	if(WAVEDATA_UPDATE_FLAG & update) {							// �����f�[�^�X�V�t���O�v���L
		BootInfo.sw_bakup.wave = 0;	// �^�p�ʏ��o�b�N�A�b�v(�X�V���̃��J�o���p)
		memcpy(BootInfo.f_wave_update,wave_update_flag,sizeof(BootInfo.f_wave_update));
	}

	if(PARAM_UPDATE_FLAG & update) {							// ���ʃp�����[�^�X�V�t���O�v���L
		BootInfo.sw_bakup.parm = 0;	// �^�p�ʏ��o�b�N�A�b�v(�X�V���̃��J�o���p)
		memcpy(BootInfo.f_parm_update,parm_update_flag,sizeof(BootInfo.f_parm_update));
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�f�[�^�X�V�t���O�N���A����
//[]----------------------------------------------------------------------[]
///	@param[in]		update	: 0�r�b�g��ON�X�V �v���O���� 1�r�b�g��ON �����f�[�^�X�V 2�r�b�g��ON ���ʃp�����[�^�X�V
///	@return			void
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void update_flag_clear(unsigned char update)
{
	if(PROGRAM_UPDATE_FLAG & update) {							// �v���O�����X�V�t���O�v���L
		memset(BootInfo.f_prog_update,0x00,sizeof(BootInfo.f_prog_update));
	}

	if(WAVEDATA_UPDATE_FLAG & update) {							// �����f�[�^�X�V�t���O�v���L
		memset(BootInfo.f_wave_update,0x00,sizeof(BootInfo.f_wave_update));
	}

	if(PARAM_UPDATE_FLAG & update) {							// ���ʃp�����[�^�X�V�t���O�v���L
		memset(BootInfo.f_parm_update,0x00,sizeof(BootInfo.f_parm_update));
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			�_�E�����[�h�����v���O�����f�[�^���O��FROM�Ɋi�[����Ă��邩�`�F�b�N����
//[]----------------------------------------------------------------------[]
///	@param[in]		*dat �o�[�W���������i�[���ė~�����|�C���^(�o�[�W�����f�[�^��8Byte)
///					     NULL���w�肵���ꍇ�ƃ`�F�b�N���ʂ��f�[�^�����̏ꍇ�͊i�[���Ȃ�
///	@return			0:�f�[�^�L 1:�f�[�^����
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char chk_prog_data_onfrom(unsigned char *dat)
{
	int i,j;
	int f_ok = 1;
	FLT_read_program_version(swdata_write_buf);
	// �����O�X�̈�̊m�F
	for(i = LENGTH_OFFSET; i<(LENGTH_OFFSET+LENGTH_SIZE);i++){
		if(swdata_write_buf[i] != 0xFF){
			f_ok = 0;//�f�[�^�L
			break;
		}
	}
	if(f_ok == 1){
		return 1;									// �u�����N�Ȃ̂Ńf�[�^�Ȃ�
	}
	// �`�F�b�N�T���̈�̊m�F
	if((0xFF == swdata_write_buf[SUM_OFFSET])&&(0xFF == swdata_write_buf[SUM_OFFSET+1])) {
		return 1;									// �u�����N�Ȃ̂Ńf�[�^�Ȃ�
	}
	// �o�[�W������ASCII�̕����R�[�h�͈͊O�Ȃ�L���f�[�^�Ȃ�
	for(i = VER_OFFSET; i<(VER_OFFSET+VER_SIZE);i++){
		if((swdata_write_buf[i] < 0x20 )||(swdata_write_buf[i] > 0x7A)) {
			return 1;// ASCII�̃X�y�[�X����'z'�͈̔͊O(�����R�[�h���ΏۊO)
		}
	}
	// �����ɗ������_�Ńf�[�^����
	if(dat != NULL){
		for(i = VER_OFFSET,j = 0; i<(VER_OFFSET+VER_SIZE); i++, j++){
			dat[j] = swdata_write_buf[i];
		}
	}
	return 0;// �f�[�^�L
}
