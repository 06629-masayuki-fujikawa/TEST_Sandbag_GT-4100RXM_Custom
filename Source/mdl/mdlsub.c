/*[]----------------------------------------------------------------------[]*/
/*| Ӽޭ��(���Cذ�ް,�ųݽ)�֘A����                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"system.h"
#include	"mem_def.h"
#include	"mdl_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"prm_tbl.h"
#include	"FlashSerial.h"
#include	"message.h"
#include 	"AppServ.h"
#include 	"fla_def.h"



static	short	op_modl	( short );

unsigned char CheckCRC8( unsigned char *src, unsigned char size );
unsigned char CheckLRC8( unsigned char *src, unsigned char size );
void 	avm_sodiac_callback( int ch , D_SODIAC_ID id , D_SODIAC_E err );
void	AVM_Sodiac_SemFree( void );
uchar	AVM_Sodiac_SemGet( void );
BOOL	AVM_Sodiac_PayOff_Judge( void );

AVM_SODIAC_CTRL			AVM_Sodiac_Ctrl[2];
st_sodiac_version_info 	Avm_Sodiac_Info;
unsigned char			Avm_Sodiac_Info_end = 0;
unsigned char			Avm_Sodiac_Err_flg = 0xFF;
unsigned short			Avm_Sodiac_PayOff_Volume[2] = {0xFFFF,0xFFFF};
/***** ذ�޺���ޗpð��� *****/
const char RD_RED[][3] = {
	{ M_R_READ, 0x98, 0x93 },										// ذ�޺����(ذ�ޓ���~)
	{ M_R_READ, 0x91, 0x83 },										// ذ�޺����(���������~)
	{ M_R_READ, 0x98, 0x83 },										// ذ�޺����(ذ�ޓ���~, �v�����^�Ȃ�)
};
/***** ذ�޺����(�ړ�������A��)ð��� *****/
/* ð��ق̈Ӗ�                             */
/*  ذ�ް�����,  ����,  �؊�,  RD_mod      */
/* {  M_R_MOVE,  0x8A,  0x90,   10   }     */
/* RD_mod = 10 : �������҂����          */
/*          12 : ����p��(����ð��ق𑗐M) */
/*          13 : ����I�����              */
/*******************************************/
const char RD_CMD[][4] = {
	{ M_R_MOVE, 0x8A, 0x80, 10 },	//  0 ��o���ړ�(�����Ӱ���)
	{ M_R_MOVE, 0x86, 0x90, 10 },	//  1 ��o���ړ�(�����Ӱ���)
	{ M_R_MOVE, 0x85, 0x93, 12 },	//  2 ���Ԍ��捞(��r�o)
	{ M_R_MOVE, 0x84, 0x83, 13 },	//  3
	{ M_R_PRWT, 0x86, 0xB0, 10 },	//  4 ���Ԍ��̎���
	{ M_R_WRIT, 0xAA, 0xA0, 10 },	//  5 �������(���̧��L��)
	{ M_R_PRNT, 0x84, 0x80, 13 },	//  6 ���޽���捞(��r�o)
	{ M_R_MOVE, 0x83, 0x83, 12 },	//  7 ���޽���捞(�O�ړ�+��r�o)
	{ M_R_PRNT, 0x85, 0x80, 12 },	//  8
	{ M_R_MOVE, 0x84, 0x80, 13 },	//  9
	{ M_R_MOVE, 0x83, 0x83, 12 },	// 10 ����߲�޶��ޏ���(���̧��L��)
	{ M_R_PRWT, 0xA6, 0x90, 10 },	// 11
	{ M_R_MOVE, 0x84, 0x80, 13 },	// 12 ���Ԍ�(�̎���)�捞(��r�o)
	{ M_R_WRIT, 0xA6, 0xB0, 10 },	// 13 ���Z���~��������(���̧��L��)
	{ M_R_PRNT, 0x85, 0x90, 12 },	// 14 ���Ԍ��󎚎捞(��r�o)
	{ M_R_MOVE, 0x84, 0x83, 13 },	// 15
	{ M_R_PRNT, 0x86, 0xB0, 10 },	// 16 ���Ԍ��߂�(�a���蒆�~)
	{ M_R_PRWT, 0x86, 0xB0, 10 },	// 17 ���Ԍ��߂�(�a���蒆�~����)
	{ M_R_PRWT, 0x86, 0xB0, 10 },	// 18 ����
	{ M_R_MOVE, 0x85, 0x83, 12 },	// 19 �p��
	{ M_R_MOVE, 0x84, 0x83, 13 },	// 20
	{ M_R_PRNT, 0x80, 0x90, 13 },	// 21 ���Ԍ��󎚎捞(��󎚎��A��r�o)
	{ M_R_WRIT, 0x8A, 0xA0, 10 },	// 22 �������(���̧�����)
	{ M_R_MOVE, 0x98, 0x80, 10 },	// 23 ���ގ�荞��
	{ M_R_WRIT, 0xAA, 0xA0, 13 },	// 24 �������(���ޔ��s�p�A���̧��L��)
	{ M_R_WRIT, 0xAA, 0x80, 13 },	// 25 �W�����ޏ���(���ޔ��s�p�A���̧��L��)
	{ M_R_MOVE, 0x83, 0x83, 12 },	// 26 ����߲�޶��ޏ���(���̧�����)
	{ M_R_PRWT, 0x86, 0x90, 10 },	// 27
	{ M_R_WRIT, 0x86, 0xB0, 10 },	// 28 ���Z���~��������(���̧��Ȃ�)
	{ M_R_MOVE, 0x83, 0x83, 12 },	// 29 �񐔌�����(���̧�����)
	{ M_R_PRWT, 0x86, 0xB0, 10 },	// 30
	{ M_R_MOVE, 0x83, 0x83, 12 },	// 31 �񐔌�����(���̧��L��)
	{ M_R_PRWT, 0xA6, 0xB0, 10 },	// 32
	{ M_R_MOVE, 0x82, 0x80, 13 },	// 33 �v�����^�[�����p��(��r�o)
	{ M_R_WRIT, 0xC9, 0x80, 12 },	// 34 �v�����^�[�����p��(�p���������݁A��r�o)
	{ M_R_MOVE, 0x82, 0x80, 13 },	// 35
	{ M_R_MOVE, 0xCA, 0x80, 10 },	// 36 �p���������݁A�O�r�o
};

static short Make_FeeString(const unsigned long , unsigned short * );
static char set_digit6(unsigned short *, short, char, short *);
static char set_digit5(unsigned short *, short, short, char, short *);
static char set_digit4(unsigned short *, short, char, short *);
static char set_digit3(unsigned short *, short, char, short *);
static char set_digit2(unsigned short *, short, short, short *);
static short Make_ShashituString(const unsigned short , unsigned short *);
static char set_digit3_shashitu(unsigned short * , short , char , short *);
static char set_digit2_shashitu(unsigned short * , short , short , short *);
static char set_digit4_shashitu(unsigned short * msg, short dig, char flg, short * p);
#define GET_BYTE_HIGH(num)	(unsigned char)((num >> 8) & 0xff)		// ���ߏ�ʃo�C�g�擾
#define GET_BYTE_LOW(num)	(unsigned char)(num & 0xff)				// ���߉��ʃo�C�g�擾
#define CMNMSG_NUMBER	3
char use_avm;
char pre_volume[2] = {0xff, 0xff};
unsigned short	Shashitu_EndStr;	// �Ԏ��ǂݏグ�����̍Ō�ɂ��鉹��("�Ԃł�" or "�Ԃ�")

