// MH810100(S) K.Onodera 2019/11/11 �Ԕԃ`�P�b�g���X(GT-8700(LZ-122601)���p)
//[]----------------------------------------------------------------------[]
///	@brief			�p�P�b�g�ʐM�֘A�f�[�^��`
//[]----------------------------------------------------------------------[]
///	@author			m.onouchi
///	@date			Create	: 2009/09/25
///	@file			pkt_def.h
///	@version		LH068004
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
#ifndef	__PKT_DEF_H__
#define	__PKT_DEF_H__

#include "pkt_com.h"

/**
 *	LCD���W���[���Ƃ̓d���̃R�}���h�h�c�ƃT�u�R�}���h�h�c����S�o�C�g�̂h�c�R�[�h���v�Z����
 */
#define	lcdbm_command_id( id, subid )	( (( (unsigned long)id & 0x0000ffff ) << 16 ) | ( (unsigned long)subid & 0x0000ffff) )

/**
 *	16�r�b�g�J���[�l���쐬����
 */
#define	lcdbm_color_RGB( r, g, b )	((unsigned short)( (((r)&0x1f) << 11) | (((g)&0x3f) << 5) | (((b)&0x1f) << 0) ))

//--------------------------------------------------
//		�ʐM�s�Ǘv���ő�l��`
//--------------------------------------------------
// �ʐM�s�Ǌ֘A�G���[�S�����ׂ̈ɗv���̍ő�l(+1)���`
#define	PKT_COMM_FAIL_MAX	ERR_TKLSLCD_KPALV_RTY_OV+1


//--------------------------------------------------
//		�R�}���h�A�T�u�R�}���h��`
//		�i ���C����� ==> LCD���W���[�� �j
//--------------------------------------------------
/**
 *	�X�e�[�^�X�v��
 */
enum {
	LCDBM_CMD_ID_STATUS =				0x0001,		///< �u�X�e�[�^�X�v���v�̃R�}���h�h�c

	LCDBM_CMD_SUBID_CONFIG =			0x0001,		///< �u��{�ݒ�v���v�̃T�u�R�}���h�h�c

	// �R�}���h�{�T�u�R�}���h
	LCDBM_CMD_NOTIFY_CONFIG =	lcdbm_command_id( LCDBM_CMD_ID_STATUS, LCDBM_CMD_SUBID_CONFIG ),	// ��{�ݒ�v��
};

/**
 *	�@��ݒ�v��
 */
enum {
	LCDBM_CMD_ID_SETUP =				0x0004,		///< �u�@��ݒ�v���v�̃R�}���h�h�c

	LCDBM_CMD_SUBID_CLOCK =				0x0101,		///< �u���v�ݒ�v�̃T�u�R�}���h�h�c
	LCDBM_CMD_SUBID_BRIGHTNESS =		0x0201,		///< �uLCD�P�x�v�̃T�u�R�}���h�h�c
};

/**
 *	�@��ݒ�v��(���v�ʒm)
 */
enum {
	LCDBM_CMD_ID_SETUP_CLOCK =			0x0104,		///< �u�@��ݒ�v��(���v�ʒm)�v�̃R�}���h�h�c

	LCDBM_CMD_SUBID_SETUP_CLOCK =		0x0101,		///< �u���v�ʒm�v�̃T�u�R�}���h�h�c
};

/**
 *	�e�L�X�g�\��
 */
enum {
	LCDBM_CMD_ID_TEXT =					0x0013,		///< �u�e�L�X�g�\���v�̃R�}���h�h�c

	LCDBM_CMD_SUBID_TEXT =				0x0001,		///< �u�e�L�X�g�\���v�̃T�u�R�}���h�h�c

	/* �R���g���[���R�[�h */
	LCDBM_CMD_TEXT_CONTROL =			0x1b,		///< �R���g���[���R�[�h
	LCDBM_CMD_TEXT_FONT =				0x25,		///< �����t�H���g�w��
	LCDBM_CMD_TEXT_COLOR =				0x27,		///< �����F�w��
	LCDBM_CMD_TEXT_BLINK =				0x28,		///< ������u�����N�w��
	LCDBM_CMD_TEXT_CLEAR =				0x2b,		///< �N���A�w��
	LCDBM_CMD_TEXT_MENU =				0x2c,		///< ���j���[�\���w��
	LCDBM_CMD_TEXT_DIRECT =				0x2d,		///< ������w��
	LCDBM_CMD_TEXT_BACKGROUND =			0x2e,		///< �w�i�F�w��
};

#define	LCDBM_CMD_TEXT_CLMN_MAX			30			///< ������ő�T�C�Y

/**
 *	�A�i�E���X
 */
enum {
	LCDBM_CMD_ID_AUDIO =				0x0021,		///< �u�A�i�E���X�v���v�̃R�}���h�h�c

	LCDBM_CMD_SUBID_AUDIO_VOLUME = 		0x000A,		///< �u���ʒ����v�̃T�u�R�}���h�h�c
};

/**
 *	�u�U�[
 */
enum {
	// �R�}���h�h�c
	LCDBM_CMD_ID_BEEP =					0x0022,		///< �u�u�U�[�v�̃R�}���h�h�c

	// �T�u�R�}���h�h�c
	LCDBM_CMD_SUBID_BEEP_VOLUME =		0x000A,		///< �u���ʐݒ�v�̃T�u�R�}���h�h�c
	LCDBM_CMD_SUBID_BEEP_START =		0x000B,		///< �u�u�U�[���v���v�̃T�u�R�}���h�h�c
// MH810100(S) S.Takahashi 2020/02/21 #3898 �h�ƌx��A���[������Ȃ�
	LCDBM_CMD_SUBID_BEEP_ALARM = 		0x000C,		///< �u�x����v�̃T�u�R�}���h�h�c
// MH810100(E) S.Takahashi 2020/02/21 #3898 �h�ƌx��A���[������Ȃ�
};

// MH810100(S) S.Takahashi 2020/02/21 #3898 �h�ƌx��A���[������Ȃ�
// �u�U�[���
#define BEEP_KIND_ALARM		0x00					// �x��
#define BEEP_KIND_CHIME		0x01					// �h�A�m�u�`���C��

// �u�U�[���
#define BEEP_TYPE_START		0x00					// �J�n
#define BEEP_TYPE_STOP		0x01					// ��~
// MH810100(E) S.Takahashi 2020/02/21 #3898 �h�ƌx��A���[������Ȃ�

/**
 *	���Z�@��Ԓʒm����
 */
enum {
	// �R�}���h�h�c
	LCDBM_CMD_ID_STATUS_NOTICE =		0x0151,		///< �u���Z�@��Ԓʒm����v�̃R�}���h�h�c

	// �T�u�R�}���h�h�c
	LCDBM_CMD_SUBID_NOTICE_OPE =		0x0001,		///< �u����ʒm�v�̃T�u�R�}���h�h�c
	LCDBM_CMD_SUBID_NOTICE_OPN =		0x0002,		///< �u�c�x�ƒʒm�v�̃T�u�R�}���h�h�c
	LCDBM_CMD_SUBID_NOTICE_PAY =		0x0003,		///< �u���Z��Ԓʒm�v�̃T�u�R�}���h�h�c
	LCDBM_CMD_SUBID_NOTICE_DSP =		0x0004,		///< �u�|�b�v�A�b�v�\���v���v�̃T�u�R�}���h�h�c
	LCDBM_CMD_SUBID_NOTICE_ALM =		0x0005,		///< �u�x���ʒm�v�̃T�u�R�}���h�h�c
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
	LCDBM_CMD_SUBID_NOTICE_DEL =		0x0006,		///< �u�|�b�v�A�b�v�폜�v���v�̃T�u�R�}���h�h�c
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
};

/**
 *	���Z�E�������ʒm
 */
enum {
	// �R�}���h�h�c
	LCDBM_CMD_ID_PAY_INFO_NOTICE =		0x0160,		///< �u���Z�E�������ʒm�v�̃R�}���h�h�c

	// �T�u�R�}���h�h�c
	LCDBM_CMD_SUBID_PAY_REM_CHG =		0x0002,		///< �u���Z�c���ω��ʒm�v�̃T�u�R�}���h�h�c
	LCDBM_CMD_SUBID_PAY_DATA =			0x0003,		///< �u���Z�f�[�^�v�̃T�u�R�}���h�h�c
	LCDBM_CMD_SUBID_QR_CONF_CAN =		0x0006,		///< �uQR�m��E����f�[�^�v�̃T�u�R�}���h�h�c
	LCDBM_CMD_SUBID_QR_DATA_RES =		0x0009,		///< �uQR�f�[�^�����v�̃T�u�R�}���h�h�c
// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
	LCDBM_CMD_SUBID_REMOTE_TIME_START =	0x000A,		///< �u���Ɏ����w�艓�u���Z�J�n�v�̃T�u�R�}���h�h�c
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	LCDBM_CMD_SUBID_LANE =				0x000B,		///< �u���[�����j�^�f�[�^�v�̃T�u�R�}���h�h�c
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	LCDBM_CMD_SUBID_RECEIPT_DATA =		0x000D,		///< �u�̎��؃f�[�^�v�̃T�u�R�}���h�h�c
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
};

/**
 *	�����e�i���X�f�[�^
 */
enum {
	// �R�}���h�h�c
	LCDBM_CMD_ID_MNT_DATA =				0x0080,		///< �u�����e�i���X�f�[�^�v�̃R�}���h�h�c

	// �T�u�R�}���h�h�c
	LCDBM_CMD_SUBID_MNT_QR_CTRL_REQ =	0x0001,		//�uQRذ�ސ���v���v�̃T�u�R�}���h�h�c
	LCDBM_CMD_SUBID_MNT_RT_CON_REQ =	0x0010,		//�u���A���^�C���ʐM�a�ʗv���v�̃T�u�R�}���h�h�c
	LCDBM_CMD_SUBID_MNT_DC_CON_REQ =	0x0011,		//�uDC-NET�ʐM�a�ʗv���v�̃T�u�R�}���h�h�c
};

//--------------------------------------------------
//		�R�}���h�A�T�u�R�}���h��`
//		�i LCD���W���[�� ==> ���C����� �j
//--------------------------------------------------
/**
 *	�X�e�[�^�X�ʒm
 */
enum {
	// �R�}���h�h�c
	LCDBM_RSP_ID_NOTIFY =				0x0101,		///< �u�X�e�[�^�X�ʒm�v�̃R�}���h�h�c

	// �T�u�R�}���h�h�c
	LCDBM_RSP_SUBID_CONFIG =			0x0001,		///< �u��{�ݒ艞���v�̃T�u�R�}���h�h�c

	// �R�}���h�{�T�u�R�}���h
	LCDBM_RSP_NOTIFY_CONFIG =	lcdbm_command_id( LCDBM_RSP_ID_NOTIFY, LCDBM_RSP_SUBID_CONFIG ),	// ��{�ݒ艞��
};

/**
 *	�G���[�ʒm
 */
enum {
	// �R�}���h�h�c
	LCDBM_RSP_ID_ERROR =				0x0102,		///< �u�G���[�ʒm�v�̃R�}���h�h�c

	// �T�u�R�}���h�h�c
	LCDBM_RSP_SUBID_ERROR =				0x0000,		///< �u�G���[�ʒm�v�̃T�u�R�}���h�h�c

	// �R�}���h�{�T�u�R�}���h
	LCDBM_RSP_ERROR =	lcdbm_command_id( LCDBM_RSP_ID_ERROR, LCDBM_RSP_SUBID_ERROR ),	// �G���[�ʒm
};

/**
 *	�e���L�[����
 */
enum {
	// �R�}���h�h�c
	LCDBM_RSP_ID_TENKEY =				0x0152,		///< �u�e���L�[�����v�̃R�}���h�h�c

	// �T�u�R�}���h�h�c
	LCDBM_RSP_SUBID_TK_KEYDOWN =		0x0001,		///< �u�e���L�[�������v�̃T�u�R�}���h�h�c

	// �R�}���h�{�T�u�R�}���h
	LCDBM_RSP_TENKEY_KEYDOWN =	lcdbm_command_id( LCDBM_RSP_ID_TENKEY, LCDBM_RSP_SUBID_TK_KEYDOWN ),	// �e���L�[�������
};

/**
 *	���Z�@��Ԓʒm����
 *	�� LCD�̐��Z�@�������R�}���h�ׁ̈A�R�}���h�h�c/�T�u�R�}���h�h�c�́u���C����� ==> LCD���W���[���v����`�Q��
 */
enum {
	// �R�}���h�{�T�u�R�}���h
	LCDBM_RSP_NOTICE_OPE =	lcdbm_command_id( LCDBM_CMD_ID_STATUS_NOTICE, LCDBM_CMD_SUBID_NOTICE_OPE ),	// ����ʒm
};

/**
 *	���Z�E�������ʒm
 */
enum {
	 LCDBM_RSP_ID_PAY_INFO_NOTICE =		0x0160,		///< �u���Z�E�������ʒm�v�̃R�}���h�h�c

	// �T�u�R�}���h�h�c
	LCDBM_RSP_SUBID_IN_INFO =			0x0001,		///< �u���ɏ��v�̃T�u�R�}���h�h�c
	LCDBM_RSP_SUBID_PAY_DATA_RES =		0x0005,		///< �u���Z�����f�[�^�v�̃T�u�R�}���h�h�c
	LCDBM_RSP_SUBID_QR_CONF_CAN_RES =	0x0007,		///< �uQR�m��E����f�[�^�����v�̃T�u�R�}���h�h�c
	LCDBM_RSP_SUBID_QR_DATA =			0x0008,		///< �uQR�f�[�^�v�̃T�u�R�}���h�h�c
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	LCDBM_RSP_SUBID_LANE_RES =			0x000C,		///< �u���[�����j�^�f�[�^�����v�̃T�u�R�}���h�h�c
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	LCDBM_RSP_SUBID_RECEIPT_DATA_RES =	0x000E,		///< �u�̎��؃f�[�^�����v�̃T�u�R�}���h�h�c
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

	// �R�}���h�{�T�u�R�}���h
	LCDBM_RSP_IN_INFO =			lcdbm_command_id( LCDBM_RSP_ID_PAY_INFO_NOTICE, LCDBM_RSP_SUBID_IN_INFO ),			// ���ɏ��
	LCDBM_RSP_PAY_DATA_RES =	lcdbm_command_id( LCDBM_RSP_ID_PAY_INFO_NOTICE, LCDBM_RSP_SUBID_PAY_DATA_RES ),		// ���Z�����f�[�^
	LCDBM_RSP_QR_CONF_CAN_RES =	lcdbm_command_id( LCDBM_RSP_ID_PAY_INFO_NOTICE, LCDBM_RSP_SUBID_QR_CONF_CAN_RES ),	// QR�m��E����f�[�^����
	LCDBM_RSP_QR_DATA =			lcdbm_command_id( LCDBM_RSP_ID_PAY_INFO_NOTICE, LCDBM_RSP_SUBID_QR_DATA ),			// QR�f�[�^
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	LCDBM_RSP_LANE_RES =		lcdbm_command_id( LCDBM_RSP_ID_PAY_INFO_NOTICE, LCDBM_RSP_SUBID_LANE_RES ),			// ���[�����j�^�f�[�^����
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	LCDBM_RSP_RECEIPT_DATA_RES = lcdbm_command_id(LCDBM_RSP_ID_PAY_INFO_NOTICE, LCDBM_RSP_SUBID_RECEIPT_DATA_RES),	// �̎��؃f�[�^����
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
};

/**
 *	�����e�i���X�f�[�^
 */
enum {
	// �R�}���h�h�c
	LCDBM_RSP_ID_MNT_DATA_LCD =			0x0180,		///< �u�����e�i���X�f�[�^�v(LCD�����Z�@����)�̃R�}���h�h�c

	// �T�u�R�}���h�h�c
	LCDBM_RSP_SUBID_MNT_QR_CTRL_RES =	0x0001,		///< �uQR���[�_���䉞���v�̃T�u�R�}���h�h�c
	LCDBM_RSP_SUBID_MNT_QR_RD_RSLT =	0x0002,		///< �uQR�ǎ挋�ʁv�̃T�u�R�}���h�h�c
	LCDBM_RSP_SUBID_MNT_RT_CON_RSLT =	0x0010,		///< �u���A���^�C���ʐM�a�ʌ��ʁv�̃T�u�R�}���h�h�c
	LCDBM_RSP_SUBID_MNT_DC_CON_RSLT =	0x0011,		///< �uDC-NET�ʐM�a�ʌ��ʁv�̃T�u�R�}���h�h�c
	LCDBM_CMD_SUBID_AUDIO_START =		0x000B,		///< �u�J�n�v���v�̃T�u�R�}���h�h�c
	LCDBM_CMD_SUBID_AUDIO_END =			0x000C,		///< �u�I���v���v�̃T�u�R�}���h�h�c

