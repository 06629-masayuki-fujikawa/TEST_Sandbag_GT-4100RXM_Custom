/*[]----------------------------------------------------------------------[]*/
/*| headder file for LCDcontrol                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Y.Tanaka                                                 |*/
/*| Date        : 2002.08.XX                                               |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#ifndef _LCD_DEF_H_
#define _LCD_DEF_H_

#define LCD_CLM		0x1E

#define	RGB16(r, g, b)			((unsigned short)((((r)>>3) << 11) | (((g)>>2) << 5) | (((b)>>3) << 0)))
#define	COLOR_BLACK				((unsigned short)RGB16(  0,  0,  0))	// �u���b�N
#define	COLOR_RED				((unsigned short)RGB16(255,  0,  0))	// ���b�h
#define	COLOR_GREEN				((unsigned short)RGB16(  0,128,  0))	// �O���[��
#define	COLOR_BLUE				((unsigned short)RGB16(  0,  0,255))	// �u���[
#define	COLOR_MIDNIGHTBLUE		((unsigned short)RGB16( 25, 25,112))	// �~�b�h�i�C�g�u���[
#define	COLOR_PALETURQUOISE		((unsigned short)RGB16(175,238,238))	// �p�[���^�[�R�C�Y
#define	COLOR_LIGHTSEAGREEN		((unsigned short)RGB16( 32,178,170))	// ���C�g�V�[�O���[��
#define	COLOR_MEDIUMBLUE		((unsigned short)RGB16(  0,  0,205))	// �~�f�B�A���u���[
#define	COLOR_DODGERBLUE		((unsigned short)RGB16( 30,144,255))	// �h�W���[�u���[
#define	COLOR_MEDIUMSEAGREEN	((unsigned short)RGB16( 60,179,113))	// �~�f�B�A���V�[�O���[��
#define	COLOR_DARKGREEN			((unsigned short)RGB16(  0,100,  0))	// �_�[�N�O���[��
#define	COLOR_INDIGO			((unsigned short)RGB16( 75,  0,130))	// �C���f�B�S
#define	COLOR_DARKSLATEBLUE		((unsigned short)RGB16( 72, 61,139))	// �_�[�N�X���[�g�u���[
#define	COLOR_DARKGRAY			((unsigned short)RGB16(169,169,169))	// �_�[�N�O���[
#define	COLOR_DIMGRAY			((unsigned short)RGB16(105,105,105))	// �f�B���O���[
#define	COLOR_YELLOW			((unsigned short)RGB16(255,255,  0))	// �C�G���[
#define	COLOR_GOLD				((unsigned short)RGB16(255,215,  0))	// �S�[���h
#define	COLOR_DARKORANGE		((unsigned short)RGB16(255,140,  0))	// �_�[�N�I�����W
#define	COLOR_DARKGOLDENROD		((unsigned short)RGB16(184,134, 11))	// �_�[�N�S�[���f�����b�h
#define	COLOR_TOMATO			((unsigned short)RGB16(255, 99, 71))	// �g�}�g
#define	COLOR_SIENNA			((unsigned short)RGB16(160, 82, 45))	// �V�G���i
#define	COLOR_LIGHTBLUE			((unsigned short)RGB16(173,216,230))	// ���C�g�u���[
#define	COLOR_KHAKI				((unsigned short)RGB16(240,230,140))	// �J�[�L
#define	COLOR_BURLYWOOD			((unsigned short)RGB16(222,184,135))	// �o�[���[�E�b�h
#define	COLOR_FIREBRICK			((unsigned short)RGB16(178, 34, 34))	// �t�@�C���[�u���C�N
#define	COLOR_ROSYBROWN			((unsigned short)RGB16(188,143,143))	// ���[�Y�B�u���E��
#define	COLOR_DARKMAGENTA		((unsigned short)RGB16(139,  0,139))	// �_�[�N�}�[���^
#define	COLOR_PALEVIOLETRED		((unsigned short)RGB16(219,112,147))	// �y�[���o�C�I���b�g���b�h
#define	COLOR_CORNSILK			((unsigned short)RGB16(255,248,220))	// �R�[���V���N
#define	COLOR_THISTLE			((unsigned short)RGB16(216,191,216))	// �V�X��
#define	COLOR_CRIMSON			((unsigned short)RGB16(220, 20, 60))	// �N�����\��
#define	COLOR_HOTPINK			((unsigned short)RGB16(255,105,180))	// �z�b�g�s���N
#define	COLOR_WHITE				((unsigned short)RGB16(255,255,255))	// �z���C�g
// MH810100(S) S.Takahashi 2020/02/12 #3853 �d�����b�N������ʂŕ�������
//#define	COLOR_F1BLUE			((unsigned short)RGB16( 36,173,226))	// F1(DIC-2177)
#define	COLOR_F1BLUE			((unsigned short)RGB16( 85,255,255))	// F1
// MH810100(E) S.Takahashi 2020/02/12 #3853 �d�����b�N������ʂŕ�������
#define	COLOR_F2GREEN			((unsigned short)RGB16(138,206, 54))	// F2(DIC-2544)
#define	COLOR_F3YELLOW			((unsigned short)RGB16(255,186,  0))	// F3(DIC-86)
#define	COLOR_F4PINK			((unsigned short)RGB16(245,100,143))	// F4(DIC-50)
#define	COLOR_F5PURPLE			((unsigned short)RGB16(188,183,217))	// F5(DIC-45)

/* ���]�w�� */
#define LCD_REVERS_OFF		0					/* �ʏ�\��					*/
#define LCD_REVERS_ON		1					/* ���]�\��					*/

