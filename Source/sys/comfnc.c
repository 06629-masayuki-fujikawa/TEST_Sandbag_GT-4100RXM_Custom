/*[]----------------------------------------------------------------------[]*/
/*| Common function                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : T.Hashimoto                                              |*/
/*| Date        : 2001.11.09                                               |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#include	<string.h>
#include	<stddef.h>
#include	"system.h"
#include	"mem_def.h"
#include	"remote_dl.h"
#include	"message.h"
#include	"rtc_readwrite.h"
#include	"ope_def.h"

/*[]----------------------------------------------------------------------[]*/
/*| queset                                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : queset( id, no, nu, add )                               |*/
/*| PARAMETER    : id   : Destination to send message                      |*/
/*|                no   : event no                                         |*/
/*|                nu   : number of byte                                   |*/
/*|                add  : address                                          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
short queset( unsigned char id, unsigned short no, char nu, void *add )
{
	MsgBuf	*msb;

	if( ( msb = GetBuf() ) == NULL ){
		return( -1 );
	}
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
	if (id == PRNTCBNO) {
		PrnDat_WriteQue(no, add, (ushort)nu);
	}
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
	msb->msg.command = no;
	if( nu != 0 ){
		memcpy( msb->msg.data, add, (size_t)nu );
	}
	if( no >= FTPMSG_CTRL_RECV && no <= FTPMSG_FILE_RECVEND ){
		if( DOWNLOADING() ){
			id = REMOTEDLTCBNO;
		}
	}
	PutMsg( id, msb );
	return( 0 );
}

/*[]----------------------------------------------------------------------[]*/
/*| ���춳�Ă̶���                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : inc_dct( no, nu )                                       |*/
/*| PARAMETER    : no   : ���ڇ�                                           |*/
/*|              : nu   : ���Ēl                                           |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.HARA                                                  |*/
/*| Date         : 2001-12-14                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void
inc_dct( short no, short nu )
{
	unsigned long	CNT_wok;
	uchar	ist;					// ���荞�ݏ��������Call����邽�߁A���荞�݃X�e�[�^�X�ύX�����ύX

// MH810104 GG119201(S) �d�q�W���[�i���Ή�
	if (isEJA_USE()) {
		if (no == MOV_JPNT_CNT) {
			// �d�q�W���[�i���ڑ����͈󎚍s����
			// �J�E���g�A�b�v���Ȃ�
			return;
		}
	}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

	ist = _di2();					// ���荞�݋֎~
	CNT_wok = Mov_cnt_dat[ no ];
	CNT_wok += (unsigned long)nu;
	if( CNT_wok > 999999999L ){
		CNT_wok -= 1000000000L;
	}
	Mov_cnt_dat[ no ] = CNT_wok;
	_ei2( ist );					// ���荞�݃X�e�[�^�X�����ɖ߂�

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| bcdbin(bcd)                                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| 2 digits bcd --> 2 digits bin                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  AMANO                                                   |*/
/*| Date        :  2000- 7-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
unsigned char bcdbin( unsigned char bcd )
{
	unsigned char  c1;
	unsigned char  c2;

	c1 = (unsigned char)( bcd & 0x0f );  /* 1st digit */
	c2 = (unsigned char)( bcd & 0xf0 );  /* 2nd digit */
	return( (unsigned char)( ( c2 >> 1 ) + ( c2 >> 3 ) + c1 ) );
}


/*[]----------------------------------------------------------------------[]*/
/*| bcd��bin�ɕϊ��@12,34 -> 1234                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : bcd: Start address of string                            |*/
/*| RETURN VALUE : bin data                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hashimoto                                             |*/
/*| Date        :  2001-12-26                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
short bcdbin2( unsigned char  *bcd )
{
	unsigned char  c1;
	unsigned char  c2;
	short	ret, i;

	c1 = (unsigned char)( *bcd & 0x0f );  /* 1st digit */
	c2 = (unsigned char)( *bcd & 0xf0 );  /* 2nd digit */
	ret = (short)( ( c2 >> 1 ) + ( c2 >> 3 ) + c1 );
	c1 = (unsigned char)( *(bcd+1) & 0x0f );  /* 1st digit */
	c2 = (unsigned char)( *(bcd+1) & 0xf0 );  /* 2nd digit */
	ret <<= 2;	/* x4 */
	i = ret << 3; /* x32 */
	ret += ( i << 1 ) + i; /* x64 */
	return( (short)( ret + ( ( c2 >> 1 ) + ( c2 >> 3 ) + c1 )) );
}


/*[]----------------------------------------------------------------------[]*/
/*| bcd��bin�ɕϊ��@12,34 -> 3412                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : bcd: Start address of string                            |*/
/*| RETURN VALUE : bin data                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hashimoto                                             |*/
/*| Date        :  2001-12-26                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
short bcdbin3( unsigned char  *bcd )
{
	unsigned char  c1;
	unsigned char  c2;
	short	ret, i;

	c1 = (unsigned char)( *(bcd+1) & 0x0f );  /* 1st digit */
	c2 = (unsigned char)( *(bcd+1) & 0xf0 );  /* 2nd digit */
	ret = (short)( ( c2 >> 1 ) + ( c2 >> 3 ) + c1 );
	ret <<= 2;	/* x4 */
	i = ret << 3; /* x32 */
	ret += ( i << 1 ) + i; /* x64 */
	c1 = (unsigned char)( *bcd & 0x0f );  /* 1st digit */
	c2 = (unsigned char)( *bcd & 0xf0 );  /* 2nd digit */
	return( (short)( ret + ( ( c2 >> 1 ) + ( c2 >> 3 ) + c1 )) );
}


/*[]----------------------------------------------------------------------[]*/
/*| binbcd(bin)                                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| 2 digits bin --> 2 digits bcd -->                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  AMANO                                                   |*/
/*| Date        :  2000- 7-18                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
unsigned char binbcd( unsigned char  bin )
{
	unsigned char  c1;
	unsigned char  c2;

	c1 = (unsigned char)( bin % 10 );
	c2 = (unsigned char)( bin / 10 );
	return( (unsigned char)(( c2 << 4 ) + c1) );
}


/*[]----------------------------------------------------------------------[]*/
/*| Calculate BCC                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : bcccal( stad, nu )                                      |*/
/*| PARAMETER    : stad	: Start address of string                          |*/
/*|                nu;	: Number of string charactors                      |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
unsigned char
bcccal( char *stad, short nu )
{
	short i;
	unsigned char c;

	for( c=*stad, i=1; i<nu; i++ ){
		c^=*(stad+i);	/* Make BCC */
	}
	return( c );
}


/*[]----------------------------------------------------------------------[]*/
/*| Calculate SUM                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : sumcal( stad, nu )                                      |*/
/*| PARAMETER    : stad : Start address of string                          |*/
/*|                nu;  : Number of string charactors                      |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2002-10-18                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
unsigned char
sumcal( char *stad, short nu )
{
	short i;
	unsigned char c;

	for( c=*stad, i=1; i<nu; i++ ){
		++stad;
		c += *stad;   /* Make SUM */
	}
	return( c );
}