	// �R�}���h�{�T�u�R�}���h
	LCDBM_RSP_MNT_QR_CTRL_RES =	lcdbm_command_id( LCDBM_RSP_ID_MNT_DATA_LCD, LCDBM_RSP_SUBID_MNT_QR_CTRL_RES ),	// QR���[�_���䉞��
	LCDBM_RSP_MNT_QR_RD_RSLT =	lcdbm_command_id( LCDBM_RSP_ID_MNT_DATA_LCD, LCDBM_RSP_SUBID_MNT_QR_RD_RSLT ),	// QR�ǎ挋��
	LCDBM_RSP_MNT_RT_CON_RSLT =	lcdbm_command_id( LCDBM_RSP_ID_MNT_DATA_LCD, LCDBM_RSP_SUBID_MNT_RT_CON_RSLT ),	// ���A���^�C���ʐM�a�ʌ���
	LCDBM_RSP_MNT_DC_CON_RSLT =	lcdbm_command_id( LCDBM_RSP_ID_MNT_DATA_LCD, LCDBM_RSP_SUBID_MNT_DC_CON_RSLT ),	// DC-NET�ʐM�a�ʌ���
};
// MH810100(S) Y.Yamauchi 2019/11/28 �Ԕԃ`�P�b�g���X(�����e�i���X)
enum{
	LCDBM_QR_CTRL_VER = 0,					// �ް�ޮݗv��
	LCDBM_QR_CTRL_RD_STA ,					// �ǎ�J�n
	LCDBM_QR_CTRL_RD_STP,					// �ǎ��~
	LCDBM_QR_CTRL_INIT_REQ,					// �������v��
};
// MH810100(E) Y.Yamauchi 2019/11/28 �Ԕԃ`�P�b�g���X(�����e�i���X)

//--------------------------------------------------
//		����R�[�h��`
//--------------------------------------------------
typedef enum {
	LCDBM_OPCD_PAY_UNDEFINED	= 0,	//  0:(����`)
	LCDBM_OPCD_PAY_STP,					//  1:���Z���~
	LCDBM_OPCD_PAY_STP_RES,				//  2:���Z���~����
	LCDBM_OPCD_PAY_STA,					//  3:���Z�J�n
	LCDBM_OPCD_PAY_STA_RES,				//  4:���Z�J�n����
	LCDBM_OPCD_MAN_DTC,					//  5:�l�̌��m
	LCDBM_OPCD_DOOR_STATUS,				//  6:�h�A���
	LCDBM_OPCD_RCT_ISU,					//  7:�̎��ؔ��s
	LCDBM_OPCD_RMT_PAY_STA,				//  8:���u���Z�J�n
	LCDBM_OPCD_RMT_PAY_STA_RES,			//  9:���u���Z�J�n����
	LCDBM_OPCD_Dummy0_Send,				// 10:
	LCDBM_OPCD_STA_NOT,					// 11:�N���ʒm
	LCDBM_OPCD_STA_CMP_NOT,				// 12:�N�������ʒm
	LCDBM_OPCD_PRM_UPL_NOT,				// 13:�ݒ�A�b�v���[�h�ʒm
	LCDBM_OPCD_PRM_UPL_REQ,				// 14:�ݒ�A�b�v���[�h�v��
	LCDBM_OPCD_RST_REQ,					// 15:�t�F�[�Y���Z�b�g�v��
	LCDBM_OPCD_Dummy1_Recv,				// 16:
	LCDBM_OPCD_Dummy1_Send,				// 17:
	LCDBM_OPCD_MNT_STA_REQ,				// 18:�����e�i���X�J�n�v��
	LCDBM_OPCD_MNT_STA_RES,				// 19:�����e�i���X�J�n����
	LCDBM_OPCD_MNT_END_REQ,				// 20:�����e�i���X�I���v��
	LCDBM_OPCD_MNT_END_RES,				// 21:�����e�i���X�I������
	LCDBM_OPCD_PAY_CMP_NOT,				// 22:���Z�����ʒm
	LCDBM_OPCD_QR_STP_REQ,				// 23:QR��~�v��
	LCDBM_OPCD_QR_STP_RES,				// 24:QR��~����
	LCDBM_OPCD_QR_STA_REQ,				// 25:QR�J�n�v��
	LCDBM_OPCD_QR_STA_RES,				// 26:QR�J�n����
	LCDBM_OPCD_PAY_GUIDE_END_NOT,		// 27:���Z�ē��I���ʒm
	LCDBM_OPCD_PAY_GUIDE_END_RES,		// 28:���Z�ē��I������
	LCDBM_OPCD_RESET_NOT,				// 29:���Z�b�g�ʒm
	LCDBM_OPCD_RESTORE_REQUEST,			// 30:�����f�[�^�˗�(RXM��LCD)
	LCDBM_OPCD_RESTORE_RESULT,			// 31:�����f�[�^����(RXM��LCD)
	LCDBM_OPCD_RESTORE_NOTIFY,			// 32:�����f�[�^    (RXM��LCD) �������I�f�[�^
	LCDBM_OPCD_CHG_IDLE_DISP_NOT,		// 33:�ҋ@��ʒʒm  (RXM��LCD)
	LCDBM_OPCD_REJECT_LEVER,			// 34:���W�F�N�g�ʒm(RXM��LCD)
// MH810100(S) K.Onodera 2020/03/31 �Ԕԃ`�P�b�g���X(#3941 �̎��؎������s��A�{�^���������ł��Ă��܂�)
	LCDBM_OPCD_RCT_AUTO_ISSUE,			// 35:�̎��ؔ��s�ς�(RXM��LCD)
// MH810100(E) K.Onodera 2020/03/31 �Ԕԃ`�P�b�g���X(#3941 �̎��؎������s��A�{�^���������ł��Ă��܂�)
// MH810100(S) K.Onodera  2020/04/01 #4040 �Ԕԃ`�P�b�g���X(���ɏ��NG�Ń|�b�v�A�b�v�\�����Ȃ��ꍇ������)
	LCDBM_OPCD_IN_INFO_NG,				// 36:���ɏ��NG����
// MH810100(E) K.Onodera  2020/04/01 #4040 �Ԕԃ`�P�b�g���X(���ɏ��NG�Ń|�b�v�A�b�v�\�����Ȃ��ꍇ������)
// MH810100(S) 2020/06/08 #4205�y�A���]���w�E�����z�Z���^�[�Ƃ̒ʐM�f����QR�Ǎ������Z��������Ɖ�ʂ��ł܂�(No.02-0009)
	LCDBM_OPCD_QR_NO_RESPONSE,			// 37:QR������̓��ɏ��͗��Ȃ��ʒm
// MH810100(E) 2020/06/08 #4205�y�A���]���w�E�����z�Z���^�[�Ƃ̒ʐM�f����QR�Ǎ������Z��������Ɖ�ʂ��ł܂�(No.02-0009)
	LCDBM_OPCD_QR_DISCOUNT_OK,			// 38:QR��������OK�ʒm
// MH810100(S) S.Nishimoto 2020/07/14 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
	LCDBM_OPCD_CARD_PAY_STATE,			// 39:�J�[�h���Ϗ�����Ԓʒm
// MH810100(E) S.Nishimoto 2020/07/14 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
// MH810100(S) S.Nishimoto 2020/08/25 �Ԕԃ`�P�b�g���X(#4754 ����NG��̂Ƃ肯�����ɃJ�[�h�������҂����s��Ȃ�)
	LCDBM_OPCD_CARD_EJECTED,			// 40:�N���W�b�g�J�[�h�����ʒm
// MH810100(E) S.Nishimoto 2020/08/25 �Ԕԃ`�P�b�g���X(#4754 ����NG��̂Ƃ肯�����ɃJ�[�h�������҂����s��Ȃ�)
// MH810100(S) S.Nishimoto 2020/09/16 �Ԕԃ`�P�b�g���X(#4867 ����J�E���g(LCD�N������)�̃N���A���e�����f����Ȃ�)
	LCDBM_OPCD_CLEAR_COUNT,				// 41:����J�E���g�N���A
// MH810100(E) S.Nishimoto 2020/09/16 �Ԕԃ`�P�b�g���X(#4867 ����J�E���g(LCD�N������)�̃N���A���e�����f����Ȃ�)
// MH810103(s) �d�q�}�l�[�Ή� �d�q�}�l�[�u�����h�I�����̈ē�����
	LCDBM_OPCD_BRAND_SELECTED,			// 42:�u�����h�I��ʒm
	LCDBM_OPCD_BRAND_SELECT_CANCEL,		// 43:�u�����h�I���L�����Z��
// MH810103(e) �d�q�}�l�[�Ή� �d�q�}�l�[�u�����h�I�����̈ē�����
// MH810103(s) �d�q�}�l�[�Ή� �ҋ@��ʂł̎c���Ɖ��
	LCDBM_OPCD_INQUIRY_PAY_AFTER,		// 44:�c���Ɖ�ےʒm
// MH810103(e) �d�q�}�l�[�Ή� �ҋ@��ʂł̎c���Ɖ��
// MH810103(s) �d�q�}�l�[�Ή� #5555�yWAON�V���O���Ή��z���Z�J�n�Łu������܂ł�������^�b�`���ĉ������v���ē���������
	LCDBM_OPCD_EMONEY_SINGLE,			// 45:�d�q�}�l�[�V���O���ʒm
// MH810103(e) �d�q�}�l�[�Ή� #5555�yWAON�V���O���Ή��z���Z�J�n�Łu������܂ł�������^�b�`���ĉ������v���ē���������
// GG129000(S) H.Fujinaga 2022/12/09 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i#6411 �d�q�}�l�[�ݒ�Ȃ��̐ݒ�ŃT�[�r�X��(QR)�Ő��Z�������ɗ̎��ؔ��s�{�^�����^�b�`�\�ɂȂ��Ă���j
	LCDBM_OPCD_EJA_USE,					// 46:�d�q�W���[�i���ڑ���Ԓʒm
// GG129000(E) H.Fujinaga 2022/12/09 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i#6411 �d�q�}�l�[�ݒ�Ȃ��̐ݒ�ŃT�[�r�X��(QR)�Ő��Z�������ɗ̎��ؔ��s�{�^�����^�b�`�\�ɂȂ��Ă���j
// GG129000(S) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
	LCDBM_OPCD_SEARCH_TYPE,				// 47:�����^�C�v�ʒm(0:�ԔԌ���,1:��������,2:QR����)
// GG129000(E) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
// GG129000(S) T.Nagai 2023/09/28 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
	LCDBM_OPCD_RTM_REMOTE_PAY_REQ,		// 48:���u���Z�i���A���^�C���j�J�n�v��
	LCDBM_OPCD_RTM_REMOTE_PAY_RES,		// 49:���u���Z�i���A���^�C���j�J�n����
	LCDBM_OPCD_RTM_REMOTE_PAY_CAN,		// 50:���u���Z�i���A���^�C���j��t���~
// GG129000(E) T.Nagai 2023/09/28 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
// GG129000(S) H.Fujinaga 2022/12/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
	LCDBM_OPCD_POP_PAYQR = 90,			// 90:QR���Ԍ����s�_�C�A���O(�̎��؃{�^���Ȃ�)
	LCDBM_OPCD_POP_PAYQRBUTTON,			// 91:QR���Ԍ����s�_�C�A���O(�̎��؃{�^������)
	LCDBM_OPCD_POP_PAYQRNONE,			// 92:QR���Ԍ����s�_�C�A���O�\���Ȃ�
// GG129000(E) H.Fujinaga 2022/12/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
// GG129000(S) H.Fujinaga 2023/02/14 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
	LCDBM_OPCD_POP_PAYQRNORMAL,			// 94:QR���Ԍ����s�_�C�A���O(����)
// GG129000(E) H.Fujinaga 2023/02/14 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
} eOPE_NOTIF_CODE;

//--------------------------------------------------
//		�x�Ɨ��R�R�[�h��`
//--------------------------------------------------
enum {
	LCDBM_OPNCLS_RSN_UNDEFINED			= 0,	// (����`)
	LCDBM_OPNCLS_RSN_FORCED,					// �����c�x��
	LCDBM_OPNCLS_RSN_OUT_OF_HOURS,				// �c�Ǝ��ԊO
	LCDBM_OPNCLS_RSN_BANKNOTE_FULL,				// �������ɖ��t
	LCDBM_OPNCLS_RSN_COIN_SAFE_FULL,			// �R�C�����ɖ��t
	LCDBM_OPNCLS_RSN_BANKNOTE_DROP,				// �������[�_�[�E��
	LCDBM_OPNCLS_RSN_NO_CHANGE,					// �ޑK�؂�
	LCDBM_OPNCLS_RSN_COINMECH_COMM_FAIL,		// �R�C�����b�N�ʐM�s��
	LCDBM_OPNCLS_RSN_SEND_BUFF_FULL,			// ���M�o�b�t�@�t��
	LCDBM_OPNCLS_RSN_RESERVE,					// (�\��)
	LCDBM_OPNCLS_RSN_SIGNAL,					// �M���ɂ��x��
	LCDBM_OPNCLS_RSN_COMM,						// �ʐM�ɂ��x��
};

//--------------------------------------------------
//		���ʃR�[�h��`
//--------------------------------------------------
enum{
	LCDBM_RESUTL_OK						= 0,	// ����
	LCDBM_RESUTL_NG,							// �ُ�
	LCDBM_RESUTL_TIMEOUT,						// �^�C���A�E�g
};

//--------------------------------------------------
//		�|�b�v�A�b�v�\���v�����
//--------------------------------------------------
typedef enum{
	POP_AZUKARI_PAY = 1,// 1:�a��ؔ��s�i���Z�������j
	POP_AZUKARI_RET,	// 2:�a��ؔ��s�i�����߂����j
	POP_FAIL_REDUCTION,	// 3:���z���s
	POP_OVER_LIMIT,		// 4:���x����
	POP_INQUIRY_NG,		// 5:�Ɖ�NG
// MH810103(s) �d�q�}�l�[�Ή� �G���[�|�b�v�A�b�v�ύX
//	POP_CREDIT_SV_NG,	// 6:�ڼޯĻ��ވُ�
	POP_EMONEY_INQUIRY_NG,		// 6:�Ɖ�NG(�N���W�b�g�ȊO)
// MH810103(e) �d�q�}�l�[�Ή� �G���[�|�b�v�A�b�v�ύX
	POP_SHORT_BALANCE,	// 7:�c���s��
	POP_UNABLE_PAY,		// 8:���Z�s��
	POP_UNABLE_QR,		// 9:QR���p�s��
// MH810100(S) K.Onodera  2020/04/01 #4040 �Ԕԃ`�P�b�g���X(���ɏ��NG�Ń|�b�v�A�b�v�\�����Ȃ��ꍇ������)
	POP_UNABLE_DISCOUNT,// 10:�����s��
// MH810100(E) K.Onodera  2020/04/01 #4040 �Ԕԃ`�P�b�g���X(���ɏ��NG�Ń|�b�v�A�b�v�\�����Ȃ��ꍇ������)
// MH810105(S) MH364301 �C���{�C�X�Ή�
	POP_RECIPT_OUT_ERROR,	// 11:�̎��ؔ��s���s
// MH810105(E) MH364301 �C���{�C�X�Ή�
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	POP_QR_RECIPT,		// 12:QR�̎���
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
}ePOP_DISP_KIND;

// GG129000(S) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
//--------------------------------------------------
//		�|�b�v�A�b�v�폜�v�����
//--------------------------------------------------
typedef enum{
	POPDEL_RECIPT_OUT = 1,		// 1:�̎��ؔ��sPOP
}ePOP_DISP_DEL_KIND;
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j

#pragma pack
//-------------------------------------------------------------------------------------------------------------------------------------------------------
//���ޏ��		// �� = GT4100�Ŏg�p����
//-------------------------------------------------------------------------------------------------------------------------------------------------------

// �J�[�h�ԍ����
#define CARD_TYPE_PARKING_TKT	101
#define CARD_TYPE_PASS			102
// GG129000(S) T.Nagai 2023/01/22 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�Ԕԃf�[�^ID�擾�����C���j
#define CARD_TYPE_INOUT_ID		103
// GG129000(E) T.Nagai 2023/01/22 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�Ԕԃf�[�^ID�擾�����C���j
// MH810100(S) K.Onodera 2020/02/07 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^�ԔԑΉ�)
#define CARD_TYPE_CAR_NUM		203
// MH810100(E) K.Onodera 2020/02/07 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^�ԔԑΉ�)

