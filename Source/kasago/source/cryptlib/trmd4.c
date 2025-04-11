/*
 * Description: md4 hashing algorithm
 *
 * Filename: trmd4.c
 * Author: Jin Zhang
 * Date Created: 07/22/03
 * $Source: source/cryptlib/trmd4.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:35:33JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */ 

/*
 * Include
 */
#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>

#ifdef TM_USE_MD4

#include <trproto.h>
#include <trglobal.h>


/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
 * rights reserved.
 * 
 * License to copy and use this software is granted provided that it
 * is identified as the "RSA Data Security, Inc. MD4 Message-Digest
 * Algorithm" in all material mentioning or referencing this software
 * or this function.
  
 * License is also granted to make and use derivative works provided
 * that such works are identified as "derived from the RSA Data
 * Security, Inc. MD4 Message-Digest Algorithm" in all material
 * mentioning or referencing the derived work.
 *     
 * RSA Data Security, Inc. makes no representations concerning either
 * the merchantability of this software or the suitability of this
 * software for any particular purpose. It is provided "as is"
 * without express or implied warranty of any kind.
 * 
 * These notices must be retained in any copies of any part of this
 * documentation and/or software.
 */



/*A.3 md4c.c*/

/* MD4C.C - RSA Data Security, Inc., MD4 message-digest algorithm
*/

/* Copyright (C) 1990-2, RSA Data Security, Inc. All rights reserved.
 * License to copy and use this software is granted provided that it
 * is identified as the "RSA Data Security, Inc. MD4 Message-Digest
 * Algorithm" in all material mentioning or referencing this software
 * or this function.
 * 
 * License is also granted to make and use derivative works provided
 * that such works are identified as "derived from the RSA Data
 * Security, Inc. MD4 Message-Digest Algorithm" in all material
 * mentioning or referencing the derived work.
 *
 * RSA Data Security, Inc. makes no representations concerning either
 * the merchantability of this software or the suitability of this
 * software for any particular purpose. It is provided "as is"
 * without express or implied warranty of any kind.
 *     
 * These notices must be retained in any copies of any part of this
 * documentation and/or software.
 */

/* Constants for tfMD4Transform routine.*/
#define S11 3
#define S12 7
#define S13 11
#define S14 19
#define S21 3
#define S22 5
#define S23 9
#define S24 13
#define S31 3
#define S32 9
#define S33 11
#define S34 15

static void tfMD4Transform(tt32Bit state[4], const tt8Bit block[64]);
static void tfMD4Encode(tt8BitPtr output, tt32BitPtr input, ttPktLen len);
static void tfMD4Decode(tt32BitPtr output, ttConst8BitPtr input,
                        ttPktLen len);

static const tt8Bit tlMd4Padding[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* tm_md4_f, tm_md4_g and tm_md4_h are basic MD4 functions.
 */
#define tm_md4_f(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define tm_md4_g(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define tm_md4_h(x, y, z) ((x) ^ (y) ^ (z))

/* tm_md4_rotate_left rotates x left n bits.
 */
#define tm_md4_rotate_left(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* tm_md4_ff, tm_md4_gg and tm_md4_hh are transformations for rounds 1, 2 and 3 */
/* Rotation is separate from addition to prevent recomputation */

#define tm_md4_ff(a, b, c, d, x, s) { \
    (a) += tm_md4_f ((b), (c), (d)) + (x); \
    (a) = tm_md4_rotate_left ((a), (s)); \
    }
#define tm_md4_gg(a, b, c, d, x, s) { \
    (a) += tm_md4_g ((b), (c), (d)) + (x) + (tt32Bit) TM_UL(0x5a827999); \
    (a) = tm_md4_rotate_left ((a), (s)); \
    }
#define tm_md4_hh(a, b, c, d, x, s) { \
    (a) += tm_md4_h ((b), (c), (d)) + (x) + (tt32Bit) TM_UL(0x6ed9eba1); \
    (a) = tm_md4_rotate_left ((a), (s)); \
    }

/* MD4 initialization. Begins an MD4 operation, writing a new context.
 */

void tfMd4Init (ttVoidPtr contextPtr)
{
    ttMd4CtxPtr md4CtxPtr;
    md4CtxPtr = (ttMd4CtxPtr)contextPtr;

    md4CtxPtr->md4Count[0] = md4CtxPtr->md4Count[1] = 0;
/* Load magic initialization constants. */
    md4CtxPtr->md4State[0] = TM_UL(0x67452301);
    md4CtxPtr->md4State[1] = TM_UL(0xefcdab89);
    md4CtxPtr->md4State[2] = TM_UL(0x98badcfe);
    md4CtxPtr->md4State[3] = TM_UL(0x10325476);
}

