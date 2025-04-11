/*
 * Description: Testing the AH algorithems: SHA1
 *
 * Filename: trsha1.c
 * Author: Jin Zhang
 * Date Created: 08/01/2001
 * $Source: source/cryptlib/trsha1.c $
 *
 * Modification History
 * $Revision: 6.0.2.4 $
 * $Date: 2010/08/17 05:38:26JST $
 * $Author: pcarney $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */




/*  $KAME: sha1.c,v 1.5 2000/11/08 06:13:08 itojun Exp $    */

/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * FIPS pub 180-1: Secure Hash Algorithm (SHA-1)
 * based on: http://csrc.nist.gov/fips/fip180-1.txt
 * implemented by Jun-ichiro itojun Itoh <itojun@itojun.org>
 */


 /* sha1.c : Implementation of the Secure Hash Algorithm
 * SHA: NIST's Secure Hash Algorithm
 *    This version written November 2000 by David Ireland of
 *    DI Management Services Pty Limited <code@di-mgt.com.au>
 *
 *    Adapted from code in the Python Cryptography Toolkit,
 *    version 1.0.0 by A.M. Kuchling 1995.
 *
 * AM Kuchling's posting:-
 * Based on SHA code originally posted to sci.crypt by Peter Gutmann
 * in message <30ajo5$oe8@ccu2.auckland.ac.nz>.
 * The original specification of SHA was found to have a weakness
 * by NSA/NIST.  This code implements the fixed version of SHA.
 *
 * Here's the first paragraph of Peter Gutmann's posting:
 *
 * The following is my SHA (FIPS 180) code updated to allow use of the "fixed"
 * SHA, thanks to Jim Gillogly and an anonymous contributor for the information on
 * what's changed in the new version.  The fix is a simple change which involves
 * adding a single rotate in the initial expansion function.  It is unknown
 * whether this is an optimal solution to the problem which was discovered in the
 * SHA or whether it's simply a bandaid which fixes the problem with a minimum of
 * effort (for example the reengineering of a great many Capstone chips).
 */
#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>

#ifdef TM_USE_SHA1

static void tfSha1ToByte(tt8BitPtr output, tt32BitPtr input, ttPktLen len);

#define TM_HMAC_SHA_BLOCK_SIZE 64
/* The SHA f()-functions.  The f1 and f3 functions can be optimized to
 * save one boolean operation each - thanks to Rich Schroeppel,
 * rcs@cs.arizona.edu for discovering this
 */
#define tm_sha_f1(x,y,z)   ( z ^ ( x & ( y ^ z ) ) )      /* Rounds  0-19 */
#define tm_sha_f2(x,y,z)   ( x ^ y ^ z )                  /* Rounds 20-39 */
#define tm_sha_f3(x,y,z)   ( ( x & y ) | ( z & ( x | y ) ) ) /* Rounds 40-59 */
#define tm_sha_f4(x,y,z)   ( x ^ y ^ z )                     /* Rounds 60-79 */

/* The SHA Mysterious Constants */
#define K1  TM_UL(0x5A827999)                             /* Rounds  0-19 */
#define K2  TM_UL(0x6ED9EBA1)                             /* Rounds 20-39 */
#define K3  TM_UL(0x8F1BBCDC)                             /* Rounds 40-59 */
#define K4  TM_UL(0xCA62C1D6)                             /* Rounds 60-79 */

/* SHA initial values */
#define h0init  TM_UL(0x67452301)
#define h1init  TM_UL(0xEFCDAB89)
#define h2init  TM_UL(0x98BADCFE)
#define h3init  TM_UL(0x10325476)
#define h4init  TM_UL(0xC3D2E1F0)

/* Note that it may be necessary to add parentheses to these macros if they
 * are to be called with expressions as arguments
 * 32-bit rotate left - kludged with shifts
 */

#define tm_sha_rotl(n,X)  ( ( ( X ) << n ) | ( ( X ) >> ( 32 - n ) ) )

