// MH810100(S) K.Onodera  2019/11/11 �Ԕԃ`�P�b�g���X(GT-8700(LZ-122601)���p)
//[]----------------------------------------------------------------------[]
///	@brief			�p�P�b�g�ʐM�֘A����
//[]----------------------------------------------------------------------[]
///	@author			m.onouchi
///	@date			Create	: 2009/09/25
///	@file			pktctrl.c
///	@version		LH068004
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
#include	<stdio.h>
#include	<string.h>
#include	"system.h"
#include	"common.h"
#include	"ope_def.h"
#include	"rtc_readwrite.h"
#include	"pktctrl.h"
#include	"pkt_buffer.h"

//--------------------------------------------------
//		��`
//--------------------------------------------------
/**
 *	�e�L�X�g�R�}���h�Ǘ�
 */
typedef struct {
	BOOL	is_begun;		///< �e�L�X�g�J�n�֐����Ăяo���ς݂��i���̃t���O��TRUE�łȂ��ꍇ�̓R���g���[���R�[�h�������s��Ȃ��j
	ushort	bytes;			///< ������f�[�^�����o�C�g�i�[������
} packet_text_work_t;

//--------------------------------------------------
//		�ϐ�
//--------------------------------------------------
static lcdbm_cmd_work_t		SendCmdDt;		///< �R�}���h�쐬�p���[�N
static lcdbm_cmd_text_t		text_command;	///< �e�L�X�g�\���R�}���h�쐬�p�̃��[�N
static packet_text_work_t	text_work;		///< �e�L�X�g�\���R�}���h�Ǘ��p

//--------------------------------------------------
//		�O���֐��Q��
//--------------------------------------------------
extern	void	ClkrecUpdateFromRTC( struct	RTC_rec *Time );
extern	uchar	PktResetReqFlgRead( void );

//--------------------------------------------------
//		�X�e�[�^�X�v���R�}���h
//--------------------------------------------------
/**
 *	��{�ݒ�v��/���Z�b�g�v��
 *
 *	@retval	FALSE	�p�P�b�g���M�\�� ���s�i�p�P�b�g���M�o�b�t�@�I�[�o�[�t���[�j
 *	@retval	TRUE	�p�P�b�g���M�\�� ����I��
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/02(��)
 */
BOOL PKTcmd_request_config( void )
{
	lcdbm_cmd_config_t	*dst = &SendCmdDt.config;

	if( PktResetReqFlgRead() ){	// ���Z�b�g�v������
		return ( PKTcmd_notice_ope( LCDBM_OPCD_RST_REQ, 0 ) );
	}else{
		// ������
		memset( &SendCmdDt, 0, sizeof(lcdbm_cmd_config_t) );

		// ���ʕ�
		dst->command.length = sizeof(lcdbm_cmd_config_t) - sizeof(SendCmdDt.command.length);
		dst->command.id = LCDBM_CMD_ID_STATUS;
		dst->command.subid = LCDBM_CMD_SUBID_CONFIG;

		// �f�[�^
		dst->rsw = read_rotsw();	// ���݂�RSW��Ԃ��Z�b�g����

		// �v���O�����o�[�W����
		memcpy( dst->ver, VERSNO.ver_part, 8 );	// RXM�v���O�����o�[�W�����i�[(���l��)
// MH810100(S) 2020/06/08 #4205�y�A���]���w�E�����z�Z���^�[�Ƃ̒ʐM�f����QR�Ǎ������Z��������Ɖ�ʂ��ł܂�(No.02-0009)
		dst->opemod = OPECTL.Ope_mod;
// MH810100(E) 2020/06/08 #4205�y�A���]���w�E�����z�Z���^�[�Ƃ̒ʐM�f����QR�Ǎ������Z��������Ɖ�ʂ��ł܂�(No.02-0009)

		return ( PKTbuf_SetSendCommand( (uchar *)&SendCmdDt, sizeof(lcdbm_cmd_config_t) ) );
	}
}

//--------------------------------------------------
//		�@��ݒ�R�}���h
//--------------------------------------------------
/**
 *	���v�ݒ�
 *
 *	@retval	FALSE	�p�P�b�g���M�\�� ���s�i�p�P�b�g���M�o�b�t�@�I�[�o�[�t���[�j
 *	@retval	TRUE	�p�P�b�g���M�\�� ����I��
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/02(��)
 */