#define LCD_CURSOR_COLOR	COLOR_PALEVIOLETRED	/* �J�[�\���F(�󔒕������]�p) */

/* �_�ŊԊu(10msec�P��) */
#define LCD_BLINK_OFF		0x00				/* �_�łȂ�					*/
#define LCD_BLINK_ON		0x64				/* 1sec						*/

/* ̫�Ďw�� */
#define LCD_NORMAL_FONT		0					/* ɰ��̫��					*/
#define LCD_BIG_FONT		1					/* �ޯ��̫��				*/

/* �ި���ڲ�p�����߼��ݽ */
#define LCD_ESC_FONTTYPE	0x1b25				/* ����̫�Ďw��				*/
#define LCD_ESC_FONTCOLR	0x1b27				/* �����F�w��				*/
#define LCD_ESC_BLINK		0x1b28				/* ������̓_�Ŏw��			*/
#define LCD_ESC_CLEAR		0x1b2b				/* �ر�w��					*/
#define LCD_ESC_MENU		0x1b2c				/* �ƭ��\���w��				*/
#define LCD_ESC_STR			0x1b2d				/* ������w��				*/
#define LCD_ESC_BKCOLOR		0x1b2e				/* �w�i�F�w��				*/

// �O����`
#define LCD_GAIJI_LEFT_VERT		0x853f				/* ����g(���c��)			*/
#define LCD_GAIJI_LEFT_UP		0x8540				/* ����g(�����)			*/
#define LCD_GAIJI_RIGHT_UP		0x8541				/* ����g(�E���)			*/
#define LCD_GAIJI_HORIZON		0x8542				/* ����g(������)			*/
#define LCD_GAIJI_LEFT_DW		0x8543				/* ����g(������)			*/
#define LCD_GAIJI_RIGHT_DW		0x8544				/* ����g(�E����)			*/
#define LCD_GAIJI_RIGHT_VERT	0x8545				/* ����g(�E�c��)			*/
#define LCD_GAIJI_ARROW			0x8546				/* �E���					*/
#define LCD_GAIJI_F1_LH			0x8547				/* F1(������)				*/
#define LCD_GAIJI_F1_RH			0x8548				/* F1(�E����)				*/
#define LCD_GAIJI_F2A_LH		0x8549				/* F2A(������)				*/
#define LCD_GAIJI_F2A_RH		0x854A				/* F2A(�E����)				*/
#define LCD_GAIJI_F2R_LH		0x854B				/* F2R(������)				*/
#define LCD_GAIJI_F2R_RH		0x854C				/* F2R(�E����)				*/
#define LCD_GAIJI_F3_LH			0x854D				/* F3(������)				*/
#define LCD_GAIJI_F3_RH			0x854E				/* F3(�E����)				*/
#define LCD_GAIJI_F4_LH			0x854F				/* F4(������)				*/
#define LCD_GAIJI_F4_RH			0x8550				/* F4(�E����)				*/
#define LCD_GAIJI_F5_LH			0x8551				/* F5(������)				*/
#define LCD_GAIJI_F5_RH			0x8552				/* F5(�E����)				*/
#define LCD_GAIJI_NONE			0x0000

