/*[]---------------------------------------------------------------------------[]*/
/*|		���������䕔�F���ʏ����֐�												|*/
/*|																				|*/
/*|	̧�ٖ���	:	Pri_Cmn.c													|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author		: K.Motohashi													|*/
/*|	Date		: 2005-07-19													|*/
/*|	UpDate		:																|*/
/*|																				|*/
/*[]--------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"Message.h"
#include	"prm_tbl.h"
#include	"rkn_def.h"
#include	"LKmain.h"
#include	"flp_def.h"
#include	"Pri_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"Ope_def.h"
#include	"I2c_driver.h"
#include	"rtc_readwrite.h"
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
#include	"fla_def.h"
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

/*[]-----------------------------------------------------------------------[]*/
/*|		���������������														|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnInit( void )										|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-24													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnInit( void )
{

	rct_init_sts	= 0;					// ڼ�������	���������ؾ�āi�����������҂��j
	jnl_init_sts	= 0;					// �ެ��������	���������ؾ�āi�����������҂��j

	Header_Rsts		= 0xffffffff;			// ͯ�ް���ް��Ǎ���ԏ�����
	Footer_Rsts		= 0xffffffff;			// ̯�����ް��Ǎ���ԏ�����
	AcceptFooter_Rsts = 0xffffffff;			// ��t��̯�����ް��Ǎ���ԏ�����
	Syomei_Rsts		= 0xffffffff;			// �x�����ށE���������ް��Ǎ����
	Kamei_Rsts		= 0xffffffff;			// �����X�����ް��Ǎ����
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	EmgFooter_Rsts	= 0xffffffff;			// ��Q�A���[̯�����ް��Ǎ���ԏ�����
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

	rct_timer_end = 0;

	PrnInit_R();							// ڼ�������������

	if( PrnJnlCheck() == ON ){				// �ެ���������ڑ�����H

		//	�ެ��ِڑ�����
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
//		PrnInit_J();						// �ެ��������������
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
	}
	else{
		//	�ެ��ِڑ��Ȃ�
		jnl_init_sts	= 3;				// �ެ����������������Ծ�āi���ڑ��j
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ڼ�����������������													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnInit_R( void )									|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnInit_R( void )
{
	PCMD_INIT( R_PRI );			// [������̏�����]
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��s����QR�󎚂��x���Ĉ󎚓r���ň���������j�iGM803003���p�j
	PCMD_WRITE_RESET( R_PRI );	// [�y�[�W�o�b�t�@�[�N���A]
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��s����QR�󎚂��x���Ĉ󎚓r���ň���������j�iGM803003���p�j

	PCMD_STATUS_AUTO( R_PRI );	// [������ð���̎������M]
	PCMD_STATUS_SEND( R_PRI );	// [������ð���̑��M�v��]

	PrnMode_idle( R_PRI );		// �������Ӱ�ޏ�����Ԑݒ�

	PrnOut( R_PRI );			// ڼ��������ֺ���ޑ��M
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�ެ������������������												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnInit_J( void )									|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnInit_J( void )
{
	PCMD_INIT( J_PRI );			// [������̏�����]

	PCMD_STATUS_AUTO( J_PRI );	// [������ð���̎������M]
	PCMD_STATUS_SEND( J_PRI );	// [������ð���̑��M�v��]

	PrnMode_idle( J_PRI );		// �������Ӱ�ޏ�����Ԑݒ�

	PrnOut( J_PRI );			// �ެ���������ֺ���ޑ��M
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�������Ӱ�ޏ�����Ԑݒ�											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnMode_idle( pri_kind )							|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	pri_kind	= ��������						|*/
/*|							R_PRI  : ڼ��									|*/
/*|							J_PRI  : �ެ���									|*/
/*|							RJ_PRI : ڼ�ā��ެ���							|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-24													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnMode_idle( uchar pri_kind )
{

	switch( pri_kind ){		// �������ʁH

		case	R_PRI:			// ڼ��
			PrnMode_idle_R();	// �������Ӱ�ޏ�����Ԑݒ�iڼ�āj
			break;

		case	J_PRI:			// �ެ���
			PrnMode_idle_J();	// �������Ӱ�ޏ�����Ԑݒ�i�ެ��فj
			break;

		case	RJ_PRI:			// ڼ�ā��ެ���
			PrnMode_idle_R();	// �������Ӱ�ޏ�����Ԑݒ�iڼ�āj
			PrnMode_idle_J();	// �������Ӱ�ޏ�����Ԑݒ�i�ެ��فj
			break;

		default:				// ���̑��i�������ʴװ�j
			return;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ڼ���������Ӱ�ޏ�����Ԑݒ�										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnMode_idle_R( void )								|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-24													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnMode_idle_R( void )
{
	PrnFontSize( FONT_A_NML, R_PRI );	// ̫�Ă`�w��
	PCMD_R_SPACE( R_PRI );				// [�����̉E��߰���(0mm)]
	PCMD_SHIFTJIS( R_PRI );				// [�������ޑ̌n�I��(���JIS�̌n)]
	PCMD_KANJI4_CLR( R_PRI );			// [�����̂S�{�p����(����)]
	PCMD_FONT_K_N( R_PRI );				// [������Ӱ�ވꊇ�w��(�ʏ�)]
	PCMD_SPACE_KANJI( R_PRI );			// [������߰��ʂ̎w��(��0mm�A�E0mm)]
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�ެ����������Ӱ�ޏ�����Ԑݒ�										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnMode_idle_J( void )								|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-24													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnMode_idle_J( void )
{
	PrnFontSize( FONT_A_NML, J_PRI );	// ̫�Ă`�w��
	PCMD_R_SPACE( J_PRI );				// [�����̉E��߰���(0mm)]
	PCMD_SHIFTJIS( J_PRI );				// [�������ޑ̌n�I��(���JIS�̌n)]
	PCMD_KANJI4_CLR( J_PRI );			// [�����̂S�{�p����(����)]
	PCMD_FONT_K_N( J_PRI );				// [������Ӱ�ވꊇ�w��(�ʏ�)]
	PCMD_SPACE_KANJI( J_PRI );			// [������߰��ʂ̎w��(��0mm�A�E0mm)]
}

/*[]-----------------------------------------------------------------------[]*/
/*|		������󎚕������ސݒ�												|*/
/*|																			|*/
/*|	�������T�v��															|*/
/*|	���Ұ��Ŏw�肳�ꂽ������ʂɑΉ�����[��Ӱ�ގw��]�y��[���s�ʎw��]		|*/
/*|	������������ނ�ҏW����B												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnFontSize( font, pri_kind )						|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	font= �������								|*/
/*|																			|*/
/*|						uchar	pri_kind=	��������						|*/
/*|							R_PRI  : ڼ��									|*/
/*|							J_PRI  : �ެ���									|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-24													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnFontSize( uchar font, uchar pri_kind )
{
	PRN_PROC_DATA	*p_proc_data;	// �ҏW���������ް��߲���
	ushort	f_data;					// �w��̫�ďc�ޯĐ�
	ushort	s_data;					// �s�Ԑݒ�l�i�ޯĐ��j
	ushort	kaigyou;				// ���s���ޯĐ�
	uchar	cmd[3];					// ���s�ʎw�����ށ�ESC 3 n���ް�


	//	�ҏW���������ް��߲����擾
	switch( pri_kind ){	// �������ʁH

		case	R_PRI:		// 	ڼ��

			p_proc_data = (PRN_PROC_DATA *)&rct_proc_data;	// ڼ�ĕҏW���������ް��߲�����

			if( gyoukan_data != 0 ){
				//	���s�����w�肳��Ă���ꍇ�i�w���ް��ɏ]���j
				s_data = (ushort)gyoukan_data;
			}
			else{
				//	���s�����w�肳��Ă��Ȃ��ꍇ�i�ݒ��ް��ɏ]���j
				s_data = (ushort)CPrmSS[S_PRN][1];			// �ݒ��ް��擾�iڼ�ĉ��s���j
			}

			break;

		case	J_PRI:		// 	�ެ���

			p_proc_data = (PRN_PROC_DATA *)&jnl_proc_data;	// �ެ��ٕҏW���������ް��߲�����

			if( gyoukan_data != 0 ){
				//	���s�����w�肳��Ă���ꍇ�i�w���ް��ɏ]���j
				s_data = (ushort)gyoukan_data;
			}
			else{
				//	���s�����w�肳��Ă��Ȃ��ꍇ�i�ݒ��ް��ɏ]���j
				s_data = (ushort)CPrmSS[S_PRN][2];			// �ݒ��ް��擾�i�ެ��ى��s���j
			}

			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}

	//	��Ӱ�ސݒ�i������������ޕҏW�j
	switch( font ){		// ��Ӱ�ށi������ʁj�H

		//	̫�Ă`
		case	FONT_A_NML:			// ̫�Ă` �ʏ�

			if( p_proc_data->Font_size != font ){

				//	���݂̎w�肩��ύX�ɂȂ�ꍇ
				p_proc_data->Font_size = font;		// �w�肷�镶����ʂ��
				PCMD_FONT_A_N( pri_kind );			// [��Ӱ�ނ̈ꊇ�w��(̫�Ă`�ʏ�)]�i������������ޕҏW�j
			}
			f_data = FONT_A_N_HDOT;					// �w��̫�ďc�ޯĐ����
			break;

		case	FONT_A_TATEBAI:		// ̫�Ă` �c�{

			if( p_proc_data->Font_size != font ){

				//	���݂̎w�肩��ύX�ɂȂ�ꍇ
				p_proc_data->Font_size = font;		// �w�肷�镶����ʂ��
				PCMD_FONT_A_T( pri_kind );			// [��Ӱ�ނ̈ꊇ�w��(̫�Ă`�c�{)]�i������������ޕҏW�j
			}
			f_data = FONT_A_T_HDOT;					// �w��̫�ďc�ޯĐ����
			break;

		case	FONT_A_YOKOBAI:		// ̫�Ă` ���{

			if( p_proc_data->Font_size != font ){

				//	���݂̎w�肩��ύX�ɂȂ�ꍇ
				p_proc_data->Font_size = font;		// �w�肷�镶����ʂ��
				PCMD_FONT_A_Y( pri_kind );			// [��Ӱ�ނ̈ꊇ�w��(̫�Ă`���{)]�i������������ޕҏW�j
			}
			f_data = FONT_A_Y_HDOT;					// �w��̫�ďc�ޯĐ����
			break;

		case	FONT_A_4BAI:		// ̫�Ă` �S�{

			if( p_proc_data->Font_size != font ){

				//	���݂̎w�肩��ύX�ɂȂ�ꍇ
				p_proc_data->Font_size = font;		// �w�肷�镶����ʂ��
				PCMD_FONT_A_4( pri_kind );			// [��Ӱ�ނ̈ꊇ�w��(̫�Ă`�S�{)]�i������������ޕҏW�j
			}
			f_data = FONT_A_4_HDOT;					// �w��̫�ďc�ޯĐ����
			break;

		//	̫�Ăa
		case	FONT_B_NML:			// ̫�Ăa �ʏ�

			if( p_proc_data->Font_size != font ){

				//	���݂̎w�肩��ύX�ɂȂ�ꍇ
				p_proc_data->Font_size = font;		// �w�肷�镶����ʂ��
				PCMD_FONT_B_N( pri_kind );			// [��Ӱ�ނ̈ꊇ�w��(̫�Ăa�ʏ�)]�i������������ޕҏW�j
			}
			f_data = FONT_B_N_HDOT;					// �w��̫�ďc�ޯĐ����
			break;

		case	FONT_B_TATEBAI:		// ̫�Ăa �c�{

			if( p_proc_data->Font_size != font ){

				//	���݂̎w�肩��ύX�ɂȂ�ꍇ
				p_proc_data->Font_size = font;		// �w�肷�镶����ʂ��
				PCMD_FONT_B_T( pri_kind );			// [��Ӱ�ނ̈ꊇ�w��(̫�Ăa�c�{)]�i������������ޕҏW�j
			}
			f_data = FONT_B_T_HDOT;					// �w��̫�ďc�ޯĐ����
			break;

		case	FONT_B_YOKOBAI:		// ̫�Ăa ���{

			if( p_proc_data->Font_size != font ){

				//	���݂̎w�肩��ύX�ɂȂ�ꍇ
				p_proc_data->Font_size = font;		// �w�肷�镶����ʂ��
				PCMD_FONT_B_Y( pri_kind );			// [��Ӱ�ނ̈ꊇ�w��(̫�Ăa���{)]�i������������ޕҏW�j
			}
			f_data = FONT_B_Y_HDOT;					// �w��̫�ďc�ޯĐ����
			break;

		case	FONT_B_4BAI:		// ̫�Ăa �S�{

			if( p_proc_data->Font_size != font ){

				//	���݂̎w�肩��ύX�ɂȂ�ꍇ
				p_proc_data->Font_size = font;		// �w�肷�镶����ʂ��
				PCMD_FONT_B_4( pri_kind );			// [��Ӱ�ނ̈ꊇ�w��(̫�Ăa�S�{)]�i������������ޕҏW�j
			}
			f_data = FONT_B_4_HDOT;					// �w��̫�ďc�ޯĐ����
			break;

		//	����
		case	FONT_K_NML:			// ���� �ʏ�

			if( p_proc_data->Kanji_size != font ){

				//	���݂̎w�肩��ύX�ɂȂ�ꍇ
				p_proc_data->Kanji_size = font;		// �w�肷�镶����ʂ��
				PCMD_FONT_K_N( pri_kind );			// [������Ӱ�ނ̈ꊇ�w��(�����ʏ�)]�i������������ޕҏW�j
			}
			f_data = FONT_K_N_HDOT;					// �w��̫�ďc�ޯĐ����
			break;

		case	FONT_K_TATEBAI:		// ���� �c�{

			if( p_proc_data->Kanji_size != font ){

				//	���݂̎w�肩��ύX�ɂȂ�ꍇ
				p_proc_data->Kanji_size = font;		// �w�肷�镶����ʂ��
				PCMD_FONT_K_T( pri_kind );			// [������Ӱ�ނ̈ꊇ�w��(�����c�{)]�i������������ޕҏW�j
			}
			f_data = FONT_K_T_HDOT;					// �w��̫�ďc�ޯĐ����
			break;

		case	FONT_K_YOKOBAI:		// ���� ���{

			if( p_proc_data->Kanji_size != font ){

				//	���݂̎w�肩��ύX�ɂȂ�ꍇ
				p_proc_data->Kanji_size = font;		// �w�肷�镶����ʂ��
				PCMD_FONT_K_Y( pri_kind );			// [������Ӱ�ނ̈ꊇ�w��(�������{)]�i������������ޕҏW�j
			}
			f_data = FONT_K_Y_HDOT;					// �w��̫�ďc�ޯĐ����
			break;

		case	FONT_K_4BAI:		// ���� �S�{

			if( p_proc_data->Kanji_size != font ){

				//	���݂̎w�肩��ύX�ɂȂ�ꍇ
				p_proc_data->Kanji_size = font;		// �w�肷�镶����ʂ��
				PCMD_FONT_K_4( pri_kind );			// [������Ӱ�ނ̈ꊇ�w��(�����S�{)]�i������������ޕҏW�j
			}
			f_data = FONT_K_4_HDOT;					// �w��̫�ďc�ޯĐ����
			break;

		default:					// ������ʂm�f
			return;
	}
	//	���s�ʂ����߂�
	if( (s_data + f_data) > 0xff ){		// ���s�ʍő�l�i0xff�j���ް�H
		kaigyou = 0xff;					// YES�F���s�ʍő�l�i0xff�j���
	}
	else{
		kaigyou = s_data + f_data;		// NO�F���s�ʁi�s�Ԑݒ�l�{�w��̫�ďc�ޯĐ��j���
	}
	if( p_proc_data->Kaigyou_size != kaigyou ){

		//	���s�ʁi���j�����݂̎w�肩��ύX�ɂȂ�ꍇ

		p_proc_data->Kaigyou_size = (uchar)kaigyou;	// �w�肷����s�ʁi���j���

		cmd[0] = 0x1b;						// "ESC"
		cmd[1] = 0x33;						// "3"
		cmd[2] = (uchar)kaigyou;			// ���s���ް����(n)
		PrnCmdLen( cmd , 3 , pri_kind );	// [���s�ʎw��]�i������������ޕҏW�j
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		��������ҏW���������ް�������										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	prn_proc_data_clr( pri_kind )						|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	pri_kind=	��������						|*/
/*|							R_PRI  : ڼ��									|*/
/*|							J_PRI  : �ެ���									|*/
/*|							RJ_PRI : ڼ�ā��ެ���							|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-21													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	prn_proc_data_clr( uchar pri_kind )
{
	switch( pri_kind ){		// �������ʁH

		case	R_PRI:		// ڼ��
			prn_proc_data_clr_R();	// �ҏW���������ް��������iڼ�āj
			break;

		case	J_PRI:		// �ެ���
			prn_proc_data_clr_J();	// �ҏW���������ް��������i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���
			prn_proc_data_clr_R();	// �ҏW���������ް��������iڼ�āj
			prn_proc_data_clr_J();	// �ҏW���������ް��������i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ڼ�ĕҏW���������ް�������											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	prn_proc_data_clr_R( void )							|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	prn_proc_data_clr_R( void )
{
	uchar	i;

	rct_proc_data.Printing		= 0;		// �󎚏������
	rct_proc_data.Tyushi_Cmd	= 0;		// �󎚒��~ү���ގ�M�׸�
	rct_proc_data.Split 		= 0;		// �󎚏���������ۯ���
	rct_proc_data.Final 		= 0;		// �����󎚍ŏI��ۯ��ʒm
	rct_proc_data.EditWait		= 0;		// ���ް��ҏW�҂����

	rct_proc_data.Prn_no 		= 0;		// �󎚏������̍��ڔԍ�
	rct_proc_data.Prn_no_data1	= 0;		// �󎚏������̍��ڔԍ��̻�ޏ��P
	rct_proc_data.Prn_no_data2	= 0;		// �󎚏������̍��ڔԍ��̻�ޏ��Q
	for( i=0 ; i<10 ; i++ ){				// �󎚏����ėpܰ�
		rct_proc_data.Prn_no_wk[i] = 0;
	}

	rct_proc_data.Log_Start		= 0;		// ۸��ް��󎚊J�n�׸�
	rct_proc_data.Log_Edit		= 0;		// ۸��ް��ҏW���׸�
	rct_proc_data.Log_Count		= 0;		// ۸��ް���
	rct_proc_data.Log_DataPt	= 0;		// ۸��ް��߲��
	rct_proc_data.Log_Sdate		= 0;		// ۸��ް������J�n��
	rct_proc_data.Log_Edate		= 0;		// ۸��ް������I����
	rct_proc_data.Log_no_wk[0]	= 0;		// ۸ވ󎚏����ėpܰ�
	rct_proc_data.Log_no_wk[1]	= 0;

	rct_proc_data.Font_size		= 0;		// ̫�ĕ������ށi������Ɏw�肵��������ʂ��i�[�j
	rct_proc_data.Kanji_size	= 0;		// ����̫�Ļ��ށi������Ɏw�肵��������ʂ��i�[�j
	rct_proc_data.Kaigyou_size	= 0;		// ���s���ށi������Ɏw�肵�����s�ʁi���j���ޯĐ��P�ʂŊi�[�j

}

/*[]-----------------------------------------------------------------------[]*/
/*|		�ެ��ٕҏW���������ް�������										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	prn_proc_data_clr_J( void )							|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	prn_proc_data_clr_J( void )
{
	uchar	i;

	jnl_proc_data.Printing		= 0;		// �󎚏������
	jnl_proc_data.Tyushi_Cmd	= 0;		// �󎚒��~ү���ގ�M�׸�
	jnl_proc_data.Split 		= 0;		// �󎚏���������ۯ���
	jnl_proc_data.Final 		= 0;		// �����󎚍ŏI��ۯ��ʒm
	jnl_proc_data.EditWait		= 0;		// ���ް��ҏW�҂����

	jnl_proc_data.Prn_no 		= 0;		// �󎚏������̍��ڔԍ�
	jnl_proc_data.Prn_no_data1	= 0;		// �󎚏������̍��ڔԍ��̻�ޏ��P
	jnl_proc_data.Prn_no_data2	= 0;		// �󎚏������̍��ڔԍ��̻�ޏ��Q
	for( i=0 ; i<10 ; i++ ){				// �󎚏����ėpܰ�
		jnl_proc_data.Prn_no_wk[i] = 0;
	}

	jnl_proc_data.Log_Start		= 0;		// ۸��ް��󎚊J�n�׸�
	jnl_proc_data.Log_Edit		= 0;		// ۸��ް��ҏW���׸�
	jnl_proc_data.Log_Count		= 0;		// ۸��ް���
	jnl_proc_data.Log_DataPt	= 0;		// ۸��ް��߲��
	jnl_proc_data.Log_Sdate		= 0;		// ۸��ް������J�n��
	jnl_proc_data.Log_Edate		= 0;		// ۸��ް������I����
	jnl_proc_data.Log_no_wk[0]	= 0;		// ۸ވ󎚏����ėpܰ�
	jnl_proc_data.Log_no_wk[1]	= 0;

	jnl_proc_data.Font_size		= 0;		// ̫�ĕ������ށi������Ɏw�肵��������ʂ��i�[�j
	jnl_proc_data.Kanji_size	= 0;		// ����̫�Ļ��ށi������Ɏw�肵��������ʂ��i�[�j
	jnl_proc_data.Kaigyou_size	= 0;		// ���s���ށi������Ɏw�肵�����s�ʁi���j���ޯĐ��P�ʂŊi�[�j

// MH810104 GG119201(S) �d�q�W���[�i���Ή�
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
//	if (isEJA_USE()) {
	{
		jnl_proc_data.Prn_Job_id = 0;
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		// �󎚐���f�[�^�ƈ󎚃��b�Z�[�W���N���A����
		memset(&eja_prn_buff.eja_proc_data, 0, sizeof(eja_prn_buff.eja_proc_data));
		memset(&eja_prn_buff.PrnMsg, 0, sizeof(eja_prn_buff.PrnMsg));
	}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�ҏW��������ܰ��ر������											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	prn_edit_wk_clr( void )								|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-21													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	prn_edit_wk_clr( void )
{

	gyoukan_data	= 0;
	syuukei_kind	= 0;
	prn_zero		= 0;

	memset( prn_work,0,sizeof(prn_work) );
	memset( cTmp1,0,sizeof(cTmp1) );
	memset( cTmp2,0,sizeof(cTmp2) );
	memset( cMnydata,0,sizeof(cMnydata) );
	memset( cEditwk1,0,sizeof(cEditwk1) );
	memset( cEditwk2,0,sizeof(cEditwk2) );
	memset( cEditwk3,0,sizeof(cEditwk3) );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�󎚒��~ү���ގ�M����												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Inji_Cancel_chk( pri_kind )							|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	pri_kind=	��������						|*/