#define ONL_MAX_CARDNUM			6
#define ONL_MAX_SETTLEMENT		10
#define ONL_MAX_DISC_NUM		25
#define ONL_DISC_MAX			25
// MH810100(S) K.Onodera 2020/02/07 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
#define ONL_MAX_MEDIA_NUM		32
// MH810100(E) K.Onodera 2020/02/07 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)

typedef struct 
{
	ushort 	shYear;			// �N	2000�`2099(��������)
	uchar 	byMonth;		// ��	1�`12(��������)
	uchar 	byDay;			// ��	1�`31(��������)
	uchar 	byHours;		// ��	0�`23(��������)
	uchar 	byMinute;		// ��	0�`59(��������)
	uchar 	bySecond;		// �b	0�`59(��������)
} stDatetTimeYtoSec_t;		// �N���������b

typedef struct _stShisetuInfo_t
{
	ulong		code;			// �{�݃R�[�h
	ushort		count;			// �F�؊m�背�V�[�g����
	ulong		amount;			// �݌v���z
} stShisetuInfo_t;

typedef struct 
{
	ushort 	RevNoInfo;			// ���ޏ��Rev.No.					10�`65535
	uchar 	MasterOnOff;		// ���ޏ�� Ͻ�����  �L��			0:���1:�L	<- �L
	uchar 	ReserveOnOff;		// ���ޏ�� �\����  �L��			0:���1:�L	<- �L
	uchar 	PassOnOff;			// ���ޏ�� ����������  �L��		0:���1:�L	<- �L
	uchar 	ZaishaOnOff;		// ���ޏ�� �ݎԏ��  �L��			0:���1:�L	<- �L

	ushort 	MasterSize;			// ���ޏ�񻲽�  Ͻ�����			0�`65535	<- sizeof(stMasterInfo_t)
	ushort 	ReserveSize;		// ���ޏ�񻲽�  �\����			0�`65535	<- sizeof(stReserveInfo_t)
	ushort 	PassSize;			// ���ޏ�񻲽�  ����������		0�`65535	<- sizeof(stPassInfo_t)
	ushort 	ZaishaSize;			// ���ޏ�񻲽�  �ݎԏ��			0�`65535	<- sizeof(stZaishaInfo_t)
} stCardInformation_t;			// ���ޏ��

typedef struct 
{
	uchar	PayResultInfo;		// �v������  �����v�Z				0:��	1:�L
	uchar	TotalPayInfo;		// �v������  �݌v���z�⍇��			0:��	1:�L
	uchar 	MasterOnOff;		// �v������ Ͻ�����  �L��			0:��	1:�L
	uchar 	ReserveOnOff;		// �v������ �\����  �L��			0:��	1:�L
	uchar 	PassOnOff;			// �v������ ����������  �L��		0:��	1:�L
	uchar 	ZaishaOnOff;		// �v������ �ݎԏ��  �L��			0:��	1:�L
// MH810100(S) 2020/05/29 �Ԕԃ`�P�b�g���X(#4196)
	uchar	CalcResult;			// �v������  �����v�Z�r��			0=�v���Ȃ��A1=�r�������A2=�r�����i���̒[���Ő��Z���j
// MH810104(S) R.Endo 2021/09/21 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6001 �y�A�������z���A���^�C���ʐM - �Z���^�[�⍇�����ʃf�[�^����M���Ă����Z�J�n�ł��Ȃ�
// 	uchar	Reserve;			// �\��
// MH810104(E) R.Endo 2021/09/21 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6001 �y�A�������z���A���^�C���ʐM - �Z���^�[�⍇�����ʃf�[�^����M���Ă����Z�J�n�ł��Ȃ�
// MH810100(E) 2020/05/29 �Ԕԃ`�P�b�g���X(#4196)
// GG124100(S) R.Endo 2022/06/13 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
	uchar 	FeeCalcOnOff;		// �v������ �����v�Z����  �L��		0:��	1:�L
// GG124100(E) R.Endo 2022/06/13 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)

	ushort 	PayResultSize;		// �v�����ʻ���  �����v�Z			0�`65535
	ushort 	TotalPaySize;		// �v�����ʻ���  �݌v���z�⍇��		0�`65535
	ushort 	MasterSize;			// �v�����ʻ���  Ͻ�����			0�`65535
	ushort 	ReserveSize;		// �v�����ʻ���  �\����			0�`65535
	ushort 	PassSize;			// �v�����ʻ���  ����������		0�`65535
	ushort 	ZaishaSize;			// �v�����ʻ���  �ݎԏ��			0�`65535
// GG124100(S) R.Endo 2022/06/13 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
	ushort 	FeeCalcSize;		// �v�����ʻ���  �����v�Z����		0�`65535
// GG124100(E) R.Endo 2022/06/13 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
} stRecvCardInformation_t;		// ���ޏ��_�v������

typedef struct 
{
	ulong 		ParkingLotNo;	// ���ԏꇂ(0�`999999)
	ushort 		CardType;		// �ԍ����(0�`65000)
	uchar 		byCardNo[32];	// �ԍ�(32�� ��ASCII(���l,�c���Null�Œ�))
} stParkKindNum_t;

typedef struct {
	uchar byDispCardNo[32];		// �\�����ޔԍ�(ASCII(���l,�c���Null�Œ�)�ꗥ�[�����ž�Ă���)
} stDispCardData_t;

typedef struct 
{
	ushort 	shYear;				// �N(2000�`2099)
	uchar 	byMonth;			// ��(1�`12)
	uchar 	byDay;				// ��(1�`31)
} stDate_YYMD_t;

typedef struct 
{
	ulong 	upper;				// �\����Ǘ��ԍ� ���6��	0�`999999
	ulong 	lower;				// �\����Ǘ��ԍ� ����7��	0�`9999999
} stReserveManageNo_t;

typedef struct 
{
	uchar	RsvFeeType;			// �\�񗿋��������		0:�����Ȃ��1:�������� �2:�[������
	uchar	RsvFeeStatus;		// �\�񗿋������ð��	0:������ 1:�����ς�
	ulong	RsvFee;				// �\�񗿋����z			0�`999999
}	stReserveFeeInfo_t;			// �\�񗿋�(�\�񗿋��������, �\�񗿋������ð��, �\�񗿋����z)

typedef struct 
{
	uchar	PenaType;			// ����è�����������	0:�����Ȃ��1:�����y�щ����2:�����̂ݤ3:�����̂�
	uchar	PenaStatus;			// ����è���������ð��	0:������ 1:�����ɂ�蒥���ς� 2:�����ɂ�蒥���ς�
	ulong	PenaUnit;			// ����è��(�P��)		0�`999999 1�������������è���̒P��
	ulong	PenaFee;			// ����è�����������z	0�`999999�[���������������z�
	ushort	PenaFinishDates;	// ����è���������ςݓ���		0�`999
	ulong	PenaTotalDates;		// ����è���������ςݍ��v���z	0�`999999
}	stPenaltyFeeInfo_t;			// ����è����(����è�����������, ����è���������ð��, ����è��(�P��), ����è�����������z, ����è���������ςݓ���, ����è���������ςݍ��v���z)

typedef struct
{
	ushort 	shYear;				// �N	2000�`2099
	uchar 	byMonth;			// ��	1�`12
	uchar 	byDay;				// ��	1�`31
	uchar 	byHours;			// ��	0�`23
	uchar 	byMinute;			// ��	0�`59
} stDate_YYMDHM_t;				// �N��������

typedef struct 
{
	ulong 				ParkingNo;		// ���ԏꇂ(0�`999999)
	ushort 				MachineKind;	// �@����(0�`999)
	ushort 				MachineNo;		// �@�B��(0�`999)
	stDatetTimeYtoSec_t	dtTimeYtoSec;	// �N���������b
	uchar				Reserve1;		// �\��		0�Œ�
} stDateParkTime_t;

typedef struct 
{
	ushort 	CargeType;			// ���Z�敪		0=�Ȃ�,1=�ڼޯ�,2=��ʌn�d�q�Ȱ(Suica,PASMO,ICOCA��),
								//				3=Edy,4=PiTaPa,5=WAON,6=nanaco,7=SAPICA,8 / 9 =(�\��),
								//				10=ETC,11= ETC-EC ��90�ȍ~=�ʑΉ�
	ulong 	CargeFee;			// ���Z���z(0�`999999)
} stSettlement_t;

typedef struct {
	ushort 	MachineNo;			// �Ïؔԍ� �����@�ԍ�(0�`99)
	ulong 	Password;			// �Ïؔԍ� �ԍ�(0�`99999999)
} stPasswordNo_t;

typedef struct 
{
	ulong	DiscParkNo;				// ���ԏꇂ(0�`999999)
	ushort 	DiscSyu;				// ���(0�`9999)
	ushort 	DiscCardNo;				// ���ދ敪(0�`65000)
	uchar 	DiscNo;					// �敪(0�`9)
	uchar 	DiscCount;				// ����(0�`99)
// GG124100(S) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�J�[�h���Rev.No.11)
// 	ulong 	Discount;				// ���z/����(0�`999999)
	ulong 	Discount;				// ���z(0�`999999)
	ulong 	DiscTime;				// ����(0�`999999)
// GG124100(E) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�J�[�h���Rev.No.11)
	ushort 	DiscInfo;				// �������(0�`65000)
	ushort 	DiscCorrType;			// �Ή����ގ��(0�`65000)
	uchar 	DiscStatus;				// �ð��(0�`9)
	uchar 	DiscFlg;				// ������(0�`9)
} stDiscount_t;

typedef struct 
{
	ulong	DiscParkNo;				// ���ԏꇂ(0�`999999)
	ushort 	DiscSyu;				// ���(0�`9999)
	ushort 	DiscCardNo;				// ���ދ敪(0�`65000)
	uchar 	DiscNo;					// �敪(0�`9)
	uchar 	DiscCount;				// ����(0�`99)
// MH810100(S) 2020/05/28 �Ԕԃ`�P�b�g���X(#4196)
//	ulong 	DiscountM;				// ���z(0�`999999)
//	ulong 	DiscountT;				// ����(0�`999999)
	ulong 	Discount;				// ���z/����(0�`999999)
	ulong	UsedDisc;				// �����������p���̎g�p�ς݊����i���z/���ԁj(0�`999999)
// MH810100(E) 2020/05/28 �Ԕԃ`�P�b�g���X(#4196)
	ushort 	DiscInfo;				// �������(0�`65000)
	ushort 	DiscCorrType;			// �Ή����ގ��(0�`65000)
	uchar 	DiscStatus;				// �ð��(0�`9)
	uchar 	DiscFlg;				// ������(0�`9)
} stDiscount2_t;

typedef struct 
{
	ulong	DiscParkNo;				// ���ԏꇂ(0�`999999)
	ushort 	DiscSyu;				// ���(0�`9999)
	ushort 	DiscCardNo;				// ���ދ敪(0�`65000)
	ushort 	DiscNo;					// �敪(0�`9)
	ushort 	DiscInfo;				// �������(0�`65000)
} stDiscount3_t;

// GG124100(S) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
typedef struct 
{
	ulong	DiscParkNo;				// ���� ���ԏꇂ
	ushort	DiscSyu;				// ���� ���
	ushort	DiscCardNo;				// ���� �J�[�h�敪
	uchar	DiscNo;					// ���� �敪
	uchar	DiscCount;				// ���� ����
	ulong	PrevUsageDiscAmount;	// ���� �O�񐸎Z�܂ł̎g�p�ς݊������z(����)
	ulong	PrevUsageDiscTime;		// ���� �O�񐸎Z�܂ł̎g�p�ς݊�������(����)
	ulong	DiscAmount;				// ���� ����̊������z(����)
	ulong	DiscTime;				// ���� ����̊�������(����)
	ulong	DiscAmountSetting;		// ���� �ݒ���z(�������z�̐ݒ�l�~����)
	ulong	DiscTimeSetting;		// ���� �ݒ莞��(�������Ԃ̐ݒ�l�~����)
	uchar	DiscPercentSetting;		// ���� �ݒ�%(����%�̐ݒ�l)
	uchar	FeeKindSwitchSetting;	// ���� �ݒ��ʐ؊���Ԏ�
	ushort	DiscInfo;				// ���� �������
	ushort	DiscCorrType;			// ���� �Ή��J�[�h���
	uchar	DiscStatus;				// ���� �X�e�[�^�X
	uchar	DiscFlg;				// ���� ������
} stDiscount4_t;
// GG124100(E) R.Endo 2022/08/08 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)

typedef struct 
{
	stDiscount3_t		Discount;	// ������{���
	DATE_YMDHMS			Date;		// �F�؊m��N���������b
	uchar				Reserve;	// �\��
} stWaribikiInfo_t;

typedef struct 
{
	uchar 	PassKind;				// ��������		0=�Ȃ��1�`15
	ulong 	PassNum;				// �������			0=�Ȃ��1�`999999
	uchar 	PassStatus;				// ��������޽ð��	0=�Ȃ��1�`99 (1=�L���2=����)
} stPassKindNumSts_t;				// ��������, �������, ��������޽ð��

//-------------------------------------------------------------------------------------------------------------------------------------------------------
// �����v�Z���ʏ��(684 Byte)
// ���uر���ђʐM�̢�����⍇���ް�(ID:XX)��ɂĢ�v�� �����v�Z����1=�v�����裎��ɾ�������Ă���
//====================================================================================================
typedef struct _stPayResultInfo_t
{
	ushort 					shPayResult;						// �����v�Z����		0=���������1�ȏ�=���s
	stParkKindNum_t			dtPayMedia;							// ���Z�}��(���ԏꇂ, ���, �ԍ�)
	uchar					FeeType;							// �������(0�`12)
	stPassKindNumSts_t		dtPassKindNumSts;					// ��������, �������, ��������޽ð��
	uchar					Reserve1;							// �\��(���ޒ����p)	1	0�Œ�
	stDate_YYMD_t			dtEffectStart;						// �L���J�n(�N, ��, ��)
	stDate_YYMD_t			dtEffectStop;						// �L���I��(�N, ��, ��)
	ulong					ParkingTime;						// ���Ԏ���(0�`999999(���P��))
	ulong					ParkingFee;							// ���ԗ���(0�`999990(10�~�P��))
	ulong					Seikyuugaku;						// �����z(0�`999990(10�~�P��))
	stDatetTimeYtoSec_t		dtNyuukoYtoSec;						// ���ɔN���������b
	uchar					Reserve2;							// �\��(���ޒ����p)	1	0�Œ�
	stDatetTimeYtoSec_t		dtPrivSeisanYtoSec;					// �O�񐸎Z�N���������b
	uchar					Reserve3;							// �\��(���ޒ����p)	1	0�Œ�
	stDiscount2_t			stDiscountInfo[ONL_MAX_DISC_NUM];	// ����01�`25
} stPayResultInfo_t;

//-------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// �݌v���z���(600 Byte)
// ���uر���ђʐM�̢�����⍇���ް�(ID:XX)��ɂĢ���v�� �݌v���z�⍇������1=�v�����裎��ɾ�������Ă���
//====================================================================================================
typedef struct _stTotalPayInfo_t
{
	stShisetuInfo_t		stShisetuInfo[10];					// �{��(�{�ݺ���, �F�؊m��ڼ�Ė���, �݌v���z)
	stWaribikiInfo_t	stWaribikiInfo[ONL_MAX_DISC_NUM];	// ����01-25
} stTotalPayInfo_t;