BOOL PKTcmd_clock( uchar type )			// type = 0 (�蓮�ł̎��v�ύX),type = 1(SNTP����̎��v�ύX) 
{
	lcdbm_cmd_clock_t	*dst = &SendCmdDt.clock;

	// �ŐV��RTC�������擾����CLK_REC�X�V
	RTC_ClkDataReadReq( RTC_READ_NON_QUE );	// RTC�����ǂݏo��(OPE�ւ̍X�V�v���Ȃ�)
	ClkrecUpdateFromRTC( &RTC_CLOCK );		// CLK_REC�X�V

	// ������
	memset( &SendCmdDt, 0, sizeof(lcdbm_cmd_clock_t) );
	dst->command.length = sizeof(lcdbm_cmd_clock_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_SETUP;
	dst->command.subid = LCDBM_CMD_SUBID_CLOCK;

	// �f�[�^
	if( type == 1 ){			// SNTP����̎��v�ύX
		dst->year	= 0;	// �N
		dst->month	= 0;	// ��
		dst->day	= 0;	// ��
		dst->hour	= 0;	// ��
		dst->minute	= 0;	// ��
		dst->second	= 0;	// �b
	}else{						// �蓮�ł̎��v�ύX
	// �f�[�^
		dst->year	= CLK_REC.year;	// �N
		dst->month	= CLK_REC.mont;	// ��
		dst->day	= CLK_REC.date;	// ��
		dst->hour	= CLK_REC.hour;	// ��
		dst->minute	= CLK_REC.minu;	// ��
		dst->second	= CLK_REC.seco;	// �b
	}

	return ( PKTbuf_SetSendCommand( (uchar *)&SendCmdDt, sizeof(lcdbm_cmd_clock_t) ) );
}

/**
 *	LCD�P�x
 *
 *	@param[in]	brightness	�P�x�l�i0�`15�j
 *	@retval		FALSE		�p�P�b�g���M�\�� ���s�i�p�P�b�g���M�o�b�t�@�I�[�o�[�t���[�j
 *	@retval		TRUE		�p�P�b�g���M�\�� ����I��
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/02(��)
 */
BOOL PKTcmd_brightness( ushort brightness )
{
	BOOL ret = TRUE;
	lcdbm_cmd_brightness_t	*dst = &SendCmdDt.brightness;

	// ������
	memset( &SendCmdDt, 0, sizeof(lcdbm_cmd_brightness_t) );
	dst->command.length = sizeof(lcdbm_cmd_brightness_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_SETUP;
	dst->command.subid = LCDBM_CMD_SUBID_BRIGHTNESS;

	// �f�[�^
	if ( LCDBM_LCD_BRIGHTNESS_MAX < brightness ) {
		brightness = LCDBM_LCD_BRIGHTNESS_MAX;
	}
	dst->brightness = brightness;

	ret = PKTbuf_SetSendCommand( (uchar *)&SendCmdDt, sizeof(lcdbm_cmd_brightness_t) );

	return ret;
	
}

//--------------------------------------------------
//		�e�L�X�g�n�R�}���h
//	�i�P�R�}���h���ɕ����R���g���[���R�[�h���w�肷��j
//--------------------------------------------------
#define	LCDBM_CMD_TEXT_FONT_SIZE		3	///< �u�����t�H���g�w��v�̃f�[�^�T�C�Y
#define	LCDBM_CMD_TEXT_COLOR_SIZE		4	///< �u�����F�w��v�̃f�[�^�T�C�Y
#define	LCDBM_CMD_TEXT_BLINK_SIZE		6	///< �u������u�����N�w��v�̃f�[�^�T�C�Y
#define	LCDBM_CMD_TEXT_CLEAR_SIZE		5	///< �u�N���A�w��v�̃f�[�^�T�C�Y
#define	LCDBM_CMD_TEXT_MENU_SIZE		4	///< �u���j���[�\���w��v�̍ŏ��f�[�^�T�C�Y�i�f�[�^���ɂ��ϒ��j
#define	LCDBM_CMD_TEXT_DIRECT_SIZE		4	///< �u������w��v�̍ŏ��f�[�^�T�C�Y�i�f�[�^���ɂ��ϒ��j
#define	LCDBM_CMD_TEXT_BACKGROUND_SIZE	4	///< �u�w�i�F�w��v�̃f�[�^�T�C�Y
/**
 *	�e�L�X�g�R�}���h�쐬�J�n
 *
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/10/29(��)
 *	@attention
 *		PKTcmd_text_begin() �` PKTcmd_text_end() �̊Ԃ�
 *		�P�R�}���h���ɃR���g���[���R�[�h����̂ݎw�肷��e�L�X�g�n�R�}���h�֐����Ă΂ꂽ�肷���
 *		�R�}���h������ɐ����ł��Ȃ��Ȃ邽�߁APKTcmd_text_begin() �` PKTcmd_text_end()�͌��ʂ��̂悢�ӏ��Ŏg�����ƁB
 */
void PKTcmd_text_begin( void )
{
	// �e�L�X�g�R�}���h�쐬�J�n�֐����Ăяo���ς݂Ƃ���
	text_work.is_begun = TRUE;

	// ������f�[�^���i�[���Ă��Ȃ���ԂƂ���
	text_work.bytes = 0;

}


/**
 *	�e�L�X�g�R�}���h�쐬�I�������M�\��
 *
 *	@retval	FALSE	�p�P�b�g���M�\�� ���s�i�p�P�b�g���M�o�b�t�@�I�[�o�[�t���[�j
 *	@retval	TRUE	�p�P�b�g���M�\�� ����I��
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/02(��)
 *	@attention
 *		PKTcmd_text_begin() �` PKTcmd_text_end() �̊Ԃ�
 *		�P�R�}���h���ɃR���g���[���R�[�h����̂ݎw�肷��e�L�X�g�n�R�}���h�֐����Ă΂ꂽ�肷���
 *		�R�}���h������ɐ����ł��Ȃ��Ȃ邽�߁APKTcmd_text_begin() �` PKTcmd_text_end()�͌��ʂ��̂悢�ӏ��Ŏg�����ƁB
 */
BOOL PKTcmd_text_end( void )
{
	lcdbm_cmd_text_t	*dst = &text_command;
	ushort	data_length;

	// �e�L�X�g�R�}���h�쐬�J�n�ς݂łȂ��Ȃ烊�^�[������
	if ( text_work.is_begun == FALSE ) {
		return FALSE;
	}

	// ������
	dst->command.length = (unsigned short)( sizeof(text_command.command.id) + sizeof(text_command.command.subid) + text_work.bytes );
	dst->command.id = LCDBM_CMD_ID_TEXT;
	dst->command.subid = LCDBM_CMD_SUBID_TEXT;

	// �e�L�X�g�R�}���h�쐬�J�n�֐����Ăяo���O�Ƃ���
	text_work.is_begun = FALSE;

	data_length = (ushort)( dst->command.length + sizeof(text_command.command.length) );
	return ( PKTbuf_SetSendCommand( (uchar *)&text_command, data_length ) );
}


/**
 *	�����t�H���g�w��
 *
 *	@param[in]	font_type	�t�H���g��ʁiLCDBM_TEXT_FONT_16�`�j
 *	@retval		FALSE		�e�L�X�g�R�}���h���`�F�b�N�łm�f�A�܂���	<br>
 *							�t�H���g��ʃ`�F�b�N�łm�f
 *	@retval		TRUE		����I��
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/02(��)
 *	@note
 *	-	�R���g���[���R�[�h�F ESC+'%' �i1BH�C25H�j
 *	-	����w��A�t�H���g��ʂ̐������`�F�b�N�͊֐����ł͍s���Ă��Ȃ�
 *	-	�{�֐��Ăяo���O��� PKTcmd_text_begin(), PKTcmd_text_end() ���Ăяo���K�v����B
 */
BOOL PKTcmd_text_font( const uchar font_type )
{
	unsigned char	*p;

	// �e�L�X�g�R�}���h�쐬�J�n�ς݂łȂ��Ȃ烊�^�[������
	if ( text_work.is_begun == FALSE ) {
		return FALSE;
	}

	// �c��o�b�t�@�e�ʃ`�F�b�N
	if ( sizeof(text_command.buf) < (text_work.bytes + LCDBM_CMD_TEXT_FONT_SIZE) ) {
		return FALSE;
	}

	// �f�[�^
	p = &text_command.buf[text_work.bytes];
	*p++ = LCDBM_CMD_TEXT_CONTROL;
	*p++ = LCDBM_CMD_TEXT_FONT;

	// �t�H���g���
	*p = font_type;

	// �f�[�^�i�[�o�C�g���X�V
	text_work.bytes += LCDBM_CMD_TEXT_FONT_SIZE;

	return TRUE;
}

/**
 *	�����F�w��
 *
 *	@param[in]	color	�P�U�r�b�g�J���[�f�[�^
 *	@retval		FALSE	�e�L�X�g�R�}���h���`�F�b�N�łm�f
 *	@retval		TRUE	����I��
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/02(��)
 *	@note
 *	-	�R���g���[���R�[�h�F ESC+''' �i1BH�C27H�j
 *	-	�{�֐��Ăяo���O��� PKTcmd_text_begin(), PKTcmd_text_end() ���Ăяo���K�v����B
 */
BOOL PKTcmd_text_color( const ushort color )
{
	unsigned char	*p;

	// �e�L�X�g�R�}���h�쐬�J�n�ς݂łȂ��Ȃ烊�^�[������
	if ( text_work.is_begun == FALSE ) {
		return FALSE;
	}

	// �c��o�b�t�@�e�ʃ`�F�b�N
	if ( sizeof(text_command.buf) < (text_work.bytes + LCDBM_CMD_TEXT_COLOR_SIZE) ) {
		return FALSE;
	}

	// �f�[�^
	p = &text_command.buf[text_work.bytes];
	*p++ = LCDBM_CMD_TEXT_CONTROL;
	*p++ = LCDBM_CMD_TEXT_COLOR;

	// RGB�w��
	*p++ = (uchar)( color >> 8 );
	*p   = (uchar)( color & 0xff);

	// �f�[�^�i�[�o�C�g���X�V
	text_work.bytes += LCDBM_CMD_TEXT_COLOR_SIZE;

	return TRUE;
}


/**
 *	�����u�����N�w��
 *
 *	@param[in]	line		�s(0�`7)
 *	@param[in]	column		��(�ݒ薳��)
 *	@param[in]	blink		�J�n�^����(OFF(0):����/ON(1):�J�n)
 *	@param[in]	interval	�_�ŊԊu�i0�`255(�~10msec)�j
 *	@retval		FALSE		�e�L�X�g�R�}���h���`�F�b�N�łm�f
 *	@retval		TRUE		����I��
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/02(��)
 *	@note
 *	-	�R���g���[���R�[�h�F ESC+'(' �i1BH�C28H�j
 *	-	�{�֐��Ăяo���O��� PKTcmd_text_begin(), PKTcmd_text_end() ���Ăяo���K�v����B
 */
BOOL PKTcmd_text_blink( const uchar line, const uchar column, const uchar blink, const uchar interval )
{
	unsigned char	*p;

	// �e�L�X�g�R�}���h�쐬�J�n�ς݂łȂ��Ȃ烊�^�[������
	if ( text_work.is_begun == FALSE ) {
		return FALSE;
	}

	// �c��o�b�t�@�e�ʃ`�F�b�N
	if ( sizeof(text_command.buf) < (text_work.bytes + LCDBM_CMD_TEXT_BLINK_SIZE) ) {
		return FALSE;
	}

	// �f�[�^
	p = &text_command.buf[text_work.bytes];
	*p++ = LCDBM_CMD_TEXT_CONTROL;
	*p++ = LCDBM_CMD_TEXT_BLINK;

	// �s
	*p++ = line;

	// ��
	*p++ = column;

	// �J�n�^����
	*p++ = blink;

	// �_�ŊԊu
	*p   = interval;

	// �f�[�^�i�[�o�C�g���X�V
	text_work.bytes += LCDBM_CMD_TEXT_BLINK_SIZE;

	return TRUE;
}

/**
 *	�N���A�w��
 *
 *	@param[in]	line	�s(0�`7)
 *	@param[in]	column	��(0�Œ�)
 *	@param[in]	range	�N���A�͈�(0:1�s�N���A(LCDBM_CLEAR_1_LINE)/1:�S�N���A(LCDBM_CLEAR_ALL_LINE))
 *	@retval		FALSE	�e�L�X�g�R�}���h���`�F�b�N�łm�f
 *	@retval		TRUE	����I��
 *	@author	ASE
 *	@date	2017/04/13
 *	@note
 *	-	�󔒂�\�����鎖�ɂ��e�L�X�g����������
 */
BOOL PKTcmd_text_clear( const uchar line, const uchar column, const uchar range )
{
	unsigned char	*p;

	// �e�L�X�g�R�}���h�쐬�J�n�ς݂łȂ��Ȃ烊�^�[������
	if ( text_work.is_begun == FALSE ) {
		return FALSE;
	}

	// �c��o�b�t�@�e�ʃ`�F�b�N
	if ( sizeof(text_command.buf) < (text_work.bytes + LCDBM_CMD_TEXT_CLEAR_SIZE) ) {
		return FALSE;
	}

	// �f�[�^
	p = &text_command.buf[text_work.bytes];
	*p++ = LCDBM_CMD_TEXT_CONTROL;
	*p++ = LCDBM_CMD_TEXT_CLEAR;

	// �s
	*p++ = line;

	// ��
	*p++ = column;

	// �N���A�͈�
	*p   = range;

	// �f�[�^�i�[�o�C�g���X�V
	text_work.bytes += LCDBM_CMD_TEXT_CLEAR_SIZE;

	return TRUE;
}

/**
 *	���j���[�\���w��
 *
 *	@param[in]	line	�s(0�`7)
 *	@param[in]	column	��(0�`29���)
 *	@param[in]	src		������f�[�^(SJIS�R�[�h)�i�[�̈�ւ̃|�C���^
 *	@retval		FALSE	�e�L�X�g�R�}���h���`�F�b�N�łm�f�A�܂���	<br>
 *						������f�[�^�����O
 *	@retval		TRUE	����I��
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/02(��)
 *	@note
 *	-	�R���g���[���R�[�h�F ESC+',' �i1BH�C2CH�j
 *	-	������f�[�^��'\0'���܂܂��ꍇ�͂��̒��O�܂ł�L���ȕ�����f�[�^�Ƃ���B
 *	-	�{�֐��Ăяo���O��� PKTcmd_text_begin(), PKTcmd_text_end() ���Ăяo���K�v����B
 */
BOOL PKTcmd_text_menu( const uchar line, const uchar column, const uchar *src )
{
	unsigned char	*p;
	unsigned short	length = 0;		// '\0'���O�܂ł̃o�C�g��

	// �e�L�X�g�R�}���h�쐬�J�n�ς݂łȂ��Ȃ烊�^�[������
	if ( text_work.is_begun == FALSE ) {
		return FALSE;
	}

	// �����񂩂�'\0'��T���A'\0'�̑O�܂ł�L���Ƃ���
	for ( length = 0; length < LCDBM_CMD_TEXT_CLMN_MAX; length++ ) {
		if ( src[length] == '\0' ) {
			break;
		}
	}

	// �c��o�b�t�@�e�ʃ`�F�b�N
	if ( sizeof(text_command.buf) < (text_work.bytes + LCDBM_CMD_TEXT_MENU_SIZE + length) ) {
		return FALSE;
	}

	// �f�[�^
	p = &text_command.buf[text_work.bytes];
	*p++ = LCDBM_CMD_TEXT_CONTROL;
	*p++ = LCDBM_CMD_TEXT_MENU;

	// �s
	*p++ = line;

	// ��
	*p++ = column;

	// ������f�[�^�̃R�s�[
	memcpy( p, src, (size_t)length );

	// �f�[�^�i�[�o�C�g���X�V
	text_work.bytes += LCDBM_CMD_TEXT_DIRECT_SIZE + length;

	return TRUE;
}

/**
 *	������w��
 *
 *	@param[in]	line	�s(0�`7)
 *	@param[in]	column	��(0�`29���)
 *	@param[in]	src		������f�[�^(SJIS�R�[�h)�i�[�̈�ւ̃|�C���^
 *	@retval		FALSE	�e�L�X�g�R�}���h���`�F�b�N�łm�f�A�܂���	<br>
 *						������f�[�^�����O
 *	@retval		TRUE	����I��
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/02(��)
 *	@note
 *	-	�R���g���[���R�[�h�F ESC+'-' �i1BH�C2DH�j
 *	-	������f�[�^��'\0'���܂܂��ꍇ�͂��̒��O�܂ł�L���ȕ�����f�[�^�Ƃ���B
 *	-	�{�֐��Ăяo���O��� PKTcmd_text_begin(), PKTcmd_text_end() ���Ăяo���K�v����B
 */
BOOL PKTcmd_text_direct( const uchar line, const uchar column, const uchar *src )
{
	unsigned char	*p;
	unsigned short	length = 0;		// '\0'���O�܂ł̃o�C�g��

	// �e�L�X�g�R�}���h�쐬�J�n�ς݂łȂ��Ȃ烊�^�[������
	if ( text_work.is_begun == FALSE ) {
		return FALSE;
	}

	// �����񂩂�'\0'��T���A'\0'�̑O�܂ł�L���Ƃ���(������ő��30)
	for ( length = 0; length < 30; length++ ) {
		if ( src[length] == '\0' ) {
			break;
		}
	}

	// �c��o�b�t�@�e�ʃ`�F�b�N
	if ( sizeof(text_command.buf) < (text_work.bytes + LCDBM_CMD_TEXT_DIRECT_SIZE + length) ) {
		return FALSE;
	}

	// �f�[�^
	p = &text_command.buf[text_work.bytes];
	*p++ = LCDBM_CMD_TEXT_CONTROL;
	*p++ = LCDBM_CMD_TEXT_DIRECT;

	// �s
	*p++ = line;

	// ��
	*p++ = column;

	// ������f�[�^�̃R�s�[
	memcpy( p, src, (size_t)length );

	// �f�[�^�i�[�o�C�g���X�V
	text_work.bytes += LCDBM_CMD_TEXT_DIRECT_SIZE + length;

	return TRUE;
}

/**
 *	�w�i�F�w��
 *
 *	@param[in]	color	RGB(0000h�`FFFFh (R=5bit�CG=6bit�CB=5bit))
 *	@retval		FALSE	�e�L�X�g�R�}���h���`�F�b�N�łm�f
 *	@retval		TRUE	����I��
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/02(��)
 *	@note
 *	-	�R���g���[���R�[�h�F ESC+''' �i1BH�C27H�j
 *	-	�{�֐��Ăяo���O��� PKTcmd_text_begin(), PKTcmd_text_end() ���Ăяo���K�v����B
 */
BOOL PKTcmd_text_backgroundcolor( const ushort color )
{
	unsigned char	*p;

	// �e�L�X�g�R�}���h�쐬�J�n�ς݂łȂ��Ȃ烊�^�[������
	if ( text_work.is_begun == FALSE ) {
		return FALSE;
	}

	// �c��o�b�t�@�e�ʃ`�F�b�N
	if ( sizeof(text_command.buf) < (text_work.bytes + LCDBM_CMD_TEXT_BACKGROUND_SIZE) ) {
		return FALSE;
	}

	// �f�[�^
	p = &text_command.buf[text_work.bytes];
	*p++ = LCDBM_CMD_TEXT_CONTROL;
	*p++ = LCDBM_CMD_TEXT_BACKGROUND;

	// RGB�w��
	*p++ = (uchar)( color >> 8 );
	*p   = (uchar)( color & 0xff);

	// �f�[�^�i�[�o�C�g���X�V
	text_work.bytes += LCDBM_CMD_TEXT_BACKGROUND_SIZE;

	return TRUE;
}

//--------------------------------------------------
//		�e�L�X�g�n�R�}���h
//	�i�P�R�}���h���ɃR���g���[���R�[�h����̂ݎw�肷��j
//--------------------------------------------------
/**
 *	�����t�H���g�w��
 *
 *	@param[in]	font_type	�t�H���g��ʁiLCDBM_TEXT_FONT_16�`�j
 *	@retval		FALSE		�e�L�X�g�R�}���h���`�F�b�N�łm�f�A�܂���	<br>
 *							�t�H���g��ʃ`�F�b�N�łm�f
 *	@retval		TRUE		����I��
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/09(��)
 *	@see	PKTcmd_text_font()
 *	@note
 *	-	�R���g���[���R�[�h�F ESC+'%' �i1BH�C25H�j
 *	-	����w��A�t�H���g��ʂ̐������`�F�b�N�͊֐����ł͍s���Ă��Ȃ�
 */
BOOL PKTcmd_text_1_font( const uchar font_type )
{
	PKTcmd_text_begin();
	if ( PKTcmd_text_font( font_type ) == FALSE ) {
		return FALSE;
	}
	return PKTcmd_text_end();
}

/**
 *	�����F�w��
 *
 *	@param[in]	color	�P�U�r�b�g�J���[�f�[�^
 *	@retval		FALSE	�e�L�X�g�R�}���h���`�F�b�N�łm�f
 *	@retval		TRUE	����I��
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/09(��)
 *	@see	PKTcmd_text_color()
 *	@note
 *	-	�R���g���[���R�[�h�F ESC+''' �i1BH�C27H�j
 */
BOOL PKTcmd_text_1_color( const ushort color )
{
	PKTcmd_text_begin();
	if ( PKTcmd_text_color( color ) == FALSE ) {
		return FALSE;
	}
	return PKTcmd_text_end();
}

/**
 *	�����u�����N�w��
 *
 *	@param[in]	line		�s(0�`7)
 *	@param[in]	column		��(�ݒ薳��)
 *	@param[in]	blink		�J�n�^����(OFF(0):����/ON(1):�J�n)
 *	@param[in]	interval	�_�ŊԊu�i0�`255(�~10msec)�j
 *	@retval		FALSE		�e�L�X�g�R�}���h���`�F�b�N�łm�f
 *	@retval		TRUE		����I��
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/09(��)
 *	@see	PKTcmd_text_blink()
 *	@note
 *	-	�R���g���[���R�[�h�F ESC+'(' �i1BH�C28H�j
 */
BOOL PKTcmd_text_1_blink( const uchar line, const uchar column, const uchar blink, const uchar interval )
{
	PKTcmd_text_begin();
	if ( PKTcmd_text_blink( line, column, blink, interval ) == FALSE ) {
		return FALSE;
	}
	return PKTcmd_text_end();
}

/**
 *	�N���A�w��
 *
 *	@param[in]	line	�s(0�`7)
 *	@param[in]	column	��(0�Œ�)
 *	@param[in]	range	�N���A�͈�(0:1�s�N���A(LCDBM_CLEAR_1_LINE)/1:�S�N���A(LCDBM_CLEAR_ALL_LINE))
 *	@retval		FALSE	�e�L�X�g�R�}���h���`�F�b�N�łm�f
 *	@retval		TRUE	����I��
 *	@author	ASE
 *	@date	2017/04/13
 *	@note
 *	-	�󔒂�\�����鎖�ɂ��e�L�X�g����������
 */
BOOL PKTcmd_text_1_clear( const uchar line, const uchar column, const uchar range )
{
	PKTcmd_text_begin();
	if ( PKTcmd_text_clear( line, column, range ) == FALSE ) {
		return FALSE;
	}
	return PKTcmd_text_end();
}

/**
 *	���j���[�\���w��
 *
 *	@param[in]	line	�s(0�`7)
 *	@param[in]	column	��(0�`29���)
 *	@param[in]	src		������f�[�^(SJIS�R�[�h)�i�[�̈�ւ̃|�C���^
 *	@retval		FALSE	�e�L�X�g�R�}���h���`�F�b�N�łm�f
 *	@retval		TRUE	����I��
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/02(��)
 *	@note
 *	-	�R���g���[���R�[�h�F ESC+',' �i1BH�C2CH�j
 *	-	������f�[�^��'\0'���܂܂��ꍇ�͂��̒��O�܂ł�L���ȕ�����f�[�^�Ƃ���B
 */
BOOL PKTcmd_text_1_menu( const uchar line, const uchar column, const uchar *src )
{
	PKTcmd_text_begin();
	if ( PKTcmd_text_menu( line, column, src ) == FALSE ) {
		return FALSE;
	}
	return PKTcmd_text_end();
}

/**
 *	������w��
 *
 *	@param[in]	line	�s(0�`7)
 *	@param[in]	column	��(0�`29���)
 *	@param[in]	src		������f�[�^�i�[�̈�ւ̃|�C���^
 *	@retval		FALSE	�e�L�X�g�R�}���h���`�F�b�N�łm�f�A�܂���	<br>
 *						������f�[�^�����O
 *	@retval		TRUE	����I��
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/11/09(��)
 *	@see	PKTcmd_text_direct()
 *	@note
 *	-	�R���g���[���R�[�h�F ESC+'-' �i1BH�C2DH�j
 *	-	������f�[�^��'\0'���܂܂��ꍇ�͂��̒��O�܂ł�L���ȕ�����f�[�^�Ƃ���B
 *	-	������f�[�^�͔��p�ƑS�p������Ɗ�o�C�g�ɂȂ�\������B
 */
BOOL PKTcmd_text_1_direct( const uchar line, const uchar column, const uchar *src )
{
	PKTcmd_text_begin();
	if ( PKTcmd_text_direct(line, column, src ) == FALSE ) {
		return FALSE;
	}
	return PKTcmd_text_end();
}
/**
 *	�J���[LCD�ւ̔w�i�F�\��
 *
 *	@param[in]	color		�P�U�r�b�g�J���[�f�[�^
 *	@date	2019/12/25
 */
// MH810100(S) S.Takahashi 2019/12/25 �Ԕԃ`�P�b�g���X(�����e�i���X)
BOOL PKTcmd_text_1_backgroundcolor( ushort color )
{
	PKTcmd_text_begin();
	if ( PKTcmd_text_backgroundcolor( color ) == FALSE ) {
		return FALSE;
	}
	return PKTcmd_text_end();
}
// MH810100(E) S.Takahashi 2019/12/25 �Ԕԃ`�P�b�g���X(�����e�i���X)
/**
 *	�J���[LCD�ւ̃e�L�X�g�\��
 *
 *	@param[in]	color		�P�U�r�b�g�J���[�f�[�^
 *	@param[in]	blink		�J�n�^����(OFF(0):����/ON(1):�J�n)
 *	@param[in]	interval	�_�ŊԊu�i0�`255(�~10msec)�j
 *	@param[in]	line		�s(0�`7)
 *	@param[in]	column		��(0�`29���)
 *	@param[in]	src			������f�[�^(SJIS�R�[�h)�i�[�̈�ւ̃|�C���^
 *	@retval		TRUE		����I��
 *	@retval		FALSE		�ُ�I��
 *	@author	m.onouchi
 *	@date	2009/11/24(��)
 *	@attention	������w��̏ꍇ�͏I�[����(NULL)��t�����邱�ƁB
 *	@note
 *	-	������𒼐ڎw�肷����@�ƃe�L�X�g�t�@�C�����w�肷����@���I�ׂ܂��B
 *	-	�e�L�X�g�t�@�C���͂Q�o�C�g�z��ŕ����w�肪�\�ł��B
 */
BOOL PKTcmd_text_grachr( const ushort color, const uchar blink, const uchar interval, const uchar line, const uchar column, const uchar *src )
{

	PKTcmd_text_begin();

	// �J���[�w��
	if ( LCDBM_COLOR_DEFAULT != color ) {
		if ( PKTcmd_text_color( (ushort)(color&0x0000FFFF) ) == FALSE ) {
			goto _cmderr;				// length error
		}
	}

	// �u�����N�w��
	if ( PKTcmd_text_blink( line, column, blink, interval ) == FALSE ) {
		goto _cmderr;				// length error
	}

	// �e�L�X�g�w��
	if ( PKTcmd_text_direct( line, column, src ) == FALSE ) {
		goto _cmderr;				// length error
	}

_cmderr:
	return PKTcmd_text_end();
}

/**
 *	�J���[LCD�ւ̃e�L�X�g�\��
 *
 *	@param[in]	color		�P�U�r�b�g�J���[�f�[�^
 *	@param[in]	blink		�J�n�^����(OFF(0):����/ON(1):�J�n)
 *	@param[in]	interval	�_�ŊԊu�i0�`255(�~10msec)�j
 *	@param[in]	line		�s(0�`7)
 *	@param[in]	column		��(0�`29���)
 *	@param[in]	src			������f�[�^(SJIS�R�[�h)�i�[�̈�ւ̃|�C���^
 *	@param[in]	step		�����Step
 *	@param[in]	totalstep	�SStep��
 *	@retval		TRUE		����I��
 *	@retval		FALSE		�ُ�I��
 *	@author	m.onouchi
 *	@date	2009/11/24(��)
 *	@attention	������w��̏ꍇ�͏I�[����(NULL)��t�����邱�ƁB
 *	@note
 *	-	������𒼐ڎw�肷����@�ƃe�L�X�g�t�@�C�����w�肷����@���I�ׂ܂��B
 *	-	�e�L�X�g�t�@�C���͂Q�o�C�g�z��ŕ����w�肪�\�ł��B
 */
BOOL PKTcmd_text_1_grachr( const ushort color, const uchar blink, const uchar interval, const uchar line, const uchar column, const uchar *src, ushort step, ushort totalstep )
{

	if(step == 1) {
		PKTcmd_text_begin();

		// �J���[�w��
		if ( LCDBM_COLOR_DEFAULT != color ) {
			if ( PKTcmd_text_color((ushort)(color&0x0000FFFF) ) == FALSE) {
				goto _cmderr;				// length error
			}
		}

		// �u�����N�w��
		if ( PKTcmd_text_blink( line, column, blink, interval ) == FALSE ) {
			goto _cmderr;				// length error
		}

	}

	// �e�L�X�g�w��
	if ( PKTcmd_text_direct( line, column, src ) == FALSE ) {
		goto _cmderr;				// length error
	}

_cmderr:
	if(step >= totalstep) {
		// �SStep�I���ō쐬�I�������M�\��
		return PKTcmd_text_end();
	}
	else {
		return TRUE;
	}
}



//--------------------------------------------------
//		�A�i�E���X�v���R�}���h
//--------------------------------------------------
/**
 *	���ʐݒ�
 *
 *	@param[in]	volume	����(0�`100(0=����))
 *	@retval		FALSE	�p�P�b�g���M�\�� ���s�i�p�P�b�g���M�o�b�t�@�I�[�o�[�t���[�j
 *	@retval		TRUE	�p�P�b�g���M�\�� ����I��
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/10/21(��)
 *	@note
 */
BOOL PKTcmd_audio_volume( uchar volume )
{
	lcdbm_cmd_audio_volume_t	*dst = &SendCmdDt.au_volume;

	// ������
	memset( &SendCmdDt, 0, sizeof(lcdbm_cmd_audio_volume_t) );
	dst->command.length = sizeof(lcdbm_cmd_audio_volume_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_AUDIO;
	dst->command.subid = LCDBM_CMD_SUBID_AUDIO_VOLUME;

	// ����
	dst->volume = volume;

	return ( PKTbuf_SetSendCommand( (uchar *)&SendCmdDt, sizeof(lcdbm_cmd_audio_volume_t) ) );
}


/**
 *	�J�n�v��
 *
 *	@param[in]	src		LCD�֑��M����R�}���h���i�[���ꂽ�̈�ւ̃|�C���^
 *	@retval		FALSE	�p�P�b�g���M�\�� ���s�i�p�P�b�g���M�o�b�t�@�I�[�o�[�t���[�j
 *	@retval		TRUE	�p�P�b�g���M�\�� ����I��
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/10/15(��)
 *	@note
 *	-	���M�R�}���h�͉ϒ��Ȃ̂Œ��ӁB
 *	-	�R�}���h�̊e���ڂ̃`�F�b�N�͍s��Ȃ��B
 */
BOOL PKTcmd_audio_start( uchar startType, uchar reqKind, ushort no )
{
	lcdbm_cmd_audio_start_t	*dst = &SendCmdDt.au_start;

	// ������
	memset( &SendCmdDt, 0, sizeof(lcdbm_cmd_audio_start_t) );

	dst->command.length = (unsigned short)( sizeof(lcdbm_cmd_audio_start_t) - sizeof(SendCmdDt.command.length) );
	dst->command.id = LCDBM_RSP_ID_MNT_DATA_LCD;
	dst->command.subid = LCDBM_CMD_SUBID_AUDIO_START;

	// �f�[�^
	dst->times = 1;				// ������(1�Œ�)
	dst->condition = startType;	// �����J�n����
	dst->interval = 0;				// ���b�Z�[�W�Ԋu(������1�Œ�̂��ߖ��g�p)
	dst->channel = 0;			// �����`���l��(0�Œ�)
	dst->language = 0;			// ����w��(�Ƃ肠����0)
	dst->AnaKind = reqKind;		// ���(0=���b�Z�[�W��/1=���߇�)
	dst->AnaNo = no;			// ���b�Z�[�W��/���߇�

	return ( PKTbuf_SetSendCommand( (uchar *)&SendCmdDt, sizeof(lcdbm_cmd_audio_start_t) ) );
}


/**
 *	�I���v��
 *
 *	@param[in]	channel			�����I���`���l��(0�Œ�)
 *	@param[in]	intrpt_method	���f���@(0�Œ�)
 *	@retval		FALSE			�p�P�b�g���M�\�� ���s�i�p�P�b�g���M�o�b�t�@�I�[�o�[�t���[�j
 *	@retval		TRUE			�p�P�b�g���M�\�� ����I��
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/10/15(��)
 */
BOOL PKTcmd_audio_end( uchar channel, uchar intrpt_method )
{
	lcdbm_cmd_audio_end_t	*dst = &SendCmdDt.au_end;

	// ������
	memset( &SendCmdDt, 0, sizeof(lcdbm_cmd_audio_end_t) );
	dst->command.length = sizeof(lcdbm_cmd_audio_end_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_RSP_ID_MNT_DATA_LCD;
	dst->command.subid = LCDBM_CMD_SUBID_AUDIO_END;

	// �f�[�^
	dst->channel = channel;			// �����`���l��
	dst->abort = intrpt_method;		// ���f���@

	return ( PKTbuf_SetSendCommand( (uchar *)&SendCmdDt, sizeof(lcdbm_cmd_audio_end_t) ) );
}

//--------------------------------------------------
//		�u�U�[�v���R�}���h
//--------------------------------------------------
/**
 *	���ʐݒ�
 *
 *	@param[in]	volume	����(0�`3(0=�����C3=�ő剹��))
 *	@retval		FALSE	�p�P�b�g���M�\�� ���s�i�p�P�b�g���M�o�b�t�@�I�[�o�[�t���[�j
 *	@retval		TRUE	�p�P�b�g���M�\�� ����I��
 *	@note
 */
BOOL PKTcmd_beep_volume( uchar volume )
{
	lcdbm_cmd_beep_volume_t	*dst = (lcdbm_cmd_beep_volume_t*)&SendCmdDt.bp_volume;

	// ������
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_beep_volume_t));
	dst->command.length = sizeof(lcdbm_cmd_beep_volume_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_BEEP;
	dst->command.subid = LCDBM_CMD_SUBID_BEEP_VOLUME;

	// �f�[�^
	dst->volume = volume;	// ����

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_beep_volume_t)));
}