#define	LCD_LUMINE_DEF		11					/* �P�x�f�t�H���g�l			*/
#define	LCD_LUMINE_MAX		15					/* �ő�P�x�l				*/

// �R���g���[���R�[�h
#define CAN_CTRL_NONE	0x00		// ����`
#define CAN_CTRL_PI1	0x10		// �u�U�[���i�s�b��j
#define CAN_CTRL_PI2	0x20		// �u�U�[���i�s�s�b��j
#define CAN_CTRL_PI3	0x40		// �u�U�[���i�s�s�s�b��j
#define CAN_CTRL_BKLT	0x80		// �o�b�N���C�g

// �R���g���[���o��
#define CAN_LED_TRAY	0x80		// �ޑK��o���K�C�hLED
#define CAN_LED_TKEY	0x40		// �e���L�[�K�C�hLED(�\��)
#define CAN_LED_BTN1	0x20		// �ƌ��������{�^��1(�\��)
#define CAN_LED_BTN2	0x10		// �ƌ��������{�^��2(�\��)

#pragma	pack
// �R���g���[������p�\����
typedef struct {
	unsigned char	CtlCode;		// �R���g���[���R�[�h
	unsigned char	LEDPattan;		// LED�o�͐M��
} t_CanCtl;


// �����t�H���g�w��p�\����
typedef struct {
	unsigned short	Esc;			// �G�X�P�[�v�V�[�P���X
	unsigned char	Type;			// �t�H���g���(0=�m�[�}�� 1=�r�b�O)
} t_DispFont;

// �����t�H���g�J���[�w��p�\����
typedef struct {
	unsigned short	Esc;			// �G�X�P�[�v�V�[�P���X
	unsigned short	Color;			// RGB 0000h�`FFFFh(R=5bit G=6bit B=5bit)
} t_DispColor;

// ������u�����N�w��p�\����
typedef struct {
	unsigned short	Esc;			// �G�X�P�[�v�V�[�P���X
	unsigned char	Row;			// �s(0-7)
	unsigned char	Colmun;			// ��(�ݒ薳��)
	unsigned char	Mode;			// �J�n�^����(0=���� 1=�J�n)
	unsigned char	Interval;		// �_�ŊԊu(0�`255�~10msec)
} t_DispBlink;

// �N���A�w��p�\����
typedef struct {
	unsigned short	Esc;			// �G�X�P�[�v�V�[�P���X
	unsigned char	Row;			// �s(0-7)
	unsigned char	Colmun;			// ��(�ݒ薳��)
	unsigned char	Scope;			// �N���A�͈�(0=1�s�N���A 1=�S��ʃN���A)
} t_DispClr;

// ���j���[�\���w��p�\����
typedef struct {
	unsigned short	Esc;			// �G�X�P�[�v�V�[�P���X
	unsigned char	Row;			// �s(0-7)
	unsigned char	Colmun;			// ��(0-29)
	unsigned char	Msg[LCD_CLM];	// ������
} t_DispMenu;