//-------------------------------------------------------------------------------------------------------------------------------------------------------
// Ͻ�����(434 Byte)
// ���uر���ђʐM�̢�����⍇���ް�(ID:XX)��ɂĢ���v�� �\�񌟍���܂���
// ����v�� ����������������v�� �ݎԌ�����̂����ꂩ���1=�v�����裎��ɾ�������Ă���
//====================================================================================================
typedef struct _stMasterInfo
{
	stParkKindNum_t		stCardDataInfo[ONL_MAX_CARDNUM];		// ����1�`6(���ԏꇂ, ���, �ԍ�)	������1�̂�
	stDispCardData_t	stDispCardDataInfo[ONL_MAX_CARDNUM];	// �\���p�����ް�1�`6				������1�̂�
	stDate_YYMD_t		StartDate;								// �L���J�n�N����
	stDate_YYMD_t		EndDate;								// �L���I���N����
	uchar				SeasonKind;								// ������(0�`15)
	uchar				MemberKind;								// ������(0�`255)
	uchar				InOutStatus;							// ���o�ɽð��	0�`99(0=�������(���o�ɉ�),1=�o�ɒ�(���ɉ�),2=���ɒ�(�o�ɉ�))
	uchar				PayMethod;								// �x������i	ETC���ޗ��p���̌��ώ�i���������[���֎w�肷��
																//				0:�����Ȃ�(��),1:ETC����,2:EC����
																//				������,�ڼޯ�,�d�q�Ȱ,����߲�ނȂ�
	uchar				EntryConf;								// ����m�F�׸�	��������[���֎w�������׸ޡ
																//				0:�m�F���Ȃ� 1:�m�F����(�ްĂ��J�����W���ɂ��m�F���s��)
	uchar				Reserve1;								// �\��			0�Œ�
} stMasterInfo_t;
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// �\����(84 Byte)
// ���uر���ђʐM�̢�����⍇���ް�(ID:XX)��ɂĢ���v�� �\�񌟍�����1=�v�����裎��ɾ�������Ă���
//====================================================================================================
typedef struct _stReserveInfo
{
	stReserveManageNo_t		dtRsvManageNo;		// �\����Ǘ��ԍ�
	stDatetTimeYtoSec_t		dtRsvTimeYtoSec;	// �\��o�^�N���������b
	uchar					Reserve1;			// �\��			0�Œ�
	ushort					RsvParkFeeKindNo;	// �\�񒓎ԗ�����ʐݒ�ԍ�		0�`99
	ulong					ParkSpaceMngNo;		// ���Խ�߰��Ǘ��ԍ�	0�`999999  ��)1:�\���ʤ2:�\��g��Ҥ3:�D��\��1�4:�D��\��2 �Ȃ�
	stDate_YYMDHM_t			dtRsvStartEnter;	// �\��J�n(����\��)�N��������
	stDate_YYMDHM_t			dtRsvStartExit;		// �\��I��(�o��\��)�N��������
	stReserveFeeInfo_t		dtRsvFeeInfo;		// �\�񗿋�(�\�񗿋��������, �\�񗿋������ð��, �\�񗿋����z)
	stPenaltyFeeInfo_t		dtPenaltyFeeInfo;	// ����è����(����è�����������, ����è���������ð��, ����è��(�P��), ����è�����������z, ����è���������ςݓ���, ����è���������ςݍ��v���z)
	uchar					Under1day;			// 1���������ԗ��������ð��		0:���Z�����܂Ť1:1�����̍ő嗿��
	uchar					Reserve2;			// �\��							0�Œ�
	stDate_YYMDHM_t			dtEntryStart;		// ����\�J�n�N��������
	stDate_YYMDHM_t			dtEntryEnd;			// ����\�I���N��������
	stDate_YYMDHM_t			dtExitStart;		// �o��\�J�n�N��������
	stDate_YYMDHM_t			dtExitEnd;			// �o��\�I���N��������
	ushort					TransitTime;		// �ʉ߉\��������
												// �\���p��������\�񓏓�����ʉ߂ł��鎞�Ԑ����ɗp����
												// ���P��(0=�����Ȃ��1�`1440)
} stReserveInfo_t;
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// ����������(16 Byte)
// ���uر���ђʐM�̢�����⍇���ް�(ID:XX)��ɂĢ���v�� ���������������1=�v�����裎��ɾ�������Ă���
//====================================================================================================
typedef struct _stPassInfo
{
	ulong					ContractNo;			// �_��Ǘ��ԍ�		1�`16,000,000
	uchar					SaleStatus;			// �̔��ð��		1=�̔��ς�/9=�̔��O
	uchar					CardStatus;			// ���޽ð��		0�`99 (1=�L���2=����)
	uchar					Classification;		// ����敪			0�`99
	uchar					DivisionNo;			// �_����ԍ�		0�`26
	ushort					CasingNo;			// �_��Ԏ��ԍ�		1�`9999
	uchar					RackInfo;			// �_��ׯ����		��2
	uchar					UpdatePermission;	// 4���ׂ��X�V����	0�`1(0=�����Ȃ�/1=������)
	uchar					Option1;			// ��߼�ݑ���׸އ@	�e�ޯđΉ�
	uchar					Option2;			// ��߼�ݑ���׸އA	�e�ޯđΉ�
	uchar					ReWriteCount;		// �Ĕ��s��		0�`255
	uchar					Reserve1;			// �\��				0�Œ�
} stPassInfo_t;
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// �ݎԏ��(668 Byte)
// ���uر���ђʐM�̢�����⍇���ް�(ID:XX)��ɂĢ���v�� �ݎԌ�������1=�v�����裎��ɾ�������Ă���
//====================================================================================================
typedef struct _stZaishaInfo
{
	ulong					ResParkingLotNo;					// �ݎԒ��̒��ԏꇂ(0�`999999)�ݎԂȂ��̏ꍇ�͢0�
	uchar					PaymentType;						// ���Z�����敪	0=�����Z/1=���Z/2=�Đ��Z/3=���Z���~/4=�Đ��Z���~
	uchar					ExitPayment;						// ���Z�o��		0=�ʏ퐸�Z/1=�����o��/(2=���Z�Ȃ��o��)/3=�s���o��/
																//				9=�˔j�o��10=�ްĊJ��/97=ۯ��J��ׯ�ߏ㏸�O�����Z�o��/
																//				98=׸���ѓ��o��/99=���޽��ѓ��o��
	ulong					GenkinFee;							// ��������z(0�`999999)
	ushort					shFeeType;							// �������(0�`99)
	stDateParkTime_t		dtEntryDateTime;					// ����			Parking��� + �N���������b
	stDateParkTime_t		dtPaymentDateTime;					// ���Z			Parking��� + �N���������b
	stDateParkTime_t		dtExitDateTime;						// �o��			Parking��� + �N���������b
	stDateParkTime_t		dtEntranceDateTime;					// �����ʉ�		Parking��� + �N���������b
	stDateParkTime_t		dtOutleteDateTime;					// �o���ʉ�		Parking��� + �N���������b
	ulong					ParkingFee;							// ���ԗ���(0�`999990(10�~�P��))
	ulong					ParkingTime;						// ���Ԏ���(0�`999999(���P��))
	uchar					NormalFlag;							// �ꎞ���p�׸�(0=������p,1=�ꎞ���p)
// GG129004(S) M.Fujikawa 2024/12/04 AMS���P�v�] �J�[�h���Rev.12�Ή�
//	uchar					Reserve3;							// �\��
	uchar					InCarMode;							// ���Ƀ��[�h
// GG129004(E) M.Fujikawa 2024/12/04 AMS���P�v�] �J�[�h���Rev.12�Ή�
	stSettlement_t			stSettlement[ONL_MAX_SETTLEMENT];	// ���Z���01�`10
	stPasswordNo_t			stPassword;						// �Ïؔԍ�(�����@�ԍ�, �ԍ�)
	stDiscount_t			stDiscountInfo[ONL_MAX_DISC_NUM];	// ����01�`25
} stZaishaInfo_t;
// GG124100(S) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
//-------------------------------------------------------------------------------------------------------------------------------------------------------
// �����v�Z����(1113 Byte)
// ���uر���ђʐM�̢�����⍇���ް�(ID:XX)��ɂĢ�v�� �����v�Z����1=�v�����裎��ɾ�������Ă���
//====================================================================================================
typedef struct _stFeeCalcInfo
{
	ushort					FeeCalcRevNo;						// �����v�ZRev.No.
	ushort					CalcResult;							// �������擾����
	ushort 					shPayResult;						// �����v�Z����
	uchar					DateSwitchHours;					// ���t�ؑ֎��� ��
	uchar					DateSwitchMinute;					// ���t�ؑ֎��� ��
	stParkKindNum_t			dtPayMedia;							// ���Z�}��(���ԏꇂ�A��ʁA�ԍ�)
	uchar					FeeType;							// �������
	stPassKindNumSts_t		dtPassKindNumSts;					// ��������
	uchar					PassAlarm;							// ������A���[�����
	stDate_YYMD_t			dtEffectStart;						// �L���J�n �N����
	stDate_YYMD_t			dtEffectStop;						// �L���I�� �N����
	ulong					ParkingTime;						// ���Ԏ���
	ulong					ParkingFee;							// ���ԗ���
	ulong					Seikyuugaku;						// �����z
	stDatetTimeYtoSec_t		dtNyuukoYtoSec;						// ���� �N���������b
	uchar					Reserve2;							// �\��(�T�C�Y�����p)
	stDatetTimeYtoSec_t		dtPrivSeisanYtoSec;					// �O�񐸎Z �N���������b
	uchar					Reserve3;							// �\��(�T�C�Y�����p)
	stDatetTimeYtoSec_t		dtSeisanYtoSec;						// ���Z �N���������b
	uchar					Reserve4;							// �\��(�T�C�Y�����p)
// GG132000(S) ���Z�����^���~����NT-NET�ʐM�̐��Z�f�[�^�Ƀf�[�^���ڂ�ǉ�
	ushort					MaxFeeApplyFlg;						// �ő嗿���K�p�t���O
	ushort					MaxFeeApplyCnt;						// �ő嗿���K�p��
	uchar					MaxFeeSettingFlg;					// �ő嗿���ݒ�L��
// GG132000(E) ���Z�����^���~����NT-NET�ʐM�̐��Z�f�[�^�Ƀf�[�^���ڂ�ǉ�
	uchar					PaymentTimeType;					// ���Z�����敪
	stDiscount4_t			stDiscountInfo[ONL_MAX_DISC_NUM];	// ����01�`25
} stFeeCalcInfo_t;
// GG124100(E) R.Endo 2022/09/12 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
//-------------------------------------------------------------------------------------------------------------------------------------------------------
/****************************************/
/*		���ɏ��	���ޏ��(��M)		*/
/*		Rev.No.10						*/
/****************************************/
// ��ر���ѻ��ނ����M��������⍇�����ʂƓ���
// ���قȂ�_�ͤ���ʻ��ނ�LCD�ɂĕK�������
//   �e���ꂼ��̏����Ȃ��ꍇ�ͤ��ŋl�߂�(�Œ蒷�ɂ���)

// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
// typedef struct _stRecvCardInfo_Rev10
// {
// 	ushort					FormatNo;			// ̫�ϯ�Rev.No.			0�`65535
// 	stDatetTimeYtoSec_t		dtTimeYtoSec;		// �N���������b
// 	uchar					Reserve1;			// �\��						0�Œ�
// //	stCardInformation_t		dtCardInfo;			// ���ޏ��
// 
// 	ushort					RsltReason;			// ���ʗ��R					0 =����(���������)/1 =�y�ʐM�z����?MLC�ԒʐM��Q��(MLC�����)/10=�y�����zDB�Q�ƴװ(���������)/11=�y�����z�ʐM�ް���ʹװ(���������)
// 	ulong					AskMediaParkNo;		// �⍇���}�� ���ԏꇂ		0�`999999
// 	ushort					AskMediaRevNo;		// �⍇���}�� ���Rev.No.	0�`65535
// 	ushort					AskMediaKind;		// �⍇���}�� ���			0�`65000
// 	uchar					byAskMediaNo[32];	// �⍇���}�� �ԍ�			�yASCII�z32���i���l�A�c���Null�Œ�j
// 	uchar					ValidCHK_ETC;		// �L�������� ETCȶ�����	0=�����Ȃ��1=ETCȶ�OK�2=ETCȶ�NG
// 	uchar					ValidCHK_Dup;		// �L�������� �����d�����p����	0=�����Ȃ��1=�����d���Ȃ��2=�����d������3=�ް��s���99=���̑��ُ�
// // MH810104(S) R.Endo 2021/09/15 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5931 �����Z����̃��b�Z�[�W�\��
// 	uchar					byUnpaidHabitual;	// �����Z ��K�� �L��
// 	uchar					byUnpaidCount;		// �����Z ����
// // MH810104(E) R.Endo 2021/09/15 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5931 �����Z����̃��b�Z�[�W�\��
// 	stRecvCardInformation_t	dtReqRslt;			// ���ޏ��_�v������
// 
// 	stPayResultInfo_t		dtCalcInfo;			// �����v�Z���ʏ��	Max = 684 Byte
// 	stTotalPayInfo_t		dtTotalPayInfo;		// �݌v���z���		Max = 600 Byte
// 	stMasterInfo_t			dtMasterInfo;		// Ͻ�����			Max = 434 Byte
// 	stReserveInfo_t			dtReserveInfo;		// �\����			Max = 84 Byte
// 	stPassInfo_t			dtPassInfo;			// ����������	Max = 16 Byte
// 	stZaishaInfo_t			dtZaishaInfo;		// �ݎԏ��			Max = 668 Byte
// 
// } stRecvCardInfo_Rev10;

// GG129004(S) M.Fujikawa 2024/12/04 AMS���P�v�] �J�[�h���Rev.12�Ή�
// Rev.No.11
//typedef struct _stRecvCardInfo_Rev11
//{
//	ushort					FormatNo;			// ̫�ϯ�Rev.No.			0�`65535
//	stDatetTimeYtoSec_t		dtTimeYtoSec;		// �N���������b
//	uchar					Reserve1;			// �\��						0�Œ�
//
//	ushort					RsltReason;			// ���ʗ��R					0 =����(���������)/1 =�y�ʐM�z����?MLC�ԒʐM��Q��(MLC�����)/10=�y�����zDB�Q�ƴװ(���������)/11=�y�����z�ʐM�ް���ʹװ(���������)
//	ulong					AskMediaParkNo;		// �⍇���}�� ���ԏꇂ		0�`999999
//	ushort					AskMediaRevNo;		// �⍇���}�� ���Rev.No.	0�`65535
//	ushort					AskMediaKind;		// �⍇���}�� ���			0�`65000
//	uchar					byAskMediaNo[32];	// �⍇���}�� �ԍ�			�yASCII�z32���i���l�A�c���Null�Œ�j
//	uchar					ValidCHK_ETC;		// �L�������� ETCȶ�����	0=�����Ȃ��1=ETCȶ�OK�2=ETCȶ�NG
//	uchar					ValidCHK_Dup;		// �L�������� �����d�����p����	0=�����Ȃ��1=�����d���Ȃ��2=�����d������3=�ް��s���99=���̑��ُ�
//	uchar					byUnpaidHabitual;	// �����Z ��K�� �L��
//	uchar					byUnpaidCount;		// �����Z ����
//	stRecvCardInformation_t	dtReqRslt;			// ���ޏ��_�v������
//
//	stPayResultInfo_t		dtCalcInfo;			// �������	 Max = 684 Byte
//	stTotalPayInfo_t		dtTotalPayInfo;		// �݌v���z�⍇�� Max = 600 Byte
//	stMasterInfo_t			dtMasterInfo;		// �}�X�^�[��� Max = 434 Byte
//	stReserveInfo_t			dtReserveInfo;		// �\���� Max = 84 Byte
//	stPassInfo_t			dtPassInfo;			// ����E������ Max = 16 Byte
//	stZaishaInfo_t			dtZaishaInfo;		// �ݎԏ�� Max = 668 Byte
//	stFeeCalcInfo_t			dtFeeCalcInfo;		// �����v�Z���� Max = 1113 Byte
//} stRecvCardInfo_Rev11;
typedef struct _stRecvCardInfo_RevXX
{
	ushort					FormatNo;			// ̫�ϯ�Rev.No.			0�`65535
	stDatetTimeYtoSec_t		dtTimeYtoSec;		// �N���������b
	uchar					Reserve1;			// �\��						0�Œ�

	ushort					RsltReason;			// ���ʗ��R					0 =����(���������)/1 =�y�ʐM�z����?MLC�ԒʐM��Q��(MLC�����)/10=�y�����zDB�Q�ƴװ(���������)/11=�y�����z�ʐM�ް���ʹװ(���������)
	ulong					AskMediaParkNo;		// �⍇���}�� ���ԏꇂ		0�`999999
	ushort					AskMediaRevNo;		// �⍇���}�� ���Rev.No.	0�`65535
	ushort					AskMediaKind;		// �⍇���}�� ���			0�`65000
	uchar					byAskMediaNo[32];	// �⍇���}�� �ԍ�			�yASCII�z32���i���l�A�c���Null�Œ�j
	uchar					ValidCHK_ETC;		// �L�������� ETCȶ�����	0=�����Ȃ��1=ETCȶ�OK�2=ETCȶ�NG
	uchar					ValidCHK_Dup;		// �L�������� �����d�����p����	0=�����Ȃ��1=�����d���Ȃ��2=�����d������3=�ް��s���99=���̑��ُ�
	uchar					byUnpaidHabitual;	// �����Z ��K�� �L��
	uchar					byUnpaidCount;		// �����Z ����
	stRecvCardInformation_t	dtReqRslt;			// ���ޏ��_�v������

	stPayResultInfo_t		dtCalcInfo;			// �������	 Max = 684 Byte
	stTotalPayInfo_t		dtTotalPayInfo;		// �݌v���z�⍇�� Max = 600 Byte
	stMasterInfo_t			dtMasterInfo;		// �}�X�^�[��� Max = 434 Byte
	stReserveInfo_t			dtReserveInfo;		// �\���� Max = 84 Byte
	stPassInfo_t			dtPassInfo;			// ����E������ Max = 16 Byte
	stZaishaInfo_t			dtZaishaInfo;		// �ݎԏ�� Max = 668 Byte
	stFeeCalcInfo_t			dtFeeCalcInfo;		// �����v�Z���� Max = 1113 Byte
} stRecvCardInfo_RevXX;
// GG129004(E) M.Fujikawa 2024/12/04 AMS���P�v�] �J�[�h���Rev.12�Ή�
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)

