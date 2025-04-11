/************************************************************************************/
/*																					*/
/*		システム名　:　RAUシステム													*/
/*		ファイル名	:  rausub.C														*/
/*		機能		:  サブルーチン処理												*/
/*																					*/
/************************************************************************************/

#include	"rau.h"
#include	"rauIDproc.h"

/*----------------------------------*/
/* CRC-CCITT :  x^{16}+x^{12}+x^5+1 */
/*----------------------------------*/
#define		RU_CRCPOLY1				0x1021										/* 左シフト						*/
#define		RU_CRCPOLY2				0x8408  									/* 右シフト						*/
#define		RU_CHAR_BIT				8											/* number of bits in a char		*/
#define		RU_L_SHIFT				0											/* 左シフト						*/
#define		RU_R_SHIFT				1											/* 右シフト						*/

/********************* FUNCTION DEFINE **************************/
void	RAU_CrcCcitt(ushort length, ushort length2, uchar cbuf[], uchar cbuf2[], uchar *result, uchar type );


/* CRC16-CCITT 算出関数 (右シフトを使用する) */									/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	Calculate CRC-CCITT																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME  : crc_ccitt( len, cbuf, result, type )														   |*/
/*| PARAMETER    : len   : Data length																		   |*/
/*|              : cbuf  : Char data																		   |*/
/*|				 : result: Crc																				   |*/
/*|				 : type  : 0: left shift 1:rigth shift														   |*/
/*| RETURN VALUE : void																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Akiba(AMANO)																			   |*/
/*| Date		: 2005-02-23																				   |*/
/*| Update		: 2005-08-08	J.Mitani(ART)	不要なコメントを削除。バッファの折り返しに対応。			   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	・CRCの計算を行う																						   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	RAU_CrcCcitt(ushort length, ushort length2, uchar cbuf[], uchar cbuf2[], uchar *result, uchar type ) {

	ushort	i, j;
	uchar	crc_wk;
	ushort	crc;

	crc		= 0x0000;
	crc_wk	= 0x0000;

	switch( type ){
	case 0: 	/* 左シフト	*/
		for(i = 0; i < length; i++) {
			crc ^= (ushort)cbuf[i] << (16 - RU_CHAR_BIT);
			for (j = 0; j < RU_CHAR_BIT; j++){
				if (crc & 0x8000){
					crc = (crc << 1) ^ RU_CRCPOLY1;
				}else{
					crc <<= 1;
				}
			}
		}
		for(i = 0; i < length2; i++) {
			crc ^= (ushort)cbuf2[i] << (16 - RU_CHAR_BIT);
			for (j = 0; j < RU_CHAR_BIT; j++){
				if (crc & 0x8000){
					crc = (crc << 1) ^ RU_CRCPOLY1;
				}else{
					crc <<= 1;
				}
			}
		}
		break;

	case 1: 	/* 右シフト	*/
		for (i = 0; i < length; i++) {
			crc ^= cbuf[i];
			for (j = 0; j < RU_CHAR_BIT; j++){
				if (crc & 1){
					crc = (crc >> 1) ^ RU_CRCPOLY2;
				}else{
					crc >>= 1;
				}
			}
		}
		for (i = 0; i < length2; i++) {
			crc ^= cbuf2[i];
			for (j = 0; j < RU_CHAR_BIT; j++){
				if (crc & 1){
					crc = (crc >> 1) ^ RU_CRCPOLY2;
				}else{
					crc >>= 1;
				}
			}
		}
		break;
	}

	crc_wk = (uchar)(crc & 0x00ff);
	*result = crc_wk;

	crc_wk = (uchar)((crc & 0xff00) >> RU_CHAR_BIT);
	*(result+1) = crc_wk;
}

void RAU_Word2Byte(unsigned char *data, ushort uShort)
{
	*(data + 0) = (uShort & 0xff00) >> 8;
	*(data + 1) = (uShort & 0x00ff);
}

ushort	RAU_Byte2Word(unsigned char *data)
{
	union {
		uchar	uc[2];
		ushort	us;
	} u;

	u.uc[0] = *data;
	u.uc[1] = *(data + 1);
	
	return u.us;
}

ulong	RAU_Byte2Long(unsigned char *data)
{
	union {
		uchar	uc[4];
		ulong	ul;
	} u;

	u.uc[0] = *data;
	u.uc[1] = *(data + 1);
	u.uc[2] = *(data + 2);
	u.uc[3] = *(data + 3);
	
	return u.ul;
}
