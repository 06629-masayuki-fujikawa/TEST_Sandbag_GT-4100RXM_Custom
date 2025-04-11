// MH810100(S) K.Onodera 2019/11/11 �Ԕԃ`�P�b�g���X(GT-8700(LZ-122601)���p)
/**
 *	@file	lcdbm_ctrl.h
 *	@brief	LCD���W���[������w�b�_�t�@�C��
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@since	2009/10/08 08:33:50
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
#ifndef _LCDBM_CTRL_H_
#define _LCDBM_CTRL_H_

//--------------------------------------------------
//		INCLUDE
//--------------------------------------------------
#include "system.h"
#include "common.h"
//#include "mdl_def.h"
#include "pkt_def.h"
#include "lcdbm_def.h"
//#include "lcdbm_popup.h"
//#include "lcdbm_anime.h"

// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�ZDLL�C���^�[�t�F�[�X�̏C��)
// #define	_countof(a)				(sizeof(a) / sizeof((a)[0]))
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�ZDLL�C���^�[�t�F�[�X�̏C��)

//-#define	LCDBM_AUDIO_COMMAND_QUEUE_DEPTH		5	///< LCD���W���[���̉����n�R�}���h�̍ő�ێ���
/** LCD���W���[���̂����M����o�[�W�������� */
#define	LCDBM_VERSION_LENGTH				10	// �o�[�W������

//-enum {
//-	LCDBM_EVENT_ID_AUDIO,	///< �����n�C�x���g
//-	LCDBM_EVENT_ID_IMAGE,	///< �摜�n�C�x���g
//-	LCDBM_EVENT_ID_NUM,		///< �C�x���g�h�c��
//-};

/**
 *	LCD���W���[������\����
 */