/*[]----------------------------------------------------------------------[]*/
/*| ���Cذ�ް�ړ������                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : rd_mov( short faz )                                     |*/
/*| PARAMETER    : faz : RD_CMDð��ٍ��ڇ�                                 |*/
/*| RETURN VALUE : ret : 0 = OK,  -1 = NG                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	rd_mov( short faz )
{
	char	comm;

	if( op_modl( 0 ) != 0 ) {
		return( -1 );
	}
	MDLdata[0].mdl_data.idc1 = 1;									// Block No.
	MDLdata[0].mdl_data.idc2 = 3;									// Data Length
	MDLdata[0].mdl_size = 5;										// Send Length STX-ETX
	MDLdata[0].mdl_endf = 1;										// Set  ETX

	comm = 
	MDLdata[0].mdl_data.rdat[0] = RD_CMD[faz][0];
	MDLdata[0].mdl_data.rdat[1] = RD_CMD[faz][1];
	MDLdata[0].mdl_data.rdat[2] = RD_CMD[faz][2];
	if( comm != M_R_MOVE ){
		switch( faz )
		{
			case 18:												// ���Ԍ�ײ�(����)
				memcpy( &MDLdata[0].mdl_data.rdat[3], MDP_buf, (size_t)MDP_siz );
				MDLdata[0].mdl_data.idc2 += (uchar)MDP_siz;			// Data Length
				MDLdata[0].mdl_size += MDP_siz;						// Send Length STX-ETX

				md_pari( &MDLdata[0].mdl_data.rdat[ 3 + ( MDP_siz - MDP_mag ) ], MDP_mag, 0 );	// Set Data Parity

				if(( comm == M_R_WRIT )||( comm == M_R_PRWT )||( comm == M_R_PRNT)){
					MDLdata[0].mdl_data.rdat[2] |= RD_pos;
				}
				break;
			default:												// �����ײ�, ����߲��ײđ�
				memcpy( &MDLdata[0].mdl_data.rdat[3], MDP_buf, (size_t)MDP_siz );
				if((prm_get( COM_PRM,S_SYS,11,1,1 ) == 1) && (MDP_buf[127] == 1 /*MAGred[MAG_GT_APS_TYPE] == 1*/) ){
					MDLdata[MTS_RED].mdl_data.rdat[2] |= 0x08;		// ײ��ް��`�� = GT���C̫�ϯ�
					if( (faz == 11) || (faz == 27) ){	// �v���y�C�h�J�[�h�̏ꍇ
						MDLdata[MTS_RED].mdl_data.idc2 += (char)( MDP_siz + 1);// Data Length
						/* �`�F�b�N�R�[�h�ǉ� */
						/* ID���ށ`�����т܂ł�LRC��0xFF�̔r���I�_���a */
						MDLdata[MTS_RED].mdl_data.rdat[3 + MDP_siz] =
							CheckLRC8( &MDLdata[MTS_RED].mdl_data.rdat[3 + ( MDP_siz - MDP_mag )], (unsigned char)MDP_mag );
						MDLdata[MTS_RED].mdl_data.rdat[3 + MDP_siz] ^= 0xFF;
						MDLdata[MTS_RED].mdl_size +=  MDP_siz + 1;		/* Send Length STX-ETX*/
					}else if(faz == 25){	// �W���J�[�h�̏ꍇ
						MDLdata[MTS_RED].mdl_data.idc2 += (char)( MDP_siz );// Data Length
						MDLdata[MTS_RED].mdl_size +=  MDP_siz;		/* Send Length STX-ETX*/
					}else{	// �v���y�C�h�J�[�h�ȊO�̏ꍇ
						MDLdata[MTS_RED].mdl_data.idc2 += (char)( MDP_siz + 1);// Data Length
						/* CRC8�ǉ� */
						MDLdata[MTS_RED].mdl_data.rdat[3 + MDP_siz] =
							CheckCRC8( &MDLdata[MTS_RED].mdl_data.rdat[3 + ( MDP_siz - MDP_mag )], (unsigned char)MDP_mag );
						MDLdata[MTS_RED].mdl_size +=  MDP_siz + 1;		/* Send Length STX-ETX*/
					}
				}else{
					MDLdata[MTS_RED].mdl_data.idc2 += (char)( MDP_siz );// Data Length
					MDLdata[MTS_RED].mdl_size +=  MDP_siz;			/* Send Length STX-ETX*/
				}
				if(( comm == M_R_WRIT )||( comm == M_R_PRWT )||( comm == M_R_PRNT)){
					MDLdata[0].mdl_data.rdat[2] |= RD_pos;
				}
				break;
		}
	}
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	inc_dct( READ_DO, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
	switch( comm ){
		case M_R_MOVE:
			break;
		case M_R_WRIT:
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			inc_dct( READ_WR, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
			break;
		case M_R_PRWT:
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			inc_dct( READ_WR, 1 );
//			inc_dct( VPRT_DO, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
			break;
		case M_R_PRNT:
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			inc_dct( VPRT_DO, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
			break;
		default:
			break;
	}
	RD_mod = RD_CMD[faz][3];
	RD_SendCommand = (uchar)(RD_CMD[faz][1] & 0x0f);
	Mag_LastSendCmd = comm;											// ���Cذ�ނ֍Ō�ɑ��M��������ޕۑ�

	mts_req |= MTS_BCR1;											// ���M�v��FLG ON����

	return( 0 );
}

/*[]----------------------------------------------------------------------[]*/
/*| Reader Initialize                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : rd_init( short mod )                                    |*/
/*| PARAMETER    : mod : 1 = Card In,  2 = Card Out                        |*/
/*| RETURN VALUE : ret : 0 = OK,  -1 = NG                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	rd_init( short mod )
{
	short	ret = 0;

	if( op_modl( 0 ) == 0 ){

		MDLdata[0].mdl_data.idc1 = 1;								// Block No.
		MDLdata[0].mdl_data.idc2 = 2;								// Data Length
		MDLdata[0].mdl_data.rdat[0] = M_R_INIT;						// Init Command
		if( prm_get( COM_PRM,S_SYS,11,1,1 ) == 1){			// ���C���[�_�[�^�C�v(APS/GT���C�t�H�[�}�b�g���p)
			mod |= 0x80;
		}
		MDLdata[0].mdl_data.rdat[1] = (char)mod;
		MDLdata[0].mdl_size = 4;									// Send Length STX-ETX
		MDLdata[0].mdl_endf = 1;									// Set  ETX

		mts_req |= MTS_BCR1;										// ���M�v��FLG ON����

	}else{
		ret = -1;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| Reader Read Command Send                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : rd_read( short mod )                                    |*/
/*| PARAMETER    : mod : 0 = ذ�ް����~,  1 = ��������ۗ�                 |*/
/*| RETURN VALUE : ret : 0 = OK,  -1 = NG                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	rd_read( short mod )
{
	short	ret = 0;
	const char	*r_tbl;

	if( op_modl( 0 ) == 0 ){

		MDLdata[0].mdl_data.idc1 = 1;								// Block No.
		MDLdata[0].mdl_data.idc2 = 3;								// Data Length
		MDLdata[0].mdl_size = 5;									// Send Length STX-ETX
		MDLdata[0].mdl_endf = 1;									// Set  ETX
		if( mod == 0 ){
			r_tbl = &RD_RED[0][0];
		}else if (mod == 2 ) {
			r_tbl = &RD_RED[2][0];
		}else{
			r_tbl = &RD_RED[1][0];
		}
		MDLdata[0].mdl_data.rdat[0] = *r_tbl;
		MDLdata[0].mdl_data.rdat[1] = *(r_tbl+1);
		MDLdata[0].mdl_data.rdat[2] = *(r_tbl+2);

		mts_req |= MTS_BCR1;										// ���M�v��FLG ON����

	}else{
		ret = -1;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| Reader Test Command                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : rd_test( short mod )                                    |*/
/*| PARAMETER    : mod : Test Mode( 1 - 4 )                                |*/
/*| RETURN VALUE : ret : 0 = OK,  -1 = NG                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	rd_test( short mod )
{
	short	ret = 0;

	if( op_modl( 0 ) == 0 ){

		MDLdata[0].mdl_data.idc1 = 1;								// Block No.
		MDLdata[0].mdl_data.idc2 = 2;								// Data Length
		MDLdata[0].mdl_data.rdat[0] = M_R_TEST;						// Move Command
		MDLdata[0].mdl_data.rdat[1] = (char)mod;
		MDLdata[0].mdl_size = 4;									// Send Length STX-ETX
		MDLdata[0].mdl_endf = 1;									// Set  ETX

		mts_req |= MTS_BCR1;										// ���M�v��FLG ON����

	}else{
		ret = -1;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| Reader (FB7000) ����ݽ Command                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*|	'm'�R�}���h�𑗐M����B												   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : d1 : ����ݽ���ށi1=���ԗv���j						   |*/
/*|				   d2 : �\��P�i����0�Œ�j								   |*/
/*|				   d3 : �\��Q�i����0�Œ�j								   |*/
/*| RETURN VALUE : ret : 0 = OK,  -1 = NG                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : okuda                                                   |*/
/*| Date         : 2006-06-23                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
short	rd_FB7000_MntCommandSend( uchar d1, uchar d2, uchar d3 )
{
	short	ret = 0;

	if( op_modl( 0 ) == 0 ){

		MDLdata[0].mdl_data.idc1 = 1;								// Block No.
		MDLdata[0].mdl_data.idc2 = 2;								// Data Length
		MDLdata[0].mdl_data.rdat[0] = 'm';							// ����ݽ�����
		MDLdata[0].mdl_data.rdat[1] = d1;
		MDLdata[0].mdl_data.rdat[2] = d2;
		MDLdata[0].mdl_data.rdat[3] = d3;
		MDLdata[0].mdl_size = 6;									// Send Length STX-ETX
		MDLdata[0].mdl_endf = 1;									// Set  ETX

		mts_req |= MTS_BCR1;										// ���M�v��FLG ON����

		// �����������d��M�Ŕ���
		RD_SendCommand = (uchar)0;
	}else{
		ret = -1;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| Reader Font Data Send                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : rd_font( void )                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret : 0 = OK,  -1 = NG                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
const	char FONT_01[] = { 0x09, 0x0d, 0xfb, 0x08, 0x7d, 0xb7, 0xb5, 0x7d, 0x69 };

short	rd_font( void )
{
	short	ret = 0;

	if( op_modl( 0 ) == 0 ){

		MDLdata[0].mdl_data.idc1 = 1;								// Block No.
		MDLdata[0].mdl_data.idc2 = 11;								// Data Length
		MDLdata[0].mdl_data.rdat[0] = M_R_FONT;						// FONT Command
		MDLdata[0].mdl_data.rdat[1] = 0xef;
		memcpy( &( MDLdata[0].mdl_data.rdat[2] ), FONT_01, 9 );
		MDLdata[0].mdl_size = 13;									// Send Length STX-ETX
		MDLdata[0].mdl_endf = 1;									// Set  ETX

		mts_req |= MTS_BCR1;										// ���M�v��FLG ON����

	}else{
		ret = -1;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| Anounce Machine Initialize                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : an_init( char dat )                                     |*/
/*| PARAMETER    : dat : ���f�������@                                      |*/
/*| RETURN VALUE : ret : 0 = OK,  -1 = NG                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4800N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	an_init( char dat )
{
	short	ret = 0;

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| Anounce Machine Version Request                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : an_test( char mod )                                     |*/
/*| PARAMETER    : dat : 4 = Version Request                               |*/
/*| RETURN VALUE : ret : 0 = OK,  -1 = NG                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4800N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	an_test( char mod )
{
	short	ret = 0;

	if (use_avm == AVM_NOT_CONNECT) {			// �����ē��L���̔���
		return -1;
	}

	if( MDLdata[1].mdl_size == 0 ){									// Exist Data ? (N)

		MDLdata[1].mdl_data.idc1 = 1;								// Block No.
		MDLdata[1].mdl_data.idc2 = 2;								// Data Length
		MDLdata[1].mdl_data.rdat[0] = M_A_TEST;						// Test Command
		MDLdata[1].mdl_data.rdat[1] = mod;

		MDLdata[1].mdl_size = 4;									// Send Length STX-ETX
		MDLdata[1].mdl_endf = 1;									// Set  ETX

		mts_req |= MTS_BAVM1;										// ���M�v��FLG ON����
		avm_snd();

	}else{
		ret = -1;
	}
	return( ret );
}


//[]----------------------------------------------------------------------[]
///	@brief			�x�����̎w��񐔐���
//[]----------------------------------------------------------------------[]
///	@return			ret : 0 = OK,  -1 = NG
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/02/05<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
short an_boo2(short	seg)
{

	if ( MDLdata[1].mdl_size != 0) {			// �����ē��L���̔���
		return -1;
	}

	// ���ʉ����w�肵���񐔕�������
	MDLdata[1].mdl_data.idc1 = 1;						// Block No.
	MDLdata[1].mdl_data.idc2 = 7;						// Data Length
	MDLdata[1].mdl_data.rdat[0] = 0x0D;					// �R�}���hID
	MDLdata[1].mdl_data.rdat[1] = GET_BYTE_HIGH(seg);	// ����No.
	MDLdata[1].mdl_data.rdat[2] = GET_BYTE_LOW(seg);	// ����No.
	MDLdata[1].mdl_data.rdat[3] = 0x0f;	// ������
	MDLdata[1].mdl_data.rdat[4] = 0;					// �E�F�C�g�b
	MDLdata[1].mdl_data.rdat[5] = 1;					// 1�`���l���Œ�
	MDLdata[1].mdl_data.rdat[6] = 0;				// ���f���@(�����ݒ�)

	MDLdata[1].mdl_endf = 1;							// Set  ETX
	MDLdata[1].mdl_size = 10;							// Send Length STX-ETX(ID1,ID2,�f�[�^,ETX)

	mts_req |= MTS_BAVM1;										// ���M�v��FLG ON����

	// �������M
	avm_snd();
	
	return 0;
}
/*[]----------------------------------------------------------------------[]*/
/*| �����R�}���h���M����(�������b�Z�[�W�Ή���)                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : an_msag2( )                                             |*/
/*| PARAMETER    : an_msgno : ���b�Z�[�W�����i�[����Ă����ޯ̧            |*/
/*|                msg_cnt : ���b�Z�[�W�̌�                              |*/
/*|                cnt : �J��Ԃ���( 0 : ������~ )                      |*/
/*|                wat : �C���^�[�o�� ( 0 - 9 Second )                     |*/
/*|                ch  : �����`�����l�� ( 1 or 2 )                         |*/
/*| RETURN VALUE : ret : 0 = OK,  -1 = NG                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2009-09-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
short	an_msag( short *an_msgno, short msg_cnt, short cnt, short wat, char ch)
{
	short	m_cnt,ret, i, j;
	
	ret = 0;

	if(avm_alarm_flg && ch == 1){			// �x�񔭖C���̓`�����l��1�ɃR�}���h����Ȃ�
		return 0;
	}
	if( MDLdata[1].mdl_size == 0 ){									// Exist Data ? (N)
		m_cnt = msg_cnt*2;
		MDLdata[1].mdl_data.rdat[0] = 0x0D;
		for(i = 0, j = 1; i < msg_cnt; i++){
			MDLdata[1].mdl_data.rdat[j] = GET_BYTE_HIGH(an_msgno[i]);
			MDLdata[1].mdl_data.rdat[j+1] = GET_BYTE_LOW(an_msgno[i]);
			j+=2;
		}
		MDLdata[1].mdl_data.idc1 = 1;								// Block No.
		MDLdata[1].mdl_data.idc2 = (char)(5 + m_cnt);				// Data Length
		MDLdata[1].mdl_data.rdat[1 + m_cnt] = (char)(cnt | 0xf0);
		MDLdata[1].mdl_data.rdat[2 + m_cnt] = (char)(wat & 0x0f);
		MDLdata[1].mdl_data.rdat[3 + m_cnt] = ch;					// �����`���l��
		MDLdata[1].mdl_data.rdat[4 + m_cnt] = 0;					// ���f���@(�����ݒ�)
		MDLdata[1].mdl_size = 8 + m_cnt;							// Send Length STX-ETX(ID1,ID2,�f�[�^,ETX)
		MDLdata[1].mdl_endf = 1;									// Set  ETX
		mts_req |= MTS_BAVM1;										// ���M�v��FLG ON����
		avm_snd();
	}
	else{
		ret = -1;
	}
	return( ret );

}
/*[]----------------------------------------------------------------------[]*/
/*| ������~����                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : an_stop( )                                             |*/
/*| PARAMETER    : ch  : �����`�����l�� ( 1 or 2 )                         |*/
/*| RETURN VALUE :                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2012-05-07                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
void an_stop(char ch)
{
	if(avm_alarm_flg && ch == 1){			// �x�񔭖C���̓`�����l��1�ɃR�}���h����Ȃ�
		return;
	}
	if( MDLdata[1].mdl_size == 0 ){									// Exist Data ? (N)
		MDLdata[1].mdl_data.idc1 = 1;			// Block No.
		MDLdata[1].mdl_data.idc2 = 4;			// Data Length
		MDLdata[1].mdl_data.rdat[0] = 0x0C;		// �R�}���hID
		MDLdata[1].mdl_data.rdat[1] = ch;		// �I���`���l��
		MDLdata[1].mdl_data.rdat[2] = 0;		// �������f���@�������ݒ�ɏ]��
		MDLdata[1].mdl_data.rdat[3] = 1;		// �L���[�C���O���̊J�n�R�}���h�����ׂĖ���

		MDLdata[1].mdl_endf = 1;				// Set  ETX
		MDLdata[1].mdl_size = 7;				// Send Length STX-ETX(ID1,ID2,�f�[�^,ETX)
		mts_req |= MTS_BAVM1;										// ���M�v��FLG ON����
		avm_snd();
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Anounce Machine Message Define                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : an_defn( no, cnt, msg )                                 |*/
/*| PARAMETER    : no   : Message No.( 40h - 44h )                         |*/
/*|                cnt  : Anounce Count( 1 - 10 )                          |*/
/*|                *msg :                                                  |*/
/*| RETURN VALUE : ret : 0 = OK,  -1 = NG                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4800N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	an_defn( char no, char cnt, char* msg )
{
	short	i;
	short	ret = 0;

	if( MDLdata[1].mdl_size == 0 ){									// Exist Data ? (N)

		MDLdata[1].mdl_data.idc1 = 1;								// Block No.
		MDLdata[1].mdl_data.idc2 = (char)( 3 + cnt );				// Data Length
		MDLdata[1].mdl_data.rdat[0] = M_A_DEFN;
		MDLdata[1].mdl_data.rdat[1] = (char)no;
		for( i = 0; i < (short)cnt; i++ ){
			MDLdata[1].mdl_data.rdat[2 + i] = msg[i];
		}
		MDLdata[1].mdl_data.rdat[2 + i] = (char)0xf9;

		MDLdata[1].mdl_size = 5 + i;								// Send Length STX-ETX
		MDLdata[1].mdl_endf = 1;									// Set  ETX

		mts_req |= MTS_BAVM1;										// ���M�v��FLG ON����

	}else{
		ret = -1;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| Wait Module Communication Buffer Empty                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : op_modl( short no )                                     |*/
/*| PARAMETER    : no  : Module No.                                        |*/
/*| RETURN VALUE : ret : 0 = OK,  -1 = NG                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static short	op_modl( short no )
{
	short	ret;
	ret		=	0;
	OP_MODUL = 50;													// Wait 100ms

	while(( mts_req & MTS_BCR1 ) == MTS_BCR1 ){						// ذ�ޑ��M�v������?

		taskchg( IDLETSKNO );

		if( OP_MODUL == 0 ) {
			break;
		}
	}

	OP_MODUL = -1;

	ret = ((short)( mts_req & MTS_BCR1 ) == 0 ) ? 0 : -1; 
	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| Set Card Data Parity                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : md_pari( dat, siz, mod )                                |*/
/*| PARAMETER    : *dat : Input Data Address                               |*/
/*|                siz  : Character Byte                                   |*/
/*|                mod  : 0 = ODD,  1 = EVEN                               |*/
/*| RETURN VALUE : ret : 0 = OK,  -1 = NG                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	md_pari( unsigned char *dat, unsigned short siz, char mod )
{
	unsigned short	sdat;
	unsigned short	pcnt;
	unsigned char	rtc;
	unsigned char	i;
	uchar			tik_syu;										// ����
	uchar			set_id;											// �VID�ɕϊ�������ID
	
	if( prm_get( COM_PRM,S_PAY,10,1,4 ) != 0 ){						// �V�J�[�h�h�c�g�p����H

		// �V�J�[�h�h�c���g�p����ꍇ

		set_id = *dat;												// �����݂��s����ID(�������Ұ�)�擾

		for( tik_syu = 1 ; tik_syu <= TIK_SYU_MAX ; tik_syu++ ){	// �����݂��s����ID�̌��������

			if( set_id == tik_id_tbl[tik_syu][0] ){					// ID��v�H
				break;												// YES
			}
		}
		// ���ۂɏ������ތ�ID���擾
		switch( tik_syu ){											// ����H

			case	1:												// ���Ԍ��i�`�q�|�����j
			case	2:												// ���Ԍ��i�`�q-�T�����~���j
			case	3:												// ���Ԍ��i�`�q-�o�񒆎~���j
			case	4:												// ���Ԍ��i���Z�O�j
			case	5:												// ���Ԍ��i���Z��j
			case	6:												// ���Ԍ��i���~���j
			case	7:												// ���Ԍ��i�|�����j
			case	8:												// ���Ԍ��i�Đ��Z���~���j
			case	9:												// ���Ԍ��i���Z�ς݌��j
			case	10:												// �Ĕ��s��
			case	11:												// ������

				set_id = tik_id_tbl[tik_syu][ReadIdSyu.pk_tik-1];
				break;

			case	12:												// �`�o�r�����

				set_id = tik_id_tbl[tik_syu][ReadIdSyu.teiki-1];
				break;

			case	13:												// �v���y�C�h�J�[�h

				set_id = tik_id_tbl[tik_syu][ReadIdSyu.pripay-1];
				break;

			case	14:												// �񐔌�

				set_id = tik_id_tbl[tik_syu][ReadIdSyu.kaisuu-1];
				break;

			case	15:												// �T�[�r�X���i�|�����E�������j

				set_id = tik_id_tbl[tik_syu][ReadIdSyu.svs_tik-1];
				break;

			case	16:												// �W���J�[�h

				set_id = tik_id_tbl[tik_syu][ReadIdSyu.kakari-1];
				break;
		}
		*dat = set_id;												// ���ۂɏ������ތ�ID�ɏ������ް���ύX
	}

	while( siz ){													// size loop
		sdat = *dat;												// Source Data
		pcnt = 0;													// Parity Bit Count
		for( i = 0; i < 7; i++ ){									// Bit0 to BIT6
			rtc = (uchar)( sdat & 0x01 );							// LSB Check
			if( rtc == 1 ){											// BIT SET ?(Y)
				pcnt ++;											// Bit Count
			}
			sdat >>=1;												// Next Bit Set (LSB)
		}
		if( mod == 0 ){												// Odd Parity Add?(Y)
			if(( pcnt % 2 ) == 0 ){									// Not Odd?(Y)
				*dat |= 0x80;										// Odd Parity Set
			}
		}else{														// Even Parity Add?(Y)
			if(( pcnt % 2 ) != 0 ){									// Not Even?(Y)
				*dat |= 0x80;										// Even Parity Set
			}
		}
		dat++;														// Next Data
		siz--;														// Size -1
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Set Card Data Parity                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : md_pari2( dat, siz, mod )                               |*/
/*| PARAMETER    : *dat : Input Data Address                               |*/
/*|                siz  : Character Byte                                   |*/
/*|                mod  : 0 = ODD,  1 = EVEN                               |*/
/*| RETURN VALUE : ret : 0 = OK,  -1 = NG                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700N��藬�p                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	md_pari2( unsigned char *dat, unsigned short siz, char mod )
{
	unsigned short	sdat;
	unsigned short	pcnt;
	unsigned char	rtc;
	unsigned char	i;
	
	while( siz ){													// size loop
		sdat = *dat;												// Source Data
		pcnt = 0;													// Parity Bit Count
		for( i = 0; i < 7; i++ ){									// Bit0 to BIT6
			rtc = (uchar)( sdat & 0x01 );							// LSB Check
			if( rtc == 1 ){											// BIT SET ?(Y)
				pcnt ++;											// Bit Count
			}
			sdat >>=1;												// Next Bit Set (LSB)
		}
		if( mod == 0 ){												// Odd Parity Add?(Y)
			if(( pcnt % 2 ) == 0 ){									// Not Odd?(Y)
				*dat |= 0x80;										// Odd Parity Set
			}
		}else{														// Even Parity Add?(Y)
			if(( pcnt % 2 ) != 0 ){									// Not Even?(Y)
				*dat |= 0x80;										// Even Parity Set
			}
		}
		dat++;														// Next Data
		siz--;														// Size -1
	}
}


//[]----------------------------------------------------------------------[]
///	@brief			���ʃp�^�[��X�J�n���Ԏ擾
//[]----------------------------------------------------------------------[]
///	@return			���ʃp�^�[��(1�`3)
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/01/17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
char get_timeptn(void)
{
	char ptn, num, i;
	short nowtime, settime1, settime2;
	
	nowtime = (short)CLK_REC.hour * 100 + (short)CLK_REC.minu;
	num = (char)prm_get(COM_PRM,S_SYS, 53, 1, 1);
	
	if (num <= 1) {
		// �ݒ萔��0��1�Ȃ�p�^�[��1�̉������Q�Ƃ���
		ptn = 1;
	} else {
		for (i = 0; i < num - 1; i++) {
			settime1 = (short)prm_get(COM_PRM, S_SYS, (short)(i + 54), 4, 1);		// 01-0054�`
			if (settime1 <= nowtime) {
				settime2 = (short)prm_get(COM_PRM, S_SYS, (short)(i + 55), 4, 1);
				if (nowtime < settime2) {
					ptn = (char)(i + 1);
					break;
				}
			}
		}
		if (i == num - 1) {
			ptn = num;
		}
	}
	
	return ptn;
}

//[]----------------------------------------------------------------------[]
///	@brief			���ʃp�^�[��X�̂Ƃ��̉��ʎ擾
//[]----------------------------------------------------------------------[]
///	@return			����
///	@param[in]		ptn: ���ʃp�^�[��X
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/01/17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
char get_anavolume(char ch, char ptn)
{
	char pos, vol;
	
	switch (ptn) {
	case 1:	pos = 5;	break;
	case 2:	pos = 3;	break;
	case 3:	pos = 1;	break;
	default:			return 0;
	}
	
	if(ch == 0){
		vol = (char)prm_get(COM_PRM, S_SYS, 51, 2, pos);
	}
	else{
		vol = (char)prm_get(COM_PRM, S_SYS, 49, 2, pos);
		if(vol == 99){
			vol = (char)prm_get(COM_PRM, S_SYS, 51, 2, pos);
		}
	}
	
	return vol;
}



//[]----------------------------------------------------------------------[]
///	@brief			�A�i�E���X���Z�b�g����
//[]----------------------------------------------------------------------[]
///	@return			void
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/04/02<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void an_reset(void)
{
	pre_volume[0]=0xff;
	pre_volume[1]=0xff;
	mts_req = (mts_req & ~MTS_BAVM1);
	MDLdata[MTS_AVM].mdl_size = 0;		// �A�i�E���X�v���N���A
}

//[]----------------------------------------------------------------------[]
///	@brief			AVM�ڑ�����
//[]----------------------------------------------------------------------[]
///	@return			1:�ڑ��L�� 0:�Ȃ�
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/05/13<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
char Is_AVM_connect(void)
{
	return use_avm;
}

/*[]-------------------------------------------------------------------[]*/
/*|	CRC�Z�o�֐�															|*/
/*[]-------------------------------------------------------------------[]*/
/*| MODULE NAME  : crc = CheckCRC8( *src, size );						|*/
/*| PARAMETER	 : unsigned char	*src;	: CRC�v�Z�f�[�^�A�h���X		|*/
/*|				 : unsigned char	size; 	: CRC�v�Z�f�[�^�T�C�Y		|*/
/*| RETURN VALUE : unsigned char	 crc;	: CRC�v�Z����				|*/
/*[]-------------------------------------------------------------------[]*/
unsigned char CheckCRC8( unsigned char *src, unsigned char size )
{
	unsigned char crc = 0xFF;
	unsigned char index;
	unsigned char  b;

	for( index = 0; index < size ; index++ )
	{
		crc ^= src[index];

		for( b = 0 ; b < 8; ++b )
		{
			if( crc & 0x80 ){
				crc = (unsigned char)((crc << 1) ^ 0x31);
			}else{
				crc = (unsigned char)(crc << 1);
			}
		}
	}
	return(crc);
}

/*[]-------------------------------------------------------------------[]*/
/*|	LRC�Z�o�֐�															|*/
/*[]-------------------------------------------------------------------[]*/
/*| MODULE NAME  : crc = CheckLRC8( *src, size );						|*/
/*| PARAMETER	 : unsigned char	*src;	: LRC�v�Z�f�[�^�A�h���X		|*/
/*|				 : unsigned char	size; 	: LRC�v�Z�f�[�^�T�C�Y		|*/
/*| RETURN VALUE : unsigned char	 crc;	: LRC�v�Z����				|*/
/*[]-------------------------------------------------------------------[]*/
unsigned char CheckLRC8( unsigned char *src, unsigned char size )
{
	unsigned char lrc;
	unsigned char index;

	lrc = 0;
	for( index = 0; index < size ; index++ )
	{
		lrc ^= src[index];
	}
	return(lrc);
}
//[]----------------------------------------------------------------------[]
///	@brief			���ꃁ�b�Z�[�W�쐬����
//[]----------------------------------------------------------------------[]
///	@param[in]		mode : �쐬���郁�b�Z�[�W�̎w��  AVM_RYOUKIN(�����ǂݏグ)
///	@return			ret : ���b�Z�[�W��
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/02/02<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
short an_msag_edit_rxm(short mode)
{
	short len;

	memset(an_msgbuf, 0, sizeof(an_msgbuf));// �����ǂݏグ�p���b�Z�[�W�ҏW�o�b�t�@
	len = 0;

	switch (mode) {
	case AVM_RYOUKIN:			// �����ǂݏグ
		// �����ݒ�
		len = Make_FeeString(announceFee, &(an_msgbuf[0]));
		break;
	case AVM_SHASHITU:			// �Ԏ��ǂݏグ
		// �����ݒ�
		len = Make_ShashituString(key_num, &(an_msgbuf[0]));
		break;
	}

		
	return len;
}
//[]----------------------------------------------------------------------[]
///	@brief			�ǂݏグ����������쐬
//[]----------------------------------------------------------------------[]
///	@return			void
///	@param[in]		fee		: ����
///	@param[in]		*anndata: ���ߓo�^�p������
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/01/17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static short Make_FeeString(const unsigned long fee, unsigned short * anndata)
{
	short digit[6];	// �e���̐��l [0]=1���ڂ̐��l,[1]=2����...
	char flag[5];	// 1�`n�������ׂ�0�Ȃ�0���Z�b�g�A1�ł�0�ȊO�Ȃ�1���Z�b�g
	short i, j, pos = 0;
	unsigned long copy;
	char ret;
	
	copy = fee;

	if(0 == prm_get( COM_PRM,S_SYS, 47, 1, 2 )){		// �Ԏ��ǂݏグ�Ȃ�
		anndata[pos++] = AVM_RYOUKIN_HA;
	}

	// �e���̐��l�擾
	for (i = 0; i < 6; i++) {
		digit[i] = (short)(copy % 10);
		copy /= 10;
	}

	// 1�`n���ڂ�0���ǂ����̔���
	for (i = 0, j = 0; i < 5; i++) {
		// 1���ڂ���a���Ƃ��Ă���
		j += digit[i];
		flag[i] = (j > 0);	// [0]=1����,[1]=1�`2����,[2]=1�`3���ځ`,5���܂Ō���̂�i<5
	}

	// �\���̈ʓo�^
	ret = set_digit6(anndata, digit[5], flag[4], &pos);
	
	// ���̈ʓo�^
	if (ret == 1) {
		ret = set_digit5(anndata, digit[5], digit[4], flag[3], &pos);
	} else {
		goto END;
	}
	
	// ��̈ʓo�^
	if (ret == 1) {
		ret = set_digit4(anndata, digit[3], flag[2], &pos);
	} else {
		goto END;
	}
	
	// �S�̈ʓo�^
	if (ret == 1) {
		ret = set_digit3(anndata, digit[2], flag[1], &pos);
	} else {
		goto END;
	}
	
	// �\���̈ʖړo�^
	if (ret == 1) {
		set_digit2(anndata, digit[1], digit[0], &pos);
	} else {
		goto END;
	}
END:
	return pos;
}


//[]----------------------------------------------------------------------[]
///	@brief			�����\���̈ʉ����ݒ�
//[]----------------------------------------------------------------------[]
///	@return			1		: �����ݒ�p��<br>
///					0		: �����ݒ�I��<br>
///	@param[in]		*msg	: ���ߓo�^�p������
///	@param[in]		dig		: 6���ڂ̒l
///	@param[in]		flg		: 1�`5���ڂ����ׂ�0���ǂ����̔���t���O
///	@param[in]		*p		: *msg�̃|�C���^
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/01/17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static char set_digit6(unsigned short * msg, short dig, char flg, short * p)
{
	short num;
	
	if (dig > 0) {
		// 6���ڂ�0�łȂ��Ƃ�
		if (flg == 0) {
			// 1�`5���ڂ����ׂ�0
			if (dig != 1) {
				// 6���ڂ�1���傫��
				// "n�W���E"Fee2nc.wav
				num = ANN_JUU + 2 + ((dig-1) * 3);
				msg[(*p)++] = num;
				// "�}��"Fee50a.wav
				msg[(*p)++] = ANN_MAN;
				// "�G���f�X"Fee9xa.wav
				msg[(*p)++] = ANN_YEN_DESU;
			} else {
				// "�W���E"Fee21b.wav
				msg[(*p)++] = ANN_JUU+1;
				// "�}��"Fee50b.wav
				msg[(*p)++] = ANN_MAN+1;
				// �G���f�XFee9xb.wav
				msg[(*p)++] = ANN_YEN_DESU+1;
			}
			return 0;
		}
	}
	
	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief			�������̈ʉ����ݒ�
//[]----------------------------------------------------------------------[]
///	@return			1		: �����ݒ�p��<br>
///					0		: �����ݒ�I��<br>
///	@param[in]		*msg	: ���ߓo�^�p������
///	@param[in]		dig6	: 6���ڂ̒l
///	@param[in]		dig5	: 5���ڂ̒l
///	@param[in]		flg		: 1�`4���ڂ����ׂ�0���ǂ����̔���t���O
///	@param[in]		*p		: *msg�̃|�C���^
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/01/17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static char set_digit5(unsigned short * msg, short dig6, short dig5, char flg, short * p)
{
	short num;
	
	if (dig5 > 0) {
		// 5���ڂ�1�ȏ�̂Ƃ�
		if (dig6 > 0) {
			// 6���ڂ�0���傫��
			// "n�W���E"Fee2mb.wav
			num = ANN_JUU + 1 + ((dig6-1) * 3);
			msg[(*p)++] = num;
		}
		
		if (flg != 0) {
			// 1�`4���ڂ�0�łȂ�
			// "n�}��"Fee5na.wav
			num = ANN_MAN + (dig5 * 2);				// ���̌��́u�C�`�}���v�̉��������邽��-1����K�v�Ȃ�
			msg[(*p)++] = num;
		} else {
			// "n�}��"Fee5nb.wav
			num = ANN_MAN + 1 + (dig5 * 2);			// ���̌��́u�C�`�}���v�̉��������邽��-1����K�v�Ȃ�
			msg[(*p)++] = num;
			// "�G���f�X"Fee9xb.wav
			msg[(*p)++] = ANN_YEN_DESU+1;
			return 0;
		}
	} else {
		// 5���ڂ�0�̂Ƃ�
		if (dig6 != 0) {
			// 6���ڂ�0�łȂ�
			// "n�W���E"Fee2nc.wav
			num = ANN_JUU + 2 + ((dig6-1) * 3);
			msg[(*p)++] = num;
			// "�}��"Fee50a.wav
			msg[(*p)++] = ANN_MAN;
		}
	}
	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief			������̈ʉ����ݒ�
//[]----------------------------------------------------------------------[]
///	@return			1		: �����ݒ�p��<br>
///					0		: �����ݒ�I��<br>
///	@param[in]		*msg	: ���ߓo�^�p������
///	@param[in]		dig		: 4���ڂ̒l
///	@param[in]		flg		: 1�`3���ڂ����ׂ�0���ǂ����̔���t���O
///	@param[in]		*p		: *msg�̃|�C���^
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/01/17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static char set_digit4(unsigned short * msg, short dig, char flg, short * p)
{
	short num;
	
	if (dig > 0) {
		// 4���ڂ�1�ȏ�̂Ƃ�
		if (flg != 0) {
			// 1�`3���ڂ�0�łȂ�
			// "n�Z��"Fee4na.wav
			if (dig > 1) {
				num = ANN_SEN + (((dig-1) * 2) - 1);
			}else{
				num = ANN_SEN;
			}
			msg[(*p)++] = num;
		} else {
			if (dig > 1) {
				// 4���ڂ�1�łȂ�
				// "n�Z��"Fee4nb.wav
				num = ANN_SEN + 1 + (((dig-1) * 2) - 1);
				msg[(*p)++] = num;
				// "�G���f�X"Fee9xb.wav
				msg[(*p)++] = ANN_YEN_DESU+1;
			} else {
				// "�Z��"Fee41a.wav
				msg[(*p)++] = ANN_SEN;
				// "�G���f�X"Fee9xa.wav
				msg[(*p)++] = ANN_YEN_DESU;
			}
			return 0;
		}
	}
	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief			�����S�̈ʉ����ݒ�
//[]----------------------------------------------------------------------[]
///	@return			1		: �����ݒ�p��<br>
///					0		: �����ݒ�I��<br>
///	@param[in]		*msg	: ���ߓo�^�p������
///	@param[in]		dig		: 3���ڂ̒l
///	@param[in]		flg		: 1�`2���ڂ����ׂ�0���ǂ����̔���t���O
///	@param[in]		*p		: *msg�̃|�C���^
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/01/17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static char set_digit3(unsigned short * msg, short dig, char flg, short * p)
{
	short num;
	char wk = 0;			/* �S�̌���3,4,7,9��2��ނ���̂ŁA�����ԍ������p�Ɏg�p����ϐ� */
	
	if (dig > 0) {
		// 3���ڂ�1�ȏ�̂Ƃ�
		if (flg != 0) {
			// 1�`2���ڂ�0�łȂ�
			// "n�q���N"Fee3na.wav
			switch (dig) {
			case 1:	
				// "�q���N"Fee3na.wav
				msg[(*p)++] = ANN_HYAKU;
				break;
			case 9:
			case 8:
				wk++;
			case 7:
			case 6:
			case 5:
				wk++;
			case 4:
				wk++;
			case 3:
			case 2:	
				// "n�q���N"Fee3na.wav
				num = ANN_HYAKU + ((dig-1) + wk);
				msg[(*p)++] = num;
				break;
			}
		} else {
			switch (dig) {
			case 1:	
				// "�q���N"Fee3na.wav
				msg[(*p)++] = ANN_HYAKU;
				// "�G���f�X"Fee9xb.wav
				msg[(*p)++] = ANN_YEN_DESU+1;
				break;
			case 8:
				wk++;
			case 6:
			case 5:
				wk+=2;
			case 2:
				// "n�q���N"Fee3na.wav
				num = ANN_HYAKU + ((dig-1) + wk);
				msg[(*p)++] = num;
				// "�G���f�X"Fee9xa.wav
				msg[(*p)++] = ANN_YEN_DESU;
				break;
			case 9:
				wk++;
			case 7:
				wk++;
			case 4:
				wk++;
			case 3:
				// "n�q���N"Fee3nb.wav
				num = ANN_HYAKU + ((dig-1) + wk) + 1;
				msg[(*p)++] = num;
				// "�G���f�X"Fee9xa.wav
				msg[(*p)++] = ANN_YEN_DESU;
				break;
			}
			return 0;
		}
	}
	
	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief			������̈ʁE�\�̈ʉ����ݒ�
//[]----------------------------------------------------------------------[]
///	@return			1		: �����ݒ�p��<br>
///					0		: �����ݒ�I��<br>
///	@param[in]		*msg	: ���ߓo�^�p������
///	@param[in]		dig2	: 2���ڂ̒l
///	@param[in]		dig1	: 1���ڂ̒l
///	@param[in]		*p		: *msg�̃|�C���^
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/01/17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static char set_digit2(unsigned short * msg, short dig2, short dig1, short * p)
{
	short num;
	
	if (dig2 > 0) {
		// 2���ڂ�1�ȏ�̂Ƃ�
		if (dig1 > 0) {
			// 1���ڂ�0�łȂ�
			// "n�W���E"Fee2nb.wav
			num = ANN_JUU + 1 + ((dig2-1) * 3);
			msg[(*p)++] = num;
		} else {
			// "n�W���E"Fee2na.wav
			num = ANN_JUU + ((dig2-1) * 3);
			msg[(*p)++] = num;
			if (dig2 != 1) {
				// 2���ڂ�1�łȂ�
				// "�G���f�X"Fee9xa.wav
				msg[(*p)++] = ANN_YEN_DESU;
			} else {
				// "�G���f�X"Fee9xb.wav
				msg[(*p)++] = ANN_YEN_DESU+1;
			}
			return 0;
		}
	}
	num = dig1;
	msg[(*p)++] = num;
	// "�G���f�X"Fee9xa.wav
	msg[(*p)++] = ANN_YEN_DESU;
	
	return 1;
}
//[]----------------------------------------------------------------------[]
///	@brief			�Ԏ��ǂݏグ������쐬
//[]----------------------------------------------------------------------[]
///	@return			void
///	@param[in]		car_no		: �Ԏ��ԍ�(��ʕ\������ԍ�)
///	@param[in]		*anndata: ���ߓo�^�p������
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/02/13<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static short Make_ShashituString(const unsigned short car_no, unsigned short * anndata)
{

	short digit[4];	// �e���̐��l [0]=1���ڂ̐��l,[1]=2����,[2]=3����
	char flag[4];	// 1�`n�������ׂ�0�Ȃ�0���Z�b�g�A1�ł�0�ȊO�Ȃ�1���Z�b�g
	short i, j, posi;
	unsigned short copy;
	char ret;
	
	copy = car_no;
	posi = 0;
	if(0 == prm_get( COM_PRM,S_SYS, 47, 1, 1 )){		// �����ǂݏグ�Ȃ�
		anndata[posi++] = AVM_SHASHITU_HA;		// �u�Ԏ��ԍ��́v
		Shashitu_EndStr = AVM_BANGOU_2;
	}
	else{
		anndata[posi++] = AVM_SHASHITU_BAN;		// �u�Ԏ��ԍ��v
		Shashitu_EndStr = AVM_BANGOU_1;
	}

	if(car_no == 0){// �Ԏ��ԍ�0�Ԃ͑��݂��Ȃ�
		return (short)0;
	}else{// �Ԏ��ԍ�31�Ԃ���͐�����g�ݍ��킹�ĉ����𐶐�����
		
		// �e���̐��l�擾
		for (i = 0; i < 4; i++) {
			digit[i] = (short)(copy % 10);
			copy /= 10;
		}

		// 1�`n���ڂ�0���ǂ����̔���
		for (i = 0, j = 0; i < 4; i++) {
			// 1���ڂ���a���Ƃ��Ă���
			j += digit[i];
			flag[i] = (j > 0);	//	[0]=1����,[1]=1�`2����,[2]=1�`3���ڂ܂Ō���̂�i<3
		}

		// ��̈ʓo�^
		ret = set_digit4_shashitu(anndata, digit[3], flag[2], &posi);
		if(ret == 1){
			// �S�̈ʓo�^
			ret = set_digit3_shashitu(anndata, digit[2], flag[1], &posi);
		}
	
		// �\���̈ʖړo�^
		if (ret == 1) {
			set_digit2_shashitu(anndata, digit[1], digit[0], &posi);
		}
	}
	return posi;
}


static char set_digit4_shashitu(unsigned short * msg, short dig, char flg, short * p)
{
	short num;
	
	if (dig > 0) {
		// 4���ڂ�1�ȏ�̂Ƃ�
		if (flg != 0) {
			// 1�`3���ڂ�0�łȂ�
			// "n�Z��"Fee4na.wav
			if (dig > 1) {
				num = ANN_SEN + (((dig-1) * 2) - 1);
			}else{
				num = ANN_SEN;
			}
			msg[(*p)++] = num;
		} else {
			if (dig > 1) {
				// 4���ڂ�1�łȂ�
				// "n�Z��"Fee4nb.wav
				num = ANN_SEN + 1 + (((dig-1) * 2) - 1);
				msg[(*p)++] = num;
				// "�G���f�X"Fee9xb.wav
				msg[(*p)++] = Shashitu_EndStr;
			} else {
				// "�Z��"Fee41a.wav
				msg[(*p)++] = ANN_SEN;
				// "�G���f�X"Fee9xb.wav
				msg[(*p)++] = Shashitu_EndStr;
			}
			return 0;
		}
	}
	return 1;
	
}
//[]----------------------------------------------------------------------[]
///	@brief			�Ԏ��S�̈ʉ����ݒ�
//[]----------------------------------------------------------------------[]
///	@return			1		: �Ԏ��ݒ�p��<br>
///					0		: �Ԏ��ݒ�I��<br>
///	@param[in]		*msg	: ���ߓo�^�p������
///	@param[in]		dig		: 3���ڂ̒l
///	@param[in]		flg		: 1�`2���ڂ����ׂ�0���ǂ����̔���t���O
///	@param[in]		*p		: *msg�̃|�C���^
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/02/13<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static char set_digit3_shashitu(unsigned short * msg, short dig, char flg, short * p)
{
	short num;
	char wk = 0;			/* �S�̌���3,4,7,9��2��ނ���̂ŁA�����ԍ������p�Ɏg�p����ϐ� */
	
	if (dig > 0) {
		// 3���ڂ�1�ȏ�̂Ƃ�
		if (flg != 0) {
			// 1�`2���ڂ�0�łȂ�
			// "n�q���N"Fee3na.wav
			switch (dig) {
			case 1:	
				// "�q���N"Fee3na.wav
				msg[(*p)++] = ANN_HYAKU;
				break;
			case 9:
			case 8:
				wk++;
			case 7:
			case 6:
			case 5:
				wk++;
			case 4:
				wk++;
			case 3:
			case 2:	
				// "n�q���N"Fee3na.wav
				num = ANN_HYAKU + ((dig-1) + wk);
				msg[(*p)++] = num;
				break;
			}
		} else {
			switch (dig) {
			case 1:	
				// "�q���N"Fee3na.wav
				msg[(*p)++] = ANN_HYAKU;
				// "�o���f�X"Fee9xb.wav
				msg[(*p)++] = Shashitu_EndStr;
				break;
			case 8:
				wk++;
			case 6:
			case 5:
				wk+=2;
			case 2:
				// "n�q���N"Fee3na.wav
				num = ANN_HYAKU + ((dig-1) + wk);
				msg[(*p)++] = num;
				// "�o���f�X"Fee9xa.wav
				msg[(*p)++] = Shashitu_EndStr;
				break;
			case 9:
				wk++;
			case 7:
				wk++;
			case 4:
				wk++;
			case 3:
				// "n�q���N"Fee3nb.wav
				num = ANN_HYAKU + ((dig-1) + wk) + 1;
				msg[(*p)++] = num;
				// "�o���f�X"Fee9xa.wav
				msg[(*p)++] = Shashitu_EndStr;
				break;
			}
			return 0;
		}
	}
	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief			�Ԏ���̈ʁE�\�̈ʉ����ݒ�
//[]----------------------------------------------------------------------[]
///	@return			1		: �Ԏ��ݒ�p��<br>
///					0		: �Ԏ��ݒ�I��<br>
///	@param[in]		*msg	: ���ߓo�^�p������
///	@param[in]		dig2	: 2���ڂ̒l
///	@param[in]		dig1	: 1���ڂ̒l
///	@param[in]		*p		: *msg�̃|�C���^
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/02/13<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static char set_digit2_shashitu(unsigned short * msg, short dig2, short dig1, short * p)
{
	short num;
	
	if (dig2 > 0) {
		// 2���ڂ�1�ȏ�̂Ƃ�
		if (dig1 > 0) {
			// 1���ڂ�0�łȂ�
			// "n�W���E"Fee2nb.wav
			num = ANN_JUU + 1 + ((dig2-1) * 3);
			msg[(*p)++] = num;
		} else {
			// "n�W���E"Fee2na.wav
			num = ANN_JUU + ((dig2-1) * 3);
			msg[(*p)++] = num;
			if (dig2 != 1) {
				// 2���ڂ�1�łȂ�
				// "�o���f�X"
				msg[(*p)++] = Shashitu_EndStr;
			} else {
				// "�o���f�X"
				msg[(*p)++] = Shashitu_EndStr;
			}
			return 0;
		}
	}
	// "n"Fee1na.wav
	num = dig1;
	msg[(*p)++] = num;
	// "�o���f�X"
	msg[(*p)++] = Shashitu_EndStr;
	
	return 1;
}


//[]----------------------------------------------------------------------[]
///	@brief			Sodiac�R�[���o�b�N�֐�
//[]----------------------------------------------------------------------[]
/// @parameter		unsigned short ch 	:	�Đ��Ώ�ch
///					D_SODIAC_ID	   id 	: 	�ʒm���
///					D_SODIAC_E 	   err 	: 	��������
///	@return			void
///	@author			Tanaka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/02/03
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void avm_sodiac_callback( int ch , D_SODIAC_ID id , D_SODIAC_E err )
{
	unsigned char	ptr;										//read�|�C���^����p
	unsigned char	resend_count;
	unsigned char	message_num;
	unsigned char 	ptr_tmp;
	unsigned char 	next_message_num;
	unsigned char	que_data[2];
	

	/* ch�`�F�b�N */
	/*	���������G���[����̏ꍇ�͗v���������s��Ȃ� */
	if( SODIAC_ERR_NONE != Avm_Sodiac_Err_flg )
	{
		return;
	}

	/* read_ptr��write_ptr�������ꍇ	*/
	if( D_SODIAC_ID_STOP == id || D_SODIAC_ID_ERROR == id  )
	{
		ptr	= AVM_Sodiac_Ctrl[ch].read_ptr;
	}

	switch( id )												/*	�ʒm��ʂɂ�蕪��				�@	*/
	{															/*                                    	*/

		case D_SODIAC_ID_STOP:									/*  ������~�̏ꍇ                    	*/
			if( AVM_Sodiac_Ctrl[ch].play_cmp == 0 && AVM_Sodiac_Ctrl[ch].stop == 0){
#if 1
				resend_count = AVM_Sodiac_Ctrl[ch].resend_count_now;
				message_num = AVM_Sodiac_Ctrl[ch].sd_req[ptr].message_num;
				/* �����Đ��v���I������	*/
				next_message_num = AVM_Sodiac_NextMessageNum( ch, ptr );
				/* ���̕��߂��������b�Z�[�W�ԍ��̏ꍇ�������͌J��Ԃ��񐔂��c���Ă���ꍇ	*/
                if( (message_num == next_message_num) || (0x01 != resend_count)) {
					if( message_num == next_message_num ) {
						AVM_Sodiac_ReadPtrInc( ch, ptr);												/* read�|�C���^�C���N�������g					*/
					} else {
						//�������b�Z�[�W�ԍ����T�[�`���ē���̔ԍ�������΂���readptr��Ԃ�
						ptr_tmp = AVM_Sodiac_EqualReadPtr( ch, ptr, message_num );
						if( ptr != ptr_tmp ) {
							AVM_Sodiac_Ctrl[ch].read_ptr = ptr_tmp;
						}
						/* �����J��Ԃ��ȊO�̏ꍇ	*/
						if( (0x00 != resend_count) && (0x0F != resend_count )) {
							AVM_Sodiac_Ctrl[ch].resend_count_now--;
						}
					}
					que_data[0] = ch;
					que_data[1] = ptr = AVM_Sodiac_Ctrl[ch].read_ptr;
					AVM_Sodiac_Ctrl[ch].play_message_cmp = 1;
					queset( OPETCBNO, SODIAC_NEXT_REQ, sizeof(que_data), &que_data );	

					/*	������J��Ԃ��Đ��łȂ��ꍇ�������͎��ɍĐ��\��̃��b�Z�[�W�ԍ����قȂ�ꍇ	*/
					return;
				}
				AVM_Sodiac_EqualMsgBuffClear( ch, ptr, message_num );										/* �������b�Z�[�W�ԍ��̗v��	*/
																											/* �o�b�t�@���N���A����B	*/
				/* �Đ�������read�|�C���^�C���N�������g */	
				AVM_Sodiac_ReadPtrInc( ch, ptr);															/* read�|�C���^�C���N�������g					*/
				/* ���̍Đ��v�����c���Ă���ꍇ	*/
				ptr = AVM_Sodiac_Ctrl[ch].read_ptr;
				if( ptr != AVM_Sodiac_Ctrl[ch].write_ptr) {
					que_data[0] = ch;
					que_data[1] = ptr;
					AVM_Sodiac_Ctrl[ch].play_message_cmp = 1;
					queset( OPETCBNO, SODIAC_NEXT_REQ, sizeof(que_data), &que_data );	
					AVM_Sodiac_Ctrl[ch].resend_count_now	= AVM_Sodiac_Ctrl[ch].sd_req[ptr].resend_count;	/* �c��đ��񐔐ݒ�					*/
				} else {
					if(avm_alarm_flg && ch == 1){															/* �x�񔭖C��						*/
						avm_alarm_flg = 0;																	/* �x��t���Ooff					*/
					}
					queset( OPETCBNO, SODIAC_PLAY_CMP, 0, 0 );	
					AVM_Sodiac_Ctrl[ch].play_cmp = 1;														/*  �����Đ���~���t���OON�i��~���j*/
					AVM_Sodiac_Ctrl[ch].play_message_cmp = 1;
				}
#endif
			} else {
			}
			break;
		case D_SODIAC_ID_ERROR:
			/*	���g���C���M�񐔂��܂��c���Ă���ꍇ	*/
			if( 0 != AVM_Sodiac_Ctrl[ch].retry_count )
			{
				/* ���M���s�G���[�o�^���s��	*/
				AVM_Sodiac_Err_Chk( err, D_SODIAC_ID_ERROR );	/* �G���[�o�^			*/
				que_data[0] = ch;
				que_data[1] = ptr = AVM_Sodiac_Ctrl[ch].read_ptr;
				AVM_Sodiac_Ctrl[ch].play_message_cmp = 1;
				queset( OPETCBNO, SODIAC_NEXT_REQ, sizeof(que_data), &que_data );	/* �����Đ��v������	*/
				AVM_Sodiac_Ctrl[ch].retry_count--;
				return;
			}
			else
			{
				/*�@���g���C�G���[�o�^�@		*/
				AVM_Sodiac_Err_Chk((D_SODIAC_E)ERR_SODIAC_RETRY_ERR, D_SODIAC_ID_ERROR);
				// �����Đ��������Ƃ��Z�}�t�H���������
				AVM_Sodiac_Ctrl[0].stop = 0;
				AVM_Sodiac_Ctrl[1].stop = 0;
				AVM_Sodiac_Ctrl[0].play_message_cmp = 1;
				AVM_Sodiac_Ctrl[1].play_message_cmp = 1;
				AVM_Sodiac_SemFree();
			}
			break;
		case D_SODIAC_ID_MUTEOFF:
			/* MUTE���W�X�^�ݒ� */
			SODIAC_MUTE_SW	=	1;									//MUTE����
			return;
		case D_SODIAC_ID_MUTEON:
			SODIAC_MUTE_SW	=	0;									//MUTE�ݒ�
		case D_SODIAC_ID_PWMON:
			break;
		case D_SODIAC_ID_PWMOFF:
			AVM_Sodiac_Ctrl[0].stop = 0;
			AVM_Sodiac_Ctrl[1].stop = 0;
			AVM_Sodiac_Ctrl[0].play_message_cmp = 1;
			AVM_Sodiac_Ctrl[1].play_message_cmp = 1;
			AVM_Sodiac_SemFree();							//	DebugNo39 arex�l�w��
		break;
		case D_SODIAC_ID_NEXT:
			break;
		default:
			return;
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			Sodiac����������
//[]----------------------------------------------------------------------[]
///	@return			void
///	@author			Tanaka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/02/03
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void	AVM_Sodiac_Init( void )
{
	D_SODIAC_E	err;

	/* MUTE���W�X�^�ݒ� */
	/* �|�[�g�̐ݒ� */
	PORT0.PMR.BIT.B2 = 0;												/* P02 */
	PORT0.PDR.BIT.B2 = 1;												/* �A���v�R���g���[�� �o�� */

	memset( &AVM_Sodiac_Ctrl[0], 0x00, sizeof(AVM_SODIAC_CTRL)*2 );		//sodiac�Ǘ��̈揉����
	Sodiac_FROM_Get 			= 0;									// �����f�[�^FROM���擾
	Avm_Sodiac_Err_flg			= 0x00;
	//�A���v�ݒ�(�����͎d�l���킩��܂ŉ��ݒ�)

	err = sodiac_amp_ctrl( 75,1,1);
	if( D_SODIAC_E_OK != err )
	{
		Avm_Sodiac_Err_flg |= SODIAC_AMP_ERR;							//�ُ�I���Ȃ̂ňȌ��SodiacAPI�̓R�[�����Ȃ�
		AVM_Sodiac_Err_Chk( err, ERR_SOIDAC_NOT_ID );
		return;
	}

	/* Sodiac�~�h���E�F�A�̏����� �^�C�}�[���� PCLKB=50MHz */
	err = sodiac_init(50000000UL);
	if( D_SODIAC_E_OK != err )
	{
		Avm_Sodiac_Err_flg |= SODIAC_INIT_ERR; 							//�ُ�I���Ȃ̂ňȌ��SodiacAPI�̓R�[�����Ȃ�
		AVM_Sodiac_Err_Chk( err, ERR_SOIDAC_NOT_ID );
		use_avm = AVM_NOT_CONNECT;
		return;
	}
		
	err = sodiac_regist_eventhook(avm_sodiac_callback);
	if( D_SODIAC_E_OK != err )
	{
		Avm_Sodiac_Err_flg |= SODIAC_EVEHOOK_ERR;						//�ُ�I���Ȃ̂ňȌ��SodiacAPI�̓R�[�����Ȃ�
		AVM_Sodiac_Err_Chk( err, ERR_SOIDAC_NOT_ID );
		use_avm = AVM_NOT_CONNECT;
		return;
	}
	use_avm = AVM_CONNECT;												// AVM�ڑ��L��
	AVM_Sodiac_Ctrl[0].play_cmp = 1;									// �����Đ���~���t���OON�i��~���j
	AVM_Sodiac_Ctrl[1].play_cmp = 1;
	AVM_Sodiac_Ctrl[0].play_message_cmp = 1;
	AVM_Sodiac_Ctrl[1].play_message_cmp = 1;

	ope_anm( AVM_TEST );												// �i�ް�ޮݗv���j���M
	return;
}
//[]----------------------------------------------------------------------[]
///	@brief			�����Đ��v������
//[]----------------------------------------------------------------------[]
///	@return			void
///	@author			Tanaka
/// @parameter		unsigned short ch 	:	�Đ��Ώ�ch
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/02/07
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void	AVM_Sodiac_Execute( unsigned short ch )
{
	D_SODIAC_E		err;
	unsigned char	que_data[2];
	unsigned char	ptr;																			/* read�|�C���^�i�[�p				*/
	unsigned short*	volume;
	unsigned short	ptn;
	unsigned short	ch1_vol;

	ptr	= AVM_Sodiac_Ctrl[ch].read_ptr;
	volume = &AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm.volume;
	if( AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm.num != AVM_BOO ){
		/*	���Z���ł���Γ������ʂŖ炷���߂Ɍ��݂̉��ʒl��ݒ肷��	*/
		if( TRUE == AVM_Sodiac_PayOff_Judge() )
		{
			if( Avm_Sodiac_PayOff_Volume[ch] != (*volume))
			{
				if( Avm_Sodiac_PayOff_Volume[ch] != 0xFFFF ){
					(*volume) = Avm_Sodiac_PayOff_Volume[ch];
				}
			}
		}
		/*	�񐸎Z���ł���ΐ��Z���ɓ������ʂŖ炷���߂ɍĐ��ݒ艹�ʒl��ۑ�����	*/
		else
		{
			/* �����Đ���~���t���OON(��~��) 	*/
			if( 1 == AVM_Sodiac_Ctrl[ch].play_cmp )
			{
				Avm_Sodiac_PayOff_Volume[ch] = (*volume);
				if( ch == 0 ){
					/* �񐸎Z����CH1�̉��ʂ�ۑ����� */
					ptn = get_timeptn();
					ch1_vol = get_anavolume(1, (uchar)ptn);
					if(ch1_vol == 0){
						Avm_Sodiac_PayOff_Volume[1] = 0;
					}
					else if(ch1_vol == 99){
						Avm_Sodiac_PayOff_Volume[1] = Avm_Sodiac_PayOff_Volume[ch];
					}
					else if(ch1_vol > 15){
						Avm_Sodiac_PayOff_Volume[1] = 1;
					}
					else{
						Avm_Sodiac_PayOff_Volume[1] = 15 - (ch1_vol - 1);
					}
				}
			}
		}
	}

	if( AVM_Sodiac_SemGet() ){																			/*	�A�N�Z�X���擾					*/
		return;
	}

	if(AVM_Sodiac_Ctrl[ch].sd_req[ptr].message_num != 0) {
		err = sodiac_execute( &AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm );							/* �����Đ��v�����M					*/
	}
	else {
		// message_num��0�̏ꍇ�͍Đ����Ȃ�
		err = D_SODIAC_E_OTHER;
	}
	if( D_SODIAC_E_OK != err )																		/* 									*/
	{
		AVM_Sodiac_SemFree();																		/* �A�N�Z�X���J��					*/
		if(AVM_Sodiac_Ctrl[ch].sd_req[ptr].message_num != 0) {
			AVM_Sodiac_Err_Chk( err, ERR_SOIDAC_NOT_ID );											/*	�G���[�o�^						*/
		}

		memset( &AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm, 0x00, sizeof(AVM_SODIAC_REQ));			/* �v���N���A						*/
		AVM_Sodiac_ReadPtrInc( ch, ptr);															/* read�|�C���^�C���N�������g	  	*/
		ptr = AVM_Sodiac_Ctrl[ch].read_ptr;

		/* ���̍Đ��v�����c���Ă���ꍇ	*/
		if( ptr != AVM_Sodiac_Ctrl[ch].write_ptr)
		{

			que_data[0] = ch;
			que_data[1] = ptr;
			queset( OPETCBNO, SODIAC_NEXT_REQ, sizeof(que_data), &que_data );
			AVM_Sodiac_Ctrl[ch].resend_count_now	= AVM_Sodiac_Ctrl[ch].sd_req[ptr].resend_count;	/* �c��đ��񐔐ݒ�					*/
		}
		/* ���̍Đ��v�����c���Ă��Ȃ��ꍇ*/
		else
		{
			if(avm_alarm_flg && ch == 1){															/* �x�񔭖C��						*/
			avm_alarm_flg = 0;																		/* �x��t���Ooff					*/
			}
			AVM_Sodiac_Ctrl[ch].play_cmp = 1; 														/* �����Đ���~���t���OON(��~��) 	*/
			AVM_Sodiac_Ctrl[ch].play_message_cmp = 1;
		}
	}
	else
	{
		AVM_Sodiac_Ctrl[ch].play_cmp = 0; 															/* �����Đ���~���t���OOFF(�Đ���) */
		AVM_Sodiac_Ctrl[ch].play_message_cmp = 0;
	}																								/*  							   */
	return;
}
//[]----------------------------------------------------------------------[]
///	@brief			Sodiac�Đ��C���^�[�o����ѱ�ď���
//[]----------------------------------------------------------------------[]
///	@return			void
///	@author			Tanaka
/// @parameter		unsigned short ch 	:	�Đ��Ώ�ch
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/02/07
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void	AVM_Sodiac_Play_Wait_Tim( unsigned short ch )
{

	Lagcan( OPETCBNO, 22 + ch );
	AVM_Sodiac_Execute( ch );											/* �����Đ��v������			*/
	return;
}
//[]----------------------------------------------------------------------[]
///	@brief			Sodiac�G���[�o�^����
//[]----------------------------------------------------------------------[]
///	@return			void
///	@author			Tanaka
/// @parameter		D_SODIAC_E	err 			:	�G���[���
/// @parameter		unsigned char notice_id 	:	�ʒm���	
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/02/07
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void	AVM_Sodiac_Err_Chk( D_SODIAC_E err, unsigned char notice_id )
{
	long	err_inf;
	err_inf = (long)notice_id;	
	if( ERR_SODIAC_RETRY_ERR == err )
	{
		err_chk2( ERRMDL_SODIAC, (char)err, 2, 2, 0, &err_inf );					// �G���[�o�^�E�������������E��񂠂�(bin)�E�`�F�b�N����
	}
	else if( D_SODIAC_E_STATE == err) {												// SodiacAPI����Ԉُ��Ԃ���
		// NOTE:	�Z���^�[E2509�̃G���[���ʒm�����̂�}�~���鉼�΍�
		//			SodiacAPI(sodiac_execute)����Ԉُ��Ԃ��ꍇ�̓G���[�o�^���Ȃ�
		;
	}
	else
	{
		err_chk2( ERRMDL_SODIAC, (char)(err+5), 2, 2, 0, &err_inf );				// �G���[�o�^�E�������������E��񂠂�(bin)�E�`�F�b�N����
	}
	return;
}
//[]----------------------------------------------------------------------[]
///	@brief			Sodiac�Đ��p�o�b�t�@�̎��̃��b�Z�[�W�ԍ����擾���鏈��
//[]----------------------------------------------------------------------[]
///	@return			unsigned char				:�Đ����o�b�t�@��
///												���̃��b�Z�[�W�ԍ�
///	@author			Tanaka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/02/07
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
unsigned char	AVM_Sodiac_NextMessageNum( unsigned short ch,	unsigned char ptr)
{
	/* �|�C���^�C���N�������g */	
	if( ptr >= (AVM_REQ_BUFF_SIZE -1) )
	{
		ptr = 0;
	}
	else
	{
		ptr++;
	}

	/* ���̃o�b�t�@�̃��b�Z�[�W�ԍ����擾���� */
	return AVM_Sodiac_Ctrl[ch].sd_req[ptr].message_num;
}
//[]----------------------------------------------------------------------[]
///	@brief			Sodiac�Đ��p�o�b�t�@�Ɠ������b�Z�[�W�ԍ���read�|�C���^��Ԃ�����
//[]----------------------------------------------------------------------[]
///	@return			unsigned char			:�Đ����o�b�t�@�Ɠ���
///											���b�Z�[�W�ԍ���read�|�C���^
///	@author			Tanaka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/02/07
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
unsigned char	AVM_Sodiac_EqualReadPtr( unsigned short ch,	unsigned char  ptr, unsigned char message_num )
{
	unsigned char count;
	
	for( count = 0; count < AVM_REQ_BUFF_SIZE ; count++ )
	{
		/* �|�C���^�C���N�������g */	
		if( ptr >= (AVM_REQ_BUFF_SIZE -1) )
		{
			ptr = 0;
		}
		else
		{
			ptr++;
		}
		/* �������b�Z�[�W�ԍ��̃o�b�t�@���T�[�`���� */
		if( message_num == AVM_Sodiac_Ctrl[ch].sd_req[ptr].message_num )
		{
			return ptr;		//�������b�Z�[�W�ԍ���read�|�C���^��Ԃ�
		}
	}
	/* �����ɂ͂��Ȃ��͂� */
	return ptr;
}
//[]----------------------------------------------------------------------[]
///	@brief			Sodiac�Đ��p�o�b�t�@��read�|�C���^�̃C���N�������g����
//[]----------------------------------------------------------------------[]
///	@return			unsigned char				:�Đ��p�o�b�t�@�̎��̃��b�Z�[�W�ԍ�
///	@author			Tanaka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/02/07
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void	AVM_Sodiac_ReadPtrInc( unsigned short ch,	unsigned char ptr)
{
	/* �Đ�������read�|�C���^�C���N�������g */	
	if( ptr >= (AVM_REQ_BUFF_SIZE -1) )
	{
		AVM_Sodiac_Ctrl[ch].read_ptr = 0;
	}
	else
	{
		AVM_Sodiac_Ctrl[ch].read_ptr++;
	}
	return;
}
//[]----------------------------------------------------------------------[]
///	@brief			Sodiac�Đ��p�o�b�t�@�̓������b�Z�[�W�ԍ��̃o�b�t�@�N���A����
//[]----------------------------------------------------------------------[]
///	@return			unsigned char	:�Đ��p�o�b�t�@�̎��̃��b�Z�[�W�ԍ�
///	@author			Tanaka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/02/09
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void	AVM_Sodiac_EqualMsgBuffClear( unsigned short ch, unsigned char  ptr, unsigned char message_num )
{
	unsigned char count;
	
	for( count = 0; count < AVM_REQ_BUFF_SIZE ; count++ )
	{
		/* �|�C���^�C���N�������g */	
		if( ptr >= (AVM_REQ_BUFF_SIZE -1) )
		{
			ptr = 0;
		}
		else
		{
			ptr++;
		}
		/* �������b�Z�[�W�ԍ��̃o�b�t�@���T�[�`���� */
		if( message_num == AVM_Sodiac_Ctrl[ch].sd_req[ptr].message_num )
		{
			/* �g�p�ς݃o�b�t�@�N���A	*/
			memset( &AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm, 0x00, sizeof(AVM_SODIAC_REQ));
		}
	}
	return;
}
//[]----------------------------------------------------------------------[]
///	@brief			Sodiac�Đ��v���C���^�[�o���N������
//[]----------------------------------------------------------------------[]
///	@return			void
///	@author			Tanaka
/// @parameter		
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/02/07
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void	AVM_Sodiac_Play_WaitReq( unsigned char ch,  unsigned char ptr )
{
	/* �C���^�[�o��������ꍇ(��Ŋ֐�������) */
	if( AVM_Sodiac_Ctrl[ch].sd_req[ptr].wait >= 1 ){
		if( 0 != AVM_Sodiac_Ctrl[ch].sd_req[ptr].wait )
		{
			Lagtim( OPETCBNO, (unsigned char)(ch + 22), 
					(unsigned short)(AVM_Sodiac_Ctrl[ch].sd_req[ptr].wait*50) );			/* ���̍Đ��v���܂ł̃C���^�[�o���^�C�}�[�N��	*/
		}
		else
		{
			AVM_Sodiac_Play_Wait_Tim( ch );													/* �^�C�}�[���N�������Ɏ��̍Đ��v�����s��		*/
		}
	}else{
		AVM_Sodiac_Play_Wait_Tim( ch );														/* �^�C�}�[���N�������Ɏ��̍Đ��v�����s��		*/
	}
	return;
}
//[]----------------------------------------------------------------------[]
///	@brief			Sodiac From�A�N�Z�X���J������
//[]----------------------------------------------------------------------[]
///	@return			void
///	@author			Tanaka
/// @parameter		
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/03/23
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void	AVM_Sodiac_SemFree( void )
{
	/* ch0, ch1�����Đ���~���̏ꍇ	*/
	if( (1 == AVM_Sodiac_Ctrl[0].play_message_cmp) && ( 1 == AVM_Sodiac_Ctrl[1].play_message_cmp) )
	{
		From_Access_SemFree(2);						/* �A�N�Z�X���J��					*/
		Lagcan( OPETCBNO, 24 );						/* ��ϰ24��ݾ�						*/
	}
	
	return;
	
}
//[]----------------------------------------------------------------------[]
///	@brief			Sodiac From�A�N�Z�X���擾����
//[]----------------------------------------------------------------------[]
///	@return			void
///	@author			Tanaka
/// @parameter		
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/03/23
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
uchar	AVM_Sodiac_SemGet( void )
{
	
	uchar	ret = 0;
	/* ch0, ch1�ǂ��炩���Đ����̏ꍇ	*/
	if( (0 == AVM_Sodiac_Ctrl[0].play_message_cmp) ||( 0 == AVM_Sodiac_Ctrl[1].play_message_cmp) )
	{
		return ret;
	}

	// �������݌��擾
	ret = From_Access_SemGet_Common( 2 );

	Lagtim( OPETCBNO, 24, (50 * 600));		/* Sodiac����̍Đ��I���ʒm�����Ȃ��ꍇ����ϰ�N�� */

	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			Sodiac �Đ��I���ʒm������ϰ��ѱ�ď���
//[]----------------------------------------------------------------------[]
///	@return			void
///	@author			Tanaka
/// @parameter		
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/03/23
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void	AVM_Sodiac_TimeOut( void )
{
	From_Access_SemFree(2);											/* �A�N�Z�X���J��								*/
																	/* Sodiac�����삵�Ă��Ȃ��Ƃ݂Ȃ�   			*/
	Avm_Sodiac_Err_flg |= SODIAC_NOTSTOP_ERR;						/* �ُ�I���Ȃ̂ňȌ��SodiacAPI�̓R�[�����Ȃ�  */
	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			Sodiac ���Z�����񐸎Z�������菈��
//[]----------------------------------------------------------------------[]
///	@return			TRUE 	���Z��
///					FALSE	�񐸎Z��
///	@author			Tanaka
/// @parameter		
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/04/06
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
BOOL	AVM_Sodiac_PayOff_Judge( void )
{
	BOOL ret = FALSE;											//�����l�͔񐸎Z��
	// ���Z���̏ꍇ	
	if ( (OPECTL.Ope_mod == 2) || (OPECTL.Ope_mod == 3))
	{
		ret = TRUE;
	}
	return ret;
}
