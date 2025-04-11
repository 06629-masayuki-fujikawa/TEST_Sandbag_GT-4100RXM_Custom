// MH810100(S) K.Onodera 2019/11/11 �Ԕԃ`�P�b�g���X(GT-8700(LZ-122601)���p)
/**
 *	@file	lcdbm_def.h
 *	@brief	LCD���W���[���ň����Ǘ��ԍ��̒�`
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@since	2009/11/10 10:31:26
 *
 *	@internal
 *	$Name:$
 *	$Author:$
 *	$Date::                            $
 *	$Revision:$
 *
 *		Copyright (c)  Amano Co.,Ltd. 2009
 *		Licensed Material of Amano Co.,Ltd.
 *
 */
#ifndef _LCDBM_DEF_H_
#define _LCDBM_DEF_H_

//--------------------------------------------------
//		�ʃ\�t�g�Ŏg�p����ԍ�
//--------------------------------------------------
/**
 *	@note	SD�J�[�h���ɕۑ�����e��f�[�^�̔ԍ��Ǘ�
 *
 *	�� ��{��ʃp�^�[���t�@�C�� ��
 *	�t�@�C�����iscr?????.ini�j�Ɏw�肷��10�i5���̐��l
 *	+---------------+--------------------------------------------------+
 *	|   �͈�        |    ���蓖��                                      |
 *	+===============+==================================================+
 *	| 00000 - 49999 | �W���\�t�g�Ŏg�p����                             |
 *	+---------------+--------------------------------------------------+
 *	| 50000 - 64999 | �ʃ\�t�g�Ŏg�p����                             |
 *	+---------------+--------------------------------------------------+
 *	| 65000 - 65535 | �J���e�X�g�p�i���i�łɂ͎������Ȃ��j             |
 *	+---------------+--------------------------------------------------+
 *
 *	�� �O���t�B�b�N�p�^�[���t�@�C�� ��
 *	GrphPtn.ini�t�@�C�����̃p�^�[���ԍ��i[Image?????]�j�Ŏw�肷��10�i5���̐��l
 *	+---------------+--------------------------------------------------+
 *	|   �͈�        |    ���蓖��                                      |
 *	+===============+==================================================+
 *	| 00000 - 49999 | �W���\�t�g�Ŏg�p����                             |
 *	+---------------+--------------------------------------------------+
 *	| 50000 - 64999 | �ʃ\�t�g�Ŏg�p����                             |
 *	+---------------+--------------------------------------------------+
 *	| 65000 - 65535 | �J���e�X�g�p�i���i�łɂ͎������Ȃ��j             |
 *	+---------------+--------------------------------------------------+
 *
 *	�� �摜�E����t�@�C�� ��
 *	�t�@�C�����iimg?????.bmp, img?????.jpg, img?????.3g2�j�Ɏw�肷��10�i5���̐��l
 *	*.bmp�F�r�b�g�}�b�v�`���t�@�C��
 *	*.jpg�FJPEG�t�@�C��
 *	*.3g2�F����t�@�C��
 *	+---------------+--------------------------------------------------+
 *	|   �͈�        |    ���蓖��                                      |
 *	+===============+==================================================+
 *	| 00000 - 49999 | �W���\�t�g�Ŏg�p����                             |
 *	+---------------+--------------------------------------------------+
 *	| 50000 - 64999 | �ʃ\�t�g�Ŏg�p����                             |
 *	+---------------+--------------------------------------------------+
 *	| 65000 - 65535 | �J���e�X�g�p�i���i�łɂ͎������Ȃ��j             |
 *	+---------------+--------------------------------------------------+
 *
 *	�� �e�L�X�g�p�^�[���t�@�C�� ��
 *	TextPtn.ini�t�@�C�����̃p�^�[���ԍ��i[TextPtn?????]�j�Ŏw�肷��10�i5���̐��l
 *	+---------------+--------------------------------------------------+
 *	|   �͈�        |    ���蓖��                                      |
 *	+===============+==================================================+
 *	| 00000 - 49999 | �W���\�t�g�Ŏg�p����                             |
 *	+---------------+--------------------------------------------------+
 *	| 50000 - 64999 | �ʃ\�t�g�Ŏg�p����                             |
 *	+---------------+--------------------------------------------------+
 *	| 65000 - 65535 | �J���e�X�g�p�i���i�łɂ͎������Ȃ��j             |
 *	+---------------+--------------------------------------------------+
 *
 *	�� �e�L�X�g�f�[�^��
 *	TextMsg.txt�t�@�C�����Ńe�L�X�g�ԍ����w�肷��s����10�i5���̐��l
 *	+---------------+--------------------------------------------------+
 *	|   �͈�        |    ���蓖��                                      |
 *	+===============+==================================================+
 *	| 00000 - 49999 | �W���\�t�g�Ŏg�p����                             |
 *	+---------------+--------------------------------------------------+
 *	| 50000 - 64999 | �ʃ\�t�g�Ŏg�p����                             |
 *	+---------------+--------------------------------------------------+
 *	| 65000 - 65535 | �J���e�X�g�p�i���i�łɂ͎������Ȃ��j             |
 *	+---------------+--------------------------------------------------+
 *
 *	�� �|�b�v�A�b�v�p�^�[���t�@�C�� ��
 *	�t�@�C�����ipop?????.ini�j�Ɏw�肷��10�i5���̐��l
 *	+---------------+--------------------------------------------------+
 *	|   �͈�        |    ���蓖��                                      |
 *	+===============+==================================================+
 *	| 00000 - 49999 | �W���\�t�g�Ŏg�p����                             |
 *	+---------------+--------------------------------------------------+
 *	| 50000 - 64999 | �ʃ\�t�g�Ŏg�p����                             |
 *	+---------------+--------------------------------------------------+
 *	| 65000 - 65535 | �J���e�X�g�p�i���i�łɂ͎������Ȃ��j             |
 *	+---------------+--------------------------------------------------+
 *
 *	�� �����f�[�^�t�@�C�� ��
 *	�t�@�C�����iphr?????.wav�j�Ɏw�肷��10�i5���̐��l
 *	+---------------+--------------------------------------------------+
 *	|   �͈�        |    ���蓖��                                      |
 *	+===============+==================================================+
 *	| 00000 - 49999 | �W���\�t�g�Ŏg�p����                             |
 *	+---------------+--------------------------------------------------+
 *	| 50000 - 64999 | �ʃ\�t�g�Ŏg�p����                             |
 *	+---------------+--------------------------------------------------+
 *	| 65000 - 65535 | �J���e�X�g�p�i���i�łɂ͎������Ȃ��j             |
 *	+---------------+--------------------------------------------------+
 *
 *	�� ���ߔԍ� ��
 *	AnnTbl.ini���ŕ��ߔԍ����w�肷��10�i3���̐��l
 *	+---------------+--------------------------------------------------+
 *	|   �͈�        |    ���蓖��                                      |
 *	+===============+==================================================+
 *	|   001 -   799 | �W���\�t�g�Ŏg�p����                             |
 *	+---------------+--------------------------------------------------+
 *	|   800 -   999 | �ʃ\�t�g�Ŏg�p����                             |
 *	+---------------+--------------------------------------------------+
 *
 */


//--------------------------------------------------
//		��{��ʔԍ�
//--------------------------------------------------
/**
 *	��{��ʔԍ�
 */
enum {
	LCDBM_IMAGE_BASE__WAITING_CPS =	20000,						// ���O���Z�@�p �ҋ@���
	LCDBM_IMAGE_BASE__WAITING_EPS =	20100,						// �o�����Z�@�p �ҋ@���
	LCDBM_IMAGE_BASE__PAYING =		20200,						// �����ē��p���
	LCDBM_IMAGE_BASE__PAY_SATRT =	LCDBM_IMAGE_BASE__PAYING,	// ���Ԍ��}���ē�
	LCDBM_IMAGE_BASE__PAID =		LCDBM_IMAGE_BASE__PAYING,	// ���Z����
	LCDBM_IMAGE_BASE__PAY_CANCEL =	20270,						// �Ƃ肯�����
	LCDBM_IMAGE_BASE__EXIT =		20400,						// �o�ɒ��x�[�X���
	LCDBM_IMAGE_BASE__BREAK =		20500,						// �x�ƒ��x�[�X���
	LCDBM_IMAGE_BASE__WARNING =		20600,						// �x���x�[�X���
	LCDBM_IMAGE_BASE__MAINTENANCE =	20700,						// �����e�i���X���
	LCDBM_IMAGE_BASE__REFILL =		20800,						// �ޑK��[�x�[�X���
	LCDBM_IMAGE_BASE__MANUAL =		LCDBM_IMAGE_BASE__WARNING,	// �蓮���Z���
};


//--------------------------------------------------
//		�����e�i���X �摜�ԍ�
//--------------------------------------------------
enum {
	LCDBM_IMAGE_MAINTENANCE__BASE =		LCDBM_IMAGE_BASE__BREAK,		// �x�ƒ��Ƌ��p
	LCDBM_IMAGE_MAINTENANCE__LUMINE =	LCDBM_IMAGE_BASE__MAINTENANCE,	// �P�x����(�J���[�p�^�[��)
// LH068005 sekiguchi(S) 2010/03/25 LCD���W���[���Ή� ����`�F�b�N
	LCDBM_IMAGE_MAINTENANCE__YELLOW,									// ���F
	LCDBM_IMAGE_MAINTENANCE__RED,										// �ԐF
	LCDBM_IMAGE_MAINTENANCE__BLUE,										// �F
	LCDBM_IMAGE_MAINTENANCE__BLACK,										// ���F
	LCDBM_IMAGE_MAINTENANCE__GREEN,										// �ΐF
	LCDBM_IMAGE_MAINTENANCE__WHITE,										// ���F
	LCDBM_IMAGE_MAINTENANCE__PATTERN,									// �p�^�[��
// LH068005 sekiguchi(E)
};


//--------------------------------------------------
//		�ޑK��[ �摜�ԍ�
//--------------------------------------------------
/**
 *	�ޑK��[�̕��i�ԍ�
 *	@note
 *	-	�ޑK��[���ɕ\������摜�̒�`
 */