/**
 *	�u�U�[���v��
 *
 *	@param[in]	beep	���(0=�s�b��A1=�s�s�B��A2=�s�s�s�B��)
 *	@retval		FALSE	�p�P�b�g���M�\�� ���s�i�p�P�b�g���M�o�b�t�@�I�[�o�[�t���[�j
 *	@retval		TRUE	�p�P�b�g���M�\�� ����I��
 *	@author	ASE
 *	@date	2017/04/13
 *	@note
 */
BOOL PKTcmd_beep_start( uchar beep )
{
	lcdbm_cmd_beep_start_t	*dst = (lcdbm_cmd_beep_start_t*)&SendCmdDt.bp_start;

	// ������
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_beep_start_t));
	dst->command.length = sizeof(lcdbm_cmd_beep_start_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_BEEP;
	dst->command.subid = LCDBM_CMD_SUBID_BEEP_START;

	// �f�[�^
	dst->beep = beep;	// ���

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_beep_start_t)));
}

/**
 *	�x����v��
 *
 *	@param[in]	kind	���(0=�Œ�)
 *	@param[in]	type	���(0=�J�n�A1=��~)
 *	@retval		FALSE	�p�P�b�g���M�\�� ���s�i�p�P�b�g���M�o�b�t�@�I�[�o�[�t���[�j
 *	@retval		TRUE	�p�P�b�g���M�\�� ����I��
 *	@date	2019/11/28
 *	@note
 */