/*[]----------------------------------------------------------------------[]*/
/*| Decimal Data Convert To Ascii (2Byte)                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cnvdec2( outp, data )                                   |*/
/*| PARAMETER    : outp;	: Output Address                               |*/
/*|                data;	: Convert Data                                 |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  AMANO                                                  |*/
/*| Date         : 2001-10-31                                              |*/
/*[]----------------------------------------------------------------------[]*/
void
cnvdec2( char *outp, short data )
{
	* outp      = (char)( ( ( (char)data % (char)100 ) / (char)10 ) | '0' );
	*(outp + 1) = (char)( ( (char)data % (char)10 ) | '0' );

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| Decimal Data Convert To Ascii (4Byte)                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : cnvdec4( outp, data )                                   |*/
/*| PARAMETER    : outp;	: Output Address                               |*/
/*|                data;	: Convert Data                                 |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  AMANO                                                  |*/
/*| Date         : 2001-10-31                                              |*/
/*[]----------------------------------------------------------------------[]*/
void
cnvdec4( char *outp, long data )
{
	* outp      = (char)( (char)(( data % 10000l ) / 1000l ) | '0' );
	*(outp + 1) = (char)( (char)(( data % 1000l ) / 100l ) | '0' );
	*(outp + 2) = (char)( (char)(( data % 100l ) / 10l ) | '0' );
	*(outp + 3) = (char)( (char)( data % 10l ) | '0' );

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| Ascii Data Convert To Binary                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : astoin( buf, size )                                     |*/
/*| PARAMETER    : buf  : Iutput Address                                   |*/
/*|                size : Number of Ascii charactor                        |*/
/*| RETURN VALUE : uint : Convert Data                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  AMANO                                                  |*/
/*| Date         : 2001-10-31                                              |*/
/*[]----------------------------------------------------------------------[]*/
unsigned short
astoin ( unsigned char *buf, short size )
{
	unsigned short	i;
	unsigned long	j, k;

	for( i = (unsigned short)1, j = 1L; i < size; i++ ) {
		j *= 10L;
	}
	for( i = (unsigned )0, k = 0L; i < size; i++ ) {
		k += (unsigned long)( buf[i] & 0x0f ) * j;
		if( ( j /= 10L ) == 0L ) {
			j = 1L;
		}
	}
	return( (unsigned short)k );
}


/*[]----------------------------------------------------------------------[]*/
/*| Ascii Data Convert To Time Binary                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : astotm( buf )                                           |*/
/*| PARAMETER    : buf  : Iutput Address                                   |*/
/*| RETURN VALUE : uint : Convert Data                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  AMANO                                                  |*/
/*| Date         : 2001-10-31                                              |*/
/*[]----------------------------------------------------------------------[]*/
unsigned short
astotm ( unsigned char *buf )
{
	return( (unsigned short)( ( ( (short)( buf[0] & 0x0f ) * (short)10 ) + (short)( buf[1] & 0x0f ) * (short)60 ) +
		        ( (short)( buf[2] & 0x0f ) * (short)10 ) + (short)( buf[3] & 0x0f ) ) );
}


/*[]----------------------------------------------------------------------[]*/
/*| Ascii Data Convert To Time Binary(long)                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : astotm( buf )                                           |*/
/*| PARAMETER    : buf  : Iutput Address                                   |*/
/*| RETURN VALUE : ulong: Convert Data                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  AMANO                                                  |*/
/*| Date         : 2001-10-31                                              |*/
/*[]----------------------------------------------------------------------[]*/
unsigned long
astoinl ( unsigned char	*buf, short size )
{
	short	i;
	unsigned long	j, k;

	for( i = (short)1, j = 1L; i < size; i++ ) {
		j *= 10L;
	}
	for( i = (short)0, k = 0L; i < size; i++ ) {
		k += (unsigned long)( buf[i] & 0x0f ) * j;
		if( ( j /= 10L ) == 0L ) {
			j = 1L;
		}
	}
	return( k );
}

/*[]----------------------------------------------------------------------[]*/
/*| Ascii Data(2byte) Convert To Hex Data                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : astohx( buf )                                           |*/
/*| PARAMETER    : buf  : Iutput Address                                   |*/
/*| RETURN VALUE : uchar: Convert Data                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  AMANO                                                  |*/
/*| Date         : 2001-10-31                                              |*/
/*[]----------------------------------------------------------------------[]*/
unsigned char
astohx ( unsigned char *buf )
{
	unsigned char	i[2];

	i[0] = buf[0];
	i[1] = buf[1];

	if( ( 'a' <= i[0] ) && ( i[0] <= 'f' ) ) {
		i[0] -= ' ';
	}
	if( ( 'a' <= i[1] ) && ( i[1] <= 'f' ) ) {
		i[1] -= ' ';
	}
	if( ( 'A' <= i[0] ) && ( i[0] <= 'F' ) ) {
		i[0] -= 0x07;
	}
	if( ( 'A' <= i[1] ) && ( i[1] <= 'F' ) ) {
		i[1] -= 0x07;
	}
	return((unsigned char)( ( ( ( i[0] & 0x0f ) << 4 ) & 0xf0 ) | ( i[1] & 0x0f ) ) );
}

/*[]----------------------------------------------------------------------[]*/
/*| Ascii Data(4byte) Convert To Hex Data                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : astohx2( buf )                                          |*/
/*| PARAMETER    : buf  : Iutput Address                                   |*/
/*| RETURN VALUE : short : Convert Data                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  AMANO                                                  |*/
/*| Date         : 2001-10-31                                              |*/
/*[]----------------------------------------------------------------------[]*/
unsigned short
astohx2 ( unsigned char	*buf )
{
	return( (unsigned short)( ( ( (short)astohx( &buf[0] ) << (short)8 ) & 0xff00 ) | (short)astohx( &buf[2] ) ) );
}


/*[]----------------------------------------------------------------------[]*/
/*| Binary Data Convert To Ascii                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : intoas( buf, data, size )                               |*/
/*| PARAMETER    : buf  : output Address                                   |*/
/*|                data : convert data                                     |*/
/*|                size : Number of Ascii charactor                        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  AMANO                                                  |*/
/*| Date         : 2001-10-31                                              |*/
/*[]----------------------------------------------------------------------[]*/
void
intoas( unsigned char *buf, unsigned short data, unsigned short size )
{
	unsigned short	i;
	unsigned long	j, k;

	for( i = (short)1, j = 1L; i < size; i++ ) {
		j *= 10L;
	}
	for( i = (short)0, k = (unsigned long)data; i < size; i++ ) {
		buf[i] = (char)( ( (char)( k / j ) % (char)10 ) | '0' );
		k %= j;
		if( ( j /= 10L ) == (short)0 ) {
			j = 1L;
		}
	}
	return;
}


/*[]----------------------------------------------------------------------[]*/
/*| Binary Data(long) Convert To Ascii                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : intoasl( buf, data, size )                              |*/
/*| PARAMETER    : buf  : output Address                                   |*/
/*|                data : convert long data                                |*/
/*|                size : Number of Ascii charactor                        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  AMANO                                                  |*/
/*| Date         : 2001-10-31                                              |*/
/*[]----------------------------------------------------------------------[]*/
void
intoasl( unsigned char *buf, unsigned long data, unsigned short size )
{
	unsigned short	i;
	unsigned long	j, k;

	for( i = (short)1, j = 1L; i < size; i++ ) {
		j *= 10L;
	}

	if( size < 10 ){										// Max����菬����
		k = j * 10L;
		data %= k;											// �s�v�ȏ�ʌ��}�X�N
	}

	for( i = (short)0, k = (unsigned long)data; i < size; i++ ) {
		buf[i] = (char)( ( (char)( k / j ) % (char)10 ) | '0' );
		k %= j;
		if( ( j /= 10L ) == (short)0 ) {
			j = 1L;
		}
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| Binary Data(long) Convert To Ascii (0 suppress)                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : intoasl( buf, data, size )                              |*/
/*| PARAMETER    : buf  : output Address (need size over 10 byte)          |*/
/*|                data : convert long data                                |*/
/*|                size : Number of Ascii charactor (request size)         |*/
/*| RETURN VALUE : Ascii character count (1-10)                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  AMANO                                                  |*/
/*| Date         : 2005-07-11                                              |*/
/*[]----------------------------------------------------------------------[]*/
unsigned char	
intoasl_0sup( unsigned char *buf, unsigned long data, unsigned short size )
{
	unsigned short	i, s;
	unsigned long	j, k;

	if( 0L == data ){
		buf[0] = '0';
		return	(unsigned char)1;
	}

	for( i = s = (short)1, j = 1L; i < size; i++ ) {
		j *= 10L;
	}

	if( size < 10 ){										// Max����菬����
		k = j * 10L;
		data %= k;											// �s�v�ȏ�ʌ��}�X�N
	}

	for( i = s = (short)0, k = (unsigned long)data; i < size; ++i ) {
		buf[s] = (char)( ( (char)( k / j ) % (char)10 ) | '0' );
		k %= j;

		if( (0 == s) && ('0' == buf[0]) ){
			;
		}
		else{
			++s;
		}

		if( ( j /= 10L ) == (short)0 ) {
			j = 1L;
		}
	}
	return (unsigned char)s;
}

/*[]----------------------------------------------------------------------[]*/
/*| Hex Data Convert To BIN(1byte)                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : hexcnv1( char )                                         |*/
/*| PARAMETER    : dat  : convert hex data                                 |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  AMANO                                                  |*/
/*| Date         : 2001-10-31                                              |*/
/*[]----------------------------------------------------------------------[]*/
char hexcnv1( char dat )
{
	if(  '0' <= dat && dat <= '9' ) return( (char)( dat-'0')  );
	if(  'a' <= dat && dat <= 'f' ) return(  (char)( (dat-'a')+10) );
	if(  'A' <= dat && dat <= 'F' ) return(  (char)( (dat-'A')+10) );
	return( 0 );
}

/*[]----------------------------------------------------------------------[]*/
/*| Hex Data Convert To Ascii(1byte)                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : hxtoasc( buf, dat )                                     |*/
/*| PARAMETER    : buf  : output Address                                   |*/
/*|                dat  : convert hex data                                 |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  AMANO                                                  |*/
/*| Date         : 2001-10-31                                              |*/
/*[]----------------------------------------------------------------------[]*/
void
hxtoasc( unsigned char *buf,unsigned char dat )
{
	if(( buf[0] = (char)(( dat & 0x0f ) | '0' ) ) > '9' ) {
		buf[0] += 0x07;
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| Hex Data Convert To Ascii(2byte)                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : hxtoas( buf, data )                                     |*/
/*| PARAMETER    : buf  : output Address                                   |*/
/*|                dat  : convert hex data                                 |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  AMANO                                                  |*/
/*| Date         : 2001-10-31                                              |*/
/*[]----------------------------------------------------------------------[]*/
void
hxtoas ( unsigned char *buf,unsigned char dat )
{
	if( ( buf[0] = (char)( ( ( dat >> 4 ) & 0x0f ) | '0' ) ) > '9' ) {
		buf[0] += 0x07;
	}
	if( ( buf[1] = (char)(( dat & 0x0f ) | '0' ) ) > '9' ) {
		buf[1] += 0x07;
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| Hex Data Convert To Ascii(4byte)                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : hxtoas2( buf, data )                                    |*/
/*| PARAMETER    : buf  : output Address                                   |*/
/*|                dat  : convert hex data                                 |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  AMANO                                                  |*/
/*| Date         : 2001-10-31                                              |*/
/*[]----------------------------------------------------------------------[]*/
void
hxtoas2 ( unsigned char	*buf1,unsigned short data )
{
	hxtoas( &buf1[0], (unsigned char)( ( data & 0xff00 ) >> (short)8 ) );
	hxtoas( &buf1[2], (unsigned char)( ( data & 0x00ff )             ) );

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| Get Binary Data Length (MAX6��)                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : binlen( dat )                                           |*/
/*| PARAMETER    : dat : data                                              |*/
/*| RETURN VALUE : ret : Length                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  AMANO                                                  |*/
/*| Date         : 2005-05-05                                              |*/
/*[]----------------------------------------------------------------------[]*/
ushort
binlen( ulong dat )
{
	if( dat / 100000L )	return( 6 );
	if( dat / 10000L )	return( 5 );
	if( dat / 1000L )	return( 4 );
	if( dat / 100L )	return( 3 );
	if( dat / 10L )		return( 2 );
	return( 1 );
}

/*[]----------------------------------------------------------------------[]*/
/*| Time Binary Data Convert To Ascii                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : tmtoas( buf, dat )                                      |*/
/*| PARAMETER    : buf  : output Address                                   |*/
/*|                dat  : Time binary data                                 |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  AMANO                                                  |*/
/*| Date         : 2001-10-31                                              |*/
/*[]----------------------------------------------------------------------[]*/
void
tmtoas( unsigned char *buf, short dat )
{
	intoas( &buf[0], (unsigned short)( dat / 60 ), 2 );
	intoas( &buf[2], (unsigned short)( dat % 60 ), 2 );

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| Data Copy ( Power Down OK )  ...MAX [NMI_DAT_MAX]byte                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : nmisave( ptr, dat, siz )                                |*/
/*| PARAMETER    : ptr	: Save Address                                     |*/
/*|                dat  : Data Address                                     |*/
/*|                siz	: Save Size                                        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  AMANO                                                  |*/
/*| Date         : 2001-10-31                                              |*/
/*[]----------------------------------------------------------------------[]*/
static	void	nmisv_b	( void );

void
nmisave ( void *ptr,void *dat, short siz )
{
	if( siz >= NMI_DAT_MAX ) {
		siz = NMI_DAT_MAX;
	}
	memcpy( FLAGPT.nmi_dat, dat, (size_t)siz );
	FLAGPT.nmi_siz = siz;
	FLAGPT.nmi_adr = ptr;

	FLAGPT.nmi_mod = 1;

	nmisv_b();
	return;
}

static	void
nmisv_b( void )
{
	memcpy( FLAGPT.nmi_adr, FLAGPT.nmi_dat, (size_t)FLAGPT.nmi_siz );
	FLAGPT.nmi_mod = 0;

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| Data Clear ( Power Down OK )                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : nmicler( ptr, siz )                                     |*/
/*| PARAMETER    : ptr	: Save Address                                     |*/
/*|                siz	: Save Size                                        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  AMANO                                                  |*/
/*| Date         : 2001-10-31                                              |*/
/*[]----------------------------------------------------------------------[]*/
static	void	nmicl_b	( void );

void
nmicler ( void *ptr, short siz )
{
	FLAGPT.nmi_siz = siz;
	FLAGPT.nmi_adr = ptr;

	FLAGPT.nmi_mod = 2;

	nmicl_b();
	return;
}

static	void
nmicl_b( void )
{
	memset( FLAGPT.nmi_adr, 0, (size_t)FLAGPT.nmi_siz );
	FLAGPT.nmi_mod = 0;

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| Data Set ( Power Down OK )                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : nmimset( ptr, dat, siz )                                |*/
/*| PARAMETER    : ptr	: Save Address                                     |*/
/*|                dat	: Save Data                                        |*/
/*|                siz	: Save Size                                        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :  AMANO                                                  |*/
/*| Date         : 2001-10-31                                              |*/
/*[]----------------------------------------------------------------------[]*/
static	void	nmist_b	( void );

void
nmimset ( void *ptr, unsigned char dat, short siz)
{
	FLAGPT.nmi_dat[0] = dat;
	FLAGPT.nmi_siz = siz;
	FLAGPT.nmi_adr = ptr;

	FLAGPT.nmi_mod = 3;

	nmist_b();
	return;
}

static	void
nmist_b( void )
{
	memset( FLAGPT.nmi_adr, (int)FLAGPT.nmi_dat[0], (size_t)FLAGPT.nmi_siz );
	FLAGPT.nmi_mod = 0;

	return;
}

void
nmitrap( void )
{
	switch( FLAGPT.nmi_mod ) {
		case 1:
			nmisv_b();
			break;
		case 2:
			nmicl_b();
			break;
		case 3:
			nmist_b();
			break;
		default:
			FLAGPT.nmi_mod = 0;
			break;
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| day normlize                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME : dnrmlzm( yyyy, mm, dd )                                  |*/
/*| PARAMETER   : yyyy  : �N                                               |*/
/*|             : mm    : ��                                               |*/
/*|             : dd    : ��                                               |*/
/*| RETURN VALUE: normlize data                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : R.Hara(AMANO)                                            |*/
/*| Date        : 2001-11-19                                               |*/
/*| Update      : 2004.6.28 T.Nakayama                                     |*/
/*|             : URUTBL�̋L�q�R��C��                                     |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
/***************************************************************/
/*	�P�X�W�O�N�R���P���i�y�j���j������Ƃ���				   */
/***************************************************************/

const ushort	URUTBL[2][14] = {{ 0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335,   0 },
								 { 0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 }};

unsigned short dnrmlzm( short yyyy, short mm, short dd )
{
	unsigned short	nday;
	short			elpyear;
	short			data;

	elpyear = yyyy - (short)1980;

	nday = ( elpyear * (short)365 ) + ( elpyear / (short)4 );

	if( ( yyyy & 0x0003 ) == 0x0000 ) {
		data = URUTBL[0][ mm ];
	}
	else {
		data = URUTBL[1][ mm ];
		nday++;
	}

	return( (unsigned short)(( nday + data + dd ) - (short)61) );
}

/*[]----------------------------------------------------------------------[]*/
/*| innormlize                                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME : idnrmlzm( dat, yyyy, mm, dd )                            |*/
/*| PARAMETER   : dat   : normlize data                                    |*/
/*|             : yyyy  : �N                                               |*/
/*|             : mm    : ��                                               |*/
/*|             : dd    : ��                                               |*/
/*| RETURN VALUE: void  :                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : R.Hara(AMANO)                                            |*/
/*| Date        : 2001-11-19                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/

#define	YEAR_4	(unsigned short)( (short)366 + ( (short)365 * (short)3 ) )

void idnrmlzm( unsigned short dat, short *yyyy, short *mm, short *dd )
{
	unsigned short	i = (short)1;
	unsigned short	k = (short)0;
	unsigned short	j[12];

	dat += (unsigned short)60;

	(*yyyy) = ( ( dat / YEAR_4 ) * (unsigned short)4 ) + (unsigned short)1980;

	if( ( dat %= YEAR_4 ) == (unsigned short)0 ) {
		(*mm) = (short)1;
		(*dd) = (short)1;
		return;
	}

	(*yyyy) += ( ( --dat ) / (unsigned short)365 );

	dat %= (unsigned short)365;

	if( ( (*yyyy) & 0x0003 ) == 0x0000 ) {
		i++;
		k = (short)1;
	}

	for( j[0] = j[1] = i, i = (short)2; i < (short)12; i++ ) {
		j[ i ] = (short)1;
	}

	for( i = (short)0; i < (short)12; i++ ) {
		if( i != (short)0 ) {
			k = (short)0;
		}
		if( dat < ( URUTBL[1][ i + (short)2 ] - k ) ) {
			(*mm) = (short)( i + (short)1 );
			(*dd) = dat - URUTBL[1][ i + (short)1 ] + j[ i ];
			break;
		}
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| Month Last Date Check                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME : medget( yyyy, mm )                                       |*/
/*| PARAMETER   : yyyy  : �N                                               |*/
/*|             : mm    : ��                                               |*/
/*| RETURN VALUE: ���̍ŏI��                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : R.Hara(AMANO)                                            |*/
/*| Date        : 2001-11-19                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
const uchar	OPWEEK[12] = { 31, 29, 31, 30, 31, 30,				/* Month Last Date Table		*/
						   31, 31, 30, 31, 30, 31 };
					   
short medget( short yyyy, short	mm )
{
	short	i = (short)0;

	if( ( yyyy & 0x0003 ) != 0x0000 ) {
		if( mm == (short)2 ) {
			i = (short)1;
		}
	}

	return( (short)( (short)OPWEEK[ mm - (short)1 ] - i ) );
}

/*[]----------------------------------------------------------------------[]*/
/*| Time normlize                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME : tnrmlz( bhh, bmm, hh, mm )                               |*/
/*| PARAMETER   : bhh   : BASE TIME (HOUR)                                 |*/
/*|             : bmm   : BASE TIME (MINUTS)                               |*/
/*|             : hh    : TARGET TIME (HOUR)                               |*/
/*|             : mm    : TARGET TIME (MINUTS)                             |*/
/*| RETURN VALUE: normlize data                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : R.Hara(AMANO)                                            |*/
/*| Date        : 2001-11-19                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
short tnrmlz ( short bhh, short	bmm, short hh, short mm )
{
	short	bndat;
	short	tndat;
	short	ndat;

	/* BASE TIME NORMALIZE KEISAN	*/
	/* bndat = bhh*60+bmm;			*/
	/* TARGET-NORMALIZE DATA KEISAN */
	/* tndat = hh*60+mm;			*/
	/* NORMALIZE DATA KEISAN 		*/

	if(( bndat = (( bhh * (short)60 )+ bmm )) > ( tndat = (( hh * (short)60 )+ mm ))){
		ndat = (unsigned short)1440 - ( bndat - tndat );
	}
	else {
		ndat = tndat - bndat;
	}
	return( ndat );
}

/*[]----------------------------------------------------------------------[]*/
/*| Time innormlize                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME : tnrmlz( dat, bhh, bmm, hh, mm )                          |*/
/*| PARAMETER   : dat   : TIME-NORMALIZE DATA                              |*/
/*|             : bhh   : BASE TIME (HOUR)                                 |*/
/*|             : bmm   : BASE TIME (MINUTS)                               |*/
/*|             : hh    : TARGET TIME (HOUR)                               |*/
/*|             : mm    : TARGET TIME (MINUTS)                             |*/
/*| RETURN VALUE: void  :                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : R.Hara(AMANO)                                            |*/
/*| Date        : 2001-11-19                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void itnrmlz ( short dat, short	bhh, short bmm, short *hh, short *mm )
{
	short	adat;		/* ABSOLUTE TIME	*/

	/* BASE TIME NORMALIZE (0:00)	*/
	/* bndat = bhh*60+bmm;			*/
	/* ABSOLUTE TIME NORM DATA YOUI */
	/* adat = bndat+dat;			*/
	/* HOSEI 						*/
	/* adat = adat%1440;			*/

	adat = ( ( ( bhh * (short)60 ) + bmm ) + dat ) % (short)1440;

	*hh = adat / (short)60;			/* HOUR   KEISAN	*/
	*mm = adat % (short)60;			/* MINUTS KEISAN	*/

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| day and time normlize                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME : enc_nmlz_mdhm(year, month, day, hour, min)               |*/
/*| PARAMETER   : year   : �N                                              |*/
/*|             : month  : ��                                              |*/
/*|             : day    : ��                                              |*/
/*|             : hour   : ��                                              |*/
/*|             : min    : ��                                              |*/
/*| RETURN VALUE: normlize data                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Y.Ise                                                    |*/
/*| Date        : 2009-06-10                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
unsigned long enc_nmlz_mdhm(unsigned short year, unsigned char month, unsigned char day, unsigned char hour, unsigned char min)
{
// NOTE:�m�[�}���C�Y�f�[�^(ulong�^)�̍\���͈ȉ��̒ʂ�
// �N  �F2000�`2099   2099=833�����g�p�r�b�g��12bit
// �����F1/1�`12/31   12/31��12*32+31=19Fh���g�p�r�b�g��9bit(�Ђƌ���32�ɐ��l�����Ȃ���ΐ������f�R�[�h�ł��Ȃ�����)
// �����F0:00�`23�F59 23:59��23*60+59=59Fh���g�p�r�b�g��11bit
// ���� �N   |���� |����
// bit  31-20|19-11|10-0
	unsigned long ret, tmp;					/* �Ԃ�l,�v�Z�p�̈� */

	ret = (unsigned long)hour;				/* �����ړ� */
	ret = (ret * 60) + (unsigned long)min;	/* ���~60�ƕ��𑫂� */
	tmp = (unsigned long)month;				/* �����ꎞ�ۑ��̈�� */
	tmp = (tmp * 32) + (unsigned long)day;	/* ���~32�Ɠ��𑫂� */
	ret = ret + (tmp << 11);				/* �������������킹�� */
	tmp = (unsigned long)year;				/* �N���ꎞ�ۑ��̈�� */
	ret = ret + (tmp << 20);				/* �N�����킹�� */
	return ret;								/* �ϊ��l��Ԃ� */
}
/*[]----------------------------------------------------------------------[]*/
/*| day and time normlize                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME : dec_nmlz_mdhm(n, *year, *month, *day, *hour, *min)       |*/
/*| PARAMETER   : n      : enc_nmlz_mdhm�֐��ŕϊ������l                   |*/
/*|             : year   : �N                                              |*/
/*|             : month  : ��                                              |*/
/*|             : day    : ��                                              |*/
/*|             : hour   : ��                                              |*/
/*|             : min    : ��                                              |*/
/*| RETURN VALUE: un normlize data                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Y.Ise                                                    |*/
/*| Date        : 2009-06-10                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void dec_nmlz_mdhm(unsigned long n,unsigned short* year, unsigned char* month, unsigned char* day, unsigned char* hour, unsigned char* min)
{
// NOTE:�m�[�}���C�Y�f�[�^(ulong�^)�̍\���͈ȉ��̒ʂ�
// �N  �F2000�`2099   2099=833�����g�p�r�b�g��12bit
// �����F1/1�`12/31   12/31��12*32+31=19Fh���g�p�r�b�g��9bit
// �����F0:00�`23�F59 23:59��23*60+59=59Fh���g�p�r�b�g��11bit
// ���� �N   |���� |����
// bit  31-20|19-11|10-0
	unsigned long tmp;						/* �v�Z�p�̈� */

	*year = (unsigned char)(n >> 20);		/* �N���擾 */
	*year = (unsigned short)(n >> 20);		/* �N���擾 */
	tmp = (n >> 11) & 0x1FF;				/* �������擾 */
	*month = (unsigned char)(tmp / 32);		/* �����擾 */
	*day =  (unsigned char)(tmp % 32);		/* �����擾 */
	tmp = n & 0x07FF;						/* �������擾 */
	*hour = (unsigned char)(tmp / 60);		/* �����擾 */
	*min = (unsigned char)(tmp % 60);		/* �����擾 */
	return;									/* �����I�� */
}

/*[]----------------------------------------------------------------------[]*/
/*|             majorty(&indat[0],n)                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|       decide by majority whether the bits are high or low respectively |*/
/*[]----------------------------------------------------------------------[]*/
/*| Argument :  &indat[0] = address of input port data array               |*/
/*|                    n  = number of array elements : indat[n]            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Return :    input port value (bit= 0:Low Level, 1:High Level)          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  M.Mochizuki                                             |*/
/*| Date        :  2000- 3-16                                              |*/
/*| Update      :                                                          |*/
/*[]-------------------------------- Copyright(C) 2000,2001 AMANO Corp.---[]*/
unsigned char majorty(unsigned char *indat,unsigned char n)
{
    unsigned char  acvbit[8];   /* HIGH level count [0]-[7]=bit0-7 */
    unsigned char  k;           /* bit counter */
    unsigned char  bitmask;     /* check bit */
    unsigned char  i;           /* byte counter */

    memset(&acvbit[0],0,sizeof(acvbit));  /* buffer clear */
    bitmask = 0x01;  /* check bit initialize */

    k = 0;  /* counter initialize */
    do{
        i = 0;
        do{
            if( (indat[i] & bitmask ) != 0 ){  /* Port is high level ? (Y) */
                acvbit[k]++;  /* How many times did the bit k become high level ? */
            }
        }while( ++i < n );
        bitmask <<= 1;  /* next check bit */

    }while( ++k < sizeof(acvbit) );

    bitmask = 0x00;  /* bit k = 0:low level */

    k = sizeof(acvbit);
    do{
        k--;
        bitmask <<= 1;
        if( acvbit[k] >= ((n + 1) / 2) ){  /* decide by majority */
            bitmask |= 0x01;  /* bit k = 1:high level */
        }
    }while( k != 0 );

    return( bitmask );
}

/*[]----------------------------------------------------------------------[]*/
/*|             majomajo(&indat[0],n,prests)                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|  decide by like majority whether the bits are high or low respectively |*/
/*[]----------------------------------------------------------------------[]*/
/*| Argument :  &indat[0] = address of input port data array               |*/
/*|                    n  = number of array elements : indat[n]            |*/
/*|              prests   = previous status value                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Return :    input port value (bit= 0:Low Level, 1:High Level)          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  M.Okuda                                                 |*/
/*| Date        :  2000- 4-29                                              |*/
/*| Update      :                                                          |*/
/*[]-------------------------------- Copyright(C) 2000,2001 AMANO Corp.---[]*/
unsigned char majomajo(unsigned char *indat,unsigned char n,unsigned char prests)
{
    unsigned char  acvbit[8];   /* HIGH level count [0]-[7]=bit0-7 */
    unsigned char  k;           /* bit counter */
    unsigned char  bitmask;     /* check bit */
    unsigned char  i;           /* byte counter */
    unsigned char  bitresult;   /* result bit */

    memset(&acvbit[0],0,sizeof(acvbit));  /* buffer clear */
    bitmask = 0x01;  /* check bit initialize */

    /** High bit counts get by personal all bit **/
    k = 0;  /* counter initialize */
    do{
        i = 0;
        do{
            if( (indat[i] & bitmask ) != 0 ){  /* Port is high level ? (Y) */
                acvbit[k]++;  /* How many times did the bit k become high level ? */
            }
        }while( ++i < n );
        bitmask <<= 1;  /* next check bit */

    }while( ++k < sizeof(acvbit) );


    /** Magic algolithm for strong noise start **/
    k = 0;
    bitresult = 0;   /* result set area initial */
    bitmask = 0x01;  /* check bit initialize */
    do{
        /** previous status is low **/
        if( 0 == (prests & bitmask) ){  /* terget bit is low */
            if( (n - 1) <= acvbit[k] ){ /* decide low to high */
                bitresult |= bitmask;   /* high information set */
            }
        }

        /** previous status is high **/
        else{
            if( 1 < acvbit[k] ){        /* decide not high to low */
                bitresult |= bitmask;   /* high information set */
            }
        }
        bitmask <<= 1;  /* next target bit */
    }while( ++k < sizeof(acvbit) );

    return( bitresult );
}

/*[]----------------------------------------------------------------------[]*/
/*| Now Time Update                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME : ClkrecUpdate( BcdTime[6] )                               |*/
/*| PARAMETER   : [0] : Year upper (BCD : 19 or 20)                        |*/
/*|             : [1] : Year lower (BCD : 00-99)                           |*/
/*|             : [2] : Month      (BCD : 01-12)                           |*/
/*|             : [3] : Day        (BCD : 01-31)                           |*/
/*|             : [4] : Hour       (BCD : 00-23)                           |*/
/*|             : [5] : Minute     (BCD : 00-59)                           |*/
/*| RETURN VALUE: void  :                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : M.Okuda                                                  |*/
/*| Date        : 2002-10-11                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
void
ClkrecUpdate( unsigned char *BcdTime )
{
	union {
		ulong	ul;
		uchar	uc[4];
	} u_LifeTime;
	ulong	ist;
	struct	clk_rec wk_CLK_REC;				// ��M�d�����e(�o�C�i��)
	ushort	wk_CLK_REC_msec;				//							�~���b
	ushort	NormDay;						// �N�����m�[�}���C�Y�l
	ulong	NormTime;						// �����b�~���b�m�[�}���C�Y�l


	wk_CLK_REC.year =  (unsigned short)( bcdbin( BcdTime[0] ) * 100 );
	wk_CLK_REC.year += (unsigned short)( bcdbin( BcdTime[1] ) );
	wk_CLK_REC.mont = bcdbin( BcdTime[2] );
	wk_CLK_REC.date = bcdbin( BcdTime[3] );
	wk_CLK_REC.hour = bcdbin( BcdTime[4] );
	wk_CLK_REC.minu = bcdbin( BcdTime[5] );
	wk_CLK_REC.seco = bcdbin( BcdTime[6] );
	wk_CLK_REC_msec = 0;

	c_Normalize_ms( &wk_CLK_REC, &wk_CLK_REC_msec, &NormDay, &NormTime );	// �ݒ莞���m�[�}���C�Y�l(ms)get

	u_LifeTime.uc[0] = BcdTime[7];
	u_LifeTime.uc[1] = BcdTime[8];
	u_LifeTime.uc[2] = BcdTime[9];
	u_LifeTime.uc[3] = BcdTime[10];								// ���i�C�x���g��M���_��LifeTime�l get

	u_LifeTime.ul = c_2msPastTimeGet( u_LifeTime.ul );			// ���i�C�x���g��M���Ă���̌o�ߎ���(ms)get
	NormTime += u_LifeTime.ul;									// �o�ߎ��ԉ��Z

	// �␳�l���Z�������v�ݒ�l���A���m�[�}���C�Y
	c_UnNormalize_ms( &NormDay, &NormTime, &wk_CLK_REC, &wk_CLK_REC_msec );

	ist = _di2();												// ���荞�݋֎~
	CLK_REC = wk_CLK_REC;
	CLK_REC_msec = wk_CLK_REC_msec;

	CLK_REC.ndat = dnrmlzm( (short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date );
	CLK_REC.nmin = tnrmlz ( (short)0, (short)0, (short)CLK_REC.hour, (short)CLK_REC.minu );
	CLK_REC.week = (unsigned char)((CLK_REC.ndat + 6) % 7);
	_ei2( ist );												// ���荞�݋���
}

/*[]----------------------------------------------------------------------[]*/
/*| Now Time Update                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME : ClkrecUpdate( BcdTime[6] )                               |*/
/*| PARAMETER   : [0] : Year upper (BCD : 19 or 20)                        |*/
/*|             : [1] : Year lower (BCD : 00-99)                           |*/
/*|             : [2] : Month      (BCD : 01-12)                           |*/
/*|             : [3] : Day        (BCD : 01-31)                           |*/
/*|             : [4] : Hour       (BCD : 00-23)                           |*/
/*|             : [5] : Minute     (BCD : 00-59)                           |*/
/*| RETURN VALUE: void  :                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : M.Okuda                                                  |*/
/*| Date        : 2002-10-11                                               |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
void ClkrecUpdateFromRTC( struct	RTC_rec *Time )
{
	date_time_rec2	rtcTime;
	
	rtcTime.Year = (unsigned short)( bcdbin( Time->year ) + 2000);
	rtcTime.Mon  = bcdbin( Time->month );
	rtcTime.Day  = bcdbin( Time->day );
	rtcTime.Hour = bcdbin( Time->hour );
	rtcTime.Min  = bcdbin( Time->min );
	rtcTime.Sec  = (unsigned short)bcdbin( Time->sec );
	
	if(0 == chkdate2( (short)rtcTime.Year, (short)rtcTime.Mon, (short)rtcTime.Day) &&
			(rtcTime.Hour < 24) && (rtcTime.Min < 60) && (rtcTime.Sec < 60) ) {
		// RTC����̎������L���Ȃ�ݒ肷��
		CLK_REC.year = rtcTime.Year;
		CLK_REC.mont = rtcTime.Mon;
		CLK_REC.date = rtcTime.Day;
		CLK_REC.hour = rtcTime.Hour;
		CLK_REC.minu = rtcTime.Min;
		CLK_REC.seco = (unsigned char)rtcTime.Sec;
		CLK_REC.ndat = dnrmlzm( (short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date );
		CLK_REC.nmin = tnrmlz ( (short)0, (short)0, (short)CLK_REC.hour, (short)CLK_REC.minu );
		CLK_REC.week = (unsigned char)((CLK_REC.ndat + 6) % 7);
	}
	else {
		// RTC����̎����������ȏꍇ�͌��ݎ�����ݒ肷��
		timset(&CLK_REC);
	}
}

void ChgTimedata_RTCBCD( struct	RTC_rec *Time, unsigned char *BCDTime)
{
	BCDTime[0] = (unsigned char)(( ( bcdbin( Time->year )  + 2000)/100));
	BCDTime[0] = binbcd(BCDTime[0]);
	BCDTime[1] = (unsigned char)( bcdbin( Time->year ) % 100);
	BCDTime[1] = binbcd(BCDTime[1]);
	BCDTime[2] = Time->month;
	BCDTime[3] = Time->day;
	BCDTime[4] = Time->hour;
	BCDTime[5] = Time->min;
	BCDTime[6] = Time->sec;
}

/*[]----------------------------------------------------------------------[]*/
/*| ���Z�@���v�ǂݏo���ims�܂Łj�@                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : �l���Z�b�g����|�C���^                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : okuda                                                   |*/
/*| Date         : 2009/05/12                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
void	c_Now_CLK_REC_ms_Read( struct clk_rec *pCLK_REC, ushort *pCLK_REC_msec )
{
	ulong	ist;

	ist = _di2();													// ���荞�݋֎~
	memcpy( pCLK_REC, &CLK_REC, sizeof(CLK_REC) );
	*pCLK_REC_msec = CLK_REC_msec;									// ���ݎ�get
	_ei2( ist );
}
/*[]----------------------------------------------------------------------[]*/
/*| �L���N���������b�~���b�`�F�b�N                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : ���v�f�[�^			�@                                 |*/
/*| RETURN VALUE : 1=OK, 0=NG                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| �N�̗L���͈͂� 2000�`2050�Ƃ���										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : okuda                                                   |*/
/*| Date         : 2009/05/12                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
uchar	c_ClkCheck( struct clk_rec *pCLK_REC, ushort *pCLK_REC_msec )
{
	if( (pCLK_REC->year < 2000) || (2050 < pCLK_REC->year) ){
		return	(uchar)0;											// NG
	}

	if( (pCLK_REC->mont < 1) || (12 < pCLK_REC->mont) ){
		return	(uchar)0;											// NG
	}

	if( (pCLK_REC->date < 1) || (OPWEEK[pCLK_REC->mont - 1] < pCLK_REC->date) ){
		return	(uchar)0;											// NG
	}

	if( (pCLK_REC->mont == 2) && ((pCLK_REC->year % 4) != 0) ){		// 2���ł��邤�N�łȂ�
		if( 28 < pCLK_REC->date ){
			return	(uchar)0;										// NG
		}
	}

	if( 23 < pCLK_REC->hour ){
		return	(uchar)0;											// NG
	}

	if( 59 < pCLK_REC->minu ){
		return	(uchar)0;											// NG
	}

	if( 59 < pCLK_REC->seco ){
		return	(uchar)0;											// NG
	}

	if( 999 < *pCLK_REC_msec ){
		return	(uchar)0;											// NG
	}

	return	(uchar)1;												// OK
}

/*[]----------------------------------------------------------------------[]*/
/*| �N���������b�~���b�@�m�[�}���C�Y                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : ���Q�Ɓ@�@			�@                                 |*/
/*| RETURN VALUE : none													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| �L���N���������b�~���b�`�F�b�N c_ClkCheck() ���Call���邱��		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : okuda                                                   |*/
/*| Date         : 2009/05/12                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
void	c_Normalize_ms( 
			struct clk_rec *pCLK_REC, 	// input :�����ԁi�N���������b�j
			ushort *pCLK_REC_msec,		// input :�����ԁi�~���b�j
			ushort *pNormDay,			// output:���P�ʐ��K���l
			ulong  *pNormTime )			// output:�~���b�P�ʐ��K���l
{
	ulong	wkul;

	*pNormDay = dnrmlzm( (short)pCLK_REC->year, (short)pCLK_REC->mont, (short)pCLK_REC->date );	// ���P�ʐ��K���lget

	// �����b�~���b ���K��
	wkul = (ulong)tnrmlz( 0, 0, (short)pCLK_REC->hour, (short)pCLK_REC->minu );	// ����
	wkul *= 60;																	// �P�ʂ�b��
	wkul += (ulong)pCLK_REC->seco;												// �b���Z
	wkul *= 1000;																// �P�ʂ��~���b��
	*pNormTime = wkul + *pCLK_REC_msec;											// (Max:86,399.999)
}

/*[]----------------------------------------------------------------------[]*/
/*| �N���������b�~���b�@�A���m�[�}���C�Y                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : ���v�f�[�^			�@                                 |*/
/*| RETURN VALUE : 1=OK, 0=NG                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| c_Normalize_ms() �Ńm�[�}���C�Y�����l�̋t�ϊ�						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : okuda                                                   |*/
/*| Date         : 2009/05/12                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
void	c_UnNormalize_ms( 
			ushort *pNormDay,			// input :���P�ʐ��K���l
			ulong  *pNormTime,			// input :�~���b�P�ʐ��K���l
			struct clk_rec *pCLK_REC, 	// output:���v�`���f�[�^�i�N���������b�j
			ushort *pCLK_REC_msec )		// output:�i�~���b�j
{
	ushort	NormDay;
	ulong	NormTime;
	short	s1,s2;

	NormDay = *pNormDay;
	NormTime = *pNormTime;

	if( NormTime >= _1DAY_TIME_MILLI_SEC ){						// 1������
		NormDay += (ushort)(NormTime / _1DAY_TIME_MILLI_SEC);	// ���ߓ���get
		NormTime = NormTime % _1DAY_TIME_MILLI_SEC;				// 1�����̐��K���lget
	}

	*pCLK_REC_msec = (ushort)(NormTime % 1000);					// �~���b�lset
	NormTime /= 1000;											// NormTime�̒P�� = �b

	pCLK_REC->seco = (uchar)(NormTime % 60);					// �b�lset
	NormTime /= 60;												// NormTime�̒P�� = ��

	idnrmlzm( NormDay, (short*)(&pCLK_REC->year), &s1, &s2 );	// �N�����A���m�[�}���C�Y
	pCLK_REC->mont = (uchar)s1;
	pCLK_REC->date = (uchar)s2;

	itnrmlz ( (short)NormTime, 0, 0, &s1, &s2 );				// �����A���m�[�}���C�Y
	pCLK_REC->hour = (uchar)s1;
	pCLK_REC->minu = (uchar)s2;
}

/*[]----------------------------------------------------------------------[]*/
/*| clk_rec�^ �N���������b�f�[�^����A�b�P�ʂ̃m�[�}���C�Y�lget            |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : ���v�f�[�^			�@                                 |*/
/*| RETURN VALUE : �m�[�}���C�Y�l                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| �N�̗L���͈͂� 1980�N3��1������̌o�ߕb								   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : okuda                                                   |*/
/*| Date         : 2009/05/12                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
ulong	c_Normalize_sec( struct clk_rec *pCLK_REC )
{
	ulong	wkul;

	wkul = (ulong)dnrmlzm( (short)pCLK_REC->year, (short)pCLK_REC->mont, (short)pCLK_REC->date );	// ���P�ʐ��K���lget
	wkul *= 1440L;		// �P�ʂ𕪂�

	wkul += (ulong)tnrmlz( 0, 0, (short)pCLK_REC->hour, (short)pCLK_REC->minu );	// ���� ���K��
	wkul *= 60;																		// �P�ʂ�b��

	wkul += (ulong)pCLK_REC->seco;			// �b�����Z

	return	wkul;
}

/*[]----------------------------------------------------------------------[]*/
/*| �b�P�ʂ̃m�[�}���C�Y�l����clk_rec�^ �N���������b�f�[�^get              |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : ���v�f�[�^			�@                                 |*/
/*|				   �Z�b�gclk_rec�^�f�[�^�|�C���^						   |*/
/*| RETURN VALUE : none													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| c_Normalize_sec() �ō쐬�����m�[�}���C�Y�l��clk_rec�^�ɕϊ�����		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : okuda                                                   |*/
/*| Date         : 2009/05/12                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
void	c_UnNormalize_sec( ulong NormSec, struct clk_rec *pCLK_REC )
{
	ushort	NormDay, NormMin;
	short	s1,s2;

	pCLK_REC->seco = (uchar)(NormSec % 60);
	NormSec /= 60;

	NormMin = (ushort)(NormSec % 1440L);
	NormDay = (ushort)(NormSec / 1440L);

	idnrmlzm( NormDay, (short*)(&pCLK_REC->year), &s1, &s2 );	// �N�����A���m�[�}���C�Y
	pCLK_REC->mont = (uchar)s1;
	pCLK_REC->date = (uchar)s2;

	itnrmlz ( (short)NormMin, 0, 0, &s1, &s2 );					// �����A���m�[�}���C�Y
	pCLK_REC->hour = (uchar)s1;
	pCLK_REC->minu = (uchar)s2;
}

/********************************************************************/
/*                       C MODULE NOTE                              */
/********************************************************************/
/* SYSTEM         :                                                 */
/* MODULE NAME    : chkdate    VERSION : 1.00                       */
/* COPYRIGHT      : chkdate    COPYRIGHT AMANO CORP. 1987           */
/* LANGUAGE       : C                                               */
/* CONTENTS       : check day data                                  */
/*                                                                  */
/*==================================================================*/
/* CALL METHOD    : chkdate(yyyy,mm,dd)                             */
/*------------------------------------------------------------------*/
/* INPUT   ARGUMENT                                                 */
/*     ARG.NAME          TYPE        CONTENTS                       */
/*  1: yyyy              int         data year                      */
/*  2: mm                int         data month                     */
/*  3: dd                int         data day                       */
/*  4:                                                              */
/*  5:                                                              */
/*                                                                  */
/* FUNCTION  ARGUMENT                                               */
/*                       TYPE        CONTENTS                       */
/*                       int         OK:0 NG:-1                     */
/*                                                                  */
/*==================================================================*/
/* AUTHOR         : T.ASANUMA                                       */
/* DATE           : 1987 10 29                                      */
/*                                                                  */
/* UP-DATE                                                          */
/*  No.  DATE        CONTENTS                                       */
/*  1.   YYYY MM DD                                                 */
/*  2.   YYYY MM DD                                                 */
/*  3.   YYYY MM DD                                                 */
/*                                                                  */
/*==================================================================*/
/**/
/*EXTERNAL MEM & DATA     */


/*EXTERNAL PROCEDURE      */

/*DEFINE                  */

/*INCLUDE FILES           */
short chkdate(short yyyy,short mm,short dd)
{
	/* hutei-koumoku mae-shori */
	if(yyyy == 0){
		yyyy = 1980;	/* uruu-doshi wo set */
	}
	if(mm == 0){
		mm = 1;		/* dai-no-tsuki wo set */
	}
	if(dd == 0){
		dd = 1;		/* saishou-chi wo set */
	}

	/* data  check */
	if((yyyy < 1980) || (yyyy > 2079)){
		return(-1);		/* YEAR  ERROR */
	}
	if((mm < 1) || (mm > 12)){
		return(-1);		/* MONTH ERROR */
	}
	if((dd < 1) || (dd > medget(yyyy,mm))){
		return(-1);		/* DAY   ERROR */
	}

	return(0);	/* NON ERROR */
}

/********************************************************************/
/*                       C MODULE NOTE                              */
/********************************************************************/
/* SYSTEM         :                                                 */
/* MODULE NAME    : youbiget   VERSION : 1.00                       */
/* COPYRIGHT      : youbiget   COPYRIGHT AMANO CORP. 1987           */
/* LANGUAGE       : Microsoft C (Ver.4)                             */
/* CONTENTS       : DATE(YEAR,MONTH,DAY) to YOUBI-CODE HENKAN       */
/*                  <YEAR 4 Digit>                                  */
/*==================================================================*/
/* CALL METHOD    : youbiget(yyyy,mm,dd)                            */
/*------------------------------------------------------------------*/
/* INPUT   ARGUMENT                                                 */
/*     ARG.NAME          TYPE        CONTENTS                       */
/*  1: yyyy              int         DATE(YEAR)  4-Digit            */
/*  2: mm                int         DATE(MONTH) 2-Digit            */
/*  3: dd                int         DATE(DAY)   2-Digit            */
/*  4:                                                              */
/*  5:                                                              */
/*                                                                  */
/* FUNCTION  ARGUMENT                                               */
/*                       int         YOUBI-CODE                     */
/*                                   (SUN:0,MON:1,,,,,,SAT:6)       */
/*                                                                  */
/*==================================================================*/
/* AUTHOR         : TAKESHI ASANUMA                                 */
/* DATE           : 1987 11 11                                      */
/*                                                                  */
/* UP-DATE                                                          */
/*  No.  DATE        CONTENTS                                       */
/*  1.   YYYY MM DD                                                 */
/*  2.   YYYY MM DD                                                 */
/*  3.   YYYY MM DD                                                 */
/*                                                                  */
/*==================================================================*/
/**/

short youbiget(short yyyy,short mm,short dd)
{
	short			youbi;
	unsigned short	ndat;
	
	ndat = dnrmlzm(yyyy,mm,dd);	/* NORMALIZE DATA GET */

	/* NORMALIZE DATA to YOUBI CODE */
	youbi = (ndat+6)%7;

	return(youbi);
}

/* CRC16-CCITT �Z�o�֐� (�E�V�t�g���g�p����) */									/*								*/
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
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�ECRC�̌v�Z���s��																						   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	crc_ccitt(																/*								*/
ushort	length,																	/*								*/
uchar	cbuf[],																	/*								*/
uchar	*result,																/*								*/
uchar	type )																	/*								*/
{																				/*								*/
	ushort	i, j;																/*								*/
	uchar	crc_wk;																/*								*/
	ushort	crc;																/*								*/
																				/*								*/
	crc		= 0x0000;															/*								*/
	crc_wk	= 0x0000;															/*								*/
																				/*								*/
	switch( type ){																/*								*/
																				/*								*/
	case 0: 	/* ���V�t�g	*/													/*								*/
		for(i = 0; i < length; i++) {											/*								*/
			crc ^= (ushort)cbuf[i] << (16 - CHAR_BIT);							/*								*/
			for (j = 0; j < CHAR_BIT; j++){										/*								*/
				if (crc & 0x8000){												/*								*/
					crc = (crc << 1) ^ CRCPOLY1;								/*								*/
				}else{															/*								*/
					crc <<= 1;													/*								*/
				}																/*								*/
			}																	/*								*/
		}																		/*								*/
		break;																	/*								*/
	case 1: 	/* �E�V�t�g	*/													/*								*/
		for (i = 0; i < length; i++) {											/*								*/
			crc ^= cbuf[i];														/*								*/
			for (j = 0; j < CHAR_BIT; j++){										/*								*/
				if (crc & 1){													/*								*/
					crc = (crc >> 1) ^ CRCPOLY2;								/*								*/
				}else{															/*								*/
					crc >>= 1;													/*								*/
				}																/*								*/
			}																	/*								*/
		}																		/*								*/
		break;																	/*								*/
	}																			/*								*/
	crc_wk = (uchar)(crc & 0x00ff);												/*								*/
	*result = crc_wk;															/*								*/
																				/*								*/
	crc_wk = (uchar)((crc & 0xff00) >> CHAR_BIT);								/*								*/
	*(result+1) = crc_wk;														/*								*/
																				/*								*/
}																				/*								*/

// GG120600(S) // Phase9 CRC�`�F�b�N
/* CRC16-CCITT �Z�o�֐� (�E�V�t�g���g�p����) */
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	Calculate CRC-CCITT	(Update)																			   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME  : crc_ccitt( len, cbuf, result, type )														   |*/
/*| PARAMETER    : len   : Data length																		   |*/
/*|              : cbuf  : Char data																		   |*/
/*|				 : result: Crc																				   |*/
/*|				 : type  : 0: left shift 1:rigth shift														   |*/
/*| RETURN VALUE : void																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: 																							   |*/
/*| Date		: 																							   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�ECRC�̌v�Z���s�� (�O��̌v�Z�l�������p��)																   |*/
/*|	�E����̌v�Z����result��0�ŃN���A���Ă�������															   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2020 AMANO Corp.------[]*/
void	crc_ccitt_update(
ushort	length,
uchar	cbuf[],
uchar	*result,
uchar	type )
{
#ifdef USE_CRC_CALC
	ushort i;
	ushort	crc;
	
	wai_sem( ID_CRC_CALC );
	
	CRC.CRCCR.BIT.GPS = 3;
	if( type == L_SHIFT ){
		CRC.CRCCR.BIT.LMS = 1;
	} else {
		CRC.CRCCR.BIT.LMS = 0;
	}
	CRC.CRCCR.BIT.DORCLR = 1;

	// �O��v�Z�����l���o�̓��W�X�^�ɃZ�b�g
	crc = (((ushort)(*(result + 1)) << CHAR_BIT) & 0xFF00) | (ushort)(*result) & 0xFF;
	CRC.CRCDOR = crc;
	
	for( i = 0; i < length; i++ ){
		CRC.CRCDIR = *cbuf++;
	}
	
	crc = CRC.CRCDOR;
	
	sig_sem( ID_CRC_CALC );
	
	*result =  (uchar)crc;
	*(result+1) = (uchar)(crc >> CHAR_BIT);
	
#else
	ushort	i, j;
	uchar	crc_wk;
	ushort	crc;

	// �O��v�Z�����l��ݒ�
	crc_wk  = *(result + 1);
	crc		= ((ushort)crc_wk << CHAR_BIT) & 0xFF00;
	crc_wk	= *result;
	crc		= crc + (ushort)crc_wk;

	switch( type ){

	case L_SHIFT: 	/* ���V�t�g	*/
		for(i = 0; i < length; i++) {
			crc ^= (ushort)cbuf[i] << (16 - CHAR_BIT);
			for (j = 0; j < CHAR_BIT; j++){
				if (crc & 0x8000){
					crc = (crc << 1) ^ CRCPOLY1;
				}else{
					crc <<= 1;
				}
			}
		}
		break;
	case R_SHIFT: 	/* �E�V�t�g	*/
		for (i = 0; i < length; i++) {
			crc ^= (ushort)cbuf[i];
			for (j = 0; j < CHAR_BIT; j++){
				if (crc & 1){
					crc = (crc >> 1) ^ CRCPOLY2;
				}else{
					crc >>= 1;
				}
			}
		}
		break;
	}
	crc_wk = (uchar)(crc & 0x00ff);
	*result = crc_wk;
	crc_wk = (uchar)((crc & 0xff00) >> CHAR_BIT);
	*(result+1) = crc_wk;
#endif
}
// GG120600(E) // Phase9 CRC�`�F�b�N

/*[]-----------------------------------------------------------------------[]*/
/*|	8�����܂ł�ASCII������i16�i����:'0'�`'F'�j�� �o�C�i��(long)�ɕϊ�		|*/
/*|																			|*/
/*|	�w�蕶����16�i�\�L�ȊO�̏ꍇ�̓G���[��Ԃ��B							|*/
/*|	������������̏ꍇ�͕␳����iOK�Ƃ���j�B							|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : pAsc = �ϊ���Ascii������ւ̃|�C���^						|*/
/*|				   Length = pAsc �̕�����									|*/
/*|				   pBin = �ϊ���f�[�^�Z�b�g�G���A�ւ̃|�C���^				|*/
/*| RETURN VALUE : 1=OK(�ϊ�����)�A0=NG(�f�[�^�ُ�)							|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	HexAsc_to_LongBin_withCheck( uchar *pAsc, uchar Length, ulong *pBin )
{
	uchar	i;
	uchar	dat;
	ushort	BinDat;
	uchar	wkAsc[10];
	ulong	RtnValue;

	if( 8 < Length ){						/* length over error (Y) */
		return	(uchar)0;
	}

	/* �����������ɐ��` */
	if( 0 != (Length & 1) ){				/* length odd (Y) */
		wkAsc[0] = '0';
		memcpy( &wkAsc[1], &pAsc[0], (size_t)Length );
		++Length;
	}
	else{
		memcpy( &wkAsc[0], &pAsc[0], (size_t)Length );
	}

	RtnValue = 0;

	for( i=0; i<Length; ++i ){				/* half byte���ɕϊ� */

		dat = wkAsc[i];

		if( ('0' <= dat) && (dat <= '9') ){
			BinDat = dat-'0';
		}
		else if( ('A' <= dat) && (dat <= 'F') ){
			BinDat = dat-'A'+10;
		}
		else if( ('a' <= dat) && (dat <= 'f') ){
			BinDat = dat-'a'+10;
		}
		else{
			return	(uchar)0;				/* Ascii data NG */
		}

		RtnValue <<= 4;
		RtnValue |= (ulong)BinDat;
	}

	*pBin = RtnValue;
	return	(uchar)1;						/* OK */
}

/*[]-----------------------------------------------------------------------[]*/
/*|	10�����܂ł�ASCII������i10�i����:'0'�`'9'�j�� �o�C�i��(long)�ɕϊ�		|*/
/*|																			|*/
/*|	�w�蕶����10�i�\�L�ȊO�̏ꍇ�̓G���[��Ԃ��B							|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : pAsc = �ϊ���Ascii������ւ̃|�C���^						|*/
/*|				   Length = pAsc �̕�����									|*/
/*|				   pBin = �ϊ���f�[�^�Z�b�g�G���A�ւ̃|�C���^				|*/
/*| RETURN VALUE : 1=OK(�ϊ�����)�A0=NG(�f�[�^�ُ�)							|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	DeciAsc_to_LongBin_withCheck( uchar *pAsc, uchar Length, ulong *pBin )
{
	uchar	i;
	uchar	dat;
	ushort	BinDat;
	ulong	RtnValue;

	if( 10 < Length ){						/* length over error (Y) */
		return	(uchar)0;
	}

	RtnValue = 0;

	for( i=0; i<Length; ++i ){				/* half byte���ɕϊ� */

		dat = pAsc[i];

		if( ('0' <= dat) && (dat <= '9') ){
			BinDat = dat-'0';
		}
		else{
			return	(uchar)0;				/* Ascii data NG */
		}

		RtnValue *= 10;
		RtnValue += (ulong)BinDat;
	}

	*pBin = RtnValue;
	return	(uchar)1;						/* OK */
}

/*[]----------------------------------------------------------------------[]*/
/*| Calculate SUM (unsigned short �P�ʂ̘a�Z�T�����Z)                      |*/
/*|																		   |*/
/*|	�{�֐��ɂ͈ȉ��̏���������B�K���������������Call���邱�ƁB		   |*/
/*|	�@ �����O�X�͋������ł��邱�ƁB										   |*/
/*| �A ���Z���G���A�̐擪�A�h���X�͋����Ԓn�ł��邱�ƁB					   |*/
/*| �� �������x����̂��߃G���[�`�F�b�N�͍s���܂���B					   |*/
/*|    �g�p���_�ŏ[�����ӂ��Ă��������B									   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : sumcal_ushort( stad, nu )                               |*/
/*| PARAMETER    : stad : Start address of string                          |*/
/*|                nu;  : Number of string charactors (�o�C�g��)           |*/
/*| RETURN VALUE : �T���l                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2005-06-27                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	sumcal_ushort( unsigned short *stad, unsigned short nu )
{
	unsigned short i;
	unsigned short c;

	nu >>= 1;
	c=0;
	for( i=0; i<nu; ++i ){
		c += stad[i];
	}
	return( c );
}

/*[]----------------------------------------------------------------------[]*/
/*| �������[�u���b�N��r												   |*/
/*|																		   |*/
/*|	���[�h�A�N�Z�X�ł� memcmp() �����֐�			                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : sumcal_ushort( stad, nu )                               |*/
/*| PARAMETER    : pSrc1/2 : Start address of string                       |*/
/*|                Length  : Number of string charactors (�o�C�g��)        |*/
/*| RETURN VALUE : 0=OK, 1=NG                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2005-06-27                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned char	memcmp_w( const unsigned short *pSrc1, const unsigned short *pSrc2, unsigned long Length )
{
	unsigned long	i;

	Length >>= 1;							/* Length 1/2 */
	for( i=0; i < Length; ++i ){
		if( pSrc1[i] != pSrc2[i] ){
			return	(unsigned char)1;		/* NG */
		}
	}
	return	(unsigned char)0;				/* OK */
}

/*[]-----------------------------------------------------------------------[]*/
/*|	10�����܂ł�ASCII������i10�i����:'0'�`'9'�j�� �o�C�i��(long)�ɕϊ�		|*/
/*|																			|*/
/*|	�w�蕶����10�i�\�L�ȊO�̏ꍇ�̓G���[��Ԃ��B							|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : pAsc = �ϊ���Ascii������ւ̃|�C���^						|*/
/*|				   Length = pAsc �̕�����									|*/
/*|				   pBin = �ϊ���f�[�^�Z�b�g�G���A�ւ̃|�C���^				|*/
/*| RETURN VALUE : 1=OK(�ϊ�����)�A0=NG(�f�[�^�ُ�)							|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	DecAsc_to_LongBin_withCheck( uchar *pAsc, uchar Length, ulong *pBin )
{
	uchar	i;

	if( 10 < Length ){						/* length over error (Y) */
		return	(uchar)0;
	}

	/* �����`�F�b�N */
	for( i=0; i<Length; ++i ){

		if( ('0' <= pAsc[i]) && (pAsc[i] <= '9') ){
			;	/* OK */
		}
		else{
			return	(uchar)0;				/* NG */
		}
	}

	*pBin = astoinl( pAsc, (short)Length );	/* Decimal to Ascii change */
	return	(uchar)1;						/* OK */
}

/*[]----------------------------------------------------------------------[]*
 *| get bitmap data length
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : get_bmplen
 *| PARAMETER    : bmp - bmp data
 *| RETURN VALUE : data length
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ulong	get_bmplen(uchar *bmp)
{
	ulong len;
	
	/* set length(little endian) */
	len =	bmp[2];
	len +=	((ulong)bmp[3] << (8*1));
	len +=	((ulong)bmp[4] << (8*2));
	len +=	((ulong)bmp[5] << (8*3));
	
	return len;
}

/*[]----------------------------------------------------------------------[]*
 *| bcd(1byte) --> ascii(1byte)
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : bcdtoascii
 *| PARAMETER    : bcd(1byte)
 *|              : type(=0 / !=0)
 *| RETURN VALUE : ascii(1byte)
 *| ex) bcd=0x19 (type=0)-->ascii=0x31 / (type!=0)-->ascii=0x39
 *[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
uchar	bcdtoascii( uchar bcd, ushort type )
{
	uchar	ascii;

	if( type == 0 ) {
		bcd >>= 4;
		ascii = bcd;
	} else {
		bcd  &= 0x0f;
		ascii = bcd;
	}
	ascii |= 0x30;

	return(ascii);
}

/*[]-----------------------------------------------------------------------[]*/
/*|	BCD��ASCII������i10�i����:'0'�`'9'�j�ɕϊ�								|*/
/*|																			|*/
/*|	BCD�͌��l�߂ŋ󂫂ɂ�0x0���i�[����Ă��邱�ƁB						|*/
/*|	ex) BCD=(00|00|00|00|00|00|00|00|01|23) --> ASCII=(31|32|33) 			|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : bcd = �ϊ���BCD�ւ̃|�C���^								|*/
/*|				 : ascii  = �ϊ���f�[�^�Z�b�g�G���A�ւ̃|�C���^			|*/
/*|				   len = BCD�̊i�[�̈�(char[])�T�C�Y						|*/
/*| RETURN VALUE : �Ȃ�														|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Date         : 2006/02/22												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	BCDtoASCII( uchar *bcd, uchar *ascii, ushort len )
{
	ushort	i,j;
	uchar	w_bcd, w_ascii;

	for( i=0,j=0; i<len; i++) {
		w_bcd = bcd[i];
		w_ascii = (uchar)bcdtoascii(w_bcd, 0);
		ascii[j] = w_ascii;
		j++;
		w_ascii = bcdtoascii(w_bcd, 1);
		ascii[j] = w_ascii;
		j++;
	}
	return;
}

/*[]-----------------------------------------------------------------------[]*/
/*|	10�����܂ł�ASCII������i10�i����:'0'�`'9'�j�� �o�C�i��(long)�ɕϊ�		|*/
/*|	�����ɑΉ�																|*/
/*|	�w�蕶����10�i�\�L�ȊO�̏ꍇ�̓G���[��Ԃ��B							|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : pAsc = �ϊ���Ascii������ւ̃|�C���^						|*/
/*|				   Length = pAsc �̕�����									|*/
/*|				   pBin = �ϊ���f�[�^�Z�b�g�G���A�ւ̃|�C���^				|*/
/*| RETURN VALUE : 1=OK(�ϊ�����)�A0=NG(�f�[�^�ُ�)							|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Namioka													|*/
/*| Date         : 2009/06/19												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	DecAsc_to_LongBin_Minus( uchar *pAsc, uchar Length, long *pBin )
{
	uchar	i;
	uchar	minus=0;
	
	if( 10 < Length ){						/* length over error (Y) */
		return	(uchar)0;
	}

	if( '-' == pAsc[0] ){					// �ް��̐擪���}�C�i�X�H
		minus = 1;							// �t���O�Z�b�g
		pAsc++;								// �|�C���^��i�߂�
		Length--;							// �����T�C�Y���Z�o
	}
	/* �����`�F�b�N */
	for( i=0; i<Length; ++i ){

		if( ('0' <= pAsc[i]) && (pAsc[i] <= '9') ){
			;	/* OK */
		}
		else{								// �擪�ȊO�ɐ����ȊO��NG�Ƃ���
			return	(uchar)0;				/* NG */
		}
	}
	*pBin = (long)astoinl( pAsc, (short)Length );	/* Decimal to Ascii change */
	if( minus ){
		*pBin = 0-*pBin;
	}
	return	(uchar)1;						/* OK */
}
//------------------------------------------------------------------------------------------
// �[�N�`�F�b�N.
//
// Param:
//  year(in) : ���� 2011 ��.
// Return:
//  0=���N 1=�[�N .
//------------------------------------------------------------------------------------------
unsigned char	check_uru ( unsigned short year )
{
	unsigned char		uru;
	uru = 0;
	if((year % 4) == 0 && (year % 100) != 0) uru = 1;
	if((year % 400) == 0) uru = 1;
	return uru;
}