//-------------------------------------------------------------------------------------------------------------------------------------------------------
/****************************************/
/*		���Z�ް�	���ޏ��(���M)		*/
/*		Rev.No.10						*/
/****************************************/
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�J�[�h���Rev.No.11)
// typedef struct _stSendCardInfo_Rev10
// {
// 	ushort					FormatNo;			// ̫�ϯ�Rev.No.		0�`65535
// 	stDatetTimeYtoSec_t		dtTimeYtoSec;		// �N���������b
// 	uchar					Reserve1;			// �\��					0�Œ�
// 	stCardInformation_t		dtCardInfo;			// ���ޏ��
// 			// �ȉ��̏��͗L�Ƃ��ČŒ蒷��log�o�^ & RXM -> LCD���M����
// 			//		uchar 	MasterOnOff;		// ���ޏ�� Ͻ�����  �L��			0:���1:�L
// 			//		uchar 	ReserveOnOff;		// ���ޏ�� �\����  �L��			0:���1:�L
// 			//		uchar 	PassOnOff;			// ���ޏ�� ����������  �L��		0:���1:�L
// 			//		uchar 	ZaishaOnOff;		// ���ޏ�� �ݎԏ��  �L��			0:���1:�L
// 
// 	stMasterInfo_t			dtMasterInfo;		// ��Ͻ�����			Max = 434 Byte
// 	stReserveInfo_t			dtReserveInfo;		// ���\����			Max = 84 Byte
// 	stPassInfo_t			dtPassInfo;			// ������������		Max = 16 Byte
// 	stZaishaInfo_t			dtZaishaInfo;		// ���ݎԏ��			Max = 668 Byte
// 
// } stSendCardInfo_Rev10;

// GG129004(S) M.Fujikawa 2024/12/04 AMS���P�v�] �J�[�h���Rev.12�Ή�
// Rev.No.11
//typedef struct _stSendCardInfo_Rev11
//{
//	ushort					FormatNo;			// ̫�ϯ�Rev.No.		0�`65535
//	stDatetTimeYtoSec_t		dtTimeYtoSec;		// �N���������b
//	uchar					Reserve1;			// �\��					0�Œ�
//	stCardInformation_t		dtCardInfo;			// ���ޏ��
//			// �ȉ��̏��͗L�Ƃ��ČŒ蒷��log�o�^ & RXM -> LCD���M����
//			//		uchar 	MasterOnOff;		// ���ޏ�� Ͻ�����  �L��			0:���1:�L
//			//		uchar 	ReserveOnOff;		// ���ޏ�� �\����  �L��			0:���1:�L
//			//		uchar 	PassOnOff;			// ���ޏ�� ����������  �L��		0:���1:�L
//			//		uchar 	ZaishaOnOff;		// ���ޏ�� �ݎԏ��  �L��			0:���1:�L
//
//	stMasterInfo_t			dtMasterInfo;		// ��Ͻ�����			Max = 434 Byte
//	stReserveInfo_t			dtReserveInfo;		// ���\����			Max = 84 Byte
//	stPassInfo_t			dtPassInfo;			// ������������		Max = 16 Byte
//	stZaishaInfo_t			dtZaishaInfo;		// ���ݎԏ��			Max = 668 Byte
//} stSendCardInfo_Rev11;
typedef struct _stSendCardInfo_RevXX
{
	ushort					FormatNo;			// ̫�ϯ�Rev.No.		0�`65535
	stDatetTimeYtoSec_t		dtTimeYtoSec;		// �N���������b
	uchar					Reserve1;			// �\��					0�Œ�
	stCardInformation_t		dtCardInfo;			// ���ޏ��
			// �ȉ��̏��͗L�Ƃ��ČŒ蒷��log�o�^ & RXM -> LCD���M����
			//		uchar 	MasterOnOff;		// ���ޏ�� Ͻ�����  �L��			0:���1:�L
			//		uchar 	ReserveOnOff;		// ���ޏ�� �\����  �L��			0:���1:�L
			//		uchar 	PassOnOff;			// ���ޏ�� ����������  �L��		0:���1:�L
			//		uchar 	ZaishaOnOff;		// ���ޏ�� �ݎԏ��  �L��			0:���1:�L

	stMasterInfo_t			dtMasterInfo;		// ��Ͻ�����			Max = 434 Byte
	stReserveInfo_t			dtReserveInfo;		// ���\����			Max = 84 Byte
	stPassInfo_t			dtPassInfo;			// ������������		Max = 16 Byte
	stZaishaInfo_t			dtZaishaInfo;		// ���ݎԏ��			Max = 668 Byte
} stSendCardInfo_RevXX;
// GG129004(E) M.Fujikawa 2024/12/04 AMS���P�v�] �J�[�h���Rev.12�Ή�
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�J�[�h���Rev.No.11)

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
/****************************************/
/*		�̎��؃f�[�^ �̎��؏��			*/
/****************************************/
// RT�̎��؃f�[�^�p����
typedef struct _stReceiptDiscount {
	uchar	ParkingLotNo[3];		// ���� ���ԏꇂ
	ushort	Kind;					// ���� ���
	ushort	CardType;				// ���� �敪
	ushort	UsageCount;				// ���� �g�p��
	ushort	PrevUsageCount;			// ���� �O��g�p��
	ulong	DisAmount;				// ���� �����z
	ulong	PrevUsageDisAmount;		// ���� �O�񊄈��z
	ushort	FeeKindSwitchSetting;	// ���� ��ʐ؊����
	ulong	DisType;				// ���� �������1
	ulong	DisType2;				// ���� �������2
	uchar	EligibleInvoiceTaxable;	// ���� �K�i������ �ېőΏ�
} stReceiptDiscount;

// �̎��؏��
typedef	struct _stReceiptInfo {
	ushort					FormatNo;							// �t�H�[�}�b�gRev.No.
	stDatetTimeYtoSec_t		dtTimeYtoSec;						// ���� �N���������b
	uchar					Reserve1;							// �\��
	ushort					ModelCode;							// ���Z�@ �@��R�[�h
	ulong					MachineNo;							// ���Z�@ �@�B��
	ulong					IndividualNo;						// ���Z�� �ʒǔ�
	ulong					ConsecutiveNo;						// ���Z�� �ʂ��ǔ�
	stDate_YYMDHM_t			dtEntry;							// ���ɔN��������
	stDate_YYMDHM_t			dtOldPayment;						// �O�񐸎Z�N��������
	stDate_YYMDHM_t			dtPayment;							// ���Z�N��������
	stDate_YYMDHM_t			dtExitDateTime;						// �o�ɔN��������
	uchar					SettlementModeMode;					// ���Z���[�h
	uchar					FeeKind;							// �������
	ulong					ParkingTime;						// ���Ԏ���
	ulong					ParkingFee;							// ���ԗ���
	ulong					SeasonParkingLotNo;					// ����� ���ԏꇂ
	ushort					SeasonKind;							// ����� ���
	ulong					SeasonId;							// ����� ID
	stDate_YYMD_t			stSeasonValidStart;					// ����� �L���J�n�N����
	stDate_YYMD_t			stSeasonValidEnd;					// ����� �L���I���N����
	uchar					VehicleNumberSerialNo[12];			// �Ԕ� ��A�w��ԍ�
	uchar					UniqueID[6];						// �Ԕԃ`�P�b�g���X ���j�[�NID�i���o��ID�j
	ulong					PaidFeeAmount;						// �x���ςݗ���
	ulong					Parking_Bil;						// ���v���z�i�����z�j
	ushort					CardPaymentCategory;				// �J�[�h���� �敪
	uchar					CardPaymentTerminalCategory;		// �J�[�h���� �[���敪
	uchar					CardPaymentTransactionStatus;		// �J�[�h���� ����X�e�[�^�X
	stDatetTimeYtoSec_t		dtCardPayment;						// �J�[�h���� �N���������b
	uchar					Reserve2;							// �\��
	union {
		struct {
			ulong			amount;								// ���p���z
			uchar			card_no[20];						// ����ԍ�
			uchar			cct_num[16];						// �[�����ʔԍ�
			uchar			kid_code[6];						// KID�R�[�h
			ulong			app_no;								// ���F�ԍ�
			ulong			center_oiban;						// �Z���^�[�����ʔ�
			uchar			ShopAccountNo[20];					// �����X����ԍ�
			ulong			slip_no;							// �`�[�ԍ�
			uchar			reserve[2];							// �\��
		} credit;
		struct {
			ulong			amount;								// ���p���z
			uchar			card_id[30];						// �J�[�h�ԍ�
			ulong			card_zangaku;						// �J�[�h�c�z
			uchar			inquiry_num[16];					// �₢���킹�ԍ�
			ulong			approbal_no;						// ���F�ԍ�
			uchar			terminal_id[13];					// ���̒[��ID
			uchar			reserve[9];							// �\��
		} emoney;
		struct {
			ulong			amount;								// ���p���z
			uchar			MchTradeNo[32];						// Mch����ԍ�
			uchar			PayTerminalNo[16];					// �x���[��ID
			uchar			DealNo[16];							// ����ԍ�
			uchar			PayKind;							// ���σu�����h
			uchar			reserve[11];						// �\��
		} qr;
	} card;
	ushort					EligibleInvoiceHasInfo;				// �K�i������ ���L��
	uchar					EligibleInvoiceRegistrationNo[14];	// �K�i������ �o�^�ԍ�
	ulong					EligibleInvoiceTaxableAmount;		// �K�i������ �ېőΏۊz(�ō���)
	ushort					EligibleInvoiceTaxRate;				// �K�i������ �K�p�ŗ�
	ulong					EntryMachineNo;						// �����@�ԍ� [Rev.No.1�ǉ�]
	ulong					ParkingTicketNo;					// ���Ԍ��ԍ� [Rev.No.1�ǉ�]
	ulong					CashReceiptAmount;					// �����̎��z [Rev.No.1�ǉ�]
	ulong					WInPrice;							// �a����z [Rev.No.1�ǉ�]
	ulong					WChgPrice;							// �ނ�K�z [Rev.No.1�ǉ�]
	ulong					WFusoku;							// ���ߕs�����z [Rev.No.1�ǉ�]
	stReceiptDiscount		arrstDiscount[25];					// ����01-25
	uchar					Reserve3[2];						// �Í��� �o�C�g���� [Rev.No.1�ύX]
} stReceiptInfo;

// QR�̎���
#define QRCODE_RECEIPT_HEADER_SIZE 128
#define QRCODE_RECEIPT_ENCRYPT_SIZE 80
#define QRCODE_RECEIPT_SIZE (QRCODE_RECEIPT_HEADER_SIZE + (QRCODE_RECEIPT_ENCRYPT_SIZE * 2))
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

//-------------------------------------------------------------------------------------------------------------------------------------------------------

//--------------------------------------------------
//		�R�}���h�\���̒�`
//		�i ���C����� ==> LCD���W���[�� �j
//--------------------------------------------------

//--------------------------------------------------
//		�R�}���h���ʗ̈�
//--------------------------------------------------
/**
 *	LCD���W���[�����R�}���h�̋��ʕ��� �\����
 */
typedef	struct {
	unsigned short	length;		///< �R�}���h��
	unsigned short	id;			///< �R�}���h�h�c
	unsigned short	subid;		///< �T�u�R�}���h�h�c
} lcdbm_cmd_base_t;

//--------------------------------------------------
//		�X�e�[�^�X�v��
//--------------------------------------------------
/**
 *	��{�ݒ�v��
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	// �R�}���h���ʏ��
	unsigned char		rsw;		// RSW���
// MH810100(S) 2020/06/08 #4205�y�A���]���w�E�����z�Z���^�[�Ƃ̒ʐM�f����QR�Ǎ������Z��������Ɖ�ʂ��ł܂�(No.02-0009)
//	unsigned char		ver[10];	// �v���O�����o�[�W����
	unsigned char		ver[8];		// �v���O�����o�[�W����
	unsigned char		opemod;		// Ope��Mode
	unsigned char		reserved;	// 
// MH810100(E) 2020/06/08 #4205�y�A���]���w�E�����z�Z���^�[�Ƃ̒ʐM�f����QR�Ǎ������Z��������Ɖ�ʂ��ł܂�(No.02-0009)
} lcdbm_cmd_config_t;

//--------------------------------------------------
//		�@��ݒ�
//--------------------------------------------------
/**
 *	���v�ݒ�
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< �R�}���h���ʏ��
	unsigned short		year;		///< ����N
	unsigned char		month;		///< ��
	unsigned char		day;		///< ��
	unsigned char		hour;		///< ��
	unsigned char		minute;		///< ��
	unsigned char		second;		///< �b
	unsigned char		reserved1;	///< �\��
	unsigned char		reserved2;	///< �\��
	unsigned char		reserved3;	///< �\��
} lcdbm_cmd_clock_t;

/**
 *	LCD�P�x
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< �R�}���h���ʏ��
	unsigned short		brightness;	///< �P�x�l(0�`15)
} lcdbm_cmd_brightness_t;

//--------------------------------------------------
//		�e�L�X�g�\��
//--------------------------------------------------
// �e�L�X�g�\���R�}���h�ҏW�o�b�t�@�̃T�C�Y(1�p�P�b�g���M�f�[�^�ő�T�C�Y-�d����(2Byte)-�R�}���h�w�b�_�T�C�Y(6Byte))
#define	TEXT_CMD_BUF_SIZE	(PKT_CMD_MAX-2-6)

/**
 *	�e�L�X�g�\���R�}���h�\����
 *	@attention	�e�L�X�g�f�[�^�쐬���l������ lcdbm_cmd_work_t�\���̂ɂ͊܂܂Ȃ��B
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< �R�}���h���ʏ��
	unsigned char		buf[ TEXT_CMD_BUF_SIZE ];	///< �e�L�X�g�\���R�}���h����邽�߂̃o�b�t�@�̈�
} lcdbm_cmd_text_t;

//--------------------------------------------------
//		�A�i�E���X�v��
//--------------------------------------------------
/** WAVE�Đ��łP�R�}���h���Ɏw��ł��镶�ߐ� */
#define	LCDBM_CMD_AUDIO_PHRASES		10

///** ���������łP�R�}���h���Ɏw��ł��镶�ߐ� */
//#define	LCDBM_CMD_SPEECH_PHRASES	10
//
///** ���������Ŏg���`���l���ԍ� */
//#define	LCDBM_CMD_SPEECH_CHANNEL	0