BOOL PKTcmd_alarm_start( uchar kind, uchar type )
{
	lcdbm_cmd_alarm_start_t	*dst = (lcdbm_cmd_alarm_start_t*)&SendCmdDt.bp_start;

	// ������
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_alarm_start_t));
	dst->command.length = sizeof(lcdbm_cmd_alarm_start_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_BEEP;
// MH810100(S) S.Takahashi 2020/02/21 #3898 �h�ƌx��A���[������Ȃ�
//	dst->command.subid = LCDBM_CMD_SUBID_BEEP_START;
	dst->command.subid = LCDBM_CMD_SUBID_BEEP_ALARM;
// MH810100(E) S.Takahashi 2020/02/21 #3898 �h�ƌx��A���[������Ȃ�

	// �f�[�^
	dst->kind = kind;	// ���
	dst->type = type;	// ���

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_alarm_start_t)));
}

//--------------------------------------------------
//		���Z�@��Ԓʒm����R�}���h
//--------------------------------------------------
/**
 *	����ʒm
 *
 *	@param[in]	ope_code	����R�[�h
 *	@param[in]	status		���
 *	@retval		FALSE		�p�P�b�g���M�\�� ���s�i�p�P�b�g���M�o�b�t�@�I�[�o�[�t���[�j
 *	@retval		TRUE		�p�P�b�g���M�\�� ����I��
 *	@note
 */