//------------------------------------------------------------------------------------------
const unsigned char mon_max_tbl[2][12] ={ 
	{31,28,31,30,31,30,31,31,30,31,30,31},
	{31,29,31,30,31,30,31,31,30,31,30,31}
};
//------------------------------------------------------------------------------------------
// �N�����ɂ��������Z�� -1 ������
//
// Param:
//  clk_date(in/out) : �N���� (�����͖��g�p) : �������`�F�b�N��.
//  add_val(in)      : ���Z���錎�� 1�`n
//------------------------------------------------------------------------------------------
void add_month ( struct clk_rec *clk_date, unsigned short add_val )
{
	unsigned char		uru;
	unsigned char		day;

	if(clk_date->date == 1){ //1���`�̏ꍇ�͌����Z�l�͂P���Ȃ�
		add_val--;
	}
	clk_date->mont += (unsigned char)add_val;		//���Z

	if(clk_date->mont > 12){		//�N�ς��
		clk_date->mont --;
		clk_date->year += (unsigned char)(clk_date->mont / 12);
		clk_date->mont =  (unsigned char)((clk_date->mont % 12) + 1);
	}
	//�N�[�N�`�F�b�N
	uru = check_uru(clk_date->year);

	day = (unsigned char)(clk_date->date - 1);		//�I����
	if( day == 0 || day > mon_max_tbl[ uru ][ clk_date->mont-1 ] ){ //�I������0(=���̌��̖����܂�)@@1@@ .or.�������z���Ă���ꍇ
		day = mon_max_tbl[ uru ][ clk_date->mont-1 ];
	}
	clk_date->date = day;
}
/*[]----------------------------------------------------------------------[]*/
/*|             c_arraytoint32                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�o�C�g��̐��l�� 32bit���l�֕ϊ�����B						       |*/
/*|		�E�o�C�g��̐擪���������E�ł͂Ȃ��ꍇ���l�������֐��ł��B	       |*/
/*|		�E�o�C�g��̐���4�o�C�g�𒴂������ʌ��͐؂�̂Ă܂��B		       |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : buf[] = �o�C�g��ւ̃|�C���^			 					   |*/
/*|			 size  = �o�C�g��̐��i"buf[]"�̒����j						   |*/
/*|	return : 32bit���l�i�ϊ����ʁj										   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	��j3�o�C�g��̐��l�� 32bit���l�֕ϊ�����ꍇ�B					       |*/
/*|		buf[0] 0x01													       |*/
/*|		buf[1]=0x23													       |*/
/*|		buf[2]=0x45													       |*/
/*|		  size=3													       |*/
/*|	0x00012345=c_arraytoint32(buf, size)							       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  ����                                                    |*/
/*| Date        :  2005-02-04                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned long	c_arraytoint32(unsigned char array[], short size)
{
    short			ni;
    unsigned long	value32;

	if (size < 1) {
		return 0L;
	}
	value32 = (unsigned long)array[0];
    for (ni=1; ni < size; ni++) {
        value32 <<= 8;
        value32  |= (unsigned long)array[ni];
    }

    return value32;
}
/*[]----------------------------------------------------------------------[]*/
/*|             c_int32toarray                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*|		32bit���l�� �o�C�g��̐��l�֕ϊ�����B						       |*/
/*|		�E�o�C�g��̐擪���������E�ł͂Ȃ��ꍇ���l�������֐��ł��B	       |*/
/*|		�E�o�C�g��̐���4�o�C�g�ɖ����Ȃ��ꍇ�A�ϊ��l��ʌ��͐؂�̂Ă܂��B|*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : buf[] = �o�C�g��ւ̃|�C���^			 					   |*/
/*|			 data  = �ϊ��Ώےl�i32bit���l�j							   |*/
/*|			 size  = �o�C�g��̐��i"buf[]"�̒����j						   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	��j3�o�C�g��̐��l�֕ϊ�����ꍇ�B								       |*/
/*|		  data=0x00987654											       |*/
/*|		  size=3													       |*/
/*|	c_int32toarray(buf, data, size)									       |*/
/*|		buf[0] 0x98													       |*/
/*|		buf[1]=0x76													       |*/
/*|		buf[2]=0x54													       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  ����                                                    |*/
/*| Date        :  2005-02-04                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	c_int32toarray(unsigned char array[], unsigned long data, short size)
{
    short			ni;

    for (ni=0; ni < size; ni++) {
		array[ni] = 0;
	}

	if (size < 1) {
		return;
	}
	size--;						/* �o�C�g��̖������疄�ߎn�߂܂��B*/
    for (ni=1; ni < 4; ni++) {	/* 4�o�C�g�Ƃ́A�ϊ��Ώےl�i32bit���l�j�ł��B*/
        array[size--] = (unsigned char)(data & 0x000000ff);
        if (size < 0) {
			return;
		}
        data >>= 8;
    }
}
/*[]----------------------------------------------------------------------[]*/
/*|             CheckCRC() 				                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		CRC�`�F�b�N														   |*/
/*[]----------------------------------------------------------------------[]*/
/*|	return : CRC�`�F�b�N���ʁiBoolean�j									   |*/
/*|				1=��v													   |*/
/*|				0=�s��v�i�d���ُ�j									   |*/
/*|                                                                        |*/
/*|	note :	DIP-SW(SW2-6��ON�Ȃ�)�m�[�`�F�b�N�ŏ�Ɂu1=TRUE�v��Ԃ��܂��B  |*/
/*|			�f�o�b�O���A���Z�@�V�~�����[�^���葀��ȂǁA				   |*/
/*|			CRC��肪����ȏꍇ�ɁADIP-SW�����p���Ă�������				   |*/
/*|                                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase 		                                           |*/
/*| Date        :  2005-03-16                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	CheckCRC(uchar *pFrom, ushort usSize, uchar *pHere)
{
	uchar	crc[2];

	/* CRC�Z�o�i�upFrom�v����uusSize�v�����Z�o�ΏۂŁA�Z�o���ʂƔ�r���鑊�肪�upHere�v�j*/
	crc_ccitt(usSize, pFrom, crc, 1);		/* CRC16-CCITT �Z�o�֐��Ăяo�����ɂ́A�u�E�V�t�g�v���w�肷�� */
	if (crc[0] == pHere[0] &&
		crc[1] == pHere[1])			/* CRC ���/���ʂ��������v�Ȃ�OK */
	{
		return 1;					/* ��v�Ȃ̂Łu1=TRUE�v�Ԃ� */
	} else {
		return 0;					/* �s��v�Ȃ̂Łu0=FALSE�v�Ԃ� */
	}
}
/*[]----------------------------------------------------------------------[]*/
/*|             astohex() 				                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		�A�X�L�[��16�i�ϊ�												   |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : signed char*	: �ϊ������ڽ			 					   |*/
/*|			 unsigned int	: �ϊ���									   |*/
/*|			 unsigned char	: 0:A-F		1:a-f							   |*/
/*|	return : unsigned long	: �ϊ����ꂽ���l(16�i�p)					   |*/
/*|                                                                        |*/
/*|	note : rismtool����̈ڐA											   |*/
/*|                                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :  2012-03-26                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
unsigned long	astohex ( char *buf, unsigned int size, unsigned char LS )
{																	/*							*/
	unsigned char	c;												/*							*/
	unsigned long	li, lk;											/* �v�Z�p					*/
																	/*							*/
	lk = 0;															/*							*/
	for( li = 0L; li < (unsigned long)size; li++) {					/*							*/
		lk *= 16;													/*							*/
		c = buf[li];												/*							*/
		if( c > 0x40 )	c -= ( 0x37 + ( 0x20 * LS ));				/*							*/
		else			c -= 0x30;									/*							*/
		lk += (unsigned long)c;										/*							*/
	}																/*							*/
	return( lk );													/*							*/
}																	/*							*/