/* The initial expanding function.  The hash function is defined over an
 * 80-UINT2 expanded input array W, where the first 16 are copies of the input
 * data, and the remaining 64 are defined by
 *
 *       W[ i ] = W[ i - 16 ] ^ W[ i - 14 ] ^ W[ i - 8 ] ^ W[ i - 3 ]
 *
 * This implementation generates these values on the fly in a circular
 * buffer - thanks to Colin Plumb, colin@nyx10.cs.du.edu for this
 * optimization.
 *
 * The updated SHA changes the expanding function by adding a rotate of 1
 * bit.  Thanks to Jim Gillogly, jim@rand.org, and an anonymous contributor
 * for this information
 */

#define tm_sha_expand(W,i) \
    ( W[ i & 15 ] = tm_sha_rotl( 1, ( W[ i & 15 ] ^ W[ (i - 14) & 15 ] ^ \
      W[ (i - 8) & 15 ] ^ W[ (i - 3) & 15 ] ) ) )


/* The prototype SHA sub-round.  The fundamental sub-round is:
 *
 *      a' = e + tm_sha_rotl( 5, a ) + f( b, c, d ) + k + data;
 *      b' = a;
 *      c' = tm_sha_rotl( 30, b );
 *      d' = c;
 *      e' = d;
 *
 * but this is implemented by unrolling the loop 5 times and renaming the
 * variables ( e, a, b, c, d ) = ( a', b', c', d', e' ) each iteration.
 * This code is then replicated 20 times for each of the 4 functions, using
 * the next 20 values from the W[] array each time
 */

#define tm_sha_sub_round_f1(a, b, c, d, e, k, data) \
    ( e += tm_sha_rotl( 5, a ) + tm_sha_f1( b, c, d ) + k + data, b =   \
      tm_sha_rotl( 30, b ) )

#define tm_sha_sub_round_f2(a, b, c, d, e, k, data) \
    ( e += tm_sha_rotl( 5, a ) + tm_sha_f2( b, c, d ) + k + data, b =   \
      tm_sha_rotl( 30, b ) )

#define tm_sha_sub_round_f3(a, b, c, d, e, k, data) \
    ( e += tm_sha_rotl( 5, a ) + tm_sha_f3( b, c, d ) + k + data, b =   \
      tm_sha_rotl( 30, b ) )

#define tm_sha_sub_round_f4(a, b, c, d, e, k, data) \
    ( e += tm_sha_rotl( 5, a ) + tm_sha_f4( b, c, d ) + k + data, b =   \
      tm_sha_rotl( 30, b ) )

/* Initialize the SHA values */

void tfSha1Init(ttVoidPtr  contextPtr)
{
    ttSha1CtxPtr shaContextPtr;
    shaContextPtr = (ttSha1CtxPtr)contextPtr;

/* Set the h-vars to their initial values */
    shaContextPtr->shaDigest[ 0 ] = h0init;
    shaContextPtr->shaDigest[ 1 ] = h1init;
    shaContextPtr->shaDigest[ 2 ] = h2init;
    shaContextPtr->shaDigest[ 3 ] = h3init;
    shaContextPtr->shaDigest[ 4 ] = h4init;

/* Initialise bit count */
    shaContextPtr->shaCountLo = shaContextPtr->shaCountHi = 0;
}


/* Perform the SHA transformation.  Note that this code, like MD5, seems to
 * break some optimizing compilers due to the complexity of the expressions
 * and the size of the basic block.  It may be necessary to split it into
 * sections, e.g. based on the four tm_sha_sub_rounds
 *
 * Note that this corrupts the  shaContextPtr->data area
 */