BOOL PKTcmd_notice_ope( uchar ope_code, ushort status )
{
	lcdbm_cmd_notice_ope_t	*dst = (lcdbm_cmd_notice_ope_t*)&SendCmdDt.operate;

	// ������
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_notice_ope_t));
	dst->command.length = sizeof(lcdbm_cmd_notice_ope_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_STATUS_NOTICE;
	dst->command.subid = LCDBM_CMD_SUBID_NOTICE_OPE;

	// �f�[�^
	dst->ope_code = ope_code;	// ����R�[�h
	dst->status = status;		// ���

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_notice_ope_t)));
}

/**
 *	�c�x�ƒʒm
 *
 *	@param[in]	opn_cls	�c�x��(0=�c��,1=�x��)
 *	@param[in]	reason	�x�Ɨ��R
 *	@retval		FALSE	�p�P�b�g���M�\�� ���s�i�p�P�b�g���M�o�b�t�@�I�[�o�[�t���[�j
 *	@retval		TRUE	�p�P�b�g���M�\�� ����I��
 *	@note
 */
BOOL PKTcmd_notice_opn( uchar opn_cls, uchar reason )
{
	lcdbm_cmd_notice_opn_t	*dst = (lcdbm_cmd_notice_opn_t*)&SendCmdDt.opn_cls;

	// ������
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_notice_opn_t));
	dst->command.length = sizeof(lcdbm_cmd_notice_opn_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_STATUS_NOTICE;
	dst->command.subid = LCDBM_CMD_SUBID_NOTICE_OPN;

	// �f�[�^
	dst->opn_cls = opn_cls;	// �c�x��
	dst->reason = reason;	// �x�Ɨ��R

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_notice_opn_t)));
}