typedef struct {

//-	/** ���� */
//-	struct {
//-		unsigned char	no;		///< �J�����g����i�O�����{��A�P���p��A�Q���؍���A�R��������j
//-		unsigned char	index;	///< ���݂̑I������C���f�b�N�X�i�O�`LCDBM_LANGUAGE_CONFIG_NUM�j
//-
//-	} language;
//-
//-	/** ��� */
//-	struct {
//-		unsigned short	base_No;	///< ���ݕ\�����̊�{��ʔԍ�
//-		unsigned char	part_No;	///< �Đ��I���������ԍ�
//-		unsigned short	lcd_no;		// LCDNO��ۑ�����i�\���������̂��߂ɓ�����ʂ����x���\������̂�}������j
//-	} display;
//-
//-	/** ���� */
//-	struct {
//-		unsigned char	volume;			///< LCD���W���[���ɐݒ肵���{�����[���l�i�O�`LCDBM_VOLUME_MAX-1�j
//-		unsigned long	command_No;		///< �O��̃R�}���h���ʔԍ��i�P�ȏ�Ƃ���j
//-		unsigned long	issued_buf[LCDBM_AUDIO_COMMAND_QUEUE_DEPTH];	///< �I���҂��R�}���h���ʔԍ���ێ����Ă����o�b�t�@.
//-
//-	} audio;

	/** LCD���W���[�������M�����f�[�^ */
	struct {
		// ��{�ݒ艞��
		struct {
			unsigned char	prgm_ver[LCDBM_VERSION_LENGTH];		///< �v���O�����o�[�W����(LCD���䃂�W���[���\�t�g�E�F�A�̃o�[�W�������(ASCI))
			unsigned char	audio_ver[LCDBM_VERSION_LENGTH];	///< �����f�[�^�o�[�W����(�����f�[�^�̃o�[�W�������(ASCI))
			unsigned long	lcd_startup_time;					///< LCD���W���[���N������
			unsigned short	lcd_brightness;						///< LCD�P�x(0�`255)
			unsigned char	reserved[10];						///< �\��
		} config;
//		// ��ԉ���
//		struct {
//			unsigned short	progress;		///< LCD���W���[�����
//			unsigned long	SD_writing;		///< SD�J�[�h�������݉�
//		} status;
//		// �ʐM�ݒ艞��
//		struct {
//			unsigned char	result;			///< �������� 00H�F���� 00H�ȊO�F���s
//			unsigned char	hostip[12];		///< ����IP�A�h���X�iASCII�j
//			unsigned char	lcdip[12];		///< LCD���W���[��IP�A�h���X�iASCII�j
//			unsigned char	hostsubnet[12];	///< ���ǃ��W���[���T�u�l�b�g�}�X�N�iASCII�j
//			unsigned char	hostgateway[12];///< ���ǃ��W���[���f�t�H���g�Q�[�g�E�F�C�iASCII�j
//			unsigned char	lcdtcpport[5];	///< LCD���W���[��TCP�|�[�g�iASCII�j
//		} tcpip;
//		// SD�J�[�h�e�X�g
//		struct {
//			unsigned char		res;		///< ���ʁi0=OK�A1=SD�������A2=���[�h�I�����[�j
//			unsigned long		readtime;	///< SD���[�h�^�C���i�P�ʁF�~���b�j
//			unsigned long		writetime;	///< SD���C�g�^�C���i�P�ʁF�~���b�j
//		} sdreadwrite;
//		// �e�ʃ`�F�b�N
//		struct {
//			unsigned char		res;		///< ���ʁi0=OK�j
//			unsigned long		freesize;	///< �󂫃T�C�Y�i�P�ʁFMB�j
//			unsigned long		totalsize;	///< �S�̃T�C�Y�i�P�ʁFMB�j
//		} sdcapacitycheck;
//		// ���\�e�X�g
//		struct {
//			unsigned char		res;		///< ���ʁi0=OK�A1=NG�j
//			unsigned long		speed;		///< ���\*10�i�P�ʁFKB/S�j
//		} sdperformance;
	} receive_data;

//-	/** �C�x���g�t���O */
//-	struct {
//-		unsigned short	bit;	/**< �C�x���g�����҂����킹�t���O
//-								 *	�C�x���g���������Ă��Ȃ��ꍇ�ɂ͊Y���r�b�g���O�ƂȂ��Ă���B
//-								 *	�C�x���g�����҂��ɓ���O�ɊY���r�b�g���O�N���A���Ă����B
//-								 *	�C�x���g�����������ۂɂ͊Y������r�b�g���P�ɂ���B�i�C�x���g��������������\���j
//-								 *	�҂��Ă���S�ẴC�x���g�������I���������ǂ����͊Y������r�b�g�p�^�[���Ɣ�r���鎖�Ŕ��肷��B�iAND�҂��j
//-								 *	�҂��Ă���ǂꂩ�̃C�x���g�������I���������ǂ����͊Y������r�b�g�p�^�[�����O���ǂ����Ŕ��肷��B�iOR�҂��j
//-								 */
//-	} event[LCDBM_EVENT_ID_NUM];

	/** �e��X�e�[�^�X */
	struct {
//-		BOOL	restart;				///< LCD���W���[���̂P��ڂ̍ċN�������o�ς݂��ǂ���
//-		BOOL	detail;					///< ��������\���ς݂��ǂ���
//-		BOOL	image_base;				///< ��{��ʂ������I�ɏ��������邩�ǂ���
//-		unsigned char	return_card;	/**< �ԋp�J�[�h���
//-										 *	cardANOTHER =	�Ȃ�
//-										 *	cardTICKET =	���Ԍ�
//-										 *	cardPASS =		�����
//-										 */
//-		struct {
//-			unsigned char	WarSts;		///< ���[�j���O���
//-			unsigned char	ReciSw;		///< �̎��؃{�^��������
//-		} Ope2_Sale_StsDisp;
//-		struct {
//-			unsigned short	change;			///< ��؂�\��������r�b�g��OR�w�肷��iLCDBM_CHANGE_BIT_10�`�j
//-			unsigned short	waiting;		///< �ҋ@��ʂŕ\������A���[���iLCDBM_WAITING_ALARM_CHANGE�`�j
//-			short			count_500ms;	///< �ҋ@��ʂŕ\������A���[���̐ؑւ��Ԋu�J�E���^
//-		} alarm;
		int		keepalive;
	} status;

} lcdbm_ctrl_t;

/**
 *	LCD���W���[���ݒ���\����
 */
