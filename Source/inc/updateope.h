#ifndef	_UPDATEOPE_H_
#define	_UPDATEOPE_H_
//[]----------------------------------------------------------------------[]
///	@file		update_ope.h
///	@brief		�^�p�ʊǗ������̃w�b�_�[
/// @date		2012/04/24
///	@author		A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]

/*--- Define --------------------*/
#define PROGRAM_UPDATE_FLAG            0x01
#define WAVEDATA_UPDATE_FLAG           0x02
#define PARAM_UPDATE_FLAG              0x04

#define OPERATE_AREA1                  0x5555// ��1
#define OPERATE_AREA2                  0xAAAA// ��2

#define LENGTH_OFFSET                  3
#define LENGTH_SIZE                    4

#define SUM_OFFSET                     7
#define VER_OFFSET                     9
#define VER_SIZE                       9
#define PARM_AREA_ERR                  2
#define WAVE_AREA_ERR_SETAREA1         2
#define WAVE_AREA_ERR_SETAREA2         3
#define WAVE_AREA_NODATA               4
/*--- Gloval Value --------------*/

extern unsigned char ver_datwk_prog[VER_SIZE];// �V�X�e�������e�i���X�ŕ\������_�E�����[�h�v���O�����o�[�W�����i�[�G���A
extern unsigned char ver_datwk_wave_ope[VER_SIZE];// �V�X�e�������e�i���X�ŕ\������_�E�����[�h�����o�[�W�����^�p�ʊi�[�G���A
extern unsigned char ver_datwk_wave_sus[VER_SIZE];// �V�X�e�������e�i���X�ŕ\������_�E�����[�h�����o�[�W�����ҋ@�ʊi�[�G���A

/*--- Extern --------------------*/
extern void update_sw_area_init(void);
extern void get_from_swdata(void);
extern unsigned char chk_wave_data_onfrom(unsigned short , unsigned char *);
extern void wave_data_swupdate(void);
extern void parm_data_swupdate(void);
extern unsigned char wave_data_swchk(void);
extern unsigned char parm_data_swchk(void);
extern void update_flag_set(unsigned char);
extern void update_flag_clear(unsigned char);
extern unsigned char chk_prog_data_onfrom(unsigned char *);

#endif