/**
 *	���Z��Ԓʒm
 *
 *	@param[in]	event	����(Bit�P��:0=����,1=����)
 *	@retval		FALSE	�p�P�b�g���M�\�� ���s�i�p�P�b�g���M�o�b�t�@�I�[�o�[�t���[�j
 *	@retval		TRUE	�p�P�b�g���M�\�� ����I��
 *	@note
 */
BOOL PKTcmd_notice_pay( ulong event )
{
	lcdbm_cmd_notice_pay_t	*dst = (lcdbm_cmd_notice_pay_t*)&SendCmdDt.pay_state;

	// ������
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_notice_pay_t));
	dst->command.length = sizeof(lcdbm_cmd_notice_pay_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_STATUS_NOTICE;
	dst->command.subid = LCDBM_CMD_SUBID_NOTICE_PAY;

	// �f�[�^
	dst->event = event;	// ����

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_notice_pay_t)));
}

/**
 *	�|�b�v�A�b�v�\���v��
 *
 *	@param[in]	text_code	�e�L�X�g�R�[�h(1:�a��ؔ��s,2:�c���s��,3:���z���s(�ă^�b�`�v��))
 *	@retval		FALSE		�p�P�b�g���M�\�� ���s�i�p�P�b�g���M�o�b�t�@�I�[�o�[�t���[�j
 *	@retval		TRUE		�p�P�b�g���M�\�� ����I��
 *	@note
 */