// ������w��p�\����
typedef struct {
	unsigned short	Esc;			// �G�X�P�[�v�V�[�P���X
	unsigned char	Row;			// �s(0-7)
	unsigned char	Colmun;			// ��(0-29)
	unsigned char	Msg[LCD_CLM];	// ������
} t_DispStr;

// �J���[�p���b�g�w��p�\����
typedef struct {
	unsigned short	Esc;			// �G�X�P�[�v�V�[�P���X
	unsigned short	Rgb;			// �J���[�p���b�g(0=�f�t�H���g)
} t_BackColor;
// ���v�\���w��p�\����
#pragma unpack


extern	uchar	backlight;			// LCD�ޯ�ײċP�x�ް�
extern	uchar	blightLevel;		// �O���LCD�ޯ�ײċP�x
extern	uchar	blightMode;			// �O���LCD�ޯ�ײ�ON/OFF���


/*** Timer request area ***/
typedef	struct {
	unsigned long	ReqValue;				/* Timer request value (x10ms) : ex) 100=1s */
	unsigned long	StartTime;				/* Timer start time (LifeTime) */
} t_LCD_Timer;

#define	LCD_TIM1_VALUE	(unsigned long)80L	/* 800 msec */

/*** Blink data          ***/
typedef struct {
	unsigned short	lin;
	unsigned short	col;
	unsigned short	cnt;
	unsigned short	mod;
	unsigned short	color;
	unsigned char	dat[LCD_CLM];
} t_blk_lg;

#define LCD_BLK_LG				10


/*** Retry counter         ***/
#define LCD_NGSTA_RTYMAX	3

/*** Status check mode         ***/
#define LCD_STA01_CMD	0
#define LCD_STA2_ADR	2
#define LCD_STA3_ADW	3
#define LCD_STA5_ABL	5

typedef struct {
	unsigned short	Color;			// RGB 0000h�`FFFFh(R=5bit G=6bit B=5bit)
	unsigned char	Type;			// �t�H���g���(0=�m�[�}�� 1=�r�b�O)
}t_DispColorFontNow;
typedef struct {
	unsigned char	Colmun;			// ��(�ݒ薳��)
	unsigned char	Mode;			// �J�n�^����(0=���� 1=�J�n)
	unsigned char	Interval;		// �_�ŊԊu(0�`255�~10msec)
}t_DispBlinkNow;

extern t_DispColorFontNow	Disp_color_font_now;		//���ݐݒ蒆�̃J���[�ƃt�H���g��ʂ�ێ����Ă���
extern t_DispBlinkNow 		Disp_blink_now[8];			//���ݐݒ蒆�̃u�����N����ێ����Ă���

/*** dispinit.c          ***/
extern	char			dispinit( void );
extern	void			dispmod( unsigned short mod );
extern	void			dispclr( void );
extern	void			displclr( unsigned short line );
extern	void			dispmlclr( unsigned short from, unsigned short to );
extern	void			grachr( unsigned short lin,unsigned short col,unsigned short cnt,unsigned short mod, unsigned short color, unsigned short blink, const unsigned char * data );
extern	void			grawaku( unsigned char startLine,  unsigned char endLine );
extern	void			numchr( unsigned short lin, unsigned short col, unsigned short color, unsigned short blink, unsigned char  dat );
extern	void			bigcr( unsigned short lin, unsigned short col, unsigned short cnt, unsigned short color, unsigned short blink, const unsigned char *dat );
extern	void			blink_reg( unsigned short lin, unsigned short col, unsigned short cnt, unsigned short mod, unsigned short color, const unsigned char *dat );
extern	void			blink_end( void );
extern	void			Fun_Dsp( const unsigned char *dat );
extern	void			lcd_backlight( char mode );
extern	void			lcd_contrast( unsigned char value );
extern	void			Vram_BkupRestr( unsigned short mode ,unsigned short lin, short show_sw );