typedef struct {
	unsigned char	Version[LCDBM_VERSION_LENGTH];					// �\�t�g�E�F�A�o�[�W����
	unsigned char	Res1[2];										// �\��
	unsigned char	SDVersion[LCDBM_VERSION_LENGTH];				// SD�J�[�h�f�[�^�o�[�W����
	unsigned char	Res2;											// �\��
	unsigned char	SD_read_time;									// SD�J�[�h���[�h����
	unsigned char	Res3[2];										// �\��
	unsigned short	brightness;										// �P�x�l
	unsigned char	touch;											// �^�b�`�p�l���L��
	unsigned char	Res4;											// �\��
	unsigned char	Res5;											// �\��
	unsigned char	tenkey;											// �e���L�[�Ή�
	unsigned char	ioext1;											// �g��IO 1
	unsigned char	ioext2;											// �g��IO 2
	unsigned char	ioext3;											// �g��IO 3
	unsigned char	ioext4;											// �g��IO 4
	unsigned char	rotsw;											// ���[�^���[�X�C�b�`
	unsigned char	dipsw;											// �f�B�b�v�X�C�b�`
} lcdbm_base_info_t;

/**
 *	ARCNET����M�f�[�^�\����
 *	�i�u���b�N���A�f�[�^���A�f�[�^�{���A���\���̉��j
 */
//typedef struct red_rec	red_rec_t;
//--------------------------------------------------
//		DEFINE
//	�iGT-7000�֘A�̏����`����j
//--------------------------------------------------
#define	LCD_HORIZONTAL_CHARS	31		// ����LCD�̉��������i���p��30�����{'\0'��1�����j
#define	AVM_VOLUME_MAX			32		// AVM3�̉��ʐݒ�i�K�i�ݒ�\�͈͂� 0�`AVM_VOLUME_MAX-1�j


//--------------------------------------------------
//		DEFINE
//	�iGT-7700�֘A�̏����`����j
//--------------------------------------------------
/**
 *	LCD���W���[��������
 */
enum {
	LCDBM_PROGRESS_BOOT,			///< 0: �N���J�n
	LCDBM_PROGRESS_LOGO,			///< 1: ������ʁiLCD���W���[�����ÓI�ɕێ����Ă����Ѓ��S��ʁj�\����
	LCDBM_PROGRESS_SCREEN1,			///< 2: ������ʕ\���I���i���C���������{��ʕ\���R�}���h�����s���ꂽ��j
	LCDBM_PROGRESS_CHARGED,			///< 3: �d��������~�d����
	LCDBM_PROGRESS_INITIALIZED,		///< 4: ����������
};

/**
 *	�t�H���g���
 *	@note
 *	-	���p�̏ꍇ�͉��T�C�Y�������ɂȂ�B�i�c�T�C�Y�͕ς��Ȃ��j
 *	-	�T�C�Y�͉��i2009/10/29���݁j
 */
enum {
	LCDBM_TEXT_FONT_16,			///< 0: �S�p16x16
	LCDBM_TEXT_FONT_24,			///< 1: �S�p24x24
	LCDBM_TEXT_FONT_30,			///< 2: �S�p30x30�i�g�p���镶���̂݃f�[�^�����j
	LCDBM_TEXT_FONT_32,			///< 3: �S�p32x32�i�����ׂ��̂Ŏg��Ȃ��\����j
	LCDBM_TEXT_FONT_36,			///< 4: �S�p36x36
	LCDBM_TEXT_FONT_64,			///< 5: �S�p64x64�i�g�p���镶���̂݃f�[�^�����j
	LCDBM_TEXT_FONT_TYPES,		///< �t�H���g��ʐ�
};


/**
 *	�񂹎w��
 */
enum {
	LCDBM_TEXT_ALIGN_LEFT,		///< 0: ����
	LCDBM_TEXT_ALIGN_CENTER,	///< 1: ������
	LCDBM_TEXT_ALIGN_TYPES,		///< �񂹎w�� ��ʐ�
};


/**
 *	���v�t�H�[�}�b�g
 */
enum{
	LCDBM_TEXT_CLOCK_JAPANESE =	0,		///< ���{��\�L
	LCDBM_TEXT_CLOCK_ENGLISH =	400,	///< �p��\�L
	LCDBM_TEXT_CLOCK_TYPES,				///< ���v���� ��ʐ�
};


/**
 *	����
 */