// MH810100(S) S.Fujii 2020/07/20 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
//BOOL PKTcmd_notice_dsp( uchar text_code, uchar status )
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// BOOL PKTcmd_notice_dsp( uchar text_code, uchar status, ulong add_info )
BOOL PKTcmd_notice_dsp( uchar text_code, uchar status, ulong add_info, uchar *str, ulong str_size )
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// MH810100(E) S.Fujii 2020/07/20 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
{
	lcdbm_cmd_notice_dsp_t	*dst = (lcdbm_cmd_notice_dsp_t*)&SendCmdDt.display;
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	ushort length = sizeof(lcdbm_cmd_notice_dsp_t);
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

	// ������
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_notice_dsp_t));
	dst->command.length = sizeof(lcdbm_cmd_notice_dsp_t) - sizeof(SendCmdDt.command.length);
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	if ( text_code != POP_QR_RECIPT ) {
		dst->command.length = sizeof(lcdbm_cmd_notice_dsp_t) - sizeof(SendCmdDt.command.length);
	} else {
		dst->command.length = sizeof(lcdbm_cmd_notice_dsp_t) - sizeof(SendCmdDt.command.length);
	}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	dst->command.id = LCDBM_CMD_ID_STATUS_NOTICE;
	dst->command.subid = LCDBM_CMD_SUBID_NOTICE_DSP;

	// �f�[�^
	dst->text_code = text_code;	// �R�[�h
	dst->status    = status;	// ���
// MH810100(S) S.Fujii 2020/07/20 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
	dst->add_info  = add_info;	// �ǉ����
// MH810100(E) S.Fujii 2020/07/20 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// 	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_notice_dsp_t)));
	if ( text_code == POP_QR_RECIPT ) {
		// �ǉ��������ݒ肷��
		if ( str_size > LCDBM_CMD_NOTICE_DSP_ADD_STR_SIZE ) {
			memcpy(dst->add_str, str, LCDBM_CMD_NOTICE_DSP_ADD_STR_SIZE);
		} else {
			memcpy(dst->add_str, str, str_size);
		}
	} else {
		// �ǉ�����������
		dst->command.length -= LCDBM_CMD_NOTICE_DSP_ADD_STR_SIZE;
		length -= LCDBM_CMD_NOTICE_DSP_ADD_STR_SIZE;
	}

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, length));
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
}

/**
 *	�x���ʒm
 *
 *	@param[in]	event	����(Bit�P��:0=����,1=����)
 *	@retval		FALSE	�p�P�b�g���M�\�� ���s�i�p�P�b�g���M�o�b�t�@�I�[�o�[�t���[�j
 *	@retval		TRUE	�p�P�b�g���M�\�� ����I��
 *	@note
 */
BOOL PKTcmd_notice_alm( ushort event )
{
	lcdbm_cmd_notice_alm_t	*dst = (lcdbm_cmd_notice_alm_t*)&SendCmdDt.alarm;

	// ������
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_notice_alm_t));
	dst->command.length = sizeof(lcdbm_cmd_notice_alm_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_STATUS_NOTICE;
	dst->command.subid = LCDBM_CMD_SUBID_NOTICE_ALM;

	// �f�[�^
	dst->event = event;	// ����

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_notice_alm_t)));
}

// GG129000(S) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
/**
 *	�|�b�v�A�b�v�폜�v��
 *
 *	@param[in]	NONE
 *	@retval		FALSE		�p�P�b�g���M�\�� ���s�i�p�P�b�g���M�o�b�t�@�I�[�o�[�t���[�j
 *	@retval		TRUE		�p�P�b�g���M�\�� ����I��
 *	@note
 */
BOOL PKTcmd_notice_del(uchar kind, uchar status)
{
	lcdbm_cmd_notice_del_t	*dst = (lcdbm_cmd_notice_del_t*)&SendCmdDt.display_del;

	// ������
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_notice_del_t));
	dst->command.length = sizeof(lcdbm_cmd_notice_del_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_STATUS_NOTICE;
	dst->command.subid = LCDBM_CMD_SUBID_NOTICE_DEL;

	// �f�[�^
	dst->kind = kind;		// ���
	dst->status = status;	// ���

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_notice_del_t)));
}
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j

//--------------------------------------------------
//		���Z�E�������ʒm�R�}���h
//--------------------------------------------------
/**
 *	���Z�c���ω��ʒm
 *
 *	@param[in]	src		LCD�֑��M�������ނ��i�[���ꂽ�̈�ւ��߲��
 *	@retval		FALSE	�p�P�b�g���M�\�� ���s�i�p�P�b�g���M�o�b�t�@�I�[�o�[�t���[�j
 *	@retval		TRUE	�p�P�b�g���M�\�� ����I��
 *	@note
 */
BOOL PKTcmd_pay_rem_chg( lcdbm_cmd_pay_rem_chg_t *src )
{
	lcdbm_cmd_pay_rem_chg_t	*dst = (lcdbm_cmd_pay_rem_chg_t*)&SendCmdDt.pay_rem_chg;

	// ������
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_pay_rem_chg_t));
	dst->command.length = sizeof(lcdbm_cmd_pay_rem_chg_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_PAY_INFO_NOTICE;
	dst->command.subid = LCDBM_CMD_SUBID_PAY_REM_CHG;

	// �f�[�^�R�s�[
	memcpy(&SendCmdDt.pay_rem_chg.id, &(src->id), sizeof(lcdbm_cmd_pay_rem_chg_t)-sizeof(lcdbm_cmd_base_t));

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_pay_rem_chg_t)));
}

/**
 *	QR�f�[�^����
 *
 *	@param[in]	id		���ɂ��琸�Z�����܂ł��Ǘ�����ID
 *	@param[in]	result	����(0:OK,1:NG(�r��),2:NG(�������))
 *	@retval		FALSE	�p�P�b�g���M�\�� ���s�i�p�P�b�g���M�o�b�t�@�I�[�o�[�t���[�j
 *	@retval		TRUE	�p�P�b�g���M�\�� ����I��
 *	@note
 */
BOOL PKTcmd_QR_data_res( ulong id, uchar result )
{
	lcdbm_cmd_QR_data_res_t	*dst = (lcdbm_cmd_QR_data_res_t*)&SendCmdDt.QR_data_res;

	// ������
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_QR_data_res_t));
	dst->command.length = sizeof(lcdbm_cmd_QR_data_res_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_PAY_INFO_NOTICE;
	dst->command.subid = LCDBM_CMD_SUBID_QR_DATA_RES;

	// �f�[�^
	dst->id = id;			// ID
	dst->result = result;	// ����

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_QR_data_res_t)));
}

// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
//[]----------------------------------------------------------------------[]
///	@brief			���Ɏ����w�艓�u���Z�J�n
//[]----------------------------------------------------------------------[]
///	@param[in]		ushort	: ���Ɏ����w�艓�u���Z�J�n
///	@return			ret		: �p�P�b�g���M�\�񌋉�<br>
///							  TRUE  = ����I��<br>
///							  FALSE = ���s(�p�P�b�g���M�o�b�t�@�I�[�o�[�t���[)<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
BOOL PKTcmd_remote_time_start( lcdbm_cmd_remote_time_start_t *src )
{
	lcdbm_cmd_remote_time_start_t *dst = (lcdbm_cmd_remote_time_start_t*)&SendCmdDt.remote_time_start;

	// ������
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_remote_time_start_t));
	dst->command.length = sizeof(lcdbm_cmd_remote_time_start_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_PAY_INFO_NOTICE;
	dst->command.subid = LCDBM_CMD_SUBID_REMOTE_TIME_START;

	// �f�[�^�R�s�[
	memcpy(&(dst->ulPno), &(src->ulPno),
		sizeof(lcdbm_cmd_remote_time_start_t) - sizeof(lcdbm_cmd_base_t));

	return (PKTbuf_SetSendCommand((uchar*)&SendCmdDt, sizeof(lcdbm_cmd_remote_time_start_t)));
}
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)