enum {
	// �@��ʔw�i
	LCDBM_IMAGE_REFILL__BASE_TYPE1 =	LCDBM_IMAGE_BASE__REFILL,	// �������o�@�Ȃ�(���K�Ǘ�����)
	LCDBM_IMAGE_REFILL__BASE_TYPE2,									// �������o�@����(���K�Ǘ�����)
	LCDBM_IMAGE_REFILL__BASE_TYPE3,									// �����z�@����(���K�Ǘ�����)
	LCDBM_IMAGE_REFILL__BASE_TYPE4,									// �������o�@�Ȃ�(���K�Ǘ��Ȃ�)
	LCDBM_IMAGE_REFILL__BASE_TYPE5,									// �������o�@����(���K�Ǘ��Ȃ�)
	LCDBM_IMAGE_REFILL__BASE_TYPE6,									// �����z�@����(���K�Ǘ��Ȃ�)

	// �R�C�����b�N
	LCDBM_IMAGE_REFILL__COINMEC_500,			// �R�C�����b�N500�~��
	LCDBM_IMAGE_REFILL__COINMEC_500_PTN1,
	LCDBM_IMAGE_REFILL__COINMEC_500_PTN2,
	LCDBM_IMAGE_REFILL__COINMEC_500_PTN3,
	LCDBM_IMAGE_REFILL__COINMEC_100,			// �R�C�����b�N100�~��
	LCDBM_IMAGE_REFILL__COINMEC_100_PTN1,
	LCDBM_IMAGE_REFILL__COINMEC_100_PTN2,
	LCDBM_IMAGE_REFILL__COINMEC_100_PTN3,
	LCDBM_IMAGE_REFILL__COINMEC_10,				// �R�C�����b�N10�~��
	LCDBM_IMAGE_REFILL__COINMEC_10_PTN1,
	LCDBM_IMAGE_REFILL__COINMEC_10_PTN2,
	LCDBM_IMAGE_REFILL__COINMEC_10_PTN3,
	LCDBM_IMAGE_REFILL__COINMEC_50,				// �R�C�����b�N50�~��
	LCDBM_IMAGE_REFILL__COINMEC_50_PTN1,
	LCDBM_IMAGE_REFILL__COINMEC_50_PTN2,
	LCDBM_IMAGE_REFILL__COINMEC_50_PTN3,
	LCDBM_IMAGE_REFILL__COINMEC_S_INVALID,		// �R�C�����b�NSUB��(���g�p)
	LCDBM_IMAGE_REFILL__COINMEC_S10,			// �R�C�����b�NSUB��(10�~)
	LCDBM_IMAGE_REFILL__COINMEC_S10_PTN1,
	LCDBM_IMAGE_REFILL__COINMEC_S100,			// �R�C�����b�NSUB��(100�~)
	LCDBM_IMAGE_REFILL__COINMEC_S100_PTN1,

	// �R�C���z�b�p�[
	LCDBM_IMAGE_REFILL__HOPPER1_INVALID,		// �R�C���z�b�p�[1(���g�p)
	LCDBM_IMAGE_REFILL__HOPPER1_10,				// �R�C���z�b�p�[1(10�~)
	LCDBM_IMAGE_REFILL__HOPPER1_10_PTN1,
	LCDBM_IMAGE_REFILL__HOPPER1_50,				// �R�C���z�b�p�[1(50�~)
	LCDBM_IMAGE_REFILL__HOPPER1_50_PTN1,
	LCDBM_IMAGE_REFILL__HOPPER1_100,			// �R�C���z�b�p�[1(100�~)
	LCDBM_IMAGE_REFILL__HOPPER1_100_PTN1,
	LCDBM_IMAGE_REFILL__HOPPER1_500,			// �R�C���z�b�p�[1(500�~)
	LCDBM_IMAGE_REFILL__HOPPER1_500_PTN1,
	LCDBM_IMAGE_REFILL__HOPPER2_INVALID,		// �R�C���z�b�p�[2(���g�p)
	LCDBM_IMAGE_REFILL__HOPPER2_10,				// �R�C���z�b�p�[2(10�~)
	LCDBM_IMAGE_REFILL__HOPPER2_10_PTN1,
	LCDBM_IMAGE_REFILL__HOPPER2_50,				// �R�C���z�b�p�[2(50�~)
	LCDBM_IMAGE_REFILL__HOPPER2_50_PTN1,
	LCDBM_IMAGE_REFILL__HOPPER2_100,			// �R�C���z�b�p�[2(100�~)
	LCDBM_IMAGE_REFILL__HOPPER2_100_PTN1,
	LCDBM_IMAGE_REFILL__HOPPER2_500,			// �R�C���z�b�p�[2(500�~)
	LCDBM_IMAGE_REFILL__HOPPER2_500_PTN1,

	// �������o�@
	LCDBM_IMAGE_REFILL__CASSETTE1_INVALID,		// �������o�J�Z�b�g1(���g�p)
	LCDBM_IMAGE_REFILL__CASSETTE1_1000,			// �������o�J�Z�b�g1(1000�~)
	LCDBM_IMAGE_REFILL__CASSETTE1_1000_PTN1,
	LCDBM_IMAGE_REFILL__CASSETTE1_2000,			// �������o�J�Z�b�g1(2000�~)
	LCDBM_IMAGE_REFILL__CASSETTE1_2000_PTN1,
	LCDBM_IMAGE_REFILL__CASSETTE1_5000,			// �������o�J�Z�b�g1(5000�~)
	LCDBM_IMAGE_REFILL__CASSETTE1_5000_PTN1,
	LCDBM_IMAGE_REFILL__CASSETTE2_INVALID,		// �������o�J�Z�b�g2(���g�p)
	LCDBM_IMAGE_REFILL__CASSETTE2_1000,			// �������o�J�Z�b�g2(1000�~)
	LCDBM_IMAGE_REFILL__CASSETTE2_1000_PTN1,
	LCDBM_IMAGE_REFILL__CASSETTE2_2000,			// �������o�J�Z�b�g2(2000�~)
	LCDBM_IMAGE_REFILL__CASSETTE2_2000_PTN1,
	LCDBM_IMAGE_REFILL__CASSETTE2_5000,			// �������o�J�Z�b�g2(5000�~)
	LCDBM_IMAGE_REFILL__CASSETTE2_5000_PTN1,

	// �����z�@
	LCDBM_IMAGE_REFILL__NTCYCL_1000,			// �����z�@1000�~���[��
	LCDBM_IMAGE_REFILL__NTCYCL_1000_PTN1,

	// �d�ݓ�����
	LCDBM_IMAGE_REFILL__COIN_ANIME_01,			// �R�C�������A�j���[�V����
	LCDBM_IMAGE_REFILL__COIN_ANIME_02,
	LCDBM_IMAGE_REFILL__COIN_ANIME_03,
	LCDBM_IMAGE_REFILL__COIN_ANIME_04,
	LCDBM_IMAGE_REFILL__COIN_ANIME_05,
	LCDBM_IMAGE_REFILL__COIN_ANIME_06,
	LCDBM_IMAGE_REFILL__COIN_ANIME_07,
	LCDBM_IMAGE_REFILL__COIN_ANIME_08,
	LCDBM_IMAGE_REFILL__COIN_ANIME_09,
	LCDBM_IMAGE_REFILL__COIN_ANIME_10,
	LCDBM_IMAGE_REFILL__COIN_ANIME_11,
	LCDBM_IMAGE_REFILL__COIN_ANIME_12,
	LCDBM_IMAGE_REFILL__COIN_ANIME_13,
	LCDBM_IMAGE_REFILL__COIN_ANIME_14,
	LCDBM_IMAGE_REFILL__COIN_ANIME_15,
	LCDBM_IMAGE_REFILL__COIN_ANIME_16,

	LCDBM_IMAGE_REFILL__DUMMY_77,
	LCDBM_IMAGE_REFILL__DUMMY_78,
	LCDBM_IMAGE_REFILL__DUMMY_79,
	LCDBM_IMAGE_REFILL__DUMMY_80,

	// ����������
	LCDBM_IMAGE_REFILL__NOTE_ANIME_01,			// �����}���A�j���[�V����
	LCDBM_IMAGE_REFILL__NOTE_ANIME_02,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_03,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_04,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_05,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_06,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_07,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_08,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_09,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_10,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_11,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_12,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_13,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_14,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_15,
	LCDBM_IMAGE_REFILL__NOTE_ANIME_16,
};


/**
 *	�ޑK���ɂ̏�ԃp�^�[��
 */
enum {
	LCDBM_IMAGE_REFILL_NO__NORMAL,			//  0: ��[�\���K�薇����
	LCDBM_IMAGE_REFILL_NO__EXCEED,			//  1: ��[�\����������(�����ނ荇�킹�̑Ώ�)
	LCDBM_IMAGE_REFILL_NO__STOP,			//  2: ��[�s��
};


//--------------------------------------------------
//		���ԍ�
//--------------------------------------------------
/**
 *	�ҋ@��ʁiCPS�AEPS�j �̃e�L�X�g���ԍ�
 *
 *	@code
 *		+-------------------------------------------------------+
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		+-----------------------+-------+-----------------------+
 *		|                       |(Text5)|      ���v(Text6)      |  <-- ��؂�\���iText5+Text6�̗̈�j
 *		+-----------------------+-------+-----------------------+
 *	@endcode
 */
// ���O���Z�@�iCPS�j
enum {
	LCDBM_AREA_TEXT__WAITING_CPS_00,		//  0: ���g�p
	LCDBM_AREA_TEXT__WAITING_CPS_01,		//  1: ���g�p
	LCDBM_AREA_TEXT__WAITING_CPS_02,		//  2: ���g�p
	LCDBM_AREA_TEXT__WAITING_CPS_03,		//  3: ���g�p
	LCDBM_AREA_TEXT__WAITING_CPS_04,		//  4: ���g�p
	LCDBM_AREA_TEXT__WAITING_CPS_ALARM,		//  5: [24x24]	��؂ꓙ�̃A���[��
	LCDBM_AREA_TEXT__WAITING_CPS_CLOCK,		//  6: [24x24]	���v
};
// �o�����Z�@�iEPS�j
enum {
	LCDBM_AREA_TEXT__WAITING_EPS_00,		//  0: ���g�p
	LCDBM_AREA_TEXT__WAITING_EPS_01,		//  1: ���g�p
	LCDBM_AREA_TEXT__WAITING_EPS_02,		//  2: ���g�p
	LCDBM_AREA_TEXT__WAITING_EPS_03,		//  3: ���g�p
	LCDBM_AREA_TEXT__WAITING_EPS_04,		//  4: ���g�p
	LCDBM_AREA_TEXT__WAITING_EPS_ALARM,		//  5: [24x24]	��؂ꓙ�̃A���[��
	LCDBM_AREA_TEXT__WAITING_EPS_CLOCK,		//  6: [24x24]	���v
};