#define	LCDBM_VOLUME_MAX	128		///< LCD���W���[���̉��ʐݒ�i�K�i�ݒ�\�͈͂� �O�`AVM_VOLUME_MAX-1�j


/**
 *	���
 */
#define	LCDBM_LCD_WIDTH				800		///< �k�b�c�̉��h�b�g��
#define	LCDBM_LCD_HEIGHT			600		///< �k�b�c�̏c�h�b�g��
#define	LCDBM_LCD_BRIGHTNESS_MAX	15		///< �k�b�c�o�b�N���C�g�P�x�ݒ�̍ő�l
#define	LCDBM_LCD_CONTRAST_MAX		15		///< �k�b�c�R���g���X�g�ݒ�̍ő�l


/**
 *	���ߌ��E�ԋp��
 */
enum {
	LCDBM_FPORT_COIN,		// �d��
	LCDBM_FPORT_RECEIPT,	// �̎��؁E�a���
	LCDBM_FPORT_NOTE_BNA,	// �����i�����j
	LCDBM_FPORT_NOTE_BND,	// �����i�o���j
	LCDBM_FPORT_CARD,		// ���C��(���̎��؊�)
	LCDBM_FPORT_GATE,		// �Q�[�g
	LCDBM_FPORT_ALL,		// ���ׂ�
	// ��������ɒǉ�����

	LCDBM_FPORT_DUMMY,		// ���o�����^�C�}�[�ăZ�b�g�Ŏg��
};


/**
 *	�e�L�X�g�\���n(lcdbm_grachr)
 */
#define	LCDBM_STRING		0												// �����񒼐ڎw��
#define	LCDBM_COLOR_DEFAULT	0xFFFF0000										// �����F:�f�t�H���g
#define	LCDBM_COLOR_BLACK			((ulong)lcdbm_color_RGB( 0,  0,  0))	// �����F:�u���b�N
#define	LCDBM_COLOR_RED				((ulong)lcdbm_color_RGB(31,  0,  0))	// �����F:���b�h
#define	LCDBM_COLOR_GREEN			((ulong)lcdbm_color_RGB( 7, 31, 11))	// �����F:�O���[��
#define	LCDBM_COLOR_BLUE			((ulong)lcdbm_color_RGB( 0,  0, 31))	// �����F:�u���[
#define	LCDBM_COLOR_MIDNIGHTBLUE	((ulong)lcdbm_color_RGB( 2,  4,  8))	// �����F:�~�b�h�i�C�g�u���[
#define	LCDBM_COLOR_PALETURQUOISE	((ulong)lcdbm_color_RGB(21, 59, 29))	// �����F:�p�[���^�[�R�C�Y
#define	LCDBM_COLOR_LIGHTSEAGREEN	((ulong)lcdbm_color_RGB( 4, 44, 21))	// �����F:���C�g�V�[�O���[��
#define	LCDBM_COLOR_DODGERBLUE		((ulong)lcdbm_color_RGB( 3, 18, 31))	// �����F:�h�W���[�u���[
#define	LCDBM_COLOR_MEDIUMBLUE		((ulong)lcdbm_color_RGB( 0, 0, 25))		// �����F:�~�f�B�A���u���[
#define	LCDBM_NO_BLINK		0												// �u�����N�w��Ȃ�
#define	LCDBM_ATTR_NORMAL	0												// �ʏ푮��
#define	LCDBM_ATTR_WARNING	1												// �x������


/**
 *	�P�x�����n(lcdbm_lumine_change)
 */
#define	LCDBM_LUMINE_MAX	16		// �ő�P�x�l


/**
 *	LCD���[�j���O�\������(�J���[LCD)
 */
typedef struct {
	ushort	DspSts;				// LCD_WMSG_ON:���[�j���O�\�����CLCD_WMSG_OFF:���[�j���O��\����
	uchar	TextArea;			// ���[�j���O�\�����̃e�L�X�g�G���A(�����o��:LCDBM_AREA_TEXT__PAYING_CAPTION_2�C�|�b�v�A�b�v:LCDBM_AREA_TEXT__POPUP)
	ushort	excode;				// ��O�\���R�[�h(0-300)
} T_LCDBM_WMSG_CNT;

extern	T_LCDBM_WMSG_CNT	lcdbm_Lcd_Wmsg_Cnt;	// ���[�j���O�\������G���A