/*[]----------------------------------------------------------------------[]*/
/*|             hextoas() 				                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*|		16�i���A�X�L�[�ϊ�												   |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : signed char*	: �ϊ������ڽ			 					   |*/
/*|			 unsigned long	: �ϊ����ް�								   |*/
/*|			 unsigned int	: �ϊ���									   |*/
/*|	return : void														   |*/
/*|                                                                        |*/
/*|	note : rismtool����̈ڐA											   |*/
/*|                                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :  2012-03-26                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	hextoas ( unsigned char *buf, unsigned long data, unsigned int size, unsigned long LS )
{																	/*							*/
	unsigned long	li,lj;											/* �v�Z�p					*/
	for ( li = 0L; li < size; li++ ) {								/*							*/
		lj = ( unsigned long)data;									/*							*/
		lj >>= (4 * ((unsigned long)size - 1L - li));				/*							*/
		lj &= 0x0000000f;											/*							*/
		if(lj > 9)	lj += (unsigned long)( 0x37 + ( 0x20 * LS ));	/*							*/
		else		lj += 0x00000030; 								/*							*/
		buf[li] = (unsigned char)lj;								/*							*/
	}																/*							*/
	return;															/*							*/
}																	/*							*/

/*[]-----------------------------------------------------------------------[]*/
/*|		�������菈��														|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	SJIS_Size( *data)									|*/
/*|																			|*/
/*|	PARAMETER		:	uchar	*data	=	�Ώە����ް��߲���				|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret		=	�P�������޲Đ�					|*/
/*|						̫��A�̏ꍇ1 , �����̏ꍇ 2							|*/
/*|		SJIS Shift Jis 														|*/
/*|		�@ascii�����̎�(0x20�`0x7D)											|*/
/*|		�A�Q�o�C�g�����̎�(0x8140�`0x9FFC)									|*/
/*|		�B���p�J�^�J�i�̎�(0xA1�`0xDF)										|*/
/*|		�C�Q�o�C�g�����̎�(0xE040�`0xFCFC)									|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-03													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	SJIS_Size( uchar *data )
{
	uchar	rtc;

	if((( 0x81 <= data[0] )&&( data[0] <= 0x9f )) ||	// ����?(Y)
		(( 0xE0 <= data[0] )&&( data[0] <= 0xFC )) ){	// ����?(Y)
		rtc = 2;										// 2Byte
	}else{
		rtc = 1;										// 1Byte
	}
	return(rtc);
}

/*[]----------------------------------------------------------------------[]*/
/*| ���񂹂ɂ��ăR�s�[���A�������X�y�[�X�Ŗ��߂�                           |*/
/*| ���P �X�y�[�X(0x20)�����łȂ��A�k��(0x0)���폜�ΏۂƂ���               |*/
/*| ���Q ������r����0x20����0x0������ꍇ�A0x20,0x0�ȍ~�̕������폜����   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : *dist    : �i�[��|�C���^�[                             |*/
/*|				   *src     : �i�[���|�C���^�[                             |*/
/*|				   dist_len : �i�[��o�b�t�@��                             |*/
/*|				   src_len  : �i�[���o�b�t�@��                             |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Morita                                                |*/
/*| Date         : 2013/01/10                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
void	memcpyFlushLeft(uchar *dist, uchar *src, ushort dist_len, ushort src_len)
{
	ushort	moji_left, moji_len, len, i;

	moji_left	= 0;
	moji_len	= 0;
	memset(dist, 0x20, (size_t)dist_len);				// �i�[����X�y�[�X�ŏ�����

	len = src_len;
	if (dist_len < src_len) {
		len = dist_len;									// �o�b�t�@���̒Z�����ɍ��킹��
	}

	if (len < 2) return;								// �����񒷂�2��菬����

	// ������̐擪�ʒu����
	for (i=0; i<len; i++) {
		if ((src[i] != 0x0) && (src[i] != 0x20)) {		// 0x0��0x20�͏Ȃ�
			break;
		}
	}
	if (i >= len) return;								// �����񂪑��݂��Ȃ�
	moji_left = i;										// ������擪�ʒu

	// ������̖����ʒu����
	for (i=moji_left+1; i<len; i++) {
		if ((src[i] == 0x0) || (src[i] == 0x20)) {		// 0x0��0x20�ȊO�͕�����
			break;
		}
	}
	moji_len = (uchar)(i - moji_left);					// ������

	memcpy(dist, &src[moji_left], (size_t)moji_len);	// ����������񂹂ŃR�s�[
}
// MH322914(S) K.Onodera 2016/09/09 AI-V�Ή��F�̎����Ĕ��s
// Dest�̓X�y�[�X���߂��Ȃ��̂ŁA����ꍇ�͌Ăь��ōs��
void	memcpyFlushRight(uchar *dest, uchar *src, ushort dest_len, ushort src_len)
{
	ushort	len, i, sta = 0;

	len = src_len;
	// �o�b�t�@���̒Z�����ɍ��킹��
	if( dest_len < src_len ){
		len = dest_len;
	}
	// �����񒷂�2��菬����
	if( len < 2 ){
		return;
	}

	// �擪��������łȂ�
	if( (src[0] == 0x0) || (src[0] == 0x20) ){
		return;
	}
	// ������̖����ʒu����
	for( i=1; i<len; i++ ){
		if( (src[i] == 0x0) || (src[i] == 0x20) ){		// 0x0��0x20�ȊO�͕�����
			break;
		}
	}
	sta = (src_len - i);
	memcpy( &dest[sta], src, i );						// ��������E�񂹂ŃR�s�[
}
// MH322914(E) K.Onodera 2016/09/09 AI-V�Ή��F�̎����Ĕ��s
// MH322915(S) K.Onodera 2017/05/18 ���u�_�E�����[�h�C��
//[]----------------------------------------------------------------------[]
//		�N���������`�F�b�N
//[]------------------------------------- Copyright(C) 2017 AMANO Corp.---[]
uchar Check_date_time_rec( date_time_rec *pTime )
{
	uchar	ret = 0;		// �͈͊O(NG)

	// NULL�`�F�b�N
	if( pTime == NULL ){
		return ret;
	}

	// �N�����`�F�b�N
	if( chkdate2( (short)pTime->Year,(short)pTime->Mon,(short)pTime->Day ) == 0 ){
		// ���ԃ`�F�b�N
		if( pTime->Hour <= 23 ){
			if( pTime->Min <= 59 ){
				ret = 1;	// �͈͓�(OK)
			}
		}
	}
	return ret;
}
// MH322915(E) K.Onodera 2017/05/18 ���u�_�E�����[�h�C��
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
/*[]----------------------------------------------------------------------[]*/
/*| ���񂹂ɂ��ăR�s�[���A�������X�y�[�X�Ŗ��߂�                           |*/
/*| ���P �X�y�[�X(0x20)�����łȂ��A�k��(0x0)���폜�ΏۂƂ���               |*/
/*| ���Q ������r����0x20����0x0������ꍇ�A0x20,0x0�ȍ~�̕������폜����   |*/
/*| ���R �����񂪑��݂��Ȃ��ꍇ�A�����񒷂�2��菬�����ꍇ�A�k��(0x0)�Ŗ��߂�|*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : *dist    : �i�[��|�C���^�[                             |*/
/*|				   *src     : �i�[���|�C���^�[                             |*/
/*|				   dist_len : �i�[��o�b�t�@��                             |*/
/*|				   src_len  : �i�[���o�b�t�@��                             |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : D.Inaba                                                 |*/
/*| Date         : 2020/01/07                                              |*/
/*| Update       : memcpyFlushLeft�Ɠ��l�@�@	                           |*/
/*[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]*/
void	memcpyFlushLeft2(uchar *dist, uchar *src, ushort dist_len, ushort src_len)
{
	ushort	moji_left, moji_len, len, i;

	moji_left	= 0;
	moji_len	= 0;
	memset(dist, 0x20, (size_t)dist_len);				// �i�[����X�y�[�X�ŏ�����

	len = src_len;
	if (dist_len < src_len) {
		len = dist_len;									// �o�b�t�@���̒Z�����ɍ��킹��
	}

	if (len < 2){										// �����񒷂�2��菬����
		memset(dist, 0x0, (size_t)dist_len);			// �i�[����k��(0x0)�Ŗ��߂�
		return;
	}

	// ������̐擪�ʒu����
	for (i=0; i<len; i++) {
		if ((src[i] != 0x0) && (src[i] != 0x20)) {		// 0x0��0x20�͏Ȃ�
			break;
		}
	}
	if (i >= len) {										// �����񂪑��݂��Ȃ�
		memset(dist, 0x0, (size_t)dist_len);			// �i�[����k��(0x0)�Ŗ��߂�
		return;
	}
	moji_left = i;										// ������擪�ʒu

	// ������̖����ʒu����
	for (i=moji_left+1; i<len; i++) {
		if ((src[i] == 0x0) || (src[i] == 0x20)) {		// 0x0��0x20�ȊO�͕�����
			break;
		}
	}
	moji_len = i - moji_left;							// ������

	memcpy(dist, &src[moji_left], (size_t)moji_len);	// ����������񂹂ŃR�s�[
}