/*|							R_PRI : ڼ��									|*/
/*|							J_PRI : �ެ���									|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							YES : �󎚒��~ү���ގ�M����					|*/
/*|							NO  : �󎚒��~ү���ގ�M�Ȃ�					|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-25													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	Inji_Cancel_chk( uchar pri_kind )
{
	PRN_PROC_DATA	*p_proc_data;	// �ҏW���������ް��߲���
	uchar			ret = NO;		// �߂�l

	if( next_prn_msg == ON ){	// �󎚏������̎��ҏW�v���H

		switch( pri_kind ){		// �Ώ�������H

			case	R_PRI:		// ڼ��
				p_proc_data = (PRN_PROC_DATA *)&rct_proc_data;
				break;

			case	J_PRI:		// �ެ���
				p_proc_data = (PRN_PROC_DATA *)&jnl_proc_data;
				break;

			default:			// ���̑��i�������ʴװ�j
				return( ret );
		}
		if( p_proc_data->Tyushi_Cmd == ON ){		// �󎚒��~ү���ގ�M�ς݁H

			//	�󎚒��~ү���ނ���M���Ă����ꍇ
			ret = YES;								// �߂�l�Ɂu�󎚒��~�v���
		}
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�󎚒��~����														|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Inji_Cancel( *msg, pri_kind )						|*/
/*|																			|*/
/*|	PARAMETER		:	MSG		*msg	=	��Mү�����߲���				|*/
/*|						uchar	pri_kind=	��������						|*/
/*|							R_PRI  : ڼ��									|*/
/*|							J_PRI  : �ެ���									|*/
/*|							RJ_PRI : ڼ�ā��ެ���							|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-21													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	Inji_Cancel( MSG *msg, uchar pri_kind )
{
	switch( pri_kind ){		// �������ʁH

		case	R_PRI:		// ڼ��
			Inji_Cancel_R( msg );	// �󎚒��~�����iڼ�āj
			break;

		case	J_PRI:		// �ެ���
			Inji_Cancel_J( msg );	// �󎚒��~�����i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���
			Inji_Cancel_R( msg );	// �󎚒��~�����iڼ�āj
			Inji_Cancel_J( msg );	// �󎚒��~�����i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ڼ�Ĉ󎚒��~����													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Inji_Cancel_R( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG		*msg	=	��Mү�����߲���				|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	Inji_Cancel_R( MSG *msg )
{
	prn_proc_data_clr_R();			// ڼ�Ĉ󎚕ҏW����ر������
	PriBothPrint = 0;
	PrnEndMsg(	msg->command,		// �u�󎚏I���i��ݾفj�vү���ޑ��M
				PRI_CSL_END,
				PRI_ERR_NON,
				R_PRI );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�ެ��و󎚒��~����													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Inji_Cancel_J( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG		*msg	=	��Mү�����߲���				|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	Inji_Cancel_J( MSG *msg )
{
	prn_proc_data_clr_J();			// �ެ��و󎚕ҏW����ر������
	if(msg->command == BothPrintCommand){
		PriBothPrint = 0;
		BothPrintCommand = 0;
	}
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
//	if (isEJA_USE()) {
	{
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		// �f�[�^�����ݏI����ʒm����
		PrnCmd_WriteStartEnd(1, NULL);
	}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
	PrnEndMsg(	msg->command,		// �u�󎚏I���i��ݾفj�vү���ޑ��M
				PRI_CSL_END,
				PRI_ERR_NON,
				J_PRI );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�󎚈ُ�I������													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Inji_ErrEnd( command, pri_sts, pri_kind )			|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	command	=	�󎚗v��ү���޺����				|*/
/*|																			|*/
/*|						uchar	pri_sts	=	�ُ�I�����R					|*/
/*|																			|*/
/*|						uchar	pri_kind=	��������						|*/
/*|							R_PRI  : ڼ��									|*/
/*|							J_PRI  : �ެ���									|*/
/*|							RJ_PRI : ڼ�ā��ެ���							|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-03													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	Inji_ErrEnd( ushort command, uchar pri_sts, uchar pri_kind )
{
	switch( pri_kind ){		// �������ʁH

		case	R_PRI:		// ڼ��
			Inji_ErrEnd_R( command, pri_sts );	// �󎚈ُ�I�������iڼ�āj
			break;

		case	J_PRI:		// �ެ���
			Inji_ErrEnd_J( command, pri_sts );	// �󎚈ُ�I�������i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���
			Inji_ErrEnd_R( command, pri_sts );	// �󎚈ُ�I�������iڼ�āj
			Inji_ErrEnd_J( command, pri_sts );	// �󎚈ُ�I�������i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ڼ�Ĉ󎚈ُ�I������												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Inji_ErrEnd_R( command, pri_sts )					|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	command	=	�󎚗v��ү���޺����				|*/
/*|																			|*/
/*|						uchar	pri_sts	=	�ُ�I�����R					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-03													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	Inji_ErrEnd_R( ushort command, uchar pri_sts )
{
	prn_proc_data_clr_R();			// ڼ�Ĉ󎚕ҏW����ر������
	I2cSendErrDataInit(I2C_DN_RP);
	Lagcan( PRNTCBNO, 7 );
	rct_timer_end = 0;
	PriBothPrint = 0;

	PrnEndMsg(						/*-----	�󎚏I��ү���ޑ��M	-----*/
				command,			// �󎚗v������ށF�������Ұ�
				PRI_ERR_END,		// �󎚏�������	�F�ُ�I��
				pri_sts,			// �ُ�I�����R	�F�������Ұ�
				R_PRI				// ��������	�Fڼ��
			);
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�ެ��و󎚈ُ�I������												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Inji_ErrEnd_J( command, pri_sts )					|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	command	=	�󎚗v��ү���޺����				|*/
/*|																			|*/
/*|						uchar	pri_sts	=	�ُ�I�����R					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-03													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	Inji_ErrEnd_J( ushort command, uchar pri_sts )
{
	prn_proc_data_clr_J();			// �ެ��و󎚕ҏW����ر������
	I2cSendErrDataInit(I2C_DN_JP);
	if( command == BothPrintCommand ){
		PriBothPrint = 0;
		BothPrintCommand = 0;
	}

	PrnEndMsg(						/*-----	�󎚏I��ү���ޑ��M	-----*/
				command,			// �󎚗v������ށF�������Ұ�
				PRI_ERR_END,		// �󎚏�������	�F�ُ�I��
				pri_sts,			// �ُ�I�����R	�F�������Ұ�
				J_PRI				// ��������	�F�ެ���
			);
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���ް��P��ۯ��ҏW�I������											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	End_Set( *msg, pri_kind )							|*/
/*|																			|*/
/*|	PARAMETER		:	MSG		*msg	=	��Mү�����߲���				|*/
/*|						uchar	pri_kind=	��������						|*/
/*|							R_PRI  : ڼ��									|*/
/*|							J_PRI  : �ެ���									|*/
/*|							RJ_PRI : ڼ�ā��ެ���							|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	End_Set( MSG *msg, uchar pri_kind )
{
	switch( pri_kind ){		// �������ʁH

		case	R_PRI:		// ڼ��
			End_Set_R( msg );		// ڼ�Ĉ��ް��P��ۯ��ҏW�I������
			break;

		case	J_PRI:		// �ެ���
			End_Set_J( msg );		// �ެ��و��ް��P��ۯ��ҏW�I������
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���
			End_Set_R( msg );		// ڼ�Ĉ��ް��P��ۯ��ҏW�I������
			End_Set_J( msg );		// �ެ��و��ް��P��ۯ��ҏW�I������
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ڼ�Ĉ��ް��P��ۯ��ҏW�I������										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	End_Set_R( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG		*msg	=	��Mү�����߲���				|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	End_Set_R( MSG *msg )
{
	MsgBuf	*OutMsg;		// ���Mү�����ޯ̧�߲���

	if( rct_proc_data.Final == ON ){	// �ŏI��ۯ��ҏW�I���H

		//	�ŏI��ۯ��ҏW�I���̏ꍇ

		if( msg->command != PREQ_LOGO_REGIST ){
										// ���S�󎚃f�[�^�o�^�I���̏ꍇ�́Aڼ�Đ擪�󎚏��������Ȃ�
			Rct_top_edit( ON );			// ڼ�Đ擪���ް��ҏW�����iۺވ󎚁^ͯ�ް�󎚁^�p����Đ���j
		}
		PrnOut( R_PRI );				// ������N���iڼ����������M������Enable�j

		PrnEndMsg(	msg->command,		// �u�󎚏I���i����I���j�vү���ޑ��M
					PRI_NML_END,
					PRI_ERR_NON,
					R_PRI );
		if(PriBothPrint == 1) {
			if( (OutMsg = GetBuf()) == NULL ){						// ���Mү�����ޯ̧�擾
				return;												// �擾�m�f
			}
			memcpy(&OutMsg->msg, msg, sizeof(MSG));					// �R�}���h�f�[�^���R�s�[
			OutMsg->msg.data[0] = J_PRI;							// �󎚑Ώۂ��W���[�i���ɂ���
			PutMsg( PRNTCBNO, OutMsg );								// ���������ֈ󎚗v���i����ۯ��ҏW�v���jү���ޑ��M
			PriBothPrint = 2;
			BothPrintCommand = msg->command;
		}
	}
	else{
		//	�ŏI��ۯ��łȂ��ꍇ�i���ҏW��ۯ�����j
		PrnOut( R_PRI );				// ������N���iڼ����������M������Enable�j
		memcpy( &NextMsg_r, msg, sizeof(NextMsg_r) );
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�ެ��و��ް��P��ۯ��ҏW�I������									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	End_Set_J( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG		*msg	=	��Mү�����߲���				|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	End_Set_J( MSG *msg )
{

// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
//// MH810104 GG119201(S) �d�q�W���[�i���Ή�
//	if (isEJA_USE()) {
//		// �f�[�^�����ݏI����ʒm����
//		PrnCmd_WriteStartEnd(1, NULL);
//	}
//// MH810104 GG119201(E) �d�q�W���[�i���Ή�
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j

	if( jnl_proc_data.Final == ON ){	// �ŏI��ۯ��ҏW�I���H

		//	�ŏI��ۯ��ҏW�I���̏ꍇ
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		// �ŏI�u���b�N�ҏW�I�����Ƀf�[�^�����ݏI����ʒm����
		PrnCmd_WriteStartEnd(1, NULL);
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j

		PrnOut( J_PRI );				// ������N���i�ެ�����������M������Enable�j

		PrnEndMsg(	msg->command,		// �u�󎚏I���i����I���j�vү���ޑ��M
					PRI_NML_END,
					PRI_ERR_NON,
					J_PRI );
		if(msg->command == BothPrintCommand){
			PriBothPrint = 0;
			BothPrintCommand = 0;
		}
	}
	else{
		//	�ŏI��ۯ��łȂ��ꍇ�i���ҏW��ۯ�����j
		PrnOut( J_PRI );				// ������N���i�ެ�����������M������Enable�j
		memcpy( &NextMsg_j, msg, sizeof(NextMsg_j) );
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�󎚏I��ү���ޑ��M�����ito ���ڰ�������j							|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnEndMsg( comm, result, stat, pri_kind )			|*/
/*|																			|*/
/*| PARAMETER		:	ushort	comm	= �󎚗v��ү���޺����				|*/
/*|																			|*/
/*|						uchar	result	= �󎚏�������						|*/
/*|																			|*/
/*|						uchar	stat	= �ُ�I�����R						|*/
/*|																			|*/
/*|						uchar	pri_kind = ��������						|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-02													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnEndMsg(	ushort	comm,
					uchar	result,
					uchar	stat,
					uchar	pri_kind )
{
	switch( pri_kind ){		// �������ʁH

		case	R_PRI:		// ڼ��

			PrnEndMsg_R( comm, result, stat );		// �󎚏I��ү���ޑ��M�����iڼ�āj
			break;

		case	J_PRI:		// �ެ���

			PrnEndMsg_J( comm, result, stat );		// �󎚏I��ү���ޑ��M�����i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���

			PrnEndMsg_R( comm, result, stat );		// �󎚏I��ү���ޑ��M�����iڼ�āj
			PrnEndMsg_J( comm, result, stat );		// �󎚏I��ү���ޑ��M�����i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ڼ�Ĉ󎚏I��ү���ޑ��M�����ito ���ڰ�������j						|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnEndMsg_R( comm, result, stat )					|*/
/*|																			|*/
/*| PARAMETER		:	ushort	comm	= �󎚗v��ү���޺����				|*/
/*|																			|*/
/*|						uchar	result	= �󎚏�������						|*/
/*|																			|*/
/*|						uchar	stat	= �ُ�I�����R						|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-02													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnEndMsg_R(ushort	comm,
					uchar	result,
					uchar	stat )
{
	MsgBuf		*Msg;		// ���Mү�����ޯ̧�߲���
	T_FrmEnd	data;		// �󎚏I��ү�����ް��ޯ̧

	if( (Msg = GetBuf()) == NULL ){				// ���Mү�����ޯ̧�擾
		return;									// �擾�m�f
	}
	Msg->msg.command = comm|INNJI_ENDMASK;		// ���M����ށF�󎚏I��ү�����׸޾��

	/*-----		���M�ް��쐬	-----*/
	data.BMode		= result;
	data.BStat		= stat;
	data.BPrinStat	= (uchar)(rct_prn_buff.PrnState[0] & 0x0f);
	data.BPrikind	= R_PRI;

	memcpy(	&Msg->msg.data,						// ���M�ް����
			&data,
			sizeof(T_FrmEnd) );

	PutMsg( OPETCBNO, Msg );					// ���ڰ�������ֈ󎚏I��ү���ޑ��M
// MH322914 (s) kasiyama 2016/07/13 �̎��؈󎚒��͐��Z�J�n�����Ȃ�[���ʃo�ONo.1275](MH341106)
	if( comm == PREQ_RYOUSYUU ) {
		ryo_inji = 0;							// �̎��؈󎚏I��
	}
// MH322914 (e) kasiyama 2016/07/13 �̎��؈󎚒��͐��Z�J�n�����Ȃ�[���ʃo�ONo.1275](MH341106)
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�ެ��و󎚏I��ү���ޑ��M�����ito ���ڰ�������j						|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnEndMsg_J( comm, result, stat )					|*/
/*|																			|*/
/*| PARAMETER		:	ushort	comm	= �󎚗v��ү���޺����				|*/
/*|																			|*/
/*|						uchar	result	= �󎚏�������						|*/
/*|																			|*/
/*|						uchar	stat	= �ُ�I�����R						|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-02													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnEndMsg_J(ushort	comm,
					uchar	result,
					uchar	stat )
{
	MsgBuf		*Msg;		// ���Mү�����ޯ̧�߲���
	T_FrmEnd	data;		// �󎚏I��ү�����ް��ޯ̧

	if( (Msg = GetBuf()) == NULL ){				// ���Mү�����ޯ̧�擾
		return;									// �擾�m�f
	}
	Msg->msg.command = comm|INNJI_ENDMASK;		// ���M����ށF�󎚏I��ү�����׸޾��

	/*-----		���M�ް��쐬	-----*/
	data.BMode		= result;
	data.BStat		= stat;
	data.BPrinStat	= (uchar)(jnl_prn_buff.PrnState[0] & 0x0f);
	data.BPrikind	= J_PRI;

	memcpy(	&Msg->msg.data,						// ���M�ް����
			 &data,
			sizeof(T_FrmEnd) );

	PutMsg( OPETCBNO, Msg );					// ���ڰ�������ֈ󎚏I��ү���ޑ��M
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���ް�����ۯ��ҏW�v��ү���ޑ��M����								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnNext( *Msg, pri_kind )							|*/
/*|																			|*/
/*| PARAMETER		:	MSG		*msg		= ���Mү�����߲���				|*/
/*|						uchar	pri_kind	= ��������						|*/
/*|							R_PRI  : ڼ��									|*/
/*|							J_PRI  : �ެ���									|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnNext( MSG *Msg, uchar pri_kind )
{
	switch( pri_kind ){		// �������ʁH

		case	R_PRI:			// ڼ��
			PrnNext_R( Msg );	// ���ް�����ۯ��ҏW�v��ү���ޑ��M�����iڼ�āj
			break;

		case	J_PRI:			// �ެ���
			PrnNext_J( Msg );	// ���ް�����ۯ��ҏW�v��ү���ޑ��M�����i�ެ��فj
			break;

		case	RJ_PRI:			// ڼ�ā��ެ���
			PrnNext_R( Msg );	// ���ް�����ۯ��ҏW�v��ү���ޑ��M�����iڼ�āj
			PrnNext_J( Msg );	// ���ް�����ۯ��ҏW�v��ү���ޑ��M�����i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ڼ�Ĉ��ް�����ۯ��ҏW�v��ү���ޑ��M����							|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnNext_R( *Msg )									|*/
/*|																			|*/
/*| PARAMETER		:	MSG		*Msg		= ���Mү�����߲���				|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnNext_R( MSG *Msg )
{
	MsgBuf	*OutMsg;		// ���Mү�����ޯ̧�߲���
	uchar	pri_data;		// ���������ް��쐬ܰ�

	if( (OutMsg = GetBuf()) == NULL ){						// ���Mү�����ޯ̧�擾
		return;												// �擾�m�f
	}
	OutMsg->msg.command = Msg->command | INNJI_NEXTMASK;	// ���M����ށF�󎚏�����ү�����׸ށiINNJI_NEXTMASK�j���

	memcpy(	OutMsg->msg.data,								// ���M�ް����
			Msg->data,
			sizeof(OutMsg->msg.data) );

	//	���������ް��쐬�i�ؼ��ق����������ް���ʂS�ޯĂ��������ʂ�āj
	pri_data = R_PRI << 4;
	pri_data |= Msg->data[0];
	OutMsg->msg.data[0] = pri_data;

	PutMsg( PRNTCBNO, OutMsg );								// ���������ֈ󎚗v���i����ۯ��ҏW�v���jү���ޑ��M
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�ެ��و��ް�����ۯ��ҏW�v��ү���ޑ��M����							|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnNext_J( *Msg )									|*/
/*|																			|*/
/*| PARAMETER		:	MSG		*Msg		= ���Mү�����߲���				|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnNext_J( MSG *Msg )
{
	MsgBuf	*OutMsg;		// ���Mү�����ޯ̧�߲���
	uchar	pri_data;		// ���������ް��쐬ܰ�

	if( (OutMsg = GetBuf()) == NULL ){						// ���Mү�����ޯ̧�擾
		return;												// �擾�m�f
	}
	OutMsg->msg.command = Msg->command | INNJI_NEXTMASK;	// ���M����ށF�󎚏�����ү�����׸ށiINNJI_NEXTMASK�j���

	memcpy(	OutMsg->msg.data,								// ���M�ް����
			Msg->data,
			sizeof(OutMsg->msg.data) );

	//	���������ް��쐬�i�ؼ��ق����������ް���ʂS�ޯĂ��������ʂ�āj
	pri_data = J_PRI << 4;
	pri_data |= Msg->data[0];
	OutMsg->msg.data[0] = pri_data;

	PutMsg( PRNTCBNO, OutMsg );								// ���������ֈ󎚗v���i����ۯ��ҏW�v���jү���ޑ��M
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���������ւ�ү���ޑ��M												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	MsgSndFrmPrn( cmd ,data1, data2 )					|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	cmd 	= ���M�����							|*/
/*|						uchar	data1	= ���M�ް�1							|*/
/*|						uchar	data2	= ���M�ް�2							|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-10													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	MsgSndFrmPrn( ushort cmd , uchar data1, uchar data2 )
{
	MsgBuf *Msg;

	if( (Msg = GetBuf()) != NULL )
	{
		Msg->msg.command = cmd;
		Msg->msg.data[0] = data1;
		Msg->msg.data[1] = data2;
		PutMsg( PRNTCBNO , Msg );
	}
}
/*[]-----------------------------------------------------------------------[]*/
/*|		�ެ���������ڑ��L������												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnJnlCheck( void )									|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							ON 	: �ڑ�����									|*/
/*|							OFF : �ڑ��Ȃ�									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-17													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnJnlCheck( void )
{
	uchar	ret;

	if( prm_get( COM_PRM,S_PAY,21,1,1 ) == 0 ){		// �u�ެ���������ڑ��v�ݒ��ް��H
		ret = OFF;	//	�ڑ��Ȃ�
	}
	else{
		ret = ON;	//	�ڑ�����
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���v�L�^�󎚗v���̔���												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnGoukeiChk( command )								|*/
/*|																			|*/
/*|	PARAMETER		:	ushort command	: �󎚗v�������						|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							YES	: ���v�L�^									|*/
/*|							NO	: ���v�L�^�ȊO								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-08-19													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnGoukeiChk( ushort command )
{
	uchar	ret;	// �߂�l

	if(	command == PREQ_TGOUKEI		||	// �u�s���v�v			�󎚗v��
		command == PREQ_GTGOUKEI	||	// �u�f�s���v�v			�󎚗v��
		command == PREQ_MTGOUKEI	||	// �u�l�s���v�v			�󎚗v��
		command == PREQ_COKINKO_G	||	// �u�R�C�����ɍ��v�v	�󎚗v��
		command == PREQ_SIKINKO_G ){	// �u�������ɍ��v�v		�󎚗v��

		ret = YES;
	}
	else{
		ret = NO;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���v�L�^�̈󎚐����������											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnGoukeiPri( void )								|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							0 		  : �󎚂Ȃ�							|*/