/**
 *	�ҋ@��ʁiCPS�AEPS�j �̃O���t�B�b�N���ԍ�
 *
 *	@code
 *		+-------------------------------------------------------+
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                 �A�j���[�V�����̈�                    |
 *		|                      (Graphic8)                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		|                                                       |
 *		+-------------------------------------------------------+
 *		|                 �Î~��̈�(Graphic2)                  |
 *		+-------------------------------------------------------+
 *	@endcode
 */
// ���O���Z�@�iCPS�j
enum {
	LCDBM_AREA_GRAPHIC__WAITING_CPS_00,			// 0: ���g�p
	LCDBM_AREA_GRAPHIC__WAITING_CPS_01,			// 1: ���g�p
	LCDBM_AREA_GRAPHIC__WAITING_CPS_CAPTION_3,	// 2: �ŉ��i�{���v
	LCDBM_AREA_GRAPHIC__WAITING_CPS_03,			// 3: ���g�p
	LCDBM_AREA_GRAPHIC__WAITING_CPS_04,			// 4: ���g�p
	LCDBM_AREA_GRAPHIC__WAITING_CPS_05,			// 5: ���g�p
	LCDBM_AREA_GRAPHIC__WAITING_CPS_06,			// 6: ���g�p
	LCDBM_AREA_GRAPHIC__WAITING_CPS_07,			// 7: ���g�p
	LCDBM_AREA_GRAPHIC__WAITING_CPS_ANIME_1,	// 8: �A�j���[�V�����̈�
};
// �o�����Z�@�iEPS�j
enum {
	LCDBM_AREA_GRAPHIC__WAITING_EPS_00,			// 0: ���g�p
	LCDBM_AREA_GRAPHIC__WAITING_EPS_01,			// 1: ���g�p
	LCDBM_AREA_GRAPHIC__WAITING_EPS_CAPTION_3,	// 2: �ŉ��i�{���v
	LCDBM_AREA_GRAPHIC__WAITING_EPS_03,			// 3: ���g�p
	LCDBM_AREA_GRAPHIC__WAITING_EPS_04,			// 4: ���g�p
	LCDBM_AREA_GRAPHIC__WAITING_EPS_05,			// 5: ���g�p
	LCDBM_AREA_GRAPHIC__WAITING_EPS_06,			// 6: ���g�p
	LCDBM_AREA_GRAPHIC__WAITING_EPS_07,			// 7: ���g�p
	LCDBM_AREA_GRAPHIC__WAITING_EPS_ANIME_1,	// 8: �A�j���[�V�����̈�
};


/**
 *	���Ԍ��}���ē��A�����ē��p�x�[�X��� �̃e�L�X�g���ԍ�
 *
 *	@code
 *		+-------------------------------------------------------+
 *		|                  �匩�o���̈�(Text1)                  |	<-- ���z�\���̈�(Text2)
 *		|                                                       |	<-- 32x32�̑匩�o��(Text8)
 *		+-------------------------------------------------------+
 *		|                    �����o��(Text3)                    |
 *		+-----------------------+-------------------------------+
 *		|                       |     ���b�Z�[�W�̈�(Text10)    |  <-- ����1�iText14�j
 *		|                       +-------------------------------+  <-- ����2�iText15�j
 *		|                       |     ���b�Z�[�W�̈�(Text11)    |  <-- ����3�iText16�j
 *		|                       +-------------------------------+  <-- ����4�iText17�j
 *		|                       |     ���b�Z�[�W�̈�(Text12)    |  <-- ����5�iText18�j
 *		|                       +-------------------------------+  <-- ����6�iText19�j
 *		|                       |     ���b�Z�[�W�̈�(Text13)    |  <-- ����7�iText20�j
 *		|                       +-------------------------------+  <-- ����8�iText21�j
 *		|                       |                               |  <-- ����9�iText22�j
 *		|                       |                               |
 *		|                       |                               |
 *		|                       |                               |
 *		|                       |                               |
 *		+-----------------------+-------+-----------------------+
 *		|   ���Ԏ��� (Text7)    |(Text9)|      ���v(Text6)      |  <-- ��؂�\���iText5=Text9+Text6�̗̈�j
 *		+-----------------------+-------+-----------------------+  <-- Text9�̈ʒu�͉��i2010/01/14�j
 *		|                     �ŉ��i(Text4)                     |
 *		+-------------------------------------------------------+
 *	@endcode
 */
enum {
	LCDBM_AREA_TEXT__PAYING_00,			//  0: ���g�p
	LCDBM_AREA_TEXT__PAYING_CAPTION_1,	//  1: [64x64]	�匩�o��
	LCDBM_AREA_TEXT__PAYING_FEE,		//  2: [64x64]	�����\��
	LCDBM_AREA_TEXT__PAYING_CAPTION_2,	//  3: [32x32]	�����o��
	LCDBM_AREA_TEXT__PAYING_CAPTION_3,	//  4: [24x24]	�����o��
	LCDBM_AREA_TEXT__PAYING_ALARM,		//  5: [24x24]	��؂�
	LCDBM_AREA_TEXT__PAYING_CLOCK,		//  6: [24x24]	���v
	LCDBM_AREA_TEXT__PAYING_TIME,		//  7: [24x24]	���Ɏ����^���Ԏ���
	LCDBM_AREA_TEXT__PAYING_CAPTION_4,	//  8: [36x36]	�匩�o��
	LCDBM_AREA_TEXT__PAYING_09,			//  9: ���g�p

	// ���W���[���A�j��
	LCDBM_AREA_TEXT__ANIME_COMMENT_1,	// 10: [24x24]	���b�Z�[�W�i�P�s�ځj
	LCDBM_AREA_TEXT__ANIME_COMMENT_2,	// 11: [24x24]	���b�Z�[�W�i�Q�s�ځj
	LCDBM_AREA_TEXT__ANIME_COMMENT_3,	// 12: [24x24]	���b�Z�[�W�i�R�s�ځj
	LCDBM_AREA_TEXT__ANIME_COMMENT_4,	// 13: [24x24]	���b�Z�[�W�i�S�s�ځj

	// ��������
	LCDBM_AREA_TEXT__PAYING_DETAILS_1,	// 14: [24x24]	����1�i����1-9�͘A�Ԃ̕K�v����j
	LCDBM_AREA_TEXT__PAYING_DETAILS_2,	// 15: [24x24]	����2
	LCDBM_AREA_TEXT__PAYING_DETAILS_3,	// 16: [24x24]	����3
	LCDBM_AREA_TEXT__PAYING_DETAILS_4,	// 17: [24x24]	����4
	LCDBM_AREA_TEXT__PAYING_DETAILS_5,	// 18: [24x24]	����5
	LCDBM_AREA_TEXT__PAYING_DETAILS_6,	// 19: [24x24]	����6
	LCDBM_AREA_TEXT__PAYING_DETAILS_7,	// 20: [24x24]	����7
	LCDBM_AREA_TEXT__PAYING_DETAILS_8,	// 21: [24x24]	����8
	LCDBM_AREA_TEXT__PAYING_DETAILS_9,	// 22: [24x24]	����9

	// ���ו\���p�̍s��
	LCDBM_AREA_TEXT__PAYING_DETAILS_NUM =	9,
};


/**
 *	���Ԍ��}���ē��A�����ē��p�x�[�X��� �̃O���t�B�b�N���ԍ�
 *
 *	@code
 *		+-------------------------------------------------------+
 *		|                                                       |
 *		|                 �Î~��̈�(Graphic1)                  |
 *		|                                                       |
 *		+-----------------------+-------------------------------+
 *		|                       |                               |	<-- ��������̈�(Graphic6) ��Graphic4��Graphic5�����킹���̈�
 *		|                       |     �Î~��̈�(Graphic4)      |
 *		|                       |                               |
 *		|  �A�j���[�V�����̈�P +-------------------------------+
 *		|      (Graphic3)       |                               |
 *		|                       |     �A�j���[�V�����̈�Q      |
 *		|                       |         (Graphic5)            |
 *		|                       |                               |
 *		|                       |                               |
 *		+-----------------------+-------------------------------+
 *		|                 �Î~��̈�(Graphic2)                  |
 *		|                                                       |
 *		+-------------------------------------------------------+
 *	@endcode
 */
enum {
	LCDBM_AREA_GRAPHIC__PAYING_00,				// 0: ���g�p
	LCDBM_AREA_GRAPHIC__PAYING_CAPTION_1,		// 1: �匩�o���{�����o��
	LCDBM_AREA_GRAPHIC__PAYING_CAPTION_3,		// 2: �ŉ��i�{���v
	LCDBM_AREA_GRAPHIC__PAYING_ANIME_1,			// 3: �A�j���[�V�����̈�P
	LCDBM_AREA_GRAPHIC__PAYING_ANIME_COMMENT,	// 4: �A�j���[�V�����̈�Q�ɘA���������b�Z�[�W
	LCDBM_AREA_GRAPHIC__PAYING_ANIME_2,			// 5: �A�j���[�V�����̈�Q
	LCDBM_AREA_GRAPHIC__PAYING_DETAIL,			// 6: ��������
	LCDBM_AREA_GRAPHIC__PAYING_GOODBYE,			// 7: ���Z������̏o�ɓ��A�j���[�V����
};


/**
 *	���Z������ʂ̃e�L�X�g���ԍ�
 *	�i���Z����ʂƓ����������ߕs�v�����A������K�v���������ꍇ�̔����Ďc���Ă����j
 */
