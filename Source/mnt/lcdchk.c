/*[]----------------------------------------------------------------------[]*/
/*| LCD����                                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Y.Tanaka                                                 |*/
/*| Date        : 2005.05.05                                               |*/
/*| UpDate      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"Message.h"
#include	"lcd_def.h"
#include	"Strdef.h"
#include	"mnt_def.h"
#include	"LKmain.H"
#include	"PRM_TBL.H"
#include	"can_def.h"

#define	LCDBM_WAIT_TIMEOUT	5		// LCD���W���[���Ƃ̒ʐM�^�C���A�E�g���ԁi"���΂炭���҂�������"��\������ő厞�ԁj

static void	FColorLCDChk_TestPtnMenu(char pos, ushort rev);
static unsigned short	FColorLCDChk_TestPtnDsp(char ptn);
static unsigned short FColorLCDChk_GetColorPtn(char ptn);

/*[]----------------------------------------------------------------------[]*/
/*| LCD�`�F�b�N���j���[���                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Lcd( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
unsigned short	FunChk_Lcd( void ){

	unsigned short	usFlcdEvent;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; )
	{

		dispclr();

		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_STR[0] );					/* "���k�b�c�`�F�b�N���@�@�@�@�@�@�@" */
		usFlcdEvent = Menu_Slt( FLCDMENU, FLCD_CHK_TBL, (char)FLCD_CHK_MAX2, (char)1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usFlcdEvent ){
//���g�p�ł��iS�j
			case FLCD1_CHK:
				usFlcdEvent = FColorLCDChk_Bk();				// �ޯ�ײ�ý�
				break;
//���g�p�ł��iE�j
			case FLCD2_CHK:
				usFlcdEvent = FColorLCDChk_Lumine_change();		// �P�x����
				break;
//���g�p�ł��iS�j
			case FLCD3_CHK:
				usFlcdEvent = FColorLCDChk_PatternDSP();		// �\����ý�
				break;
			case FLCD4_CHK:
				//usFlcdEvent = FLcdChk_T2();
				break;
//���g�p�ł��iE�j
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return usFlcdEvent;
				break;
			default:
				break;
		}
		if( OpelogNo ){
			wopelg( OpelogNo, 0, 0 );							// ���엚��o�^
			OpelogNo = 0;
			SetChange = 1;
			UserMnt_SysParaUpdateCheck( OpelogNo );
			SetChange = 0;
		}
		
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if( usFlcdEvent == MOD_CHG){
		if( usFlcdEvent == MOD_CHG || usFlcdEvent ==  MOD_CUT ){
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return usFlcdEvent;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}


//[]----------------------------------------------------------------------[]
///	@brief			�\����e�X�g
//[]----------------------------------------------------------------------[]
///	@return			ret		MOD_CHG : mode change<br>
///							MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
///	@author			sekiguchi
///	@date			Create	: 2010/03/25<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
unsigned short	FColorLCDChk_PatternDSP(void)
{
	//�ϐ�
	short pos = 0;
	short	msg;					// ��M���b�Z�[�W
	unsigned short	ret;			// �߂�l

	//�����e�i���X��ʕ\��
	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_STR[18]);	// "���\����e�X�g���@�@�@�@�@�@�@"
	grachr(2,  6, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[0]);			// "   ��   "
	grachr(2, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[1]);			// "   ��   "
	grachr(3,  6, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[2]);			// "   ��   "
	grachr(3, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[3]);			// "   ��   "
	grachr(4,  6, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[4]);			// "   ��   "
	grachr(4, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[5]);			// "   ��   "
	FColorLCDChk_TestPtnMenu(pos,1);										// ���]
	Fun_Dsp( FUNMSG2[46] );													// "�@���@�@���@ �@�@�@�\��  �I�� "

	//����
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );

		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				dsp_background_color(COLOR_WHITE);
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:	// ���[�h�`�F���W
				BUZPI();
				dsp_background_color(COLOR_WHITE);
				return MOD_CHG;

			case KEY_TEN_F5:	// �I��
				BUZPI();
				dsp_background_color(COLOR_WHITE);
				return MOD_EXT;

			case KEY_TEN_F1:
			case KEY_TEN_F2:
				//��x�w�i��ύX����Ɖ�ʂ𔲂���܂ő��삳���Ȃ�
				BUZPI();
				FColorLCDChk_TestPtnMenu(pos,0);			// ���]
				if(msg == KEY_TEN_F1){
					pos--;
					if(pos < 0){
						pos = 5;
					}
				}
				else{
					pos++;
					if(pos > 5){
						pos = 0;
					}
				}
				FColorLCDChk_TestPtnMenu(pos,1);			// ���]
				break;
			
			case KEY_TEN_F4:
				BUZPI();
				// �J���[LCD�\��
				ret = FColorLCDChk_TestPtnDsp(pos);							// �e�X�g�\��
				if(ret == MOD_CHG) {
					dsp_background_color(COLOR_WHITE);
					return MOD_CHG;
				}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				if( ret == MOD_CUT) {
					dsp_background_color(COLOR_WHITE);
					return MOD_CUT;
				}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				else if(ret == MOD_EXT) {
					dsp_background_color(COLOR_WHITE);
					dispclr();
					grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_STR[18]);	// "���\����e�X�g���@�@�@�@�@�@�@"
					grachr(2,  6, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[0]);			// "   ��   "
					grachr(2, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[1]);			// "   ��   "
					grachr(3,  6, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[2]);			// "   ��   "
					grachr(3, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[3]);			// "   ��   "
					grachr(4,  6, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[4]);			// "   ��   "
					grachr(4, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[5]);			// "   ��   "
					FColorLCDChk_TestPtnMenu(pos,1);									// ���]
					Fun_Dsp( FUNMSG2[46] );												// "�@���@�@���@ �@�@�@�\��  �I�� "
				}
				break;
			default:
				break;
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�o�b�N���C�g�e�X�g
//[]----------------------------------------------------------------------[]
///	@return			MOD_CHG	: <br>
///					MOD_EXT	: 
//[]----------------------------------------------------------------------[]
///	@author			ichihara
///	@date			Create	: 2010/03/25<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
unsigned short	FColorLCDChk_Bk( void )
{

	char	val1 = 1;
	short			msg;

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_STR[19] );		/* // [19]	"���o�b�N���C�g�e�X�g���@�@�@�@" */
	grachr( 2,  0, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_STR[20] );		/* // [20]	"�@�o�b�N���C�g�@�@�@�@�@�@�@�@" */
	grachr( 2,  16, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT3_0[val1?0:1] );	/* // [XX]	"�@�@�@�@�@�@�@�@�@�n�m�@�@�@�@" */
	Fun_Dsp( FUNMSG[50] );														/* // [50]	"�@�@�@�@�@�@�@�@�@ ON/OF �I�� " */


	for( ; ; ){
		msg = StoF( GetMessage(), 1 );
		switch( msg ){							/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				lcd_backlight( (char)1 ); 		// ON
				return( MOD_CUT );
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				BUZPI();
												/* �������Ұ�43-21�擾 */
				lcd_backlight( (char)1 ); 		// ON
				return( MOD_CHG );
				break;
			case KEY_TEN_F5:					/* F5:Exit */
				BUZPI();
												/* �������Ұ�43-21�擾 */
				lcd_backlight( (char)1 ); 		// ON
				return( MOD_EXT );
				break;
			case KEY_TEN_F4:					/* F4:Update */
				BUZPI();
												/* ON/OFF */
				val1 = (0==val1) ? 1 : 0;
				lcd_backlight( (char)val1 );
				break;
			default:
				break;
		}
		grachr( 2,  16, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT3_0[val1?0:1] );
	}

}

