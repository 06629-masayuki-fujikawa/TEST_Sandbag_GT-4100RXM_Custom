/*[]----------------------------------------------------------------------[]*/
/*| �������[�h����                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Y.Tanaka                                                 |*/
/*| Date        : 2005.04.12                                               |*/
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
#include	"mdl_def.h"
#include	"pri_def.h"
#include	"cnm_def.h"
#include	"tbl_rkn.h"
#include	"Lkcom.h"
#include	"prm_tbl.h"
#include	"common.h"
#include	"suica_def.h"
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#include	"fla_def.h"
#include	"mnt_def.h"

/* �`�F�b�N���j���[�ԍ�(��) */
enum{
	MAG_READER = 0, 		// ���C���[�_�[
	NOTE_READER,		// �������[�_�[
	COINMEC,			// �R�C�����b�N
	R_PRINTER,			// ���V�[�g�v�����^
	J_PRINTER,			// �W���[�i���v�����^
	KEY_DISP,			// �X�C�b�`�L�[/�\����
	LED_SHUTTER,		// LED/�V���b�^�[
	ANNOUNCE,			// �ē�����
	SWITCH,				// �X�C�b�`
	INOUTSIG,			// ���o�͐M��
	
	CHK_MNU_MAX,
};

// TODO:�����Őݒ肵�Ă���e�X�g�p�^�[���͎b��Ŏ��ۂɂ̓o�[�R�[�h��񂩂猟�����s���鍀�ڂ�U�蕪���邱�ƁI
char CHK_MODE_NUM[12][CHK_MNU_MAX] = 
{	
	{1,1,1,1,1,1,1,1,1,1},		// ���[�h1(�S�p�^�[��)
	{1,1,1,1,0,1,1,1,1,1},		// ���[�h2(�W���[�i���v�����^�Ȃ�)
	{1,0,0,0,0,0,0,0,0,0},		// ���[�h3(���C���[�_�[�̂�)
	{0,1,0,0,0,0,0,0,0,0},		// ���[�h4(�������[�_�[�̂�)
	{0,0,1,0,0,0,0,0,0,0},		// ���[�h5(�R�C�����b�N�̂�)
	{0,0,0,1,0,0,0,0,0,0},		// ���[�h6(���V�[�g�v�����^�̂�)
	{0,0,0,0,1,0,0,0,0,0},		// ���[�h7(�W���[�i���v�����^�̂�)
	{0,0,0,0,0,1,0,0,0,0},		// ���[�h8(�X�C�b�`�L�[/�\����̂�)
	{0,0,0,0,0,0,1,0,0,0},		// ���[�h9(�V���b�^�[�̂�)
	{0,0,0,0,0,0,0,1,0,0},		// ���[�h10(�����ē��̂�)
	{0,0,0,0,0,0,0,0,1,0},		// ���[�h11(�X�C�b�`�̂�)
	{0,0,0,0,0,0,0,0,0,1},		// ���[�h12(���o�͐M���̂�)
};

enum {
	CIK_KNO = 0,
	CIK_TYPE,
	CIK_ROLE,
	CIK_LEVEL,
	_CIK_MAX_
};

static short mag_Chk( void );
static short Note_Chk( void );
static short Coin_Chk( void );
static short RJ_Print_Chk( void );
static short Key_DispChk( void );
static short Led_Shut_Chk( void );
static short Ann_Chk( void );
static short Inout_Chk( void );
static short Sig_Chk( void );

static unsigned char Init_Magread( void );
static unsigned char Init_NoteReader( void );
static unsigned char Init_R_Printer( void );
static unsigned char Init_J_Printer( void );
static unsigned char Init_Sound( void );

static short indexToAnn( char index );

typedef short (*FUNC)(void);
typedef uchar (*INIT)(void);

typedef struct{
	unsigned char menu;
	FUNC Func;
	INIT Init;
}CHK_FUNC;

CHK_FUNC Chkfunc_tbl[] = {
	{ MAG_READER,		mag_Chk, 		Init_Magread	},
	{ NOTE_READER,		Note_Chk, 		Init_NoteReader	},
	{ COINMEC,			Coin_Chk, 		NULL	},
	{ R_PRINTER,		RJ_Print_Chk, 	Init_R_Printer	},
	{ J_PRINTER,		RJ_Print_Chk, 	Init_J_Printer	},
	{ KEY_DISP,			Key_DispChk,  	NULL	},
	{ LED_SHUTTER,		Led_Shut_Chk, 	NULL	},
	{ ANNOUNCE,			Ann_Chk,  		Init_Sound		},
	{ SWITCH,			Inout_Chk, 		NULL	},
	{ INOUTSIG,			Sig_Chk,		NULL	},
};

/* �@����ƍ��̃`�F�b�N�e�[�u�� */
static	struct {
	char	sec;		// �@��̐ݒ�(�Z�N�V����)
	char	adr;		// �@��̐ݒ�(�A�h���X)
	char	pos;		// �@��̐ݒ�(���ʒu)
	char	err_m;		// �`�F�b�N����G���[(���W���[��)
	char	err_c;		// �`�F�b�N����G���[(�R�[�h)
} Chk_List[] = {
	{	S_PAY,	21,	3,	ERRMDL_READER,	01},		// MAG_READER,
	{	NULL,	 0,	0,	ERRMDL_NOTE,	01},		// NOTE_READER,
	{	NULL,	 0,	0,	ERRMDL_COIM,	01},		// COINMEC,
	{	NULL,	 0,	0,	ERRMDL_PRINTER,	01},		// R_PRINTER,
	{	S_PAY,	21,	1,	ERRMDL_PRINTER,	20},		// J_PRINTER,
	{	NULL,	 0,	0,	NULL,			00},		// KEY_DISP,
	{	NULL,	 0,	0,	NULL,			00},		// LED_SHUTTER,
	{	NULL,	 0,	0,	NULL,			00},		// ANNOUNCE,
	{	NULL,	 0,	0,	NULL,			00},		// SWITCH,	
	{	NULL,	 0,	0,	NULL,			00},		// INOUTSIG,
	{0,0,0,0,0},
};

// MH810100(S)
//const unsigned char	SYSMNT_CHKRESULT_FILENAME[] = {"FT4000_%s_%02d%02d%02d%02d%02d.csv"};		// ���ʏ��t�@�C�����̃x�[�X
const unsigned char	SYSMNT_CHKRESULT_FILENAME[] = {"GT4100_%s_%02d%02d%02d%02d%02d.csv"};		// ���ʏ��t�@�C�����̃x�[�X
// MH810100(E)
const unsigned char	SYSMNT_CHKRESULT_DIRECT[] = {"/SYS/"};			// ���ʏ��t�@�C���̃f�B���N�g��
void ChekModeMain(void);
short Input_Moj_Rev(void);
void Test_Param_Set(ulong chk_no);
short Test_Chk_Init(short *tbl, ushort no);
unsigned char Chk_Memory(void);
void NG_key_chek( void );
void Disp_Chk_NG(const unsigned char (*dat)[31], unsigned char cnt, unsigned char *result);
void Chk_Exit(void);
unsigned char Chkmod_Sw_rd(char);
unsigned char Init_Chkmod(void);
char Chk_mod;

extern long prm_set(char kin, short ses, short adr, char len, char pos, long dat);