/*|							R_PRI(1)  : ���V�[�g							|*/
/*|							J_PRI(2)  : �W���[�i��							|*/
/*|							RJ_PRI(3) : ���V�[�g���W���[�i��				|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-08-19													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnGoukeiPri( ushort cmd )
{
	uchar	settei;		// �u���v�L�^�̈󎚐�v�ݒ��ް�
	uchar	ret;		// �߂�l�i�󎚐��������ʁj

	settei = 0;
	if( prm_get( COM_PRM, S_TOT, 17, 1, 6 ) == 0){						// ���v�L�^�͑S�ĂP�̈ʂɏ]��
		settei = (uchar)prm_get( COM_PRM, S_TOT, 17, 1, 1 );			// �u�s���v�^�f�s���v�̈󎚐�v�ݒ��ް��擾
	}else{
		//�o�͐�͌ʐݒ�ɏ]��
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		if(( cmd == PREQ_TGOUKEI ) || (cmd == PREQ_GTGOUKEI) || (cmd == PREQ_MTGOUKEI) || (cmd == PREQ_EDY_SHIME_R) ){
		if(( cmd == PREQ_TGOUKEI ) || (cmd == PREQ_GTGOUKEI) || (cmd == PREQ_MTGOUKEI) ){
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
			settei = (uchar)prm_get( COM_PRM, S_TOT, 17, 1, 4 );		// �u�s���v�^�f�s���v�̈󎚐�v�ݒ��ް��擾
		} else if(( cmd == PREQ_COKINKO_G) || (cmd == PREQ_SIKINKO_G)){
			settei = (uchar)prm_get( COM_PRM, S_TOT, 17, 1, 2 );		// �u�������ɍ��v�^�R�C�����ɍ��v�̈󎚐�v�ݒ��ް��擾
		} else if( cmd == PREQ_TURIKAN){
			settei = (uchar)prm_get( COM_PRM, S_TOT, 17, 1, 3 );		// �u���K�Ǘ����v�̈󎚐�v�ݒ��ް��擾
		}
	}
	switch( settei ){	// ���v�L�^�̈󎚐�ݒ�H

		case	0:		// �󎚐恁�Ȃ�

			ret = 0;										// �󎚐恁�Ȃ�
			break;

		case	1:		// �󎚐恁�W���[�i���݈̂�

			if( PrnJnlCheck() == ON ){						// �ެ��ِڑ��H
				// �ެ��ِڑ�����
				ret = J_PRI;								// �󎚐恁�ެ���
			}
			else{
				// �ެ��ِڑ��Ȃ�
				ret = 0;									// �󎚐恁�Ȃ�
			}
			break;

		case	2:		// �󎚐恁���V�[�g�݈̂�

			ret = R_PRI;									// �󎚐恁ڼ��
			break;

		case	3:		// �󎚐恁�W���[�i�������V�[�g

			if( PrnJnlCheck() == ON ){						// �ެ��ِڑ��H
				// �ެ��ِڑ�����
				ret = RJ_PRI;								// �󎚐恁ڼ�ā��ެ���
			}
			else{
				// �ެ��ِڑ��Ȃ�
				ret = R_PRI;								// �󎚐恁ڼ��
			}
			break;

		default:		// ���̑��i�ݒ�G���[�j
			ret = 0;										// �󎚐恁�Ȃ�
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�����W�v�̈󎚐����������											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnAsyuukeiPri( void )								|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							0 		  : �󎚂Ȃ�							|*/
/*|							R_PRI(1)  : ���V�[�g							|*/
/*|							J_PRI(2)  : �W���[�i��							|*/
/*|							RJ_PRI(3) : ���V�[�g���W���[�i��				|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-09-01													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnAsyuukeiPri( void )
{
	uchar	settei;		// �u�����W�v�̈󎚐�v�ݒ��ް�
	uchar	ret;		// �߂�l�i�󎚐��������ʁj

	settei = (uchar)prm_get( COM_PRM, S_TOT, 8, 1, 1 );		// �u�����W�v�̈󎚐�v�ݒ��ް��擾

	switch( settei ){	// �����W�v�̈󎚐�ݒ�H

		case	0:		// �󎚐恁�Ȃ��i�����W�v�Ȃ��j
		case	3:		// �󎚐恁�Ȃ��i�����W�v����j

			ret = 0;										// �󎚐恁�Ȃ�
			break;

		case	1:		// �󎚐恁�W���[�i���݈̂�

			if( PrnJnlCheck() == ON ){						// �ެ��ِڑ��H
				// �ެ��ِڑ�����
				ret = J_PRI;								// �󎚐恁�ެ���
			}
			else{
				// �ެ��ِڑ��Ȃ�
				ret = 0;									// �󎚐恁�Ȃ�
			}
			break;

		case	2:		// �󎚐恁�W���[�i�������V�[�g

			if( PrnJnlCheck() == ON ){						// �ެ��ِڑ��H
				// �ެ��ِڑ�����
				ret = RJ_PRI;								// �󎚐恁ڼ�ā��ެ���
			}
			else{
				// �ެ��ِڑ��Ȃ�
				ret = R_PRI;								// �󎚐恁ڼ��
			}
			break;

		default:		// ���̑��i�ݒ�G���[�j
			ret = 0;										// �󎚐恁�Ȃ�
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�������ԁi�󎚉^�s�j����										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PriStsCheck( pri_kind )								|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	pri_kind	= ��������						|*/
/*|							R_PRI : ڼ��									|*/
/*|							J_PRI : �ެ���									|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							PRI_ERR_NON	 : �󎚉�							|*/
/*|							PRI_ERR_STAT : �󎚕s�i�������Ԉُ�j		|*/
/*|							PRI_ERR_BUSY : �󎚕s�i�����BUSY�j			|*/
/*|							PRI_ERR_ANY  : �󎚕s�i���̑��j				|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-03													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PriStsCheck( uchar pri_kind )
{
	uchar			ret = PRI_ERR_NON;	// �߂�l
	PRN_DATA_BUFF	*prn_buff;			// ���������M�Ǘ��ޯ̧�߲��
	uchar			pri_sts;			// ������ð��

	switch( pri_kind ){		// �������ʁH

		case	R_PRI:		// ڼ��

			prn_buff = &rct_prn_buff;

			break;

		case	J_PRI:		// �ެ���
	
			prn_buff = &jnl_prn_buff;
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�d�q�W���[�i�����ڑ����Ƀ��V�[�g�󎚂ł��Ȃ��j
			if (IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_UNCONNECTED) ||
				IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_COMFAIL)) {
				// �d�q�W���[�i�����ڑ��A�ʐM�s�ǎ��͈󎚕s�Ƃ���
				ret = PRI_ERR_STAT;
			}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�d�q�W���[�i�����ڑ����Ƀ��V�[�g�󎚂ł��Ȃ��j

			break;

		default:			// ���̑��i�������ʴװ�j

			ret = PRI_ERR_ANY;												// �󎚕s�i���̑��j
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
	}
	if( ret == PRI_ERR_NON ){

		//	�������ʁ�BUSY�������n�j�ȏꍇ

		pri_sts = (uchar)(prn_buff->PrnState[0] & 0x0e);					// ��������ݽð���擾�iƱ���޽ð���͏����j

		if( pri_sts ){														// �󎚉\�H
			ret = PRI_ERR_STAT;												// �󎚕s�i�������Ԉُ�j
		}
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�d�q�W���[�i�����ڑ����Ƀ��V�[�g�󎚂ł��Ȃ��j
//// MH810104 GG119201(S) �d�q�W���[�i���Ή�
//		if (IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_WAKEUP_ERR)) {
//			// �d�q�W���[�i���N�����s���͈󎚕s�Ƃ���
//			ret = PRI_ERR_STAT;
//		}
//// MH810104 GG119201(E) �d�q�W���[�i���Ή�
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�d�q�W���[�i�����ڑ����Ƀ��V�[�g�󎚂ł��Ȃ��j
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
		else if (isEJA_USE() && (prn_buff->PrnState[0] & 0x40) != 0) {
			ret = PRI_ERR_WAIT_INIT;						// �󎚕s�i�����������҂��j
		}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		��������M�ް��ޯ̧�������											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PriOutCheck( pri_kind )								|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	pri_kind	= ��������						|*/
/*|							R_PRI : ڼ��									|*/
/*|							J_PRI : �ެ���									|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : �ҏW�J�nOK									|*/
/*|							NG : �ҏW�J�nNG									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-10													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PriOutCheck( uchar pri_kind )
{
	ushort	WriteCnt;		// ���M�ޯ̧�o�^��
	ushort	ReadCnt;		// ���M�ޯ̧�Ǎ���
	ushort	len;			// ���M�ޯ̧���󂫐�

	switch(pri_kind){
		case	R_PRI:	// ڼ��
			WriteCnt = rct_prn_buff.PrnBufWriteCnt;
			ReadCnt  = rct_prn_buff.PrnBufReadCnt;
			break;

		case	J_PRI:	// �ެ���
			WriteCnt = jnl_prn_buff.PrnBufWriteCnt;
			ReadCnt  = jnl_prn_buff.PrnBufReadCnt;
			break;

		default:		// ���̑��i���Ұ�NG�j
			return( NG );
	}

	if( WriteCnt == PRNQUE_CNT && ReadCnt == 0 ){
		return( OK );
	}

	if( WriteCnt >= ReadCnt )
	{
		len =  ( PRNQUE_CNT - WriteCnt ) + ReadCnt;
		if( len >= 32 ){
			return( OK );	//OK
		}
		else{
			return( NG );	//NG
		}
	}
	else
	{
		len =  ReadCnt - WriteCnt;
		if( len >= 32 ){
			return( OK );	//OK
		}
		else{
			return( NG ); 	//NG
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		������ւ̺���ޕҏW����												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnCmdLen( *dat, len, pri_kind )					|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	*data	= ������ް��߲��					|*/
/*|						ushort	len		= ������ް��ݸ޽					|*/
/*|						uchar	pri_kind=	��������						|*/
/*|							R_PRI  : ڼ��									|*/
/*|							J_PRI  : �ެ���									|*/
/*|							RJ_PRI : ڼ�ā��ެ���							|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnCmdLen( const uchar *dat, ushort len, uchar pri_kind )
{
	switch( pri_kind ){		// �������ʁH

		case	R_PRI:		// ڼ��
			PrnCmdLen_R( dat, len );	// ������ֺ���ޕҏW�iڼ�āj
			break;

		case	J_PRI:		// �ެ���
			PrnCmdLen_J( dat, len );	// ������ֺ���ޕҏW�i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���
			PrnCmdLen_R( dat, len );	// ������ֺ���ޕҏW�iڼ�āj
			PrnCmdLen_J( dat, len );	// ������ֺ���ޕҏW�i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| ���V�[�g�v�����^���M����                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : RP_I2CSndReq                                            |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : 								                           |*/