/**
 *	�e�L�X�g�X�N���[��
 */
#define	LCDBM_TEXT_SCROLL_DEFAULT	float_fix16( 5.0 )		///< �e�L�X�g�X�N���[���ʂ̃f�t�H���g�l


//--------------------------------------------------
//		�^��`
//--------------------------------------------------
/**
 *	LCD���W���[�� �P�U�r�b�g�J���[�f�[�^
 */
typedef union {
	unsigned short	data;
	struct {
		unsigned short	R:5;	///< �ԁi0�`31�j
		unsigned short	G:6;	///< �΁i0�`63�j
		unsigned short	B:5;	///< �i0�`31�j
	} bit;
} lcdbm_color16_t;


/**
 *	�Œ菬���_�^
 *
 *	����������ʂP�Q�r�b�g�A�����������ʂS�r�b�g
 */
typedef unsigned short	lcdbm_fix16;


/**
 *	�w�A�x���W
 */
typedef struct {
	ushort	x;		///< X���W
	ushort	y;		///< Y���W
} lcdbm_pos_t;


//--------------------------------------------------
//		�}�N����`
//--------------------------------------------------
/**
 *	float�l���������S�r�b�g�̌Œ菬���`���ilcdbm_fix16�^�j�ɕϊ�����
 */
#define	float_fix16( x )	( (lcdbm_fix16)(((float)(x) * 16.0f)) )

/**
 *	LCD���W���[���Ƃ̓d���̃R�}���h�h�c�ƃT�u�R�}���h�h�c����S�o�C�g�̂h�c�R�[�h���v�Z����
 */
#define	lcdbm_command_id( id, subid )	( (( (unsigned long)id & 0x0000ffff ) << 16 ) | ( (unsigned long)subid & 0x0000ffff) )

/**
 *	16�r�b�g�J���[�l���쐬����
 */
#define	lcdbm_color_RGB( r, g, b )	((unsigned short)( (((r)&0x1f) << 11) | (((g)&0x3f) << 5) | (((b)&0x1f) << 0) ))

/**
 *	LCD���W���[���̋P�x�l�ɕϊ�����
 */
#define	lcdbm_lumine(n)	(unsigned short)((n)?(n*LCDBM_LUMINE_MAX-1):0)	// 1�`16:�P�x�l(15�`255)�C0:BackLight=OFF


//--------------------------------------------------
//		�e�L�X�g�ԍ��f�[�^�Ǘ�
//--------------------------------------------------
/**
 *	�e�L�X�g�t�@�C��No.���w�肷��ꍇ�̍ő�w�萔
 *	�{���� pkt_def.h �ɒu���ׂ��������d�C���N���[�h�ɂȂ��Ă��܂��̂ŁA�����Œ�`����B
 */
#define	TEXT_CMD_FILENO_NUM		10

/**
 *	�e�L�X�g�ԍ���f�[�^�i�ԍ��i�[�z�񂠂�j
 */
typedef struct {
	unsigned short	no[ TEXT_CMD_FILENO_NUM ];	///< �e�L�X�g�ԍ��i�[�z��
	unsigned short	number;						///< �e�L�X�g�ԍ��i�[��
} lcdbm_text_array_t;

/**
 *	�e�L�X�g�ԍ���f�[�^�i�ԍ��i�[�̈�ւ̃|�C���^����j
 */
typedef struct {
	unsigned short	*p;			///< �e�L�X�g�ԍ��i�[�z��ւ̃|�C���^
	unsigned short	number;		///< �e�L�X�g�ԍ��i�[��
} lcdbm_text_no_t;

/**
 *	�e�L�X�g�ԍ���f�[�^�̊Ǘ�
 */
typedef struct {
	lcdbm_text_no_t	*p;			///< �e�L�X�g�ԍ��i�[�z��ւ̃|�C���^
	unsigned short	number;		///< �e�L�X�g�ԍ��i�[��
} lcdbm_text_no_table_t;

/**
 *	�e�L�X�g�ԍ���̃f�[�^��`�}�N��
 *	@note
 *	-	lcdbm_text_no_t�^�̔z����`���鎞�Ɏg��
 */
#define	lcdbm_text( name )	{ &(name)[0], _countof(name), }


