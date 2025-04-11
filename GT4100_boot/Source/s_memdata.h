#ifndef _S_MEMDATA_
#define _S_MEMDATA_
//[]----------------------------------------------------------------------[]
///	@file		s_memdata.h
///	@brief		�u�[�g�̈惁������` �w�b�_�t�@�C��
/// @date		2012/03/29
///	@author		A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
typedef struct{
	unsigned short wave;// �����f�[�^
	unsigned short parm;// ���ʃp�����[�^
	unsigned short reserve1;
	unsigned short reserve2;
}SW;

// �ʐ؂�ւ��Z�N�V�������
typedef struct{
	unsigned char f_prog_update[4];			// �v���O�����X�V�t���O
	unsigned char f_wave_update[4];			// �����f�[�^�X�V�t���O
	unsigned char f_parm_update[4];			// ���ʃp�����[�^�X�V�t���O
	SW sw;									// �^�p�ʏ��
	SW sw_bakup;							// �^�p�ʏ��o�b�N�A�b�v(�X�V���̃��J�o���p)
	SW sw_flash;							// �t���b�V���ɏ����ꂽ�^�p�ʏ��
	unsigned short err_count;				// �u�[�g�v���O�����Ŕ��������G���[�̉�(�g�[�^��10���������畜���s�\�Ƃ���)
}SWITCH_DATA;

/*--- Define --------------------*/

/*--- Extern --------------------*/

extern const unsigned char prog_update_flag[4];
extern SWITCH_DATA	BootInfo;						// �N�����ʑI�����

#endif