/**
 *	���ʐݒ�
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< �R�}���h���ʏ��
	unsigned char		volume;		///< ����(0�`100(0=����))
	unsigned char		reserved1;	///< �\��
} lcdbm_cmd_audio_volume_t;

/**
 *	�J�n�v��
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< �R�}���h���ʏ��
	unsigned char		times;		///< ������(0�C1�`9��CFFH(0=�����Ȃ��CFFH=���̕����J�n�܂ŌJ��Ԃ�))
	unsigned char		condition;	///< �����J�n����
									///<   0=���݂̕����𒆒f���ĕ������J�n����B
									///<   1=�\��
									///<   2=�҂���Ԃ̕������܂߂��ׂĂ̕����I����ɊJ�n����
	unsigned char		interval;	///< ���b�Z�[�W�Ԋu(0�`15�b)
	unsigned char		channel;	///< �����`���l��(0�Œ�)
	unsigned char		reserved1;	///< �\��
	unsigned char		reserved2;	///< �\��
	unsigned char		language;	///< ����w��(0�`99=����ԍ�)
	unsigned char		AnaKind;	///< ���(0=���b�Z�[�W��/1=���߇�)
	unsigned short		AnaNo;		///< ���b�Z�[�W��/���߇�
} lcdbm_cmd_audio_start_t;

/**
 *	�I���v��
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< �R�}���h���ʏ��
	unsigned char		channel;	///< �����I���`���l��(0�Œ�)
	unsigned char		abort;		///< ���f���@(0�Œ�)
	unsigned char		reserved1;	///< �\��
	unsigned char		reserved2;	///< �\��
} lcdbm_cmd_audio_end_t;

//--------------------------------------------------
//		�u�U�[�v��
//--------------------------------------------------
/**
 *	���ʐݒ�
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< �R�}���h���ʏ��
	unsigned char		volume;		///< ����(0�`3(0=�����C3=�ő剹��))
	unsigned char		reserved1;	///< �\��
} lcdbm_cmd_beep_volume_t;

/**
 *	�u�U�[���v��
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< �R�}���h���ʏ��
	unsigned char		beep;		///< ���(0=�s�b��,1=�s�s�B��,2=�s�s�s�B��)
	unsigned char		reserved1;	///< �\��
} lcdbm_cmd_beep_start_t;

/**
 *	�x����v��
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< �R�}���h���ʏ��
	unsigned char		kind;		///< ���(0=�Œ�)
	unsigned char		type;		///< ���(0=�J�n,1=��~)
} lcdbm_cmd_alarm_start_t;

//--------------------------------------------------
//		���Z�@��Ԓʒm����
//--------------------------------------------------
/**
 *	����ʒm
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< �R�}���h���ʏ��
	unsigned char		ope_code;	///< ����R�[�h(���L�Q��)
	unsigned short		status;		///< ���(���L�Q��)
									///<   ����R�[�h|�f�[�^�̎��			|���
									///<   ==========+======================+=============================================
									///<   		    1|���Z���~				|0�Œ�
									///<   		    2|���Z���~����			|0=OK/1=NG
									///<   		    3|���Z�J�n				|0�Œ�
									///<   		    4|���Z�J�n����			|0=OK/1=NG
									///<   		    5|�l�̌��m				|0=OFF/1=ON
									///<   		    6|�h�A���				|0=�h�A��/1=�h�A�J
									///<   		    7|�̎��ؔ��s			|0�Œ�
									///<   		    8|���u���Z�J�n			|0�Œ�
									///<   		    9|���u���Z�J�n����		|0=OK/1=NG
									///<   		   10|�c�x��				|0=�c��/1=�x��
									///<   		   11|�N���ʒm				|0=�ʏ�N�� 1=�ݒ豯��۰�ދN��
									///<   		   12|�N�������ʒm			|0�Œ�
									///<   		   13|�ݒ�A�b�v���[�h�ʒm	|0=OK/1=NG
									///<   		   14|�ݒ�A�b�v���[�h�v��	|0�Œ�
									///<   		   15|���Z�b�g�v��			|0�Œ�
									///<   		   16|���U�[�u1_��M
									///<   		   17|���U�[�u1_���M
									///<   		   18|�����e�i���X�J�n�v��	|0�Œ�
									///<   		   19|�����e�i���X�J�n����	|0=OK/1=NG
									///<   		   20|�����e�i���X�I���v��	|0x00=�ҋ@/����ȊO�͌x���ʒm��bit
									///<   		   21|�����e�i���X�I������	|0=OK/1=NG
									///<   		   22|���Z�����ʒm			|0=0�~�\��/1=���O�^�C���������/2=���Z�ς݈ē�
									///<   		   23|QR��~�v��			|0�Œ�
									///<   		   24|QR��~����			|0=OK/1=NG
									///<   		   25|QR�J�n�v��			|0�Œ�
									///<   		   26|QR�J�n����			|0=OK/1=NG
									///<   		   27|���Z�ē��I���ʒm		|0�Œ�
									///<   		   28|���Z�ē��I������		|0�Œ�
									///<   		   29|���Z�b�g�ʒm			|0�Œ�
} lcdbm_cmd_notice_ope_t;

/**
 *	�c�x�ƒʒm
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< �R�}���h���ʏ��
	unsigned char		opn_cls;	///< �c�x��(0=�c��,1=�x��)
	unsigned char		reason;		///< �x�Ɨ��R(���L�Q��)
									///<    0:
									///<    1:�����c�x��
									///<    2:�c�Ǝ��ԊO
									///<    3:�������ɖ��t
									///<    4:�R�C�����ɖ��t
									///<    5:�������[�_�[�E��
									///<    6:�ޑK�؂�
									///<    7:�R�C�����b�N�ʐM�s��
									///<    8:���M�o�b�t�@�t��
									///<    9:(�\��)
									///<   10:�M���ɂ��x��
									///<   11:�ʐM�ɂ��x��
} lcdbm_cmd_notice_opn_t;

/**
 *	���Z��Ԓʒm
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< �R�}���h���ʏ��
	unsigned long		event;		///< ����(Bit�P��:0=����,1=����)
									///<   b0:�ޑK�s��
									///<   b1:���V�[�g�p���s��
									///<   b2:�W���[�i���p���s��
									///<   b3:Suica���p�s��
									///<   b4:�N���W�b�g���p�s��
									///<   b5:Edy���p�s��
									///<   b6 �`31:�\��
} lcdbm_cmd_notice_pay_t;

/**
 *	�|�b�v�A�b�v�\���v��
 */
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
#define LCDBM_CMD_NOTICE_DSP_ADD_STR_SIZE 484
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
typedef	struct {
	lcdbm_cmd_base_t	command;	// �R�}���h���ʏ��
	unsigned char		text_code;	// �R�[�h(���L�Q��)
									//   1:�a��ؔ��s(���Z������)
									//   2:�a��ؔ��s(�����߂���)
									//   3:���z���s
									//   4:���x����
									//   5:�N���W�b�g�Ɖ�NG
									//   6:�N���W�b�g�T�[�o�ُ�
									//   7:�c���s��
	unsigned char		status;		// ���(���L�Q��)
									// �e�L�X�g�R�[�h=1/2�̏ꍇ
									//   0:�ޑK�Ȃ�
									//   1:�ޑK����
									//   2:�ޑK�Ȃ�(�̎���)
									//   3:�ޑK����(�̎���)
									// �e�L�X�g�R�[�h=5�̏ꍇ
									//   1:�戵�s��
									//   2:�����J�[�h
									//   3:�g�p�s��
									//   4:�����؂�
									//   5:�ǂݎ��NG
									//   6:�c���s��
	unsigned long		add_info;	// �t�����
									//   �R�[�h=5�A���=6�̏ꍇ�c�J�[�h�c��
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	unsigned char		add_str[LCDBM_CMD_NOTICE_DSP_ADD_STR_SIZE];	// �ǉ�������
									//   �R�[�h=12�̏ꍇ�c QR�̎��؂̕�����
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
} lcdbm_cmd_notice_dsp_t;

/**
 *	�x���ʒm
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< �R�}���h���ʏ��
	unsigned short		event;		///< ����(Bit�P��:0=����,1=����)
									///<   b0:�R�C�����ɖ��Z�b�g
									///<   b1:�R�C���J�Z�b�g���Z�b�g
									///<   b2:�R�C�����b�N���{��
									///<   b3:�������[�_�[�W�J
									///<   b4:�������[�_�[���{��
									///<   b5:���V�[�g�v�����^�W�J
									///<   b6:�W���[�i���v�����^�W�J
									///<   b7:RSW��0�ȊO
									///<   b8�`15:�\��
} lcdbm_cmd_notice_alm_t;

// GG129000(S) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
/**
 *	�|�b�v�A�b�v�폜�v��
 */
typedef	struct {
	lcdbm_cmd_base_t	command;	///< �R�}���h���ʏ��
	unsigned char		kind;		///< ���(���L�Q��)
									///<   1:�̎��ؔ��sPOP
	unsigned char		status;		///< ���(���L�Q��)
									///<   1:�󎚊���
} lcdbm_cmd_notice_del_t;
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j

//--------------------------------------------------
//		���Z�E�������ʒm
//--------------------------------------------------
/**
 *	���Z�c���ω��ʒm
 */
typedef	struct {
	lcdbm_cmd_base_t	command;			// �R�}���h���ʏ��
	unsigned long		id;					// ID(���ɂ��琸�Z�����܂ł��Ǘ�����ID)
	unsigned char		pay_sts;			// ���Z�X�e�[�^�X(0:���Z��,1:���Z����,2:���Z��,3:���Z�s��)
	DATE_YMDHMS			pay_time;			// ���Z�N���������b
	unsigned long		prk_fee;			// ���ԗ���
	unsigned long		pay_rem;			// ���Z�c�z
	unsigned long		cash_in;			// ���������z
	unsigned long		chg_amt;			// �ޑK�z
	unsigned long		disc_amt;			// �����z
// MH810101(S) R.Endo 2021/01/22 �Ԕԃ`�P�b�g���X �t�F�[�Y2 ����Ή�
	unsigned char		season_chk_result;	// ����`�F�b�N����
// MH810101(E) R.Endo 2021/01/22 �Ԕԃ`�P�b�g���X �t�F�[�Y2 ����Ή�
// MH810103(s) �d�q�}�l�[�Ή�
    unsigned char		method;         	// �x�������@
    unsigned char		brand;          	// �u�����h
    unsigned long		emoney_balance; 	// �d�q�}�l�[�c��
// MH810103(e) �d�q�}�l�[�Ή�
// MH810103(s) �d�q�}�l�[�Ή� #5465 �Đ��Z�œd�q�}�l�[���g�p�������Z�������A���Z�@��ʏ�̈���������z�ɏ��񐸎Z�����܂�ł��܂�
    unsigned long		emoney_pay; 		// �d�q�}�l�[���ϋ��z
// MH810103(e) �d�q�}�l�[�Ή� #5465 �Đ��Z�œd�q�}�l�[���g�p�������Z�������A���Z�@��ʏ�̈���������z�ɏ��񐸎Z�����܂�ł��܂�
// GG129000(S) R.Endo 2024/01/12 #7217 ���������v�\���Ή�
	unsigned long		Kaimono;			// �������z���v
// GG129000(E) R.Endo 2024/01/12 #7217 ���������v�\���Ή�
// GG129004(S) M.Fujikawa 2024/10/22 �������z�������Ή�
	unsigned long		Kaimono_info;		// �������z�������
// GG129004(E) M.Fujikawa 2024/10/22 �������z�������Ή�
} lcdbm_cmd_pay_rem_chg_t;

/**
 *	���Z�f�[�^
 */
typedef	struct {
	lcdbm_cmd_base_t		command;			///< ����ދ��ʏ��
	unsigned long			id;					///< ID(���ɂ��琸�Z�����܂ł��Ǘ�����ID)
	unsigned long			CenterOiban;		///< �Z���^�[�ǔ�
	unsigned short			CenterOibanFusei;	///< �Z���^�[�ǔԕs���׸�
	unsigned char			shubetsu;			///< ���(0=���O���Z�^1�������Z�o�ɐ��Z)
	unsigned short			crd_info_rev_no;	///< ���ޏ��Rev.No.
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�J�[�h���Rev.No.11)
// 	stSendCardInfo_Rev10	crd_info;			///< ���ޏ��(���Z�ް�)
// GG129004(S) M.Fujikawa 2024/12/04 AMS���P�v�] �J�[�h���Rev.12�Ή�
//	stSendCardInfo_Rev11	crd_info;			///< ���ޏ��(���Z�ް�)
	stSendCardInfo_RevXX	crd_info;			///< ���ޏ��(���Z�ް�)
// GG129004(E) M.Fujikawa 2024/12/04 AMS���P�v�] �J�[�h���Rev.12�Ή�
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�J�[�h���Rev.No.11)
} lcdbm_cmd_pay_data_t;

#define PADSIZE	16-(2+sizeof(lcdbm_cmd_pay_data_t))%16	// �Í�������0�p�f�B���O�T�C�Y(16Byte�P�ʂƂ���)

typedef	struct {
	unsigned short			data_len;		///< �d����
	lcdbm_cmd_pay_data_t	pay_dat;		///< RT���Z�f�[�^
	unsigned char			pad[PADSIZE];	///< 16Byte�P�ʂƂ���ׂ�0�p�f�B���O�p�G���A
} lcdbm_cmd_pydt_0pd_t;

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
/**
 *	�̎��؃f�[�^
 */
typedef	struct {
	lcdbm_cmd_base_t		command;			///< ����ދ��ʏ��
	unsigned long			id;					///< ID(���ɂ��琸�Z�����܂ł��Ǘ�����ID)
	unsigned long			CenterOiban;		///< �Z���^�[�ǔ�
	unsigned short			CenterOibanFusei;	///< �Z���^�[�ǔԕs���t���O
	stReceiptInfo			receipt_info;		///< �̎��؏��
} lcdbm_cmd_receipt_data_t;

#define RECEIPT_PADSIZE	(16-(2+sizeof(lcdbm_cmd_receipt_data_t))%16)	// �Í�������0�p�f�B���O�T�C�Y(16Byte�P�ʂƂ���)

typedef	struct {
	unsigned short				data_len;				///< �d����
	lcdbm_cmd_receipt_data_t	receipt_dat;			///< RT�̎��؃f�[�^
	unsigned char				pad[RECEIPT_PADSIZE];	///< 16Byte�P�ʂƂ���ׂ�0�p�f�B���O�p�G���A
} lcdbm_cmd_receiptdt_0pd_t;
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

/*--------------------------------------------------------------------------*/
/* QR�m��E����f�[�^														*/
/*--------------------------------------------------------------------------*/
//  �����F�؏�� (�P��)
typedef struct{
	ulong			ParkingNo;				// ���ԏꇂ
	ushort			Kind;					// ���
	ushort			CardType;				// �J�[�h�敪
	uchar			Type;					// �敪
	uchar			reserve;				// �\��(�T�C�Y�����p)
	ushort			DiscInfo;				// �������
} QR_Discount;

// QR���V�[�g���
typedef struct{
	ulong			FacilityCode;			// �{�݃R�[�h
	ulong			PosNo;					// �[���ԍ�
	DATE_YMDHMS		IssueDate;				// ���V�[�g���s�N���������b
	uchar			reserve;				// �\��(�T�C�Y�����p)
	ulong			Amount;					// ������z
	ushort			HouseKind;				// �n�E�X�J�[�h���
	QR_Discount		Disc;					// �{�ݗ��p����
} QR_Receipt;

// QR���������
typedef struct{
	ulong			IssueNo;				// ���������s�ǔ�
	DATE_YMDHMS		IssueDate;				// ���������s�N���������b
	uchar			reserve;				// �\��(�T�C�Y�����p)
	QR_Discount		Disc;					// ����������
} QR_DiscTicket;

// QR�n�E�X�J�[�h���
typedef struct{
	ushort			Kind;					// �J�[�h���
	uchar			CardNo[32];				// �J�[�h�ԍ�
} QR_HouceCard;

// ------------------------------ //
// �����F�ؓo�^�E����v���f�[�^
// ------------------------------ //
#define		BAR_DATASIZE	512			// QR���f�[�^�T�C�Y
#define		BAR_INFO_SIZE	128			// �p�[�X��̃f�[�^�T�C�Y

