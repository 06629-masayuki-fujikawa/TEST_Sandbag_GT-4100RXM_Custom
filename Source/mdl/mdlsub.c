/*[]----------------------------------------------------------------------[]*/
/*| ﾓｼﾞｭｰﾙ(磁気ﾘｰﾀﾞｰ,ｱﾅｳﾝｽ)関連制御                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700Nより流用                                         |*/
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
/***** ﾘｰﾄﾞｺﾏﾝﾄﾞ用ﾃｰﾌﾞﾙ *****/
const char RD_RED[][3] = {
	{ M_R_READ, 0x98, 0x93 },										// ﾘｰﾄﾞｺﾏﾝﾄﾞ(ﾘｰﾀﾞ内停止)
	{ M_R_READ, 0x91, 0x83 },										// ﾘｰﾄﾞｺﾏﾝﾄﾞ(ﾌﾟﾘﾝﾀ内停止)
	{ M_R_READ, 0x98, 0x83 },										// ﾘｰﾄﾞｺﾏﾝﾄﾞ(ﾘｰﾀﾞ内停止, プリンタなし)
};
/***** ﾘｰﾀﾞｺﾏﾝﾄﾞ(移動､書込、印字)ﾃｰﾌﾞﾙ *****/
/* ﾃｰﾌﾞﾙの意味                             */
/*  ﾘｰﾀﾞｰｺﾏﾝﾄﾞ,  方向,  切換,  RD_mod      */
/* {  M_R_MOVE,  0x8A,  0x90,   10   }     */
/* RD_mod = 10 : 券抜取り待ち状態          */
/*          12 : 動作継続(次のﾃｰﾌﾞﾙを送信) */
/*          13 : 動作終了状態              */
/*******************************************/
const char RD_CMD[][4] = {
	{ M_R_MOVE, 0x8A, 0x80, 10 },	//  0 取出口移動(ﾌﾟﾘﾝﾄﾓｰﾀｵﾌ)
	{ M_R_MOVE, 0x86, 0x90, 10 },	//  1 取出口移動(ﾌﾟﾘﾝﾄﾓｰﾀｵﾝ)
	{ M_R_MOVE, 0x85, 0x93, 12 },	//  2 駐車券取込(後排出)
	{ M_R_MOVE, 0x84, 0x83, 13 },	//  3
	{ M_R_PRWT, 0x86, 0xB0, 10 },	//  4 駐車券領収証
	{ M_R_WRIT, 0xAA, 0xA0, 10 },	//  5 定期書込(ﾍﾞﾘﾌｧｲ有り)
	{ M_R_PRNT, 0x84, 0x80, 13 },	//  6 ｻｰﾋﾞｽ券取込(後排出)
	{ M_R_MOVE, 0x83, 0x83, 12 },	//  7 ｻｰﾋﾞｽ券取込(前移動+後排出)
	{ M_R_PRNT, 0x85, 0x80, 12 },	//  8
	{ M_R_MOVE, 0x84, 0x80, 13 },	//  9
	{ M_R_MOVE, 0x83, 0x83, 12 },	// 10 ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ書込(ﾍﾞﾘﾌｧｲ有り)
	{ M_R_PRWT, 0xA6, 0x90, 10 },	// 11
	{ M_R_MOVE, 0x84, 0x80, 13 },	// 12 駐車券(領収証)取込(後排出)
	{ M_R_WRIT, 0xA6, 0xB0, 10 },	// 13 精算中止券書込み(ﾍﾞﾘﾌｧｲ有り)
	{ M_R_PRNT, 0x85, 0x90, 12 },	// 14 駐車券印字取込(後排出)
	{ M_R_MOVE, 0x84, 0x83, 13 },	// 15
	{ M_R_PRNT, 0x86, 0xB0, 10 },	// 16 駐車券戻し(預かり中止)
	{ M_R_PRWT, 0x86, 0xB0, 10 },	// 17 駐車券戻し(預かり中止書込)
	{ M_R_PRWT, 0x86, 0xB0, 10 },	// 18 発券
	{ M_R_MOVE, 0x85, 0x83, 12 },	// 19 廃券
	{ M_R_MOVE, 0x84, 0x83, 13 },	// 20
	{ M_R_PRNT, 0x80, 0x90, 13 },	// 21 駐車券印字取込(後印字時、後排出)
	{ M_R_WRIT, 0x8A, 0xA0, 10 },	// 22 定期書込(ﾍﾞﾘﾌｧｲ無し)
	{ M_R_MOVE, 0x98, 0x80, 10 },	// 23 ｶｰﾄﾞ取り込み
	{ M_R_WRIT, 0xAA, 0xA0, 13 },	// 24 定期書込(ｶｰﾄﾞ発行用、ﾍﾞﾘﾌｧｲ有り)
	{ M_R_WRIT, 0xAA, 0x80, 13 },	// 25 係員ｶｰﾄﾞ書込(ｶｰﾄﾞ発行用、ﾍﾞﾘﾌｧｲ有り)
	{ M_R_MOVE, 0x83, 0x83, 12 },	// 26 ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ書込(ﾍﾞﾘﾌｧｲ無し)
	{ M_R_PRWT, 0x86, 0x90, 10 },	// 27
	{ M_R_WRIT, 0x86, 0xB0, 10 },	// 28 精算中止券書込み(ﾍﾞﾘﾌｧｲなし)
	{ M_R_MOVE, 0x83, 0x83, 12 },	// 29 回数券書込(ﾍﾞﾘﾌｧｲ無し)
	{ M_R_PRWT, 0x86, 0xB0, 10 },	// 30
	{ M_R_MOVE, 0x83, 0x83, 12 },	// 31 回数券書込(ﾍﾞﾘﾌｧｲ有り)
	{ M_R_PRWT, 0xA6, 0xB0, 10 },	// 32
	{ M_R_MOVE, 0x82, 0x80, 13 },	// 33 プリンター無し廃券(後排出)
	{ M_R_WRIT, 0xC9, 0x80, 12 },	// 34 プリンター無し廃券(廃券書き込み、後排出)
	{ M_R_MOVE, 0x82, 0x80, 13 },	// 35
	{ M_R_MOVE, 0xCA, 0x80, 10 },	// 36 廃券書き込み、前排出
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
#define GET_BYTE_HIGH(num)	(unsigned char)((num >> 8) & 0xff)		// 文節上位バイト取得
#define GET_BYTE_LOW(num)	(unsigned char)(num & 0xff)				// 文節下位バイト取得
#define CMNMSG_NUMBER	3
char use_avm;
char pre_volume[2] = {0xff, 0xff};
unsigned short	Shashitu_EndStr;	// 車室読み上げ音声の最後につける音声("番です" or "番の")

/*[]----------------------------------------------------------------------[]*/
/*| 磁気ﾘｰﾀﾞｰ移動ｺﾏﾝﾄﾞ                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : rd_mov( short faz )                                     |*/
/*| PARAMETER    : faz : RD_CMDﾃｰﾌﾞﾙ項目№                                 |*/
/*| RETURN VALUE : ret : 0 = OK,  -1 = NG                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700Nより流用                                         |*/
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
			case 18:												// 駐車券ﾗｲﾄ(発券)
				memcpy( &MDLdata[0].mdl_data.rdat[3], MDP_buf, (size_t)MDP_siz );
				MDLdata[0].mdl_data.idc2 += (uchar)MDP_siz;			// Data Length
				MDLdata[0].mdl_size += MDP_siz;						// Send Length STX-ETX

				md_pari( &MDLdata[0].mdl_data.rdat[ 3 + ( MDP_siz - MDP_mag ) ], MDP_mag, 0 );	// Set Data Parity

				if(( comm == M_R_WRIT )||( comm == M_R_PRWT )||( comm == M_R_PRNT)){
					MDLdata[0].mdl_data.rdat[2] |= RD_pos;
				}
				break;
			default:												// 定期券ﾗｲﾄ, ﾌﾟﾘﾍﾟｲﾄﾞﾗｲﾄ他
				memcpy( &MDLdata[0].mdl_data.rdat[3], MDP_buf, (size_t)MDP_siz );
				if((prm_get( COM_PRM,S_SYS,11,1,1 ) == 1) && (MDP_buf[127] == 1 /*MAGred[MAG_GT_APS_TYPE] == 1*/) ){
					MDLdata[MTS_RED].mdl_data.rdat[2] |= 0x08;		// ﾗｲﾄﾃﾞｰﾀ形式 = GT磁気ﾌｫｰﾏｯﾄ
					if( (faz == 11) || (faz == 27) ){	// プリペイドカードの場合
						MDLdata[MTS_RED].mdl_data.idc2 += (char)( MDP_siz + 1);// Data Length
						/* チェックコード追加 */
						/* IDｺｰﾄﾞ～ﾁｪｯｸｻﾑまでのLRCと0xFFの排他的論理和 */
						MDLdata[MTS_RED].mdl_data.rdat[3 + MDP_siz] =
							CheckLRC8( &MDLdata[MTS_RED].mdl_data.rdat[3 + ( MDP_siz - MDP_mag )], (unsigned char)MDP_mag );
						MDLdata[MTS_RED].mdl_data.rdat[3 + MDP_siz] ^= 0xFF;
						MDLdata[MTS_RED].mdl_size +=  MDP_siz + 1;		/* Send Length STX-ETX*/
					}else if(faz == 25){	// 係員カードの場合
						MDLdata[MTS_RED].mdl_data.idc2 += (char)( MDP_siz );// Data Length
						MDLdata[MTS_RED].mdl_size +=  MDP_siz;		/* Send Length STX-ETX*/
					}else{	// プリペイドカード以外の場合
						MDLdata[MTS_RED].mdl_data.idc2 += (char)( MDP_siz + 1);// Data Length
						/* CRC8追加 */
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
// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
//	inc_dct( READ_DO, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
	switch( comm ){
		case M_R_MOVE:
			break;
		case M_R_WRIT:
// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
//			inc_dct( READ_WR, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
			break;
		case M_R_PRWT:
// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
//			inc_dct( READ_WR, 1 );
//			inc_dct( VPRT_DO, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
			break;
		case M_R_PRNT:
// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
//			inc_dct( VPRT_DO, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
			break;
		default:
			break;
	}
	RD_mod = RD_CMD[faz][3];
	RD_SendCommand = (uchar)(RD_CMD[faz][1] & 0x0f);
	Mag_LastSendCmd = comm;											// 磁気ﾘｰﾀﾞへ最後に送信したｺﾏﾝﾄﾞ保存

	mts_req |= MTS_BCR1;											// 送信要求FLG ONする

	return( 0 );
}

/*[]----------------------------------------------------------------------[]*/
/*| Reader Initialize                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : rd_init( short mod )                                    |*/
/*| PARAMETER    : mod : 1 = Card In,  2 = Card Out                        |*/
/*| RETURN VALUE : ret : 0 = OK,  -1 = NG                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700Nより流用                                         |*/
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
		if( prm_get( COM_PRM,S_SYS,11,1,1 ) == 1){			// 磁気リーダータイプ(APS/GT磁気フォーマット併用)
			mod |= 0x80;
		}
		MDLdata[0].mdl_data.rdat[1] = (char)mod;
		MDLdata[0].mdl_size = 4;									// Send Length STX-ETX
		MDLdata[0].mdl_endf = 1;									// Set  ETX

		mts_req |= MTS_BCR1;										// 送信要求FLG ONする

	}else{
		ret = -1;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| Reader Read Command Send                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : rd_read( short mod )                                    |*/
/*| PARAMETER    : mod : 0 = ﾘｰﾀﾞｰ内停止,  1 = ﾌﾟﾘﾝﾀ内保留                 |*/
/*| RETURN VALUE : ret : 0 = OK,  -1 = NG                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4700Nより流用                                         |*/
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

		mts_req |= MTS_BCR1;										// 送信要求FLG ONする

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
/*| Author       : TF4700Nより流用                                         |*/
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

		mts_req |= MTS_BCR1;										// 送信要求FLG ONする

	}else{
		ret = -1;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| Reader (FB7000) ﾒﾝﾃﾅﾝｽ Command                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*|	'm'コマンドを送信する。												   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : d1 : ﾒﾝﾃﾅﾝｽｺｰﾄﾞ（1=部番要求）						   |*/
/*|				   d2 : 予約１（現在0固定）								   |*/
/*|				   d3 : 予約２（現在0固定）								   |*/
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
		MDLdata[0].mdl_data.rdat[0] = 'm';							// ﾒﾝﾃﾅﾝｽｺﾏﾝﾄﾞ
		MDLdata[0].mdl_data.rdat[1] = d1;
		MDLdata[0].mdl_data.rdat[2] = d2;
		MDLdata[0].mdl_data.rdat[3] = d3;
		MDLdata[0].mdl_size = 6;									// Send Length STX-ETX
		MDLdata[0].mdl_endf = 1;									// Set  ETX

		mts_req |= MTS_BCR1;										// 送信要求FLG ONする

		// 券抜き取りをＥ受信で判定
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
/*| Author       : TF4700Nより流用                                         |*/
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

		mts_req |= MTS_BCR1;										// 送信要求FLG ONする

	}else{
		ret = -1;
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| Anounce Machine Initialize                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : an_init( char dat )                                     |*/
/*| PARAMETER    : dat : 中断処理方法                                      |*/
/*| RETURN VALUE : ret : 0 = OK,  -1 = NG                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : TF4800Nより流用                                         |*/
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
/*| Author       : TF4800Nより流用                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	an_test( char mod )
{
	short	ret = 0;

	if (use_avm == AVM_NOT_CONNECT) {			// 音声案内有無の判定
		return -1;
	}

	if( MDLdata[1].mdl_size == 0 ){									// Exist Data ? (N)

		MDLdata[1].mdl_data.idc1 = 1;								// Block No.
		MDLdata[1].mdl_data.idc2 = 2;								// Data Length
		MDLdata[1].mdl_data.rdat[0] = M_A_TEST;						// Test Command
		MDLdata[1].mdl_data.rdat[1] = mod;

		MDLdata[1].mdl_size = 4;									// Send Length STX-ETX
		MDLdata[1].mdl_endf = 1;									// Set  ETX

		mts_req |= MTS_BAVM1;										// 送信要求FLG ONする
		avm_snd();

	}else{
		ret = -1;
	}
	return( ret );
}


//[]----------------------------------------------------------------------[]
///	@brief			警告音の指定回数吹鳴
//[]----------------------------------------------------------------------[]
///	@return			ret : 0 = OK,  -1 = NG
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/02/05<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
short an_boo2(short	seg)
{

	if ( MDLdata[1].mdl_size != 0) {			// 音声案内有無の判定
		return -1;
	}

	// 効果音を指定した回数放送する
	MDLdata[1].mdl_data.idc1 = 1;						// Block No.
	MDLdata[1].mdl_data.idc2 = 7;						// Data Length
	MDLdata[1].mdl_data.rdat[0] = 0x0D;					// コマンドID
	MDLdata[1].mdl_data.rdat[1] = GET_BYTE_HIGH(seg);	// 文節No.
	MDLdata[1].mdl_data.rdat[2] = GET_BYTE_LOW(seg);	// 文節No.
	MDLdata[1].mdl_data.rdat[3] = 0x0f;	// 放送回数
	MDLdata[1].mdl_data.rdat[4] = 0;					// ウェイト秒
	MDLdata[1].mdl_data.rdat[5] = 1;					// 1チャネル固定
	MDLdata[1].mdl_data.rdat[6] = 0;				// 中断方法(初期設定)

	MDLdata[1].mdl_endf = 1;							// Set  ETX
	MDLdata[1].mdl_size = 10;							// Send Length STX-ETX(ID1,ID2,データ,ETX)

	mts_req |= MTS_BAVM1;										// 送信要求FLG ONする

	// 音声送信
	avm_snd();
	
	return 0;
}
/*[]----------------------------------------------------------------------[]*/
/*| 音声コマンド送信処理(複数メッセージ対応版)                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : an_msag2( )                                             |*/
/*| PARAMETER    : an_msgno : メッセージ№が格納されているﾊﾞｯﾌｧ            |*/
/*|                msg_cnt : メッセージの個数                              |*/
/*|                cnt : 繰り返し回数( 0 : 放送停止 )                      |*/
/*|                wat : インターバル ( 0 - 9 Second )                     |*/
/*|                ch  : 放送チャンネル ( 1 or 2 )                         |*/
/*| RETURN VALUE : ret : 0 = OK,  -1 = NG                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2009-09-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
short	an_msag( short *an_msgno, short msg_cnt, short cnt, short wat, char ch)
{
	short	m_cnt,ret, i, j;
	
	ret = 0;

	if(avm_alarm_flg && ch == 1){			// 警報発砲中はチャンネル1にコマンド送らない
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
		MDLdata[1].mdl_data.rdat[3 + m_cnt] = ch;					// 放送チャネル
		MDLdata[1].mdl_data.rdat[4 + m_cnt] = 0;					// 中断方法(初期設定)
		MDLdata[1].mdl_size = 8 + m_cnt;							// Send Length STX-ETX(ID1,ID2,データ,ETX)
		MDLdata[1].mdl_endf = 1;									// Set  ETX
		mts_req |= MTS_BAVM1;										// 送信要求FLG ONする
		avm_snd();
	}
	else{
		ret = -1;
	}
	return( ret );

}
/*[]----------------------------------------------------------------------[]*/
/*| 音声停止処理                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : an_stop( )                                             |*/
/*| PARAMETER    : ch  : 放送チャンネル ( 1 or 2 )                         |*/
/*| RETURN VALUE :                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2012-05-07                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
void an_stop(char ch)
{
	if(avm_alarm_flg && ch == 1){			// 警報発砲中はチャンネル1にコマンド送らない
		return;
	}
	if( MDLdata[1].mdl_size == 0 ){									// Exist Data ? (N)
		MDLdata[1].mdl_data.idc1 = 1;			// Block No.
		MDLdata[1].mdl_data.idc2 = 4;			// Data Length
		MDLdata[1].mdl_data.rdat[0] = 0x0C;		// コマンドID
		MDLdata[1].mdl_data.rdat[1] = ch;		// 終了チャネル
		MDLdata[1].mdl_data.rdat[2] = 0;		// 放送中断方法→初期設定に従う
		MDLdata[1].mdl_data.rdat[3] = 1;		// キューイング中の開始コマンド→すべて無効

		MDLdata[1].mdl_endf = 1;				// Set  ETX
		MDLdata[1].mdl_size = 7;				// Send Length STX-ETX(ID1,ID2,データ,ETX)
		mts_req |= MTS_BAVM1;										// 送信要求FLG ONする
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
/*| Author       : TF4800Nより流用                                         |*/
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

		mts_req |= MTS_BAVM1;										// 送信要求FLG ONする

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
/*| Author       : TF4700Nより流用                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static short	op_modl( short no )
{
	short	ret;
	ret		=	0;
	OP_MODUL = 50;													// Wait 100ms

	while(( mts_req & MTS_BCR1 ) == MTS_BCR1 ){						// ﾘｰﾀﾞ送信要求あり?

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
/*| Author       : TF4700Nより流用                                         |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	md_pari( unsigned char *dat, unsigned short siz, char mod )
{
	unsigned short	sdat;
	unsigned short	pcnt;
	unsigned char	rtc;
	unsigned char	i;
	uchar			tik_syu;										// 券種
	uchar			set_id;											// 新IDに変換した券ID
	
	if( prm_get( COM_PRM,S_PAY,10,1,4 ) != 0 ){						// 新カードＩＤ使用する？

		// 新カードＩＤを使用する場合

		set_id = *dat;												// 書込みを行う券ID(入力ﾊﾟﾗﾒｰﾀ)取得

		for( tik_syu = 1 ; tik_syu <= TIK_SYU_MAX ; tik_syu++ ){	// 書込みを行う券IDの券種を検索

			if( set_id == tik_id_tbl[tik_syu][0] ){					// ID一致？
				break;												// YES
			}
		}
		// 実際に書き込む券IDを取得
		switch( tik_syu ){											// 券種？

			case	1:												// 駐車券（ＡＲ掛売券）
			case	2:												// 駐車券（ＡＲ-サ券中止券）
			case	3:												// 駐車券（ＡＲ-Ｐ回中止券）
			case	4:												// 駐車券（精算前）
			case	5:												// 駐車券（精算後）
			case	6:												// 駐車券（中止券）
			case	7:												// 駐車券（掛売券）
			case	8:												// 駐車券（再精算中止券）
			case	9:												// 駐車券（精算済み券）
			case	10:												// 再発行券
			case	11:												// 紛失券

				set_id = tik_id_tbl[tik_syu][ReadIdSyu.pk_tik-1];
				break;

			case	12:												// ＡＰＳ定期券

				set_id = tik_id_tbl[tik_syu][ReadIdSyu.teiki-1];
				break;

			case	13:												// プリペイドカード

				set_id = tik_id_tbl[tik_syu][ReadIdSyu.pripay-1];
				break;

			case	14:												// 回数券

				set_id = tik_id_tbl[tik_syu][ReadIdSyu.kaisuu-1];
				break;

			case	15:												// サービス券（掛売券・割引券）

				set_id = tik_id_tbl[tik_syu][ReadIdSyu.svs_tik-1];
				break;

			case	16:												// 係員カード

				set_id = tik_id_tbl[tik_syu][ReadIdSyu.kakari-1];
				break;
		}
		*dat = set_id;												// 実際に書き込む券IDに書込みﾃﾞｰﾀを変更
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
/*| Author       : TF4700Nより流用                                         |*/
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
///	@brief			音量パターンX開始時間取得
//[]----------------------------------------------------------------------[]
///	@return			音量パターン(1～3)
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
		// 設定数が0か1ならパターン1の音声を参照する
		ptn = 1;
	} else {
		for (i = 0; i < num - 1; i++) {
			settime1 = (short)prm_get(COM_PRM, S_SYS, (short)(i + 54), 4, 1);		// 01-0054～
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
///	@brief			音量パターンXのときの音量取得
//[]----------------------------------------------------------------------[]
///	@return			音量
///	@param[in]		ptn: 音量パターンX
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
///	@brief			アナウンスリセット処理
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
	MDLdata[MTS_AVM].mdl_size = 0;		// アナウンス要求クリア
}

//[]----------------------------------------------------------------------[]
///	@brief			AVM接続判定
//[]----------------------------------------------------------------------[]
///	@return			1:接続有り 0:なし
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
/*|	CRC算出関数															|*/
/*[]-------------------------------------------------------------------[]*/
/*| MODULE NAME  : crc = CheckCRC8( *src, size );						|*/
/*| PARAMETER	 : unsigned char	*src;	: CRC計算データアドレス		|*/
/*|				 : unsigned char	size; 	: CRC計算データサイズ		|*/
/*| RETURN VALUE : unsigned char	 crc;	: CRC計算結果				|*/
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
/*|	LRC算出関数															|*/
/*[]-------------------------------------------------------------------[]*/
/*| MODULE NAME  : crc = CheckLRC8( *src, size );						|*/
/*| PARAMETER	 : unsigned char	*src;	: LRC計算データアドレス		|*/
/*|				 : unsigned char	size; 	: LRC計算データサイズ		|*/
/*| RETURN VALUE : unsigned char	 crc;	: LRC計算結果				|*/
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
///	@brief			特殊メッセージ作成処理
//[]----------------------------------------------------------------------[]
///	@param[in]		mode : 作成するメッセージの指定  AVM_RYOUKIN(料金読み上げ)
///	@return			ret : メッセージ数
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/02/02<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
short an_msag_edit_rxm(short mode)
{
	short len;

	memset(an_msgbuf, 0, sizeof(an_msgbuf));// 料金読み上げ用メッセージ編集バッファ
	len = 0;

	switch (mode) {
	case AVM_RYOUKIN:			// 料金読み上げ
		// 料金設定
		len = Make_FeeString(announceFee, &(an_msgbuf[0]));
		break;
	case AVM_SHASHITU:			// 車室読み上げ
		// 料金設定
		len = Make_ShashituString(key_num, &(an_msgbuf[0]));
		break;
	}

		
	return len;
}
//[]----------------------------------------------------------------------[]
///	@brief			読み上げ料金文字列作成
//[]----------------------------------------------------------------------[]
///	@return			void
///	@param[in]		fee		: 料金
///	@param[in]		*anndata: 文節登録用文字列
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/01/17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static short Make_FeeString(const unsigned long fee, unsigned short * anndata)
{
	short digit[6];	// 各桁の数値 [0]=1桁目の数値,[1]=2桁目...
	char flag[5];	// 1～n桁がすべて0なら0をセット、1つでも0以外なら1をセット
	short i, j, pos = 0;
	unsigned long copy;
	char ret;
	
	copy = fee;

	if(0 == prm_get( COM_PRM,S_SYS, 47, 1, 2 )){		// 車室読み上げなし
		anndata[pos++] = AVM_RYOUKIN_HA;
	}

	// 各桁の数値取得
	for (i = 0; i < 6; i++) {
		digit[i] = (short)(copy % 10);
		copy /= 10;
	}

	// 1～n桁目が0かどうかの判定
	for (i = 0, j = 0; i < 5; i++) {
		// 1桁目から和をとっていく
		j += digit[i];
		flag[i] = (j > 0);	// [0]=1桁目,[1]=1～2桁目,[2]=1～3桁目～,5桁まで見るのでi<5
	}

	// 十万の位登録
	ret = set_digit6(anndata, digit[5], flag[4], &pos);
	
	// 万の位登録
	if (ret == 1) {
		ret = set_digit5(anndata, digit[5], digit[4], flag[3], &pos);
	} else {
		goto END;
	}
	
	// 千の位登録
	if (ret == 1) {
		ret = set_digit4(anndata, digit[3], flag[2], &pos);
	} else {
		goto END;
	}
	
	// 百の位登録
	if (ret == 1) {
		ret = set_digit3(anndata, digit[2], flag[1], &pos);
	} else {
		goto END;
	}
	
	// 十･一の位目登録
	if (ret == 1) {
		set_digit2(anndata, digit[1], digit[0], &pos);
	} else {
		goto END;
	}
END:
	return pos;
}


//[]----------------------------------------------------------------------[]
///	@brief			料金十万の位音声設定
//[]----------------------------------------------------------------------[]
///	@return			1		: 料金設定継続<br>
///					0		: 料金設定終了<br>
///	@param[in]		*msg	: 文節登録用文字列
///	@param[in]		dig		: 6桁目の値
///	@param[in]		flg		: 1～5桁目がすべて0かどうかの判定フラグ
///	@param[in]		*p		: *msgのポインタ
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/01/17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static char set_digit6(unsigned short * msg, short dig, char flg, short * p)
{
	short num;
	
	if (dig > 0) {
		// 6桁目が0でないとき
		if (flg == 0) {
			// 1～5桁目がすべて0
			if (dig != 1) {
				// 6桁目が1より大きい
				// "nジュウ"Fee2nc.wav
				num = ANN_JUU + 2 + ((dig-1) * 3);
				msg[(*p)++] = num;
				// "マン"Fee50a.wav
				msg[(*p)++] = ANN_MAN;
				// "エンデス"Fee9xa.wav
				msg[(*p)++] = ANN_YEN_DESU;
			} else {
				// "ジュウ"Fee21b.wav
				msg[(*p)++] = ANN_JUU+1;
				// "マン"Fee50b.wav
				msg[(*p)++] = ANN_MAN+1;
				// エンデスFee9xb.wav
				msg[(*p)++] = ANN_YEN_DESU+1;
			}
			return 0;
		}
	}
	
	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief			料金万の位音声設定
//[]----------------------------------------------------------------------[]
///	@return			1		: 料金設定継続<br>
///					0		: 料金設定終了<br>
///	@param[in]		*msg	: 文節登録用文字列
///	@param[in]		dig6	: 6桁目の値
///	@param[in]		dig5	: 5桁目の値
///	@param[in]		flg		: 1～4桁目がすべて0かどうかの判定フラグ
///	@param[in]		*p		: *msgのポインタ
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/01/17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static char set_digit5(unsigned short * msg, short dig6, short dig5, char flg, short * p)
{
	short num;
	
	if (dig5 > 0) {
		// 5桁目が1以上のとき
		if (dig6 > 0) {
			// 6桁目が0より大きい
			// "nジュウ"Fee2mb.wav
			num = ANN_JUU + 1 + ((dig6-1) * 3);
			msg[(*p)++] = num;
		}
		
		if (flg != 0) {
			// 1～4桁目が0でない
			// "nマン"Fee5na.wav
			num = ANN_MAN + (dig5 * 2);				// 万の桁は「イチマン」の音声があるため-1する必要なし
			msg[(*p)++] = num;
		} else {
			// "nマン"Fee5nb.wav
			num = ANN_MAN + 1 + (dig5 * 2);			// 万の桁は「イチマン」の音声があるため-1する必要なし
			msg[(*p)++] = num;
			// "エンデス"Fee9xb.wav
			msg[(*p)++] = ANN_YEN_DESU+1;
			return 0;
		}
	} else {
		// 5桁目が0のとき
		if (dig6 != 0) {
			// 6桁目が0でない
			// "nジュウ"Fee2nc.wav
			num = ANN_JUU + 2 + ((dig6-1) * 3);
			msg[(*p)++] = num;
			// "マン"Fee50a.wav
			msg[(*p)++] = ANN_MAN;
		}
	}
	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief			料金千の位音声設定
//[]----------------------------------------------------------------------[]
///	@return			1		: 料金設定継続<br>
///					0		: 料金設定終了<br>
///	@param[in]		*msg	: 文節登録用文字列
///	@param[in]		dig		: 4桁目の値
///	@param[in]		flg		: 1～3桁目がすべて0かどうかの判定フラグ
///	@param[in]		*p		: *msgのポインタ
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/01/17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static char set_digit4(unsigned short * msg, short dig, char flg, short * p)
{
	short num;
	
	if (dig > 0) {
		// 4桁目が1以上のとき
		if (flg != 0) {
			// 1～3桁目が0でない
			// "nセン"Fee4na.wav
			if (dig > 1) {
				num = ANN_SEN + (((dig-1) * 2) - 1);
			}else{
				num = ANN_SEN;
			}
			msg[(*p)++] = num;
		} else {
			if (dig > 1) {
				// 4桁目が1でない
				// "nセン"Fee4nb.wav
				num = ANN_SEN + 1 + (((dig-1) * 2) - 1);
				msg[(*p)++] = num;
				// "エンデス"Fee9xb.wav
				msg[(*p)++] = ANN_YEN_DESU+1;
			} else {
				// "セン"Fee41a.wav
				msg[(*p)++] = ANN_SEN;
				// "エンデス"Fee9xa.wav
				msg[(*p)++] = ANN_YEN_DESU;
			}
			return 0;
		}
	}
	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief			料金百の位音声設定
//[]----------------------------------------------------------------------[]
///	@return			1		: 料金設定継続<br>
///					0		: 料金設定終了<br>
///	@param[in]		*msg	: 文節登録用文字列
///	@param[in]		dig		: 3桁目の値
///	@param[in]		flg		: 1～2桁目がすべて0かどうかの判定フラグ
///	@param[in]		*p		: *msgのポインタ
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/01/17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static char set_digit3(unsigned short * msg, short dig, char flg, short * p)
{
	short num;
	char wk = 0;			/* 百の桁は3,4,7,9が2種類あるので、音声番号調整用に使用する変数 */
	
	if (dig > 0) {
		// 3桁目が1以上のとき
		if (flg != 0) {
			// 1～2桁目が0でない
			// "nヒャク"Fee3na.wav
			switch (dig) {
			case 1:	
				// "ヒャク"Fee3na.wav
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
				// "nヒャク"Fee3na.wav
				num = ANN_HYAKU + ((dig-1) + wk);
				msg[(*p)++] = num;
				break;
			}
		} else {
			switch (dig) {
			case 1:	
				// "ヒャク"Fee3na.wav
				msg[(*p)++] = ANN_HYAKU;
				// "エンデス"Fee9xb.wav
				msg[(*p)++] = ANN_YEN_DESU+1;
				break;
			case 8:
				wk++;
			case 6:
			case 5:
				wk+=2;
			case 2:
				// "nヒャク"Fee3na.wav
				num = ANN_HYAKU + ((dig-1) + wk);
				msg[(*p)++] = num;
				// "エンデス"Fee9xa.wav
				msg[(*p)++] = ANN_YEN_DESU;
				break;
			case 9:
				wk++;
			case 7:
				wk++;
			case 4:
				wk++;
			case 3:
				// "nヒャク"Fee3nb.wav
				num = ANN_HYAKU + ((dig-1) + wk) + 1;
				msg[(*p)++] = num;
				// "エンデス"Fee9xa.wav
				msg[(*p)++] = ANN_YEN_DESU;
				break;
			}
			return 0;
		}
	}
	
	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief			料金一の位・十の位音声設定
//[]----------------------------------------------------------------------[]
///	@return			1		: 料金設定継続<br>
///					0		: 料金設定終了<br>
///	@param[in]		*msg	: 文節登録用文字列
///	@param[in]		dig2	: 2桁目の値
///	@param[in]		dig1	: 1桁目の値
///	@param[in]		*p		: *msgのポインタ
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/01/17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static char set_digit2(unsigned short * msg, short dig2, short dig1, short * p)
{
	short num;
	
	if (dig2 > 0) {
		// 2桁目が1以上のとき
		if (dig1 > 0) {
			// 1桁目が0でない
			// "nジュウ"Fee2nb.wav
			num = ANN_JUU + 1 + ((dig2-1) * 3);
			msg[(*p)++] = num;
		} else {
			// "nジュウ"Fee2na.wav
			num = ANN_JUU + ((dig2-1) * 3);
			msg[(*p)++] = num;
			if (dig2 != 1) {
				// 2桁目が1でない
				// "エンデス"Fee9xa.wav
				msg[(*p)++] = ANN_YEN_DESU;
			} else {
				// "エンデス"Fee9xb.wav
				msg[(*p)++] = ANN_YEN_DESU+1;
			}
			return 0;
		}
	}
	num = dig1;
	msg[(*p)++] = num;
	// "エンデス"Fee9xa.wav
	msg[(*p)++] = ANN_YEN_DESU;
	
	return 1;
}
//[]----------------------------------------------------------------------[]
///	@brief			車室読み上げ文字列作成
//[]----------------------------------------------------------------------[]
///	@return			void
///	@param[in]		car_no		: 車室番号(画面表示する番号)
///	@param[in]		*anndata: 文節登録用文字列
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/02/13<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static short Make_ShashituString(const unsigned short car_no, unsigned short * anndata)
{

	short digit[4];	// 各桁の数値 [0]=1桁目の数値,[1]=2桁目,[2]=3桁目
	char flag[4];	// 1～n桁がすべて0なら0をセット、1つでも0以外なら1をセット
	short i, j, posi;
	unsigned short copy;
	char ret;
	
	copy = car_no;
	posi = 0;
	if(0 == prm_get( COM_PRM,S_SYS, 47, 1, 1 )){		// 料金読み上げなし
		anndata[posi++] = AVM_SHASHITU_HA;		// 「車室番号は」
		Shashitu_EndStr = AVM_BANGOU_2;
	}
	else{
		anndata[posi++] = AVM_SHASHITU_BAN;		// 「車室番号」
		Shashitu_EndStr = AVM_BANGOU_1;
	}

	if(car_no == 0){// 車室番号0番は存在しない
		return (short)0;
	}else{// 車室番号31番からは数字を組み合わせて音声を生成する
		
		// 各桁の数値取得
		for (i = 0; i < 4; i++) {
			digit[i] = (short)(copy % 10);
			copy /= 10;
		}

		// 1～n桁目が0かどうかの判定
		for (i = 0, j = 0; i < 4; i++) {
			// 1桁目から和をとっていく
			j += digit[i];
			flag[i] = (j > 0);	//	[0]=1桁目,[1]=1～2桁目,[2]=1～3桁目まで見るのでi<3
		}

		// 千の位登録
		ret = set_digit4_shashitu(anndata, digit[3], flag[2], &posi);
		if(ret == 1){
			// 百の位登録
			ret = set_digit3_shashitu(anndata, digit[2], flag[1], &posi);
		}
	
		// 十･一の位目登録
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
		// 4桁目が1以上のとき
		if (flg != 0) {
			// 1～3桁目が0でない
			// "nセン"Fee4na.wav
			if (dig > 1) {
				num = ANN_SEN + (((dig-1) * 2) - 1);
			}else{
				num = ANN_SEN;
			}
			msg[(*p)++] = num;
		} else {
			if (dig > 1) {
				// 4桁目が1でない
				// "nセン"Fee4nb.wav
				num = ANN_SEN + 1 + (((dig-1) * 2) - 1);
				msg[(*p)++] = num;
				// "エンデス"Fee9xb.wav
				msg[(*p)++] = Shashitu_EndStr;
			} else {
				// "セン"Fee41a.wav
				msg[(*p)++] = ANN_SEN;
				// "エンデス"Fee9xb.wav
				msg[(*p)++] = Shashitu_EndStr;
			}
			return 0;
		}
	}
	return 1;
	
}
//[]----------------------------------------------------------------------[]
///	@brief			車室百の位音声設定
//[]----------------------------------------------------------------------[]
///	@return			1		: 車室設定継続<br>
///					0		: 車室設定終了<br>
///	@param[in]		*msg	: 文節登録用文字列
///	@param[in]		dig		: 3桁目の値
///	@param[in]		flg		: 1～2桁目がすべて0かどうかの判定フラグ
///	@param[in]		*p		: *msgのポインタ
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/02/13<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static char set_digit3_shashitu(unsigned short * msg, short dig, char flg, short * p)
{
	short num;
	char wk = 0;			/* 百の桁は3,4,7,9が2種類あるので、音声番号調整用に使用する変数 */
	
	if (dig > 0) {
		// 3桁目が1以上のとき
		if (flg != 0) {
			// 1～2桁目が0でない
			// "nヒャク"Fee3na.wav
			switch (dig) {
			case 1:	
				// "ヒャク"Fee3na.wav
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
				// "nヒャク"Fee3na.wav
				num = ANN_HYAKU + ((dig-1) + wk);
				msg[(*p)++] = num;
				break;
			}
		} else {
			switch (dig) {
			case 1:	
				// "ヒャク"Fee3na.wav
				msg[(*p)++] = ANN_HYAKU;
				// "バンデス"Fee9xb.wav
				msg[(*p)++] = Shashitu_EndStr;
				break;
			case 8:
				wk++;
			case 6:
			case 5:
				wk+=2;
			case 2:
				// "nヒャク"Fee3na.wav
				num = ANN_HYAKU + ((dig-1) + wk);
				msg[(*p)++] = num;
				// "バンデス"Fee9xa.wav
				msg[(*p)++] = Shashitu_EndStr;
				break;
			case 9:
				wk++;
			case 7:
				wk++;
			case 4:
				wk++;
			case 3:
				// "nヒャク"Fee3nb.wav
				num = ANN_HYAKU + ((dig-1) + wk) + 1;
				msg[(*p)++] = num;
				// "バンデス"Fee9xa.wav
				msg[(*p)++] = Shashitu_EndStr;
				break;
			}
			return 0;
		}
	}
	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief			車室一の位・十の位音声設定
//[]----------------------------------------------------------------------[]
///	@return			1		: 車室設定継続<br>
///					0		: 車室設定終了<br>
///	@param[in]		*msg	: 文節登録用文字列
///	@param[in]		dig2	: 2桁目の値
///	@param[in]		dig1	: 1桁目の値
///	@param[in]		*p		: *msgのポインタ
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/02/13<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static char set_digit2_shashitu(unsigned short * msg, short dig2, short dig1, short * p)
{
	short num;
	
	if (dig2 > 0) {
		// 2桁目が1以上のとき
		if (dig1 > 0) {
			// 1桁目が0でない
			// "nジュウ"Fee2nb.wav
			num = ANN_JUU + 1 + ((dig2-1) * 3);
			msg[(*p)++] = num;
		} else {
			// "nジュウ"Fee2na.wav
			num = ANN_JUU + ((dig2-1) * 3);
			msg[(*p)++] = num;
			if (dig2 != 1) {
				// 2桁目が1でない
				// "バンデス"
				msg[(*p)++] = Shashitu_EndStr;
			} else {
				// "バンデス"
				msg[(*p)++] = Shashitu_EndStr;
			}
			return 0;
		}
	}
	// "n"Fee1na.wav
	num = dig1;
	msg[(*p)++] = num;
	// "バンデス"
	msg[(*p)++] = Shashitu_EndStr;
	
	return 1;
}


//[]----------------------------------------------------------------------[]
///	@brief			Sodiacコールバック関数
//[]----------------------------------------------------------------------[]
/// @parameter		unsigned short ch 	:	再生対象ch
///					D_SODIAC_ID	   id 	: 	通知種別
///					D_SODIAC_E 	   err 	: 	処理結果
///	@return			void
///	@author			Tanaka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/02/03
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void avm_sodiac_callback( int ch , D_SODIAC_ID id , D_SODIAC_E err )
{
	unsigned char	ptr;										//readポインタ判定用
	unsigned char	resend_count;
	unsigned char	message_num;
	unsigned char 	ptr_tmp;
	unsigned char 	next_message_num;
	unsigned char	que_data[2];
	

	/* chチェック */
	/*	初期化時エラーありの場合は要求処理を行わない */
	if( SODIAC_ERR_NONE != Avm_Sodiac_Err_flg )
	{
		return;
	}

	/* read_ptrとwrite_ptrが同じ場合	*/
	if( D_SODIAC_ID_STOP == id || D_SODIAC_ID_ERROR == id  )
	{
		ptr	= AVM_Sodiac_Ctrl[ch].read_ptr;
	}

	switch( id )												/*	通知種別により分岐				　	*/
	{															/*                                    	*/

		case D_SODIAC_ID_STOP:									/*  音声停止の場合                    	*/
			if( AVM_Sodiac_Ctrl[ch].play_cmp == 0 && AVM_Sodiac_Ctrl[ch].stop == 0){
#if 1
				resend_count = AVM_Sodiac_Ctrl[ch].resend_count_now;
				message_num = AVM_Sodiac_Ctrl[ch].sd_req[ptr].message_num;
				/* 音声再生要求終了判定	*/
				next_message_num = AVM_Sodiac_NextMessageNum( ch, ptr );
				/* 次の文節が同じメッセージ番号の場合もしくは繰り返し回数が残っている場合	*/
                if( (message_num == next_message_num) || (0x01 != resend_count)) {
					if( message_num == next_message_num ) {
						AVM_Sodiac_ReadPtrInc( ch, ptr);												/* readポインタインクリメント					*/
					} else {
						//同じメッセージ番号をサーチして同一の番号があればそのreadptrを返す
						ptr_tmp = AVM_Sodiac_EqualReadPtr( ch, ptr, message_num );
						if( ptr != ptr_tmp ) {
							AVM_Sodiac_Ctrl[ch].read_ptr = ptr_tmp;
						}
						/* 無限繰り返し以外の場合	*/
						if( (0x00 != resend_count) && (0x0F != resend_count )) {
							AVM_Sodiac_Ctrl[ch].resend_count_now--;
						}
					}
					que_data[0] = ch;
					que_data[1] = ptr = AVM_Sodiac_Ctrl[ch].read_ptr;
					AVM_Sodiac_Ctrl[ch].play_message_cmp = 1;
					queset( OPETCBNO, SODIAC_NEXT_REQ, sizeof(que_data), &que_data );	

					/*	無限回繰り返し再生でない場合もしくは次に再生予定のメッセージ番号が異なる場合	*/
					return;
				}
				AVM_Sodiac_EqualMsgBuffClear( ch, ptr, message_num );										/* 同じメッセージ番号の要求	*/
																											/* バッファをクリアする。	*/
				/* 再生完了後readポインタインクリメント */	
				AVM_Sodiac_ReadPtrInc( ch, ptr);															/* readポインタインクリメント					*/
				/* 次の再生要求が残っている場合	*/
				ptr = AVM_Sodiac_Ctrl[ch].read_ptr;
				if( ptr != AVM_Sodiac_Ctrl[ch].write_ptr) {
					que_data[0] = ch;
					que_data[1] = ptr;
					AVM_Sodiac_Ctrl[ch].play_message_cmp = 1;
					queset( OPETCBNO, SODIAC_NEXT_REQ, sizeof(que_data), &que_data );	
					AVM_Sodiac_Ctrl[ch].resend_count_now	= AVM_Sodiac_Ctrl[ch].sd_req[ptr].resend_count;	/* 残り再送回数設定					*/
				} else {
					if(avm_alarm_flg && ch == 1){															/* 警報発砲中						*/
						avm_alarm_flg = 0;																	/* 警報フラグoff					*/
					}
					queset( OPETCBNO, SODIAC_PLAY_CMP, 0, 0 );	
					AVM_Sodiac_Ctrl[ch].play_cmp = 1;														/*  音声再生停止中フラグON（停止中）*/
					AVM_Sodiac_Ctrl[ch].play_message_cmp = 1;
				}
#endif
			} else {
			}
			break;
		case D_SODIAC_ID_ERROR:
			/*	リトライ送信回数がまだ残っている場合	*/
			if( 0 != AVM_Sodiac_Ctrl[ch].retry_count )
			{
				/* 送信失敗エラー登録を行う	*/
				AVM_Sodiac_Err_Chk( err, D_SODIAC_ID_ERROR );	/* エラー登録			*/
				que_data[0] = ch;
				que_data[1] = ptr = AVM_Sodiac_Ctrl[ch].read_ptr;
				AVM_Sodiac_Ctrl[ch].play_message_cmp = 1;
				queset( OPETCBNO, SODIAC_NEXT_REQ, sizeof(que_data), &que_data );	/* 音声再生要求処理	*/
				AVM_Sodiac_Ctrl[ch].retry_count--;
				return;
			}
			else
			{
				/*　リトライエラー登録　		*/
				AVM_Sodiac_Err_Chk((D_SODIAC_E)ERR_SODIAC_RETRY_ERR, D_SODIAC_ID_ERROR);
				// 音声再生を完了としセマフォを解放する
				AVM_Sodiac_Ctrl[0].stop = 0;
				AVM_Sodiac_Ctrl[1].stop = 0;
				AVM_Sodiac_Ctrl[0].play_message_cmp = 1;
				AVM_Sodiac_Ctrl[1].play_message_cmp = 1;
				AVM_Sodiac_SemFree();
			}
			break;
		case D_SODIAC_ID_MUTEOFF:
			/* MUTEレジスタ設定 */
			SODIAC_MUTE_SW	=	1;									//MUTE解除
			return;
		case D_SODIAC_ID_MUTEON:
			SODIAC_MUTE_SW	=	0;									//MUTE設定
		case D_SODIAC_ID_PWMON:
			break;
		case D_SODIAC_ID_PWMOFF:
			AVM_Sodiac_Ctrl[0].stop = 0;
			AVM_Sodiac_Ctrl[1].stop = 0;
			AVM_Sodiac_Ctrl[0].play_message_cmp = 1;
			AVM_Sodiac_Ctrl[1].play_message_cmp = 1;
			AVM_Sodiac_SemFree();							//	DebugNo39 arex様指示
		break;
		case D_SODIAC_ID_NEXT:
			break;
		default:
			return;
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			Sodiac初期化処理
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

	/* MUTEレジスタ設定 */
	/* ポートの設定 */
	PORT0.PMR.BIT.B2 = 0;												/* P02 */
	PORT0.PDR.BIT.B2 = 1;												/* アンプコントロール 出力 */

	memset( &AVM_Sodiac_Ctrl[0], 0x00, sizeof(AVM_SODIAC_CTRL)*2 );		//sodiac管理領域初期化
	Sodiac_FROM_Get 			= 0;									// 音声データFROM未取得
	Avm_Sodiac_Err_flg			= 0x00;
	//アンプ設定(引数は仕様がわかるまで仮設定)

	err = sodiac_amp_ctrl( 75,1,1);
	if( D_SODIAC_E_OK != err )
	{
		Avm_Sodiac_Err_flg |= SODIAC_AMP_ERR;							//異常終了なので以後のSodiacAPIはコールしない
		AVM_Sodiac_Err_Chk( err, ERR_SOIDAC_NOT_ID );
		return;
	}

	/* Sodiacミドルウェアの初期化 タイマー供給 PCLKB=50MHz */
	err = sodiac_init(50000000UL);
	if( D_SODIAC_E_OK != err )
	{
		Avm_Sodiac_Err_flg |= SODIAC_INIT_ERR; 							//異常終了なので以後のSodiacAPIはコールしない
		AVM_Sodiac_Err_Chk( err, ERR_SOIDAC_NOT_ID );
		use_avm = AVM_NOT_CONNECT;
		return;
	}
		
	err = sodiac_regist_eventhook(avm_sodiac_callback);
	if( D_SODIAC_E_OK != err )
	{
		Avm_Sodiac_Err_flg |= SODIAC_EVEHOOK_ERR;						//異常終了なので以後のSodiacAPIはコールしない
		AVM_Sodiac_Err_Chk( err, ERR_SOIDAC_NOT_ID );
		use_avm = AVM_NOT_CONNECT;
		return;
	}
	use_avm = AVM_CONNECT;												// AVM接続有り
	AVM_Sodiac_Ctrl[0].play_cmp = 1;									// 音声再生停止中フラグON（停止中）
	AVM_Sodiac_Ctrl[1].play_cmp = 1;
	AVM_Sodiac_Ctrl[0].play_message_cmp = 1;
	AVM_Sodiac_Ctrl[1].play_message_cmp = 1;

	ope_anm( AVM_TEST );												// （ﾊﾞｰｼﾞｮﾝ要求）送信
	return;
}
//[]----------------------------------------------------------------------[]
///	@brief			音声再生要求処理
//[]----------------------------------------------------------------------[]
///	@return			void
///	@author			Tanaka
/// @parameter		unsigned short ch 	:	再生対象ch
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/02/07
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void	AVM_Sodiac_Execute( unsigned short ch )
{
	D_SODIAC_E		err;
	unsigned char	que_data[2];
	unsigned char	ptr;																			/* readポインタ格納用				*/
	unsigned short*	volume;
	unsigned short	ptn;
	unsigned short	ch1_vol;

	ptr	= AVM_Sodiac_Ctrl[ch].read_ptr;
	volume = &AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm.volume;
	if( AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm.num != AVM_BOO ){
		/*	精算中であれば同じ音量で鳴らすために現在の音量値を設定する	*/
		if( TRUE == AVM_Sodiac_PayOff_Judge() )
		{
			if( Avm_Sodiac_PayOff_Volume[ch] != (*volume))
			{
				if( Avm_Sodiac_PayOff_Volume[ch] != 0xFFFF ){
					(*volume) = Avm_Sodiac_PayOff_Volume[ch];
				}
			}
		}
		/*	非精算中であれば精算中に同じ音量で鳴らすために再生設定音量値を保存する	*/
		else
		{
			/* 音声再生停止中フラグON(停止中) 	*/
			if( 1 == AVM_Sodiac_Ctrl[ch].play_cmp )
			{
				Avm_Sodiac_PayOff_Volume[ch] = (*volume);
				if( ch == 0 ){
					/* 非精算中のCH1の音量を保存する */
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

	if( AVM_Sodiac_SemGet() ){																			/*	アクセス権取得					*/
		return;
	}

	if(AVM_Sodiac_Ctrl[ch].sd_req[ptr].message_num != 0) {
		err = sodiac_execute( &AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm );							/* 音声再生要求送信					*/
	}
	else {
		// message_numが0の場合は再生しない
		err = D_SODIAC_E_OTHER;
	}
	if( D_SODIAC_E_OK != err )																		/* 									*/
	{
		AVM_Sodiac_SemFree();																		/* アクセス権開放					*/
		if(AVM_Sodiac_Ctrl[ch].sd_req[ptr].message_num != 0) {
			AVM_Sodiac_Err_Chk( err, ERR_SOIDAC_NOT_ID );											/*	エラー登録						*/
		}

		memset( &AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm, 0x00, sizeof(AVM_SODIAC_REQ));			/* 要求クリア						*/
		AVM_Sodiac_ReadPtrInc( ch, ptr);															/* readポインタインクリメント	  	*/
		ptr = AVM_Sodiac_Ctrl[ch].read_ptr;

		/* 次の再生要求が残っている場合	*/
		if( ptr != AVM_Sodiac_Ctrl[ch].write_ptr)
		{

			que_data[0] = ch;
			que_data[1] = ptr;
			queset( OPETCBNO, SODIAC_NEXT_REQ, sizeof(que_data), &que_data );
			AVM_Sodiac_Ctrl[ch].resend_count_now	= AVM_Sodiac_Ctrl[ch].sd_req[ptr].resend_count;	/* 残り再送回数設定					*/
		}
		/* 次の再生要求が残っていない場合*/
		else
		{
			if(avm_alarm_flg && ch == 1){															/* 警報発砲中						*/
			avm_alarm_flg = 0;																		/* 警報フラグoff					*/
			}
			AVM_Sodiac_Ctrl[ch].play_cmp = 1; 														/* 音声再生停止中フラグON(停止中) 	*/
			AVM_Sodiac_Ctrl[ch].play_message_cmp = 1;
		}
	}
	else
	{
		AVM_Sodiac_Ctrl[ch].play_cmp = 0; 															/* 音声再生停止中フラグOFF(再生中) */
		AVM_Sodiac_Ctrl[ch].play_message_cmp = 0;
	}																								/*  							   */
	return;
}
//[]----------------------------------------------------------------------[]
///	@brief			Sodiac再生インターバルﾀｲﾑｱｳﾄ処理
//[]----------------------------------------------------------------------[]
///	@return			void
///	@author			Tanaka
/// @parameter		unsigned short ch 	:	再生対象ch
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/02/07
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void	AVM_Sodiac_Play_Wait_Tim( unsigned short ch )
{

	Lagcan( OPETCBNO, 22 + ch );
	AVM_Sodiac_Execute( ch );											/* 音声再生要求処理			*/
	return;
}
//[]----------------------------------------------------------------------[]
///	@brief			Sodiacエラー登録処理
//[]----------------------------------------------------------------------[]
///	@return			void
///	@author			Tanaka
/// @parameter		D_SODIAC_E	err 			:	エラー種別
/// @parameter		unsigned char notice_id 	:	通知種別	
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
		err_chk2( ERRMDL_SODIAC, (char)err, 2, 2, 0, &err_inf );					// エラー登録・発生解除同時・情報あり(bin)・チェックあり
	}
	else if( D_SODIAC_E_STATE == err) {												// SodiacAPIが状態異常を返した
		// NOTE:	センターE2509のエラーが通知されるのを抑止する仮対策
		//			SodiacAPI(sodiac_execute)が状態異常を返す場合はエラー登録しない
		;
	}
	else
	{
		err_chk2( ERRMDL_SODIAC, (char)(err+5), 2, 2, 0, &err_inf );				// エラー登録・発生解除同時・情報あり(bin)・チェックあり
	}
	return;
}
//[]----------------------------------------------------------------------[]
///	@brief			Sodiac再生用バッファの次のメッセージ番号を取得する処理
//[]----------------------------------------------------------------------[]
///	@return			unsigned char				:再生中バッファの
///												次のメッセージ番号
///	@author			Tanaka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/02/07
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
unsigned char	AVM_Sodiac_NextMessageNum( unsigned short ch,	unsigned char ptr)
{
	/* ポインタインクリメント */	
	if( ptr >= (AVM_REQ_BUFF_SIZE -1) )
	{
		ptr = 0;
	}
	else
	{
		ptr++;
	}

	/* 次のバッファのメッセージ番号を取得する */
	return AVM_Sodiac_Ctrl[ch].sd_req[ptr].message_num;
}
//[]----------------------------------------------------------------------[]
///	@brief			Sodiac再生用バッファと同じメッセージ番号のreadポインタを返す処理
//[]----------------------------------------------------------------------[]
///	@return			unsigned char			:再生中バッファと同じ
///											メッセージ番号のreadポインタ
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
		/* ポインタインクリメント */	
		if( ptr >= (AVM_REQ_BUFF_SIZE -1) )
		{
			ptr = 0;
		}
		else
		{
			ptr++;
		}
		/* 同じメッセージ番号のバッファをサーチする */
		if( message_num == AVM_Sodiac_Ctrl[ch].sd_req[ptr].message_num )
		{
			return ptr;		//同じメッセージ番号のreadポインタを返す
		}
	}
	/* ここにはこないはず */
	return ptr;
}
//[]----------------------------------------------------------------------[]
///	@brief			Sodiac再生用バッファのreadポインタのインクリメント処理
//[]----------------------------------------------------------------------[]
///	@return			unsigned char				:再生用バッファの次のメッセージ番号
///	@author			Tanaka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/02/07
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void	AVM_Sodiac_ReadPtrInc( unsigned short ch,	unsigned char ptr)
{
	/* 再生完了後readポインタインクリメント */	
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
///	@brief			Sodiac再生用バッファの同じメッセージ番号のバッファクリア処理
//[]----------------------------------------------------------------------[]
///	@return			unsigned char	:再生用バッファの次のメッセージ番号
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
		/* ポインタインクリメント */	
		if( ptr >= (AVM_REQ_BUFF_SIZE -1) )
		{
			ptr = 0;
		}
		else
		{
			ptr++;
		}
		/* 同じメッセージ番号のバッファをサーチする */
		if( message_num == AVM_Sodiac_Ctrl[ch].sd_req[ptr].message_num )
		{
			/* 使用済みバッファクリア	*/
			memset( &AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm, 0x00, sizeof(AVM_SODIAC_REQ));
		}
	}
	return;
}
//[]----------------------------------------------------------------------[]
///	@brief			Sodiac再生要求インターバル起動処理
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
	/* インターバルがある場合(後で関数化する) */
	if( AVM_Sodiac_Ctrl[ch].sd_req[ptr].wait >= 1 ){
		if( 0 != AVM_Sodiac_Ctrl[ch].sd_req[ptr].wait )
		{
			Lagtim( OPETCBNO, (unsigned char)(ch + 22), 
					(unsigned short)(AVM_Sodiac_Ctrl[ch].sd_req[ptr].wait*50) );			/* 次の再生要求までのインターバルタイマー起動	*/
		}
		else
		{
			AVM_Sodiac_Play_Wait_Tim( ch );													/* タイマーを起動せずに次の再生要求を行う		*/
		}
	}else{
		AVM_Sodiac_Play_Wait_Tim( ch );														/* タイマーを起動せずに次の再生要求を行う		*/
	}
	return;
}
//[]----------------------------------------------------------------------[]
///	@brief			Sodiac Fromアクセス権開放処理
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
	/* ch0, ch1両方再生停止中の場合	*/
	if( (1 == AVM_Sodiac_Ctrl[0].play_message_cmp) && ( 1 == AVM_Sodiac_Ctrl[1].play_message_cmp) )
	{
		From_Access_SemFree(2);						/* アクセス権開放					*/
		Lagcan( OPETCBNO, 24 );						/* ﾀｲﾏｰ24ｷｬﾝｾﾙ						*/
	}
	
	return;
	
}
//[]----------------------------------------------------------------------[]
///	@brief			Sodiac Fromアクセス権取得処理
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
	/* ch0, ch1どちらかが再生中の場合	*/
	if( (0 == AVM_Sodiac_Ctrl[0].play_message_cmp) ||( 0 == AVM_Sodiac_Ctrl[1].play_message_cmp) )
	{
		return ret;
	}

	// 書き込み権取得
	ret = From_Access_SemGet_Common( 2 );

	Lagtim( OPETCBNO, 24, (50 * 600));		/* Sodiacからの再生終了通知が来ない場合のﾀｲﾏｰ起動 */

	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			Sodiac 再生終了通知無しﾀｲﾏｰﾀｲﾑｱｳﾄ処理
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
	From_Access_SemFree(2);											/* アクセス権開放								*/
																	/* Sodiacが動作していないとみなす   			*/
	Avm_Sodiac_Err_flg |= SODIAC_NOTSTOP_ERR;						/* 異常終了なので以後のSodiacAPIはコールしない  */
	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			Sodiac 精算中か非精算中か判定処理
//[]----------------------------------------------------------------------[]
///	@return			TRUE 	精算中
///					FALSE	非精算中
///	@author			Tanaka
/// @parameter		
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/04/06
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
BOOL	AVM_Sodiac_PayOff_Judge( void )
{
	BOOL ret = FALSE;											//初期値は非精算中
	// 精算中の場合	
	if ( (OPECTL.Ope_mod == 2) || (OPECTL.Ope_mod == 3))
	{
		ret = TRUE;
	}
	return ret;
}
