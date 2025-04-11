/*[]----------------------------------------------------------------------[]*/
/*| Display function                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Y.Tanaka                                                 |*/
/*| Date        : 2005-03-29                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update      : �V���Z�@�Ή� 2012-12-15 k.totsuka                        |*/
/*| Date        :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdio.h>
#include	"system.h"
#include	"lcd_def.h"
#include	"iodefine.h"
#include	"strdef.h"
#include	"prm_tbl.h"

#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"mem_def.h"
#include	"ope_def.h"
#include	"tbl_rkn.h"
#include	"can_def.h"
// MH810100(S) 
#include	"pktctrl.h"
// MH810100(E) 

t_blk_lg LCD_Blk_lg[LCD_BLK_LG];			/* Blink area register */
unsigned char	LCD_BLKLG_CNT;				/* Blink area count */
unsigned short	DspSts;						// ܰ�ݸ�ү����7�s�ڕ\�����	( LCD_WMSG_ON=�\���� / LCD_WMSG_OFF=���\�� )
unsigned short	DspSts2;					// ܰ�ݸ�ү����6�s�ڕ\�����	( LCD_WMSG_ON=�\���� / LCD_WMSG_OFF=���\�� )
T_WMSG_CNT		Lcd_Wmsg_Cnt[LCD_CLM];		// ܰ�ݸޕ\������ر

unsigned char	LCD_BACKUP_MSG6[LCD_CLM];	// 6Line���ޯ����ߕ\���ް�
unsigned short	LCD_BACKUP_COLOR6[LCD_CLM];	// 6Line���ޯ����ߕ\���F
unsigned char	LCD_BACKUP_MSG7[LCD_CLM];	// 7Line���ޯ����ߕ\���ް�
unsigned short	LCD_BACKUP_COLOR7[LCD_CLM];	// 7Line���ޯ����ߕ\���F
unsigned char	LCD_BACKUP_Flag[8];


/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : dispinit                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : M66273AFP(Display Controler) initialization             |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : char result = 0:OK/ 1:NG                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanaka                                                |*/
/*| Date         : 2005-03-29                                              |*/
/*| Update       : 2005-06-24 B.Miyamoto                                   |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	dispinit( void )
{
	unsigned char	i;

	// ܰ�ݸޕ\������ر������
	DspSts = LCD_WMSG_OFF;								// ܰ�ݸ�ү���ޕ\�����	�F�n�e�e
	DspSts2 = LCD_WMSG_OFF;								// ܰ�ݸ�ү���ޕ\�����	�F�n�e�e
	for( i=0; i<LCD_CLM; i++){
		Lcd_Wmsg_Cnt[i].WarMsg = 0x20;					// ܰ�ݸ�ү���ޕ\���ް�	�F�ر
		Lcd_Wmsg_Cnt[i].WarColor = 0;					// ܰ�ݸ�ү���ޕ\���F	�F�ر

		Lcd_Wmsg_Cnt[i].RecMsg = 0x20;					// �ʏ�\���ް�			�F�ر
		Lcd_Wmsg_Cnt[i].RecColor = 0;					// �ʏ�\���F			�F�ر

		Lcd_Wmsg_Cnt[i].WarMsg2 = 0x20;					// ܰ�ݸ�ү���ޕ\���ް�	�F�ر
		Lcd_Wmsg_Cnt[i].WarColor2 = 0;					// ܰ�ݸ�ү���ޕ\���F	�F�ر

		Lcd_Wmsg_Cnt[i].RecMsg2 = 0x20;					// �ʏ�\���ް�			�F�ر
		Lcd_Wmsg_Cnt[i].RecColor2 = 0;					// �ʏ�\���F			�F�ر
	}

	LCD_BLKLG_CNT = 0;

	memset ( &LCD_BACKUP_Flag[0], 0 , sizeof(LCD_BACKUP_Flag));

	Disp_color_font_now.Color	= 0xFFFE;				//���ݐݒ蒆�̃J���[�ƃt�H���g��ʂ�������
	Disp_color_font_now.Type 	= 0xFF;

	memset(Disp_blink_now, 0xFF, sizeof(Disp_blink_now));

	return 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : dispclr                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : Display All Clear                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanaka                                                |*/