typedef struct{
	ushort			FormRev;				// �t�H�[�}�b�gRev.��
	DATE_YMDHMS		CertDate;				// �F�؊m��/���~�N���������b
	uchar			reserve1;				// �\��(�T�C�Y�����p)
	ushort			ProcessKind;			// �����敪
	struct{
		ulong		ParkingNo;				// ���ԏꇂ
		ushort		Kind;					// ���
		uchar		CardNo[32];				// �J�[�h�ԍ�
// MH810100(S) K.Onodera  2020/02/27 #3946 QR�R�[�h�Ŋ�������A���Z������Ƃ肯���ŏ�����ʂɖ߂�^�C�~���O�ōċN�����Ă��܂�
//		uchar		CardInfo[16];			// �J�[�h���
// MH810100(E) K.Onodera  2020/02/27 #3946 QR�R�[�h�Ŋ�������A���Z������Ƃ肯���ŏ�����ʂɖ߂�^�C�~���O�ōċN�����Ă��܂�
	} in_media;
// MH810100(S) K.Onodera  2020/02/27 #3946 QR�R�[�h�Ŋ�������A���Z������Ƃ肯���ŏ�����ʂɖ߂�^�C�~���O�ōċN�����Ă��܂�
	uchar			Passkind;				// ������
	uchar			MemberKind;				// ������
	ulong			InParkNo;				// ���ɒ��ԏꇂ
// MH810100(E) K.Onodera  2020/02/27 #3946 QR�R�[�h�Ŋ�������A���Z������Ƃ肯���ŏ�����ʂɖ߂�^�C�~���O�ōċN�����Ă��܂�
	DATE_YMDHMS		InDate;					// ���ɔN���������b
	uchar			reserve2;				// �\��(�T�C�Y�����p)
	ushort			ReciptCnt;				// ���V�[�g����(0�`99=�F�ؖ⍇��OK�ƂȂ������V�[�g�������Z�b�g)
	uchar			Method;					// �������@(0=�I�����C���f�[�^�A1=�I�t���C���f�[�^�A2=AR-100�^150 )
	uchar			Lastflg;				// �ŏI�t���O(0=�F�؊J�n�f�[�^�A1=�F�ؓr���f�[�^�A2=�F�؍ŏI�f�[�^)
	uchar			CertCnt;				// �����F�ؐ�(0�`25)
	uchar			reserve3;				// �\��(�T�C�Y�����p)
	ushort			DiscountInfoSize;		// �����F�؏��f�[�^�T�C�Y
	QR_Discount		DiscountInfo[25];		// �����F�؏��
	ushort			QR_ID;					// QR�R�[�hID
	ushort			QR_FormRev;				// QR�R�[�h�t�H�[�}�b�gRev.��
	uchar			QR_type;				// QR���
	ushort			QR_data_size;			// QR���f�[�^�T�C�Y(�p�[�X�f�[�^)
	ushort			QR_row_size;			// QR�R�[�h�f�[�^�T�C�Y(���f�[�^)
	uchar			QR_data[BAR_INFO_SIZE];
	uchar			QR_row_data[BAR_DATASIZE];
} DC_QR_Info;

typedef	struct {
	ulong			ID;						// ���ɂ��琸�Z�����܂ł��Ǘ�����ID
	ulong			CenterOiban;			// �Z���^�[�ǔ�
	ushort			CenterOibanFusei;		// �Z���^�[�ǔԕs���׸�
	ushort			QR_Info_Rev;			// QR���Rev.��
	DC_QR_Info		QR_Info;				// QR���

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	// �{�\���̂�ύX����ꍇ�͕K�� DC_QR_log_date ���C������
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
} DC_QR_log;
extern	DC_QR_log		DC_QR_work;			// QR�m��E����f�[�^�o�^�p���[�N�o�b�t�@

// LOG���R�[�h���t�p
typedef	struct {
	unsigned long		ID;					// ���ɂ��琸�Z�����܂ł��Ǘ�����ID
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	ulong				CenterOiban;		// �Z���^�[�ǔ�
	ushort				CenterOibanFusei;	// �Z���^�[�ǔԕs���׸�
	ushort				QR_Info_Rev;		// QR���Rev.��
	ushort				FormRev;			// �t�H�[�}�b�gRev.��
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	stDatetTimeYtoSec_t	dtTimeYtoSec;		// �N���������b
} DC_QR_log_date;

/**
 *	QR�m��E����f�[�^
 */
typedef	struct {
	lcdbm_cmd_base_t		command;			///< ����ދ��ʏ��
	unsigned long			id;					///< ID(���ɂ��琸�Z�����܂ł��Ǘ�����ID)
	unsigned long			CenterOiban;		///< �Z���^�[�ǔ�
	unsigned short			CenterOibanFusei;	///< �Z���^�[�ǔԕs���׸�
	unsigned short			crd_info_rev_no;	///< ���ޏ��Rev.No.
	DC_QR_Info				QR_code_info;		///< QR�R�[�h���
} lcdbm_cmd_QR_conf_can_t;

#define QR_PADSIZE	16-(2+sizeof(lcdbm_cmd_QR_conf_can_t))%16	// �Í�������0�p�f�B���O�T�C�Y(16Byte�P�ʂƂ���)

typedef	struct {
	unsigned short			data_len;			// �d����
	lcdbm_cmd_QR_conf_can_t	qr_dat;				// QR�f�[�^
	unsigned char			pad[QR_PADSIZE];	// 16Byte�P�ʂƂ���ׂ�0�p�f�B���O�p�G���A
} lcdbm_cmd_qrdt_0pd_t;

/**
 *	QR�f�[�^����
 */
typedef	struct {
	lcdbm_cmd_base_t		command;		///< ����ދ��ʏ��
	unsigned long			id;				///< ID(���ɂ��琸�Z�����܂ł��Ǘ�����ID)
	unsigned char			result;			///< ����(0:OK,1:NG(�r��),2:NG(�������))
	unsigned char			reserve;		///< �\��
} lcdbm_cmd_QR_data_res_t;

// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
// ���Ɏ����w�艓�u���Z�J�n
typedef	struct {
	lcdbm_cmd_base_t	command;	// ���ʏ��
	unsigned long		ulPno;		// ���ԏꇂ
	unsigned short		RyoSyu;		// �������
	DATE_YMDHMS			InTime;		// ���ɔN���������b
	unsigned char		reserve;	// �\��(�T�C�Y�����p)
} lcdbm_cmd_remote_time_start_t;
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)

// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
/**
 *	���[�����j�^�f�[�^(���[�����j�^��񕔕�)
 */
typedef struct{
	ushort			FormRev;				// �t�H�[�}�b�gRev.��
	DATE_YMDHMS		ProcessDate;			// �����N���������b
	uchar			reserve1;				// �\��(�T�C�Y�����p)
	ulong			StsSeqNo;				// ��Ԉ�A�ԍ�
	ulong			MediaParkNo;			// �}�� ���ԏꇂ
	ushort			Mediasyu;				// �}�� ���
	uchar			MediaNo[32];			// �}�� �ԍ�
	ushort			PaySyu;					// �������
	uchar			StsSyu;					// ��Ԏ��
	uchar			StsCode;				// ��ԃR�[�h
	ulong			FreeNum;				// �t���[���l
	uchar			FreeStr[60];			// �t���[����
	uchar			StsName[30];			// ��Ԗ�
	uchar			StsMese[150];			// ��ԃ��b�Z�[�W
	uchar			AES_Reserve[4];
} DC_LANE_Info;

/**
 *	���[�����j�^�f�[�^�i���[�����j�^�f�[�^�o�^�p�j
 */
typedef	struct {
	ulong			ID;						// ���ɂ��琸�Z�����܂ł��Ǘ�����ID
	ulong			CenterOiban;			// �Z���^�[�ǔ�
	ushort			CenterOibanFusei;		// �Z���^�[�ǔԕs���׸�
	DC_LANE_Info	LANE_Info;				// ���[�����j�^���

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	// �{�\���̂�ύX����ꍇ�͕K�� DC_LANE_log_date ���C������
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
} DC_LANE_log;
extern	DC_LANE_log		DC_LANE_work;		// ���[�����j�^�f�[�^�o�^�p���[�N�o�b�t�@

/**
 *	���[�����j�^�f�[�^(���[�����j�^���쐬�p)
 */
struct stLaneDataInfo {
	ulong			MediaParkNo;			// �}�� ���ԏꇂ
	ushort			Mediasyu;				// �}�� ���
	uchar			MediaNo[32];			// �}�� �ԍ�
	ushort			PaySyu;					// �������
	ulong			FreeNum;				// �t���[���l
	uchar			FreeStr[60];			// �t���[����
};
extern struct stLaneDataInfo m_stLaneWork;

// LOG���R�[�h���t�p
typedef	struct {
	unsigned long		ID;					// ���ɂ��琸�Z�����܂ł��Ǘ�����ID
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	ulong				CenterOiban;		// �Z���^�[�ǔ�
	ushort				CenterOibanFusei;	// �Z���^�[�ǔԕs���׸�
	ushort				FormRev;			// �t�H�[�}�b�gRev.��
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	stDatetTimeYtoSec_t	dtTimeYtoSec;		// �N���������b
} DC_LANE_log_date;

/**
 *	���[�����j�^�f�[�^(RXM-LCD��)
 */
typedef	struct {
	lcdbm_cmd_base_t	command;			///< ����ދ��ʏ��
	unsigned long		id;					///< ID(���ɂ��琸�Z�����܂ł��Ǘ�����ID)
	unsigned long		CenterOiban;		///< �Z���^�[�ǔ�
	unsigned short		CenterOibanFusei;	///< �Z���^�[�ǔԕs���׸�
	DC_LANE_Info		LANE_Info;			///< ���[�����j�^���
} lcdbm_cmd_lane_t;

#define LANE_PADSIZE	16-(2+sizeof(lcdbm_cmd_lane_t))%16	// �Í�������0�p�f�B���O�T�C�Y(16Byte�P�ʂƂ���)

typedef	struct {
	unsigned short			data_len;			// �d����
	lcdbm_cmd_lane_t		lane_dat;			// ���[�����j�^�f�[�^
	unsigned char			pad[LANE_PADSIZE];	// 16Byte�P�ʂƂ���ׂ�0�p�f�B���O�p�G���A
} lcdbm_cmd_lanedt_0pd_t;

// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

//--------------------------------------------------
//		�����e�i���X�f�[�^
//--------------------------------------------------
/**
 *	QR���[�_����v��
 */
typedef	struct {
	lcdbm_cmd_base_t	command;			// �R�}���h���ʏ��
	unsigned char		ctrl_cd;			// ����R�[�h(0:�o�[�W�����v��,1:�ǎ�J�n,2:�ǎ��~)
	unsigned char		reserve;			// �\��
} lcdbm_cmd_QR_ctrl_req_t;

/**
 *	���A���^�C���ʐM�a�ʗv��
 */
typedef	struct {
	lcdbm_cmd_base_t	command;			///< �R�}���h���ʏ��
} lcdbm_cmd_rt_con_req_t;


/**
 *	DC-NET�ʐM�a�ʗv��
 */
typedef	struct {
	lcdbm_cmd_base_t	command;			///< �R�}���h���ʏ��
} lcdbm_cmd_dc_con_req_t;
//--------------------------------------------------
//		�R�}���h�\���̒�`
//		�i LCD���W���[�� ==> ���C����� �j
//--------------------------------------------------

//--------------------------------------------------
//		�R�}���h���ʗ̈�
//--------------------------------------------------
/**
 *	LCD���W���[������̉����f�[�^�̋��ʕ��� �\����
 */
typedef	struct {
	unsigned short	id;			///< �R�}���h�h�c
	unsigned short	subid;		///< �T�u�R�}���h�h�c
} lcdbm_rsp_base_t;

//--------------------------------------------------
//		QR�֘A�f�[�^���ʗ̈�
//--------------------------------------------------
#define		BAR_ID_AMOUNT	(10000)		// QR���㌔
#define		BAR_ID_DISCOUNT	(10001)		// QR������
// GG129000(S) H.Fujinaga 2022/12/27 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ��j
#define		BAR_ID_TICKET	(10002)		// QR���Ԍ�
// GG129000(E) H.Fujinaga 2022/12/27 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ��j
// GG129000(S) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
#define QR_FORMAT_STANDARD 	(1)		// QR�R�[�h �W���t�H�[�}�b�g
#define QR_FORMAT_CUSTOM 	(2)		// QR�R�[�h �ʋ��ʃt�H�[�}�b�g
// GG129000(E) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j

/**
 *	LCD���W���[�������QR�֘A�f�[�^�̋��ʕ��� �\����
 */
// �o�[�R�[�h�����f�[�^
typedef struct{
	uchar			type;		// �o�[�R�[�h�^�C�v�P
	uchar			reserve;	// �\��
	ulong			data;		// �f�[�^1
} QR_DiscountData;
// ---------------------- //
// QR�f�[�^��� QR���㌔
// ---------------------- //
typedef struct{
	ulong			FacilityCode;	// �{�݃R�[�h
	ulong			ShopCode;		// �X�܃R�[�h
	ulong			PosNo;			// �X�ܓ��[���ԍ�
	ulong			IssueNo;		// ���V�[�g���s�ǔ�
	DATE_YMDHMS		IssueDate;		// QR���s�N���������b
	uchar			DataCount;		// �f�[�^��
	QR_DiscountData	BarData[3];		// �o�[�R�[�h�f�[�^
} QR_AmountInfo;

// ---------------------- //
// QR�f�[�^��� QR������
// ---------------------- //
typedef struct{
// MH810100(S) 2020/06/10 �d�l�ύX #4206 �y�A���]���w�E�����z�V�X�e���FQR�R�[�h�����j�[�N�ɂȂ�Ȃ��ꍇ������
	ulong			FacilityCode;	// �{�݃R�[�h
	ulong			ShopCode;		// �X�܃R�[�h
	ulong			PosNo;			// �X�ܓ��[���ԍ�
	ulong			IssueNo;		// ���V�[�g���s�ǔ�
// MH810100(E) 2020/06/10 �d�l�ύX #4206 �y�A���]���w�E�����z�V�X�e���FQR�R�[�h�����j�[�N�ɂȂ�Ȃ��ꍇ������
	DATE_YMDHMS		IssueDate;		// QR���s�N���������b
	uchar			reserve;		// �\���i�T�C�Y�����p�j
	ulong			ParkingNo;		// ���ԏ�ԍ�
	ushort			DiscKind;		// �������
	ushort			DiscClass;		// �����敪
	ushort			ShopNp;			// �X�ԍ�
	DATE_YMD		StartDate;		// �L���J�n�N����
	DATE_YMD		EndDate;		// �L���I���N����
} QR_DiscountInfo;

// GG129000(S) H.Fujinaga 2022/12/27 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ��j
// ---------------------- //
// QR�f�[�^��� QR���Ԍ�
// ---------------------- //
typedef struct{
	ulong			ParkingNo;			// ���ԏ�ԍ�
	ushort			EntryMachineNo;		// �����@�ԍ�
	ulong			ParkingTicketNo;	// ���Ԍ��ԍ�
	uchar			TicketLessMode;		// ��A���i�Ԕԃ`�P�b�g���X�A���L���j
	DATE_YMDHMS		IssueDate;			// QR���s�N���������b
} QR_TicketInfo;
// GG129000(E) H.Fujinaga 2022/12/27 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ��j

typedef union{
	uchar				data[BAR_INFO_SIZE];	// �T�C�Y�Œ�p
	QR_AmountInfo		AmountType;				// QR�f�[�^��� QR���㌔
	QR_DiscountInfo		DiscountType;			// QR�f�[�^��� QR������
// GG129000(S) H.Fujinaga 2022/12/27 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ��j
	QR_TicketInfo		TicketType;				// QR�f�[�^��� QR���Ԍ�
// GG129000(E) H.Fujinaga 2022/12/27 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ��j
} lcdbm_rsp_QR_com_u;

// MH810100(S) m.saito 2020/05/21 �Ԕԃ`�P�b�g���X�i#4178 �������̊����ύX�e�[�u���ݒ肪���f����Ȃ��j
// �����ύX�p�\����
typedef struct{
	DATE_YMDHMS		IssueDate;		// QR���s�N���������b
	DATE_YMD		StartDate;		// �L���J�n�N����
	DATE_YMD		EndDate;		// �L���I���N����
} QR_YMDData;
// MH810100(E) m.saito 2020/05/21 �Ԕԃ`�P�b�g���X�i#4178 �������̊����ύX�e�[�u���ݒ肪���f����Ȃ��j

// GG124100(S) R.Endo 2021/12/23 �Ԕԃ`�P�b�g���X3.0 #6123 NT-NET���Z�f�[�^�̎Ԕԏ��Z�b�g
//--------------------------------------------------
//		�Ԕԏ��
//--------------------------------------------------
typedef struct {
	uchar	LandTransOfficeName[12];	// �Ԕԁ@���^�x�ǖ�
	uchar	ClassNum[9];				// �Ԕԁ@���ޔԍ�
	uchar	Reserve1;					// �\��
	uchar	UsageCharacter[3];			// �Ԕԁ@�p�r����
	uchar	Reserve2;					// �\��
	uchar	SeqDesignNumber[12];		// �Ԕԁ@��A�w��ԍ�
	uchar	Reserve3[10];				// �\��
} SHABAN_INFO;
// GG124100(E) R.Endo 2021/12/23 �Ԕԃ`�P�b�g���X3.0 #6123 NT-NET���Z�f�[�^�̎Ԕԏ��Z�b�g

//--------------------------------------------------
//		�X�e�[�^�X�ʒm
//--------------------------------------------------
#define	PKT_VER_LEN	10

/**
 *	��{�ݒ艞��
 */