enum {
	LCDBM_AREA_TEXT__PAID_00 =			LCDBM_AREA_TEXT__PAYING_00,			//  0: ���g�p
	LCDBM_AREA_TEXT__PAID_CAPTION_1 =	LCDBM_AREA_TEXT__PAYING_CAPTION_1,	//  1: [64x64]	�匩�o��
	LCDBM_AREA_TEXT__PAID_FEE =			LCDBM_AREA_TEXT__PAYING_FEE,		//  2: [64x64]	�����\��
	LCDBM_AREA_TEXT__PAID_CAPTION_2 =	LCDBM_AREA_TEXT__PAYING_CAPTION_2,	//  3: [32x32]	�����o��
	LCDBM_AREA_TEXT__PAID_CAPTION_3 =	LCDBM_AREA_TEXT__PAYING_CAPTION_3,	//  4: [24x24]	�����o��
	LCDBM_AREA_TEXT__PAID_ALARM =		LCDBM_AREA_TEXT__PAYING_ALARM,		//  5: [24x24]	��؂�
	LCDBM_AREA_TEXT__PAID_CLOCK =		LCDBM_AREA_TEXT__PAYING_CLOCK,		//  6: [24x24]	���v
	LCDBM_AREA_TEXT__PAID_TIME =		LCDBM_AREA_TEXT__PAYING_TIME,		//  7: [24x24]	���Ɏ����^���Ԏ���
	LCDBM_AREA_TEXT__PAID_CAPTION_4 =	LCDBM_AREA_TEXT__PAYING_CAPTION_4,	//  8: [36x36]	�匩�o��
	LCDBM_AREA_TEXT__PAID_09 =			LCDBM_AREA_TEXT__PAYING_09,			//  9: ���g�p
	LCDBM_AREA_TEXT__PAID_10 =			10,									// 10: ���g�p
	LCDBM_AREA_TEXT__PAID_11 =			11,									// 11: ���g�p
	LCDBM_AREA_TEXT__PAID_12 =			12,									// 12: ���g�p
	LCDBM_AREA_TEXT__PAID_13 =			13,									// 13: ���g�p

	// ��������
	LCDBM_AREA_TEXT__PAID_DETAILS_1 =	LCDBM_AREA_TEXT__PAYING_DETAILS_1,	// 14: [24x24]	����1�i����1-9�͘A�Ԃ̕K�v����j
	LCDBM_AREA_TEXT__PAID_DETAILS_2 =	LCDBM_AREA_TEXT__PAYING_DETAILS_2,	// 15: [24x24]	����2
	LCDBM_AREA_TEXT__PAID_DETAILS_3 =	LCDBM_AREA_TEXT__PAYING_DETAILS_3,	// 16: [24x24]	����3
	LCDBM_AREA_TEXT__PAID_DETAILS_4 =	LCDBM_AREA_TEXT__PAYING_DETAILS_4,	// 17: [24x24]	����4
	LCDBM_AREA_TEXT__PAID_DETAILS_5 =	LCDBM_AREA_TEXT__PAYING_DETAILS_5,	// 18: [24x24]	����5
	LCDBM_AREA_TEXT__PAID_DETAILS_6 =	LCDBM_AREA_TEXT__PAYING_DETAILS_6,	// 19: [24x24]	����6
	LCDBM_AREA_TEXT__PAID_DETAILS_7 =	LCDBM_AREA_TEXT__PAYING_DETAILS_7,	// 20: [24x24]	����7
	LCDBM_AREA_TEXT__PAID_DETAILS_8 =	LCDBM_AREA_TEXT__PAYING_DETAILS_8,	// 21: [24x24]	����8
	LCDBM_AREA_TEXT__PAID_DETAILS_9 =	LCDBM_AREA_TEXT__PAYING_DETAILS_9,	// 22: [24x24]	����9
};


/**
 *	���Z������ʂ̃O���t�B�b�N���ԍ�
 *	�i���Z����ʂƓ����������ߕs�v�����A������K�v���������ꍇ�̔����Ďc���Ă����j
 */
enum {
	LCDBM_AREA_GRAPHIC__PAID_00 =				LCDBM_AREA_GRAPHIC__PAYING_00,				// 0: ���g�p
	LCDBM_AREA_GRAPHIC__PAID_CAPTION_1 =		LCDBM_AREA_GRAPHIC__PAYING_CAPTION_1,		// 1: �匩�o���{�����o��
	LCDBM_AREA_GRAPHIC__PAID_CAPTION_3 =		LCDBM_AREA_GRAPHIC__PAYING_CAPTION_3,		// 2: �ŉ��i�{���v
	LCDBM_AREA_GRAPHIC__PAID_ANIME_1 =			LCDBM_AREA_GRAPHIC__PAYING_ANIME_1,			// 3: �S�ʃp�l���A�j���[�V����
	LCDBM_AREA_GRAPHIC__PAID_ANIME_COMMENT =	LCDBM_AREA_GRAPHIC__PAYING_ANIME_COMMENT,	// 4: ���g�p
	LCDBM_AREA_GRAPHIC__PAID_ANIME_2 =			LCDBM_AREA_GRAPHIC__PAYING_ANIME_2,			// 5: ���g�p
	LCDBM_AREA_GRAPHIC__PAID_DETAIL =			LCDBM_AREA_GRAPHIC__PAYING_DETAIL,			// 6: ���p����
	LCDBM_AREA_GRAPHIC__PAID_GOODBYE =			LCDBM_AREA_GRAPHIC__PAYING_GOODBYE,			// 7: ���Z������̏o�ɓ��A�j���[�V����
};


/**
 *	�x�ƒ���ʂ̃e�L�X�g���ԍ�
 *
 *	@code
 *	����������������������������������������
 *	��        [  TextArea01  ][TextArea02]��
 *	��                                    ��
 *	��[            TextArea03            ]��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��[      TextArea05      ][TextArea06]��
 *	����������������������������������������
 *	@endcode
 */
enum {
	LCDBM_AREA_TEXT__BREAK_00,	// 00: ���g�p
	LCDBM_AREA_TEXT__BREAK_01,	// 01: �匩�o��			[64x64]
	LCDBM_AREA_TEXT__BREAK_02,	// 02: �x�Ɨ��R
	LCDBM_AREA_TEXT__BREAK_03,	// 03: �����o��			[32x32]
	LCDBM_AREA_TEXT__BREAK_04,	// 04: ���g�p
	LCDBM_AREA_TEXT__BREAK_05,	// 05: �C���W�P�[�^�[	[24x24]
	LCDBM_AREA_TEXT__BREAK_06,	// 06: ���v				[24x24]
};


/**
 *	�x�ƒ���ʂ̃O���t�B�b�N���ԍ�
 *
 *	@code
 *	����������������������������������������
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��           GraphicArea01            ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	����������������������������������������
 *	@endcode
 */
enum {
	/*
	 *	������"_NOT_UPDATE"���t���Ă����`���̓R�}���h�ɂ�鏑���������s��Ȃ��̈�ł���B
	 */
	LCDBM_AREA_GRAPHIC__BREAK_00_NOT_UPDATE,	// 00: ���g�p
	LCDBM_AREA_GRAPHIC__BREAK_01_NOT_UPDATE,	// 01: �w�i(�S���)
};


/**
 *	�x����ʂ̃e�L�X�g���ԍ�
 *
 *	@code
 *	����������������������������������������
 *	��          [  TextArea01  ]          ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��[            TextArea10            ]��
 *	��[            TextArea11            ]��
 *	��[            TextArea12            ]��
 *	��[            TextArea13            ]��
 *	��[            TextArea14            ]��
 *	��[            TextArea15            ]��
 *	��[            TextArea16            ]��
 *	��[            TextArea17            ]��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��[      TextArea05      ][TextArea06]��
 *	����������������������������������������
 *	@endcode
 */
enum {
	LCDBM_AREA_TEXT__WARNING_00,	// 00: ���g�p
	LCDBM_AREA_TEXT__WARNING_01,	// 01: �匩�o��			[64x64]
	LCDBM_AREA_TEXT__WARNING_02,	// 02: ���g�p
	LCDBM_AREA_TEXT__WARNING_03,	// 03: ���g�p
	LCDBM_AREA_TEXT__WARNING_04,	// 04: ���g�p
	LCDBM_AREA_TEXT__WARNING_05,	// 05: �C���W�P�[�^�[	[24x24]
	LCDBM_AREA_TEXT__WARNING_06,	// 06: ���v				[24x24]
	LCDBM_AREA_TEXT__WARNING_07,	// 07: ���g�p
	LCDBM_AREA_TEXT__WARNING_08,	// 08: ���g�p
	LCDBM_AREA_TEXT__WARNING_09,	// 09: ���g�p
	LCDBM_AREA_TEXT__WARNING_10,	// 10: �x�����e�@
	LCDBM_AREA_TEXT__WARNING_11,	// 11: �x�����e�A
	LCDBM_AREA_TEXT__WARNING_12,	// 12: �x�����e�B
	LCDBM_AREA_TEXT__WARNING_13,	// 13: �x�����e�C
	LCDBM_AREA_TEXT__WARNING_14,	// 14: �x�����e�D
	LCDBM_AREA_TEXT__WARNING_15,	// 15: �x�����e�E
	LCDBM_AREA_TEXT__WARNING_16,	// 16: �x�����e�F
	LCDBM_AREA_TEXT__WARNING_17,	// 17: �x�����e�G
};


/**
 *	�x����ʂ̃O���t�B�b�N���ԍ�
 *
 *	@code
 *	����������������������������������������
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��           GraphicArea01            ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	��                                    ��
 *	����������������������������������������
 *	@endcode
 */
enum {
	/*
	 *	������"_NOT_UPDATE"���t���Ă����`���̓R�}���h�ɂ�鏑���������s��Ȃ��̈�ł���B
	 */
	LCDBM_AREA_GRAPHIC__WARNING_00_NOT_UPDATE,	// 00: ���g�p
	LCDBM_AREA_GRAPHIC__WARNING_01_NOT_UPDATE,	// 01: �w�i(�S���)
};


/**
 *	�ޑK��[��ʂ̃e�L�X�g���ԍ�
 */