/*[]----------------------------------------------------------------------[]*/
/*| �������[�h���C������                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void ChekModeMain(void){
	
	/* �`�F�b�N���[�h�I����ʕ\�� */
	ushort msg;
	short	Chk_Tbl[CHK_MNU_MAX];
	char	faze,i,j,fuku;
	char	chk_cnt = 0;
	ushort	inp, kakari_wk, lin;
	ulong	chk_no = 0;

	OPE_SIG_OUT_DOORNOBU_LOCK(0);									// �d�����b�N����
	key_read();														// key scan data read
	ExIOSignalwt(EXPORT_JP_RES, 0);									// �W���[�i���v�����^���Z�b�g
	Ope_KeyRepeatEnable(0);
	inp = 0;
	faze = 0;
	fuku = 0;
	
	fuku = Init_Chkmod();
	if(fuku == 1){		// �����r���̕��d
		memset(&Chk_result.set, 0xff, sizeof(Chk_result.set));		/* �e���W���[���̏��������s�����ߋ@��\�����̓N���A */
		goto _CHK_START;
	}
	else if(fuku == 2){
		goto _CHK_END;
	}
	
	for( ; ; )
	{
		if(faze == 0){						// ��ʕ\��
			dispclr();
			chk_cnt = 0;
			bigcr(0, 2, 18, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (uchar *)CHK_CHR_BIG[1]);	// "
			grachr(2, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, OPE_CHR[2]);				// "������������������������������"
			grachr(3, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, CHK_CHR[6]);				// "
			grawaku2( 4 );
			teninb(inp, 4, 5, 10, COLOR_FIREBRICK);	// ���Ԉʒu�ԍ��\��
			faze = 1;
		}
		msg = GetMessage();
		switch (KEY_TEN0to9(msg)) {
		/* mode change */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)

			case KEY_MODECHG:			// �����e�i���X�L�[�͖�������
				break;
		/* number key */
			case KEY_TEN:
				BUZPI();
				if(faze == 1){					// �Ј��ԍ��̓���
					lin = 5;
					inp = ((inp*10 + (msg - KEY_TEN))%10000);
					teninb(inp, 4, lin, 10, COLOR_FIREBRICK);	// �ԍ��\��
				}
				else if(faze == 2){				// �����p�^�[���̓���
					lin = 3;
					chk_no = ((chk_no*10 + (msg - KEY_TEN))%1000000L);
					teninb2(chk_no, 6, lin, 6, COLOR_FIREBRICK);	// �ԍ��\��
				}
				else{
					break;
				}
				break;
			case KEY_TEN_CL:				// �N���A�L�[
				BUZPI();
				if(faze == 1){
					lin = 5;
					inp = 0;
					teninb(inp, 4, lin, 10, COLOR_FIREBRICK);	// �ԍ��\��
				}
				else if(faze == 2){
					lin = 3;
					chk_no = 0;
					teninb2(chk_no, 6, lin, 6, COLOR_FIREBRICK);	// �ԍ��\��
				}
				break;
			case KEY_TEN_F1:			// �߂�
				if(faze == 2){
					faze = 0;
					chk_no = 0;
				}
				break;
			case KEY_TEN_F2:
			case KEY_TEN_F4:
				break;
			case KEY_TEN_F3:			// FTP
				BUZPI();
				SysMnt_FTPServer();
				faze = 0;
				break;
			case KEY_TEN_F5:			// ����
				if(faze == 1){			// �Ј��ԍ�
					if(inp == 0){
						BUZPIPI();
						break;
					}
					kakari_wk = inp;
					faze = 2;			// �����p�^�[�����͂�
					inp = 0;
					dispclr();
					grachr(0, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, CHK_CHR[7]);
					grachr(6, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, CHK_CHR[2]);
					/* ���_�\���͂��܂̂Ƃ���ł��Ȃ��݂����Ȃ̂łO�\�� */
					teninb2(chk_no, 6, 3, 6, COLOR_FIREBRICK);
					Fun_Dsp( FUNMSG[114] );
					break;
				}
				if(chk_no == 0 || chk_no >= 12){				// TODO:�b�菈��
					if( faze == 2 ){
						lin = 3;
					}
					BUZPIPI();
					chk_no = 0;
					teninb2(chk_no, 6, lin, 6, COLOR_FIREBRICK);
					break;
				}
				BUZPI();
				/* �`�F�b�N���[�h���� */
				/* �`�F�b�N���鍀�ڂƐ��Z�@�̍\�����r���� */
				/* �@��\���`�F�b�N�������� */
				Chk_info.pt_no = chk_no;
				memset(&Chk_result, 0xff, sizeof(Chk_result));		// �������ʊi�[�o�b�t�@������
				Chk_result.Kakari_no = kakari_wk;
				Chk_result.Chk_no = chk_no;
				Chk_result.Card_no = 0;
				memcpy( &Chk_result.Chk_date, &CLK_REC, sizeof(date_time_rec));
				memset(&Chk_res_ftp, 0, sizeof(t_Chk_res_ftp));

				Test_Param_Set( chk_no );							// ���ʃp�����[�^�Z�b�g
				return;												// ���ʃp�����[�^�Z�b�g��A�ċN������̂ł����ɗ��邱�Ƃ͂Ȃ��B
_CHK_START:
				strcpy((char*)Chk_info.Chk_str1, (char*)CHKMODE_STR1);			// �Œ蕶����@�Z�b�g
				chk_cnt = Test_Chk_Init(Chk_Tbl, Chk_info.pt_no-1);			// �������`�F�b�N�����W���[������������
_CHK_MODULE:
				/* ���W���[����� */
				if(Input_Moj_Rev() == -1){			// �߂�
					inp = 0;
					chk_no = 0;
					dispclr();
					grachr(0, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, CHK_CHR[7]);
					grachr(6, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, CHK_CHR[2]);
					/* ���_�\���͂��܂̂Ƃ���ł��Ȃ��݂����Ȃ̂łO�\�� */
					teninb2(chk_no, 6, 3, 6, COLOR_FIREBRICK);
					Fun_Dsp( FUNMSG[111] );
					break;
				}
				for(i = 0; i < chk_cnt; i++){
_CHK_REVERSE:
					for(j=0;i<CHK_MNU_MAX;j++){
						if(Chkfunc_tbl[j].menu == Chk_Tbl[i]){
							if(fuku){				// �╡�d��̌����ĊJ
								if(Chk_info.Chk_mod != 0){
									if(Chk_info.Chk_mod != Chk_Tbl[i]){		// ��d���Ɏ��{���Ă��������܂ŃX�L�b�v
										break;
									}
								}
								fuku = 0;
							}
							Chk_info.Chk_mod = Chkfunc_tbl[j].menu;
							if(Chkfunc_tbl[j].Func() == -1){			// �e�������{
								// �߂�
								if(i == 0){
									// ���W���[�����
									goto _CHK_MODULE;
								}
								else{
									i--;
									goto _CHK_REVERSE;
								}
								break;
							}
							else{
								// ����
								break;
							}
						}
					}
				}
				strcpy((char*)Chk_info.Chk_str2, (char*)CHKMODE_STR2);			// �Œ蕶����A�Z�b�g
_CHK_END:
				if(fuku == 2){
					// ���d����̌����I����ʎ��͈󎚂ł���悤�Ƀ��W���[�����������s��
					Test_Chk_Init(Chk_Tbl, Chk_info.pt_no-1);					// �������`�F�b�N�����W���[������������
				}
				Chk_Exit();						// �����I�����
				faze = 0;						// �����I��
				inp = 0;
				chk_no = 0;
				fuku = 0;
				break;
			
			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �p�����[�^�Z�b�g����                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Test_Param_Set( ulong chk_no )                          |*/
/*| PARAMETER    : chk_no	:�����p�^�[���ԍ�                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void Test_Param_Set( ulong chk_no )
{
	ushort	msg;
	uchar	i;
	long	val;

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CHK_CHR[9]);
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CHK_CHR[10]);
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CHK_CHR[11]);
	Fun_Dsp( FUNMSG[0] );

	Lagtim( OPETCBNO, 8, (50*10));			// �����ċN���܂ł�10�b�^�C�}�Z�b�g

	strcpy((char*)Chk_info.Chk_str1, (char*)CHKMODE_STR1);			// �Œ蕶����@�Z�b�g
	Chk_info.Chk_mod = 0;

	prm_clr( COM_PRM, 1, 0 );				// �������Ұ���̫��

	// �h�A�m�u�߂��Y��A���[������
	CPrmSS[S_PAY][39] = 0;


	// TODO:�����p�^�[�����m�肵�Ă��Ȃ��̂Ŏb��
	for( i=0; i<CHK_MNU_MAX; i++){
		switch(i){
		case 0:		// ���C���[�_�[
			if(CHK_MODE_NUM[chk_no-1][i]){
				// ��������
			}
			else{
				// �������Ȃ�
			}
			break;
		case 1:		// �������[�_�[
			if(CHK_MODE_NUM[chk_no-1][i]){
				// ��������
			}
			else{
				// �������Ȃ�
			}
			break;
		case 2:		// �R�C�����b�N
			if(CHK_MODE_NUM[chk_no-1][i]){
				// ��������
			}
			else{
				// �������Ȃ�
			}
			break;
		case 3:		// ���V�[�g�v�����^
			if(CHK_MODE_NUM[chk_no-1][i]){
				// ��������
			}
			else{
				// �������Ȃ�
			}
			break;
		case 4:		// �W���[�i���v�����^
			if(CHK_MODE_NUM[chk_no-1][i]){
				// ��������
				val = prm_set(COM_PRM, S_PAY, 21, 1, 1, 1);
				CPrmSS[S_PAY][21] = val;
			}
			else{
				// �������Ȃ�
				val = prm_set(COM_PRM, S_PAY, 21, 1, 1, 0);
				CPrmSS[S_PAY][21] = val;
			}
			break;
		case 5:		// �X�C�b�`�L�[/�\����
			if(CHK_MODE_NUM[chk_no-1][i]){
				// ��������
			}
			else{
				// �������Ȃ�
			}
			break;
		case 6:		// LED/�V���b�^�[
			if(CHK_MODE_NUM[chk_no-1][i]){
				// ��������
			}
			else{
				// �������Ȃ�
			}
			break;
		case 7:		// �ē�����
			if(CHK_MODE_NUM[chk_no-1][i]){
				// ��������
			}
			else{
				// �������Ȃ�
			}
			break;
		case 8:		// �X�C�b�`
			if(CHK_MODE_NUM[chk_no-1][i]){
				// ��������
			}
			else{
				// �������Ȃ�
			}
		case 9:		// ���o�͐M��
			if(CHK_MODE_NUM[chk_no-1][i]){
				// ��������
			}
			else{
				// �������Ȃ�
			}
			break;
		default:
			break;
		}
	}

	DataSumUpdate(OPE_DTNUM_COMPARA);

	while( 1 ){
		msg = GetMessage();					// ү���ގ�M
		switch(msg){						// ��Mү���ށH
			case TIMEOUT8:					// �ċN����ϰ��ѱ��
				Lagcan( OPETCBNO, 6 );
				// �V�X�e���ċN��
				System_reset();				// Main CPU reset (It will not retrun from this function)
				break;
			default:
				break;
		}
	}

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| �e���W���[������������                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Test_Chk_Init(short *tbl, ushort no)                    |*/
/*| PARAMETER    : *tbl	:�`�F�b�N���j���[�e�[�u���ւ̃|�C���^              |*/
/*|                no	:�`�F�b�N�p�^�[���i���o�[                          |*/
/*| RETURN VALUE : unsigned short: �`�F�b�N���鍀�ڐ�                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short Test_Chk_Init(short *tbl, ushort no)
{
	short result,cnt,i;
	cnt = 0;
	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CHK_CHR[3]);
	bigcr(3, 0, 20, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (uchar *)CHK_CHR_BIG[2]);
	Fun_Dsp( FUNMSG[0] );
	/* �������`�F�b�N */
	Chk_Memory();

	/* �e���W���[�������� */
	for(i = 0; i < CHK_MNU_MAX; i++){
		if( CHK_MODE_NUM[no][i] == 1){	// �������{�H
			if( Chk_List[i].sec != NULL ){
				if( 0 == prm_get(COM_PRM, Chk_List[i].sec, Chk_List[i].adr, 1, Chk_List[i].pos) ){
					/* �ݒ�Ȃ� */
					continue;
				}
			}
			
			if(Chkfunc_tbl[i].Init != NULL){
				Chk_mod = i;
				result = Chkfunc_tbl[i].Init();		// �������������{
			}
			else{
				result = 0;
			}
			if(result == 0){				// ����������
				tbl[cnt] = i;				// �`�F�b�N���ڔԍ��i�[
				cnt++;						// ����+1
			}
		}
	}
	return cnt;
}
/*[]----------------------------------------------------------------------[]*/
/*| ���W���[���o�[�R�[�h�ǂݎ�菈��                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : short 0:���� -1:�߂�                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define	BLUETOOTH_EVENT	100			// �o�[�R�[�h�X�L�����ʒm�̃C�x���g(��)
#define	MOJU_ANN_ST	200				// ���W���[���̃A�i�E���X�ԍ��J�n
#define	MOJU_MAX	16				// ���W���[���̍ő��
short Input_Moj_Rev(void)
{
	ushort msg;
	char	i, lin, index;
	
	dispclr();
	
	memset(&Chk_result.moj, 0, 16);
	for(i=0;i<7;i++){
		grachr( i,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MOJ_INPUT_STR[i] );
	}
	Fun_Dsp( FUNMSG[111] );
	avm_test_cnt = 2;
	lin = 1;					// ���]�\������s��
	index = 1;					// ���]�\�����镶����ԍ�
	avm_test_no[0] = 200;		// ���ꂩ��w������u���b�N�̃o�[�R�[�h���X�L�������Ă�������
	avm_test_no[1] = indexToAnn( index );		// RXM���
	
	ope_anm( AVM_AN_TEST );		// ���肪�Ƃ��������܂����B
	grachr( 1,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, MOJ_INPUT_STR[1] );		// ���]�\��
	for(;;){
			
		msg = GetMessage();
		switch(msg){
			case BLUETOOTH_EVENT:
				/* ��M�f�[�^�����ʃG���A�� */
			case KEY_TEN_F1:		// �߂�
				BUZPI();
				return -1;
			case KEY_TEN_F2:		// ��
				if(index == 1){
					break;
				}
				grachr( lin,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MOJ_INPUT_STR[index] );		// �ʏ�\��
				index--;
				lin--;
				if(lin == 0){
					dispmlclr(1,6);
					for(i=0;i<6;i++){
						grachr( i+1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MOJ_INPUT_STR[(index-5)+i] );
					}
					lin = 6;
				}
				grachr( lin,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, MOJ_INPUT_STR[index] );		// ���]�\��
				avm_test_no[0] = indexToAnn( index );		// �O�̃A�i�E���X�ԍ�
				avm_test_cnt = 1;
				ope_anm( AVM_AN_TEST );		// ���肪�Ƃ��������܂����B
				break;
			case KEY_TEN_F3:		// ��
				if(index >= MOJU_MAX){
					return 0;
				}
				grachr( lin,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MOJ_INPUT_STR[index] );		// �ʏ�\��
				index++;
				lin++;
				if(lin == 7){
					dispmlclr(1,6);
					for(i=0;i<6;i++){
						grachr( i+1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MOJ_INPUT_STR[index+i] );
						if((index+i)>=MOJU_MAX){
							break;
						}
					}
					lin = 1;
				}
				grachr( lin,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, MOJ_INPUT_STR[index] );		// ���]�\��
				avm_test_no[0] = indexToAnn( index );		// ���̃A�i�E���X�ԍ�
				avm_test_cnt = 1;
				ope_anm( AVM_AN_TEST );		// ���肪�Ƃ��������܂����B
				break;
			case KEY_TEN_F5:		// ����
				BUZPI();
				return 0;
			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �A�i�E���X�ԍ��擾                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : indexToAnn( char index )                                |*/
/*| PARAMETER    : char index : ���W���[���@��                             |*/
/*| RETURN VALUE : short      : �A�i�E���X�ԍ�                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static short indexToAnn( char index )
{
	short ann = 0;

	switch(index){
	case 1:			// RXM���
		ann = 201;
		break;
	case 2:			// RXF���
		ann = 202;
		break;
	case 3:			// RXI���
		ann = 203;
		break;
	case 4:			// �R�C�����b�N
		ann = 205;
		break;
	case 5:			// �������[�_�[
		ann = 206;
		break;
	case 6:			// �d�q���σ��[�_�[
		ann = 207;
		break;
	case 7:			// �W���[�i���v�����^�[
		ann = 208;
		break;
	case 8:			// ���V�[�g�v�����^�[
		ann = 209;
		break;
	case 9:			// ������FOMA���W���[��
		ann = 210;
		break;
	case 10:		// �E����FOMA���W���[��
		ann = 211;
		break;
	case 11:		// ���C���[�_�[
		ann = 213;
		break;
	case 12:		// V�v�����^�[
		ann = 214;
		break;
	case 13:		// �J���[LCD���W���[��
		ann = 215;
		break;
	case 14:		// �d�����j�b�g
		ann = 218;
		break;
	case 15:		// �g���d�����j�b�g
		ann = 219;
		break;
	case 16:		// �e���L�[���W���[��
		ann = 220;
		break;
	default:
		break;
	}
	return ann;
}

/*[]----------------------------------------------------------------------[]*/
/*| �L�[����/�\����m�F                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChkMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short Key_DispChk( void )
{
	struct {
		unsigned short str;		// ������̔z��ԍ�
		unsigned short lin;		// �\���s
		unsigned short col;		// �\���ʒu
		unsigned short back;	// �ω�������w�i�F�A�P�x
	} KEY_pos[16] = {
	//  {�\���s, �\���ʒu, �w�i}
		{6,	5,	11,	COLOR_BLACK},		// �O
		{3,	2,	 5,	COLOR_WHITE},		// �P
		{3,	2,	11,	COLOR_HOTPINK},		// �Q
		{3,	2,	17,	COLOR_BLUE},		// �R
		{4,	3,	 5,	COLOR_GREEN},		// �S
		{4,	3,	11,	COLOR_GOLD},		// �T
		{4,	3,	17,	COLOR_SIENNA},		// �U
		{5,	4,	 5,	COLOR_RED},			// �V
		{5,	4,	11,	COLOR_PALEVIOLETRED},// �W
		{5,	4,	17,	0},					// �X
		{3,	2,	23,	0},					// F1
		{4,	3,	23,	1},					// F2
		{5,	4,	23,	6},					// F3
		{6,	5,	23,	11},				// F4
		{6,	5,	17,	LCD_LUMINE_MAX},	// F5
		{6,	5,	 5,	COLOR_PALETURQUOISE},// �b
	};
	
	char	KEY_flg[16] = {0};
	unsigned short	color;
	ushort msg;
	char	i,Chk_end,key_comp,disp_reset;
	
	Chk_end = 0;
	key_comp = 0;
	disp_reset = 1;
	memset(&Chk_result.key_disp, 0, 2);
	for( ;Chk_end == 0; )
	{
		if(disp_reset){
			/* ��ʕ`�悵�Ȃ��� */
			dispclr();													// Display All Clear
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, KEY_DISP_STR[0]);
			for(i = 0; i < 16; i++){
				if(KEY_flg[i]){
					color = COLOR_GOLD;					// �����ς݂̃L�[�̐F
				}
				else{
					color = COLOR_SIENNA;				// �������̃L�[�̐F
				}
				grachr( KEY_pos[i].lin,  KEY_pos[i].col, 2, 0, 
						color, LCD_BLINK_OFF, &KEY_DISP_STR[KEY_pos[i].str][KEY_pos[i].col] );	// �����ꂽ�L�[��F��؂�ւ��ĕ\��
			}
			if((key_comp) || (Chk_result.key_disp[0] == 1)){
				Fun_Dsp( FUNMSG[109] );				
			}else{
				Funckey_one_Dsp(&FUNMSG[109][18], 4);
			}
			disp_reset = 0;
		}
		msg = GetMessage();
		if(msg >= KEY_TEN0 && msg <= KEY_TEN_CL){
			if(msg == KEY_TEN_F1 && ((key_comp) || (Chk_result.key_disp[0] == 1))){
				dsp_background_color(COLOR_WHITE);
				dispclr();													// Display All Clear
				lcd_backlight(1);
				return -1;
			}
			/* �S�ĉ����ς݂̏�Ԃ�F5�L�[��������I�� */
			if(msg == KEY_TEN_F5 && ((key_comp) || (Chk_result.key_disp[0] == 1))){
				dsp_background_color(COLOR_WHITE);
				dispclr();													// Display All Clear
				lcd_backlight(1);
				return 0;
			}
			/* �����ꂽ�L�[�̔��]�\�� */
			KEY_flg[msg-KEY_TEN] = 1;
			color = COLOR_GOLD;
			grachr( KEY_pos[msg-KEY_TEN].lin,  KEY_pos[msg-KEY_TEN].col, 2, 0, 
					color, LCD_BLINK_OFF, &KEY_DISP_STR[KEY_pos[msg-KEY_TEN].str][KEY_pos[msg-KEY_TEN].col] );	// �����ꂽ�L�[��F��؂�ւ��ĕ\��
			for(i = 0; i < 16; i++){
				if(KEY_flg[i] == 0){
					break;
				}
			}
			if(i >= 15){
				key_comp = 1;
				Fun_Dsp( FUNMSG[109] );
			}
		}
			
		switch (msg) {
		/* mode change */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				break;
		/* number key */
			case KEY_TEN0:
			case KEY_TEN1:
			case KEY_TEN2:
			case KEY_TEN3:
			case KEY_TEN4:
			case KEY_TEN5:
			case KEY_TEN6:
			case KEY_TEN7:
			case KEY_TEN8:
			case KEY_TEN_CL:
				BUZPI();
				dsp_background_color(KEY_pos[msg-KEY_TEN].back);			// �w�i�F�ύX
				disp_reset = 1;												// ��ʍĕ`��
				break;
			case KEY_TEN9:
				BUZPI();
				break;
			case KEY_TEN_F1:		// 
			case KEY_TEN_F2:		// 
			case KEY_TEN_F3:		// 
			case KEY_TEN_F4:		// 
			case KEY_TEN_F5:		// 
				BUZPI();
				lcd_contrast((uchar)KEY_pos[msg-KEY_TEN].back);				// �o�b�N���C�g�i�R���g���X�g�j
				break;
			case TIMEOUT8:
				dsp_background_color(COLOR_WHITE);
				lcd_backlight(1);
				Disp_Chk_NG(KEY_DISP_NGSTR, 2, &Chk_result.key_disp[0]);
				disp_reset = 1;
				break;
			default:
				break;
		}
		NG_key_chek();
	}
	return 0;
}
/*[]----------------------------------------------------------------------[]*/
/*| ���C���[�_�[�`�F�b�N                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short mag_Chk( void )
{
	short		msg = -1;			// ��Mү����
	unsigned short	Chk_end = 0;	// �`�F�b�N�I���t���O
	char		faze,disp;
	uchar		prn;
	m_kakari	*crd_data;			// ���C�����ް��߲��
	ulong		atend_no = 0;
	char		write_mode;			// ���C�g���[�h��� 0:�����ȊO 1:������
	uchar		IdSyuPara;			// �g�pID��ݒ����Ұ�
	char		prm[_CIK_MAX_];		// �ݒ�p�����[�^
	uchar		type_tbl[] = {0x20, 0x49, 0x4b, 0x53, 0x4a};	/* ' ', 'I', 'K', 'S', 'J' */

	dispclr();

	prn = (uchar)prm_get( COM_PRM, S_PAY, 21, 1, 3 );	// ���Cذ�ް��ʎ擾
	if( prn == 1 || prn == 2 ){						// �v�����^����H
		prn = 1;
	}
	else{
		prn = 0;
	}

	prm[CIK_KNO] = 1;
	prm[CIK_TYPE] = 2;
	prm[CIK_ROLE] = 1;
	prm[CIK_LEVEL] = 0;

	faze = 0;
	disp = 1;
	OPE_red = 0;
	for( ; Chk_end == 0 ; ){
		if(disp != 0){
			grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MAG_CHK_STR[disp] );	//
			disp = 0;
		}

		if( faze == 0 ){
			read_sht_opn();						// ���Cذ�ް������u�J�v
			opr_snd(200);						// �J�[�h��荞�ݗv��
			write_mode = 1;						// �����ݒ�
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MAG_CHK_STR[0] );	//
			displclr(2);
			grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MAG_CHK_STR[5] );	// " �������J�[�h��}�����ĉ����� "
			bigcr(4, 10, 10, COLOR_FIREBRICK, LCD_BLINK_OFF, (uchar *)CHK_CHR_BIG[3]);
			faze = 1;
			Fun_Dsp( FUNMSG[109] );
		}
		msg = GetMessage();				// ү���ގ�M
		switch(msg){
			case TIMEOUT13:
				displclr(6);
				Lagcan( OPETCBNO, 13 );
				if(faze == 3){
					if( opr_snd( 93 ) != 0 ){		// ��ýėv��
						Chk_end = 1;
					}
				}
				break;
			case ARC_CR_R_EVT:
				if(MAGred[MAG_ID_CODE] != 0x41){		// �W���J�[�h�ȊO
					grachr( 6,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, MAG_CHK_STR[7] );	// "���̃J�[�h�͎g���܂���"
					Lagtim( OPETCBNO, 13, (50*6));
					ope_anm( AVM_CARD_ERR1 );
					opr_snd(13);
					break;
				}
				if(faze == 2){				// ���[�h�e�X�g
					crd_data = (m_kakari *)&MAGred[MAG_ID_CODE];
					atend_no = (( (crd_data->kkr_kno[0] & 0x0f) * 1000)  +						//1000�̂��擾
								( (crd_data->kkr_kno[1] & 0x0f) * 100 )	 +						//100�̌����擾
								( (crd_data->kkr_kno[2] & 0x0f) * 10 )	 +						//10�̌����擾
								( (crd_data->kkr_kno[3] & 0x0f) * 1 ));							//1�̌����擾
					teninb((ushort)atend_no, 2, 4, 13, COLOR_FIREBRICK);	// �������ԍ��\��
					Chk_result.Card_no = atend_no;
					displclr(6);
					if(prn){						// �v�����^����H
						opr_snd(2);					// �߂�
					}
					else{							// �v�����^�Ȃ��̏ꍇ�͔p�����Ė߂�
						if( opr_snd( 102 ) == 0 ){				// �p��
							faze = 4;							// �����C�e�X�g
							disp = 4;
						}
					}
				}
				else if(faze == 4){					// �󎚃e�X�g��̌��}���������C�e�X�g
					if(opr_snd(102) != 0){
						Chk_end = 1;
					}
				}
				break;
			case ARC_CR_E_EVT:
				switch(faze){
				case 1:
					if (write_mode == 1) {
						/* �W���J�[�h�f�[�^�쐬 */
						crd_data = (m_kakari *)MDP_buf;
						if( (prm_get( COM_PRM,S_PAY,10,1,4 ) != 0) &&						// �V�J�[�h�h�c�g�p����H
							(prm_get( COM_PRM,S_SYS,12,1,6 ) == 0) ){						// APS�t�H�[�}�b�g�Ŏg�p����H
							// �V�J�[�h�h�c���g�p����ꍇ
							IdSyuPara = (uchar)prm_get( COM_PRM,S_PAY,10,1,1 );				// ��{���ԏ�No.��ID�i�W�����ށj�ݒ����Ұ��擾
							switch( IdSyuPara ){
								case	1:													// ID1
									crd_data->kkr_idc = 0x41;
									break;
								case	2:													// ID2
									crd_data->kkr_idc = 0x69;
									break;
								case	3:													// ID3
									crd_data->kkr_idc = 0x6F;
									break;
								case	4:													// ID4
									crd_data->kkr_idc = 0x75;
									break;
								case	5:													// ID5
									crd_data->kkr_idc = 0x7B;
									break;
								case	6:													// ID1��ID2
									crd_data->kkr_idc = 0x41;
									break;
								case	7:													// ID1��ID3
									crd_data->kkr_idc = 0x41;
									break;
								case	8:													// ID6
									crd_data->kkr_idc = 0x7E;
									break;
								case	0:													// �ǂ܂Ȃ�
								default:													// ���̑��i�ݒ�l�װ�j
									crd_data->kkr_idc = 0x41;
									break;
							}
						}
						else{
							// �V�J�[�h�h�c���g�p���Ȃ��ꍇ
							crd_data->kkr_idc = 0x41;
						}
						crd_data->kkr_year[0] = (uchar)(CLK_REC.year%100/10 + 0x30);
						crd_data->kkr_year[1] = (uchar)(CLK_REC.year%10 + 0x30);
						crd_data->kkr_mon[0] = (uchar)(CLK_REC.mont/10 + 0x30);
						crd_data->kkr_mon[1] = (uchar)(CLK_REC.mont%10 + 0x30);
						crd_data->kkr_day[0] = (uchar)(CLK_REC.date/10 + 0x30);
						crd_data->kkr_day[1] = (uchar)(CLK_REC.date%10 + 0x30);
						crd_data->kkr_did = 0x53;
						if( prm_get( COM_PRM,S_SYS,12,1,6 ) == 1){//GT�t�H�[�}�b�g
							crd_data->kkr_park[0] = 0x50;	/* 'P' */
							crd_data->kkr_park[1] = 0x47;	/* 'G' */
							crd_data->kkr_park[2] = 0x54;	/* 'T' */
						}else{//APS�t�H�[�}�b�g
							crd_data->kkr_park[0] = 0x50;	/* 'P' */
							crd_data->kkr_park[1] = 0x41;	/* 'A' */
							crd_data->kkr_park[2] = 0x4B;	/* 'K' */
						}
						crd_data->kkr_role = (uchar)(prm[CIK_ROLE] + 0x30);
						crd_data->kkr_lev = (uchar)(prm[CIK_LEVEL] + 0x30);
						crd_data->kkr_type = type_tbl[prm[CIK_TYPE]];
						crd_data->kkr_kno[0] = 0x30;
						crd_data->kkr_kno[1] = 0x30;
						crd_data->kkr_kno[2] = (uchar)(prm[CIK_KNO]/10 + 0x30);
						crd_data->kkr_kno[3] = (uchar)(prm[CIK_KNO]%10 + 0x30);
						memset(&crd_data->kkr_jdg, 0x30, sizeof(crd_data->kkr_jdg));
						memset(crd_data->kkr_rsv1, 0x20, sizeof(crd_data->kkr_rsv1));
						memset(crd_data->kkr_rsv2, 0x20, sizeof(crd_data->kkr_rsv2));
						memset(crd_data->kkr_rsv3, 0x20, sizeof(crd_data->kkr_rsv3));
						if( prm_get( COM_PRM,S_SYS,12,1,6 ) == 1){//GT�t�H�[�}�b�g
							crd_data->kkr_rsv2[4] = (uchar)(CPrmSS[S_SYS][1]/100000 + 0x30);
							crd_data->kkr_rsv2[5] = (uchar)(CPrmSS[S_SYS][1]%100000/10000 + 0x30);
							crd_data->kkr_pno[0] = (uchar)(CPrmSS[S_SYS][1]%10000/1000 + 0x30);
							crd_data->kkr_pno[1] = (uchar)(CPrmSS[S_SYS][1]%1000/100 + 0x30);
							crd_data->kkr_pno[2] = (uchar)(CPrmSS[S_SYS][1]%100/10 + 0x30);
							crd_data->kkr_pno[3] = (uchar)(CPrmSS[S_SYS][1]%10 + 0x30);
							MDP_buf[127] = 1;
						}else{//APS�t�H�[�}�b�g
							crd_data->kkr_pno[0] = (uchar)(CPrmSS[S_SYS][1]/1000 + 0x30);
							crd_data->kkr_pno[1] = (uchar)(CPrmSS[S_SYS][1]%1000/100 + 0x30);
							crd_data->kkr_pno[2] = (uchar)(CPrmSS[S_SYS][1]%100/10 + 0x30);
							crd_data->kkr_pno[3] = (uchar)(CPrmSS[S_SYS][1]%10 + 0x30);
							MDP_buf[127] = 0;
						}
						MDP_mag = sizeof( m_kakari );
						MDP_siz = sizeof(m_kakari);
						md_pari2((uchar *)MDP_buf, (ushort)MDP_siz, 1);								/* �p���e�B�쐬(�������è) */
						/* �W���J�[�h�������� */
						opr_snd(202);
						write_mode = 2;
					}
					/* �J�[�h�������ݏI�� */
					else {
						/* �����m�f */
						if (RED_REC.ercd) {
							// TODO:�����ɏ����݂m�f�̃G���[�\������������
							grachr( 6,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, MAG_CHK_STR[9] );	// "���̃J�[�h�͏����܂���"
							Lagtim( OPETCBNO, 13, (50*6));
							ope_anm( AVM_CARD_ERR1 );
							
							faze = 0;
							disp = 1;
							OPE_red = 0;
						}
						/* �����n�j */
						else {
							write_mode = 0;
							faze = 2;
							disp = 2;
							opr_snd( 3 );
							Chk_result.mag[0] = 0;		// �����݃e�X�gOK
						}
					}
					break;
				case 2:									// ���[�h�e�X�g
					if( RED_REC.ercd == 0x21){			// �ǎ�s�ŏ����e�X�g����I��
						grachr( 6,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, MAG_CHK_STR[8] );	// "���̃J�[�h�͓ǂ߂܂���"
						Lagtim( OPETCBNO, 13, (50*6));
						ope_anm( AVM_CARD_ERR2 );
						opr_snd( 3 );
					}
					break;
				case 3:									// �󎚃e�X�g�I��
					if( RED_REC.ercd == 0){
						faze = 4;
						disp = 4;
						opr_snd( 3 );
						Chk_result.mag[2] = 0;			// �󎚃e�X�gOK
					}
					else{
						grachr( 6,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, MAG_CHK_STR[7] );	// "���̃J�[�h�͎g���܂���"
						Lagtim( OPETCBNO, 13, (50*6));
						ope_anm( AVM_CARD_ERR1 );
					}
					break;
				case 4:									// �����e�X�g�I��
					if( RED_REC.ercd == 0x21){
						grachr( 6,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, MAG_CHK_STR[7] );	// "���̃J�[�h�͎g���܂���"
						Lagtim( OPETCBNO, 13, (50*6));
						ope_anm( AVM_CARD_ERR1 );
						opr_snd( 3 );
					}
					break;
				case 5:
					if( RED_REC.ercd == 0x21){			// �ǎ�s�ŏ����e�X�g����I��
						read_sht_cls();						// ���Cذ�ް������u�v
						OPE_red = 2;
						Chk_end = 1;
						Chk_result.mag[3] = 0;			// �����e�X�gOK
						break;
					}
					break;
				default:
					opr_snd( 3 );
					break;
				}
				break;
			
			case ARC_CR_EOT_EVT:									// ���������
				switch(faze){
				case 1:
					// ���C�g�e�X�g
					break;
				case 2:
					if(atend_no != 0){								// �W���J�[�h�������
						if(prn){
							if( opr_snd( 93 ) == 0 ){				// ��ýėv��
								faze = 3;
								disp = 3;
								Chk_result.mag[1] = 0;				// ���[�h�e�X�gOK
							}
						}
						else{
							displclr(6);
							/* �W���J�[�h�ȊO�̌�������� */
						}
					}
					else{					// �W���J�[�h�ȊO�̌�
						opr_snd( 3 );		// ���[�h�v��
					}
					break;
				case 4:				// �p����������
					faze = 5;		// �p���}���҂�
					opr_snd( 3 );
					break;
				default:
					faze = 0;
					break;
				}
				break;
				
			case KEY_TEN_F1:						// �߂�
				BUZPI();
				read_sht_cls();						// ���Cذ�ް������u�v
				OPE_red = 6;
				opr_snd( 90 );						// ذ�޺���ނ𖳌��Ƃ���ׁAýĺ���ނ𑗐M
				Chk_end = 1;
				return -1;
			case KEY_TEN_F5:						// ����
				BUZPI();
				read_sht_cls();						// ���Cذ�ް������u�v
				OPE_red = 6;
				opr_snd( 90 );						// ذ�޺���ނ𖳌��Ƃ���ׁAýĺ���ނ𑗐M
				Chk_end = 1;
				break;
			case TIMEOUT8:
				read_sht_cls();						// ���Cذ�ް������u�v
				OPE_red = 6;
				opr_snd( 90 );						// ذ�޺���ނ𖳌��Ƃ���ׁAýĺ���ނ𑗐M
				Disp_Chk_NG(MAG_CHK_NGSTR, 4, &Chk_result.mag[0]);
				OPE_red = 0;
				faze = 0;
				disp = 1;
				atend_no = 0;
				displclr(4);
				break;
			default:
				break;
		}
		NG_key_chek();
	}
	return 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| �������[�_�[�`�F�b�N                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short Note_Chk( void )
{
	short			msg = -1;			// ��Mү����
	unsigned short	Chk_end = 0;			// �`�F�b�N�I���t���O
	unsigned short	rd_cnt	= 0;		// �����Ǎ�����
	unsigned char	disp_reset = 1;
	unsigned char	rd_sts	= 0;		// ����ذ�ް���
										//  0:��
										//  1:�����ۗ���
										//  2:�����߂������҂�
										//  3:�����捞�����҂�
	unsigned short	First_flag = 0;		// �S���ڂ̓��������ꂽ���ǂ���
	
	cn_stat( 1, 1 );						// ��������
	memset( Chk_result.note, 0, 3);
	for( ; Chk_end == 0; ){
		if(disp_reset){
			dispclr();
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NOTE_CHK_STR[0] );	//
			grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NOTE_CHK_STR[1] );		// "�P�O�O�O�~�D"
			opedsp( 3, 14,  rd_cnt, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );					// "�O"
			opedsp( 3, 22,  4, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );					// "�S"
			Fun_Dsp( FUNMSG[109] );					// "�@�@�@�@�@�@ �߂�  �捞  �I�� "
			disp_reset = 0;
		}
		msg = GetMessage();				// ү���ގ�M
		switch(msg){
			case KEY_TEN_F1:			// �߂�
				BUZPI();
				return -1;
			case KEY_TEN_F5:			// ����
				BUZPI();
				cn_stat( 2, 1 );	// �����s��
				Chk_end = 1;
				break;
			case TIMEOUT8:
				if(rd_sts < 2){
					cn_stat( 2, 1 );	// �����s��
					Disp_Chk_NG(NOTE_CHK_NGSTR, 3, &Chk_result.note[0]);
					cn_stat( 1, 1 );						// ��������
					disp_reset = 1;
				}
				break;
			case NOTE_EVT:					// �������[�_�[�C�x���g
				switch( OPECTL.NT_QSIG ){
					case	1:				// ����ذ�ް����u��������v��M
						rd_cnt++;																// �Ǎ������X�V�i�{�P�j
						opedsp( 3, 14, rd_cnt, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );		// �Ǎ������\��
						grachr( 6,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, NOTE_CHK_STR[2] );		// "�@������ۗ����ł��@�@�@�@�@�@"
						rd_sts = 1;								// ��ԁF�����ۗ���
						if(rd_cnt >= 4 ){				// �S���ڈȍ~?
							if( !First_flag ){			// ����Ȃ�߂�
								cn_stat( 2, 1 ); 		// �����s��
								rd_sts = 2;				// ��ԁF�����߂������҂�
								First_flag++;
							}else{						// �đ}���Ȃ��荞��
								First_flag++;
								nt_com = 0x80 + 4;		/* ���ێ�����荞�� */
							}
						}
						break;
					case	2:				// ����ذ�ް����u���o�������v��M
						if( rd_sts == 2 ){		// �����߂������҂��H
							rd_cnt--;															// �Ǎ������X�V�i�|�P�j
							opedsp( 3, 14, rd_cnt, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );	// �Ǎ������\��
							displclr( 6 );														// "�@������ۗ����ł��@�@�@�@�@�@"�\���I�t
							rd_sts = 0;															// ��ԁF��
							cn_stat( 1, 1);														// ��������
						}
						break;
					case	3:				// ����ذ�ް����u���������v��M
						if( rd_sts == 3 ){		// �����捞�����҂��H
							displclr( 6 );						// "�@������ۗ����ł��@�@�@�@�@�@"�\���I�t
							rd_sts = 0;							// ��ԁF��
						}else if( First_flag == 2 ){
							cn_stat( 2, 1 );	// �����s��
							Chk_end = 1;
						}
						break;
					case 	5:									// �㑱��������
						cn_stat( 1, 1 );						// ��������
						rd_sts = 3;								// ��ԁF�����捞�����҂�
						break;
					case	7:				// ����ذ�ް����u���o����ԁv��M
						if( rd_sts == 2 ){		// �����߂������҂��H
							cn_stat( 10, 1);
						}
						break;
					default:				// ���̑�
						break;
				}
				break;
			default:
				break;
		}
		NG_key_chek();
	}
	return 0;
}
/*[]----------------------------------------------------------------------[]*/
/*| �R�C�����b�N�`�F�b�N                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define COINSYU_CNT 4
short Coin_Chk( void )
{
	unsigned short	Chk_end = 0;			// �`�F�b�N�I���t���O
	short	msg = -1;
	int		i;
	int		subtube = 0;
	ushort	current_cnt[10];		// �����f�[�^(BCD)
	uchar	wk1,wk2,cnt_total = 0;
	unsigned char	disp_reset = 1;
	long ref_val = 0;
	long total_val = 0;
	
	memset(&ref_coinsyu, 0, sizeof(ref_coinsyu));
	if( CN_SUB_SET[1] != 0 ){	// 100�~SUB����
		subtube = 0x02;
	} else {					// 10�~SUB����
		subtube = 0x01;
	}
	memset(current_cnt, 0, sizeof(current_cnt));
	current_cnt[4] = 0x05;			// �\�����ɂ͗\��5���̍d�݂���[����Ă���Ƃ݂Ȃ�
	if( subtube & 0x01 ){
		current_cnt[5] = 0x05;		// 10�~�K��l
	}
	else{
		current_cnt[5] = 0x05;
	}

	current_cnt[6] = 0x05;			// 50�~�K��l

	if( subtube & 0x02 ){
		current_cnt[7] = 0x05;		// 10�~�K��l
	}
	else{
		current_cnt[7] = 0x05;		// 100�~�K��l
	}
	current_cnt[8] = 0x05;			// 500�~�K��l
	current_cnt[9] = 0x05;			// �\��
	cn_stat( 33, 0 );				// ��������(CREM:ON)

	memset( Chk_result.coin, 0, 3);
	for( ; Chk_end == 0; ){
		if(disp_reset){
			dispclr();
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, COIN_CHK_STR[0] );	// "���R�C�����b�N�m�F��"
			for( i = 0; i < 4 ; i++ ){
				grachr( i+1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, COIN_CHK_STR[i+1] );	// 
				opedsp( (unsigned short)(i+1), 14, (unsigned short)bcdbin(current_cnt[i]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
				opedsp( (unsigned short)(i+1), 22, (unsigned short)bcdbin(current_cnt[5+i]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
			}

			// �\�~����\��
			if( subtube ){
				if( subtube & 0x01 ){
					grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, COIN_CHK_STR[5] );	//
				}else{
					grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, COIN_CHK_STR[6] );	//
				}
				opedsp( 5, 14, (unsigned short)bcdbin(current_cnt[4]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
				opedsp( 5, 22, (unsigned short)bcdbin(current_cnt[9]), 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );

			}
			Fun_Dsp( FUNMSG[109] );
			disp_reset = 0;
		}
		msg = GetMessage();				// ү���ގ�M
		switch(msg){
			case KEY_TEN_F1:			// �߂�
				BUZPI();
				return -1;
			case KEY_TEN_F5:			// ����
				BUZPI();
				cn_stat( 2, 0 );
				Chk_end = 1;
				break;
			case TIMEOUT8:				// F4�L�[������
					cn_stat( 2, 0 );
					Disp_Chk_NG(COIN_CHK_NGSTR, 3, &Chk_result.coin[0]);
					cn_stat( 33, 0 );
					disp_reset = 1;
				break;
			case COIN_EVT:					// �R�C�����b�N�C�x���g
				switch( OPECTL.CN_QSIG ){
					case	1:				// ���ү�����u��������v��M
					case	5:				// ���ү�����u��������imax�B���j�v��M
						for( i = 0; i < COINSYU_CNT ; i++ ){		// ��ݓ��������\��
							cnt_total = bcdadd( current_cnt[i], CN_RDAT.r_dat08[i] );
							opedsp( (unsigned short)(i + 1), 14, (unsigned short)bcdbin(cnt_total), 2, 0, 0,
																			COLOR_BLACK, LCD_BLINK_OFF );
							if(cnt_total >= current_cnt[5+i]){
								ref_coinsyu[i] = bcdbin(current_cnt[5+i]);					// ���했�̕����o�������ɃZ�b�g
								if( (i==0 && (subtube & 0x01)) || (i==2 && (subtube & 0x02)) ){
									ref_coinsyu[i] += 5;			/* �T�u���\���������Z */
								}
							}
						}
							for( i = 0; i < COINSYU_CNT ; i++ ){	// ��ݓ��������ۑ�ܰ��Ɏ�M�ް��i�����j�����Z
								current_cnt[i] = bcdadd( current_cnt[i], CN_RDAT.r_dat08[i] );
							}
							cn_stat( 5, 0 );						// ��������(CREM:ON,������ر)
						ref_val = 0;
						for(i=0;i<COINSYU_CNT;i++){
							if(ref_coinsyu[i] == 0){
								break;
							}
							ref_val += (long)(ref_coinsyu[i]*coin_vl[i]);
						}
						if(i>=COINSYU_CNT){
							cn_stat( 2, 0 );													// �����s��
						}
						break;
					case	2:				// ���ү�����u���o�������v��M
						ref_val = 0;															// ���o���z�ر
						for( i = 0; i < COINSYU_CNT ; i++ ){		// ��ݓ��������\��
							cnt_total = bcdbin(CN_RDAT.r_dat09[i]) * coin_vl[i];
							wk1 = bcdbin( CN_RDAT.r_dat0a[i]);
							wk2 = bcdbin( cnt_total );
							if(wk1>wk2){
								wk2 = 0;
							}
							else{
								wk2 -= wk1;
							}
							cnt_total = binbcd(wk2);
							opedsp( (unsigned short)(i + 1), 14, (unsigned short)bcdbin(cnt_total), 2, 0, 0,
																			COLOR_BLACK, LCD_BLINK_OFF );
							current_cnt[i] = cnt_total;
						}
						opedsp( 5, 14, 0, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
						cn_stat( 33, 0 );					// ��������(CREM:ON)
						break;

					case	7:				// ���ү�����u���o����ԁv��M
						if(ref_val){
							total_val = 0;
							for(i=0;i<COINSYU_CNT;i++){
								total_val += bcdbin(CN_RDAT.r_dat09[i]) * coin_vl[i];
							}
							if( subtube & 0x01 )
							{	// 10�~SUB����
								total_val += current_cnt[4] * coin_vl[0];
							}
							if( subtube & 0x02 )
							{	// 100�~SUB����
								total_val += current_cnt[4] * coin_vl[2];
							}
							refund(total_val);				// ���o�J�n(�S�Ă̍d�݂𕥂��o��)
						}
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}
		NG_key_chek();
	}
	return 0;
}
/*[]----------------------------------------------------------------------[]*/
/*| �v�����^�`�F�b�N(���V�[�g�A�W���[�i������)                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define		TSTPRI_END	(PREQ_PRINT_TEST | INNJI_ENDMASK)
const	unsigned char	PriSts[4] =	// �������������׸�
{
	0x01,	// b0:�߰�߰Ʊ����
	0x02,	// b1:����ݵ����
	0x04,	// b2:�߰�߰����
	0x08	// b3:ͯ�މ��x�ُ�
};
short RJ_Print_Chk( void )
{
	short			msg = -1;			// ��Mү����
	T_FrmPrintTest	FrmPrintTest;		// �󎚗v��ү���ލ쐬�ر
	unsigned short	Chk_end = 0;			// �`�F�b�N�I���t���O
	unsigned char	priend, sts, pState, sts_bk, com, com_bk;
	unsigned char	line, i;
	
	
	dispclr();
	if(Chk_info.Chk_mod == R_PRINTER){
		FrmPrintTest.prn_kind = R_PRI;				// �������ʁFڼ��
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRI_CHK_STR[0] );	//���V�[�g
		memset( Chk_result.r_print, 0, 6);
	}
	else if(Chk_info.Chk_mod == J_PRINTER){
		FrmPrintTest.prn_kind = J_PRI;				// �������ʁFڼ��
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRI_CHK_STR[1] );	//�W���[�i��
		memset( Chk_result.j_print, 0, 5);
	}
	
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRI_CHK_STR[10] );	//
	Fun_Dsp( FUNMSG[0] );
	
	queset( PRNTCBNO, PREQ_PRINT_TEST, sizeof( T_FrmPrintTest ), &FrmPrintTest ); 			// �e�X�g�󎚗v��

	Lagtim( OPETCBNO, 6, 3*50 );					// �󎚒���ʕ\����ϰ6(3s)�N���iýĈ󎚒��̉�ʕ\�����R�b�ԕێ�����j
	priend		= OFF;								// �󎚏I���׸�ؾ��
	
	for( ; priend == OFF; ){

		msg = GetMessage();				// ү���ގ�M

		switch( msg){			// ��Mү���ށH
			case TSTPRI_END:	// �e�X�g�󎚏I��

				priend = ON;						// NO�@���󎚏I���׸޾��

				break;

			case TIMEOUT6:		// �󎚒���ʏI����ϰ��ѱ��

				priend = ON;							// �󎚒���ʕ\����ϰ��ѱ��

				break;
		}
	}
	Lagcan( OPETCBNO, 6 );				
	
	sts = 0;
	pState = 0;
	sts_bk = 0xff;
	com = OFF;
	com_bk = 0xff;
	
	Fun_Dsp( FUNMSG[109] );
	for( ; Chk_end == 0; ){
		if( Chk_info.Chk_mod == R_PRINTER ){
			//	���V�[�g�v�����^
			if(ERR_CHK[ERRMDL_PRINTER][01] != 0){
				com = ON;										// �ʐM�s��
			}
			pState = (uchar)(rct_prn_buff.PrnStateMnt & 0x0f);	// ���ذ��
		}
		else{
			//	�W���[�i���v�����^
			if(ERR_CHK[ERRMDL_PRINTER][20] != 0){
				com = ON;										// �ʐM�s��
			}
			pState = (uchar)(jnl_prn_buff.PrnStateMnt & 0x0f);	// ���ذ��
		}
		sts |= pState;											// ��x���������G���[�͉������Ȃ�
		if( com == OFF ){

			// �ʐM��ԁ�����
			if(sts != sts_bk || com != com_bk){					// �X�e�[�^�X�ω�����
				sts_bk = sts;
				com_bk = com;
				dispmlclr(3, 6);
				line	= 2;
				for( i = 0 ; i < 3 ; i++ ){						// ��Ԃ��������������Ă���װ����S�ĕ\������
					grachr( line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRI_CHK_STR[i+5] );	// �װ���\��
					if( sts & PriSts[i] ){
						grachr( line, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, PRI_CHK_STR[i+5] );	// �װ���\��
					}
					line++;
				}
			}
		}
		else{
			if(com_bk == OFF){
				// �ʐM��ԁ��ُ�
				dispmlclr(3, 6);
				grachr( 3,  0, 30, 0, COLOR_RED, LCD_BLINK_OFF, PRI_CHK_STR[9] );					// "�@�ʐM�s�ǁ@�@�@�@�@�@�@�@�@�@"
				com_bk =com;
			}
		}
		msg = GetMessage();				// ү���ގ�M
		
		switch(msg){
			case KEY_TEN_F1:			// �߂�
				BUZPI();
				return -1;
			case KEY_TEN_F5:			// ����
				BUZPI();
				Chk_end = 1;
				break;
			case TIMEOUT8:
				if(Chk_info.Chk_mod == R_PRINTER){
					Disp_Chk_NG(PRI_CHK_NGSTR, 6, &Chk_result.r_print[0]);
					grachr( 0,  0, 30, 0, COLOR_RED, LCD_BLINK_OFF, PRI_CHK_STR[0] );	//���V�[�g
				}
				else{
					Disp_Chk_NG(PRI_CHK_NGSTR, 5, &Chk_result.j_print[0]);
					grachr( 0,  0, 30, 0, COLOR_RED, LCD_BLINK_OFF, PRI_CHK_STR[1] );	//�W���[�i��
				}
				displclr(1);
				Fun_Dsp( FUNMSG[109] );
				sts_bk = 0xff;
				break;
			default:
				break;
		}
		NG_key_chek();
	}
	return 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| �V���b�^�[/LED�`�F�b�N                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short Led_Shut_Chk( void )
{
	short			msg = -1;			// ��Mү����
	unsigned short	Chk_end = 0;			// �`�F�b�N�I���t���O
	unsigned char	shutflg,startflg;
	startflg = 1;
	memset( Chk_result.led_shut, 0, 2);
	for( ; Chk_end == 0; ){
		if( startflg ){													// ��ʕ\��
			dispclr();
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SHUT_CHK_STR[0] );	//
			grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SHUT_CHK_STR[1] );	//
			grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SHUT_CHK_STR[2] );	//
			grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SHUT_CHK_STR[3] );	//
			Fun_Dsp(FUNMSG[109]);
			read_sht_opn();								// �V���b�^�[�J
			Lagtim(OPETCBNO, 6, 50*2);					// 2�b
			shutflg = 1;
			startflg = 0;
		}
		msg = GetMessage();				// ү���ގ�M
		switch(msg){
			case KEY_TEN_F1:					// �߂�
				BUZPI();
				return -1;
			case KEY_TEN_F5:					// ����
				BUZPI();
				Lagcan(OPETCBNO, 6);					// �^�C�}�[�L�����Z��
				read_sht_cls();
				Chk_end = 1;
				break;
			case TIMEOUT6:
				if(shutflg){
					read_sht_cls();
					shutflg = 0;
				}
				else{
					read_sht_opn();	
					shutflg = 1;
				}
				Lagtim(OPETCBNO, 6, 50*2);					// 2�b
				break;
			case TIMEOUT8:
				Lagcan(OPETCBNO, 6);					// �^�C�}�[�L�����Z��
				read_sht_cls();
				Disp_Chk_NG(SHUT_CHK_NGSTR, 2, &Chk_result.led_shut[0]);
				startflg = 1;
				break;
			default:
				break;
		}
		NG_key_chek();
	}
	return 0;
}
/*[]----------------------------------------------------------------------[]*/
/*| �A�i�E���X�`�F�b�N                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define BIG_VOL		16
#define MID_VOL		8
#define SMALL_VOL	1
short Ann_Chk( void )
{
	short			msg = -1;			// ��Mү����
	unsigned short	Chk_end = 0;			// �`�F�b�N�I���t���O
	unsigned char	disp_reset, ann;
	unsigned char	Ann_prm[3] =  { BIG_VOL, MID_VOL, SMALL_VOL };
	disp_reset = 1;
	key_num = 1;
	announceFee = 100;
	memset( Chk_result.ann, 0, 2);
	for( ; Chk_end == 0; ){
		if(disp_reset){
			dispclr();
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ANN_CHK_STR[0] );	// "�ē������m�F�@�@�@�@�@�@�@�@",
			grachr( 2,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, ANN_CHK_STR[1] );	// "��u�Ԏ��ԍ�����͂��ĉ������v"
			grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ANN_CHK_STR[2] );	// "���u�����͂P�O�O�~�ł��v�@�@�@"
			grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ANN_CHK_STR[3] );	// "���u���̃J�[�h�͎g���܂���v�@"
			Fun_Dsp(FUNMSG[109]);
			disp_reset = 0;
			ann = 0;
			pre_volume[0] = Ann_prm[0];
			avm_test_no[0] = 132;		// �Ԏ��ԍ�����͂��ĉ�����
			avm_test_cnt = 1;
			
			ope_anm(AVM_AN_TEST);		// �A�i�E���X
		}
		msg = GetMessage();				// ү���ގ�M
		switch(msg){
			case KEY_TEN_F1:			// �߂�
				BUZPI();
				return -1;
			case KEY_TEN_F5:			// ����
				BUZPI();
				ope_anm( AVM_STOP );
				Chk_end = 1;
				break;
			case SODIAC_PLAY_CMP:
				grachr( ann+2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ANN_CHK_STR[ann+1] );	//
				ann++;
				if(ann >= 3){
					ann = 0;
				}
SOUND_REPLAY:
				if(ann < 3){
					pre_volume[0] = Ann_prm[ann];
				}
				switch(ann){
				case 0:
					avm_test_no[0] = 132;				// �Ԏ��ԍ�����͂��ĉ�����
					avm_test_cnt = 1;
					ope_anm(AVM_AN_TEST);				// �A�i�E���X
					break;
				case 1:
					ope_anm(AVM_RYOUKIN);				// �����ǂݏグ;
					break;
				case 2:
					avm_test_no[0] = 163;				// �Ԏ��ԍ�����͂��ĉ�����
					avm_test_cnt = 1;
					ope_anm(AVM_AN_TEST);				// �A�i�E���X
					break;
				case 3:
					Lagtim( OPETCBNO, 13, (50*3));
				default:
					break;
				}
				if(ann < 3){
					grachr( ann+2,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, ANN_CHK_STR[ann+1] );	// �Y���s�𔽓]�\��
				}
				break;
			case TIMEOUT13:
				ann = 0;
				goto	SOUND_REPLAY;
				break;
			case TIMEOUT8:
				ope_anm( AVM_STOP );
				Disp_Chk_NG(ANN_CHK_NGSTR, 2, &Chk_result.ann[0]);
				ann = 0;
				disp_reset = 1;
				break;
			default:
				break;
		}
		NG_key_chek();						// F4�L�[�̊Ď�
	}
	return 0;
}
/*[]----------------------------------------------------------------------[]*/
/*| ���o�̓`�F�b�N                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short Inout_Chk( void )
{

	short			msg = -1;			// ��Mү����
	unsigned short	Chk_end = 0;			// �`�F�b�N�I���t���O
	unsigned char mod,i,pos,sts[5];
	struct {
		char sw;				// Chkmod_Sw_rd()�Ɏw�肷�����
		unsigned short lin;		// �\���s
		unsigned short col;		// �\���ʒu
		unsigned short ann;		// �A�i�E���X�ԍ�
	} sw_disp[5] = {
		{1, 1, 0, 223 },		// �R�C�����b�N��
		{2, 2, 0, 225 },		// �R�C�����ɒ��E
		{5, 3, 0, 229 },		// �h�A
		{4, 4, 0, 230 },		// �h�A�m�u
		{3, 5, 0, 224 },		// �������[�_�[��
	};
	mod = 0;
	dispclr();
	grachr( 0,  0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[0] );	//
	for(i=0;i<5;i++){
		sts[i] = Chkmod_Sw_rd(sw_disp[i].sw);
		if(sts[i]){
			grachr( sw_disp[i].lin,  sw_disp[i].col, 16, 1, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[i+1] );
		}
		else{
			grachr( sw_disp[i].lin,  sw_disp[i].col, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[i+1] );
		}
	}
	memset( Chk_result.sw, 0, 5);
	Fun_Dsp(FUNMSG[109]);
	Lagcan( OPETCBNO, 8 );
	for( ; Chk_end == 0; ){
		msg = GetMessage();				// ү���ގ�M
		if(mod == 0){
			for(i=0; i<5; i++){
				if(sts[i] != Chkmod_Sw_rd(sw_disp[i].sw)){
					sts[i] = Chkmod_Sw_rd(sw_disp[i].sw);
					if(sts[i]){
						grachr( sw_disp[i].lin,  sw_disp[i].col, 16, 1, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[i+1] );
						avm_test_ch = 0;
						avm_test_cnt = 1;
						avm_test_no[0] = sw_disp[i].ann;
						ope_anm(AVM_AN_TEST);
					}
				}
			}
			switch(msg){
				case KEY_TEN_F1:			// �߂�
					BUZPI();
					return -1;
				case KEY_TEN_F5:			// ����
					BUZPI();
					Chk_end = 1;
					break;
				case TIMEOUT8:
					mod = 1;
					pos = 0;
					grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CHK_CHR[4] );	//
					grachr( sw_disp[0].lin,  sw_disp[0].col, 16, 1, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[1] );
					for(i=1;i<5;i++){
						if(Chk_result.sw[i] == 0){
							grachr( sw_disp[i].lin,  sw_disp[i].col, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[i+1] );
						}
						else{
							grachr( sw_disp[i].lin,  sw_disp[i].col, 16, 0, COLOR_MEDIUMBLUE, LCD_BLINK_OFF, INOUT_CHK_STR[i+1] );
						}
					}
					Fun_Dsp(FUNMSG[110]);
					break;
				default:
					break;
			}
			NG_key_chek();
		}
		else if(mod == 1){					// NG���ړo�^
			switch(msg){
				case KEY_TEN_F1:
					BUZPI();
					if(Chk_result.sw[pos] == 0){
						grachr( sw_disp[pos].lin,  sw_disp[pos].col, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[pos+1] );
					}
					else{
						grachr( sw_disp[pos].lin,  sw_disp[pos].col, 16, 0, COLOR_MEDIUMBLUE, LCD_BLINK_OFF, INOUT_CHK_STR[pos+1] );
					}
					if(pos > 0){
						pos-=1;
					} else {
						pos = 4;
					}
					grachr( sw_disp[pos].lin,  sw_disp[pos].col, 16, 1, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[pos+1] );
					break;
				case KEY_TEN_F2:
					BUZPI();
					if(Chk_result.sw[pos] == 0){
						grachr( sw_disp[pos].lin,  sw_disp[pos].col, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[pos+1] );
					}
					else{
						grachr( sw_disp[pos].lin,  sw_disp[pos].col, 16, 0, COLOR_MEDIUMBLUE, LCD_BLINK_OFF, INOUT_CHK_STR[pos+1] );
					}
					if(pos < 4){
						pos++;
					}
					else{
						pos = 0;
					}
					grachr( sw_disp[pos].lin,  sw_disp[pos].col, 16, 1, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[pos+1] );
					break;
				case KEY_TEN_F4:
					BUZPI();
					mod = 0;
					grachr( 0,  0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[0] );	//
					for(i=0;i<5;i++){
						sts[i] = Chkmod_Sw_rd(sw_disp[i].sw);
						if(sts[i]){
							grachr( sw_disp[i].lin,  sw_disp[i].col, 16, 1, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[i+1] );
						}
						else{
							grachr( sw_disp[i].lin,  sw_disp[i].col, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, INOUT_CHK_STR[i+1] );
						}
					}
					Fun_Dsp(FUNMSG[109]);
					break;
				case KEY_TEN_F5:
					BUZPI();
					if(Chk_result.sw[pos] == 0){
						Chk_result.sw[pos] = 1;
					}
					else{
						Chk_result.sw[pos] = 0;
					}
					break;
				default:
					break;
			}
		}
	}
	return 0;
}
// �X�C�b�`���[�h(funcchk.c��藬�p�A�ꕔ����)
unsigned char	Chkmod_Sw_rd(  char idx  )
{
	unsigned char ret;

	switch( idx ){		// �X�C�b�`��ʁH

		case	0:		// �ݒ�r�v
			ret = ( CP_MODECHG ) ? 0 : 1;
			break;

		case	1:		// �R�C�����b�N�Z�b�g�r�v
			ret = ( FNT_CN_DRSW ) ? 1 : 0;
			break;

		case	2:		// �R�C�����ɂr�v
			ret = ( FNT_CN_BOX_SW ) ? 1 : 0;
			break;

		case	3:		// �������ɂr�v
			ret = ( FNT_NT_BOX_SW ) ? 1 : 0;
			break;

		case	4:		// �h�A�m�u
			ret = ( OPE_SIG_DOORNOBU_Is_OPEN ) ? 0 : 1;
			break;

		case	5:		// �h�A�J
			ret = ( OPE_SIG_DOOR_Is_OPEN ) ? 0 : 1;
			break;

		case	6:		// �U���Z���T�[
			ret = FNT_VIB_SNS;
			break;

		case	7:		// �������[�_�[�E���Z���T�[
			ret = ( FNT_NT_FALL_SW ) ? 0: 1;
			break;

		case	15:		// �l�̌��m
			ret = ( FNT_MAN_DET ) ? 1: 0;
			break;

		default:		// ���̑�
			ret = 0;
			break;

	}
	return(ret);
}


/*[]----------------------------------------------------------------------[]*/
/*| �M���`�F�b�N                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short Sig_Chk( void )
{
	short			msg = -1;			// ��Mү����
	unsigned short	sig_sts,Chk_end = 0;			// �`�F�b�N�I���t���O
	unsigned char	onoff, sig_no,i,ng_mod = 0;
	unsigned char	res[8];
	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SIG_CHK_STR[0] );	//
	for(i = 0; i < 7; i++){
		grachr( 3,  (7+(i*2)), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_OKNG[3] );	//
	}
	Fun_Dsp(FUNMSG[0]);
	memset( Chk_result.sig, 0, 1);
	sig_no = 8;
	onoff = 1;
	ExIOSignalwt( sig_no , onoff);	
	Lagtim( OPETCBNO, 6, 10 );								// ý���ϰ6(200ms)�N��
	Lagcan( OPETCBNO, 8 );
	for( ; Chk_end == 0; ){
		msg = GetMessage();				// ү���ގ�M
		switch(msg){
			case KEY_TEN_F1:			// �߂�
				BUZPI();
				return -1;
			case KEY_TEN_F4:			// NG
				if(ng_mod){
					dispclr();
					grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SIG_CHK_STR[0] );	//
					for(i = 0; i < 7; i++){
						grachr( 3,  (7+(i*2)), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_OKNG[res[i]] );	//
					}
					Fun_Dsp(FUNMSG[0]);
					ng_mod = 0;
				}
				break;
			case KEY_TEN_F5:			// ����
				if(sig_no >= 15){
					if(ng_mod){
						Chk_result.sig[0] = 1;		// �M���`�F�b�NNG
					}
					BUZPI();
					for(i = 0; i < 7; i++){
						ExIOSignalwt( i+7 , 0);
					}
					Chk_end = 1;
				}
				break;
			case TIMEOUT6:									// ý���ϰ��ѱ��
				sig_sts = ExIOSignalrd(INSIG_RXIIN);
				if(onoff){
					if(sig_sts != onoff){
						grachr( 3,  (7+((sig_no-8)*2)), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_OKNG[1] );	// "�~"
						res[sig_no-8] = 1;		// NG
						sig_no++;
					}
					else{
						onoff = 0;
					}
				}
				else{
					if(sig_sts != onoff){
						grachr( 3,  (7+((sig_no-8)*2)), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_OKNG[1] );	// "�~"
						res[sig_no-8] = 1;		// NG
					}
					else{
						grachr( 3,  (7+((sig_no-8)*2)), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_OKNG[0] );	// "��"
						res[sig_no-8] = 0;		// OK
					}
					sig_no++;
					onoff = 1;
				}
				if(sig_no < 15){
					ExIOSignalwt( sig_no , onoff);	
					Lagtim( OPETCBNO, 6, 10 );	
				}
				else{
					for(i=0; i<7; i++){
						if(res[i]){
							grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SIG_CHK_STR[2] );	// "�ُ�I��"
							break;
						}
					}
					if(i>=8){
						grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SIG_CHK_STR[1] );	// "����I��"
					}
					Fun_Dsp(FUNMSG[109]);
				}
				break;
			case TIMEOUT8:
				if(sig_no >= 15){
					ng_mod = 1;
					dispclr();
					grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CHK_CHR[4] );	//
					grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CHK_NGSTR[0] );	//
					Fun_Dsp( FUNMSG[110] );
					Chk_result.sig[0] = 1;
				}
				break;
			default:
				break;
		}
		NG_key_chek();
	}
	return 0;
}
/*[]----------------------------------------------------------------------[]*/
/*| NG�L�[(F4)���������菈��                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void NG_key_chek( void )
{
	if( (OPECTL.Comd_knd == 0x01) && (OPECTL.Comd_cod == (0x00FF & KEY_TEN_F4)) ){
		if( OPECTL.on_off == 0 ){
			Lagcan( OPETCBNO, 8 );
		}
		else{
			Lagtim( OPETCBNO, 8, 3*50 );
		}
	}
	
}

/*[]----------------------------------------------------------------------[]*/
/*| �������`�F�b�N(funccheck.c���痬�p)                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned char                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
const	unsigned long	memory_adr[3] =
	{
		// �q�`�l�P�͕����I�ɂ͈�ł��邪�A�h���X�f�R�[�h��2�̋�Ԃɕ�������
		0x7000000,		// �q�`�l�P�|�P�FCS1:�A�h���X���0x07000000-0x070FFFFF(1MB)
		0x7400000,		// �q�`�l�P�|�Q�FCS1:�A�h���X���0x07400000-0x074FFFFF(1MB)
		0x6000000,		// �q�`�l�Q    �FCS2:�A�h���X���0x06000000-0x060FFFFF(1MB)
	};
#define		FCK_INSRAM_ADR	0x00000000	// �����r�q�`�l�J�n�A�h���X�F0x00000000�`0x00020000�i0x20000Byte�j
#define		FCK_INSRAM_SIZ	0x20000		// �����r�q�`�l�T�C�Y
#define		FCK_EXSRAM_SIZ	0x100000	// �O���r�q�`�l�T�C�Y
#define		FCK_EXSRAM_CHKSIZ	1024*10	// �O���r�q�`�l�P�ʓ�����̃`�F�b�N�����O�X

// ��ذ�q�vý��ް�
#define		FMCK_DATA1	0xA5
#define		FMCK_DATA2	0x5A

#define 	SEC			50
unsigned char Chk_Memory(void)
{
	uchar	ret	= 0;		// �߂�l
	ulong	i;				// ٰ�ߏ��������
	ushort	chk_cnt;		// �����o��
	ushort	ram_no;			// �����Ώ�RAM No.
	uchar	*chk_adr;		// ������ذ���ڽ
	uchar	chk_err;		// ��������
	uchar	sav_data;		// �ް��ޔ�ر
	uchar	chk_data1;		// �����ް��P
	uchar	chk_data2;		// �����ް��Q
	uchar	chk_no = 0;
	xPause( 10 );

	//�����q�`�l����
	fck_chk_err = 0;
	fck_chk_adr = (uchar *)FCK_INSRAM_ADR;
	for (i = 0 ; i < FCK_INSRAM_SIZ ; i++) {						// �����ݸ޽������������s��
		WACDOG;											// WATCHDOG��ϰؾ��
		_di();											// �����݋֎~
		fck_sav_data 	= *fck_chk_adr;					// ������ذ�ް�����
		*fck_chk_adr	= FMCK_DATA1;					// �����ް��Pײ�
		fck_chk_data1	= *fck_chk_adr;					// �����ް��Pذ��
		*fck_chk_adr	= FMCK_DATA2;					// �����ް��Qײ�
		fck_chk_data2	= *fck_chk_adr;					// �����ް��Qذ��
		if ((fck_chk_data1 != FMCK_DATA1) || (fck_chk_data2 != FMCK_DATA2)) {
			// �q�v�����m�f
			fck_chk_err = 1;
		}
		*fck_chk_adr = fck_sav_data;					// ������ذ�ް���ض�ް
		fck_chk_adr++;										// �������ڽ�X�V
		_ei();											// �����݋֎~����
		if (fck_chk_err != 0) {
			// �q�v�����m�f
			break;
		}
	}
	Chk_result.set[chk_no] = fck_chk_err;
	chk_no++;
	xPause( 10 );

	//�q�`�l�P�������[�`�F�b�N
	chk_err = 0;
	for (ram_no = 0 ; ram_no <= 1 ; ram_no++) {			// RAM������ذ��������ٰ��
		chk_err = 0;
		chk_adr = (uchar *)memory_adr[ram_no];
		for (chk_cnt = 0 ; chk_cnt < 100 ; chk_cnt++) {	// RAM�i1Mbyte�j��10K�Â�����
			WACDOG;										// WATCHDOG��ϰؾ��
			for (i = 0 ; i < FCK_EXSRAM_CHKSIZ; i++) {	// �����ݸ޽������������s��
				_di();									// �����݋֎~
				sav_data 	= *chk_adr;					// ������ذ�ް�����
				*chk_adr	= FMCK_DATA1;				// �����ް��Pײ�
				chk_data1	= *chk_adr;					// �����ް��Pذ��
				*chk_adr	= FMCK_DATA2;				// �����ް��Qײ�
				chk_data2	= *chk_adr;					// �����ް��Qذ��
				if ((chk_data1 != FMCK_DATA1) || (chk_data2 != FMCK_DATA2)) {
					// �q�v�����m�f
					chk_err = 1;
				}
				*chk_adr = sav_data;					// ������ذ�ް���ض�ް
				chk_adr++;								// �������ڽ�X�V
				_ei();									// �����݋֎~����
				if (chk_err != 0) {
					// �q�v�����m�f
					break;
				}
			}
			if (chk_err != 0) {
				break;
			}
		}
	}
	Chk_result.set[chk_no] = fck_chk_err;
	chk_no++;
	xPause( 10 );

	//�q�`�l�Q�������[�`�F�b�N
	chk_err = 0;
	chk_adr = (uchar *)memory_adr[2];
	for (chk_cnt = 0 ; chk_cnt < 100 ; chk_cnt++) {	// RAM�i1Mbyte�j��10K�Â�����
		WACDOG;										// WATCHDOG��ϰؾ��
		for (i = 0 ; i < FCK_EXSRAM_CHKSIZ; i++) {	// �����ݸ޽������������s��
			_di();									// �����݋֎~
			sav_data 	= *chk_adr;					// ������ذ�ް�����
			*chk_adr	= FMCK_DATA1;				// �����ް��Pײ�
			chk_data1	= *chk_adr;					// �����ް��Pذ��
			*chk_adr	= FMCK_DATA2;				// �����ް��Qײ�
			chk_data2	= *chk_adr;					// �����ް��Qذ��
			if ((chk_data1 != FMCK_DATA1) || (chk_data2 != FMCK_DATA2)) {
				// �q�v�����m�f
				chk_err = 1;
			}
			*chk_adr = sav_data;					// ������ذ�ް���ض�ް
			chk_adr++;								// �������ڽ�X�V
			_ei();									// �����݋֎~����
			if (chk_err != 0) {
				// �q�v�����m�f
				break;
			}
		}
		if (chk_err != 0) {
			break;
		}
	}
	Chk_result.set[chk_no] = fck_chk_err;
	xPause( 10 );

	return(ret);
	
}

/*[]----------------------------------------------------------------------[]*/
/*| ���C���[�_�[������                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned char ���� 0:����                               |*/
/*|              :                    1:�ُ�                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define		DIPSWITCH_MAX		4
unsigned char Init_Magread( void )
{
	unsigned short	msg = 0;
	MsgBuf		*msb;		// ��Mү�����ޯ̧�߲��
	int		loop;
	short	rd_snd;
	char	ret, i,j;
	uchar	result = 0;
	uchar	sts,sts_bit,dsw;				// �f�B�b�v�X�C�b�`�̏��(0:OFF, 1:ON)
	
	t_TARGET_MSGGET_PRM	Target_WaitMsgID;

	Target_WaitMsgID.Count = 3;
	Target_WaitMsgID.Command[0] = TIMEOUT6;
	Target_WaitMsgID.Command[1] = ARC_CR_SND_EVT;
	Target_WaitMsgID.Command[2] = ARC_CR_E_EVT;

	OPE_red = 0;													// ذ�ް�����r�o�Ƃ��Ȃ�
	Lagtim( OPETCBNO, 6, 15*50 );								// ��ϰ1(15s)�N��

	rd_snd = i = 0;
	RD_mod = 0;
	for( ret = 0; ret == 0; ){
		if( i == 0 ){
			switch( RD_mod ){
				case 0:
					opr_snd( rd_snd = 0 );							// �������R�}���h
					i = 1;
					break;
				case 6:
				case 8:
				case 20:
					opr_snd( rd_snd = 95 );							// ��ԗv��
					i = 1;
					break;
				case 10:
				case 11:
					Lagcan( OPETCBNO, 6 );							// Timer Cancel
					return 0;
				default:
					i = 1;
					break;
			}
		}

		for( loop=1; loop ; ){
			taskchg( IDLETSKNO );									// Change task to idletask
			msb = Target_MsgGet( OPETCBNO, &Target_WaitMsgID );		// ���҂���Ұق�����M�i���͗��߂��܂܁j
			if( NULL == msb ){										// ���҂���ҰقȂ��i����M�j
				continue;
			}

			msg = msb->msg.command;
			switch( msg ){
				case TIMEOUT6:
					loop = 0;
					break;
				case ARC_CR_SND_EVT:								// ���M����
				case ARC_CR_E_EVT:									// �I������ގ�M
					opr_ctl( msg );									// message���͏���
					loop = 0;
					break;
				default:
					break;
			}
			if( !loop ){
				FreeBuf( msb );										// ��Mү�����ޯ̧�J��
			}
		}

		switch( msg ){
			case TIMEOUT6:											// ��ϰ3��ѱ��
				/*** ذ�ސ���ł͂Ȃ� ***/
				opr_snd( 0 );
				ret = 1;
				result = 1;
				break;
			case ARC_CR_SND_EVT:									// ���M����
			case ARC_CR_E_EVT:										// �I������ގ�M
				if(( rd_snd == 95 )&&( msg == ARC_CR_E_EVT )){		// ��ԃf�[�^��M
					ret = 1;
					for(i = 0;i<3;i++){						// �f�B�b�v�X�C�b�`��Ԃ����ʏ��֊i�[
						if(i == 2){
							dsw = 3;
						}
						else{
							dsw = i;
						}
						sts = RDS_REC.state[dsw];			// ����4bit�擾
						sts_bit = (0x01 << (DIPSWITCH_MAX-1));
						for (j = 0; j < 4; j++, sts_bit >>= 1) {
							if (sts & sts_bit) {
								Chk_res_ftp.Chk_Res03.Dip_sw[i][j] = 1;
								
							} else {
								Chk_res_ftp.Chk_Res03.Dip_sw[i][j] = 0;
							}
						}
					}
				}else{
					i = 0;											// ���̏�����
				}
				break;
			default:
				break;
		}
	}
	Chk_result.set[3] = result;		// ����������
	w_stat2 = 0;													// R����ގ�M New Status
	Lagcan( OPETCBNO, 6 );											// Timer Cancel

	return result;
	
}

