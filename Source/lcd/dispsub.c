/*[]----------------------------------------------------------------------[]*/
/*| Display function                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Y.Tanaka                                                 |*/
/*| Date        : 2002-03-29                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update      : �V���Z�@�Ή� 2012-12-15 k.totsuka                        |*/
/*| Date        :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	"system.h"
#include	<string.h>
#include	<stdio.h>
#include	"lcd_def.h"
#include	"iodefine.h"
#include	"can_def.h"

t_DispColorFontNow	Disp_color_font_now;		//���ݐݒ蒆�̃J���[�ƃt�H���g��ʂ�ێ����Ă���
t_DispBlinkNow 		Disp_blink_now[8];			//���ݐݒ蒆�̃u�����N����ێ����Ă���

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lcd_WmsgDisp_ON( uchar mod, uchar *msg )                |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ܰ�ݸ�ү���ޕ\���J�n                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : ushort mod  = �\�����@�i0:�ʏ� / 1:���]�j               |*/
/*|                ushort color= �\���F                                    |*/
/*|                ushort blink= ���ݸ�\�� 0(none) - blink time(10ms)      |*/
/*|                uchar  *msg = �\���������߲���i�������30�����Œ�j     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005-11-14                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Lcd_WmsgDisp_ON( unsigned short mod, unsigned short color, unsigned short blink, const unsigned char *msg )
{
// MH810100(S) K.Onodera 2020/01/06 �Ԕԃ`�P�b�g���X�i��ʕ\���Ή��j
//	t_DispFont		DispFont;
//	t_DispColor		DispColor;
//	t_DispBlink		DispBlink;
//	t_DispStr		DispStr;
//	unsigned char	i;
//
//	memset(&DispFont, 0, sizeof(t_DispFont));
//	memset(&DispColor, 0, sizeof(t_DispColor));
//	memset(&DispBlink, 0, sizeof(t_DispBlink));
//	memset(&DispStr, 0, sizeof(t_DispStr));
//
//	DspSts = LCD_WMSG_ON;								// �\����Ԃn�m
//
//	// �����t�H���g�w��
//	DispFont.Esc = LCD_ESC_FONTTYPE;					// ����̫�Ďw��
//	DispFont.Type = LCD_NORMAL_FONT;					// ɰ��̫��
//	DispFontDataSnd(&DispFont);
//
//	// �����J���[�w��
//	DispColor.Esc = LCD_ESC_FONTCOLR;					// �����װ�w��
//	DispColor.Color = color;							// �F�w��
//	DispColorDataSnd( &DispColor );
//
//	// �u�����N�w��
//	DispBlink.Esc = LCD_ESC_BLINK;						// ���ݸ�w��
//	DispBlink.Row = LCD_WMSG_LINE;						// �s�w��
//	DispBlink.Colmun = 0;								// ��w��(�ݒ薳��)
//	if(blink == 0) {
//		DispBlink.Mode = 0;								// ���ݸ����
//		DispBlink.Interval = 0;							// ���ݸ�Ԋu
//	} else {
//		DispBlink.Mode = 1;								// ���ݸ�J�n
//		DispBlink.Interval = (unsigned char)blink;		// ���ݸ�Ԋu
//	}
//	DispBlinkDataSnd(&DispBlink);
//
//	// ������w��
//	DispStr.Esc = LCD_ESC_STR;							// ������w��
//	DispStr.Row = LCD_WMSG_LINE;						// �s�w��
//	DispStr.Colmun = 0;									// ��w��
//	memcpy( &DispStr.Msg[0], msg, LCD_CLM );
//	can_snd_data4((char *)&DispStr, 4 + LCD_CLM);
//
//	for( i=0; i<LCD_CLM; i++ ){
//		Lcd_Wmsg_Cnt[i].WarMsg = DispStr.Msg[i];
//		Lcd_Wmsg_Cnt[i].WarColor = DispColor.Color;		// ܰ�ݸ�ү���ޕ\���F�ۑ�
//	}
// MH810100(E) K.Onodera 2020/01/06 �Ԕԃ`�P�b�g���X�i��ʕ\���Ή��j
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lcd_WmsgDisp_OFF( void )                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ܰ�ݸ�ү���ޕ\���I��                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005-11-14                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Lcd_WmsgDisp_OFF( void )
{
	t_DispFont		DispFont;
	t_DispColor		DispColor;
	t_DispStr		DispStr;
	unsigned char	i;

	memset(&DispFont, 0, sizeof(t_DispFont));
	memset(&DispColor, 0, sizeof(t_DispColor));
	memset(&DispStr, 0, sizeof(t_DispStr));

	if( DspSts == LCD_WMSG_ON ){

		// ܰ�ݸ�ү���ޕ\�����̏ꍇ
		DspSts = LCD_WMSG_OFF;										// �\����Ԃn�e�e

		// �����t�H���g�w��
		DispFont.Esc = LCD_ESC_FONTTYPE;							// ����̫�Ďw��
		DispFont.Type = LCD_NORMAL_FONT;							// ɰ��̫��
		DispFontDataSnd(&DispFont);

		for( i=0; i<LCD_CLM; i++ ){
			// �����J���[�w��
			DispColor.Esc = LCD_ESC_FONTCOLR;						// �����װ�w��
			DispColor.Color = Lcd_Wmsg_Cnt[i].RecColor;				// �F�w��
			DispColorDataSnd( &DispColor );

			// ������w��
			DispStr.Esc = LCD_ESC_STR;								// ������w��
			DispStr.Row = LCD_WMSG_LINE;							// �s�w��
			DispStr.Colmun = i;										// ��w��
			if(SJIS_Size(&Lcd_Wmsg_Cnt[i].RecMsg) == 1){
				// ���p
				DispStr.Msg[0] = Lcd_Wmsg_Cnt[i].RecMsg;
				can_snd_data4((void *)&DispStr, 4 + 1);
			}
			else{
				// �S�p
				DispStr.Msg[0] = Lcd_Wmsg_Cnt[i].RecMsg;
				DispStr.Msg[1] = Lcd_Wmsg_Cnt[i+1].RecMsg;
				can_snd_data4((void *)&DispStr, 4 + 2);
				i++;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lcd_Wmsg_Disp( void )                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ܰ�ݸ�ү���ޕ\��                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005-11-14                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Lcd_Wmsg_Disp( void )
{
	t_DispFont		DispFont;
	t_DispColor		DispColor;
	t_DispStr		DispStr;
	unsigned char	i;

	memset(&DispFont, 0, sizeof(t_DispFont));
	memset(&DispColor, 0, sizeof(t_DispColor));
	memset(&DispStr, 0, sizeof(t_DispStr));

	// �����t�H���g�w��
	DispFont.Esc = LCD_ESC_FONTTYPE;							// ����̫�Ďw��
	DispFont.Type = LCD_NORMAL_FONT;							// ɰ��̫��
	DispFontDataSnd(&DispFont);

	for( i=0; i<LCD_CLM; i++ ){
		// �����J���[�w��
		DispColor.Esc = LCD_ESC_FONTCOLR;							// �����װ�w��
		DispColor.Color = Lcd_Wmsg_Cnt[i].WarColor;					// �F�w��
		DispColorDataSnd( &DispColor );

		// ������w��
		DispStr.Esc = LCD_ESC_STR;									// ������w��
		DispStr.Row = LCD_WMSG_LINE;								// �s�w��
		DispStr.Colmun = i;											// ��w��
		if(SJIS_Size(&Lcd_Wmsg_Cnt[i].WarMsg) == 1){
			// ���p
			DispStr.Msg[0] = Lcd_Wmsg_Cnt[i].WarMsg;					// ܰ�ݸ�ү���ޕ\���ް�ض�ް
			can_snd_data4((void *)&DispStr, 4 + 1);
		}
		else{
			// �S�p
			DispStr.Msg[0] = Lcd_Wmsg_Cnt[i].WarMsg;					// ܰ�ݸ�ү���ޕ\���ް�ض�ް
			DispStr.Msg[1] = Lcd_Wmsg_Cnt[i+1].WarMsg;					// ܰ�ݸ�ү���ޕ\���ް�ض�ް
			can_snd_data4((void *)&DispStr, 4 + 2);
			i++;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lcd_WmsgDisp_ON( uchar mod, uchar *msg )                |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ܰ�ݸ�ү���ޕ\���J�n                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : ushort mod  = �\�����@�i0:�ʏ� / 1:���]�j               |*/