enum {
	LCDBM_AREA_TEXT__REFILL_00,					// 00: ���g�p
	LCDBM_AREA_TEXT__REFILL_CAPTION_1,			// 01: [64x64]	�匩�o��
	LCDBM_AREA_TEXT__REFILL_CAPTION_2,			// 02: [36x36]	�����o��
	LCDBM_AREA_TEXT__REFILL_CHANGE,				// 03: [24x24]	�ޑK���ɍ���(�����������C�d�ݓ������C�R�C���z�b�p�[)
	LCDBM_AREA_TEXT__REFILL_SAFE_TYPE1,			// 04: [24x24]	���ɍ���(�������ɁC�R�C������)
	LCDBM_AREA_TEXT__REFILL_SAFE_TYPE2,			// 05: [24x24]	���ɍ���(��������)
	LCDBM_AREA_TEXT__REFILL_COINMEC,			// 06: [36x36]	�R�C�����b�N���ݖ���
	LCDBM_AREA_TEXT__REFILL_IN_COIN,			// 07: [24x24]	�R�C�����b�N��������
	LCDBM_AREA_TEXT__REFILL_HOPPER,				// 08: [36x36]	�R�C���z�b�p�[���ݖ���
	LCDBM_AREA_TEXT__REFILL_CSAFE_10,			// 09: [36x36]	�R�C������10�~���ݖ���
	LCDBM_AREA_TEXT__REFILL_CSAFE_50,			// 10: [36x36]	�R�C������50�~���ݖ���
	LCDBM_AREA_TEXT__REFILL_CSAFE_100,			// 11: [36x36]	�R�C������100�~���ݖ���
	LCDBM_AREA_TEXT__REFILL_CSAFE_500,			// 12: [36x36]	�R�C������500�~���ݖ���
	LCDBM_AREA_TEXT__REFILL_CASSETTE1,			// 13: [36x36]	�������o�J�Z�b�g1���ݖ���
	LCDBM_AREA_TEXT__REFILL_CASSETTE2,			// 14: [36x36]	�������o�J�Z�b�g2���ݖ���
	LCDBM_AREA_TEXT__REFILL_NTCYCL,				// 15: [36x36]	�����z�@1000�~���ݖ���
	LCDBM_AREA_TEXT__REFILL_IN_NOTE,			// 16: [36x36]	�����z�@1000�~��������
	LCDBM_AREA_TEXT__REFILL_NSAFE_1000,			// 17: [36x36]	��������1000�~���ݖ���
	LCDBM_AREA_TEXT__REFILL_NSAFE_2000,			// 18: [36x36]	��������2000�~���ݖ���
	LCDBM_AREA_TEXT__REFILL_NSAFE_5000,			// 19: [36x36]	��������5000�~���ݖ���
	LCDBM_AREA_TEXT__REFILL_NSAFE_10000,		// 20: [36x36]	��������10000�~���ݖ���
};


/**
 *	�ޑK��[��ʂ̃O���t�B�b�N���ԍ�
 */
enum {
	LCDBM_AREA_GRAPHIC__REFILL_00,				// 00: ���g�p
	LCDBM_AREA_GRAPHIC__REFILL_BACKGROUND,		// 01: �w�i
	LCDBM_AREA_GRAPHIC__REFILL_COINMEC_500,		// 02: �R�C�����b�N500�~��
	LCDBM_AREA_GRAPHIC__REFILL_COINMEC_100,		// 03: �R�C�����b�N100�~��
	LCDBM_AREA_GRAPHIC__REFILL_COINMEC_10,		// 04: �R�C�����b�N10�~��
	LCDBM_AREA_GRAPHIC__REFILL_COINMEC_50,		// 05: �R�C�����b�N50�~��
	LCDBM_AREA_GRAPHIC__REFILL_COINMEC_SUB,		// 06: �R�C�����b�NSUB��
	LCDBM_AREA_GRAPHIC__REFILL_HOPPER1,			// 07: �R�C���z�b�p�[1
	LCDBM_AREA_GRAPHIC__REFILL_HOPPER2,			// 08: �R�C���z�b�p�[2
	LCDBM_AREA_GRAPHIC__REFILL_CASSETTE1,		// 09: �������o�@�J�Z�b�g1
	LCDBM_AREA_GRAPHIC__REFILL_CASSETTE2,		// 10: �������o�@�J�Z�b�g2
	LCDBM_AREA_GRAPHIC__REFILL_NTCYCL,			// 11: �����z�@1000�~���[��
	LCDBM_AREA_GRAPHIC__REFILL_COIN_ANIME,		// 12: �R�C�������A�j���[�V����
	LCDBM_AREA_GRAPHIC__REFILL_NOTE_ANIME,		// 13: �����}���A�j���[�V����
};


/**
 *	�����e�i���X���
 *	�i�w�i�摜�A��ʃ��C�A�E�g�͋x�ƒ��̎g���܂킵�j
 */
enum {
	LCDBM_AREA_TEXT__MAINTENANCE_00,		// 00: ���g�p
	LCDBM_AREA_TEXT__MAINTENANCE_CAPTION_1,	// 01: [64x64]	�匩�o��
	LCDBM_AREA_TEXT__MAINTENANCE_02,		// 02: ���g�p
	LCDBM_AREA_TEXT__MAINTENANCE_CAPTION_2,	// 03: [36x36]	�����o��
	LCDBM_AREA_TEXT__MAINTENANCE_04,		// 04: ���g�p
	LCDBM_AREA_TEXT__MAINTENANCE_05,		// 05: ���g�p
	LCDBM_AREA_TEXT__MAINTENANCE_CLOCK,		// 06: [24x24]	���v
};
enum {
	LCDBM_AREA_GRAPHIC__MAINTENANCE_00,		// 00: ���g�p
	LCDBM_AREA_GRAPHIC__MAINTENANCE_01,		// 01: �w�i
};


//--------------------------------------------------
//		���Z�@�p�l�� �摜�ԍ�
//--------------------------------------------------
#define	PANEL_PATTERN_NUM	50									// �p�l���G�̃p�^�[����
#define	PANEL_INCREMENT	(PANEL_PATTERN_NUM + PANEL_PATTERN_NUM)	// �p�l���G�̃x�[�X�ԍ������iLCD�����O���t�B�b�N�����{LED�_���O���t�B�b�N�����j

/**
 *	���Z�@�p�l���̃I�v�V������ �I�t�Z�b�g�ԍ�
 */
enum {
	LCDBM_IMAGE_PANEL__OFFSET_MIFARE =											PANEL_INCREMENT,	// Mifare
	LCDBM_IMAGE_PANEL__OFFSET_SUICA =		LCDBM_IMAGE_PANEL__OFFSET_MIFARE +	PANEL_INCREMENT,	// Suica
	LCDBM_IMAGE_PANEL__OFFSET_EDY =			LCDBM_IMAGE_PANEL__OFFSET_SUICA +	PANEL_INCREMENT,	// Edy
	LCDBM_IMAGE_PANEL__OFFSET_WAON =		LCDBM_IMAGE_PANEL__OFFSET_EDY +		PANEL_INCREMENT,	// WAON
	LCDBM_IMAGE_PANEL__OFFSET_PARKRIDE =	LCDBM_IMAGE_PANEL__OFFSET_WAON +	PANEL_INCREMENT,	// ��ԗ���
	LCDBM_IMAGE_PANEL__OFFSET_RESERVE1 =	LCDBM_IMAGE_PANEL__OFFSET_PARKRIDE +PANEL_INCREMENT,	// �\��
	LCDBM_IMAGE_PANEL__OFFSET_RESERVE2 =	LCDBM_IMAGE_PANEL__OFFSET_RESERVE1 +PANEL_INCREMENT,	// �\��
	LCDBM_IMAGE_PANEL__OFFSET_RESERVE3 =	LCDBM_IMAGE_PANEL__OFFSET_RESERVE2 +PANEL_INCREMENT,	// �\��
	LCDBM_IMAGE_PANEL__OFFSET_RESERVE4 =	LCDBM_IMAGE_PANEL__OFFSET_RESERVE3 +PANEL_INCREMENT,	// �\��
	LCDBM_IMAGE_PANEL__OFFSET_RESERVE5 =	LCDBM_IMAGE_PANEL__OFFSET_RESERVE4 +PANEL_INCREMENT,	// �\��
};


/**
 *	���Z�@�O�ʃp�l���ԍ�
 *	@note
 *	-	���Z���̉�ʍ����ɕ\������
 *	-	CPS�AEPS�͋��ʂœ����G���g��
 */
enum {
	// ���Z�@�p�l���G �x�[�X�ԍ�
	LCDBM_IMAGE_PANEL__BASE =	1000,

	// GT-7700 �������o�@�Ȃ�
	LCDBM_IMAGE_PANEL__GT7700_NORMAL =			LCDBM_IMAGE_PANEL__BASE,													//
	LCDBM_IMAGE_PANEL__GT7700_MIFARE =			LCDBM_IMAGE_PANEL__GT7700_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_MIFARE,		// Mifare
	LCDBM_IMAGE_PANEL__GT7700_SUICA =			LCDBM_IMAGE_PANEL__GT7700_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_SUICA,			// Suica
	LCDBM_IMAGE_PANEL__GT7700_EDY =				LCDBM_IMAGE_PANEL__GT7700_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_EDY,			// Edy
	LCDBM_IMAGE_PANEL__GT7700_WAON =			LCDBM_IMAGE_PANEL__GT7700_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_WAON,			// WAON
	LCDBM_IMAGE_PANEL__GT7700_PARKRIDE =		LCDBM_IMAGE_PANEL__GT7700_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_PARKRIDE,		// ��ԗ���
	LCDBM_IMAGE_PANEL__GT7700_RESERVE1 =		LCDBM_IMAGE_PANEL__GT7700_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE1,		// �\��
	LCDBM_IMAGE_PANEL__GT7700_RESERVE2 =		LCDBM_IMAGE_PANEL__GT7700_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE2,		// �\��
	LCDBM_IMAGE_PANEL__GT7700_RESERVE3 =		LCDBM_IMAGE_PANEL__GT7700_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE3,		// �\��
	LCDBM_IMAGE_PANEL__GT7700_RESERVE4 =		LCDBM_IMAGE_PANEL__GT7700_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE4,		// �\��
	LCDBM_IMAGE_PANEL__GT7700_RESERVE5 =		LCDBM_IMAGE_PANEL__GT7700_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE5,		// �\��

	// GT-7700 �������o�@����
	LCDBM_IMAGE_PANEL__GT7700_BDU_NORMAL =		LCDBM_IMAGE_PANEL__GT7700_RESERVE5 + PANEL_INCREMENT,						//
	LCDBM_IMAGE_PANEL__GT7700_BDU_MIFARE =		LCDBM_IMAGE_PANEL__GT7700_BDU_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_MIFARE,	// Mifare
	LCDBM_IMAGE_PANEL__GT7700_BDU_SUICA =		LCDBM_IMAGE_PANEL__GT7700_BDU_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_SUICA,		// Suica
	LCDBM_IMAGE_PANEL__GT7700_BDU_EDY =			LCDBM_IMAGE_PANEL__GT7700_BDU_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_EDY,		// Edy
	LCDBM_IMAGE_PANEL__GT7700_BDU_WAON =		LCDBM_IMAGE_PANEL__GT7700_BDU_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_WAON,		// WAON
	LCDBM_IMAGE_PANEL__GT7700_BDU_PARKRIDE =	LCDBM_IMAGE_PANEL__GT7700_BDU_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_PARKRIDE,	// ��ԗ���
	LCDBM_IMAGE_PANEL__GT7700_BDU_RESERVE1 =	LCDBM_IMAGE_PANEL__GT7700_BDU_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE1,	// �\��
	LCDBM_IMAGE_PANEL__GT7700_BDU_RESERVE2 =	LCDBM_IMAGE_PANEL__GT7700_BDU_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE2,	// �\��
	LCDBM_IMAGE_PANEL__GT7700_BDU_RESERVE3 =	LCDBM_IMAGE_PANEL__GT7700_BDU_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE3,	// �\��
	LCDBM_IMAGE_PANEL__GT7700_BDU_RESERVE4 =	LCDBM_IMAGE_PANEL__GT7700_BDU_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE4,	// �\��
	LCDBM_IMAGE_PANEL__GT7700_BDU_RESERVE5 =	LCDBM_IMAGE_PANEL__GT7700_BDU_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE5,	// �\��

	// GT-7750 �����z�@����
	LCDBM_IMAGE_PANEL__GT7750_NORMAL =			LCDBM_IMAGE_PANEL__GT7700_BDU_RESERVE5 + PANEL_INCREMENT,					//
	LCDBM_IMAGE_PANEL__GT7750_MIFARE =			LCDBM_IMAGE_PANEL__GT7750_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_MIFARE,		// Mifare
	LCDBM_IMAGE_PANEL__GT7750_SUICA =			LCDBM_IMAGE_PANEL__GT7750_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_SUICA,			// Suica
	LCDBM_IMAGE_PANEL__GT7750_EDY =				LCDBM_IMAGE_PANEL__GT7750_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_EDY,			// Edy
	LCDBM_IMAGE_PANEL__GT7750_WAON =			LCDBM_IMAGE_PANEL__GT7750_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_WAON,			// WAON
	LCDBM_IMAGE_PANEL__GT7750_PARKRIDE =		LCDBM_IMAGE_PANEL__GT7750_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_PARKRIDE,		// ��ԗ���
	LCDBM_IMAGE_PANEL__GT7750_RESERVE1 =		LCDBM_IMAGE_PANEL__GT7750_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE1,		// �\��
	LCDBM_IMAGE_PANEL__GT7750_RESERVE2 =		LCDBM_IMAGE_PANEL__GT7750_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE2,		// �\��
	LCDBM_IMAGE_PANEL__GT7750_RESERVE3 =		LCDBM_IMAGE_PANEL__GT7750_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE3,		// �\��
	LCDBM_IMAGE_PANEL__GT7750_RESERVE4 =		LCDBM_IMAGE_PANEL__GT7750_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE4,		// �\��
	LCDBM_IMAGE_PANEL__GT7750_RESERVE5 =		LCDBM_IMAGE_PANEL__GT7750_NORMAL + LCDBM_IMAGE_PANEL__OFFSET_RESERVE5,		// �\��
};


/**
 *	���Z�@�O�ʃp�l���́���p�^�[��
 *	@note
 *	-	LCDBM_IMAGE_PANEL_* �Œ�`�����l�ɉ��Z���ăO���t�B�b�N�ԍ����Z�o����
 *	-	LED�����O���t�B�b�N��LED�_���O���t�B�b�N��g�ݍ��킹��LED�_�ŃA�j���[�V�������\������
 */
enum {
	// �O���t�B�b�N�p�^�[���t�@�C���ԍ����w�肷��ꍇ�ɂ̓p�l���G�̃x�[�X�ԍ��ɂ��̒l�����Z����
	LCDBM_IMAGE_PANEL_NO__NOTHING,			//  0: ����Ȃ�
	LCDBM_IMAGE_PANEL_NO__TICKET_IN,		//  1: ���Ԍ�������
	LCDBM_IMAGE_PANEL_NO__COIN_IN,			//  2: �d�ݓ�����
	LCDBM_IMAGE_PANEL_NO__NOTE_IN,			//  3: ����������
	LCDBM_IMAGE_PANEL_NO__TICKET_OUT,		//  4: ���Ԍ��ԋp��
	LCDBM_IMAGE_PANEL_NO__COIN_OUT,			//  5: �d�݁E�̎��ؕԋp��
	LCDBM_IMAGE_PANEL_NO__NOTE_OUT,			//  6: �����ԋp��
	LCDBM_IMAGE_PANEL_NO__ICCARD1,			//  7: IC�J�[�h���[�_�[�i�c�z�Ɖ�j
	LCDBM_IMAGE_PANEL_NO__ICCARD2,			//  8: IC�J�[�h���[�_�[�i���ρj
	LCDBM_IMAGE_PANEL_NO__ICCARD3,			//  9: ��ԗ������[�_�[
};


//--------------------------------------------------
//		���W���[���A�j�� �摜�ԍ�
//--------------------------------------------------
/**
 *	���W���[���A�j��
 *	@note
 *	-	���Z���̉�ʉE���ɕ\������
 */
enum {
	LCDBM_IMAGE_MODULE__ANIME_COMMENT =	15000,	// ���W���[���A�j������e�L�X�g�w�i
	LCDBM_IMAGE_MODULE__START =	10000,			// ���W���[���A�j���̃x�[�X�摜�ԍ�

	/*
	 *	���C���[�_
	 *
	 *	10000:���C���[�_ �g�p�s��
	 *	10001:���C�J�[�h �}��
	 *	10002:���C�J�[�h �ԋp
	 */
	LCDBM_IMAGE_MODULE__MAG_READER_NG =		LCDBM_IMAGE_MODULE__START,
	LCDBM_IMAGE_MODULE__MAG_READER_IN,
	LCDBM_IMAGE_MODULE__MAG_READER_OUT,

	/*
	 *	�T�C�o�l���[�_
	 *
	 *	10010:�T�C�o�l���[�_ �g�p�s��
	 *	10011:�T�C�o�l�J�[�h �}��
	 *	10012:�T�C�o�l�J�[�h �ԋp
	 */
	LCDBM_IMAGE_MODULE__CYBERNE_READER_NG =	LCDBM_IMAGE_MODULE__START + 10,
	LCDBM_IMAGE_MODULE__CYBERNE_READER_IN,
	LCDBM_IMAGE_MODULE__CYBERNE_READER_OUT,

	/*
	 *	�d�� ����
	 *
	 *	10020:�d�� �����s��
	 *	10021:�d�� ����
	 *	10022:�d�� �����s�i�܂Ƃߓ���j
	 *	10023:�d�� �����i�܂Ƃߓ���j
	 */
	LCDBM_IMAGE_MODULE__COIN_READER_NG =	LCDBM_IMAGE_MODULE__START + 20,
	LCDBM_IMAGE_MODULE__COIN_READER_IN,
// LH068007 �@�\�A�b�v (S) 2011/04/15(��) <Yoshiaki_Hamada@amano.co.jp> �R�C���܂Ƃߓ���O���t�B�b�N�ǉ�
	LCDBM_IMAGE_MODULE__COIN_READER_2_NG,
	LCDBM_IMAGE_MODULE__COIN_READER_2_IN,
// LH068007 �@�\�A�b�v (E) 2011/04/15(��) <Yoshiaki_Hamada@amano.co.jp> �R�C���܂Ƃߓ���O���t�B�b�N�ǉ�

	/*
	 *	�������[�_�[ ����
	 *
	 *	10030:�������[�_�[ �����s��
	 *	10031:�������[�_�[ ����
	 *	10032:�������[�_�[ �ԋp�s��
	 *	10033:�������[�_�[ �ԋp
	 */
	LCDBM_IMAGE_MODULE__NOTE_READER_NG =	LCDBM_IMAGE_MODULE__START + 30,
	LCDBM_IMAGE_MODULE__NOTE_READER_IN,
	LCDBM_IMAGE_MODULE__NOTE_READER_NG2,
	LCDBM_IMAGE_MODULE__NOTE_READER_OUT,

	/*
	 *	���������o���@ �o��
	 *
	 *	10040:���������o���@ �o���s��
	 *	10041:���������o���@ �o��
	 */
	LCDBM_IMAGE_MODULE__NOTE_PAYOUT_NG =	LCDBM_IMAGE_MODULE__START + 40,
	LCDBM_IMAGE_MODULE__NOTE_PAYOUT_OUT,

	/*
	 *	�����z�@ ����
	 *
	 *	10050:�����z�@ �����s��
	 *	10051:�����z�@ ����
	 */
	LCDBM_IMAGE_MODULE__BRU_READER_NG =		LCDBM_IMAGE_MODULE__START + 50,
	LCDBM_IMAGE_MODULE__BRU_READER_IN,

	/*
	 *	�����z�@ �o��
	 *
	 *	10060:�����z�@ �o���s��
	 *	10061:�����z�@ �o��
	 */
	LCDBM_IMAGE_MODULE__BRU_PAYOUT_NG =		LCDBM_IMAGE_MODULE__START + 60,
	LCDBM_IMAGE_MODULE__BRU_PAYOUT_OUT,

	/*
	 *	Mifare
	 *
	 *	10070:Mifare���[�_ ���p�s��
	 *	10071:Mifare���[�_������
	 */
	LCDBM_IMAGE_MODULE__MIFARE_NG =			LCDBM_IMAGE_MODULE__START + 70,
	LCDBM_IMAGE_MODULE__MIFARE_TOUCH,

	/*
	 *	Suica�iPASMO�������G���g���j
	 *
	 *	10080:Suica���[�_ ���p�s��
	 *	10081:Suica���[�_�������i���ρj
	 *	10082:Suica���[�_�������i�c�z�Ɖ�j
	 */
	LCDBM_IMAGE_MODULE__SUICA_NG =			LCDBM_IMAGE_MODULE__START + 80,
	LCDBM_IMAGE_MODULE__SUICA_PAY,
	LCDBM_IMAGE_MODULE__SUICA_BALANCE,

	/*
	 *	PASMO��Suica�Ɠ����G���g���̂Ō���
	 *	LCDBM_IMAGE_MODULE__PASMO =		LCDBM_IMAGE_MODULE__START + 90,
	 */

	/*
	 *	Edy
	 *
	 *	10100:Edy���[�_ ���p�s��
	 *	10101:Edy���[�_�������i���ρj
	 *	10102:Edy���[�_�������i�c�z�Ɖ�j
	 */
	LCDBM_IMAGE_MODULE__EDY_NG =			LCDBM_IMAGE_MODULE__START + 100,
	LCDBM_IMAGE_MODULE__EDY_PAY,
	LCDBM_IMAGE_MODULE__EDY_BALANCE,