typedef struct {
	lcdbm_rsp_base_t	command;							///< �R�}���h���ʏ��
	unsigned char		prgm_ver[PKT_VER_LEN];				///< �v���O�����o�[�W����(LCD���䃂�W���[���\�t�g�E�F�A�̃o�[�W�������(ASCI))
	unsigned char		audio_ver[PKT_VER_LEN];				///< �����f�[�^�o�[�W����(�����f�[�^�̃o�[�W�������(ASCI))
	unsigned long		lcd_startup_time;					///< LCD���W���[���N������
	unsigned short		lcd_brightness;						///< LCD�P�x(0�`255)
	unsigned char		reserved[10];						///< �\��
} lcdbm_rsp_config_t;

/**
 *	�G���[�ʒm
 */
typedef struct {
	lcdbm_rsp_base_t	command;	///< �R�}���h���ʏ��
	unsigned short		kind;		///< �G���[���
	unsigned short		code;		///< �G���[�R�[�h
	unsigned char		occur;		///< ����/����(1:����,2:����,3:������������)
	unsigned short		year;		///< ����N(2000�`2099)
	unsigned char		month;		///< ��(01�`12)
	unsigned char		day;		///< ��(01�`31)
	unsigned char		hour;		///< ��(00�`23)
	unsigned char		minute;		///< ��(00�`59)
	unsigned char		second;		///< �b(00�`59)
	unsigned char		info[10];	///< �G���[���
} lcdbm_rsp_error_t;

//--------------------------------------------------
//		�e���L�[����
//--------------------------------------------------
/**
 *	�e���L�[�������
 */
typedef	struct {
	lcdbm_rsp_base_t	command;	///< �R�}���h���ʏ��
	///< �{�^�����(Bit�P��:0=�ʏ���(�����ꂽ),1=�������(�����ꂽ))
	unsigned char 		bit15:1;  	///< �e���L�[C
	unsigned char 		bit14:1;  	///< �e���L�[F5
	unsigned char 		bit13:1;  	///< �e���L�[F4
	unsigned char		bit12:1;  	///< �e���L�[F3
	unsigned char 		bit11:1;  	///< �e���L�[F2
	unsigned char 		bit10:1;  	///< �e���L�[F1
	unsigned char 		bit9:1;  	///< �e���L�[9
	unsigned char 		bit8:1; 	///< �e���L�[8
	unsigned char 		bit7:1;  	///< �e���L�[7
	unsigned char 		bit6:1;  	///< �e���L�[6
	unsigned char 		bit5:1;  	///< �e���L�[5
	unsigned char		bit4:1;  	///< �e���L�[4
	unsigned char 		bit3:1;  	///< �e���L�[3
	unsigned char 		bit2:1;  	///< �e���L�[2
	unsigned char 		bit1:1;  	///< �e���L�[1
	unsigned char 		bit0:1; 	///< �e���L�[0
} lcdbm_rsp_tenkey_info_t;

//--------------------------------------------------
//		���Z�@��Ԓʒm����
//--------------------------------------------------
/**
 *	����ʒm
 */
typedef	struct {
	lcdbm_rsp_base_t	command;	///< �R�}���h���ʏ��
	unsigned char		ope_code;	///< ����R�[�h(lcdbm_cmd_notice_ope_t�Q��)
	unsigned short		status;		///< ���(lcdbm_cmd_notice_ope_t�Q��)
} lcdbm_rsp_notice_ope_t;

//--------------------------------------------------
//		���Z�E�������ʒm
//--------------------------------------------------
/**
*	���ɏ��(��������)
 */
typedef	struct {
	lcdbm_rsp_base_t		command;			///< ����ދ��ʏ��
	unsigned long			id;					///< ID(���ɂ��琸�Z�����܂ł��Ǘ�����ID)

// GG129000(S) H.Fujinaga 2022/12/27 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ�/�R�����g�j
//	unsigned char			kind;				///< ���(0=�ԔԌ���/1=��������)
	unsigned char			kind;				///< ���(0=�ԔԌ���/1=��������/2=QR����)
// GG129000(E) H.Fujinaga 2022/12/27 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ�/�R�����g�j
	union {
//		unsigned char BYTE;
		struct {
			uchar	shaban[4];
			uchar	reserve[2];
		} shabanSearch;							// kind == �ԔԌ����̏ꍇ
		struct {
			unsigned short	Year;
			unsigned char	Month;
			unsigned char	Day;
			unsigned char	Hours;
			unsigned char	Minute;
		} nichijiSearch;						// kind == ���������̏ꍇ
// GG129000(S) H.Fujinaga 2023/02/24 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
		struct {
			uchar	state;						// ���u���Z(���Z���ύX)���Ɏg�p 1:����,2:2��ڈȍ~(QR����)
			uchar	reserve[5];
		} PayInfoChange;						// kind == ���u���Z�̏ꍇ
// GG129000(E) H.Fujinaga 2023/02/24 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
	} data;
	uchar	shaban[4];							// �����⍇�����̎Ԃ̎Ԕ�
	uchar	shubetsu;							// ���(0=���O���Z�^1�������Z�o�ɐ��Z)
// GG124100(S) R.Endo 2021/12/23 �Ԕԃ`�P�b�g���X3.0 #6123 NT-NET���Z�f�[�^�̎Ԕԏ��Z�b�g
	SHABAN_INFO				ShabanInfo;			// �Ԕԏ��
// GG124100(E) R.Endo 2021/12/23 �Ԕԃ`�P�b�g���X3.0 #6123 NT-NET���Z�f�[�^�̎Ԕԏ��Z�b�g
// GG124100(S) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(LCD-RXM���Z�J�n��������)
	DATE_YMDHMS				PayDateTime;		// ���Z�J�n����
// GG124100(E) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(LCD-RXM���Z�J�n��������)

	unsigned short			crd_info_rev_no;	///< ���ޏ��Rev.No.
// GG124100(S) R.Endo 2022/06/13 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
// 	stRecvCardInfo_Rev10	crd_info;			///< ���ޏ��(���ɏ��)
// GG129004(S) M.Fujikawa 2024/12/04 AMS���P�v�] �J�[�h���Rev.12�Ή�
//	stRecvCardInfo_Rev11	crd_info;			///< ���ޏ��(���ɏ��)
	stRecvCardInfo_RevXX	crd_info;			///< ���ޏ��(���ɏ��)
// GG129004(E) M.Fujikawa 2024/12/04 AMS���P�v�] �J�[�h���Rev.12�Ή�
// GG124100(E) R.Endo 2022/06/13 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
} lcdbm_rsp_in_car_info_t;

/**
 *	QR�f�[�^
 */
typedef	struct {
	lcdbm_rsp_base_t	command;				// �R�}���h���ʏ��
	ushort				id;						// QR�R�[�hID
	ushort				rev;					// QR�R�[�h�t�H�[�}�b�gRev.
	ushort				data_size;				// QR�R�[�h�f�[�^(���f�[�^)�T�C�Y
	ushort				enc_type;				// QR�R�[�h�G���R�[�h�^�C�v
	uchar				data[BAR_DATASIZE];		// QR�R�[�h�f�[�^(���f�[�^)
	ushort				info_size;				// QR�f�[�^���(�p�[�X�f�[�^)�T�C�Y
	lcdbm_rsp_QR_com_u	QR_data;				// �������z�`�\��
// GG129000(S) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
	ushort				qr_type;				// QR�R�[�h�t�H�[�}�b�g�^�C�v
// GG129000(E) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
} lcdbm_rsp_QR_data_t;

/**
 *	���Z�����f�[�^
 */
typedef	struct {
	lcdbm_rsp_base_t	command;				///< �R�}���h���ʏ��
	unsigned long		id;						///< ID(���ɂ��琸�Z�����܂ł��Ǘ�����ID)
	unsigned char		result;					///< ����(0:OK,1:NG)
	unsigned char		reserve;				///< �\��
} lcdbm_rsp_pay_data_res_t;

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
/**
 *	�̎��؃f�[�^����
 */
typedef	struct {
	lcdbm_rsp_base_t	command;				///< �R�}���h���ʏ��
	unsigned long		id;						///< ID(���ɂ��琸�Z�����܂ł��Ǘ�����ID)
	unsigned char		result;					///< ����(0:OK,1:NG)
	unsigned char		reserve;				///< �\��
} lcdbm_rsp_receipt_data_res_t;
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

/**
 *	QR�m��E����f�[�^����
 */
typedef	struct {
	lcdbm_rsp_base_t	command;				///< �R�}���h���ʏ��
	unsigned long		id;						///< ID(���ɂ��琸�Z�����܂ł��Ǘ�����ID)
	unsigned char		result;					///< ����(0:OK,1:NG)
	unsigned char		reserve;				///< �\��
} lcdbm_rsp_QR_conf_can_res_t;

// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
/**
 *	���[�����j�^�f�[�^����
 */
typedef	struct {
	lcdbm_rsp_base_t	command;				///< �R�}���h���ʏ��
	unsigned long		id;						///< ID(���ɂ��琸�Z�����܂ł��Ǘ�����ID)
	unsigned short		result;					///< ����(0:OK,1:NG)
	unsigned char		reserve;				///< �\��
} lcdbm_rsp_LANE_res_t;
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

//--------------------------------------------------
//		�����e�i���X�f�[�^
//--------------------------------------------------
/**
 *	QR���[�_���䉞��
 */
typedef	struct {
	lcdbm_rsp_base_t	command;			///< �R�}���h���ʏ��
	unsigned char		ctrl_cd;			///< ����R�[�h(�v����������R�[�h)
	unsigned char		result;				///< ����(00H:����,01H:�ُ�)
	unsigned char		part_no[32];		///< �i��
	unsigned char		serial_no[32];		///< �V���A���i���o�[
	unsigned char		version[32];		///< �o�[�W����
} lcdbm_rsp_QR_ctrl_res_t;

/**
 *	QR�ǎ挋��
 */
typedef	struct {
	lcdbm_rsp_base_t	command;			///< �R�}���h���ʏ��
	unsigned char		result;				///< ����(00H:����,01H:�ُ�)
	ushort				id;						// QR�R�[�hID
	ushort				rev;					// QR�R�[�h�t�H�[�}�b�gRev.
	ushort				data_size;				// QR�R�[�h�f�[�^(���f�[�^)�T�C�Y
	ushort				enc_type;				// QR�R�[�h�G���R�[�h�^�C�v
	uchar				data[BAR_DATASIZE];		// QR�R�[�h�f�[�^(���f�[�^)
	ushort				info_size;				// QR�f�[�^���(�p�[�X�f�[�^)�T�C�Y
	lcdbm_rsp_QR_com_u	QR_data;				// �������z�`�\��
// GG129000(S) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
	ushort				qr_type;				// QR�R�[�h�t�H�[�}�b�g�^�C�v
// GG129000(E) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
} lcdbm_rsp_QR_rd_rslt_t;

/**
 *	���A���^�C���ʐM�a�ʌ���
 */
typedef	struct {
	lcdbm_rsp_base_t	command;			///< �R�}���h���ʏ��
	unsigned char		result;				///< ����(00H:����,01H:�ʐM�ُ�(TCP�ڑ��s��),02H:�^�C���A�E�g)
} lcdbm_rsp_rt_con_rslt_t;


/**
 *	DC-NET�ʐM�a�ʌ���
 */
typedef	struct {
	lcdbm_rsp_base_t	command;			// �R�}���h���ʏ��
	unsigned char		result;				// ����(00H:����,01H:�ʐM�ُ�(TCP�ڑ��s��),02H:�^�C���A�E�g)
} lcdbm_rsp_dc_con_rslt_t;
//--------------------------------------------------
//		��M�d���̕������p
//--------------------------------------------------
/**
 *	�Í������ꂽ��M�d���̕������p�o�b�t�@
 */
#define DECRYPT_COM	4096	///< �P�d���̍ő咷�͗]�T��������4096byte�Ƃ���

//--------------------------------------------------
//		�R�}���h���p��
//--------------------------------------------------
/**
 *	���M�R�}���h�i ���C����� ==> LCD���W���[�� �j���p��
 */
typedef	union {

	lcdbm_cmd_base_t			command;		///< �R�}���h���ʏ��

	/** �X�e�[�^�X�v�� */
	lcdbm_cmd_config_t			config;			///< ��{�ݒ�v��

	/** �@��ݒ�v�� */
	lcdbm_cmd_clock_t			clock;			///< ���v�ݒ�
	lcdbm_cmd_brightness_t		brightness;		///< LCD�P�x

	/** �e�L�X�g */
	/*
	 *	�e�L�X�g�͕�����f�[�^�̍쐬���P�ӏ��ōs���Ȃ��i���ꂽ�^�C�~���O�ōs���j�\�����l����
	 *	���̃R�}���h�Ƃ͕ʃ��[�N�ɍ쐬����B
	 */

	/** �A�i�E���X�v�� */
	lcdbm_cmd_audio_volume_t	au_volume;		///< ���ʐݒ�
	lcdbm_cmd_audio_start_t		au_start;		///< �J�n�v��
	lcdbm_cmd_audio_end_t		au_end;			///< �I���v��

	/** �u�U�[�v�� */
	lcdbm_cmd_beep_volume_t		bp_volume;		///< ���ʐݒ�
	lcdbm_cmd_beep_start_t		bp_start;		///< �u�U�[���v��

	/** ���Z�@��Ԓʒm���� */
	lcdbm_cmd_notice_ope_t		operate;		///< ����ʒm
	lcdbm_cmd_notice_opn_t		opn_cls;		///< �c�x�ƒʒm
	lcdbm_cmd_notice_pay_t		pay_state;		///< ���Z��Ԓʒm
	lcdbm_cmd_notice_dsp_t		display;		///< �\���v��
	lcdbm_cmd_notice_alm_t		alarm;			///< �x���ʒm
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
	lcdbm_cmd_notice_del_t		display_del;	///< �폜�v��
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j

	/** ���Z�E�������ʒm */
	lcdbm_cmd_pay_rem_chg_t		pay_rem_chg;	///< ���Z�c���ω��ʒm
	lcdbm_cmd_pay_data_t		pay_data;		///< ���Z�f�[�^
	lcdbm_cmd_QR_conf_can_t		QR_conf_can;	///< QR�m��E����f�[�^
	lcdbm_cmd_QR_data_res_t		QR_data_res;	///<QR�f�[�^����
// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
	lcdbm_cmd_remote_time_start_t	remote_time_start;	// ���Ɏ����w�艓�u���Z�J�n
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	lcdbm_cmd_lane_t			lane_data;		///< ���[�����j�^�f�[�^
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	lcdbm_cmd_receipt_data_t	receipt_data;	///< �̎��؃f�[�^
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

	/** �����e�i���X�f�[�^ */
	lcdbm_cmd_QR_ctrl_req_t		QR_ctrl_req;	///< QR���[�_����v��
	lcdbm_cmd_rt_con_req_t		rt_con_req;		///< ���A���^�C���ʐM�a�ʗv��
	lcdbm_cmd_dc_con_req_t		dc_con_req;		// DC-NET�ʐM�a�ʗv��

} lcdbm_cmd_work_t;

/**
 *	��M�f�[�^�i LCD���W���[�� ==> ���C����� �j���p��
 */
typedef	union {

	lcdbm_rsp_base_t			command;		///< �R�}���h���ʏ��

	/** �X�e�[�^�X�ʒm */
	lcdbm_rsp_config_t			config;			///< ��{�ݒ艞��

	/** �G���[�ʒm */
	lcdbm_rsp_error_t			error;			///< �G���[�ʒm

	/** �e���L�[���� */
	lcdbm_rsp_tenkey_info_t		tenkey;			///< �e���L�[�������

	/** ���Z�@��Ԓʒm���� */
	lcdbm_rsp_notice_ope_t		operate;		///< ����ʒm

	/** ���Z�E�������ʒm */
	lcdbm_rsp_in_car_info_t		pay_rem_chg;	///< ���ɏ��
	lcdbm_rsp_QR_data_t			QR_data;		///< QR�f�[�^
	lcdbm_rsp_pay_data_res_t	pay_data_res;	///< ���Z�����f�[�^
	lcdbm_rsp_QR_conf_can_res_t	QR_conf_can_res;///< QR�m��E����f�[�^����
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	lcdbm_rsp_LANE_res_t		lane_data_res;	///< ���[�����j�^�f�[�^
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	lcdbm_rsp_receipt_data_res_t receipt_data_res;	///< �̎��؃f�[�^����
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�


} lcdbm_rsp_work_t;
#pragma unpack

#endif	// __PKT_DEF_H__
// MH810100(E) K.Onodera 2019/11/11 �Ԕԃ`�P�b�g���X(GT-8700(LZ-122601)���p)