/*| Date         : 2005-03-29                                              |*/
/*| Update       : 2005-06-24 B.Miyamoto                                   |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	dispclr( void )
{
// MH810100(S) K.Onodera 2019/12/17 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
#if GT4100_NO_LCD
// MH810100(E) K.Onodera 2019/12/17 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
	t_DispClr DispClr;
	unsigned char	i;

	memset( &DispClr, 0, sizeof(t_DispClr));
	DispClr.Esc = LCD_ESC_CLEAR;							// �ر�w��
	DispClr.Row = 0;										// �w��s
	DispClr.Colmun = 0;										// �w����
	DispClr.Scope = 1;										// �S��ʸر
	can_snd_data4((void *)&DispClr, sizeof(DispClr));

	for( i=0; i<LCD_CLM; i++){
		Lcd_Wmsg_Cnt[i].RecMsg = 0x20;						// �ʏ�\���ް�	�F�ر
		Lcd_Wmsg_Cnt[i].RecColor = 0;						// �ʏ�\���F	�F�ر

		Lcd_Wmsg_Cnt[i].RecMsg2 = 0x20;						// �ʏ�\���ް�	�F�ر
		Lcd_Wmsg_Cnt[i].RecColor2 = 0;						// �ʏ�\���F	�F�ر
	}

	if( DspSts == LCD_WMSG_ON ){
		// ܰ�ݸ�ү���ޕ\�����@���@�\���ʂ��ʏ�\���ʁi���Z�\���ʁj
		Lcd_Wmsg_Disp();									// ܰ�ݸ�ү���ޕ\��
	}
	if( DspSts2 == LCD_WMSG_ON ){
		// ܰ�ݸ�ү���ޕ\�����@���@�\���ʂ��ʏ�\���ʁi���Z�\���ʁj
		Lcd_Wmsg_Disp2();									// ܰ�ݸ�ү���ޕ\��
	}
// MH810100(S) K.Onodera 2019/12/17 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
#else
	// �����e���̂ݕ\�����s��
	if( OPECTL.Mnt_mod == 0 ){
		return;
	}
	PKTcmd_text_1_clear( 0, 0, 1 );	// �S�N���A
// MH810100(S) Y.Yamauchi 2020/03/18 �Ԕԃ`�P�b�g���X(#4036 �����ݒ�̓X������X001����005�܂Őݒ肷��ƁA��ʑ���s�\�ɂȂ�)
	taskchg( IDLETSKNO );
// MH810100(E) Y.Yamauchi 2020/03/18 �Ԕԃ`�P�b�g���X(#4036 �����ݒ�̓X������X001����005�܂Őݒ肷��ƁA��ʑ���s�\�ɂȂ�)
#endif
// MH810100(E) K.Onodera 2019/12/17 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : displclr                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : Display Line Clear                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned short line	 = 0-7                             |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanaka                                                |*/
/*| Date         : 2005-03-29                                              |*/
/*| Update       : 2005-06-24 B.Miyamoto                                   |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	displclr( unsigned short lin )
{
// MH810100(S) K.Onodera 2019/12/17 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
#if GT4100_NO_LCD
// MH810100(E) K.Onodera 2019/12/17 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
	t_DispClr DispClr;
	unsigned char	i;

	memset( &DispClr, 0, sizeof(t_DispClr));
	DispClr.Esc = LCD_ESC_CLEAR;							// �ر�w��
	DispClr.Row = (unsigned char)lin;						// �w��s
	DispClr.Colmun = 0;										// �w����
	DispClr.Scope = 0;										// 1�s�N���A
	can_snd_data4((void *)&DispClr, sizeof(DispClr));

	if( lin == LCD_WMSG_LINE ){
		// ܰ�ݸ�ү���ޕ\���s�@�̏ꍇ
		for( i=0; i<LCD_CLM; i++){
			Lcd_Wmsg_Cnt[i].RecMsg = 0x20;					// �ʏ�\���ް�	�F�ر
			Lcd_Wmsg_Cnt[i].RecColor = 0;					// �ʏ�\���F	�F�ر
		}
	}

	if( DspSts == LCD_WMSG_ON ){
		// ܰ�ݸ�ү���ޕ\�����@���@�\���ʂ��ʏ�\���ʁi���Z�\���ʁj
		Lcd_Wmsg_Disp();									// ܰ�ݸ�ү���ޕ\��
	}

	if( lin == LCD_WMSG_LINE2 ){
		// ܰ�ݸ�ү���ޕ\���s�@�̏ꍇ
		for( i=0; i<LCD_CLM; i++){
			Lcd_Wmsg_Cnt[i].RecMsg2 = 0x20;					// �ʏ�\���ް�	�F�ر
			Lcd_Wmsg_Cnt[i].RecColor2 = 0;					// �ʏ�\���F	�F�ر
		}
	}

	if( DspSts2 == LCD_WMSG_ON ){
		// ܰ�ݸ�ү���ޕ\�����@���@�\���ʂ��ʏ�\���ʁi���Z�\���ʁj
		Lcd_Wmsg_Disp2();									// ܰ�ݸ�ү���ޕ\��
	}
// MH810100(S) K.Onodera 2019/12/17 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
#else
	// �����e���̂ݕ\�����s��
	if( OPECTL.Mnt_mod == 0 ){
		return;
	}
	PKTcmd_text_1_clear( lin, 0, 0 );	// �s�N���A
#endif
// MH810100(E) K.Onodera 2019/12/17 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : dispmlclr                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : Display Multi Line Clear                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : from = start line                                       |*/
/*|                to   = end line                                         |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida.k                                               |*/
/*| Date         : 2005-11-30                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	dispmlclr( unsigned short from, unsigned short to )
{
	while( from <= to ){
		displclr( from++ );
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : grachr                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : Display Graphic Character Disp (8LineMode)              |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned short lin	line = 0-7(8line mode) / 0-6(7)    |*/
/*|              : unsigned short col	colmn No. = 0-29th                 |*/
/*|              : unsigned short cnt	character number = 1-30            |*/
/*|              : unsigned short mod	mode = 0(normal) / 1(reverse)      |*/
/*|              : unsigned short color	character color                    |*/
/*|              : unsigned short blink	blink = 0(none) - blink time(10ms) |*/
/*|              : unsigned short * dat	data(GDC character codes)          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanaka                                                |*/
/*| Date         : 2002-08-26                                              |*/
/*| Update       : 2005-06-24 B.Miyamoto                                   |*/
/*| Update       : font_change 2005-09-01 T.Hashimoto                      |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	grachr( unsigned short lin,
					unsigned short col,
					unsigned short cnt,
					unsigned short mod,
					unsigned short color,
					unsigned short blink,
					const unsigned char *dat )
{
	t_DispFont		DispFont;
	t_DispColor		DispColor;
	t_DispBlink		DispBlink;
	t_DispStr		DispStr;
	char i, j, cursor;



	memset(&DispFont, 0, sizeof(t_DispFont));
	memset(&DispColor, 0, sizeof(t_DispColor));
	memset(&DispBlink, 0, sizeof(t_DispBlink));
	memset(&DispStr, 0, sizeof(t_DispStr));

// MH810100(S) 
#if GT4100_NO_LCD
// MH810100(E) 
	/* �󔒂𔽓]�w�肳�ꂽ�ꍇ�͔w�i�F��ς��ċ󔒕\������ */
	cursor = 0;
	if(mod != 0){					// ���]�w��
		for(i=0; i<cnt; i+=2){
			if(dat[i] == 0x81){
				if(dat[i+1] == 0x40){
					continue;
				}
			}
			break;
		}
		if( i >= cnt){					// �S�ċ�
			dsp_background_color(LCD_CURSOR_COLOR);
			cursor = 1;
		}
	}

	// �����t�H���g�w��
	DispFont.Esc = LCD_ESC_FONTTYPE;						// ����̫�Ďw��
	DispFont.Type = LCD_NORMAL_FONT;						// ɰ��̫��
	DispFontDataSnd(&DispFont);

	// �u�����N�w��
	DispBlink.Esc = LCD_ESC_BLINK;						// ���ݸ�w��
	DispBlink.Row = (unsigned char)lin;					// �s�w��
	DispBlink.Colmun = 0;								// ��w��(�ݒ薳��)
	if(blink == 0) {
		DispBlink.Mode = 0;								// ���ݸ����
		DispBlink.Interval = 0;							// ���ݸ�Ԋu
	} else {
		DispBlink.Mode = 1;								// ���ݸ�J�n
		DispBlink.Interval = (unsigned char)blink;		// ���ݸ�Ԋu
	}
	DispBlinkDataSnd(&DispBlink);

	// ܰ�ݸޕ\������
/* 	ܰ�ݸޕ\������ܰ�ݸޕ\���s(6,7�s��)�ɒʏ�\�����悤�Ƃ����ꍇ�A����̏����ł�
	�@�ȑO�̒ʏ탁�b�Z�[�W���J�o�� 
	�Aܰ�ݸލĕ\��	���I��
	�܂�\�����悤�Ƃ����ʏ탁�b�Z�[�W�͕\������Ȃ�
	���̏����̈Ӑ}���s�������Ƃ肠�����@�̏����͕s�v�Ǝv���̂ō폜
	���̏ꍇ�̏����́A�u�ʏ�\�����悤�Ƃ������b�Z�[�W�����J�o���G���A�ɃZ�b�g���ĕ\���͍X�V���Ȃ��v�悤�ɏC������		*/
	// �����J���[�w��
	DispColor.Esc = LCD_ESC_FONTCOLR;					// �����װ�w��
	/* mod��normal�w��̏ꍇ */
	if( 0 == mod )
	{
		DispColor.Color = color;							// �F�w��
	}
	/* FT4800�Ŕ��]�w��̏ꍇ��FT4000�ł͐ԕ\�����s�� */
	else
	{
		DispColor.Color = COLOR_RED;					// �Ԏw��
	}
	DispColorDataSnd( &DispColor );

	// ������w��
	DispStr.Esc = LCD_ESC_STR;							// ������w��
	DispStr.Row = (unsigned char)lin;					// �s�w��
	DispStr.Colmun = (unsigned char)col;				// ��w��
	memcpy( &DispStr.Msg, dat, (size_t)cnt );
	if(lin != LCD_WMSG_LINE && lin != LCD_WMSG_LINE2){
		can_snd_data4((void *)&DispStr, 4 + cnt);
	}

	// ܰ�ݸޕ\������
	if( lin == LCD_WMSG_LINE ){
		// �\���s��ܰ�ݸޕ\���s�̏ꍇ�A�ʏ�\���ް��ۑ�
		j = 0;
		for( i=col; i< col+cnt; i++){
			Lcd_Wmsg_Cnt[i].RecMsg = DispStr.Msg[j];
			Lcd_Wmsg_Cnt[i].RecColor = DispColor.Color;
			j++;
		}

		if(DspSts != LCD_WMSG_ON){
			can_snd_data4((void *)&DispStr, 4 + cnt);
		}
	}
	if( lin == LCD_WMSG_LINE2 ){
		// �\���s��ܰ�ݸޕ\���s�̏ꍇ�A�ʏ�\���ް��ۑ�
		j = 0;
		for( i=col; i< col+cnt; i++){
			Lcd_Wmsg_Cnt[i].RecMsg2 = DispStr.Msg[j];
			Lcd_Wmsg_Cnt[i].RecColor2 = DispColor.Color;
			j++;
		}

		if(DspSts2 != LCD_WMSG_ON){
			can_snd_data4((void *)&DispStr, 4 + cnt);
		}
	}
	if(cursor){
		dsp_background_color(COLOR_WHITE);
	}
// MH810100(S) 
#else
	// �����e���̂ݕ\�����s��
	if( OPECTL.Mnt_mod == 0 ){
		return;
	}
// MH810100(E) S.Takahashi 2020/02/07 #3843 �J�[�\�����\������Ȃ�
//	PKTcmd_text_begin();
// MH810100(E) S.Takahashi 2020/02/07 #3843 �J�[�\�����\������Ȃ�
	/* �󔒂𔽓]�w�肳�ꂽ�ꍇ�͔w�i�F��ς��ċ󔒕\������ */	
	cursor = 0;
	if(mod != 0){					// ���]�w��
		for(i=0; i<cnt; i+=2){
			if(dat[i] == 0x81){
				if(dat[i+1] == 0x40){
					continue;
				}
			}
			break;
		}
		if( i >= cnt){					// �S�ċ�
			PKTcmd_text_1_backgroundcolor(LCD_CURSOR_COLOR);
			cursor = 1;
		}
	}
// MH810100(S) S.Takahashi 2020/02/07 #3843 �J�[�\�����\������Ȃ�
	PKTcmd_text_begin();
// MH810100(E) S.Takahashi 2020/02/07 #3843 �J�[�\�����\������Ȃ�

	// �����t�H���g�w��
//	DispFont.Esc = LCD_ESC_FONTTYPE;						// ����̫�Ďw��
//	DispFont.Type = LCD_NORMAL_FONT;						// ɰ��̫��
//	PKTcmd_text_color(&DispFont);
	PKTcmd_text_font(LCD_NORMAL_FONT);

	// �u�����N�w��
//	DispBlink.Esc = LCD_ESC_BLINK;						// ���ݸ�w��
	DispBlink.Row = (unsigned char)lin;					// �s�w��
	DispBlink.Colmun = 0;								// ��w��(�ݒ薳��)
	if(blink == 0) {
		DispBlink.Mode = 0;								// ���ݸ����
		DispBlink.Interval = 0;							// ���ݸ�Ԋu
	} else {
		DispBlink.Mode = 1;								// ���ݸ�J�n
		DispBlink.Interval = (unsigned char)blink;		// ���ݸ�Ԋu
	}
//	PKTcmd_text_blink(&DispBlink);
	PKTcmd_text_blink(lin, 0, (uchar)DispBlink.Mode, (uchar)DispBlink.Interval);

	// ܰ�ݸޕ\������
/* 	ܰ�ݸޕ\������ܰ�ݸޕ\���s(6,7�s��)�ɒʏ�\�����悤�Ƃ����ꍇ�A����̏����ł�
	�@�ȑO�̒ʏ탁�b�Z�[�W���J�o�� 
	�Aܰ�ݸލĕ\��	���I��
	�܂�\�����悤�Ƃ����ʏ탁�b�Z�[�W�͕\������Ȃ�
	���̏����̈Ӑ}���s�������Ƃ肠�����@�̏����͕s�v�Ǝv���̂ō폜
	���̏ꍇ�̏����́A�u�ʏ�\�����悤�Ƃ������b�Z�[�W�����J�o���G���A�ɃZ�b�g���ĕ\���͍X�V���Ȃ��v�悤�ɏC������		*/
	// �����J���[�w��
//	DispColor.Esc = LCD_ESC_FONTCOLR;					// �����װ�w��
	/* mod��normal�w��̏ꍇ */
	if( 0 == mod )
	{
		DispColor.Color = color;							// �F�w��
	}
// MH810100(S) S.Takahashi 2020/02/07 #3843 �J�[�\�����\������Ȃ�
	else if(cursor == 1) {
		DispColor.Color = LCD_CURSOR_COLOR;				// �J�[�\��
	}
// MH810100(E) S.Takahashi 2020/02/07 #3843 �J�[�\�����\������Ȃ�
	/* FT4800�Ŕ��]�w��̏ꍇ��FT4000�ł͐ԕ\�����s�� */
	else
	{
		DispColor.Color = COLOR_RED;					// �Ԏw��
	}
//	PKTcmd_text_backgroundcolor( &DispColor );
// MH810100(S) S.Takahashi 2019/12/18
//	PKTcmd_text_backgroundcolor( DispColor.Color );
	PKTcmd_text_color(DispColor.Color);
// MH810100(E) S.Takahashi 2019/12/18

	// ������w��
//	DispStr.Esc = LCD_ESC_STR;							// ������w��
	DispStr.Row = (unsigned char)lin;					// �s�w��
	DispStr.Colmun = (unsigned char)col;				// ��w��
	memcpy( &DispStr.Msg, dat, (size_t)cnt );
	if(lin != LCD_WMSG_LINE && lin != LCD_WMSG_LINE2){
//		PKTcmd_text_direct((void *)&DispStr, 4 + cnt);
		PKTcmd_text_direct(DispStr.Row, DispStr.Colmun, (uchar*)&DispStr.Msg);
	}

	// ܰ�ݸޕ\������
	if( lin == LCD_WMSG_LINE ){
		// �\���s��ܰ�ݸޕ\���s�̏ꍇ�A�ʏ�\���ް��ۑ�
		j = 0;
		for( i=col; i< col+cnt; i++){
			Lcd_Wmsg_Cnt[i].RecMsg = DispStr.Msg[j];
			Lcd_Wmsg_Cnt[i].RecColor = DispColor.Color;
			j++;
		}

		if(DspSts != LCD_WMSG_ON){
//			PKTcmd_text_direct((void *)&DispStr, 4 + cnt);
			PKTcmd_text_direct(DispStr.Row, DispStr.Colmun, (uchar*)&DispStr.Msg);
		}
	}
	if( lin == LCD_WMSG_LINE2 ){
		// �\���s��ܰ�ݸޕ\���s�̏ꍇ�A�ʏ�\���ް��ۑ�
		j = 0;
		for( i=col; i< col+cnt; i++){
			Lcd_Wmsg_Cnt[i].RecMsg2 = DispStr.Msg[j];
			Lcd_Wmsg_Cnt[i].RecColor2 = DispColor.Color;
			j++;
		}

		if(DspSts2 != LCD_WMSG_ON){
//			PKTcmd_text_direct((void *)&DispStr, 4 + cnt);
			PKTcmd_text_direct(DispStr.Row, DispStr.Colmun, (uchar*)&DispStr.Msg);
		}
	}
	
// MH810100(S) S.Takahashi 2020/02/07 #3843 �J�[�\�����\������Ȃ�
	PKTcmd_text_end();
// MH810100(E) S.Takahashi 2020/02/07 #3843 �J�[�\�����\������Ȃ�
	if(cursor){
		PKTcmd_text_1_backgroundcolor(COLOR_WHITE);
	}

// MH810100(E) S.Takahashi 2020/02/07 #3843 �J�[�\�����\������Ȃ�
//	PKTcmd_text_end();
// MH810100(E) S.Takahashi 2020/02/07 #3843 �J�[�\�����\������Ȃ�

#endif
// MH810100(E) 
}
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : grawaku                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ����g�\��                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned char startLine	�J�n�s 0-6(7)                  |*/
/*|              : unsigned char endLine	�I���s 0-6(7)                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : k.totsuka                                               |*/
/*| Date         : 2012-12-15                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
	unsigned short CanDispLine[2][15] = {
	{ LCD_GAIJI_LEFT_UP, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_RIGHT_UP },
	{ LCD_GAIJI_LEFT_DW, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_HORIZON, LCD_GAIJI_RIGHT_DW }
	};
void	grawaku( unsigned char startLine,  unsigned char endLine )
{
	unsigned short i;
	unsigned short linL, linR;

	if((startLine > 6) || (endLine > 6)){
		return;
	}
	if(startLine >= endLine){
		return;
	}

	grachr( startLine, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (unsigned char*)&CanDispLine[0] );	//�㉡��
	grachr( endLine, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (unsigned char*)&CanDispLine[1] );	//������

	linL = LCD_GAIJI_LEFT_VERT;
	linR = LCD_GAIJI_RIGHT_VERT;
	for(i=startLine+1; i<endLine; i++){
		grachr( i , 0, 2, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (unsigned char*)&linL);	//���c��
		grachr( i , 28, 2, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (unsigned char*)&linR);	//�E�c��
	}

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : numchr                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : BIG FONT ( 24 * 32 ) DISP ( 1 chacter )                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned short lin	line = 0-7                         |*/
/*|              : unsigned short col	colmn = 0-27                       |*/
/*|              : unsigned short color	character color                    |*/
/*|              : unsigned short blink	blink = 0(none) - blink time(10ms) |*/
/*|              : unsigned char  dat	data = '0'-'9' or ':'              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanaka                                                |*/
/*| Date         : 2002-08-29                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	numchr( unsigned short lin,
				unsigned short col,
				unsigned short color,
				unsigned short blink,
				unsigned char  dat )
{
	unsigned short	datsav;
	t_DispFont		DispFont;
	t_DispColor		DispColor;
	t_DispBlink		DispBlink;
	t_DispStr		DispStr;

	memset(&DispFont, 0, sizeof(t_DispFont));
	memset(&DispColor, 0, sizeof(t_DispColor));
	memset(&DispBlink, 0, sizeof(t_DispBlink));
	memset(&DispStr, 0, sizeof(t_DispStr));

	if(('0' <= dat) && (dat <= '9')){
		datsav = (unsigned short)((0x824f)+(dat-0x30));	// ASCII -> CGC Change
	} else {
		if (dat == ':') {
			datsav = 0x8146;
		} else {
			datsav = 0x8140;
		}
	}
// MH810100(S) Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
#if GT4100_NO_LCD
// MH810100(E) Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j

	// �����t�H���g�w��
	DispFont.Esc = LCD_ESC_FONTTYPE;					// ����̫�Ďw��
	DispFont.Type = LCD_BIG_FONT;						// �ޯ��̫��
	DispFontDataSnd(&DispFont);

	// �����J���[�w��
	DispColor.Esc = LCD_ESC_FONTCOLR;					// �����װ�w��
	DispColor.Color = color;							// �F�w��
	DispColorDataSnd( &DispColor );

	// �u�����N�w��
	DispBlink.Esc = LCD_ESC_BLINK;						// ���ݸ�w��
	DispBlink.Row = (unsigned char)lin;					// �s�w��
	DispBlink.Colmun = 0;								// ��w��(�ݒ薳��)
	if(blink == 0) {
		DispBlink.Mode = 0;								// ���ݸ����
		DispBlink.Interval = 0;							// ���ݸ�Ԋu
	} else {
		DispBlink.Mode = 1;								// ���ݸ�J�n
		DispBlink.Interval = (unsigned char)blink;		// ���ݸ�Ԋu
	}
	DispBlinkDataSnd(&DispBlink);

	// ������w��
	DispStr.Esc = LCD_ESC_STR;							// ������w��
	DispStr.Row = (unsigned char)lin;					// �s�w��
	DispStr.Colmun = (unsigned char)col;				// ��w��
	memcpy( DispStr.Msg, &datsav, sizeof(short) );	
	can_snd_data4((void *)&DispStr, 4 + sizeof(short));
// MH810100(S) Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j	
#else
// MH810100(S) Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j	
	memcpy( DispStr.Msg, &datsav, sizeof(short) );
// MH810100(E) Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
	// �����t�H���g�w��
	PKTcmd_text_begin();
	PKTcmd_text_font(LCD_BIG_FONT);		// �r�b�O�t�H���g

	// �����J���[�w��
	PKTcmd_text_color(color);			// �����J���[�w��

	// �u�����N�w��
	if(blink == 0) {
		DispBlink.Mode = 0;								// ���ݸ����
		DispBlink.Interval = 0;							// ���ݸ�Ԋu
		PKTcmd_text_blink((unsigned char)lin,0,(unsigned char)0,(unsigned char)0);				// �u�����N�w��
	} else {
		DispBlink.Mode = 1;								// ���ݸ�J�n
		DispBlink.Interval = (unsigned char)blink;		// ���ݸ�Ԋu
		PKTcmd_text_blink((unsigned char)lin,0,(unsigned char)1,(unsigned char)blink);				// �u�����N�w��
	}
	
	// ������w��
// MH810100(S) S.Takahashi 2020/02/12 #3853 �d�����b�N������ʂŕ��������i�Ïؔԍ����\������Ȃ��j
	memcpy( DispStr.Msg, &datsav, sizeof(short) );
// MH810100(E) S.Takahashi 2020/02/12 #3853 �d�����b�N������ʂŕ��������i�Ïؔԍ����\������Ȃ��j
	PKTcmd_text_direct((unsigned char)lin,(unsigned char)col, (uchar*)&DispStr.Msg);
	PKTcmd_text_end();
#endif
// MH810100(E) Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : bigcr                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : BIG FONT ( 24 * 32 ) DISP                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned short lin	line = 0-7(8 line mode)            |*/
/*|              : unsigned short col	colmn = 0-27                       |*/
/*|              : unsigned short cnt	character number                   |*/
/*|              : unsigned short color	character color                    |*/
/*|              : unsigned short blink	blink = 0(none) - blink time(10ms) |*/
/*|              : unsigned char * dat	data(GDC character codes)          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanaka                                                |*/
/*| Date         : 2002-08-29                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	bigcr(  unsigned short lin,
				unsigned short col,
				unsigned short cnt,
				unsigned short color,
				unsigned short blink,
				const unsigned char *dat )
{
	unsigned char	DispMsg[LCD_CLM];
	t_DispFont		DispFont;
	t_DispColor		DispColor;
	t_DispBlink		DispBlink;
	t_DispStr		DispStr;

	memset(&DispMsg, 0, LCD_CLM);
	memset(&DispFont, 0, sizeof(t_DispFont));
	memset(&DispColor, 0, sizeof(t_DispColor));
	memset(&DispBlink, 0, sizeof(t_DispBlink));
	memset(&DispStr, 0, sizeof(t_DispStr));
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
#if GT4100_NO_LCD
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
	// �����t�H���g�w��
	DispFont.Esc = LCD_ESC_FONTTYPE;					// ����̫�Ďw��
	DispFont.Type = LCD_BIG_FONT;						// �ޯ��̫��
	DispFontDataSnd(&DispFont);

	// �����J���[�w��
	DispColor.Esc = LCD_ESC_FONTCOLR;					// �����װ�w��
	DispColor.Color = color;							// �F�w��
	DispColorDataSnd( &DispColor );

	// �u�����N�w��
	DispBlink.Esc = LCD_ESC_BLINK;						// ���ݸ�w��
	DispBlink.Row = (unsigned char)lin;					// �s�w��
	DispBlink.Colmun = 0;								// ��w��(�ݒ薳��)
	if(blink == 0) {
		DispBlink.Mode = 0;								// ���ݸ����
		DispBlink.Interval = 0;							// ���ݸ�Ԋu
	} else {
		DispBlink.Mode = 1;								// ���ݸ�J�n
		DispBlink.Interval = (unsigned char)blink;		// ���ݸ�Ԋu
	}
	DispBlinkDataSnd(&DispBlink);

	// ������w��
	DispStr.Esc = LCD_ESC_STR;							// ������w��
	DispStr.Row = (unsigned char)lin;					// �s�w��
	DispStr.Colmun = (unsigned char)col;				// ��w��
	memcpy( &DispStr.Msg[0], dat, (size_t)cnt );
	can_snd_data4((void *)&DispStr, 4 + cnt);

// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
#else
	// �����t�H���g�w��
	PKTcmd_text_begin();
	PKTcmd_text_font(LCD_BIG_FONT);		// �r�b�O�t�H���g

	// �����J���[�w��
	PKTcmd_text_color(color);			// �����J���[�w��

	// �u�����N�w��
	if(blink == 0) {
		DispBlink.Mode = 0;								// ���ݸ����
		DispBlink.Interval = 0;							// ���ݸ�Ԋu
		PKTcmd_text_blink((unsigned char)lin,0,(unsigned char)0,(unsigned char)0);				// �u�����N�w��
	} else {
		DispBlink.Mode = 1;								// ���ݸ�J�n
		DispBlink.Interval = (unsigned char)blink;		// ���ݸ�Ԋu
		PKTcmd_text_blink((unsigned char)lin,0,(unsigned char)1,(unsigned char)blink);				// �u�����N�w��
	}

	// ������w��
	PKTcmd_text_direct((unsigned char)lin,(unsigned char)col, (uchar*)&DispStr.Msg);
	PKTcmd_text_end();
#endif
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : blink_reg                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : Blink area registation                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned short lin	line = 0-7                         |*/
/*|              : unsigned short col	colmn = 0-29                       |*/
/*|              : unsigned short cnt	character number = 1-30            |*/
/*|              : unsigned short mod	mode = 0(normal)                   |*/
/*|              : unsigned short color	character color                    |*/
/*|              : unsigned short * dat	data(GDC character codes)          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanaka                                                |*/
/*| Date         : 2005-03-29                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	blink_reg(  unsigned short lin,
					unsigned short col,
					unsigned short cnt,
					unsigned short mod,
					unsigned short color,
					const unsigned char *dat )
{

	if( LCD_BLKLG_CNT >= 10 ){
		return;
	}
	// ���ݸ��~�p�����ݸ�J�n�����ް���ޔ�
	LCD_Blk_lg[LCD_BLKLG_CNT].lin = lin;						// �w��s
	LCD_Blk_lg[LCD_BLKLG_CNT].col = col;						// �w����
	LCD_Blk_lg[LCD_BLKLG_CNT].cnt = cnt;						// ������
	LCD_Blk_lg[LCD_BLKLG_CNT].mod = mod;						// ���]
	LCD_Blk_lg[LCD_BLKLG_CNT].color = color;					// �����F
	memcpy( &LCD_Blk_lg[LCD_BLKLG_CNT].dat, dat, (size_t)cnt );	// �\������

	LCD_BLKLG_CNT++;

	grachr( lin, col, cnt, mod, color, LCD_BLINK_ON, dat );		// ���ݸ�\���J�n
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : blink_end (���g�p�֐�)                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : Blinking end                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanaka                                                |*/
/*| Date         : 2005-03-29                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	blink_end( void )
{
	unsigned short	i;

	for( i = 0; i < LCD_BLKLG_CNT; i++ ){

		// ���ݸ�\����~
		grachr( LCD_Blk_lg[i].lin,								// �w��s
				LCD_Blk_lg[i].col,								// �w����
				LCD_Blk_lg[i].cnt,								// ������
				LCD_Blk_lg[i].mod,								// ���]�w��
				LCD_Blk_lg[i].color,							// �\�������F
				LCD_BLINK_OFF,									// ���ݸ�w��:�Ȃ�
				LCD_Blk_lg[i].dat );							// �\��������
	}

	LCD_BLKLG_CNT = 0;
	memset( LCD_Blk_lg, 0, sizeof(t_blk_lg) * LCD_BLK_LG );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : lcd_backlight                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : LCDC back light on/off                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : mode = 0:off/ 1:on                                      |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Y.Tanaka                                                 |*/