	/*
	 *	WAON
	 *
	 *	10110:WAON���[�_ ���p�s��
	 *	10111:WAON���[�_�������i���ρj
	 *	10112:WAON���[�_�������i�c�z�Ɖ�j
	 */
	LCDBM_IMAGE_MODULE__WAON_NG =			LCDBM_IMAGE_MODULE__START + 110,
	LCDBM_IMAGE_MODULE__WAON_PAY,
	LCDBM_IMAGE_MODULE__WAON_BALANCE,

	/*
	 *	��ԗ������[�_�[�i�{�^�����j
	 *
	 *	10120:��ԗ������[�_�[ ���p�s��
	 *	10121:��ԗ������[�_�[�ɂ�����
	 */
	LCDBM_IMAGE_MODULE__PARKRIDE1_NG =		LCDBM_IMAGE_MODULE__START + 120,
	LCDBM_IMAGE_MODULE__PARKRIDE1_TOUCH,

	/*
	 *	(��ԗ���)Suica���[�_�[�i�{�^�����j
	 *
	 *	10130:��ԗ����{�^��������
	 *	10131:�d�q�}�l�[�{�^��������
	 */
	LCDBM_IMAGE_MODULE__PARKRIDE2_NG =		LCDBM_IMAGE_MODULE__START + 130,
	LCDBM_IMAGE_MODULE__PARKRIDE2_TOUCH,

	/*
	 *	�R�C���ԋ�
	 *
	 *	10140:�R�C���ԋ��s�i���ۂɂ͂��蓾�Ȃ��̂Ŕԍ��\��Ƃ���j
	 *	10141:�R�C���ԋ�
	 */
	LCDBM_IMAGE_MODULE__COIN_PAYOUT_NG =	LCDBM_IMAGE_MODULE__START + 140,
	LCDBM_IMAGE_MODULE__COIN_PAYOUT_OUT,

	/*
	 *	�̎��؁E�a���
	 *
	 *	10150:�̎��؁E�a��� ����s�i�s�v�ƍl����̂Ŕԍ��\��Ƃ���j
	 *	10151:�̎��؁E�a��� ����i�̎��؎�o�����j
	 */
	LCDBM_IMAGE_MODULE__RECEIPT_NG =		LCDBM_IMAGE_MODULE__START + 150,
	LCDBM_IMAGE_MODULE__RECEIPT_OUT,
};


//--------------------------------------------------
//		�O���t�B�b�N�p�^�[��
//--------------------------------------------------
/**
 *	�O���t�B�b�N�\���ؑփR�}���h�Ŏw�肷��O���t�B�b�N�p�^�[���ԍ�
 */
enum {
	LCDBM_IMAGE_PATTERN__PAID_GOODBYE = 20440,	// ���Z������̏o�ɓ��A�j���[�V����
};


//--------------------------------------------------
//		�e�L�X�g
//--------------------------------------------------
/** ���W���[���A�j���̃��b�Z�[�W�̍s�� */
#define	LCDBM_AREA_TEXT__ANIME_COMMENT_NUM	( LCDBM_AREA_TEXT__ANIME_COMMENT_4 - LCDBM_AREA_TEXT__ANIME_COMMENT_1 + 1 )


/**
 *	�e�L�X�g�ԍ�
 */
enum {
	//--------------------------------------------------
	//		����
	//--------------------------------------------------
	LCDBM_FILE_TEXT__NOTHING =	0,					//	0: �����\�����Ȃ��i�\�����Ă���e�L�X�g���������鎞�Ɏg���j


	LCDBM_FILE_TEXT__CARD_NAME_2 =			100,	/*	������2�i���Ԍ��̖��́j
													 *	+0:	"���Ԍ�"
													 *	+1:	"���ԃJ�[�h"
													 *	+2:	"���Ԑ�����"
													 *	+3:	"������"
													 *	+4:	"�������p��"
													 *	+5:	"�ꎞ���p��"
													 *	+6:	"������p��"
													 */
	LCDBM_FILE_TEXT__CARD_NAME_3 =			200,	/*	������3�i������̖��́j
													 *	+0:	"�����"
													 *	+1:	"������p��"
													 *	+2:	"�p�X�J�[�h"
													 *	+3:	"�_��J�[�h"
													 */
	LCDBM_FILE_TEXT__CARD_NAME_4 =			300,	/*	������4�i�T�[�r�X���^�|�����̖��́j
													 *	+0:	"�T�[�r�X��"
													 *	+1:	"������"
													 */
	LCDBM_FILE_TEXT__CARD_NAME_5 =			400,	/*	������5�i���Z�Ō��ϑO�ɓ����悤�ɑ�������j
													 *	+0:	"�v���y�C�h�J�[�h"
													 *	+1:	"�񐔌�"
													 *	+2:	"�N���W�b�g�J�[�h"
													 */
	LCDBM_FILE_TEXT__CARD_NAME_ICCARD =		500,	/*	�d�q�J�[�h�n
													 *	+0:	"�d����"
													 */


	//--------------------------------------------------
	//		�e������e�[�u���ɑΉ�����ԍ�
	//	�i�v���O���������ɌŒ�f�[�^�Ƃ��Ď����Ă��镶����ɑΉ��j
	//--------------------------------------------------
	LCDBM_FILE_TEXT__OPE_CHR =				10000,	// OPE_CHR[0]
	LCDBM_FILE_TEXT__OPE_CHR_G =			10200,	// OPE_CHR_G[0]
	LCDBM_FILE_TEXT__OPE_CHR_G_SALE_MSG	=	10400,	// OPE_CHR_G_SALE_MSG[0]
	LCDBM_FILE_TEXT__OPE_CHR_G_SALE_MSG2 =	10600,	// OPE_CHR_G_SALE_MSG2[0]
	LCDBM_FILE_TEXT__OPE_CHR_G_SALE_28 =	10800,	// OPE_CHR_G_SALE_28[0]
	LCDBM_FILE_TEXT__OPE_CHR_G_SALE_8 =		11000,	// OPE_CHR_G_SALE_8[0]
	LCDBM_FILE_TEXT__CLSMSG =				11200,	// CLSMSG[0]
	LCDBM_FILE_TEXT__DRCLS_ALM =			11400,	// DRCLS_ALM[0]

	// ���p�\�}�̖��́i24-0031�`24-0034�j
	LCDBM_FILE_TEXT__OPE_CHR_CYCLIC_MSG =	11600,	// OPE_CHR_CYCLIC_MSG[0]

	// ��������
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_0 =		11800,	// [0]  "���ԗ����@�@�@�@"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_1,				// [1]  "�������z�@�@�@�@"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_2,				// [2]  "�������z�@�@�@�@"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_3,				// [3]  "�v���y�C�h�J�[�h"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_4,				// [4]  "�񐔌��@�@�@�@�@"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_5,				// [5]  "�d�q�}�l�[�@�@�@"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_6,				// [6]  "�O��x���z�@�@�@"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_7,				// [7]  "�N���W�b�g�J�[�h"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_8,				// [8]  "����@�@�@�@�@"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_9,				// [9]  "�@�@�@�@�@�i�c�z"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_10,				// [10] "�@�@�@�@�i�c�x��"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_11,				// [11] "�~  "
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_12,				// [12] "�~�j"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_13,				// [13] "��j"
// GW-861011 �J���[LCD��Edy�Ή� (S) 2010/07/29(��) <Yoshiaki_Hamada@amano.co.jp> Edy�Ή�
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_14,				// [14] "�d�����x���@�@�@"
	LCDBM_FILE_TEXT__OPE_CHR_DETAIL_15,				// [15] "�d�����c���@�@�@"
// GW-861011 �J���[LCD��Edy�Ή� (E) 2010/07/29(��) <Yoshiaki_Hamada@amano.co.jp> Edy�Ή�


	//--------------------------------------------------
	//		�|�b�v�A�b�v
	//--------------------------------------------------
	LCDBM_FILE_TEXT__POPUP =	30000,				// �|�b�v�A�b�v�ɕ\������e�L�X�g

	LCDBM_FILE_TEXT__POPUP_LINEFEED =	30000,		/*	���s�p�e�L�X�g
													 *	+1:	�P�s���s�������ꍇ
													 *	+2:	�Q�s���s�������ꍇ
													 *	+2:	�R�s���s�������ꍇ
													 */
	LCDBM_FILE_TEXT__POPUP_EXCEPTION =	30010,		// �|�b�v�A�b�v��O�\���Ɏg���e�L�X�g

	LCDBM_FILE_TEXT__POPUP_PASS_CPS =	30211,		/*	CPS�̂��߂ɒ�����ł̐��Z���o�����ɃG���[�ƂȂ����ꍇ�̈ē�(43-0064)
													 *	+0:	"�͏o���ɂĂ����p��������"
													 *	+1:	"�o�����Z�@�Ő��Z���Ă�������"
													 *	+2:	"��t�ɂ����ł�������"
													 */


	//--------------------------------------------------
	//		���̑��̕W���\�t�g�p�f�[�^
	//--------------------------------------------------
	LCDBM_FILE_TEXT__UNIVERSAL_HA =		40000,		// 40000: "��"
	LCDBM_FILE_TEXT__UNIVERSAL_TADAIMA,				// 40001: "�����A"
	LCDBM_FILE_TEXT__UNIVERSAL_KONO,				// 40002: "����"
	LCDBM_FILE_TEXT__UNIVERSAL_HAVE,				// 40003: "���������̕���"
	LCDBM_FILE_TEXT__UNIVERSAL_TOUCH,				// 40004: "�^�b�`���Ă�������"
	LCDBM_FILE_TEXT__UNIVERSAL_NOT_USE,				// 40005: "�����p�ł��܂���"
	LCDBM_FILE_TEXT__UNIVERSAL_MIDDLE_DOT,			// 40006: "�E"
	LCDBM_FILE_TEXT__UNIVERSAL_MON,					// 40007: "��"
	LCDBM_FILE_TEXT__UNIVERSAL_TOUTEN,				// 40008: "�A"

	LCDBM_FILE_TEXT__CANCEL_CAPTION_1 =	40100,		// �匩�o�� "�Ɓ@��@���@��"
	LCDBM_FILE_TEXT__TICKET_TIME =		40101,		/*	���Z���̎��C�J�[�h����\�����鎞�ԃf�[�^�̑O�ɕt������
													 *	+0: "���Ɏ��� "
													 *	+1: "�O�񐸎Z "
													 *	+2: "���Ԏ��� "
													 */
	LCDBM_FILE_TEXT__NOTICE_CHANGE =	40104,		// "���Y��ɂ����ӂ�������"
	LCDBM_FILE_TEXT__YOU_CAN_PAY =		40105,		// "�ł��x�����ł��܂�"
	LCDBM_FILE_TEXT__GET_PLEASE =		40106,		// "������肭������"