/*|                ushort color= �\���F                                    |*/
/*|                ushort blink= ���ݸ�\�� 0(none) - blink time(10ms)      |*/
/*|                uchar  *msg = �\���������߲���i�������30�����Œ�j     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.SUZUKI                                                |*/
/*| Date         : 2006-08-23                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Lcd_WmsgDisp_ON2( unsigned short mod, unsigned short color, unsigned short blink, const unsigned char *msg )
{
	t_DispFont		DispFont;
	t_DispColor		DispColor;
	t_DispBlink		DispBlink;
	t_DispStr		DispStr;
	unsigned char	i;

	memset(&DispFont, 0, sizeof(t_DispFont));
	memset(&DispColor, 0, sizeof(t_DispColor));
	memset(&DispBlink, 0, sizeof(t_DispBlink));
	memset(&DispStr, 0, sizeof(t_DispStr));

	DspSts2 = LCD_WMSG_ON;								// �\����Ԃn�m

	// �����t�H���g�w��
	DispFont.Esc = LCD_ESC_FONTTYPE;					// ����̫�Ďw��
	DispFont.Type = LCD_NORMAL_FONT;					// ɰ��̫��
	DispFontDataSnd(&DispFont);

	// �����J���[�w��
	DispColor.Esc = LCD_ESC_FONTCOLR;					// �����װ�w��
	DispColor.Color = color;							// �F�w��
	DispColorDataSnd( &DispColor );

	// �u�����N�w��
	DispBlink.Esc = LCD_ESC_BLINK;						// ���ݸ�w��
	DispBlink.Row = LCD_WMSG_LINE;						// �s�w��
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
	DispStr.Row = LCD_WMSG_LINE2;						// �s�w��
	DispStr.Colmun = 0;									// ��w��
	memcpy( &DispStr.Msg[0], msg, LCD_CLM );
	can_snd_data4((void *)&DispStr, 4 + LCD_CLM);

	for( i=0; i<LCD_CLM; i++ ){
		Lcd_Wmsg_Cnt[i].WarMsg2 = DispStr.Msg[i];		// ܰ�ݸ�ү���ޕ\���ް��ۑ�
		Lcd_Wmsg_Cnt[i].WarColor2 = DispColor.Color;	// ܰ�ݸ�ү���ޕ\���F�ۑ�
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lcd_WmsgDisp_OFF( void )                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ܰ�ݸ�ү���ޕ\���I��                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.SUZUKI                                                |*/
/*| Date         : 2006-08-23                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Lcd_WmsgDisp_OFF2( void )
{
	t_DispFont		DispFont;
	t_DispColor		DispColor;
	t_DispStr		DispStr;
	unsigned char	i;

	memset(&DispFont, 0, sizeof(t_DispFont));
	memset(&DispColor, 0, sizeof(t_DispColor));
	memset(&DispStr, 0, sizeof(t_DispStr));

	if( DspSts2 == LCD_WMSG_ON ){
		// ܰ�ݸ�ү���ޕ\�����̏ꍇ
		DspSts2 = LCD_WMSG_OFF;										// �\����Ԃn�e�e

		// �����t�H���g�w��
		DispFont.Esc = LCD_ESC_FONTTYPE;							// ����̫�Ďw��
		DispFont.Type = LCD_NORMAL_FONT;							// ɰ��̫��
		DispFontDataSnd(&DispFont);

		for( i=0; i<LCD_CLM; i++ ){
			// �����J���[�w��
			DispColor.Esc = LCD_ESC_FONTCOLR;						// �����װ�w��
			DispColor.Color = Lcd_Wmsg_Cnt[i].RecColor2;			// �F�w��
			DispColorDataSnd( &DispColor );

			// ������w��
			DispStr.Esc = LCD_ESC_STR;								// ������w��
			DispStr.Row = LCD_WMSG_LINE2;							// �s�w��
			DispStr.Colmun = i;										// ��w��
			if(SJIS_Size(&Lcd_Wmsg_Cnt[i].RecMsg2) == 1){
				// ���p
				DispStr.Msg[0] = Lcd_Wmsg_Cnt[i].RecMsg2;				// �ʏ�\���ް�ض�ް
				can_snd_data4((void *)&DispStr, 4 + 1);
			}
			else{
				// �S�p
				DispStr.Msg[0] = Lcd_Wmsg_Cnt[i].RecMsg2;				// �ʏ�\���ް�ض�ް
				DispStr.Msg[1] = Lcd_Wmsg_Cnt[i+1].RecMsg2;				// �ʏ�\���ް�ض�ް
				can_snd_data4((void *)&DispStr, 4 + 2);
				i++;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lcd_Wmsg_Disp2( void )                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ܰ�ݸ�ү���ޕ\��                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.SUZUKI                                                |*/