/* MD4 block update operation. Continues an MD4 message-digest
 * operation, processing another message block, and updating the
 * context.
 */
void tfMd4Update (ttVoidPtr             contextPtr, 
                  ttConst8BitPtr        input,         
                  ttPktLen              inputLen)        
{
    ttPktLen      i;
    ttPktLen      index;
    ttPktLen      partLen;
    ttMd4CtxPtr   md4CtxPtr;

    md4CtxPtr = (ttMd4CtxPtr)contextPtr;
    
/* Compute number of bytes mod 64 */
    index = (ttPktLen)((md4CtxPtr->md4Count[0] >> 3) & 0x3F);
/* Update number of bits */
    if ((md4CtxPtr->md4Count[0] += ((tt32Bit)inputLen << 3))
        < ((tt32Bit)inputLen << 3))
    {
        md4CtxPtr->md4Count[1]++;
    }
    md4CtxPtr->md4Count[1] += ((tt32Bit)inputLen >> 29);
    
    partLen = (ttPktLen) (64 - index);
    
/* Transform as many times as possible.
 */
    if (inputLen >= partLen) 
    {
        tm_bcopy((tt8BitPtr)input, 
                 (tt8BitPtr)&md4CtxPtr->md4Buffer[index],
                 partLen);
        tfMD4Transform (md4CtxPtr->md4State, md4CtxPtr->md4Buffer);
        
        for (i = partLen; i + 63 < inputLen; i += 64)
        {
            tfMD4Transform (md4CtxPtr->md4State, &input[i]);
        }
        
        index = 0;
    }
    else
    {
        i = 0;
    }
    
/* Buffer remaining input */
    tm_bcopy((tt8BitPtr)&input[i],
             (tt8BitPtr)&md4CtxPtr->md4Buffer[index],
             inputLen-i);
}

/* MD4 finalization. Ends an MD4 message-digest operation, writing the
 * the message digest and zeroizing the context.
 */
void tfMd4Final (tt8BitPtr   digestPtr,
                 ttVoidPtr   contextPtr)
{
    tt8Bit        bits[8];
    ttPktLen      index;
    ttPktLen      padLen;
    ttMd4CtxPtr   md4CtxPtr;

    md4CtxPtr = (ttMd4CtxPtr)contextPtr;
    
/* Save number of bits */
    tfMD4Encode (bits, md4CtxPtr->md4Count, 8);
    
/* Pad out to 56 mod 64.
 */
    index = (ttPktLen)((md4CtxPtr->md4Count[0] >> 3) & 0x3f);
    padLen = (index < 56) ? (56 - index) : (120 - index);
    tfMd4Update (md4CtxPtr, tlMd4Padding, padLen);
    
/* Append length (before tlMd4Padding) */
    tfMd4Update (md4CtxPtr, (ttConst8BitPtr)bits, 8);
/* Store md4State in digest */
    tfMD4Encode (digestPtr, md4CtxPtr->md4State, 16);
    
/* Zeroize sensitive information.
 */
    tm_bzero((tt8BitPtr)md4CtxPtr, sizeof (*md4CtxPtr));
}

/* MD4 basic transformation. Transforms md4State based on block.
 */