/*[]----------------------------------------------------------------------[]*/
/*| ���񂹂ɂ��ăR�s�[���A�������X�y�[�X�Ŗ��߂�                           |*/
/*| ���P �X�y�[�X(0x20)�����łȂ��A�k��(0x0)���폜�ΏۂƂ���               |*/
/*| ���Q ������r����0x20����0x0������ꍇ�A0x20,0x0�ȍ~�̕������폜����   |*/
/*| ���R �����񂪑��݂��Ȃ��ꍇ�A�����񒷂�2��菬�����ꍇ�A�k��(0x0)�Ŗ��߂�|*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : *dist    : �i�[��|�C���^�[                             |*/
/*|				   *src     : �i�[���|�C���^�[                             |*/
/*|				   dist_len : �i�[��o�b�t�@��                             |*/
/*|				   src_len  : �i�[���o�b�t�@��                             |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : D.Inaba                                               |*/
/*| Date         : 2020/01/07                                            |*/
/*| Update       : memcpyFlushLeft�Ɠ��l�@�@	                          |*/
/*[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]*/
void	memcpyFlushLeft_Electron_ID(uchar *dist, uchar *src, ushort dist_len, ushort src_len)
{
	ushort	moji_left, moji_len, len, i;

	moji_left	= 0;
	moji_len	= 0;
	memset(dist, 0x20, (size_t)dist_len);				// �i�[����X�y�[�X�ŏ�����

	len = src_len;
	if (dist_len < src_len) {
		len = dist_len;									// �o�b�t�@���̒Z�����ɍ��킹��
	}

	if (len < 2){										// �����񒷂�2��菬����
		memset(dist, 0x0, (size_t)dist_len);			// �i�[����k��(0x0)�Ŗ��߂�
		memset(dist, 'Z', 2);							// �i�[���2����Z�Ŗ��߂�
		return;
	}

	// ������̐擪�ʒu����
	for (i=0; i<len; i++) {
		if ((src[i] != 0x0) && (src[i] != 0x20)) {		// 0x0��0x20�͏Ȃ�
			break;
		}
	}
	if (i >= len) {										// �����񂪑��݂��Ȃ�
		memset(dist, 0x0, (size_t)dist_len);			// �i�[����k��(0x0)�Ŗ��߂�
		memset(dist, 'Z', 2);							// �i�[���2����Z�Ŗ��߂�
		return;
	}
	moji_left = i;										// ������擪�ʒu

	// ������̖����ʒu����
	for (i=moji_left+1; i<len; i++) {
		if ((src[i] == 0x0) || (src[i] == 0x20)) {		// 0x0��0x20�ȊO�͕�����
			break;
		}
	}
	moji_len = i - moji_left;							// ������

	memcpy(dist, &src[moji_left], (size_t)moji_len);	// ����������񂹂ŃR�s�[
}