static void tfSha1Transform( tt32BitPtr digestPtr, tt32BitPtr dataPtr )
{
    tt32Bit A, B, C, D, E;     /* Local vars */
    tt32Bit eData[ 16 ];       /* Expanded dataPtr */
    int     i;

/* Set up first buffer and local dataPtr buffer */
    A = digestPtr[ 0 ];
    B = digestPtr[ 1 ];
    C = digestPtr[ 2 ];
    D = digestPtr[ 3 ];
    E = digestPtr[ 4 ];
    tm_memcpy( (tt8BitPtr)eData, (tt8BitPtr)dataPtr, TM_HMAC_SHA_BLOCK_SIZE );

/* Heavy mangling, in 4 sub-rounds of 20 interations each. */
    for (i = 0; i < 15; i += 5)
    {
        tm_sha_sub_round_f1( A, B, C, D, E, K1, eData[ i + 0 ] );
        tm_sha_sub_round_f1( E, A, B, C, D, K1, eData[ i + 1 ] );
        tm_sha_sub_round_f1( D, E, A, B, C, K1, eData[ i + 2 ] );
        tm_sha_sub_round_f1( C, D, E, A, B, K1, eData[ i + 3 ] );
        tm_sha_sub_round_f1( B, C, D, E, A, K1, eData[ i + 4 ] );
    }
    tm_sha_sub_round_f1( A, B, C, D, E, K1, eData[ 15 ] );
    tm_sha_sub_round_f1( E, A, B, C, D, K1, tm_sha_expand(eData, 16));
    tm_sha_sub_round_f1( D, E, A, B, C, K1, tm_sha_expand(eData, 17));
    tm_sha_sub_round_f1( C, D, E, A, B, K1, tm_sha_expand(eData, 18));
    tm_sha_sub_round_f1( B, C, D, E, A, K1, tm_sha_expand(eData, 19));

    for (i = 20; i < 40; i += 5)
    {
        tm_sha_sub_round_f2( A, B, C, D, E, K2, tm_sha_expand(eData, (i+0)));
        tm_sha_sub_round_f2( E, A, B, C, D, K2, tm_sha_expand(eData, (i+1)));
        tm_sha_sub_round_f2( D, E, A, B, C, K2, tm_sha_expand(eData, (i+2)));
        tm_sha_sub_round_f2( C, D, E, A, B, K2, tm_sha_expand(eData, (i+3)));
        tm_sha_sub_round_f2( B, C, D, E, A, K2, tm_sha_expand(eData, (i+4)));
    }

    for (i = 40; i < 60; i += 5)
    {
        tm_sha_sub_round_f3( A, B, C, D, E, K3, tm_sha_expand(eData, (i+0)));
        tm_sha_sub_round_f3( E, A, B, C, D, K3, tm_sha_expand(eData, (i+1)));
        tm_sha_sub_round_f3( D, E, A, B, C, K3, tm_sha_expand(eData, (i+2)));
        tm_sha_sub_round_f3( C, D, E, A, B, K3, tm_sha_expand(eData, (i+3)));
        tm_sha_sub_round_f3( B, C, D, E, A, K3, tm_sha_expand(eData, (i+4)));
    }

    for (i = 60; i < 80; i += 5)
    {
        tm_sha_sub_round_f4( A, B, C, D, E, K4, tm_sha_expand(eData, (i+0)));
        tm_sha_sub_round_f4( E, A, B, C, D, K4, tm_sha_expand(eData, (i+1)));
        tm_sha_sub_round_f4( D, E, A, B, C, K4, tm_sha_expand(eData, (i+2)));
        tm_sha_sub_round_f4( C, D, E, A, B, K4, tm_sha_expand(eData, (i+3)));
        tm_sha_sub_round_f4( B, C, D, E, A, K4, tm_sha_expand(eData, (i+4)));
    }

/* Build message digestPtr */
    digestPtr[ 0 ] += A;
    digestPtr[ 1 ] += B;
    digestPtr[ 2 ] += C;
    digestPtr[ 3 ] += D;
    digestPtr[ 4 ] += E;
}

/* When run on a little-endian CPU we need to perform byte reversal on an
 * array of long words.
 */

static void tfSha1LongReverse(tt32BitPtr buffer, tt32Bit    byteCount)
{
#ifdef TM_LITTLE_ENDIAN
    tt32Bit value;

    byteCount /= sizeof( tt32Bit );
    while( byteCount-- )
    {
        value = *buffer;
        tm_ntohl(value, *buffer);
        buffer++;
    }
#else /* !TM_LITTLE_ENDIAN */
   TM_UNREF_IN_ARG(buffer);
   TM_UNREF_IN_ARG(byteCount);
#endif /* !TM_LITTLE_ENDIAN */
    return;
}