/*** dispsub.c          ***/
extern	void			Lcd_WmsgDisp_ON( unsigned short mod, unsigned short color, unsigned short blink, const uchar *msg );
																	// ܰ�ݸޕ\���J�n
extern	void			Lcd_WmsgDisp_OFF( void );					// ܰ�ݸޕ\���I��
extern	void			Lcd_Wmsg_Disp( void );						// ܰ�ݸ�ү���ޕ\��

extern	void			Lcd_WmsgDisp_ON2( unsigned short mod, unsigned short color, unsigned short blink, const uchar *msg );
																	// ܰ�ݸޕ\���J�n
extern	void			Lcd_WmsgDisp_OFF2( void );					// ܰ�ݸޕ\���I��
extern	void			Lcd_Wmsg_Disp2( void );						// ܰ�ݸ�ү���ޕ\��
extern	unsigned char	DispColorDataSnd( t_DispColor*	DispColor ); 	// �J���[���ݒ菈��
extern	unsigned char	DispFontDataSnd( t_DispFont*	DispFont );		// �t�H���g���ݒ菈��
extern	unsigned char	DispBlinkDataSnd( t_DispBlink*	DispBlink );	// �u�����N���ݒ菈��
extern	void	Funckey_one_Dsp( const unsigned char* dat, unsigned char num );
extern	void	grawaku2( unsigned short start_line );

/*** lcdctrl.c          ***/
extern	void	dspclk(ushort lin, ushort color);

//==============================================
// LCDܰ�ݸޕ\������
//==============================================
typedef struct
{
//	unsigned short	DspSts;					// ܰ�ݸ�ү����7�s�ڕ\�����	( LCD_WMSG_ON=�\���� / LCD_WMSG_OFF=���\�� )
	unsigned char	WarMsg;					// ܰ�ݸ�ү����7�s�ڕ\���ް�					(�\���������ް�)
	unsigned short	WarColor;				// ܰ�ݸ�ү����7�s�ڕ\���ް�					(�\���F)
	unsigned char	RecMsg;					// ܰ�ݸ�ү����7�s��OFF����ض�ް����\���ް�	(�\���������ް�)
	unsigned short	RecColor;				// ܰ�ݸ�ү����7�s��OFF����ض�ް����\���ް�	(�\���F)
//	unsigned short	DspSts2;				// ܰ�ݸ�ү����6�s�ڕ\�����	( LCD_WMSG_ON=�\���� / LCD_WMSG_OFF=���\�� )
	unsigned char	WarMsg2;				// ܰ�ݸ�ү����6�s�ڕ\���ް�					(�\���������ް�)
	unsigned short	WarColor2;				// ܰ�ݸ�ү����6�s�ڕ\���ް�					(�\���F)
	unsigned char	RecMsg2;				// ܰ�ݸ�ү����6�s��OFF����ض�ް����\���ް�	(�\���������ް�)
	unsigned short	RecColor2;				// ܰ�ݸ�ү����6�s��OFF����ض�ް����\���ް�	(�\���F)
} T_WMSG_CNT;

extern	unsigned short	DspSts;				// ܰ�ݸ�ү����7�s�ڕ\�����	( LCD_WMSG_ON=�\���� / LCD_WMSG_OFF=���\�� )
extern	unsigned short	DspSts2;			// ܰ�ݸ�ү����6�s�ڕ\�����	( LCD_WMSG_ON=�\���� / LCD_WMSG_OFF=���\�� )
extern	T_WMSG_CNT	Lcd_Wmsg_Cnt[LCD_CLM];	// ܰ�ݸޕ\������ر

#define		LCD_WMSG_ON		1		// ܰ�ݸޕ\������
#define		LCD_WMSG_OFF	0		// ܰ�ݸޕ\���Ȃ�
#define		LCD_WMSG_LINE	7		// ܰ�ݸޕ\���s�i7�s�ځj
#define		LCD_WMSG_LINE2	6		// ܰ�ݸޕ\���s�i6�s�ځj

#endif	// _LCD_DEF_H_