//--------------------------------------------------
//		�v���g�^�C�v�錾
//--------------------------------------------------
// �R�}���h���b�Z�[�W�`�F�b�N
ushort lcdbm_check_message( const ulong data );

// �L�[�v�A���C�u���g���C��ԃ`�F�b�N
BOOL lcdbm_check_keepalive_status( uchar cnt );

// �L�[�v�A���C�u���g���C�J�E���^���Z�b�g
void lcdbm_reset_keepalive_status( void);

// ��{�ݒ艞�� ��M������
void lcdbm_receive_config( lcdbm_rsp_config_t *p_rcv );

// ���ɏ�� ��M������
void lcdbm_receive_in_car_info( lcdbm_rsp_in_car_info_t *p_rcv );

// QR�f�[�^ ��M������
void lcdbm_receive_QR_data( lcdbm_rsp_QR_data_t *p_rcv );

// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
// ���[�����j�^�f�[�^���� ��M������
void lcdbm_receive_DC_LANE_res( lcdbm_rsp_LANE_res_t *p_rcv, ulong *id, uchar *result );
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

// ���Z�����f�[�^ ��M������
void lcdbm_receive_RTPay_res( lcdbm_rsp_pay_data_res_t *p_rcv, ulong *id, uchar *result );

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// �̎��؃f�[�^���� ��M������
void lcdbm_receive_RTReceipt_res( lcdbm_rsp_receipt_data_res_t *p_rcv, ulong *id, uchar *result );
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

// QR�m��E��������f�[�^ ��M������
void lcdbm_receive_DC_QR_res( lcdbm_rsp_QR_conf_can_res_t *p_rcv, ulong *id, uchar *result );

// QR���[�_���䉞�� ��M������
void lcdbm_receive_QR_ctrl_res( lcdbm_rsp_QR_ctrl_res_t *p_rcv );

// QR�ǎ挋�� ��M������
void lcdbm_receive_QR_rd_rslt( lcdbm_rsp_QR_rd_rslt_t *p_rcv );

// LCD���䃂�W���[���\�t�g�E�F�A�̃o�[�W��������Ԃ�
uchar lcdbm_get_config_prgm_ver(char *dst, uchar size);

// �����f�[�^�̃o�[�W��������Ԃ�
uchar lcdbm_get_config_audio_ver(char *dst, uchar size);

// LCD���W���[���N�����Ԃ�Ԃ�
ulong lcdbm_get_config_lcd_startup_time(void);

// LCD�P�x��Ԃ�
ushort lcdbm_get_config_lcd_brightness(void);

// �擾������{�ݒ艞���̐擪�|�C���^��Ԃ�
uchar* lcdbm_get_config_ptr(void);

//-// LCD���W���[�� �C�x���g����
//-int lcdbm_receive_event( const unsigned short message_id );
//-
//-// �p�P�b�g�ʐM�^�X�N�֘A�̎�M���[�����N���A�i�p���j����
//-//void lcdbm_Ope_MsgBoxClear( const MsgBuf * const msb );

//--------------------------------------------------
//		�C���N���[�h
//--------------------------------------------------
//#include "lcdbm_sub.h"		// �T�u���[�`���Q
void number_string_to_zenkaku( const unsigned short *dst, const unsigned char *src, const unsigned short count );
void number_to_zenkaku( unsigned short *dst, const unsigned long number, unsigned short count );
void intoasl_zrschr(ushort *buf, const ulong data, uchar count);
void zrschr_al(char *dat, uchar cnt);
void zrschr( unsigned char* dat, unsigned char cnt );
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

BOOL lcdbm_lumine_change(ushort brightness);

#ifdef __cplusplus
}
#endif // __cplusplus

//--------------------------------------------------
//		�e�X�g�R�[�h
//--------------------------------------------------
#ifdef	CRW_DEBUG
void test_lcdbm_function( void );
#else
#define	test_lcdbm_function()	;	// �����[�X�r���h����"No prototype function"���[�j���O���o�Ȃ��悤�ɂ���
#endif//CRW_DEBUG


#endif//_LCDBM_CTRL_H_
// MH810100(E) K.Onodera 2019/11/11 �Ԕԃ`�P�b�g���X(GT-8700(LZ-122601)���p)