static void tfMD4Transform (tt32Bit state[4], const tt8Bit block[64])
{
    tt32Bit a = state[0], b = state[1], c = state[2], d = state[3], x[16];
    
    tfMD4Decode (x, block, 64);
    
/* Round 1 */
    tm_md4_ff (a, b, c, d, x[ 0], S11); /* 1 */
    tm_md4_ff (d, a, b, c, x[ 1], S12); /* 2 */
    tm_md4_ff (c, d, a, b, x[ 2], S13); /* 3 */
    tm_md4_ff (b, c, d, a, x[ 3], S14); /* 4 */
    tm_md4_ff (a, b, c, d, x[ 4], S11); /* 5 */
    tm_md4_ff (d, a, b, c, x[ 5], S12); /* 6 */
    tm_md4_ff (c, d, a, b, x[ 6], S13); /* 7 */
    tm_md4_ff (b, c, d, a, x[ 7], S14); /* 8 */
    tm_md4_ff (a, b, c, d, x[ 8], S11); /* 9 */
    tm_md4_ff (d, a, b, c, x[ 9], S12); /* 10 */
    tm_md4_ff (c, d, a, b, x[10], S13); /* 11 */
    tm_md4_ff (b, c, d, a, x[11], S14); /* 12 */
    tm_md4_ff (a, b, c, d, x[12], S11); /* 13 */
    tm_md4_ff (d, a, b, c, x[13], S12); /* 14 */
    tm_md4_ff (c, d, a, b, x[14], S13); /* 15 */
    tm_md4_ff (b, c, d, a, x[15], S14); /* 16 */
    
/* Round 2 */
    tm_md4_gg (a, b, c, d, x[ 0], S21); /* 17 */
    tm_md4_gg (d, a, b, c, x[ 4], S22); /* 18 */
    tm_md4_gg (c, d, a, b, x[ 8], S23); /* 19 */
    tm_md4_gg (b, c, d, a, x[12], S24); /* 20 */
    tm_md4_gg (a, b, c, d, x[ 1], S21); /* 21 */
    tm_md4_gg (d, a, b, c, x[ 5], S22); /* 22 */
    tm_md4_gg (c, d, a, b, x[ 9], S23); /* 23 */
    tm_md4_gg (b, c, d, a, x[13], S24); /* 24 */
    tm_md4_gg (a, b, c, d, x[ 2], S21); /* 25 */
    tm_md4_gg (d, a, b, c, x[ 6], S22); /* 26 */
    tm_md4_gg (c, d, a, b, x[10], S23); /* 27 */
    tm_md4_gg (b, c, d, a, x[14], S24); /* 28 */
    tm_md4_gg (a, b, c, d, x[ 3], S21); /* 29 */
    tm_md4_gg (d, a, b, c, x[ 7], S22); /* 30 */
    tm_md4_gg (c, d, a, b, x[11], S23); /* 31 */
    tm_md4_gg (b, c, d, a, x[15], S24); /* 32 */
    
/* Round 3 */
    tm_md4_hh (a, b, c, d, x[ 0], S31); /* 33 */
    tm_md4_hh (d, a, b, c, x[ 8], S32); /* 34 */
    tm_md4_hh (c, d, a, b, x[ 4], S33); /* 35 */
    tm_md4_hh (b, c, d, a, x[12], S34); /* 36 */
    tm_md4_hh (a, b, c, d, x[ 2], S31); /* 37 */
    tm_md4_hh (d, a, b, c, x[10], S32); /* 38 */
    tm_md4_hh (c, d, a, b, x[ 6], S33); /* 39 */
    tm_md4_hh (b, c, d, a, x[14], S34); /* 40 */
    tm_md4_hh (a, b, c, d, x[ 1], S31); /* 41 */
    tm_md4_hh (d, a, b, c, x[ 9], S32); /* 42 */
    tm_md4_hh (c, d, a, b, x[ 5], S33); /* 43 */
    tm_md4_hh (b, c, d, a, x[13], S34); /* 44 */
    tm_md4_hh (a, b, c, d, x[ 3], S31); /* 45 */
    tm_md4_hh (d, a, b, c, x[11], S32); /* 46 */
    tm_md4_hh (c, d, a, b, x[ 7], S33); /* 47 */
    tm_md4_hh (b, c, d, a, x[15], S34); /* 48 */
    
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    
/* Zeroize sensitive information.
 */
    tm_bzero((tt8BitPtr)x, sizeof (x));
}

/* tfMD4Encodes input (tt32Bit) into output (tt8Bit). Assumes len is
 * a multiple of 4.
 */
static void tfMD4Encode (tt8BitPtr  output,
                         tt32BitPtr input,
                         ttPktLen   len)
{
    ttPktLen i, j;
    
    for (i = 0, j = 0; j < len; i++, j += 4) 
    {
        output[j] = tm_8bit(input[i]);
        output[j+1] = tm_8bit(input[i] >> 8);
        output[j+2] = tm_8bit(input[i] >> 16);
        output[j+3] = tm_8bit(input[i] >> 24);
    }
}

/* tfMD4Decodes input (tt8Bit) into output (tt32Bit). Assumes len is
 * a multiple of 4.
 */
static void tfMD4Decode (tt32BitPtr      output,
                         ttConst8BitPtr  input,
                         ttPktLen        len)
{
    ttPktLen i, j;
    
    for (i = 0, j = 0; j < len; i++, j += 4)
    {
        output[i] = ((tt32Bit)input[j]) | (((tt32Bit)input[j+1]) << 8) |
        (((tt32Bit)input[j+2]) << 16) | (((tt32Bit)input[j+3]) << 24);
    }
}


#else /* ! TM_USE_MD4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_MD4 is not defined */
int tlMd4Dummy = 0;

#endif /* TM_USE_MD4 */