/*[]----------------------------------------------------------------------[]*/
/*| �������[�_�[������                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned char ���� 0:����                               |*/
/*|              :                    1:�ُ�                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned char Init_NoteReader( void )
{
	uchar	ret = 0;
	MsgBuf		*msb;		// ��Mү�����ޯ̧�߲��
	MsgBuf		msg;		// ��Mү���ފi�[�ޯ̧
	uchar	Syn_flg, result = 0;
	t_TARGET_MSGGET_PRM	Target_WaitMsgID;
	Target_WaitMsgID.Count = 1;
	Target_WaitMsgID.Command[0] = TIMEOUT6;
	
	CP_CN_SYN = 0;			/* SYM Enable	*/
	Cnm_Force_Reset = 1;
	Lagtim( OPETCBNO, 6, 50 );	//1�b
	Syn_flg = 0;
	for(;ret == 0;){
		taskchg( IDLETSKNO );															// ���������ؑ�
		if(Cnm_Force_Reset == 0xff){
			ret = 1;
		}
		msb = Target_MsgGet( OPETCBNO, &Target_WaitMsgID);								// ���҂���Ұق�����M�i���͗��߂��܂܁j
		if( NULL == msb ){																// ���҂���ҰقȂ��i����M�j
			continue;
		}
		//	��Mү���ނ���
		memcpy( &msg , msb , sizeof(MsgBuf) );											// ��Mү���ފi�[
		FreeBuf( msb );																	// ��Mү�����ޯ̧�J��

		switch( msg.msg.command ){														// ү���޺���ނɂ��e�����֕���
			case TIMEOUT6:
				if(Syn_flg == 0){
					CNMTSK_START = 1;													// ���ү�����N��
					CP_CN_SYN = 1;
					Syn_flg = 1;
					Lagtim( OPETCBNO, 6, 50*10 );	//10�b
				}
				else{
					result = 1;
					ret = 1;
				}
				break;
			default:
				break;
		}
	}
	Lagcan( OPETCBNO, 6 );																// Timer Cancel

	if( result ){
		Chk_result.set[4] = 1;															// ����������
		Chk_result.set[5] = 1;															// ����������
	}else{
		Chk_result.set[4] = cn_errst[1];												// ����������
		Chk_result.set[5] = cn_errst[0];												// ����������
	}
	return result;
}