/*| Date         : 				                                           |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RP_I2CSndReq( int type )
{
	ushort			exec = 0;
	I2C_REQUEST 	request;
	MsgBuf 			*Msg;
// GG129000(S) H.Fujinaga 2022/12/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
	ushort len;
	ushort useBuf;		//�g�p�o�b�t�@��
// GG129000(E) H.Fujinaga 2022/12/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j

	switch( type ){
		case	0:
			if( !rct_prn_buff.PrnBufCnt ){
				if( rct_prn_buff.PrnBufWriteCnt != rct_prn_buff.PrnBufReadCnt ){
					exec = 1;
				}
			}
			break;
		case	1:
			rct_prn_buff.PrnBufCnt = 0;
// GG129000(S) H.Fujinaga 2022/12/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
//			rct_prn_buff.PrnBufReadCnt++;
			len = rct_prn_buff.PrnBufCntLen[rct_prn_buff.PrnBufReadCnt];
			//�g�p�����o�b�t�@��(���{��)�Z�o
			useBuf = len / PRNBUF_SIZE;
			if( len % PRNBUF_SIZE ){
				useBuf++;		//�؂�グ
			}
			rct_prn_buff.PrnBufReadCnt += useBuf;
// GG129000(E) H.Fujinaga 2022/12/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
			Lagcan( PRNTCBNO, 1 );
			if( rct_prn_buff.PrnBufReadCnt >= PRNQUE_CNT ){
				rct_prn_buff.PrnBufReadCnt = 0;
			}
			if( (rct_prn_buff.PrnBufWriteCnt != rct_prn_buff.PrnBufReadCnt) && !(rct_prn_buff.PrnState[0] & 0x0e)){
				exec = 1;
			}else{
				rct_prn_buff.PrnBufWriteCnt = 0;
				rct_prn_buff.PrnBufReadCnt = 0;
				if( rct_init_sts == 0 ){							// �����������҂��H
					if( (Msg = GetBuf()) != NULL ){
						Msg->msg.command = PREQ_INIT_END_R;			// ����������ޏI���ʒm���M
						PutMsg( OPETCBNO , Msg );
					}
				}else{
					MsgSndFrmPrn(	PREQ_INNJI_END,					// �I���R�}���h���M
									R_PRI,
									rct_prn_buff.PrnState[0] );
				}
			}
			break;
		case	2:
			if( !rct_prn_buff.PrnBufCnt ){
				MsgSndFrmPrn(	PREQ_INNJI_END,						// �I���R�}���h���M
								R_PRI,
								rct_prn_buff.PrnState[0] );
			}
			break;
	}
	
	if( exec && !jnl_prn_buff.PrnBufCnt ){
		_ei();
		request.TaskNoTo	 			= I2C_TASK_PRN;					// 
		request.DeviceNo 				= I2C_DN_RP;					// RP�ւ̗v��
		request.RequestCode    			= I2C_RC_RP_SND;				// 
		request.I2cReqDataInfo.RWCnt	= rct_prn_buff.PrnBufCntLen[rct_prn_buff.PrnBufReadCnt];	//
		request.I2cReqDataInfo.pRWData	= rct_prn_buff.PrnBuf[rct_prn_buff.PrnBufReadCnt];			//
		rct_prn_buff.PrnBusyCnt 		= 0;							// ���M�҂��J�E���g������
		rct_prn_buff.PrnBufCnt 			= 1;							// ���M���̃t���O�Ƃ��Ďg�p
		Lagtim( PRNTCBNO, 1, 50 );										// ����̑҂��͂P�b
		I2C_Request( &request, EXE_MODE_QUEUE );						// 
		rct_timer_end = 1;
		Lagtim( PRNTCBNO, 7, 50*2 );									// ���V�[�g�v�����^�󎚊�����^�C�}�[�Z�b�g(�Q�b)
	}
	// �Ƃ肠����NG�ł��������Ȃ�
}

/*[]----------------------------------------------------------------------[]*/
/*| �W���[�i���v�����^���M����                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : JP_I2CSndReq                                            |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : 								                           |*/
/*| Date         : 				                                           |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	JP_I2CSndReq( int type )
{
	ushort					exec = 0;
	I2C_REQUEST 			request;
	MsgBuf *Msg;
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
	ushort	rp;
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

	switch( type ){
		case	0:
			if( !jnl_prn_buff.PrnBufCnt ){
				if( jnl_prn_buff.PrnBufWriteCnt != jnl_prn_buff.PrnBufReadCnt ){
					exec = 1;
				}
			}
			break;
		case	1:
			jnl_prn_buff.PrnBufCnt = 0;
			jnl_prn_buff.PrnBufReadCnt++;
			Lagcan( PRNTCBNO, 2 );
			if( jnl_prn_buff.PrnBufReadCnt >= PRNQUE_CNT ){
				jnl_prn_buff.PrnBufReadCnt = 0;
			}
			if( (jnl_prn_buff.PrnBufWriteCnt != jnl_prn_buff.PrnBufReadCnt ) && !(jnl_prn_buff.PrnState[0] & 0x0e)){
				exec = 1;
			}else{
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
				// ���[�h�|�C���^
				rp = jnl_prn_buff.PrnBufReadCnt;
				if (rp == 0) {
					rp = PRNQUE_CNT - 1;
				}
				else {
					rp--;
				}
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�f�[�^�����݊�����҂����Ƀf�[�^���M���Ă��܂��j
//				if (isEJA_USE()) {
//					jnl_prn_buff.PrnBufWriteCnt = 0;
//					jnl_prn_buff.PrnBufReadCnt = 0;
//					if (!PrnCmd_CheckSendData(&jnl_prn_buff.PrnBuf[rp][0])) {
//						// �d�q�W���[�i���p���M�f�[�^�͈󎚏I����ʒm���Ȃ�
//						if (jnl_init_sts != 0) {							// �����������҂��ȊO
//							MsgSndFrmPrn(	PREQ_INNJI_END,					// �I���R�}���h���M
//											J_PRI,
//											jnl_prn_buff.PrnState[0] );
//						}
//					}
//				}
//				else {
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�f�[�^�����݊�����҂����Ƀf�[�^���M���Ă��܂��j
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
				jnl_prn_buff.PrnBufWriteCnt = 0;
				jnl_prn_buff.PrnBufReadCnt = 0;
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�f�[�^�����݊�����҂����Ƀf�[�^���M���Ă��܂��j
				if (isEJA_USE()) {
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
//					// �d�q�W���[�i���ڑ����͈󎚃f�[�^���M�����C�x���g��ʒm���Ȃ�
//					// �f�[�^�����݊�����҂�
					if (jnl_proc_data.Final != 0) {
						// �d�q�W���[�i���̏ꍇ�͍ŏI�u���b�N�̈󎚏I���ʒm�͑��M���Ȃ�
						// EJA��̃f�[�^�����݊�����҂�
						break;
					}
					if (PrnCmd_CheckSendData(&jnl_prn_buff.PrnBuf[rp][0])) {
						// �d�q�W���[�i���p�f�[�^�i�@����v�����j�̑��M��
						// �󎚏I���ʒm�𑗐M���Ȃ�
						break;
					}
					MsgSndFrmPrn(	PREQ_INNJI_END,					// �I���R�}���h���M
									J_PRI,
									jnl_prn_buff.PrnState[0] );
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
					break;
				}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�f�[�^�����݊�����҂����Ƀf�[�^���M���Ă��܂��j
				if( jnl_init_sts == 0 ){							// �����������҂��H
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
					if (!PrnCmd_CheckSendData(&jnl_prn_buff.PrnBuf[rp][0])) {
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
					if( (Msg = GetBuf()) != NULL ){
						Msg->msg.command = PREQ_INIT_END_J;			// ����������ޏI���ʒm���M
						PutMsg( OPETCBNO , Msg );
					}
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
					}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
				}else{
					MsgSndFrmPrn(	PREQ_INNJI_END,					// �I���R�}���h���M
									J_PRI,
									jnl_prn_buff.PrnState[0] );
				}
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�f�[�^�����݊�����҂����Ƀf�[�^���M���Ă��܂��j
//// MH810104 GG119201(S) �d�q�W���[�i���Ή�
//				}
//// MH810104 GG119201(E) �d�q�W���[�i���Ή�
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�f�[�^�����݊�����҂����Ƀf�[�^���M���Ă��܂��j
			}
			break;
		case	2:
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
			if( jnl_init_sts == 0 ){								// �����������҂��H
				break;
			}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
			if (isEJA_USE()) {
				// �d�q�W���[�i���ڑ�����I2C_NEXT_SND_REQ�i�󎚊����j����
				// PREQ_INNJI_END�̂ݒʒm����
				break;
			}
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
			if( !jnl_prn_buff.PrnBufCnt ){
				MsgSndFrmPrn(	PREQ_INNJI_END,						// �I���R�}���h���M
								J_PRI,
								jnl_prn_buff.PrnState[0] );
			}
			break;
	}
	if( exec && !rct_prn_buff.PrnBufCnt ){
		_ei();
		request.TaskNoTo	 			= I2C_TASK_PRN;					// 
		request.DeviceNo 				= I2C_DN_JP;					// JP�ւ̗v��
		request.RequestCode    			= I2C_RC_JP_SND;				// 
		request.I2cReqDataInfo.RWCnt	= jnl_prn_buff.PrnBufCntLen[jnl_prn_buff.PrnBufReadCnt];					// 
		request.I2cReqDataInfo.pRWData	= jnl_prn_buff.PrnBuf[jnl_prn_buff.PrnBufReadCnt];		// 
		jnl_prn_buff.PrnBusyCnt = 0;									// ���M���̃t���O�Ƃ��Ďg�p
		jnl_prn_buff.PrnBufCnt = 1;										// ���M�҂��J�E���g������
		Lagtim( PRNTCBNO, 2, 50 );										// ����̑҂��͂P�b
		I2C_Request( &request, EXE_MODE_QUEUE );						// 
	}
	// �Ƃ肠����NG�ł��������Ȃ�
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ڼ��������ւ̺���ޕҏW����											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnCmdLen_R( *dat , len )							|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	*data	= ������ް��߲��					|*/
/*|						ushort	len		= ������ް��ݸ޽					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnCmdLen_R(const uchar *dat , ushort len)
{
// GG129000(S) H.Fujinaga 2022/12/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
	int useBuf;		//�g�p�o�b�t�@��
// GG129000(E) H.Fujinaga 2022/12/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
// GG129000(S) H.Fujinaga 2022/12/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
//	memcpy(	rct_prn_buff.PrnBuf[rct_prn_buff.PrnBufWriteCnt],				// ���M�ް��ޯ̧�ɑ��M�ް���߰
//			dat,
//			(size_t)len );
//	rct_prn_buff.PrnBufCntLen[rct_prn_buff.PrnBufWriteCnt] = len;			// ���M�ް��ݸ޽���
//	rct_prn_buff.PrnBufWriteCnt++;											// ���M�ް��ޯ̧�o�^���{�P
//	
//	if( rct_prn_buff.PrnBufWriteCnt >= PRNQUE_CNT ){	// ���M�ް��ޯ̧�󂫂���H
//
//		// �󂫂Ȃ��̏ꍇ�A�װ�������s��
//
//		rct_prn_buff.PrnBufWriteCnt = 0;
//		Prn_errlg( ERR_PRNT_BUFF_FULL, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// ���M�ޯ̧���ް�۰�װ�o�^
//	}
	//�g�p����o�b�t�@��(���{��)�Z�o
	useBuf = len / PRNBUF_SIZE;
	if( len % PRNBUF_SIZE ){
		useBuf++;		//�؂�グ
	}
	
	if( ( rct_prn_buff.PrnBufWriteCnt + useBuf ) > PRNQUE_CNT ){	// ���M�ް��ޯ̧�󂫂���H

		// �󂫂Ȃ��̏ꍇ�A�װ�������s��
		rct_prn_buff.PrnBufWriteCnt = 0;

		Prn_errlg( ERR_PRNT_BUFF_FULL, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// ���M�ޯ̧���ް�۰�װ�o�^
	}
	memcpy(	rct_prn_buff.PrnBuf[rct_prn_buff.PrnBufWriteCnt],				// ���M�ް��ޯ̧�ɑ��M�ް���߰
			dat,
			(size_t)len );
	rct_prn_buff.PrnBufCntLen[rct_prn_buff.PrnBufWriteCnt] = len;			// ���M�ް��ݸ޽���
	rct_prn_buff.PrnBufWriteCnt += useBuf;						// ���M�ް��ޯ̧�o�^���{�g�p�o�b�t�@��
// GG129000(E) H.Fujinaga 2022/12/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
	RP_I2CSndReq( I2C_PRI_REQ );

}

/*[]-----------------------------------------------------------------------[]*/
/*|		�ެ���������ւ̺���ޕҏW����										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnCmdLen_J( *dat , len )							|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	*data	= ������ް��߲��					|*/
/*|						ushort	len		= ������ް��ݸ޽					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnCmdLen_J(const uchar *dat , ushort len)
{
	if( PrnJnlCheck() != ON ){		// �ެ���������ڑ�����H
		return;						// �ڑ��Ȃ�
	}
	memcpy(	jnl_prn_buff.PrnBuf[jnl_prn_buff.PrnBufWriteCnt],				// ���M�ް��ޯ̧�ɑ��M�ް���߰
			dat,
			(size_t)len );
	jnl_prn_buff.PrnBufCntLen[jnl_prn_buff.PrnBufWriteCnt] = len;			// ���M�ް��ݸ޽���
	jnl_prn_buff.PrnBufWriteCnt++;											// ���M�ް��ޯ̧�o�^���{�P
	if( jnl_prn_buff.PrnBufWriteCnt >= PRNQUE_CNT ){	// ���M�ް��ޯ̧�󂫂���H

		// �󂫂Ȃ��̏ꍇ�A�װ�������s��

		jnl_prn_buff.PrnBufWriteCnt = 0;
		Prn_errlg( ERR_PRNT_BUFF_FULL, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// ���M�ޯ̧���ް�۰�װ�o�^
	}
	JP_I2CSndReq( I2C_PRI_REQ );
	
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ڼ��������ւ̕������ް��ҏW����										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnCmd_R( *dat )									|*/
/*|																			|*/
/*|	PARAMETER		:	char	*data	= �������ް��߲��					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnCmd_R( char *dat )
{
	ushort len;

	len = (ushort)strlen(dat);

	memcpy(	rct_prn_buff.PrnBuf[rct_prn_buff.PrnBufWriteCnt],				// ���M�ް��ޯ̧�ɑ��M�ް���߰
			dat,
			(size_t)len );

	rct_prn_buff.PrnBufCntLen[rct_prn_buff.PrnBufWriteCnt]=len;				// ���M�ް��ݸ޽���
	rct_prn_buff.PrnBufWriteCnt++;											// ���M�ް��ޯ̧�o�^���{�P
	
	if( rct_prn_buff.PrnBufWriteCnt >= PRNQUE_CNT ){	// ���M�ް��ޯ̧�󂫂���H

		// �󂫂Ȃ��̏ꍇ�A�װ�������s��

		rct_prn_buff.PrnBufWriteCnt = 0;
		Prn_errlg( ERR_PRNT_BUFF_FULL, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// ���M�ޯ̧���ް�۰�װ�o�^
	}
	RP_I2CSndReq( I2C_PRI_REQ );
	
}
/*[]-----------------------------------------------------------------------[]*/
/*|		�ެ���������ւ̕������ް��ҏW����									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnCmd_J( *dat )									|*/
/*|																			|*/
/*|	PARAMETER		:	char	*data	= �������ް��߲��					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnCmd_J( char *dat )
{
	ushort len;
	
	len = (ushort)strlen(dat);

	if( PrnJnlCheck() != ON ){		// �ެ���������ڑ�����H
		return;						// �ڑ��Ȃ�
	}

	memcpy(	jnl_prn_buff.PrnBuf[jnl_prn_buff.PrnBufWriteCnt],				// ���M�ް��ޯ̧�ɑ��M�ް���߰
			dat,
			(size_t)len );

	jnl_prn_buff.PrnBufCntLen[jnl_prn_buff.PrnBufWriteCnt]=len;				// ���M�ް��ݸ޽���
	jnl_prn_buff.PrnBufWriteCnt++;											// ���M�ް��ޯ̧�o�^���{�P
	if( jnl_prn_buff.PrnBufWriteCnt >= PRNQUE_CNT ){	// ���M�ް��ޯ̧�󂫂���H

		// �󂫂Ȃ��̏ꍇ�A�װ�������s��

		jnl_prn_buff.PrnBufWriteCnt = 0;
		Prn_errlg( ERR_PRNT_BUFF_FULL, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// ���M�ޯ̧���ް�۰�װ�o�^
	}
	JP_I2CSndReq( I2C_PRI_REQ );
	
}

/*[]-----------------------------------------------------------------------[]*/
/*|		������ւ̂P�s���������ް��ҏW����									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnStr( *dat, pri_kind )							|*/
/*|																			|*/
/*|	PARAMETER		:	char	*data	= �������ް��߲��					|*/
/*|						uchar	pri_kind=	��������						|*/
/*|							R_PRI  : ڼ��									|*/
/*|							J_PRI  : �ެ���									|*/
/*|							RJ_PRI : ڼ�ā��ެ���							|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnStr(const char *dat, uchar pri_kind )
{
	switch( pri_kind ){		// �������ʁH

		case	R_PRI:		// ڼ��
			PrnStr_R( dat );	// ������ւP�s���������ް��ҏW�iڼ�āj
			break;

		case	J_PRI:		// �ެ���
			PrnStr_J( dat );	// ������ւP�s���������ް��ҏW�i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���
			PrnStr_R( dat );	// ������ւP�s���������ް��ҏW�iڼ�āj
			PrnStr_J( dat );	// ������ւP�s���������ް��ҏW�i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ڼ��������ւ̂P�s���������ް��ҏW����								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnStr_R( *dat )									|*/