/*[]----------------------------------------------------------------------[]*/
/*| �z��Ɋi�[����Ă��镶����ϊ�����           			�@                |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : *src     : �i�[���|�C���^�[                             |*/
/*|				   src_len  : �i�[���o�b�t�@��                             |*/
/*|				   start	: �擪���琔�����u���J�n�ʒu(��)�@              |*/
/*|				   end	    : ������̖����i0x20, 0x0�������j���琔�����u���I���ʒu |*/
/*|				   before   : �u���������@�@�@                             |*/
/*|				   after    : �u���敶���@�@�@                             |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : D.Inaba                                                |*/
/*| Date         : 2020/01/20                                              |*/
/*| note  	     : start = 7, end = 5,  ���@  XXXXXXAAAAAAXXXX (7���ځ`��5���ڂ܂�) |*/
/*[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]*/
void	change_CharInArray(uchar *src, ushort src_len, ushort start, ushort end, uchar before, uchar after)
{
	ushort i, str_len, start_dig, end_dig;

	// ������̖����ʒu����
	for (i = 0; i < src_len; i++) {
		if ((src[i] == 0x0) || (src[i] == 0x20)) {
			break;
		}
	}

// MH810103 GG119202(S) �����񌟍��s�
//	// �����񂪑��݂��Ȃ�
//	if (i >= src_len) {
//		return;
//	}
// MH810103 GG119202(E) �����񌟍��s�
	// �����񒷁i0x20, 0x0�������j
	str_len = i;

	// �J�n�ʒu��0��菬����
	if( start <= 0 ){
		return;
	}
	// �J�n�ʒu����
	start_dig = start - 1;
	
	// �u���I���ʒu�������񒷂��傫��
// MH810103(S) S.Nishimoto NT-NET�ڑ�����ł݂Ȃ����ςɂ���Ɛ��Z�@�����u�[�g����(#5099)
//	if( end > str_len && end <= 0){
	if (end > str_len || end <= 0) {
// MH810103(E) S.Nishimoto NT-NET�ڑ�����ł݂Ȃ����ςɂ���Ɛ��Z�@�����u�[�g����(#5099)
		return;
	}
	// ������̖����i0x20, 0x0�������j���琔�����u���I���ʒu��
	// �擪���琔�����u���I���ʒu�ɕϊ�����
	end_dig = str_len - (end - 1);

	if( start_dig <= end_dig ){
		// �u��
		for ( i = start_dig; i < end_dig; i++ ){
			if( src[i] == before ){
				src[i] = after;
			}
		}
	}
}
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
//[]----------------------------------------------------------------------[]
///	@brief			�����`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		*clk_data : �`�F�b�N�Ώ�
///	@return			0=OK/-1=NG
//[]----------------------------------------------------------------------[]
///	@author			MATSUSHITA
///	@date			Create	: 2008/08/28<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
int		datetimecheck(struct clk_rec *clk_data)
{
// DateTimeCheck()��chkdate()�ƂŔN�L���͈͂��قȂ�̂Œ���
// �g�p�ړI�̓s����DateTimeCheck�ɍ��킹�Ă���
	if( (clk_data->year < 1990)||(clk_data->year > 2050) ){
		return( -1 );
	}
	if( (clk_data->mont < 1)||(clk_data->mont > 12) ){
		return( -1 );
	}
	if( (clk_data->date < 1)||
		(clk_data->date > medget((short)clk_data->year, (short)clk_data->mont)) ){
		return( -1 );
	}
	if( (clk_data->hour > 23) ){
		return( -1 );
	}
	if( (clk_data->minu > 59) ){
		return( -1 );
	}
	if( (clk_data->seco > 59) ){
		return( -1 );
	}
	return( 0 );
}
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