/*| Date         : 2006-08-23                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Lcd_Wmsg_Disp2( void )
{
	t_DispFont		DispFont;
	t_DispColor		DispColor;
	t_DispStr		DispStr;
	unsigned char	i;

	memset(&DispFont, 0, sizeof(t_DispFont));
	memset(&DispColor, 0, sizeof(t_DispColor));
	memset(&DispStr, 0, sizeof(t_DispStr));


	// �����t�H���g�w��
	DispFont.Esc = LCD_ESC_FONTTYPE;							// ����̫�Ďw��
	DispFont.Type = LCD_NORMAL_FONT;							// ɰ��̫��
	DispFontDataSnd(&DispFont);

	for( i=0; i<LCD_CLM; i++ ){
		// �����J���[�w��
		DispColor.Esc = LCD_ESC_FONTCOLR;							// �����װ�w��
		DispColor.Color = Lcd_Wmsg_Cnt[i].WarColor2;				// �F�w��
		DispColorDataSnd( &DispColor );

		// ������w��
		DispStr.Esc = LCD_ESC_STR;									// ������w��
		DispStr.Row = LCD_WMSG_LINE2;								// �s�w��
		DispStr.Colmun = i;											// ��w��
		if(SJIS_Size(&Lcd_Wmsg_Cnt[i].WarMsg2) == 1){
			// ���p
			DispStr.Msg[0] = Lcd_Wmsg_Cnt[i].WarMsg2;					// �ʏ�\���ް�ض�ް
			can_snd_data4((void *)&DispStr, 4 + 1);
		}
		else{
			// �S�p
			DispStr.Msg[0] = Lcd_Wmsg_Cnt[i].WarMsg2;					// �ʏ�\���ް�ض�ް
			DispStr.Msg[1] = Lcd_Wmsg_Cnt[i+1].WarMsg2;					// �ʏ�\���ް�ض�ް
			can_snd_data4((void *)&DispStr, 4 + 2);
			i++;
		}
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : DispColorDataSnd                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      :   �J���[���ݒ菈��                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : t_DispColor*	DispColor			�J���[���             |*/
/*| RETURN VALUE : �O�F����I���F�@�P�F�ُ�I���i�O��ݒ�l�Ɠ����j        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Tanaka                                               |*/
/*| Date         : 2012-02-21                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned char	DispColorDataSnd( t_DispColor*	DispColor )
{
	//�O��ݒ�l�ƈقȂ�ꍇ
	if(Disp_color_font_now.Color !=  DispColor->Color)
	{
		Disp_color_font_now.Color =  DispColor->Color;
		can_snd_data4((void *)DispColor, sizeof(t_DispColor));
		return 0;				//����I��
	}
	return	1;					//�ُ�I��
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : DispFontDataSnd                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      :  �t�H���g���ݒ菈��                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : t_DispFont*	DispFont			�t�H���g���           |*/
/*| RETURN VALUE : �O�F����I���F�@�P�F�ُ�I���i�O��ݒ�l�Ɠ����j        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Tanaka                                                |*/
/*| Date         : 2006-02-21                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned char	DispFontDataSnd( t_DispFont*	DispFont )
{
	//�O��ݒ�l�ƈقȂ�ꍇ
	if( Disp_color_font_now.Type !=  DispFont->Type)
	{
		Disp_color_font_now.Type =  DispFont->Type;
		can_snd_data4((void *)DispFont, sizeof(t_DispFont));
		return 0;				//����I��
	}
	return	1;					//�ُ�I��
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : DispBlinkDataSnd                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      :  �u�����N���ݒ菈��                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : t_DispBlink*	DispBlink			�u�����N���           |*/
/*| RETURN VALUE : �O�F����I���F�@�P�F�ُ�I���i�O��ݒ�l�Ɠ����j        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.Tanaka                                                |*/
/*| Date         : 2006-02-21                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned char	DispBlinkDataSnd( t_DispBlink*	DispBlink )
{
	//�O��ݒ�l�ƈقȂ�ꍇ
	if( ( Disp_blink_now[DispBlink->Row].Colmun !=  DispBlink->Colmun ) ||
		( Disp_blink_now[DispBlink->Row].Mode !=  DispBlink->Mode ) ||
		( Disp_blink_now[DispBlink->Row].Interval !=  DispBlink->Interval ) )
	{
		Disp_blink_now[DispBlink->Row].Colmun =  DispBlink->Colmun;
		Disp_blink_now[DispBlink->Row].Mode =  DispBlink->Mode;
		Disp_blink_now[DispBlink->Row].Interval =  DispBlink->Interval;
		can_snd_data4((void *)DispBlink, sizeof(t_DispBlink));
		return 0;				//����I��
	}
	// �O��Ɠ����ł��u�����N�J�n�v���ł���΃u�����N�w��𑗐M
	if( Disp_blink_now[DispBlink->Row].Mode == 1 ){
		can_snd_data4((void *)DispBlink, sizeof(t_DispBlink));
		return 0;				//����I��
	}
	return	1;					//�ُ�I��
}