/*[]----------------------------------------------------------------------[]*/
/*| ���V�[�g�v�����^������                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned char ���� 0:����                               |*/
/*|              :                    1:�ُ�                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned char Init_R_Printer( void )
{
	MsgBuf		*msb;		// ��Mү�����ޯ̧�߲��
	MsgBuf		msg;		// ��Mү���ފi�[�ޯ̧
	uchar	result = 0;
	t_TARGET_MSGGET_PRM	Target_WaitMsgID;
	Target_WaitMsgID.Count = 3;
	Target_WaitMsgID.Command[0] = PREQ_INIT_END_R;
	Target_WaitMsgID.Command[1] = TIMEOUT6;
	Target_WaitMsgID.Command[2] = PREQ_SND_COMP_RP;
	Lagtim( OPETCBNO, 6, 50*10 );												// ����������������҂���ϰ���āi�R�b�j
	rct_init_sts = 0;
	PrnInit_R();
	if( PrnJnlCheck() != ON ){				// �ެ���������ڑ�����H
		//	�ެ��ِڑ��Ȃ�
		jnl_init_sts	= 3;				// �ެ����������������Ծ�āi���ڑ��j
	}
	for( ; ; ){
	
		taskchg( IDLETSKNO );															// ���������ؑ�
		msb = Target_MsgGet( OPETCBNO, &Target_WaitMsgID);								// ���҂���Ұق�����M�i���͗��߂��܂܁j
		if( NULL == msb ){																// ���҂���ҰقȂ��i����M�j
			continue;
		}
		//	��Mү���ނ���
		memcpy( &msg , msb , sizeof(MsgBuf) );											// ��Mү���ފi�[
		FreeBuf( msb );																	// ��Mү�����ޯ̧�J��

		switch( msg.msg.command ){														// ү���޺���ނɂ��e�����֕���
			case TIMEOUT6:
				result = 1;
				break;
			case PREQ_INIT_END_R:														// ��ڼ��������������I���ʒm��

				rct_init_sts = 1;														// ڼ���������������Ծ�āi�����������j
				break;

			case	PREQ_SND_COMP_RP:
				RP_I2CSndReq( I2C_NEXT_SND_REQ );
				break;
				
			default:
				break;
		}
		if(msg.msg.command == TIMEOUT6 || msg.msg.command == PREQ_INIT_END_R){
			break;
		}
	}
	Chk_result.set[6] = result;		// ����������
	Lagcan( OPETCBNO, 6 );														// ����������������҂���ϰ����
	return result;
}

/*[]----------------------------------------------------------------------[]*/
/*| �W���[�i���g�v�����^������                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned char ���� 0:����                               |*/
/*|              :                    1:�ُ�                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned char Init_J_Printer( void )
{
	MsgBuf		*msb;		// ��Mү�����ޯ̧�߲��
	MsgBuf		msg;		// ��Mү���ފi�[�ޯ̧
	uchar	result = 0;
	t_TARGET_MSGGET_PRM	Target_WaitMsgID;

	Target_WaitMsgID.Count = 3;
	Target_WaitMsgID.Command[0] = PREQ_INIT_END_J;
	Target_WaitMsgID.Command[1] = TIMEOUT6;
	Target_WaitMsgID.Command[2] = PREQ_SND_COMP_JP;
	Lagtim( OPETCBNO, 6, 50*10 );												// ����������������҂���ϰ���āi�R�b�j
	jnl_init_sts = 0;
	PrnInit_J();
	for( ; ; ){
	
		taskchg( IDLETSKNO );															// ���������ؑ�
		msb = Target_MsgGet( OPETCBNO, &Target_WaitMsgID);								// ���҂���Ұق�����M�i���͗��߂��܂܁j
		if( NULL == msb ){																// ���҂���ҰقȂ��i����M�j
			continue;
		}
		//	��Mү���ނ���
		memcpy( &msg , msb , sizeof(MsgBuf) );											// ��Mү���ފi�[
		FreeBuf( msb );																	// ��Mү�����ޯ̧�J��

		switch( msg.msg.command ){														// ү���޺���ނɂ��e�����֕���
			case TIMEOUT6:
				result = 1;
				break;

			case PREQ_INIT_END_J:														// ���ެ���������������I���ʒm��

				jnl_init_sts = 1;														// �ެ����������������Ծ�āi�����������j
				break;

			case	PREQ_SND_COMP_JP:
				JP_I2CSndReq( I2C_NEXT_SND_REQ );
				break;
			default:
				break;
		}
		if(msg.msg.command == TIMEOUT6 || msg.msg.command == PREQ_INIT_END_J){
			break;
		}
	}
	Chk_result.set[7] = result;		// ����������
	Lagcan( OPETCBNO, 6 );														// ����������������҂���ϰ����
	return result;
}

/*[]----------------------------------------------------------------------[]*/
/*| �������W���[��������                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Init_Sound( void )      	                               |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned char ���� 0:����                               |*/
/*|              :                    1:�ُ�                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
unsigned char Init_Sound( void )
{
	AVM_Sodiac_Init();
	return (uchar)( Avm_Sodiac_Err_flg != SODIAC_ERR_NONE );
}
/*[]----------------------------------------------------------------------[]*/
/*| NG���ړo�^����                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                    |*/
/*| PARAMETER    : unsigned char (*dat)[31]: �\�����镶����                |*/
/*|              : unsigned char cnt       : ���ڐ�                        |*/
/*|              : unsigned char *result   : ���ʏ��̈�̃|�C���^        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void Disp_Chk_NG(const unsigned char (*dat)[31], unsigned char cnt, unsigned char *result)
{
	short			msg = -1;			// ��Mү����
	unsigned char	i, end = 0;			// �`�F�b�N�I���t���O
	char pos,pre;
	char ng_tbl[6] = {0};
	
	memcpy(ng_tbl, result, cnt);
	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CHK_CHR[4] );	//
	for(i=0;i<cnt;i++){
		if((ng_tbl[i] == 0) || (ng_tbl[i] == 0xff)){							// ����܂��͖����{
			grachr( i+1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, dat[i] );	//
		}
		else{
			grachr( i+1,  0, 30, 0, COLOR_MEDIUMBLUE, LCD_BLINK_OFF, dat[i] );	//
		}
	}
	pos = pre = 1;
	grachr( pos,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF,dat[pos-1] );	//���]�\��
	Fun_Dsp( FUNMSG[110] );
	for( ; end == 0; ){
		msg = GetMessage();				// ү���ގ�M
		switch(msg){
			case KEY_TEN_F1:
				BUZPI();
				pos--;
				if(pos <= 0){
					pos = cnt;
				}
				if((ng_tbl[pre-1] == 0) || (ng_tbl[pre-1] == 0xff)){
					grachr( pre,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, dat[pre-1] );	//�ʏ�\��
				}
				else{
					grachr( pre,  0, 30, 0, COLOR_MEDIUMBLUE, LCD_BLINK_OFF, dat[pre-1] );	//�ُ�\��
				}
				grachr( pos,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, dat[pos-1] );	//���]�\��
				pre = pos;
				break;
			case KEY_TEN_F2:
				BUZPI();
				pos++;
				if(pos > cnt){
					pos = 1;
				}
				if((ng_tbl[pre-1] == 0) || (ng_tbl[pre-1] == 0xff)){
					grachr( pre,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, dat[pre-1] );	//�ʏ�\��
				}
				else{
					grachr( pre,  0, 30, 0, COLOR_MEDIUMBLUE, LCD_BLINK_OFF, dat[pre-1] );	//�ُ�\��
				}
				grachr( pos,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, dat[pos-1] );	//���]�\��
				pre = pos;
				break;
			case KEY_TEN_F5:						// �o�^
				BUZPI();
				if(ng_tbl[pos-1] == 1){
					ng_tbl[pos-1] = 0;				// ����
				}
				else{
					ng_tbl[pos-1] = 1;				// NG
				}
				break;
			case KEY_TEN_F4:
				BUZPI();
				memcpy(result, ng_tbl, cnt);		// ���ʏ��ɔ��f
				end = 1;
				break;
			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �����I������                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void Chk_Exit(void)
{
	ushort msg;
	uchar	i;
	T_FrmChk_result	Frm_result;
	dispclr();
	bigcr(0, 2, 18, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (uchar *)CHK_CHR_BIG[4]);	// "��������
	grachr(2, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, OPE_CHR[2]);				// "������������������������������"
	grachr(3, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, CHK_CHR[5]);				// "
	// debug �t�@�C�����쐬
	/* ���ʏ��t�@�C�����̍쐬 */
	sprintf((char *)&Chk_info.dir_chkresult, (const char*)SYSMNT_CHKRESULT_DIRECT);
	sprintf((char *)&Chk_info.fn_chkresult, (const char*)SYSMNT_CHKRESULT_FILENAME, 
			"123456"							/*�p�^�[���ԍ�������\��*/,
			Chk_result.Chk_date.Year%100, 		/*��������*/
			Chk_result.Chk_date.Mon, 			/**/
			Chk_result.Chk_date.Day, 			/**/
			Chk_result.Chk_date.Hour, 			/**/
			Chk_result.Chk_date.Min			/**/
	);
	// ���ʏ��i�[����(��)
	memcpy( &Chk_res_ftp.Chk_Res01.Chk_date, &Chk_result.Chk_date, 6);
	Chk_res_ftp.Chk_Res01.Kakari_no = Chk_result.Kakari_no;
	strcpy((char *)Chk_res_ftp.Chk_Res01.Machine_No, "123456");
	strcpy((char *)Chk_res_ftp.Chk_Res01.Model, "FT4000FX");
	Chk_res_ftp.Chk_Res01.System = 10;
	Chk_res_ftp.Chk_Res01.Sub_Money = 100;
	strcpy((char *)Chk_res_ftp.Chk_Res02.Version[0], "MH123456");
	strcpy((char *)Chk_res_ftp.Chk_Res02.Version[1], "MH012345");
	strcpy((char *)Chk_res_ftp.Chk_Res02.Version[2], "MH111111");
	for(i=0;i<27;i++){
		strcpy( (char *)Chk_res_ftp.Chk_Res05.Mojule[i], "MH111111" );
	}
	Fun_Dsp( FUNMSG[113] );

	for( ; ; )
	{
		msg = GetMessage();				// ү���ގ�M
		switch(msg){
			case KEY_TEN_F5:			// �Č���
				Chk_info.pt_no = 0;
				Chk_info.Chk_mod = 0;
				memset(&Chk_info.Chk_str1, 0, 30);		// �Œ蕶����1�E2���N���A����
				return;
			case KEY_TEN_F3:			// FTP
				BUZPI();
				SysMnt_FTPServer();
				dispclr();
				bigcr(0, 2, 18, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, (uchar *)CHK_CHR_BIG[4]);	// "��������
				grachr(2, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, OPE_CHR[2]);				// "������������������������������"
				grachr(3, 0, 30, 0, COLOR_INDIGO, LCD_BLINK_OFF, CHK_CHR[5]);				// "]
				Fun_Dsp( FUNMSG[113] );
				break;
			case KEY_TEN_F4:		// ��
				Frm_result.prn_kind = R_PRI;
				Frm_result.Serial_no = 999999;			// TODO:�����̓o�[�R�[�h�X�L�����������i���@��ݒ肷��̂ŕۗ�
				memcpy(&Frm_result.ChkTime, &Chk_result.Chk_date, sizeof(date_time_rec));
				Frm_result.Kakari_no = Chk_result.Kakari_no;
				Frm_result.Chk_no = Chk_result.Chk_no;
				Frm_result.Card_no = Chk_result.Card_no;
				queset( PRNTCBNO, PREQ_CHKMODE_RESULT, sizeof(T_FrmChk_result), &Frm_result );
				break;;
			case KEY_TEN_F1:			// �Č���
				Chk_info.pt_no = 0;
				Chk_info.Chk_mod = 0;
				System_reset();				// �ċN��
				break;
			default:
				break;
		}
	}
}
	
/*[]----------------------------------------------------------------------[]*/
/*| �������[�h�O����                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ChekModeMain( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned char 0:����                                    |*/
/*|                              1:������                                  |*/
/*|                              2:�����I��                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned char Init_Chkmod(void)
{
	
	if (stricmp((char*)Chk_info.Chk_str1, (char*)CHKMODE_STR1) != 0){			// �Œ蕶����@�s��v
		/*�C�j�V�����C�Y���čŏ�������{*/
		log_init();
		memclr(1);
		memset(&Chk_info, 0, sizeof(t_Chk_info));		// �������G���A�N���A
		return 0;
	}
	else if(stricmp((char*)Chk_info.Chk_str2, (char*)CHKMODE_STR2) != 0){		// �Œ蕶����@�݈̂�v
		/* �����r���ł̕��d */
		return 1;
	}
	else{				// ������v
		/* �����I����̕��d */
		return 2;
	}
}