/* Update SHA for a block of data */

void tfSha1Update(ttVoidPtr  contextPtr,
                  tt8BitPtr  buffer,
                  ttPktLen   count,
                  ttPktLen   offset)
{
    tt32Bit   tmp;
    ttPktLen  dataCount;
    tt8BitPtr p;

    ttSha1CtxPtr shaContextPtr;
    shaContextPtr = (ttSha1CtxPtr)contextPtr;

    TM_UNREF_IN_ARG(offset); /* for 16Bit or 32Bit DSP usage */
/* Update bitcount */
    tmp = shaContextPtr->shaCountLo;
    shaContextPtr->shaCountLo += ((tt32Bit)count << 3 );
    if( shaContextPtr->shaCountLo < tmp )
    {
        shaContextPtr->shaCountHi++;             /* Carry from low to high */
    }
    shaContextPtr->shaCountHi += count >> 29;

/* Get count of bytes already in data */
    dataCount = (ttPktLen) (( tmp >> 3 ) & 0x3F);

/* Handle any leading odd-sized chunks */
    if( dataCount )
    {
        p = ( tt8BitPtr ) shaContextPtr->shaData + dataCount;

        dataCount = TM_HMAC_SHA_BLOCK_SIZE - dataCount;
        if( count < dataCount )
        {
            tm_memcpy( p, buffer, count );
            goto UPDATE_RETURN;
        }
        tm_memcpy( p, buffer, dataCount );
        tfSha1LongReverse( shaContextPtr->shaData, TM_HMAC_SHA_BLOCK_SIZE);
        tfSha1Transform( shaContextPtr->shaDigest, shaContextPtr->shaData );
        buffer += dataCount;
        count -= dataCount;
    }

/* Process data in TM_HMAC_SHA_BLOCK_SIZE chunks */
    while( count >= TM_HMAC_SHA_BLOCK_SIZE )
    {
        tm_memcpy( (tt8BitPtr) shaContextPtr->shaData,
            (tt8BitPtr)buffer,
            TM_HMAC_SHA_BLOCK_SIZE );
        tfSha1LongReverse( shaContextPtr->shaData, TM_HMAC_SHA_BLOCK_SIZE);
        tfSha1Transform( shaContextPtr->shaDigest, shaContextPtr->shaData );
        buffer += TM_HMAC_SHA_BLOCK_SIZE;
        count -= TM_HMAC_SHA_BLOCK_SIZE;
    }

/* Handle any remaining bytes of data. */
    tm_memcpy( (tt8BitPtr) shaContextPtr->shaData, (tt8BitPtr)buffer, count );
UPDATE_RETURN:
    return;
}

/* Final wrapup - pad to TM_HMAC_SHA_BLOCK_SIZE-byte boundary
 * with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */

void tfSha1Final(tt8BitPtr output, ttVoidPtr contextPtr)
{
    ttPktLen  count;
    tt8BitPtr dataPtr;

    ttSha1CtxPtr shaContextPtr;
    shaContextPtr = (ttSha1CtxPtr)contextPtr;

/* Compute number of bytes mod 64 */
    count = (ttPktLen) ((shaContextPtr->shaCountLo >> 3) & 0x3F);

/* Set the first char of padding to 0x80.  This is safe since there is
 * always at least one byte free
 */
    dataPtr = ( tt8BitPtr ) shaContextPtr->shaData + count;
    *dataPtr++ = 0x80;

/* Bytes of padding needed to make 64 bytes */
    count = TM_HMAC_SHA_BLOCK_SIZE - 1 - count;

/* Pad out to 56 mod 64 */
    if( count < 8 )
    {
/* Two lots of padding:  Pad the first block to 64 bytes */
        tm_memset( dataPtr, 0, count );
        tfSha1LongReverse( shaContextPtr->shaData, TM_HMAC_SHA_BLOCK_SIZE);
        tfSha1Transform( shaContextPtr->shaDigest, shaContextPtr->shaData );

/* Now fill the next block with 56 bytes */
        tm_memset( (tt8BitPtr) shaContextPtr->shaData, 0,
            TM_HMAC_SHA_BLOCK_SIZE - 8 );
    }
    else
    {
/* Pad block to 56 bytes */
        tm_memset( dataPtr, 0, count - 8 );
    }

/* Append length in bits and transform */
    shaContextPtr->shaData[ 14 ] = shaContextPtr->shaCountHi;
    shaContextPtr->shaData[ 15 ] = shaContextPtr->shaCountLo;

    tfSha1LongReverse( shaContextPtr->shaData, TM_HMAC_SHA_BLOCK_SIZE - 8);
    tfSha1Transform( shaContextPtr->shaDigest, shaContextPtr->shaData );

/* Output to an array of bytes */
    tfSha1ToByte(output, shaContextPtr->shaDigest, 20);

/* Zeroise sensitive stuff */
    tm_memset((tt8BitPtr) shaContextPtr, 0, sizeof(shaContextPtr));
}

static void tfSha1ToByte(tt8BitPtr output, tt32BitPtr  input, ttPktLen len)
{    /* Output SHA digest in tt8Bit array */
    ttPktLen i, j;

    for(i = 0, j = 0; j < len; i++, j += 4)
    {
        output[j+3] = tm_8bit(input[i]);
        output[j+2] = tm_8bit(input[i] >> 8 );
        output[j+1] = tm_8bit(input[i] >> 16);
        output[j  ] = tm_8bit(input[i] >> 24);
    }
}

/*
 * hashKeyPtr points to a 20-byte key (TM_SHA1_HASH_SIZE)
 */
void tfHmacSha1(tt8BitPtr    dataPtr,
                int          dataLen,
                tt8BitPtr    hashKeyPtr,
                tt8BitPtr    digestPtr,
                int          digestLen)
{
    ttSha1Ctx      sha1Ctx;
    tt8Bit         buf[20];
    tt8Bit         k1[64];
    tt8Bit         k2[64];
    int            i;

    tm_memset(k1, 0x36, 64);
    tm_memset(k2, 0x5c, 64);

    for (i = 0; i < 20; i++)
    {
        k1[i] ^= (tt8Bit)(hashKeyPtr[i]);
        k2[i] ^= (tt8Bit)(hashKeyPtr[i]);
    }

    tfSha1Init(&sha1Ctx);
    tfSha1Update(&sha1Ctx, k1, 64, 0);
    tfSha1Update(&sha1Ctx, dataPtr, (ttPktLen)dataLen, 0);
    tfSha1Final(buf, &sha1Ctx);

    tfSha1Init(&sha1Ctx);
    tfSha1Update(&sha1Ctx, k2, 64, 0);
    tfSha1Update(&sha1Ctx, buf, 20, 0);
    tfSha1Final(buf, &sha1Ctx);

    if (digestLen > 20)
    {
        digestLen = 20;
    }
    tm_memcpy(digestPtr, buf, digestLen);
    return;
}


void tfSha1Checksum(tt8BitPtr    dataPtr,
                    int          dataLen,
                    tt8BitPtr    digestPtr,
                    int          digestLen)
{
    ttSha1Ctx   sha1Ctx;
    tt8Bit      buf[20];

    tfSha1Init(&sha1Ctx);
    tfSha1Update(&sha1Ctx, dataPtr,(ttPktLen)dataLen, 0);
    tfSha1Final(buf, &sha1Ctx);

    if (digestLen > 20)
    {
        digestLen = 20;
    }
    tm_memcpy(digestPtr, buf, digestLen);
    return;
}
#else /* TM_USE_SHA1 */

LINT_UNUSED_HEADER

/* To allow link for builds when TM_USE_SHA1 is not defined */
int tvSha1Dummy = 0;
#endif /* TM_USE_SHA1 */

/***************** End Of File *****************/