//[]----------------------------------------------------------------------[]
///	@brief			�P�x����
//[]----------------------------------------------------------------------[]
///	@return			ret		MOD_CHG : mode change<br>
///							MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
///	@author			sekiguchi
///	@date			Create	: 2010/03/25<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
unsigned short	FColorLCDChk_Lumine_change(void)
{
	ushort	msg;
	int		lumine_change = 0;
	uchar	side;								// 0:�_��(���Z����鎞�ԑ�)�C1:����(�ҋ@��������ԑ�)
	unsigned short	lumine_num[2];						// [0]:�_�����̋P�x�C[1]:�������̋P�x
	int		mode_change = 0;
	ushort	column;								// �`��ʒu
	side = 0;
	lumine_num[0] = (ushort)prm_get(COM_PRM, S_PAY, 27, 2, 1);	// 02-0027�D�E
	lumine_num[1] = (ushort)prm_get(COM_PRM, S_PAY, 28, 2, 1);	// 02-0028�D�E
	/*	�s���ȋP�x���擾�����ꍇ */
	if( lumine_num[side] > LCD_LUMINE_MAX )
	{
		lumine_num[side] = LCD_LUMINE_MAX;
	}

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_LUMINSTR[0]);	// "���P�x�������@�@�@�@�@�@�@�@�@"
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_LUMINSTR[7]);	// "�����L�[�ŋP�x��ݒ肵�ĉ�����"
	displclr(3);
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_LUMINSTR[8]);	// "�P�x�ݒ�F�@�@�@�@�@�@�@�@�@�@"
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CLRLCD_LUMINSTR[9]);	// "0 �@�@�@�@�@�@�@�@�@�@�@�@�@15"
	Fun_Dsp(FUNMSG2[2]);						// "�@���@�@���@�@�@�@ ����  �I�� "
	lumine_change = 1;

	while (1) {
		msg = StoF(GetMessage(), 1);											// ү���ގ�M

		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			mode_change = 2;	// �u���샂�[�h�ؑփC�x���g��M�ς݁v�Ƃ���
			break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:		// ���샂�[�h�ؑ�
			mode_change = 1;	// �u���샂�[�h�ؑփC�x���g��M�ς݁v�Ƃ���
			break;

		case KEY_TEN_F4:														// ����
			BUZPI();
			CPrmSS[S_PAY][27] = (unsigned char)(lumine_num[side]);				// CPrmSS & Flash Data Update
			OpelogNo = OPLOG_CONTRAST;				// ���엚��o�^
			f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
// MH810100(S) Y.Yamauchi 2020/02/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
 			mnt_SetFtpFlag( FTP_REQ_NORMAL );
// MH810100(E) Y.Yamauchi 2020/02/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
			mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�

			break;

		case KEY_TEN_F5:														// �I��
			BUZPI();
			// �ݒ�l��LCD���W���[���ɔ��f
			lcd_contrast((unsigned char)prm_get(COM_PRM, S_PAY, 27, 2, 1));
			Lagcan( OPETCBNO, 6 );
			return MOD_EXT;
			break;
		case KEY_TEN_F1:						// ��
			BUZPI();
			// �v���O���X�o�[���ֈړ�
			if (lumine_num[side] > 0) {
				lumine_num[side]--;
				lumine_change = 1;
			}
			break;
		case KEY_TEN_F2:						// ��
			BUZPI();
			if (lumine_num[side] < LCD_LUMINE_MAX ) {
				lumine_num[side]++;
				lumine_change = 1;
			}
			break;
		default:
			break;
		}

		if (lumine_change) {
			lcd_contrast((unsigned char)lumine_num[side]);
			backlight = lumine_num[side];
			opedsp(4, 10, (ushort)lumine_num[side], 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );					// �P�x
			if (lumine_num[side] < LCD_LUMINE_MAX) {
				dsp_background_color(COLOR_DARKORANGE);
				column = (ushort)(lumine_num[side]*2);
				if(column) {
					grachr(6 , 0, column, 0, COLOR_DARKORANGE, LCD_BLINK_OFF, UMSTR3[154]);	// �v���O���X�o�[�\��
				}
				dsp_background_color(COLOR_WHITE);
				if(column < 30) {
					grachr(6, column, (ushort)(30 - column), 0, COLOR_DARKORANGE, LCD_BLINK_OFF, UMSTR3[154]);
				}
			} else {
				dsp_background_color(COLOR_DARKORANGE);
				grachr(6 , 0, 30, 0, COLOR_DARKORANGE, LCD_BLINK_OFF, UMSTR3[154]);			// �v���O���X�o�[�\��
				dsp_background_color(COLOR_WHITE);
				lumine_num[side] = LCD_LUMINE_MAX;
			}
			lumine_change = 0;
		}
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if ( mode_change != 0 ) {
//			return MOD_CHG;
//		} 
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)	
		if ( mode_change == 1 ) {
			return MOD_CHG;
		} else if( mode_change == 2 ) {
			return MOD_CUT;
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�\����e�X�g�p�^�[���\��
//[]----------------------------------------------------------------------[]
///	@param[in]		pos		: �\���J�n�ʒu
///	@param[in]		rev		: ���]�w��
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			sekiguchi
///	@date			Create	: 2010/03/25<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
static void	FColorLCDChk_TestPtnMenu(char pos, ushort rev)
{
	switch(pos){
	case 0:
		grachr(2,  6, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[0]);				// "   ��   "
		break;

	case 1:
		grachr(2, 16, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[1]);				// "   ��   "
		break;

	case 2:
		grachr(3,  6, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[2]);				// "   ��   "
		break;

	case 3:
		grachr(3, 16, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[3]);				// "   ��   "
		break;

	case 4:
		grachr(4,  6, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[4]);				// "   ��   "
		break;

	case 5:
		grachr(4, 16, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[5]);				// "   ��   "
		break;

	case 6:
		grachr(5,  6, 8, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT4_6[5]);				// "�p�^�[��"
		break;
	default:
		break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�\����e�X�g�p�^�[���\��
//[]----------------------------------------------------------------------[]
///	@param[in]		ptn		: �e�X�g�p�^�[��
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			sekiguchi
///	@date			Create	: 2010/03/25<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
static unsigned short	FColorLCDChk_TestPtnDsp(char ptn)
{
	t_DispFont		DispFont;
	t_DispColor		DispColor;
	t_DispStr		DispStr;
	unsigned short	color;
	unsigned char	line;
	char			i;
	unsigned char	column;
	ushort	msg;

	if (ptn < 6) {
		// ���A�ԁA�A���A�΁A��
		color = FColorLCDChk_GetColorPtn(ptn);
		dsp_background_color( color );
		dispclr();											//��ʃN���A
	} else {
		// �����
		for( line = 0; line < 8; line++ ) {
			// �c8ײݕ�
			i = 0;
			for( column = 0; column < LCD_CLM; column++ ) {
				// 30����
				color = FColorLCDChk_GetColorPtn(i);

				// �����t�H���g�w��
				memset(&DispFont, 0, sizeof(t_DispFont));
				DispFont.Esc = LCD_ESC_FONTTYPE;					// ����̫�Ďw��
				DispFont.Type = LCD_NORMAL_FONT;					// ɰ��̫��
				DispFontDataSnd(&DispFont);

				// �����J���[�w��
				memset(&DispColor, 0, sizeof(t_DispColor));
				DispColor.Esc = LCD_ESC_FONTCOLR;					// �����װ�w��
				DispColor.Color = color;							// �F�w��
				DispColorDataSnd( &DispColor );

				// ������w��
				memset(&DispStr, 0, sizeof(t_DispStr));
				DispStr.Esc = LCD_ESC_STR;							// ������w��
				DispStr.Row = line;									// �w��s
				DispStr.Colmun = 0;									// �w����
				memcpy( &DispStr.Msg, "  ", 2 );
				can_snd_data4((void *)&DispStr, 4 + 2);

				if( i < 5 ) {
					i++;
				} else {
					i = 0;
				}
			}
		}
		dispclr();											//��ʃN���A
	}
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );

		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:	// ���[�h�`�F���W
			BUZPI();
			return MOD_CHG;
		default:
			if(msg >= KEY_TEN0 && msg <= KEY_TEN_CL) {		// �e���L�[�őO�̉�ʂɖ߂�
				BUZPI();
				return MOD_EXT;
			}
			break;
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�F���(RGB)�擾
//[]----------------------------------------------------------------------[]
///	@param[in]		ptn		: �e�X�g�p�^�[��
///	@return			unsigned short �F �F���(RGB)
//[]----------------------------------------------------------------------[]
///	@author			sekiguchi
///	@date			Create	: 2010/03/25<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
static unsigned short FColorLCDChk_GetColorPtn(char ptn)
{
	unsigned short	color;

	switch(ptn){
	case 0:			// ��
		color = COLOR_YELLOW;
		break;

	case 1:			// ��
		color = COLOR_RED;
		break;

	case 2:			// ��
		color = COLOR_BLUE;
		break;

	case 3:			// ��
		color = COLOR_BLACK;
		break;

	case 4:			// ��
		color = COLOR_GREEN;
		break;

	case 5:			// ��
		color = COLOR_WHITE;
		break;

	case 6:			// �����
	default:
		return 0;
	}
	return color;
}