//--------------------------------------------------
//		�����e�i���X�f�[�^�R�}���h
//--------------------------------------------------
/**
 *	QR���[�_����v��
 *
 *	@param[in]	ctrl_cd	����R�[�h(0:�o�[�W�����v��,1:�ǎ�J�n,2:�ǎ��~)
 *	@retval		FALSE	�p�P�b�g���M�\�� ���s�i�p�P�b�g���M�o�b�t�@�I�[�o�[�t���[�j
 *	@retval		TRUE	�p�P�b�g���M�\�� ����I��
 *	@note
 */
BOOL PKTcmd_mnt_qr_ctrl_req( unsigned char ctrl_cd )
{
	lcdbm_cmd_QR_ctrl_req_t	*dst = (lcdbm_cmd_QR_ctrl_req_t*)&SendCmdDt.QR_ctrl_req;

	// ������
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_QR_ctrl_req_t));
	dst->command.length = sizeof(lcdbm_cmd_QR_ctrl_req_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_MNT_DATA;
	dst->command.subid = LCDBM_CMD_SUBID_MNT_QR_CTRL_REQ;

	// �f�[�^
	dst->ctrl_cd = ctrl_cd;	// ����R�[�h

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_QR_ctrl_req_t)));
}

/**
 *	���A���^�C���ʐM�a�ʗv��
 *
 *	@retval	FALSE	�p�P�b�g���M�\�� ���s�i�p�P�b�g���M�o�b�t�@�I�[�o�[�t���[�j
 *	@retval	TRUE	�p�P�b�g���M�\�� ����I��
 *	@note
 */
BOOL PKTcmd_mnt_rt_con_req( void )
{
	lcdbm_cmd_rt_con_req_t	*dst = (lcdbm_cmd_rt_con_req_t*)&SendCmdDt.rt_con_req;

	// ������
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_rt_con_req_t));
	dst->command.length = sizeof(lcdbm_cmd_rt_con_req_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_MNT_DATA;
	dst->command.subid = LCDBM_CMD_SUBID_MNT_RT_CON_REQ;

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_rt_con_req_t)));
}

/**
 *	DC-NET�ʐM�a�ʗv��
 *
 *	@retval	FALSE	�p�P�b�g���M�\�� ���s�i�p�P�b�g���M�o�b�t�@�I�[�o�[�t���[�j
 *	@retval	TRUE	�p�P�b�g���M�\�� ����I��
 *	@note
 */
BOOL PKTcmd_mnt_dc_con_req( void )
{
	lcdbm_cmd_dc_con_req_t	*dst = (lcdbm_cmd_dc_con_req_t*)&SendCmdDt.dc_con_req;

	// ������
	memset(&SendCmdDt, 0, sizeof(lcdbm_cmd_dc_con_req_t));
	dst->command.length = sizeof(lcdbm_cmd_dc_con_req_t) - sizeof(SendCmdDt.command.length);
	dst->command.id = LCDBM_CMD_ID_MNT_DATA;
	dst->command.subid = LCDBM_CMD_SUBID_MNT_DC_CON_REQ;

	return (PKTbuf_SetSendCommand((uchar *)&SendCmdDt, sizeof(lcdbm_cmd_dc_con_req_t)));
}

//@debug <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//--------------------------------------------------
//		�e�X�g�R�[�h
//--------------------------------------------------
#ifdef	CRW_DEBUG

// ������f�[�^
uchar	*test_text_message[] = {
	"�T�[�r�X��A���ɓ���ĉ�����",	// �S�p�̓r���ɔ��p�������Ă���p�^�[��
};

// �e�L�X�g�t�@�C��No.�f�[�^
ushort	test_text_fileNo[] = {
	0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987,
};

// �|�b�v�A�b�v�摜�̕\��������W
lcdbm_pos_t	test_popup_image_pos = { 55, 89 };

// �|�b�v�A�b�v�e�L�X�g�̊J�n���W
lcdbm_pos_t	test_popup_text_pos = { 11, 13 };


void test_packet_function( void )
{
#if 0
//
//	unsigned char	area = 1;
//
//
//		PKTcmd_image_part( 1, 0, FALSE );	// �\�����No.=1�A�O���t�B�b�N�p�^�[��No.=0�A�����~=���Ȃ�
//
//		// �e�L�X�g
//		PKTcmd_text_begin();
//
//// 		// �o�^�ς݂̕�����f�[�^���o�b�t�@�T�C�Y�𒴂����ꍇ�̃e�X�g
//// 		PKTcmd_text_direct( test_text_message[0], TEXT_CMD_BUF_SIZE-LCDBM_CMD_TEXT_DIRECT_SIZE );
//// 		PKTcmd_text_fileNo( &test_text_fileNo[0], TEXT_CMD_BUF_SIZE-LCDBM_CMD_TEXT_FILENO_SIZE );
//
//		// �e�֐��̃e�X�g
//		PKTcmd_text_font( LCDBM_LANGUAGE_CHINESE, LCDBM_TEXT_FONT_64 );
//		PKTcmd_text_color( lcdbm_color_RGB(22,45,22) );
//		PKTcmd_text_blink( ON, 4875/*0x130b*/ );
//
//		PKTcmd_text_align( LCDBM_TEXT_ALIGN_CENTER );
//		PKTcmd_text_scroll( float_fix16(33.333333) );
//		PKTcmd_text_clock( LCDBM_TEXT_CLOCK_ENGLISH, ON );
//
//		PKTcmd_text_direct( test_text_message[0], 0 );	// �����񒷂��O�Ƃ��ăe�X�g
//		PKTcmd_text_direct( test_text_message[0], 10 );	// ���ۂ̕����񒷂��Z���f�[�^�����w�肵�ăe�X�g
//		PKTcmd_text_direct( test_text_message[0], 40 );	// ���ۂ̕����񒷂�蒷���f�[�^�����w�肵�ăe�X�g
//		PKTcmd_text_fileNo( &test_text_fileNo[0], 0 );	// �f�[�^�����O�Ƃ��ăe�X�g
//		PKTcmd_text_fileNo( &test_text_fileNo[0], 10 );	// �w��\�ȃf�[�^���҂�����
//		PKTcmd_text_fileNo( &test_text_fileNo[0], 15 );	// �w��\�ȃf�[�^�����������ꍇ
//
//		PKTcmd_text_end( 254 );
//
//		// �|�b�v�A�b�v
//		PKTcmd_popup_on( test_popup_image_pos, test_popup_text_pos, 123 );
//		PKTcmd_popup_off();
//
//		// �ݒ�v��
//		PKTcmd_request_config();
//		PKTcmd_request_status();
//
//		// �@��ݒ�v��
//		PKTcmd_clock();
//		PKTcmd_backlight( ON );
//		PKTcmd_brightness( 21930 );	// �e�X�g�̂��߂ɑ傫���l���w�肵�Ă݂�
//
//
//		// �^�X�N�؂�ւ������āA��������d���𑗐M����
//		taskchg( IDLETSKNO );
//
//		// �e�L�X�g�֐��̃e�X�g
//		PKTcmd_text_1_font(   area, LCDBM_LANGUAGE_CHINESE, LCDBM_TEXT_FONT_64 );
//		PKTcmd_text_1_color(  area, lcdbm_color_RGB(22,45,22) );
//		PKTcmd_text_1_blink(  area, ON, 4875/*0x130b*/ );
//		PKTcmd_text_1_align(  area, LCDBM_TEXT_ALIGN_CENTER );
//		PKTcmd_text_1_scroll( area, float_fix16(33.333333) );
//		PKTcmd_text_1_clock(  area, LCDBM_TEXT_CLOCK_ENGLISH, ON );
//		PKTcmd_text_1_direct( area, test_text_message[0], 40 );
//		PKTcmd_text_1_fileNo( area, &test_text_fileNo[0], 15 );
//
#endif
}

#endif//CRW_DEBUG
//@debug >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// MH810100(E) K.Onodera  2019/11/11 �Ԕԃ`�P�b�g���X(GT-8700(LZ-122601)���p)