/*|																			|*/
/*|	PARAMETER		:	char	*data	= �������ް��߲��					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnStr_R(const char *dat )
{

	ushort len;
	
	len = (ushort)strlen(dat);

	memcpy(	rct_prn_buff.PrnBuf[rct_prn_buff.PrnBufWriteCnt],				// ���M�ް��ޯ̧�ɑ��M�ް���߰
			dat,
			(size_t)len );

	rct_prn_buff.PrnBuf[rct_prn_buff.PrnBufWriteCnt][len] = LF;				// ���s�ް����
	rct_prn_buff.PrnBufCntLen[rct_prn_buff.PrnBufWriteCnt]=len+1;			// ���M�ް��ݸ޽���
	rct_prn_buff.PrnBufWriteCnt++;											// ���M�ް��ޯ̧�o�^���{�P

	if( rct_prn_buff.PrnBufWriteCnt >= PRNQUE_CNT ){	// ���M�ް��ޯ̧�󂫂���H

		// �󂫂Ȃ��̏ꍇ�A�װ�������s��

		rct_prn_buff.PrnBufWriteCnt = 0;
		Prn_errlg( ERR_PRNT_BUFF_FULL, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// ���M�ޯ̧���ް�۰�װ�o�^
	}
	inc_dct( MOV_PRNT_CNT, 1 );												// �uڼ��������󎚍s���v���춳��
	GyouCnt_r++;															// ���M�ς݈��ް��s���iڼ�āj�{�P

	RP_I2CSndReq( I2C_PRI_REQ );

}

/*[]-----------------------------------------------------------------------[]*/
/*|		ڼ��������ւ̓��ʓ��p�������ް��ҏW����								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnStr_SPDAY( *dat )								|*/
/*|																			|*/
/*|	PARAMETER		:	char	*data	: �������ް��߲��					|*/
/*|						uchar	pos		: ���s�׸�							|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	T.Namioka													|*/
/*|	Date	:	2006-03-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
void	PrnStr_SPDAY(char *dat ,uchar pos)
{

	ushort len;
	
	len = (ushort)strlen(dat);

	memcpy(	rct_prn_buff.PrnBuf[rct_prn_buff.PrnBufWriteCnt],				// ���M�ް��ޯ̧�ɑ��M�ް���߰
			dat,
			(size_t)len );

	rct_prn_buff.PrnBufCntLen[rct_prn_buff.PrnBufWriteCnt]=len;				// ���M�ް��ݸ޽���
	rct_prn_buff.PrnBufWriteCnt++;											// ���M�ް��ޯ̧�o�^���{�P
	if( rct_prn_buff.PrnBufWriteCnt >= PRNQUE_CNT ){						// ���M�ް��ޯ̧�󂫂���H

		// �󂫂Ȃ��̏ꍇ�A�װ�������s��

		rct_prn_buff.PrnBufWriteCnt = 0;
		Prn_errlg( ERR_PRNT_BUFF_FULL, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// ���M�ޯ̧���ް�۰�װ�o�^
	}

	if(pos){
		rct_prn_buff.PrnBuf[rct_prn_buff.PrnBufWriteCnt][len] = LF;			// ���s�ް����
		inc_dct( MOV_PRNT_CNT, 1 );											// �uڼ��������󎚍s���v���춳��
		GyouCnt_r++;														// ���M�ς݈��ް��s���iڼ�āj�{�P
	}
	RP_I2CSndReq( I2C_PRI_REQ );

}

/*[]-----------------------------------------------------------------------[]*/
/*|		�ެ���������ւ̂P�s���������ް��ҏW����								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnStr_J( *dat )									|*/
/*|																			|*/
/*|	PARAMETER		:	char	*data	= �������ް��߲��					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnStr_J(const char *dat )
{

	ushort len;
	
	len = (ushort)strlen(dat);

	if( PrnJnlCheck() != ON ){		// �ެ���������ڑ�����H
		return;						// �ڑ��Ȃ�
	}

	memcpy(	jnl_prn_buff.PrnBuf[jnl_prn_buff.PrnBufWriteCnt],				// ���M�ް��ޯ̧�ɑ��M�ް���߰
			dat,
			(size_t)len );

	jnl_prn_buff.PrnBuf[jnl_prn_buff.PrnBufWriteCnt][len] = LF;				// ���s�ް����
	jnl_prn_buff.PrnBufCntLen[jnl_prn_buff.PrnBufWriteCnt]=len+1;			// ���M�ް��ݸ޽���
	jnl_prn_buff.PrnBufWriteCnt++;											// ���M�ް��ޯ̧�o�^���{�P
	if( jnl_prn_buff.PrnBufWriteCnt >= PRNQUE_CNT ){	// ���M�ް��ޯ̧�󂫂���H

		// �󂫂Ȃ��̏ꍇ�A�װ�������s��

		jnl_prn_buff.PrnBufWriteCnt = 0;
		Prn_errlg( ERR_PRNT_BUFF_FULL, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// ���M�ޯ̧���ް�۰�װ�o�^
	}

// MH810104 GG119201(S) �d�q�W���[�i���Ή�
	if (!isEJA_USE()) {
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
	inc_dct( MOV_JPNT_CNT, 1 );												// �u�ެ���������󎚍s���v���춳��
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
	}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
	GyouCnt_j++;															// ���M�ς݈��ް��s���i�ެ��فj�{�P
	JP_I2CSndReq( I2C_PRI_REQ );
	
}

/*[]-----------------------------------------------------------------------[]*/
/*|		������N������														|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnOut( pri_kind )									|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	pri_kind	= ��������						|*/
/*|							R_PRI : ڼ��									|*/
/*|							J_PRI : �ެ���									|*/
/*|							RJ_PRI : ڼ�ā��ެ���							|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnOut( uchar pri_kind )
{
	switch( pri_kind ){		// �������ʁH

		case	R_PRI:		// ڼ��
			PrnOut_R();		// ������N���iڼ�āj
			break;

		case	J_PRI:		// �ެ���
			PrnOut_J();		// ������N���i�ެ��فj
			break;

		case	RJ_PRI:		// ڼ�ā��ެ���
			PrnOut_R();		// ������N���iڼ�āj
			PrnOut_J();		// ������N���i�ެ��فj
			break;

		default:			// ���̑��i�������ʴװ�j
			return;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ڼ��������N������													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnOut_R( void )									|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnOut_R( void )
{
	if(	f_Prn_R_SendTopChar == 1){							// ڼ��������󎚗v����A�擪��1������
		(void)Printer_Lifter_Wait(0);						// ���M�O V24�nUnit �N���r���i�K�v����΂��̊֐����ő҂j
															// �r�����䌠�擾
		JR_Print_Wait_tim[0] = 0;							// ��V24Unit�����ԋN���֎~�i������ϰ���āj
		f_Prn_R_SendTopChar = 0;							// �擪1��׸����M�v���ς�
	}
	rct_prn_buff.PrnBusyCnt = 0;							// ������޼ް�p���Ď������ر
	RP_I2CSndReq( I2C_EVENT_QUE_REQ );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�ެ���������N������													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnOut_J( void )									|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnOut_J( void )
{
	if(	f_Prn_J_SendTopChar == 1){							// �ެ���������󎚗v����A�擪��1������
		(void)Printer_Lifter_Wait(1);						// ���M�O V24�nUnit �N���r���i�K�v����΂��̊֐����ő҂j
															// �r�����䌠�擾
		JR_Print_Wait_tim[1] = 0;							// ��V24Unit�����ԋN���֎~�i������ϰ���āj
		f_Prn_J_SendTopChar = 0;							// �擪1��׸����M�v���ς�
	}
	jnl_prn_buff.PrnBusyCnt = 0;							// ������޼ް�p���Ď������ر
	JP_I2CSndReq( I2C_EVENT_QUE_REQ );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ڼ���������~����													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnStop_R( void )									|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnStop_R( void )
{
	rct_prn_buff.PrnBufWriteCnt	= 0;
	rct_prn_buff.PrnBufReadCnt	= 0;
	rct_prn_buff.PrnBufCnt		= 0;
	if( rct_proc_data.Printing != 0 ){
		err_chk( ERRMDL_PRINTER, ERR_PRNT_R_PRINTCOM, 1, 0, 0 );// ڼ��������ʐM�ُ�o�^
		PrnEndMsg(							/*-----	�󎚏I��ү���ޑ��M	-----*/
					rct_proc_data.Printing,	// �󎚗v������ށF�������Ұ�
					PRI_ERR_END,			// �󎚏�������	�F�ُ�I��
					PRI_ERR_STAT,			// �ُ�I�����R	�F�������Ұ�
					R_PRI					// ��������	�Fڼ��
				);
		JP_I2CSndReq( I2C_PRI_REQ );		/* �W���[�i���ɑ��M����f�[�^������Α��M���� */
	}
	prn_proc_data_clr_R();
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�ެ����������~����													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnStop_J( void )									|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-22													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnStop_J( void )
{
	jnl_prn_buff.PrnBufWriteCnt	= 0;
	jnl_prn_buff.PrnBufReadCnt	= 0;
	jnl_prn_buff.PrnBufCnt		= 0;
	if( jnl_proc_data.Printing != 0 ){
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�W���[�i���v�����^�ʐM�s�ǂ��������Ă��܂��j
		if (!isEJA_USE()) {
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�W���[�i���v�����^�ʐM�s�ǂ��������Ă��܂��j
		err_chk( ERRMDL_PRINTER, ERR_PRNT_J_PRINTCOM, 1, 0, 0 );// ڼ��������ʐM�ُ�o�^
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�W���[�i���v�����^�ʐM�s�ǂ��������Ă��܂��j
		}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�W���[�i���v�����^�ʐM�s�ǂ��������Ă��܂��j
		PrnEndMsg(							/*-----	�󎚏I��ү���ޑ��M	-----*/
					jnl_proc_data.Printing,	// �󎚗v������ށF�������Ұ�
					PRI_ERR_END,			// �󎚏�������	�F�ُ�I��
					PRI_ERR_STAT,			// �ُ�I�����R	�F�������Ұ�
					J_PRI					// ��������	�F�ެ���
				);
		RP_I2CSndReq( I2C_PRI_REQ );		/* �W���[�i���ɑ��M����f�[�^������Α��M���� */
	}
	prn_proc_data_clr_J();
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ڼ�Đ擪���ް��ҏW�����iۺވ󎚁^ͯ�ް�󎚁^�p����Đ���j			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Rct_top_edit( cut )									|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	cut	=	�p����Đ���L��						|*/
/*|							OFF	:�p����Ă��Ȃ�								|*/
/*|							ON	:�p����Ă���								|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-01													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	Rct_top_edit( uchar cut )
{
	uchar	settei;			// �uڼ�Đ擪�󎚁v�ݒ��ް�
	uchar	hight_size;		// �uڼ�Đ擪���ް��v�̏c���ށi�ޯĐ��j


	Feed_set( R_PRI, 32 );	// ڼ��ͯ�ް�㕔�̋󂫕������i������4mm�j

	//	ڼ��ͯ�ް�󎚏���
	settei = (uchar)CPrmSS[S_PRN][3];	// �uڼ�Ķ�Ď��̈󎚁v�ݒ��ް��擾

	switch( settei ){	// �󎚐ݒ�ɂ��e�����֕���

		case	0:		// �󎚂Ȃ�
			// 0x4c��������(0x4c=(LOGO_HDOT_SIZE(0x90)-0x44)
			// �����肵�Ȃ��ƑO�̈󎚂̓r���ŃJ�b�g���Ă��܂�
			hight_size = 0x44;
			break;

		case	1:		// ۺވ�
			Prn_edit_logo( R_PRI );									// ۺވ��ް��ҏW
			hight_size = LOGO_HDOT_SIZE;
			break;

		case	2:		// ͯ�ް��
			Prn_edit_headhoot( PRI_HEADER, R_PRI );					// ͯ�ް���ް��ҏW
			hight_size = header_hight;								// ͯ�ް���ް��c���ށi�ޯĐ��j�擾
			break;
		case	3:		// ۺށ{ͯ�ް
			Prn_edit_logo( R_PRI );									// ۺވ��ް��ҏW
			hight_size = LOGO_HDOT_SIZE;
			break;

		default:		// ���̑�
			hight_size = 0;
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
	}
	if( (LOGO_HDOT_SIZE - hight_size) > 0 ){

		//	�󎚻��ށi�c�j��ۺނ̻��ށi�c�j�ɖ����Ȃ��ꍇ

		Feed_set( R_PRI, (uchar)(LOGO_HDOT_SIZE - hight_size) );	// �s����������
	}

	if( cut == ON ){	// �p����Đ��䂠��H

		// �p����Ă���ꍇ
		// �h�b�g�w��̃o�b�N�t�B�[�h�Ή��܂ł̉��Ώ�
		// �Ή����
		// 		settei = 1,3	��  9.5mm(76 dot)
		// 		settei = 2		hight_size > LOGO_HDOT_SIZE�̏ꍇ
		//							�� (hight_size-LOGO_HDOT_SIZE(144))+9.5mm(76 dot)
		//						hight_size <= LOGO_HDOT_SIZE�̏ꍇ
		//							��  9.5mm(76 dot)
		// �̃o�b�N�t�B�[�h���K�v
		if(( settei == 1 ) || ( settei == 3 )){
// NOTE:���̎��_�Ń��S�󎚂��Ă���̂Ő�[��18mm�i�ށA�󎚃w�b�h����J�b�^�[�̈ʒu�܂ł�
// 9.5�}1mm�Ȃ̂�18-9.5-1=9.5mm�̃o�b�N�t�B�[�h���K�v�ƂȂ�
			if( OPE_SIG_DOOR_Is_OPEN == 0 ){		// �h�A��?
				PCMD_BS_CUT( R_PRI );				// 9.5mm�ޯ�̨��ނ���������p�����
				f_partial_cut = 0;					// �p�[�V�����J�b�g������s�t���O�N���A
			}else{
				PCMD_BS_PCUT( R_PRI );				// 9.5mm�ޯ�̨��ނ���������p���߰��ٶ��
				f_partial_cut = 1;					// �p�[�V�����J�b�g������s�t���O�Z�b�g
			}
		}else if( settei == 2 ){
			if(hight_size > LOGO_HDOT_SIZE){
				Back_Feed_set( R_PRI, (uchar)(hight_size-LOGO_HDOT_SIZE+0x4c) );
			}
			if( OPE_SIG_DOOR_Is_OPEN == 0 ){		// �h�A��?
				PCMD_BS_CUT( R_PRI );				// 9.5mm�ޯ�̨��ނ���������p�����
				f_partial_cut = 0;					// �p�[�V�����J�b�g������s�t���O�N���A
			}else{
				PCMD_BS_PCUT( R_PRI );				// 9.5mm�ޯ�̨��ނ���������p���߰��ٶ��
				f_partial_cut = 1;					// �p�[�V�����J�b�g������s�t���O�Z�b�g
			}
		}else{
	if( OPE_SIG_DOOR_Is_OPEN == 0 ){								// �h�A��?
		PCMD_CUT( R_PRI );							// �p����ĺ���ޕҏW
		f_partial_cut = 0;							// �p�[�V�����J�b�g������s�t���O�N���A
	} else{
		PCMD_PCUT( R_PRI );							// �p���߰��ٶ�ĺ���ޕҏW
		f_partial_cut = 1;							// �p�[�V�����J�b�g������s�t���O�Z�b�g
	}
		}
		inc_dct( MOV_CUT_CNT, 1 );					// ��ڼ�ėp����ā�����񐔶���
		if( settei != 3 ){
			if( settei == 0 ){						// ۺށEͯ�ވ󎚂Ȃ�
			// ���S�ƃw�b�_�̍��v�T�C�Y��18mm���g�[�^��18mm����K�v������
			// Feed_set()��9.5mm�t�B�[�h�ƃJ�b�g�ɂ�莩���I��2mm�t�B�[�h�c��6.5mm���t�B�[�h����
				Feed_set( R_PRI, RCUT_END_FEED_NP );// ������iۺށEͯ�ވ󎚂Ȃ����̶�Č�Ɏ����肷�黲�ޕ��j
			}else if( settei == 2 ){				// ͯ�ވ󎚂̂�
			// �w�b�_�󎚎��̎c��t�B�[�h�͉��s�ɂ��3.5mm(Prn_LogoHead_PostHead ()���̉��s1�s��)
				PrnCmdLen( (uchar *)"\x0a" , 1 , R_PRI );// [���s]x1
			}
		}else{
			//ۺ�+ͯ�ް�̏ꍇ�͎�����ł͂Ȃ��A�w�b�_�[�������Q�s���i�c�{�Ȃǂ͂P)�󎚂���.
			//�c��̍s�͎���A�󎚂̑O�ɂ���
			Prn_LogoHead_PreHead ( );
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���s�ް��ҏW�����i���s�ʁ��ݒ��ް��j								|*/
/*|																			|*/
/*|		�����l��															|*/
/*|			���s����������ʂ̍����{�ݒ��ް��Őݒ肳��Ă�����s��			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Kaigyou_set( pri_kind, font, cnt )					|*/
/*|																			|*/
/*|	PARAMETER		:	uchar		pri_kind	=	�󎚐������				|*/
/*|							R_PRI  : ڼ��									|*/
/*|							J_PRI  : �ެ���									|*/
/*|																			|*/
/*|						uchar		font		=	�������				|*/
/*|																			|*/
/*|						uchar		cnt			=	���s��					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-04													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	Kaigyou_set( uchar pri_kind, uchar font, uchar cnt )
{
	uchar	cmd[PRNBUF_SIZE];	// ���s�����ް��ҏWܰ�
	uchar	i;


	if( (cnt > 0) && (cnt <= PRNBUF_SIZE) ){		// �w����s������

		PrnFontSize( font, pri_kind );				// [��Ӱ�ގw��]��[���s�ʁi���j�w��]

		for( i = 0 ; i < cnt ; i++ ){				// �v�����ꂽ�����̉��s�ް��쐬
			cmd[i] = 0x0a;
		}
		PrnCmdLen( cmd , (ushort)cnt , pri_kind );	// [���s]�ް��ҏW
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�������ް��ҏW����													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Feed_set( pri_kind, size )							|*/
/*|																			|*/
/*|	PARAMETER		:	uchar		pri_kind	=	�󎚐������				|*/
/*|							R_PRI  : ڼ��									|*/
/*|							J_PRI  : �ެ���									|*/
/*|																			|*/
/*|						uchar		size		=	�����軲�ށi�ޯĐ��j	|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-04-18													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	Feed_set( uchar pri_kind, uchar size )
{
	uchar	cmd[3];		// ����������ް��ҏWܰ�

	cmd[0] = 0x1b;						// "ESC"
	cmd[1] = 0x4a;						// "J"
	cmd[2] = size;						// �����軲�޾��(n)
	PrnCmdLen( cmd , 3 , pri_kind );	// [������]�i������������ޕҏW�j
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���t�ް���������													|*/
/*|																			|*/
/*|			�N�A���A���A���A�����S�ĂO�̏ꍇ�A�`�F�b�NOK�Ƃ���				|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	DateChk( *date )									|*/
/*|																			|*/
/*|	PARAMETER		:	date_time_rec	*date	=	���t�ް��߲���			|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : �����ް�����								|*/
/*|							NG : �����ް��ُ�								|*/
/*|																			|*/
/*|	���l�F�N�A���A���A���A�����S�ĂO�̏ꍇ�A�`�F�b�NOK�Ƃ���				|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-03													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	DateChk( date_time_rec *date )
{
	uchar	ret = OK;	// ��������


	if( date->Year != 0 ){						// �N�ް�����
		if( (date->Year<1000) || (date->Year>9999) ){
			Prn_errlg( ERR_PRNT_YEAR_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// ���t�i�N�j�ް��װ�o�^
			ret = NG;
		}
	}
	if( date->Mon != 0 ){						// ���ް�����
		if( (date->Mon<1) || (date->Mon>12) ){
			Prn_errlg( ERR_PRNT_MON_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );		// ���t�i���j�ް��װ�o�^
			ret = NG;
		}
	}
	if( date->Day != 0 ){						// ���ް�����
		if( (date->Day<1) || (date->Day>31) ){
			Prn_errlg( ERR_PRNT_DAY_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );		// ���t�i���j�ް��װ�o�^
			ret = NG;
		}
	}
	if( date->Hour>23 ){						// ���ް�����
		Prn_errlg( ERR_PRNT_HOUR_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );		// ���t�i���j�ް��װ�o�^
		ret = NG;
	}
	if( date->Min>59 ){							// ���ް�����
		Prn_errlg( ERR_PRNT_MIN_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );			// ���t�i���j�ް��װ�o�^
		ret = NG;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���ް���������													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Prn_data_chk( req, data )							|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	req		= ���������ް����					|*/
/*|																			|*/
/*|						ulong	data	= �����ް�							|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : �����ް�����								|*/
/*|							NG : �����ް��ُ�								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-08													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	Prn_data_chk( uchar req, ulong data )
{
	uchar	ret = OK;	// ��������
	ushort	data1;
	ushort	data2;

	switch( req ){

		case	RYOUSYUU_PRN_SYU1:	// ���̎��؈󎚎�ʂP��	�����v��
			if( data > RYOUSYUU_PRN_SYU1_MAX ){
				ret = NG;
			}
			break;

		case	RYOUSYUU_PRN_SYU2:	// ���̎��؈󎚎�ʂQ��	�����v��
			if( data > RYOUSYUU_PRN_SYU2_MAX ){
				ret = NG;
			}
			break;

		case	PKICHI_DATA:			// �����Ԉʒu�ް���	�����v��

			Prn_pkposi_chg( data, &data1, &data2 );						// ���Ԉʒu�ް��ϊ�

			if( data1 > PKICHI_KU_MAX ){								// ���ԍ�����
				ret = NG;
			}

			if( data2 > PKICHI_NO_MAX ){								// ���Ԉʒu�ԍ�����
				ret = NG;
			}
			break;

		case	SEISAN_SYU:			// �����Z��ʁ�	�����v��
			if( (data > SEISAN_SYU_MAX) && (data != 0x000000ff) ){
				ret = NG;
			}
			break;

		case	RYOUKIN_SYU:		// ��������ʁ�		�����v��
			if( (data < RYOUKIN_SYU_MIN) || (data > RYOUKIN_SYU_MAX) ){
				ret = NG;
			}
			break;

		case	TEIKI_SYU:			// ���������ʁ�	�����v��
			if( (data < TEIKI_SYU_MIN) || (data > TEIKI_SYU_MAX) ){
				ret = NG;
			}
			break;

		case	PKNO_SYU:			// �����ԏ�m���D��ʁ�	�����v��
			if( data > PKNO_SYU_MAX ){
				ret = NG;
			}
			break;

		case	SERVICE_SYU:		// �����޽����ʁ�	�����v��
			if( (data < SERVICE_SYU_MIN) || (data > SERVICE_SYU_MAX) ){
				ret = NG;
			}
			break;

		case	TIKUSE_CNT:			// ���������g�p������	�����v��
			if( (data < TIKUSE_CNT_MIN) || (data > TIKUSE_CNT_MAX) ){
				ret = NG;
			}
			break;

		case	MISE_NO:			// ���X�m���D��	�����v��
			if( (data < MISE_NO_MIN) || (data > MISE_NO_MAX) ){
				ret = NG;
			}
			break;

		case	WARIBIKI_SYU:		// ��������ʁi�������j��	�����v��
			if( (data < WARIBIKI_SYU_MIN) || (data > WARIBIKI_SYU_MAX) ){
				ret = NG;
			}
			break;
// MH810100(S) m.saito 2020/05/13 �Ԕԃ`�P�b�g���X(�̎��؈󎚁F�����Z�o�ɑΉ�)
		case	UNPAID_SYU:			// �����Z�����Z��ʁ�	�����v��
			if(  data > UNPAID_SYU_MAX ){
				ret = NG;
			}
			break;
// MH810100(E) m.saito 2020/05/13 �Ԕԃ`�P�b�g���X(�̎��؈󎚁F�����Z�o�ɑΉ�)

		default:
			ret = NG;
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�e��۸ޏ��i�����A�߲���j��������									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Prn_log_check( cnt, wp, max, log_no )				|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	cnt		= �Ώ��ޯ̧�̌��݊i�[����			|*/
/*|																			|*/
/*|						ushort	wp		= �Ώ��ޯ̧��ײ��߲��				|*/
/*|																			|*/
/*|						ushort	max		= �Ώ��ޯ̧�̍ő�i�[����			|*/
/*|																			|*/
/*|						uchar	log_no	= ۸ޏ��No.						|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : �����ް�����								|*/
/*|							NG : �����ް��ُ�								|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-09													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	Prn_log_check( ushort cnt, ushort wp, ushort max, uchar log_no )
{
	uchar	ret = OK;	// ��������

	if( cnt > max ){	// ���݊i�[��������

		//	���݊i�[�������ُ�ȏꍇ
		Prn_errlg( ERR_PRNT_LOG_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// ۸��ް��װ�o�^
		ret = NG;
	}
	if( wp >= max ){	// ײ��߲������

		//	ײ��߲�����ُ�ȏꍇ
		Prn_errlg( ERR_PRNT_LOG_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// ۸��ް��װ�o�^
		ret = NG;
	}

	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�e��۸ޏ��̍Ō��ް���������										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Prn_log_search1( cnt, wp, max )						|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	cnt	= �Ώ��ޯ̧�̌��݊i�[����				|*/
/*|																			|*/
/*|						ushort	wp	= �Ώ��ޯ̧��ײ��߲��					|*/
/*|																			|*/
/*|						ushort	max	= �Ώ��ޯ̧�̍ő�i�[����				|*/
/*|																			|*/
/*|	RETURN VALUE	:	ushort	ret	= �Ō��ް����ް��߲���i�z��ԍ��j		|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-04-27													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
ushort	Prn_log_search1( ushort cnt, ushort wp, ushort max )
{
	ushort	ret;		// �Ō��ް����ް��߲���i�z��ԍ��j

	if( cnt == max ){
		//	���݊i�[�������ő�i�[�����̏ꍇ
		ret = wp;
	}
	else{
		//	���݊i�[�������ő�i�[�����̏ꍇ
		ret = 0;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�e��۸ޏ����ް��߲���X�V����										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Prn_log_search2( no, max )							|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	no	= �Ώ��ޯ̧���ް��߲���i�z��ԍ��j		|*/
/*|																			|*/
/*|						ushort	max	= �Ώ��ޯ̧�̍ő�i�[����				|*/
/*|																			|*/
/*|	RETURN VALUE	:	ushort	ret	= �X�V����ް��߲���i�z��ԍ��j			|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-04-27													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
ushort	Prn_log_search2( ushort no, ushort max )
{

	no++;				// �ް��߲���X�V�i�{�P�j

	if( no >= max ){

		//	�ް��߲�����ő�i�[���ɂȂ����ꍇ
		no = 0;
	}
	return( no );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���Ԉʒu�ް��ϊ�����												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Prn_pkposi_chg( no, *area, *pkno )					|*/
/*|																			|*/
/*|	PARAMETER		:	ulong	no		= ���Ԉʒu�ް�						|*/
/*|																			|*/
/*|						ushort	*area	= ���ԍ���ı��ڽ					|*/
/*|																			|*/
/*|						ushort	*pkno	= ���Ԉʒu�ԍ���ı��ڽ				|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-04-28													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	Prn_pkposi_chg( ulong no, ushort *area, ushort *pkno )
{
	*area	= (ushort)(no / 10000);		// ���ԍ����
	*pkno	= (ushort)(no % 10000);		// ���Ԉʒu�ԍ����
}

/*[]-----------------------------------------------------------------------[]*/
/*|		���Ԉʒu�i���^�ԍ��j�ް��쐬										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Prn_pkno_make( lk_no )								|*/
/*|																			|*/
/*|	PARAMETER		:	ushort	lk_no	= ۯ����uNo.						|*/
/*|																			|*/
/*|	RETURN VALUE	:	ulong	pk_posi	= ���Ԉʒu�i���^�ԍ��j�ް�		|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-05-12													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
ulong	Prn_pkposi_make( ushort lk_no )
{
	ulong	area;			// �u���v	�ݒ��ް��擾ܰ�
	ulong	posi;			// �u�ԍ��v	�ݒ��ް��擾ܰ�
	ulong	pk_posi = 0;	// ���Ԉʒu�ް�

	if( lk_no > 0 && lk_no <= LOCK_MAX ){	// ۯ����uNO.����

		//	ۯ����uNo.���͈͓��i�P�`�R�Q�S�j�̏ꍇ

		area =LockInfo[lk_no-1].area;		// �u���v	�ݒ��ް��擾
		posi =LockInfo[lk_no-1].posi;		// �u�ԍ��v	�ݒ��ް��擾
		pk_posi = (area*10000) + posi;		// ���ԈʒuNo.�i���^�ԍ��j�ް��쐬
	}
	return( pk_posi );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		��������䕔�װ���o�^����											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Prn_errlg( err_no, knd, data )						|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	err_no	=	�װNo.�iope_def.h�Q�Ɓj			|*/
/*|																			|*/
/*|						uchar	knd		=	�����^����						|*/
/*|																			|*/
/*|											PRN_ERR_OFF   (0)�F����			|*/
/*|											PRN_ERR_ON    (1)�F����			|*/
/*|											PRN_ERR_ONOFF (2)�F����/����	|*/
/*|																			|*/
/*|						uchar	data	=	�װ���L��						|*/
/*|																			|*/
/*|											PRN_ERR_JOHO_NON(0)�F����		|*/
/*|											PRN_ERR_JOHO_ARI(1)�F�L��		|*/
/*|																			|*/
/*|	���l�F�װ���L��̏ꍇ�AcErrlog[]�ɴװ��񂩾�Ă���Ă��鎖			|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-08-23													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	Prn_errlg( uchar err_no, uchar knd, uchar data )
{
		ex_errlg(							// ----- �G���[���o�^ -----
					ERRMDL_PRINTER,			// 	�v�����^�^�X�N���W���[���R�[�h
					err_no,					// 	�G���[�R�[�h
					knd,					// 	�����^����
					data					// 	�G���[���L��
				);
		cErrlog[0] = 0;						// string data clear
}
//[]-----------------------------------------------------------------------[]
// �w�b�_�[�����I�ȂQ�s���́@�ݒ�̃[���Ԗڂ��牽���H
// cnt(out) : ����
// Return:
//			�����I�ȍs����Ԃ� 0=�w�b�_�[�󎚎��̂Ȃ��ꍇ
//
// T.Okamoto	2006/08/30		Create
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]
uchar	Prn_LogHeadSub_Get2LineSetNum (uchar	*cnt )
{
	uchar	gyou_cnt;
	uchar	m_siz;

	gyou_cnt = (uchar)prm_get( COM_PRM, S_PRN, 4, 2, 3 );					// �uͯ�ް�s���v�ݒ��ް��擾
	if( gyou_cnt > HEAD_GYO_MAX ){
		gyou_cnt = HEAD_GYO_MAX;											// �s���ް��̏ꍇ�Aͯ�ް�ő�s���i�S�s�j�Ƃ���
	}
	*cnt = 0;
	if( gyou_cnt == 0 ) return 0;

	m_siz = (uchar)prm_get( COM_PRM, S_PRN, (short)5, 1, 4 );	// �u�������ށv�ݒ��ް��擾
	if( m_siz == 2 || m_siz == 3 ){
		*cnt = 1;
		return 2;		//�ݒ�̍ŏ��� 2�s��
	}else if( 1 == gyou_cnt ){
		*cnt = 1;
		return 1;	//1�s�����Ȃ�
	}
	*cnt = 2;
	return 2;	//�ݒ�Q�Ԗڂ܂ł�2�s��
}
//[]-----------------------------------------------------------------------[]
// ۺށ{ͯ�ް�󎚎w��̏ꍇ�Aۺވ�,��Ă̌�ŁAͯ�ް���Q�s���󎚂���
// �Q�s�ɖ����Ȃ��ꍇ�͎����萧����s��
//
// T.Okamoto	2006/08/30		Create
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]
void	Prn_LogoHead_PreHead ( void )
{
	uchar		cnt;
	uchar		ret;

	// �Q�s���́@�ݒ�̃[���Ԗڂ��牽���H=>cnt , �����s��=>ret
	ret = Prn_LogHeadSub_Get2LineSetNum ( &cnt );

	Prn_LogHead_PostHead_Offs = 0;

	if(cnt){
		Prn_edit_headhoot_new ( 0, cnt, PRI_HEADER, R_PRI ); // ͯ�ް���ް��ҏW
		if( header_hight == 0 ) ret = 0;					 // ۰�ނ���Ă��Ȃ��ꍇ
	}
	switch ( ret )
	{
	case	0:
		PrnCmdLen( (uchar *)"\x0a\x0a" , 2 , R_PRI );			// [���s]x2
		break;
	case	1:
		PrnCmdLen( (uchar *)"\x0a" , 1 , R_PRI );				// [���s]x1
	default:
		Prn_LogHead_PostHead_Offs = cnt;
		break;
	}
}

//[]-----------------------------------------------------------------------[]
// ۺށ{ͯ�ް�󎚎w��̏ꍇ�Aۺވ�,��Ă̌�ŁAͯ�ް���Q�s���󎚂���Ă���
// �̂ŁA�̂���̍s���󎚂���
//
// T.Okamoto	2006/08/30		Create
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]
void	Prn_LogoHead_PostHead ( void )
{
	short		cnt;
	short		max;
	uchar		settei;

	if((terget_pri == R_PRI)&&(f_partial_cut != 0)){// �󎚑Ώۂ����V�[�g�v�����^���p�[�V�����J�b�g���s�ς�
		// NOTE:�p�[�V�����J�b�g��A����؂藣�����߂ɗp�����������������邽�߁A�v���e���ɕ��ׂ�������B
		// ���̏�Ԃň󎚂��s���ƕ����̈󎚍s�����ׂ��\�������邽�߂��̑΍�
		// �󎚍s���ׂ�h�~�̂���1mm(8dot)�t�B�[�h����(�v�����^NP-2701�d�l�����)
		// ���ۂɊm�F�����Ƃ���1mm�̂���͖ڎ��ł͖w�ǔ��ʕs�ł������̂Ŗ��Ȃ�
		f_partial_cut = 0;					// �p�[�V�����J�b�g������s�t���O�N���A
		Feed_set( R_PRI, RCUT_END_PERTIAL_FEED );// ������i1mm�j
	}
	settei = (uchar)CPrmSS[S_PRN][3];	// �uڼ�Ķ�Ď��̈󎚁v�ݒ��ް��擾
	if( settei == 3 && Prn_LogHead_PostHead_Offs >= 1 && Prn_LogHead_PostHead_Offs < HEAD_GYO_MAX ) 
	{
		cnt = (short)Prn_LogHead_PostHead_Offs;
		max = (short)prm_get( COM_PRM, S_PRN, 4, 2, 3 );					// �uͯ�ް�s���v�ݒ��ް��擾
		max -= cnt;
		if(max > 0){
			Prn_edit_headhoot_new ( (uchar)cnt, (uchar)max, PRI_HEADER, R_PRI ); // ͯ�ް���ް��ҏW
		}
		PrnCmdLen( (uchar *)"\x0a" , 1 , R_PRI );			// [���s]x1
	}
	Prn_LogHead_PostHead_Offs = 0;
}
//[]-----------------------------------------------------------------------[]
// �̂���s�󎚃I�t�Z�b�g��������������
// 
//
// T.Okamoto	2006/08/30		Create
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]
void	Prn_LogoHead_Init ( void )
{
	uchar		ret, cnt;

	Prn_LogHead_PostHead_Offs = 0;

	if( CPrmSS[S_PRN][3] != 3 ) return;	// �uڼ�Ķ�Ď��̈󎚁v�ݒ��ް��擾

	// �Q�s���́@�ݒ�̃[���Ԗڂ��牽���H=>cnt , �����s��=>ret
	ret = Prn_LogHeadSub_Get2LineSetNum ( &cnt );
	if( ret ){
		Prn_LogHead_PostHead_Offs = cnt;
	}
}

uchar	Prn_JRprintSelect( uchar set_wk, uchar p_kind )
{
uchar	ret;

	ret = 0;
	switch( set_wk ){
		case 1:
			if( p_kind == J_PRI ){
				ret = 1;
			}
			break;
		case 2:
			if( p_kind == R_PRI ){
				ret = 1;
			}
			break;
		case 3:
			ret = 1;
			break;
	}
	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			�o�b�N�t�B�[�h�f�[�^�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		uchar		pri_kind	=	�󎚐������
///					R_PRI  : ڼ��
///					J_PRI  : �ެ���
///	@param[in]		uchar		size		=	�����軲�ށi�ޯĐ��j
///	@return			void
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/06/18<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	Back_Feed_set( uchar pri_kind, uchar size )
{
	uchar	cmd[3];		// ����������ް��ҏWܰ�

	cmd[0] = 0x1b;						// "ESC"
	cmd[1] = 0x42;						// "B"
	cmd[2] = size;						// �����軲�޾��(n)
	PrnCmdLen( cmd , 3 , pri_kind );	// [������]�i������������ޕҏW�j
}
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
//[]----------------------------------------------------------------------[]
///	@brief			�W���[�i���󎚏������`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return     ret  : 0K:�󎚒��łȂ� NG:�󎚒�
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2018/09/10<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
uchar	PriJrnExeCheck( void )
{
	uchar	ret = OK;		// �߂�l

	if( PrnJnlCheck() == ON ){	//	�ެ���������ڑ�����̏ꍇ
		// �{�֐��̓W���[�i���ɑ�ʂɈ󎚃f�[�^�𑗐M���Ȃ��悤�Ƀ`�F�b�N����ړI�̂��̂ł���
		// FT4000�V���[�Y��RXM���I2C�o�X�ŃW���[�i���^���V�[�g���ʂɂȂ��Ă��蓯���ɂ͕Е������󎚂ł��Ȃ��d�l�ł���
		// ���̂��߃��V�[�g���󎚂��Ă��Ȃ����Ƃ��`�F�b�N����
		if((jnl_proc_data.Printing != 0)|| // �W���[�i���v�����^�󎚒�
		   (rct_proc_data.Printing != 0)){ // ���V�[�g�v�����^�󎚒�
			ret = NG; // �󎚒�
		}else{
			ret = OK; // �󎚒��łȂ�
		}
	}else{
		ret = OK; // �󎚒��łȂ�
	}
	return(ret);
}
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)

// MH810104 GG119201(S) �d�q�W���[�i���Ή�
//[]----------------------------------------------------------------------[]
///	@brief			�v�����^���v��
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: �v�����
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_InfoReq(uchar kind)
{
	PRN_EJA_DATA_BUFF	*p = &eja_prn_buff;
	uchar	dat[3];
	short	timer;

// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�����ݒ��Ɏ��v�ݒ肵�Ȃ��j
	if (jnl_proc_data.Printing != 0) {
		// �����݊�����҂�
		eja_prn_buff.PrnInfReqFlg = kind;
		return;
	}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�����ݒ��Ɏ��v�ݒ肵�Ȃ��j

	memset(p->PrnRcvBuf, 0, sizeof(p->PrnRcvBuf));
	p->PrnRcvBufReadCnt = 0;

	// �v�����^���v���iESC s n�j
	memcpy(dat, "\x1b\x73", 2);
	dat[2] = kind;
	PrnCmdLen(dat, 3, J_PRI);

	SetPrnInfoReq(1);

	if (kind == PINFO_SD_TEST_RW) {
		timer = (PRN_RW_RES_WAIT_TIMER * 2) + 1;
	}
	else {
		timer = PRN_PINFO_RES_WAIT_TIMER + 1;
	}
	LagTim500ms(LAG500_EJA_PINFO_RES_WAIT_TIMER, timer, PrnCmd_InfoResTimeout);
}

//[]----------------------------------------------------------------------[]
///	@brief			�v�����^��񉞓��҂��^�C���A�E�g
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_InfoResTimeout(void)
{
	SetPrnInfoReq(0);

	if (jnl_init_sts == 0) {
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�����ݒ芮���҂��^�C�}�ύX�j
//		queset(OPETCBNO, TIMEOUT9, 0, NULL);
		queset(OPETCBNO, EJA_INIT_WAIT_TIMEOUT, 0, NULL);
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�����ݒ芮���҂��^�C�}�ύX�j
	}
	else {
		MsgSndFrmPrn(PREQ_FS_ERR, J_PRI, 0);
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�v�����^��񉞓��̃f�[�^�T�C�Y�ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: �v�����
///	@return			�f�[�^�T�C�Y
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
uchar PrnCmd_InfoRes_DataSize_Get(uchar kind)
{
	uchar	size = 2;

	switch (kind) {
	case PINFO_SD_INFO:				// SD�J�[�h���v��
		size += 20;
		break;
	case PINFO_MACHINE_INFO:		// �@����v��
		size += 10;
		break;
	case PINFO_VER_INFO:			// �\�t�g�o�[�W�����v��
// MH810104 GG119201(S) �d�q�W���[�i���Ή��iSD�o�[�W�������擾�ł��Ȃ��j
	case PINFO_SD_VER_INFO:			// SD�J�[�h�o�[�W�����v��
// MH810104 GG119201(E) �d�q�W���[�i���Ή��iSD�o�[�W�������擾�ł��Ȃ��j
	case PINFO_SD_TEST_RW:			// SD�J�[�h�e�X�g�i���[�h���C�g�j
		size += 8;
		break;
	default:
		break;
	}
	return size;
}

//[]----------------------------------------------------------------------[]
///	@brief			�v�����^��񉞓���M����
//[]----------------------------------------------------------------------[]
///	@param[in]		*pBuf	: ��M�f�[�^
///	@param[in]		RcvSize	: ��M�f�[�^�T�C�Y
///	@return			0=��M������, 1=��M����
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
uchar PrnCmd_InfoRes_Proc(uchar *pBuf, ushort RcvSize)
{
	PRN_EJA_DATA_BUFF	*p = &eja_prn_buff;
	uchar	ret = 0;
	uchar	rp, DatSize;

	// ��M�f�[�^�Z�b�g
	rp = p->PrnRcvBufReadCnt;
	memcpy(&p->PrnRcvBuf[rp], pBuf, (size_t)RcvSize);
	p->PrnRcvBufReadCnt += (uchar)RcvSize;

	// �v�����^��񉞓�
	// 1byte�ځ�0xC0�i�W���[�i���v�����^�j�A0xFF�i�d�q�W���[�i���j
	// 2byte�ځ��v�����
	// 3byte�ڈȍ~���e�f�[�^
	if (p->PrnRcvBufReadCnt >= 2) {
		// ��M�f�[�^�T�C�Y�擾
		DatSize = PrnCmd_InfoRes_DataSize_Get(p->PrnRcvBuf[1]);
		if (p->PrnRcvBufReadCnt >= DatSize) {
			// ��M����
			LagCan500ms(LAG500_EJA_PINFO_RES_WAIT_TIMER);
			SetPrnInfoReq(0);
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i���Z�b�g���g���C�񐔂�ǉ��j
//			p->PrnRetryCnt = 0;
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i���Z�b�g���g���C�񐔂�ǉ��j
			ret = 1;
		}
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�v�����^��񉞓����b�Z�[�W
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_InfoRes_Msg(void)
{
	PRN_EJA_DATA_BUFF	*p = &eja_prn_buff;
	ushort	msg;
	uchar	*pBuf;

	memset(eja_work_buff, 0, sizeof(eja_work_buff));
	pBuf = &p->PrnRcvBuf[2];
	switch (p->PrnRcvBuf[1]) {
	case PINFO_SD_INFO:				// SD�J�[�h���v��
		msg = PREQ_SD_INFO;
		memcpy(eja_work_buff, pBuf, 20);
		break;
	case PINFO_MACHINE_INFO:		// �@����v��
		msg = PREQ_MACHINE_INFO;
		if (!memcmp(pBuf, EJA_MACHINE_INFO, 3)) {
			// �d�q�W���[�i���ڑ�����
			p->EjaUseFlg = 1;
		}
		else {
			// �d�q�W���[�i���ڑ��Ȃ�
			p->EjaUseFlg = 0;
		}
		break;
	case PINFO_VER_INFO:			// �\�t�g�o�[�W�����v��
		msg = PREQ_SW_VER_INFO;
		memcpy(eja_work_buff, pBuf, 8);
		break;
	case PINFO_SD_VER_INFO:			// SD�J�[�h�o�[�W�����v��
		msg = PREQ_SD_VER_INFO;
		memcpy(eja_work_buff, pBuf, 8);
		break;
	case PINFO_SD_TEST_RW:			// SD�J�[�h�e�X�g�i���[�h���C�g�j
		msg = PREQ_SD_TEST_RW;
		memcpy(eja_work_buff, pBuf, 8);
		break;
	default:
		return;
	}
	queset(OPETCBNO, msg, 0, NULL);
}

//[]----------------------------------------------------------------------[]
///	@brief			�����ݒ�i�d�q�W���[�i���p�j
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_Clock(void)
{
	uchar	dat[9];

	// �����ݒ�iGS E n1 n2 n3 n4 n5 n6 n7�j
	memcpy(dat, "\x1d\x45", 2);
	// �N���������b
	memcpy(&dat[2], &CLK_REC.year, 7);
	PrnCmdLen(dat, 9, J_PRI);
}

//[]----------------------------------------------------------------------[]
///	@brief			�����ݒ�i�d�q�W���[�i���p�j
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_Encrypt(void)
{
#pragma pack
	struct {
		uchar	cmd[2];
		ulong	dat[16];
	} code;
#pragma unpack
	uchar	zero[12];
	ushort	i;
	ulong	work;

// MH810104 GG119201(S) �d�q�W���[�i���Ή� #5942 ���Z�@����d�q�W���[�i���ɑ��M����鏉���ݒ�R�}���h�̋@�햼���uFT-4000�v�ɂȂ�
//	const char name[] = "FT-4000";
	const char name[] = "GT-4100";
// MH810104 GG119201(E() �d�q�W���[�i���Ή� #5942 ���Z�@����d�q�W���[�i���ɑ��M����鏉���ݒ�R�}���h�̋@�햼���uFT-4000�v�ɂȂ�

	// �����ݒ�iGS K Dn�j
	memset(&code, 0, sizeof(code));
	memset(zero, 0, sizeof(zero));
	memcpy(code.cmd, "\x1d\x4b", 2);

	// 1�����O���[�h�� �Í��������i0=AES128bit, 1=�Í����Ȃ��j
	work = prm_get(COM_PRM, S_REC, 2, 1, 1);
	if (work != 1) {
		work = 0;
	}
	code.dat[0] = work;

	// 2�`4�����O���[�h�� �Í����L�[
	for (i = 0; i < 3; i++) {
		code.dat[1+i] = prm_get(COM_PRM, S_REC, 3+i, 6, 1);
	}
	// �Í����L�[�ݒ肪�I�[���O�̏ꍇ�͈Í����L�[��������������
	if (!memcmp(&code.dat[1], zero, 12)) {
		// �b�P�ʂ̃m�[�}���C�Y�l��seed�Ƃ���
		srand((unsigned int)c_Normalize_sec(&CLK_REC));

		for (i = 0; i < 6; i++) {
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�Í����L�[�͉p�����݂̂Ƃ���j
//			do {
//				work = (ulong)rand();
//				// ���䕶���ȊO��ASCII�R�[�h���Í����L�[�Ƃ���
//				work %= 0x7F;
//			} while (work < 0x20);
			while (1) {
				work = (ulong)rand();
				work %= 0x7A;
				// �Í����L�[�͉p�����݂̂Ƃ���
				if (('0' <= work && work <= '9') ||
					('A' <= work && work <= 'Z') ||
					('a' <= work && work <= 'z')) {
					break;
				}
			}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�Í����L�[�͉p�����݂̂Ƃ���j

			if ((i % 2) == 0) {
				work *= 1000;
			}
			CPrmSS[S_REC][3+i/2] += work;
			code.dat[1+i/2] += work;
		}
		// 1.���ʃp�����[�^�̃T�����Čv�Z
		DataSumUpdate(OPE_DTNUM_COMPARA);	/* update parameter sum on ram */
		// 2.�t���b�V���ɃZ�[�u
		(void)FLT_WriteParam1(FLT_NOT_EXCLUSIVE);		// FlashROM update
	}
	
	// 5�����O���[�h�� ���ԏ�No.
	code.dat[4] = prm_get(COM_PRM, S_SYS, 1, 6, 1);
	
	// 6�����O���[�h�� �@�BNo.
	code.dat[5] = prm_get(COM_PRM, S_PAY, 2, 2, 1);
	
	// 7�����O���[�h�� �f�o�C�XID
	code.dat[6] = 0;	// �W����0
	
	// 8�`11�����O���[�h�� �@�햼
	memcpy(&code.dat[7], name, strlen(name) );

	PrnCmdLen((uchar*)&code, sizeof(code), J_PRI);
}

//[]----------------------------------------------------------------------[]
///	@brief			�����^�����ݒ�i�d�q�W���[�i���p�j
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_InitSetting(void)
{
	// �����ݒ�
	PrnCmd_Clock();
	// �����ݒ�
	PrnCmd_Encrypt();

	if (jnl_init_sts == 0) {
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�����ݒ芮���҂��^�C�}�ύX�j
//		Lagtim(OPETCBNO, 9, PRN_INIT_SET_WAIT_TIMER);
		Lagtim(OPETCBNO, TIMERNO_EJA_INIT_WAIT, PRN_INIT_SET_WAIT_TIMER);
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�����ݒ芮���҂��^�C�}�ύX�j
	}
	else {
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�����^�����ݒ�҂��^�C�}�������j
//		Lagtim(PRNTCBNO, 9, PRN_CLK_SET_WAIT_TIMER);
		Lagtim(PRNTCBNO, 9, PRN_INIT_SET_WAIT_TIMER);
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�����^�����ݒ�҂��^�C�}�������j
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�f�[�^�����݊J�n�^�I�����M�i�d�q�W���[�i���p�j
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: 0=�����݊J�n, 1=�����ݏI��
///	@param[in]		*msg	: �󎚃��b�Z�[�W
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_WriteStartEnd(uchar kind, MSG *msg)
{
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
	if (!isEJA_USE()) {
		if (kind == 0) {
			// �W���uID
			memcpy(&jnl_proc_data.Prn_Job_id, &msg->data[PRN_QUE_OFFSET], sizeof(ulong));
			// �W���[�i���v�����^�͈󎚐���f�[�^�ƈ󎚃��b�Z�[�W�̕ێ��̂ݍs��
			memcpy(&eja_prn_buff.eja_proc_data, &jnl_proc_data, sizeof(jnl_proc_data));
			memcpy(&eja_prn_buff.PrnMsg, msg, sizeof(*msg));
			// �󎚃f�[�^�L���[�ɓ����f�[�^������ꍇ�͍폜����
			PrnDat_DelQue(jnl_proc_data.Printing, jnl_proc_data.Prn_Job_id);
		}
		return;
	}
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j

	if (kind == 0) {
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		// �W���uID
		memcpy(&jnl_proc_data.Prn_Job_id, &msg->data[PRN_QUE_OFFSET], sizeof(ulong));
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		// �f�[�^�����݊J�n
		PCMD_WRITE_START(J_PRI);
		// �󎚐���f�[�^�ƈ󎚃��b�Z�[�W��ێ�����
		memcpy(&eja_prn_buff.eja_proc_data, &jnl_proc_data, sizeof(jnl_proc_data));
		memcpy(&eja_prn_buff.PrnMsg, msg, sizeof(*msg));
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		// �󎚃f�[�^�L���[�ɓ����f�[�^������ꍇ�͍폜����
		PrnDat_DelQue(jnl_proc_data.Printing, jnl_proc_data.Prn_Job_id);
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
	}
	else if (kind == 1) {
		// �f�[�^�����ݏI��
		PCMD_WRITE_END(J_PRI);
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�f�[�^�����݊����҂��^�C�}�ύX�j
//		Lagtim(PRNTCBNO, 8, PRN_WRITE_CMP_TIMER);
		Lagtim(PRNTCBNO, 10, PRN_WRITE_CMP_TIMER);
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�f�[�^�����݊����҂��^�C�}�ύX�j
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�d�q�W���[�i���p���M�f�[�^�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		*pBuf	: ���M�f�[�^
///	@return			0=�d�q�W���[�i���p�f�[�^�ȊO, 1=�d�q�W���[�i���p�f�[�^
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
uchar PrnCmd_CheckSendData(uchar *pBuf)
{
	uchar	ret = 0;

	// �@����v��
	if (isCMD_MACHINE_INFO(pBuf)) {
		ret = 1;
	}
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
	// �f�[�^�����݊J�n
	else if (isCMD_WRITE_START(pBuf)) {
		ret = 1;
	}
	// �Í����L�[�R�}���h
	else if (isCMD_ENCRYPT(pBuf)) {
		ret = 1;
	}
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�v�����^�X�e�[�^�X��M�����i�d�q�W���[�i���p�j
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_Sts_Proc(void)
{
	PRN_DATA_BUFF	*p = &jnl_prn_buff;	// ������ް��ޯ̧�߲������
	uchar	ebit;

// MH810104 GG119201(S) �d�q�W���[�i���Ή��iSD�J�[�h�}�E���g������҂j
//	ebit = (uchar)((p->PrnState[0] ^ p->PrnState[1]) & 0xc8);
	ebit = (uchar)((p->PrnState[0] ^ p->PrnState[1]) & 0xCA);
// MH810104 GG119201(E) �d�q�W���[�i���Ή��iSD�J�[�h�}�E���g������҂j
	if (ebit != 0) {
// MH810104 GG119201(S) �d�q�W���[�i���Ή��iSD�J�[�h�}�E���g������҂j
		if ((ebit & 0x02) != 0) {
			// SD�J�[�h��Ԃɕω�����
			if ((p->PrnState[0] & 0x02) != 0) {
				// SD�J�[�h�g�p�s��
				if (jnl_init_sts == 0) {
					// �����������҂�
					LagCan500ms(LAG500_EJA_RESET_WAIT_TIMER);
					Lagcan(OPETCBNO, TIMERNO_EJA_INIT_WAIT);
					queset(OPETCBNO, EJA_INIT_WAIT_TIMEOUT, 0, NULL);
				}
				else {
					if (LagTim500ms_Is_Counting(LAG500_EJA_RESET_WAIT_TIMER)) {
						// ���Z�b�g�����҂�
						LagCan500ms(LAG500_EJA_RESET_WAIT_TIMER);
						// �n�[�h���Z�b�g
						MsgSndFrmPrn(PREQ_RESET, J_PRI, 0);
					}
				}
			}
			else {
				// SD�g�p��
				if ((p->PrnState[0] & 0x40) != 0) {
					// �����^�����ݒ�=���ݒ�
					// �����^�����ݒ�f�[�^
					MsgSndFrmPrn(PREQ_INIT_SET_REQ, J_PRI, 0);
				}
			}
		}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��iSD�J�[�h�}�E���g������҂j

		if ((ebit & 0x08) != 0) {
			// �t�@�C���V�X�e����Ԃɕω�����
			if ((p->PrnState[0] & 0x08) != 0) {
				// �t�@�C���V�X�e���ُ�
// MH810104 GG119201(S) �d�q�W���[�i���Ή��iSD�J�[�h�}�E���g������҂j
//				Lagcan(PRNTCBNO, 8);
//				MsgSndFrmPrn(PREQ_FS_ERR, J_PRI, 0);
				if (jnl_init_sts == 0) {
					// �����������҂�
					LagCan500ms(LAG500_EJA_RESET_WAIT_TIMER);
					Lagcan(OPETCBNO, TIMERNO_EJA_INIT_WAIT);
					queset(OPETCBNO, EJA_INIT_WAIT_TIMEOUT, 0, NULL);
				}
				else {
					if (LagTim500ms_Is_Counting(LAG500_EJA_RESET_WAIT_TIMER)) {
						// ���Z�b�g�����҂�
						LagCan500ms(LAG500_EJA_RESET_WAIT_TIMER);
						// �n�[�h���Z�b�g
						MsgSndFrmPrn(PREQ_RESET, J_PRI, 0);
					}
					else {
						// �ҋ@��� or �����ݒ�
						Lagcan(PRNTCBNO, 9);
						Lagcan(PRNTCBNO, 10);
						// �\�t�g���Z�b�g
						MsgSndFrmPrn(PREQ_FS_ERR, J_PRI, 0);
					}
				}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��iSD�J�[�h�}�E���g������҂j
			}
		}

		if ((ebit & 0x40) != 0) {
			// �����^�����ݒ��Ԃɕω�����
			if ((p->PrnState[0] & 0x40) != 0) {
				// �����^�����ݒ�=���ݒ�
// MH810104 GG119201(S) �d�q�W���[�i���Ή��iSD�J�[�h�}�E���g������҂j
//				if ((p->PrnState[0] & 0x80) != 0) {
//					// �f�[�^�����ݒ�
//					Lagcan(PRNTCBNO, 8);
//					MsgSndFrmPrn(PREQ_FS_ERR, J_PRI, 0);
//				}
//				else {
//					LagCan500ms(LAG500_EJA_RESET_WAIT_TIMER);
//					PrnCmd_ResetTimeout();
//				}
				// �����^�����ݒ�f�[�^���M
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�󎚃f�[�^���đ����Ȃ��j
//				LagCan500ms(LAG500_EJA_RESET_WAIT_TIMER);
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�󎚃f�[�^���đ����Ȃ��j
				if (jnl_init_sts == 0) {
					if (eja_prn_buff.EjaUseFlg < 0) {
						// �d�q�W���[�i������O�ł���΁A�d�q�W���[�i�������Ɏ����^�����ݒ�f�[�^�𑗐M����
					}
					else {
						// �����������҂�
						queset(OPETCBNO, PREQ_MACHINE_INFO, 0, NULL);
					}
				}
				else {
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�󎚃f�[�^���đ����Ȃ��j
//					if (jnl_proc_data.Printing != 0) {
//						// �����ݒ�
//						Lagcan(PRNTCBNO, 10);
//						// �\�t�g���Z�b�g
//						MsgSndFrmPrn(PREQ_FS_ERR, J_PRI, 0);
//					}
//					else {
//						// �ҋ@��� or ���Z�b�g�����҂�
//						// �����^�����ݒ�f�[�^
//						MsgSndFrmPrn(PREQ_CLOCK_REQ, J_PRI, 0);
//					}
					if (LagTim500ms_Is_Counting(LAG500_EJA_RESET_WAIT_TIMER)) {
						// ���Z�b�g�����҂�
						LagCan500ms(LAG500_EJA_RESET_WAIT_TIMER);
						// �����^�����ݒ�f�[�^
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�����ݒ��Ɏ��v�ݒ肵�Ȃ��j
//						MsgSndFrmPrn(PREQ_CLOCK_REQ, J_PRI, 0);
						MsgSndFrmPrn(PREQ_INIT_SET_REQ, J_PRI, 0);
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�����ݒ��Ɏ��v�ݒ肵�Ȃ��j
					}
					else {
						if (jnl_proc_data.Printing != 0) {
							// �����ݒ�
							// �����݊����҂��^�C���A�E�g��҂�
						}
						else {
							// �ҋ@��� or ���Z�b�g�����҂�
							// �����^�����ݒ�f�[�^
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�����ݒ��Ɏ��v�ݒ肵�Ȃ��j
//							MsgSndFrmPrn(PREQ_CLOCK_REQ, J_PRI, 0);
							MsgSndFrmPrn(PREQ_INIT_SET_REQ, J_PRI, 0);
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�����ݒ��Ɏ��v�ݒ肵�Ȃ��j
						}
					}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�󎚃f�[�^���đ����Ȃ��j
				}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��iSD�J�[�h�}�E���g������҂j
			}
			else {
				// �����^�����ݒ�=�ݒ�ς�
				if (jnl_init_sts == 0) {
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�����ݒ芮���҂��^�C�}�ύX�j
//					Lagcan(OPETCBNO, 9);
					Lagcan(OPETCBNO, TIMERNO_EJA_INIT_WAIT);
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�����ݒ芮���҂��^�C�}�ύX�j
// MH810105 GG119202(S) �d�q�W���[�i���Ή��iSD�}�E���g�Ɏ��Ԃ�������ƒʐM�s�ǂ���������j
//					PrnMode_idle_J();
// MH810105 GG119202(E) �d�q�W���[�i���Ή��iSD�}�E���g�Ɏ��Ԃ�������ƒʐM�s�ǂ���������j
					// �������I���ʒm
					queset(OPETCBNO, PREQ_INIT_END_J, 0, NULL);
				}
				else {
					Lagcan(PRNTCBNO, 9);
					// �����ݒ芮���ʒm
					MsgSndFrmPrn(PREQ_CLOCK_SET_CMP, J_PRI, 0);
				}
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�󎚃f�[�^���đ����Ȃ��j
//// MH810104 GG119201(S) �d�q�W���[�i���Ή��i���Z�b�g���g���C�񐔂�ǉ��j
////				eja_prn_buff.PrnRetryCnt = 0;
//				eja_prn_buff.PrnHResetRetryCnt = 0;
//				eja_prn_buff.PrnSResetRetryCnt = 0;
//// MH810104 GG119201(E) �d�q�W���[�i���Ή��i���Z�b�g���g���C�񐔂�ǉ��j
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�󎚃f�[�^���đ����Ȃ��j
			}
		}

		if ((ebit & 0x80) != 0) {
			// �f�[�^�����ݏ�Ԃɕω�����
			if ((p->PrnState[0] & 0x80) == 0) {
				// �f�[�^�������݊���
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�f�[�^�����݊����҂��^�C�}�ύX�j
//				Lagcan(PRNTCBNO, 8);
				Lagcan(PRNTCBNO, 10);
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�f�[�^�����݊����҂��^�C�}�ύX�j
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�f�[�^�����݊�����҂����Ƀf�[�^���M���Ă��܂��j
//				if (p->PrnBufWriteCnt == p->PrnBufReadCnt) {
//					p->PrnBufWriteCnt = 0;
//					p->PrnBufReadCnt = 0;
//				}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�f�[�^�����݊�����҂����Ƀf�[�^���M���Ă��܂��j
				// �󎚏I��
				PrnINJI_END(NULL);
				err_chk(ERRMDL_EJA, ERR_EJA_WRITE_ERR, 0, 0, 0);
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�󎚃f�[�^���đ����Ȃ��j
				eja_prn_buff.PrnHResetRetryCnt = 0;
				eja_prn_buff.PrnSResetRetryCnt = 0;
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�󎚃f�[�^���đ����Ȃ��j
			}
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�N�����G���[�o�^�����i�d�q�W���[�i���p�j
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_EJAErrRegist(void)
{
	if (!isEJA_USE()) {
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
		err_pr_chk2(jnl_prn_buff.PrnState[0], 0);
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
		return;
	}

	// �N�����ォ��SD�J�[�h�j�A�G���h���̃r�b�g��ON�̏ꍇ�A
	// �ω������o�ł����ɃG���[�o�^���ł��Ȃ��̂�
	// �v�����^�^�X�N�N�����Ƀr�b�g��ON�ł���΃G���[�o�^����
	err_eja_chk(jnl_prn_buff.PrnState[0], 0);
}

//[]----------------------------------------------------------------------[]
///	@brief			�f�[�^�����ݍĊJ����
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			0=�ĊJ�Ȃ�, 1=�ĊJ����i�f�[�^�����ݓr���ŕ��d�j,
///					2=�ĊJ����i�L���[����j, 3=�ĊJ����i�f�[�^�����ݓr���ŕ��d�E�L���[����j
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
uchar PrnCmd_ReWriteCheck(void)
{
	uchar	ret = 0;

// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
//	if (!isEJA_USE()) {
//		return ret;
//	}
	// �d�q�W���[�i���E�W���[�i���v�����^�Ƃ��Ƀf�[�^�����ݍĊJ�������s��
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j

	if (eja_prn_buff.eja_proc_data.Printing != 0) {
		// �f�[�^�����ݓr���ŕ��d�������߁A�f�[�^�����݊J�n����ĊJ����
		memcpy(&jnl_proc_data, &eja_prn_buff.eja_proc_data, sizeof(jnl_proc_data));
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		// �󎚃f�[�^�L���[�ɓ����f�[�^������ꍇ�͍폜����
		PrnDat_DelQue(jnl_proc_data.Printing, jnl_proc_data.Prn_Job_id);
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ret = 1;
	}
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
	if (PrnDat_GetQueCount() > 0) {
		if (ret == 0) {
			// �󎚃f�[�^�L���[�Ƀf�[�^����
			prn_proc_data_clr_J();

			// �󎚃f�[�^�L���[����ǂݎ��
			PrnDat_ReadQue(&eja_prn_buff.PrnMsg);
			// �W���uID���Z�b�g
			memcpy(&eja_prn_buff.eja_proc_data.Prn_Job_id,
					&eja_prn_buff.PrnMsg.data[PRN_QUE_OFFSET],
					sizeof(ulong));
			// �󎚗v���R�}���h���Z�b�g
			eja_prn_buff.eja_proc_data.Printing = eja_prn_buff.PrnMsg.command;
			jnl_proc_data.Printing = eja_prn_buff.PrnMsg.command;
			jnl_proc_data.Prn_Job_id = eja_prn_buff.eja_proc_data.Prn_Job_id;
			ret = 2;
		}
		else {
			// ��Ƀf�[�^�����ݓr���ŕ��d�����ꍇ�̈󎚂��s��
			ret = 3;
		}
	}
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�f�[�^�����ݍĊJ
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: 0=�ʏ�, 1=���d
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_ReWriteStart(uchar kind)
{
	MsgBuf	*OutMsg, *msb;

	if (jnl_proc_data.Printing == 0) {
		return;
	}

	if (kind != 0) {
		if ((OutMsg = GetBuf()) == NULL) {
			return;
		}
		// �_�~�[���b�Z�[�W�o�^�i���d���O����Ɉ󎚂����邽�߁j
		OutMsg->msg.command = PREQ_DUMMY;
		PutMsg(PRNTCBNO, OutMsg);
		PriDummyMsg = ON;
	}

	if ((msb = GetBuf()) == NULL) {
		return;
	}

	// �󎚃��b�Z�[�W�쐬
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�󎚃f�[�^�đ������ύX�j
//// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�󎚃f�[�^���đ����Ȃ��j
//	if (kind == 0) {
//		memcpy(&jnl_proc_data, &eja_prn_buff.eja_proc_data, sizeof(jnl_proc_data));
//	}
//// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�󎚃f�[�^���đ����Ȃ��j
//	msb->msg.command = jnl_proc_data.Printing;
//	if (jnl_proc_data.Prn_no > 0) {
//		// �󎚏������t���O���Z�b�g����
//		msb->msg.command |= INNJI_NEXTMASK;
//	}
//// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�󎚃f�[�^���đ����Ȃ��j
//	else {
//		// �����
//		jnl_proc_data.Printing = 0;
//	}
//// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�󎚃f�[�^���đ����Ȃ��j
	if (kind == 0) {
		// �ʏ�
		memcpy(&jnl_proc_data, &eja_prn_buff.eja_proc_data, sizeof(jnl_proc_data));
		msb->msg.command = jnl_proc_data.Printing;
		if (jnl_proc_data.Prn_no > 0) {
			// �󎚏������t���O���Z�b�g����
			msb->msg.command |= INNJI_NEXTMASK;
		}
		else {
			// �����
			jnl_proc_data.Printing = 0;
		}
	}
	else {
		// ���d
		msb->msg.command = jnl_proc_data.Printing;
		// ���d���͕K���󎚏������t���O���Z�b�g����
		// terget_pri=J_PRI�Ɍ��肵��������
		msb->msg.command |= INNJI_NEXTMASK;
	}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�󎚃f�[�^�đ������ύX�j
	memcpy(&msb->msg.data, &eja_prn_buff.PrnMsg.data, sizeof(msb->msg.data));
	msb->msg.data[0] = (J_PRI << 4);
	msb->msg.data[0] |= J_PRI;

	if (kind != 0) {
		// �����Ώۃ��b�Z�[�W�ۑ�
		TgtMsg = msb;
	}
	else {
		// �����Ώۃ��b�Z�[�W�o�^
		PutMsg(PRNTCBNO, msb);
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�d�q�W���[�i�����Z�b�g����
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: 0=�\�t�g���Z�b�g, 1=�n�[�h���Z�b�g
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_EJAReset(uchar kind)
{
	PRN_EJA_DATA_BUFF	*p = &eja_prn_buff;
	short	timer;
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i���Z�b�g��ɃX�e�[�^�X���N���A����j
	ulong	ist;
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i���Z�b�g��ɃX�e�[�^�X���N���A����j

// MH810104 GG119201(S) �d�q�W���[�i���Ή��i���Z�b�g���g���C�񐔂�ǉ��j
//	if (kind == 0) {
	if (kind == 0 && p->PrnSResetRetryCnt == 0) {
		p->PrnSResetRetryCnt++;
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i���Z�b�g���g���C�񐔂�ǉ��j
		// �\�t�g���Z�b�g
		PCMD_RESET(J_PRI);
	}
	else {
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i���Z�b�g���g���C�񐔂�ǉ��j
//		if (p->PrnRetryCnt > 0) {
		if (p->PrnHResetRetryCnt > 0) {
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i���Z�b�g���g���C�񐔂�ǉ��j
			// ���g���C�I�[�o�[
// MH810104 GG119201(S) �d�q�W���[�i���Ή��iSD�J�[�h�}�E���g������҂j
//			Inji_ErrEnd(p->PrnMsg.command, PRI_ERR_STAT, J_PRI);
//			if (jnl_init_sts == 0) {
//				// �G���[���o�^�i���������s�j
//				jnl_init_sts = 2;
//				Prn_errlg( ERR_PRNT_INIT_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );
//				err_chk(ERRMDL_EJA, ERR_EJA_WAKEUP_ERR, 1, 0, 0);
//			}
//			else {
//				err_chk(ERRMDL_EJA, ERR_EJA_WRITE_ERR, 1, 0, 0);
//			}
			if (jnl_init_sts == 0) {
				// �G���[���o�^�i���������s�j
				jnl_init_sts = 2;
				Prn_errlg( ERR_PRNT_INIT_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );
				if (isEJA_USE()) {
					// �v�����^�X�e�[�^�X�̒ʒm���Ȃ��ꍇ��
					// SD�J�[�h�}�E���g���s�Ƃ��āuSD�J�[�h�g�p�s�v�G���[��o�^����
					err_chk(ERRMDL_EJA, ERR_EJA_SD_UNAVAILABLE, 1, 0, 0);
				}
				else {
					// �@���񉞓��^�C���A�E�g�Łu���ڑ��v�G���[��o�^����
					err_chk(ERRMDL_EJA, ERR_EJA_UNCONNECTED, 1, 0, 0);
				}
			}
			else {
				// �u�����ݎ��s�v�G���[��o�^����
				err_chk(ERRMDL_EJA, ERR_EJA_WRITE_ERR, 1, 0, 0);
			}
			Inji_ErrEnd(p->PrnMsg.command, PRI_ERR_STAT, J_PRI);
// MH810104 GG119201(E) �d�q�W���[�i���Ή��iSD�J�[�h�}�E���g������҂j
			queset(OPETCBNO, PREQ_ERR_END, 0, NULL);
			return;
		}
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i���Z�b�g���g���C�񐔂�ǉ��j
//		p->PrnRetryCnt++;
		p->PrnHResetRetryCnt++;
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i���Z�b�g���g���C�񐔂�ǉ��j

		// �n�[�h���Z�b�g
		ExIOSignalwt(EXPORT_JP_RES, 1);
		xPause_PRNTSK(2);					// 20ms Wait
		ExIOSignalwt(EXPORT_JP_RES, 0);
	}

// MH810104 GG119201(S) �d�q�W���[�i���Ή��i���Z�b�g��ɃX�e�[�^�X���N���A����j
	// �v�����^�X�e�[�^�X���N���A����
	ist = _di2();
	jnl_prn_buff.PrnStateMnt = 0;
	jnl_prn_buff.PrnStWork = 0;
	jnl_prn_buff.PrnState[2] = jnl_prn_buff.PrnState[1];
	jnl_prn_buff.PrnState[1] = jnl_prn_buff.PrnState[0];
	jnl_prn_buff.PrnState[0] = jnl_prn_buff.PrnStWork;
	_ei2(ist);
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i���Z�b�g��ɃX�e�[�^�X���N���A����j

// MH810104 GG119201(S) �d�q�W���[�i���Ή��iSD�J�[�h�}�E���g������҂j
//	// �����^�Í����L�[���ݒ�̃v�����^�X�e�[�^�X���ő�8�b�҂��ƂƂ���
//	timer = (PRN_RESET_WAIT_TIMER * 2) + 1;
	if (eja_prn_buff.EjaUseFlg < 0) {
		// ���Z�b�g������҂�
		timer = PRN_RESET_WAIT_TIMER + 1;
	}
	else {
		// �����^�����ݒ�=���ݒ�̃v�����^�X�e�[�^�X��҂�
		timer = (PRN_SD_MOUNT_WAIT_TIMER * 2) + 1;
	}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��iSD�J�[�h�}�E���g������҂j
	LagTim500ms(LAG500_EJA_RESET_WAIT_TIMER, timer, PrnCmd_ResetTimeout);
}

//[]----------------------------------------------------------------------[]
///	@brief			�d�q�W���[�i�����Z�b�g�^�C���A�E�g
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_ResetTimeout(void)
{
// MH810104 GG119201(S) �d�q�W���[�i���Ή��iSD�J�[�h�}�E���g������҂j
//	if (jnl_init_sts == 0) {
//		// �@����ʒm
//		queset(OPETCBNO, PREQ_MACHINE_INFO, 0, NULL);
//	}
//	else {
//		// �����ݒ�v��
//		MsgSndFrmPrn(PREQ_CLOCK_REQ, J_PRI, 0);
//	}
	if (jnl_init_sts == 0) {
		if (eja_prn_buff.EjaUseFlg < 0) {
			// �@����v��
			queset(OPETCBNO, PREQ_MACHINE_INFO, 0, NULL);
		}
		else {
			// �n�[�h���Z�b�g
			queset(OPETCBNO, EJA_INIT_WAIT_TIMEOUT, 0, NULL);
		}
	}
	else {
		// �n�[�h���Z�b�g
		MsgSndFrmPrn(PREQ_RESET, J_PRI, 0);
	}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��iSD�J�[�h�}�E���g������҂j
}

//[]----------------------------------------------------------------------[]
///	@brief			�󎚃��b�Z�[�W�đ��M
//[]----------------------------------------------------------------------[]
///	@param[in]		*msg	: �󎚃��b�Z�[�W
///	@return			void
///	@date			2021/02/22
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
void PrnCmd_MsgResend(MSG *msg)
{
	MsgBuf	*msb;

	if (!isEJA_USE()) {
		return;
	}

	if ((msb = GetBuf()) == NULL) {
		return;
	}

	// ���b�Z�[�W���đ��M
	memcpy(&msb->msg, msg, sizeof(MSG));
	PutMsg(PRNTCBNO, msb);
}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
//[]----------------------------------------------------------------------[]
///	@brief			�󎚃f�[�^�L���[�Ƀf�[�^������
//[]----------------------------------------------------------------------[]
///	@param[in]		command	: �󎚗v���R�}���h
///	@param[in,out]	buf		: �󎚃f�[�^�i�Ō�ɃW���uID��t������j
///	@param[in]		size	: �f�[�^�T�C�Y
///	@return			0:�����݂Ȃ��A1�`:�W���uID
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/09
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
ulong PrnDat_WriteQue(ushort command, uchar *buf, ushort size)
{
	ushort	w_counter[3];
	ulong	job_id;

	if (*buf == R_PRI) {
		// ���V�[�g�󎚗v���͏������Ȃ�
		return 0;
	}
	if (command < CMND_CHK2_SNO || CMND_CHK2_ENO <= command) {
		// �󎚗v���R�}���h�ȊO�͏������Ȃ�
		return 0;
	}
	if (command != PREQ_RYOUSYUU) {
		// ����A�̎��؈󎚂̂ݑΏۂƂ���
		// ���󎚂��Ή��؂ɂ���ꍇ�A���̏������R�����g�A�E�g���邱��
		return 0;
	}
	if (size > PRN_QUE_DAT_SIZE) {
		// �T�C�Y�I�[�o�[�̂��߁A�������Ȃ�
		return 0;
	}

	memcpy(w_counter, &prn_dat_que.count, sizeof(w_counter));

	// �L���[FULL�`�F�b�N
	if ((w_counter[0] == PRN_QUE_COUNT_MAX) &&
		(w_counter[1] == w_counter[2])) {
		// �W���[�i�����؂�A�W���[�i���̏ᓙ�ŗ̎��؈󎚕s�̂���
		// �L���[FULL�̃p�^�[���͂Ȃ�
		// ���������ꍇ�͍ŌÃf�[�^��ǂݎ̂Ă�
		if (++w_counter[2] >= PRN_QUE_COUNT_MAX) {
			w_counter[2] = 0;
		}
	}

	// �󎚗v���R�}���h
	prn_dat_que.prn_dat[w_counter[1]].command = command;
	// �W���uID���擾
	job_id = PrnDat_GetJobID();
	memcpy(&buf[PRN_QUE_OFFSET], &job_id, sizeof(job_id));
	// �󎚃f�[�^
	memcpy(prn_dat_que.prn_dat[w_counter[1]].prndata, buf, (size_t)size);
	// ���C�g�|�C���^�X�V
	if (++w_counter[1] >= PRN_QUE_COUNT_MAX) {
		w_counter[1] = 0;
	}
	// �J�E���^�X�V
	if (w_counter[0] < PRN_QUE_COUNT_MAX) {
		w_counter[0]++;
	}

	nmisave(&prn_dat_que.count, w_counter, sizeof(w_counter));

	return job_id;
}

//[]----------------------------------------------------------------------[]
///	@brief			�󎚃f�[�^�L���[����f�[�^�Ǎ���
//[]----------------------------------------------------------------------[]
///	@param[in,out]	msg	: ���b�Z�[�W�o�b�t�@
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/09
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void PrnDat_ReadQue(MSG *msg)
{
	if (msg == NULL) {
		return;
	}

	// �󎚗v���R�}���h
	msg->command = prn_dat_que.prn_dat[prn_dat_que.rdp].command;
	// �󎚃f�[�^
	memcpy(msg->data, prn_dat_que.prn_dat[prn_dat_que.rdp].prndata, sizeof(msg->data));
}

//[]----------------------------------------------------------------------[]
///	@brief			�󎚃f�[�^�L���[����f�[�^1���폜
//[]----------------------------------------------------------------------[]
///	@param[in]		command	: �󎚗v���R�}���h
///	@param[in]		job_id	: �W���uID
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/09
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void PrnDat_DelQue(ushort command, ulong job_id)
{
	ushort	w_counter[3];
	ulong	que_job_id;

	if (prn_dat_que.count == 0) {
		return;
	}

	memcpy(&que_job_id,
			&prn_dat_que.prn_dat[prn_dat_que.rdp].prndata[PRN_QUE_OFFSET],
			sizeof(que_job_id));
	if (command != prn_dat_que.prn_dat[prn_dat_que.rdp].command ||
		job_id != que_job_id) {
		// �󎚗v���R�}���h�A�W���uID���قȂ�
		return;
	}

	memcpy(w_counter, &prn_dat_que.count, sizeof(w_counter));
	// ���[�h�|�C���^�X�V
	if (++w_counter[2] >= PRN_QUE_COUNT_MAX) {
		w_counter[2] = 0;
	}
	// �J�E���^�X�V
	if (w_counter[0] > 0) {
		w_counter[0]--;
	}

	nmisave(&prn_dat_que.count, w_counter, sizeof(w_counter));
}

//[]----------------------------------------------------------------------[]
///	@brief			�󎚃f�[�^�L���[�J�E���g�擾
//[]----------------------------------------------------------------------[]
///	@return			�L���[�J�E���g
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/09
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
ushort PrnDat_GetQueCount(void)
{
	return prn_dat_que.count;
}

//[]----------------------------------------------------------------------[]
///	@brief			�W���uID�擾
//[]----------------------------------------------------------------------[]
///	@return			�W���uID
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/09
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
ulong PrnDat_GetJobID(void)
{
	if (++prn_job_id >= PRN_JOB_ID_MAX) {
		prn_job_id = 1;
	}
	return prn_job_id;
}

//[]----------------------------------------------------------------------[]
///	@brief			PRI�^�X�N�p���b�Z�[�W�N���A
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/04/18
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void PrnMsgBoxClear(void)
{
	// �v�����^�^�X�N�N���O�̈󎚗v����
	// �s�v�ȃ��b�Z�[�W���폜����

	// �̎��؈�
	// pritask�N���O��fukuden()�����s�����
	// �̎��؁i���j�󎚗v�����b�Z�[�W���o�^�����
	// �̎��؈󎚂�prn_dat_que�ɓo�^����Ă��邽�߁A���b�Z�[�W���폜����
	// 2�d�Ɉ󎚂����̂�h��
	Target_MsgGet_delete1(PRNTCBNO, PREQ_RYOUSYUU);
}
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
// GG129000(S) H.Fujinaga 2022/12/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
//[]----------------------------------------------------------------------[]
///	@brief			QR�R�[�h�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		pri_kind		��������<br>
///										R_PRI	  : ���V�[�g<br>
///										J_PRI	  : �W���[�i��<br>
///										RJ_PRI	  : ڼ�ā��ެ���
///	@param[in]		size			���W���[���T�C�Y�F1�`20
///	@param[in]		string			�f�[�^�A�h���X
///	@param[in]		length			�f�[�^��
///	@return			void
///	@attention		None
///	@note			None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2016/03/11<br>
///					Update	:
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
void	QrCode_set( uchar pri_kind, uchar size, char *string, ushort length  )
{
	// QR�R�[�h�󎚃t�H�[�}�b�g�͊����pQR�R�[�h(�u���U�[)�ɍ��킹��B
	// ���̂��ߌŒ�l�ɂ���K�v������G���A�͌Œ�l�Z�b�g�ɕύX�B
	char *ptr;
	
	ptr = (char *)(&Logo_Fdata[0][0]);		//���䂪�d�����邱�Ƃ͂Ȃ��̂Ń��S�̃��[�N���g�p����
	*ptr++ = ESC;
	*ptr++ = 'q';
	*ptr++ = size;							//���W���[���T�C�Y
	*ptr++ = 2;								//���������x��	2:Q �Œ�
	*ptr++ = 9;								//�o�[�W����		9�Œ�
	*ptr++ = 0;								//�}�X�N�p�^�[��	0:���� 1�`8:�C��
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��s����QR�󎚂��x���Ĉ󎚓r���ň���������j�iGM803003���p�j
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
//	*ptr++ = 0;								//�f�[�^���
	if( isNewModel_R() ){
		*ptr++ = 0;							//�f�[�^���
	}
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��s����QR�󎚂��x���Ĉ󎚓r���ň���������j�iGM803003���p�j
	*ptr++ = (char)( length & 0xFF );		//�f�[�^��(L)
	*ptr++ = (char)( length >> 8 );			//�f�[�^��(H)
	memcpy( ptr, string, length );
	ptr+=length;
	PrnCmdLen( (uchar *)Logo_Fdata, (ushort)(ptr - (&Logo_Fdata[0][0])), pri_kind );	// �ҏW���ް��o�^
}

// GG129000(E) H.Fujinaga 2022/12/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
//[]----------------------------------------------------------------------[]
///	@brief			���V�[�g�v�����^���f���擾�^�X�N
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2023/09/22
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void	Prn_GetModel_R( void )
{
	MsgBuf		*msb;											// ��Mү�����ޯ̧�߲��
	MsgBuf		msg;											// ��Mү���ފi�[�ޯ̧
	t_TARGET_MSGGET_PRM	Target_WaitMsgID;

	Target_WaitMsgID.Count = 2;
	Target_WaitMsgID.Command[0] = PREQ_MACHINE_INFO_R;			// ���V�[�g�@����ʒm
	Target_WaitMsgID.Command[1] = RPT_INFORES_WAIT_TIMEOUT;		// ���V�[�g���v���^�C�}

	memset(&pri_rcv_buff_r, 0, sizeof(PRN_RCV_DATA_BUFF_R));

	// �����N���̃f�[�^���Ō�܂Ŏ�M���������邽�߁Await
	wait2us(50000L);											// wait 100ms

	pri_rcv_buff_r.NewModelFlg = -1;
	PrnCmd_InfoReq_R(PINFO_MACHINE_INFO);						// �@����v��

	/*----------------------------------------------*/
	/*	����������������҂�����ٰ��					*/
	/*----------------------------------------------*/
	for( ; ; ){

		taskchg( IDLETSKNO );									// ���������ؑ�
		msb = Target_MsgGet( OPETCBNO, &Target_WaitMsgID);		// ���҂���Ұق�����M�i���͗��߂��܂܁j
		if( NULL == msb ){										// ���҂���ҰقȂ��i����M�j
			continue;											// ��Mү���ނȂ� �� continue
		}

		//	��Mү���ނ���
		memcpy( &msg , msb , sizeof(MsgBuf) );					// ��Mү���ފi�[
		FreeBuf( msb );											// ��Mү�����ޯ̧�J��

		switch( msg.msg.command ){								// ү���޺���ނɂ��e�����֕���

			case PREQ_MACHINE_INFO_R:							// �@����ʒm
				// pri_rcv_buff_r.NewModelFlg �� -1�@�ȊO���Z�b�g����Ă��邽��
				// �����ł͉������Ȃ�
				break;

			case RPT_INFORES_WAIT_TIMEOUT:						// ���V�[�g���v���^�C�}
				pri_rcv_buff_r.NewModelFlg = 0;					// ���V�[�g�v�����^�]�����f��
				break;

			default:
				break;

		}
		if( pri_rcv_buff_r.NewModelFlg != -1 ){					// ���f���m��
			break;												// ���f���擾�^�X�N�𔲂���
		}
	}

	// ���f���`�F�b�N
	Prn_Model_Chk_R();
}

//[]----------------------------------------------------------------------[]
///	@brief			���V�[�g�v�����^���v��
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: �v�����
///	@return			void
///	@date			2023/09/22
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void PrnCmd_InfoReq_R(uchar kind)
{
	uchar	dat[3];
	short	timer;

	// �v�����^���v���iESC s n�j
	memcpy(dat, "\x1b\x73", 2);
	dat[2] = kind;
	PrnCmdLen(dat, 3, R_PRI);

	// �v�����t���OON
	SetPrnInfoReq_R(1);

	timer = 5;
	LagTim500ms(LAG500_RCT_PINFO_RES_WAIT_TIMER, timer, PrnCmd_InfoResTimeout_R);
}

//[]----------------------------------------------------------------------[]
///	@brief			���V�[�g�v�����^��񉞓��҂��^�C���A�E�g
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2023/09/22
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void PrnCmd_InfoResTimeout_R(void)
{

	// �v�����t���OOFF
	SetPrnInfoReq_R(0);
	queset(OPETCBNO, RPT_INFORES_WAIT_TIMEOUT, 0, NULL);
}

//[]----------------------------------------------------------------------[]
///	@brief			�v�����^��񉞓��̃f�[�^�T�C�Y�ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: �v�����
///	@return			�f�[�^�T�C�Y
///	@date			2023/09/26
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
uchar PrnCmd_InfoRes_DataSize_Get_R(uchar kind, uchar* buff)
{
	uchar	size = 2;
// GG129000(S) M.Fujikawa 2024/01/09 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@���V�[�g�v�����^�[�̃��f����񂪎擾�ł��Ȃ��s�
//	uchar	i = 0;
// GG129000(E) M.Fujikawa 2024/01/09 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@���V�[�g�v�����^�[�̃��f����񂪎擾�ł��Ȃ��s�

	switch (kind) {
	case PINFO_MACHINE_INFO:		// �@����v��
// GG129000(S) M.Fujikawa 2024/01/09 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@���V�[�g�v�����^�[�̃��f����񂪎擾�ł��Ȃ��s�
//		// �I�[�����܂ł̃T�C�Y
//		for ( i = 0; i < PRN_RCV_BUF_SIZE_R; i++ ) {
//			if (buff[i] == 0x00) {	// �I�[�����܂ł̃T�C�Y
//				size += i;
//				break;
//			}
//		}
		size += 8;		// ���f�����7�P�^+�I�[NULL��1�P�^
// GG129000(E) M.Fujikawa 2024/01/09 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@���V�[�g�v�����^�[�̃��f����񂪎擾�ł��Ȃ��s�
		break;
	default:
		break;
	}
	return size;
}

//[]----------------------------------------------------------------------[]
///	@brief			���V�[�g�v�����^��񉞓���M����
//[]----------------------------------------------------------------------[]
///	@param[in]		*pBuf	: ��M�f�[�^
///	@param[in]		RcvSize	: ��M�f�[�^�T�C�Y
///	@return			0=��M������, 1=��M����
///	@date			2023/09/26
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
uchar PrnCmd_InfoRes_Proc_R(uchar *pBuf, ushort RcvSize)
{
	PRN_RCV_DATA_BUFF_R	*p = &pri_rcv_buff_r;
	uchar	ret = 0;
	uchar	rp, DatSize;

	// ��M�f�[�^�Z�b�g
	rp = p->PrnRcvBufReadCnt;
	memcpy(&p->PrnRcvBuf[rp], pBuf, (size_t)RcvSize);
	p->PrnRcvBufReadCnt += (uchar)RcvSize;

	// �v�����^��񉞓�
	// 1byte�ځ�0xFF�i���V�[�g�j
	// 2byte�ځ��v�����
	// 3byte�ڈȍ~���e�f�[�^
	if (p->PrnRcvBufReadCnt >= 2) {
		// ��M�f�[�^�T�C�Y�擾
		DatSize = PrnCmd_InfoRes_DataSize_Get_R(p->PrnRcvBuf[1], &p->PrnRcvBuf[2]);
		if (p->PrnRcvBufReadCnt >= DatSize) {
			// ��M����
			LagCan500ms(LAG500_RCT_PINFO_RES_WAIT_TIMER);
			SetPrnInfoReq_R(0);
			ret = 1;
		}
	}


	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�v�����^��񉞓����b�Z�[�W
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2020/11/16
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PrnCmd_InfoRes_Msg_R(void)
{
	PRN_RCV_DATA_BUFF_R	*p = &pri_rcv_buff_r;
	ushort	msg;
	uchar	*pBuf;

	pBuf = &p->PrnRcvBuf[2];
	switch (p->PrnRcvBuf[1]) {
	case PINFO_MACHINE_INFO:		// �@����v��
		msg = PREQ_MACHINE_INFO_R;
		if (!memcmp(pBuf, PRN_MACHINE_NEWMODEL_R, 7)) {
			// �V���f��
			pri_rcv_buff_r.NewModelFlg = 1;
		}
		else {
			// �]�����f��
			pri_rcv_buff_r.NewModelFlg = 0;
		}
		break;
	default:
		return;
	}
	queset(OPETCBNO, msg, 0, NULL);
}

//[]----------------------------------------------------------------------[]
///	@brief			���V�[�g�v�����^�擾���f���`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2023/09/22
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void Prn_Model_Chk_R(void)
{
	uchar chk_model;

	// �p�����[�^��胂�f���ݒ�擾
	// 0=�]��
	// 1=�V
// GG129000(S) M.Fujikawa 2023/10/17 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�s�#7169�Ή�
//	chk_model = (uchar)prm_get( COM_PRM, S_PAY, 22, 1, 1 );
	chk_model = (uchar)prm_get( COM_PRM, S_PAY, 22, 1, 4 );
// GG129000(?) M.Fujikawa 2023/10/17 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�s�#7169�Ή�
	// �͈͊O��0=�]���Ƃ���
	if( chk_model > 1 ){
		chk_model = 0;
	}

	// ���f���ݒ�Ǝ擾���f�����s��v�ł���΃G���[�t���OON
	// ��v���Ă���΃G���[�t���OOFF
	if( pri_rcv_buff_r.NewModelFlg == chk_model ){
		SetModelMiss_R(0);
	}
	else{
		SetModelMiss_R(1);
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�y�[�W���C�A�E�g�w��R�}���h
//[]----------------------------------------------------------------------[]
///	@param[in]		pri_kind�F�v�����^���
///	@param[in]		dt		: ���
///							: 0x00=�y�[�W�c�����i��]0�x�j
///							: 0x01=�y�[�W�������i��]90�x�j
///							: 0x02=�y�[�W�c�����i��]180�x�j
///							: 0x03=�y�[�W�������i��]270�x�j
///	@return			void
///	@date			2023/09/29
///	@note			�y�[�W���ƃy�[�W�󎚗̈�ݒ�����������܂��B
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void	PrnCmd_PLayout_Design( uchar pri_kind, uchar dt )
{
	uchar	work[3];					// �߰��ڲ��Ďw������ް��ҏWܰ�

	work[0] = 0x1b;						// "ESC"
	work[1] = 0x54;						// "T"
	work[2] = dt;						// ���(hex)
	PrnCmdLen( work , 3 , pri_kind );	// [�߰��ڲ��Ďw��]�i������������ޕҏW�j
}

//[]----------------------------------------------------------------------[]
///	@brief			�V�v�����^�N���A�����i�o�b�t�@�N���A�{�y�[�W���C�A�E�g�������j
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2023/09/29
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void	Prn_NewPrint_Clear_R( void )
{
	// �y�[�W���C�A�E�g�w��i��]0�x�j
	PrnCmd_PLayout_Design(R_PRI, 0x00);
	// �y�[�W�o�b�t�@�N���A
	PCMD_WRITE_RESET(R_PRI);
	// �f�[�^�������ݏI��
	PCMD_WRITE_END(R_PRI);
	// ������N���iڼ����������M������Enable�j
	PrnOut( R_PRI );
}

//[]----------------------------------------------------------------------[]
///	@brief			�X�g�b�N�f�[�^�󎚊J�n
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2023/09/29
///	@note			End_Set_R �Ɠ��l�̏���
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void PrnRYOUSYUU_StockPrint( void )
{
	Rct_top_edit( ON );					// ڼ�Đ擪���ް��ҏW�����iۺވ󎚁^ͯ�ް�󎚁^�p����Đ���j

	// �f�[�^�������ݏI��
	PCMD_WRITE_END(R_PRI);

	PrnOut( R_PRI );					// ������N���iڼ����������M������Enable�j
	PrnEndMsg(	PREQ_RYOUSYUU,			// �u�󎚏I���i����I���j�vү���ޑ��M
				PRI_NML_END,
				PRI_ERR_NON,
				R_PRI );
}

//[]----------------------------------------------------------------------[]
///	@brief			�X�g�b�N�f�[�^�N���A
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@date			2023/09/29
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
void PrnRYOUSYUU_StockClear( void )
{
	Prn_NewPrint_Clear_R();				// �V�v�����^�N���A����
	prn_proc_data_clr_R();				// ڼ�Ĉ󎚕ҏW����ر������
	PrnEndMsg(	PREQ_RYOUSYUU,			// �u�󎚏I���i����I���j�vү���ޑ��M
				PRI_NML_END,
				PRI_ERR_NON,
				R_PRI );
}
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j