/*| Date        : 2005-03-29                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	lcd_backlight( char mode )
{
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
#if GT4100_NO_LCD
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
	unsigned char	CtlCode;
	unsigned char	LEDPattan;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
#endif
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j

	if( (blightMode == mode) && (blightLevel == backlight) ) {
		return;											// �O��Ɠ������e�ł���Α��M���Ȃ�
	}
	blightMode = mode;
	blightLevel = backlight;

// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
#if GT4100_NO_LCD
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
	if( mode ) {
		// �o�b�N���C�gON
		CtlCode = CAN_CTRL_BKLT | backlight;
		LEDPattan = 0;
		SD_LCD = 1;
	} else {
		// �o�b�N���C�gOFF
		CtlCode = CAN_CTRL_BKLT;
		LEDPattan = 0;
		SD_LCD = 0;
	}
	can_snd_data3(CtlCode, LEDPattan);					// CAN�R���g���[�����M
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
#else
	// ON�H
	if( mode ){
		PKTcmd_brightness( backlight );
	}else{
		PKTcmd_brightness( 0 );
	}
#endif	
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
}


/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : lcd_contrast                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : �R���g���X�g����                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : value = 1�`16:�P�x�l�C0:�o�b�N���C�gOFF                 |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Y.Tanaka                                                 |*/
/*| Date        : 2005-03-29                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	lcd_contrast( unsigned char value )
{
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
#if GT4100_NO_LCD
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
	unsigned char	CtlCode;
	unsigned char	LEDPattan;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
#endif
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j

	if( value >  LCD_LUMINE_MAX) {
		return;
	}

// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
#if GT4100_NO_LCD
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
	if( value != 0) {
		// �o�b�N���C�gON
		CtlCode = CAN_CTRL_BKLT | value;
		LEDPattan = 0;
	} else {
		// �o�b�N���C�gOFF
		CtlCode = CAN_CTRL_BKLT;
		LEDPattan = 0;
	}
	can_snd_data3(CtlCode, LEDPattan);					// CAN�R���g���[�����M
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
#else
	PKTcmd_brightness( value ) ;
#endif	
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X�i��ʕ\���ύX�j
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Fun_Dsp                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ̧ݸ��ݷ��g�\��                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned short dat	dsp data = FUNMSG[]                |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanaka                                                |*/
/*| Date         : 2005-03-29                                              |*/
/*| Update       : font_change 2005-09-01 T.Hashimoto                      |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Fun_Dsp( const unsigned char* dat )
{
// MH810100(S)
#if (!GT4100_NO_LCD)
// MH810100(E)
// MH810100(S) S.Takahashi 2019/12/18
//	unsigned short	lin;
//	t_DispMenu		DispMenu;
	unsigned short	lin = 7;
// MH810100(E) S.Takahashi 2019/12/18
	unsigned char	i;

// MH810100(S) S.Takahashi 2019/12/18
//	memset(&DispMenu, 0, sizeof(t_DispMenu));
// MH810100(E) S.Takahashi 2019/12/18

	// ���j���[�\���w��
// MH810100(S) S.Takahashi 2019/12/18
//	DispMenu.Esc = LCD_ESC_MENU;						// �ƭ��\���w��
//	DispMenu.Row = 7;									// �s�w��
//	DispMenu.Colmun = 0;								// ��w��
//	memcpy( &DispMenu.Msg[0], dat, LCD_CLM );
//	can_snd_data4((void *)&DispMenu, 4 + LCD_CLM);
	PKTcmd_text_begin();	
	PKTcmd_text_menu(lin, 0, dat);
// MH810100(E) S.Takahashi 2019/12/18

	// ܰ�ݸޕ\������
// MH810100(S) S.Takahashi 2019/12/18
//	lin = 7;
// MH810100(E) S.Takahashi 2019/12/18
	if( lin == LCD_WMSG_LINE ){

		// ܰ�ݸޕ\���s��̧ݸ��ݷ��\���s(=7)�̏ꍇ
		for( i=0; i<LCD_CLM; i++ ){
// MH810100(S) S.Takahashi 2019/12/18
//			Lcd_Wmsg_Cnt[i].RecMsg = DispMenu.Msg[i];
			Lcd_Wmsg_Cnt[i].RecMsg = dat[i];
// MH810100(E) S.Takahashi 2019/12/18
			Lcd_Wmsg_Cnt[i].RecColor = COLOR_BLACK;
		}

		if(DspSts == LCD_WMSG_ON){
			// ܰ�ݸ�ү���ޕ\�����̏ꍇ
			Lcd_Wmsg_Disp();								// ܰ�ݸ�ү���ޕ\��
		}
	}
// MH810100(S) S.Takahashi 2019/12/18
	PKTcmd_text_end();
// MH810100(E) S.Takahashi 2019/12/18
// MH810100(S)
#else
	unsigned short	lin;
	t_DispMenu		DispMenu;
	unsigned char	i;
	memset( &DispMenu, 0, sizeof(t_DispMenu) );

	DispMenu.Esc = LCD_ESC_MENU;						// �ƭ��\���w��
	DispMenu.Row = 7;									// �s�w��
	DispMenu.Colmun = 0;								// ��w��
	memcpy( &DispMenu.Msg[0], dat, LCD_CLM );
	can_snd_data4((void *)&DispMenu, 4 + LCD_CLM);

	lin = 7;
	if( lin == LCD_WMSG_LINE ){

		// ܰ�ݸޕ\���s��̧ݸ��ݷ��\���s(=7)�̏ꍇ
		for( i=0; i<LCD_CLM; i++ ){
			Lcd_Wmsg_Cnt[i].RecMsg = DispMenu.Msg[i];
			Lcd_Wmsg_Cnt[i].RecColor = COLOR_BLACK;
		}

		if(DspSts == LCD_WMSG_ON){
			// ܰ�ݸ�ү���ޕ\�����̏ꍇ
			Lcd_Wmsg_Disp();								// ܰ�ݸ�ү���ޕ\��
		}
	}
#endif
// MH810100(E)
	return;
}


/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Vram_BkupRestr                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : LCD_ADD �� 6.or.7Line�ڂ� Backup / Restore ����          */
/*|                �����S�Ă̍s���K�v�Ȃ�Q�����z��ɂ���̂��x�X�g         */
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned short mode 1:Backup 0:Restore				   |*/
/*|                unsigned short line	 = 6,7                             |*/
/*|                short show_sw : 0:Not 1:Show ON						   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Okamoto                                                 |*/
/*| Date         : 2006-09-14                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Vram_BkupRestr( unsigned short mode ,unsigned short lin , short show_sw )
{
// MH810100(S) K.Onodera 2020/03/12 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//
//	t_DispFont		DispFont;
//	t_DispColor		DispColor;
//	t_DispStr		DispStr;
//	unsigned char	i;
//
//	memset(&DispFont, 0, sizeof(t_DispFont));
//	memset(&DispColor, 0, sizeof(t_DispColor));
//	memset(&DispStr, 0, sizeof(t_DispStr));
//
//	if(mode){												// �ޯ�����
//		switch(lin){
//		case	6:
//			for( i=0; i<LCD_CLM; i++ ){
//				LCD_BACKUP_MSG6[i] = Lcd_Wmsg_Cnt[i].RecMsg2;
//				LCD_BACKUP_COLOR6[i] = Lcd_Wmsg_Cnt[i].RecColor2;
//			}
//			break;
//		case	7:
//			for( i=0; i<LCD_CLM; i++ ){
//				LCD_BACKUP_MSG7[i] = Lcd_Wmsg_Cnt[i].RecMsg;
//				LCD_BACKUP_COLOR7[i] = Lcd_Wmsg_Cnt[i].RecColor;
//			}
//			break;
//		}
//	}else{													// ؽı
//		if ( LCD_BACKUP_Flag[lin] ){
//			switch(lin){
//			case	6:
//				for( i=0; i<LCD_CLM; i++ ){
//					Lcd_Wmsg_Cnt[i].RecMsg2 = LCD_BACKUP_MSG6[i];
//					Lcd_Wmsg_Cnt[i].RecColor2 = LCD_BACKUP_COLOR6[i];
//				}
//				for( i=0; i<LCD_CLM; i++ ){
//					if( show_sw == 1 ){							// ؽı�{�\��
//						// �����t�H���g�w��
//						DispFont.Esc = LCD_ESC_FONTTYPE;					// ����̫�Ďw��
//						DispFont.Type = LCD_NORMAL_FONT;					// ɰ��̫��
//						DispFontDataSnd(&DispFont);
//
//						// �����J���[�w��
//						DispColor.Esc = LCD_ESC_FONTCOLR;					// �����װ�w��
//						DispColor.Color = LCD_BACKUP_COLOR6[i];				// �F�w��
//						DispColorDataSnd( &DispColor );
//
//						// ������w��
//						DispStr.Esc = LCD_ESC_STR;							// ������w��
//						DispStr.Row = (unsigned char)lin;					// �s�w��
//						DispStr.Colmun = i;									// ��w��
//						if(SJIS_Size(&LCD_BACKUP_MSG6[i]) == 1){
//							// ���p
//							DispStr.Msg[0] = LCD_BACKUP_MSG6[i];
//							can_snd_data4((void *)&DispStr, 4 + 1);
//						}
//						else{
//							// �S�p
//							DispStr.Msg[0] = LCD_BACKUP_MSG6[i];
//							DispStr.Msg[1] = LCD_BACKUP_MSG6[i+1];
//							can_snd_data4((void *)&DispStr, 4 + 2);
//							i++;
//						}
//					}
//				}
//				break;
//			case	7:
//				for( i=0; i<LCD_CLM; i++ ){
//					Lcd_Wmsg_Cnt[i].RecMsg = LCD_BACKUP_MSG7[i];
//					Lcd_Wmsg_Cnt[i].RecColor = LCD_BACKUP_COLOR7[i];
//				}
//				for( i=0; i<LCD_CLM; i++ ){
//					if( show_sw == 1 ){							// ؽı�{�\��
//						// �����t�H���g�w��
//						DispFont.Esc = LCD_ESC_FONTTYPE;					// ����̫�Ďw��
//						DispFont.Type = LCD_NORMAL_FONT;					// ɰ��̫��
//						DispFontDataSnd(&DispFont);
//
//						// �����J���[�w��
//						DispColor.Esc = LCD_ESC_FONTCOLR;					// �����װ�w��
//						DispColor.Color = LCD_BACKUP_COLOR7[i];				// �F�w��
//						DispColorDataSnd( &DispColor );
//
//						// ������w��
//						DispStr.Esc = LCD_ESC_STR;							// ������w��
//						DispStr.Row = (unsigned char)lin;					// �s�w��
//						DispStr.Colmun = i;									// ��w��
//						if(SJIS_Size(&LCD_BACKUP_MSG7[i]) == 1){
//							// ���p
//							DispStr.Msg[0] = LCD_BACKUP_MSG7[i];
//							can_snd_data4((void *)&DispStr, 4 + 1);
//						}
//						else{
//							// �S�p
//							DispStr.Msg[0] = LCD_BACKUP_MSG7[i];
//							DispStr.Msg[1] = LCD_BACKUP_MSG7[i+1];
//							can_snd_data4((void *)&DispStr, 4 + 2);
//							i++;
//						}
//					}
//				}
//				break;
//			}
//		}
//	}
//
//	LCD_BACKUP_Flag[lin] = (unsigned char)mode;
// MH810100(E) K.Onodera 2020/03/12 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	return;
}


/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Fun_Dsp                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ̧ݸ��ݷ�1�g�\��                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned short dat	dsp data = FUNMSG[]                |*/
/*|              : unsigned char  num  �\������ӏ��w��(�P�`�T)            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanaka                                                |*/
/*| Date         : 2005-03-29                                              |*/
/*| Update       : font_change 2005-09-01 T.Hashimoto                      |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Funckey_one_Dsp( const unsigned char* dat, unsigned char num )
{
	unsigned short	lin;
	t_DispMenu		DispMenu;
	unsigned char	i;

	
	memset(&DispMenu, 0, sizeof(t_DispMenu));

	// ���j���[�\���w��
	DispMenu.Esc = LCD_ESC_MENU;						// �ƭ��\���w��
	DispMenu.Row = 7;									// �s�w��
	DispMenu.Colmun = (num-1)*6;								// ��w��
	memcpy( &DispMenu.Msg[0], &dat[0], 6 );
	can_snd_data4((void *)&DispMenu, 4 + 6);

	// ܰ�ݸޕ\������
	lin = 7;
	if( lin == LCD_WMSG_LINE ){

		// ܰ�ݸޕ\���s��̧ݸ��ݷ��\���s(=7)�̏ꍇ
		for( i=0; i<LCD_CLM; i++ ){
			Lcd_Wmsg_Cnt[i].RecMsg = DispMenu.Msg[i];
			Lcd_Wmsg_Cnt[i].RecColor = COLOR_BLACK;
		}

		if(DspSts == LCD_WMSG_ON){
			// ܰ�ݸ�ү���ޕ\�����̏ꍇ
			Lcd_Wmsg_Disp();								// ܰ�ݸ�ү���ޕ\��
		}
	}
	return;
}
void	grawaku2( unsigned short start_line )
{
	unsigned short lin;

	grachr( start_line , 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, (unsigned char*)&CanDispLine[0] );	//�㉡��
	grachr( start_line+3 , 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, (unsigned char*)&CanDispLine[1] );	//������

	lin = LCD_GAIJI_LEFT_VERT;
	grachr( start_line+1 , 0, 2, 0, COLOR_INDIGO, LCD_BLINK_OFF, (unsigned char*)&lin);				//���c��
	grachr( start_line+2 , 0, 2, 0, COLOR_INDIGO, LCD_BLINK_OFF, (unsigned char*)&lin);				//���c��
	lin = LCD_GAIJI_RIGHT_VERT;
	grachr( start_line+1 , 28, 2, 0, COLOR_INDIGO, LCD_BLINK_OFF, (unsigned char*)&lin);				//�E�c��
	grachr( start_line+2 , 28, 2, 0, COLOR_INDIGO, LCD_BLINK_OFF, (unsigned char*)&lin);				//�E�c��

	return;
}