	LCDBM_FILE_TEXT__PAY_MANUAL =		40107,		// "�蓮���Z"

// GW861010 m.onouchi(S) 2010/07/26 LCD���W���[���Ή�(�ԔԔF��)
	LCDBM_FILE_TEXT__MATCHING =			40108,		// "���肪�Ƃ��������܂���"
// GW861010 m.onouchi(E) 2010/07/26 LCD���W���[���Ή�(�ԔԔF��)

	LCDBM_FILE_TEXT__HEADLINE_L_2 =		40110,		/*	�匩�o���Q�i36x36�j
													 *	+0: "�����́@�@�@�@�@�@�~�ł�"
													 *	+1: "���Ɓ@�@�@�@�@�@�@�~�ł�"
													 *	+2: "����́@�@�@�@�@�~�ł�"
													 */
// GW861010 m.onouchi(S) 2010/07/26 LCD���W���[���Ή�(�ԔԔF��)
	LCDBM_FILE_TEXT__VISITOR =			40113,		// "���q�l��"(������2)�^(������3)
	LCDBM_FILE_TEXT__CARD_NUM,						// "�ԍ���"����������
	LCDBM_FILE_TEXT__DESU,							// "�ł�"
// GW861010 m.onouchi(E) 2010/07/26 LCD���W���[���Ή�(�ԔԔF��)
// LH068007(S) Hamada Yoshiaki 2011/05/11 CPS�Łu���Ԍ������ĉ������v���b�Z�[�W��ݒ�\�Ƃ���
	LCDBM_FILE_TEXT__CPS_TICKET_INSERT =	40117,	/*	CPS�̒��Ԍ��}���҂����b�Z�[�W
													 *	+0: "���A��̕��́A"
													 *	+1: "���̐��Z�@"
													 *	+2: "�Ő��Z���Ă�������"
													 *	+3: "���A��̍ۂɁA"
													 */
// LH068007(E) Hamada Yoshiaki 2011/05/11 CPS�Łu���Ԍ������ĉ������v���b�Z�[�W��ݒ�\�Ƃ���

	/*-----  �ݒ�ŕ�����ύX�������  -----*/
	LCDBM_FILE_TEXT__STAFF_NAME =			40500,	/*	�W�����́i�W���̕\�����́j
													 *	+0: "�W��"
													 *	+1: "�]�ƈ�"
													 *	+2: "�Z�p��"
													 *	+3: "�Ǘ���"
													 *	+4: "�Ǘ��l"
													 *	+5:	"�ӔC��"
													 */
	LCDBM_FILE_TEXT__OFFICE_NAME =			40510,	/*	�Ǘ������́i�Ǘ����̕\�����́j
													 *	+0: "�Ǘ���"
													 *	+1: "���ԏ�Ǘ���"
													 *	+2: "�Ǘ��u�[�X"
													 *	+3: "�Ǘ����"
													 *	+4: "�T�[�r�X�J�E���^�["
													 *	+5:	"�C���t�H���[�V�����Z���^�["
													 */
	LCDBM_FILE_TEXT__ERROR_CARD_NAME =		40520,	/*	�G���[���̌�����
													 *	+0: "��"
													 *	+1: "�J�[�h"
													 */
	LCDBM_FILE_TEXT__CARD_NAME_1 =			40525,	/*	������2�ƌ�����3��A������ڑ���
													 *	"����"
													 */
	LCDBM_FILE_TEXT__PAYING_CAPTION_1 =		40550,	/*	���ԗ������Z���̑匩�o���̕�����
													 *	+0:	"�����Ă�������"
													 *	+1:	"��1�ɓ���Ă�������"
													 *	+2:	"����̕����ɓ���Ă�������"
													 */
	LCDBM_FILE_TEXT__PAY_START_CAPTION_2 =	40560,	/*	���ԗ������Z���̒����o���̕�����
													 *	+0:	�\���Ȃ�
													 *	+1:	"���������̕���"
													 *	+2:	"�����Ă�������"
													 *	+3:	"�𕴎����ꂽ����"
													 *	+4:	"�����{�^���������Ă�������"
													 *	+5:	"���X�V��������"
													 *	+6:	"�X�V�{�^���������Ă�������"
													 */
	LCDBM_FILE_TEXT__PAYING_CAPTION_2 =		40590,	/*	���ԗ������Z���̒����o���̕�����iEPS�j
													 *	+0:	�\���Ȃ�
													 *	+1:	"�͐�ɓ���Ă�������"
													 *	+2:	"�|�C���g�J�[�h�͌��������O�ɓ���Ă�������"
													 *	+3:	"�|�C���g�J�[�h��"
													 *	+4:	"�g�p��ɓ���Ă�������"
													 */
	LCDBM_FILE_TEXT__CARD_NAME_5_PASS =		40600,	/*	������5�i�̒�������́j
													 *	+0:	"������A"
													 *	+1:	"������p���A"
													 *	+2:	"�p�X�J�[�h�A"
													 *	+3:	"�_��J�[�h�A"
													 */
	LCDBM_FILE_TEXT__CARD_NAME_5_SERVICE =	40610,	/*	������5�i�̃T�[�r�X�����́j
													 *	+0:	"�T�[�r�X���A"
													 *	+1:	"�������A"
													 */
	LCDBM_FILE_TEXT__CARD_NAME_5_OTHER =	40620,	/*	������5
													 *	+0:	"�v���y�C�h�J�[�h�A"
													 *	+1:	"�񐔌��A"
													 *	+2:	"�N���W�b�g�J�[�h�A"
													 */
	LCDBM_FILE_TEXT__PAID_CAPTION_2 =	40640,		/*	���Z�������̃��b�Z�[�W�\��(CPS/EPS)
													 *	+0:	�\���Ȃ�
													 *	+1:	"�Q�[�g���J���܂��@�O�ւ��i�݂�������"
													 *	+2:	"�����p���肪�Ƃ��������܂���"
													 *	+3:	"�C�����Ă��A�肭������"
													 *	+4:	"�܂��̂����p���҂����Ă���܂�"
													 *	+5:	"�߂���"(������2)�^(������3)
													 *	+6:	"�͏o���ŕK�v�ł�"
													 */
	LCDBM_FILE_TEXT__RECEIPT_NAME =	40650,			/* �̎��ؖ���
													 * +0:"�̎���"
													 * +1:"�̎���"
													 * +2:"�m�̎��؁n"
													 * +3:"�m�̎����n"
													 */
	LCDBM_FILE_TEXT__RECEIPT_GUIDANCE =	40660,		/* �̎��؈ē�
													 * +0:"�̕K�v�ȕ���"
													 * +1:"�{�^���������Ă�������"
													 * +2:"���Z��������"
													 * +3:"�𔭍s���܂�"
													 * +4:"�܂��͗��p���׏��𔭍s���܂�"
													 * +5:"�̔��s�͏o���܂���"
													 * +6:"������肭������"
													 * +7:"���p���׏�������肭������"
													 */

	// ��؂�
	LCDBM_FILE_TEXT__ALARM_TITLE =	41000,			// "��؂� "
	LCDBM_FILE_TEXT__ALARM_10,						// "10�~ "
	LCDBM_FILE_TEXT__ALARM_50,						// "50�~ "
	LCDBM_FILE_TEXT__ALARM_100,						// "100�~ "
	LCDBM_FILE_TEXT__ALARM_500,						// "500�~ "
	LCDBM_FILE_TEXT__ALARM_1000,					// "1000�~ "
	LCDBM_FILE_TEXT__ALARM_2000,					// "2000�~ "
	LCDBM_FILE_TEXT__ALARM_5000,					// "5000�~"

	// �ҋ@��ʃA���[��
	LCDBM_FILE_TEXT__ALARM_CHANGE =	41010,			// "�ޑK�s��"
	LCDBM_FILE_TEXT__ALARM_RECEIPT,					// "���V�[�g�p���s��"
	LCDBM_FILE_TEXT__ALARM_JOURNAL,					// "�W���[�i���p���s��"
	LCDBM_FILE_TEXT__ALARM_NOTE,					// "�������p�s��"

	// �x�ƒ�
	LCDBM_FILE_TEXT__CLOSURE_CAPTION_1 =	41100,	// "�x�@�Ɓ@��"
	LCDBM_FILE_TEXT__CLOSURE_INFO,					/*	�x�Ǝ��̕\��(43-0062)
													 *	+0:	"�ق��̐��Z���Ő��Z���Ă�������"
													 *	+1:	"�o���ɂ����ł�������"
													 *	+2:	"�������܂����A��t�ɂ����ł�������"
													 *	+3:	"�������܂����A"
													 *	+4:	"�ɂ����ł�������"
													 *	+5:	"�C���^�[�z���ŘA�����Ă�������"
													 *	+6:	"�ɘA�����Ă�������"
													 *	+7:	"�Ăяo���{�^���������Ă�������"
													 *	+8:	"���܂���܂��B���΂炭���҂���������"
													 */

	// �ޑK��[�Ƌ��ɖ���
	LCDBM_FILE_TEXT__REFILL	= 41300,				// LCDBM_FILE_TEXT__REFILL+((LCDBM_AREA_TEXT__REFILL_xxxx -1)*3)+OFFSET

	// �x��
	LCDBM_FILE_TEXT__WARNING =	41400,				// �x����ʂ̑匩�o��

// GW-861011 �J���[LCD��Edy�Ή� (S) 2010/07/28(��) <Yoshiaki_Hamada@amano.co.jp> Edy�Ή�
	// �d�����֘A
	LCDBM_FILE_TEXT__EDY =	41500,							// �d�����֘A
	LCDBM_FILE_TEXT__EDY_TESTMODE =	LCDBM_FILE_TEXT__EDY,	// �d�����e�X�g���[�h�̑匩�o��
// GW-861011 �J���[LCD��Edy�Ή� (E) 2010/07/28(��) <Yoshiaki_Hamada@amano.co.jp> Edy�Ή�


	//--------------------------------------------------
	//		�ʃ\�t�g�p�i 50000 - 64999 �j
	//--------------------------------------------------
	LCDBM_FILE_TEXT__50000 =	50000,

};


#endif//_LCDBM_DEF_H_
// MH810100(E) K.Onodera 2019/11/11 �Ԕԃ`�P�b�g���X(GT-8700(LZ-122601)���p)
