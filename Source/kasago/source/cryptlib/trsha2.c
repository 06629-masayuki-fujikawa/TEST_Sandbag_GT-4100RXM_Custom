/*
 * Description: SHA-256 alg from modified Treck trsha1.c implementation,
 *              based on the KAME SHA-1 implementation
 *
 * Filename: trsha2.c
 * Author: J Poffenbarger
 * Date Created: 07/12/2007
 * $Source: source/cryptlib/trsha2.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:35:41JST $
 * $Author: odile $
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
 * FIPS pub 180-2: Secure Hash Standard
 * based on: http://csrc.nist.gov/publications/fips/fips180-2/fips180-2.pdf
 */
 
#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>

/* Only for SHA256 Algorithm */
#ifdef TM_USE_SHA256

#define TM_HMAC_SHA256_BLOCK_SIZE 64

#define ROUND_00_15(i,a,b,c,d,e,f,g,h) \
    T1 += h + Sigma1_32(e) + Ch_32(e,f,g) + K256[i];  \
    h = Sigma0_32(a) + Maj_32(a,b,c); \
    d += T1; \
    h += T1;

#define ROUND_16_63(i,a,b,c,d,e,f,g,h,X) \
            s0 = X[(i+1)&0x0f]; \
            s0 = sigma0_32(s0); \
            s1 = X[(i+14)&0x0f]; \
            s1 = sigma1_32(s1); \
            T1 = X[(i)&0x0f] += s0 + s1 + X[(i+9)&0x0f]; \
            ROUND_00_15(i,a,b,c,d,e,f,g,h);

/* SHA-256 initial values */
#define h0initSHA256  TM_UL(0x6A09E667) 
#define h1initSHA256  TM_UL(0xBB67AE85) 
#define h2initSHA256  TM_UL(0x3C6EF372) 
#define h3initSHA256  TM_UL(0xA54FF53A) 
#define h4initSHA256  TM_UL(0x510E527F) 
#define h5initSHA256  TM_UL(0x9B05688C) 
#define h6initSHA256  TM_UL(0x1F83D9AB) 
#define h7initSHA256  TM_UL(0x5BE0CD19) 

#define ROTATE_LEFT32(a,n) (((a)<<(n))|(((a)&0xffffffff)>>(32-(n))))
#define Ch_32(x,y,z)   (((x) & (y)) ^ ((~(x)) & (z)))
#define Maj_32(x,y,z)  (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

/*
 * FIPS specification refers to right rotations, while our ROTATE macro
 * is left one. This is why you might notice that rotation coefficients
 * differ from those observed in FIPS document by 32-N...
 */
#define Sigma0_32(x)   (ROTATE_LEFT32((x),30) ^ ROTATE_LEFT32((x),19) ^ \
                     ROTATE_LEFT32((x),10))
#define Sigma1_32(x)   (ROTATE_LEFT32((x),26) ^ ROTATE_LEFT32((x),21) ^ \
                     ROTATE_LEFT32((x),7))
#define sigma0_32(x)   (ROTATE_LEFT32((x),25) ^ ROTATE_LEFT32((x),14) ^ \
                    ((x)>>3))
#define sigma1_32(x)   (ROTATE_LEFT32((x),15) ^ ROTATE_LEFT32((x),13) ^ \
                    ((x)>>10))

static void tfSha2ToByte(tt8BitPtr outputPtr, tt32BitPtr input, ttPktLen len);

static void tfSha256LongReverse(tt32BitPtr buffer, tt32Bit byteCount);

static void tfSha256Transform(tt32BitPtr digestPtr, tt32BitPtr dataPtr);

void tfSha256Update(ttVoidPtr  sha2ContextPtr,
                    tt8BitPtr  buffer, 
                    ttPktLen   count, 
                    ttPktLen   offset);

/* SHA-256 constants */
static const tt32Bit K256[64] = 
{
      0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b,
      0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01,
      0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7,
      0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
      0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152,
      0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
      0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc,
      0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
      0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819,
      0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08,
      0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f,
      0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
      0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

void tfSha256Init(ttVoidPtr sha256ContextPtr)
{
    ttSha256CtxPtr shaContextPtr;

    shaContextPtr = (ttSha256CtxPtr)sha256ContextPtr;

/* Set the h-vars to their initial values */
    shaContextPtr->shaDigest[0] = h0initSHA256;
    shaContextPtr->shaDigest[1] = h1initSHA256;
    shaContextPtr->shaDigest[2] = h2initSHA256;
    shaContextPtr->shaDigest[3] = h3initSHA256;
    shaContextPtr->shaDigest[4] = h4initSHA256;
    shaContextPtr->shaDigest[5] = h5initSHA256;
    shaContextPtr->shaDigest[6] = h6initSHA256;
    shaContextPtr->shaDigest[7] = h7initSHA256;

/* Initialize bit count */
    shaContextPtr->shaCountLo = shaContextPtr->shaCountHi = 0;
}

void tfSha256Update(ttVoidPtr  sha2ContextPtr,
                    tt8BitPtr  buffer, 
                    ttPktLen   count, 
                    ttPktLen   offset)
{
    ttPktLen        dataCount;
    tt8BitPtr       p;
    ttSha256CtxPtr  shaContextPtr;

    TM_UNREF_IN_ARG(offset); /* for 16Bit or 32Bit DSP usage */
    
    shaContextPtr = (ttSha256CtxPtr)sha2ContextPtr;    

/* Get count of bytes already in data, buffer 
 * This is the data passed in % BLOCK_SIZE,
 * the amount of information that has not yet been digested
 * We convert the seen bits to bytes, then do manual modulus on (block size)
 */
    dataCount = (ttPktLen)((shaContextPtr->shaCountLo >> 3) & 
                (TM_HMAC_SHA256_BLOCK_SIZE-1));    
    
/* Update bitcount, total bits in message, not used in Update */    
/* Add number of bits in the buffer to shaCountLo, test for 32-bit overflow */
    shaContextPtr->shaCountLo += (count<<3);
    if (shaContextPtr->shaCountLo  < (count<<3))
    {
/* Carry from low to high */
        shaContextPtr->shaCountHi++;
    }
    shaContextPtr->shaCountHi += count >> 29;

/* Handle any leading odd-sized chunks */
    if (dataCount)
    {
        p = (tt8BitPtr)shaContextPtr->shaData + dataCount;
/* Find how much space is left in shaData */
        dataCount = TM_HMAC_SHA256_BLOCK_SIZE - dataCount;
/* If the new data will all fit, put it in */
        if (count < dataCount)
        {
/* Since it all fits, copy COUNT bytes of data from BUFFER to P */
            tm_memcpy(p, buffer, count);
            goto UPDATE_RETURN;
        }
/* If it doesn't all fit, copy DATACOUNT bytes of buffer to P, where
 * dataCount is the remaining space in P
 */
        tm_memcpy(p, buffer, dataCount);
/* Byte reversal for little-endian */
        tfSha256LongReverse(shaContextPtr->shaData, TM_HMAC_SHA256_BLOCK_SIZE);
/* The data to be hashed is in shaData, the buffer we just filled */
        tfSha256Transform(shaContextPtr->shaDigest, shaContextPtr->shaData);
/* next time we put anything into buffer, it goes at the end */
        buffer += dataCount;
/* we have count-count bytes left in the buffer to process */
        count -= dataCount;
    }

/* Process data in TM_HMAC_SHA_BLOCK_SIZE chunks */
    while (count >= TM_HMAC_SHA256_BLOCK_SIZE)
    {
/* copy a full block from buffer to shaData, the process it */
        tm_memcpy((tt8BitPtr)shaContextPtr->shaData, 
                  (tt8BitPtr)buffer, 
                  TM_HMAC_SHA256_BLOCK_SIZE );
        tfSha256LongReverse(shaContextPtr->shaData, 
                            TM_HMAC_SHA256_BLOCK_SIZE);
        tfSha256Transform(shaContextPtr->shaDigest, 
                          shaContextPtr->shaData);
/* adjust buffer pointer and byte count again */
        buffer += TM_HMAC_SHA256_BLOCK_SIZE;
        count  -= TM_HMAC_SHA256_BLOCK_SIZE;
    }

/* Handle any remaining bytes of data. */
/* completely empty the buffer of the remaining COUNT bytes, which is less than the block size */
    tm_memcpy((tt8BitPtr)shaContextPtr->shaData, buffer, count);
UPDATE_RETURN:
    return;
}

/* Perform the SHA transformation.  Note that this code, like MD5, seems to
 * break some optimizing compilers due to the complexity of the expressions
 * and the size of the basic block.  It may be necessary to split it into
 * sections, e.g. based on the four tm_sha_sub_rounds
 * 
 * Note that this corrupts the  shaContextPtr->data area 
 */
static void tfSha256Transform(tt32BitPtr digestPtr, tt32BitPtr dataPtr)
{
    tt32Bit i;
    tt32Bit X[16];
    tt32Bit a, b, c, d, e, f, g, h, s0, s1, T1;

    a = digestPtr[0];   
    b = digestPtr[1];   
    c = digestPtr[2];   
    d = digestPtr[3];
    e = digestPtr[4];   
    f = digestPtr[5];   
    g = digestPtr[6];   
    h = digestPtr[7];

    T1 = X[0] = dataPtr[0];   ROUND_00_15(0,a,b,c,d,e,f,g,h);
    T1 = X[1] = dataPtr[1];   ROUND_00_15(1,h,a,b,c,d,e,f,g);
    T1 = X[2] = dataPtr[2];   ROUND_00_15(2,g,h,a,b,c,d,e,f);
    T1 = X[3] = dataPtr[3];   ROUND_00_15(3,f,g,h,a,b,c,d,e);
    T1 = X[4] = dataPtr[4];   ROUND_00_15(4,e,f,g,h,a,b,c,d);
    T1 = X[5] = dataPtr[5];   ROUND_00_15(5,d,e,f,g,h,a,b,c);
    T1 = X[6] = dataPtr[6];   ROUND_00_15(6,c,d,e,f,g,h,a,b);
    T1 = X[7] = dataPtr[7];   ROUND_00_15(7,b,c,d,e,f,g,h,a);
    T1 = X[8] = dataPtr[8];   ROUND_00_15(8,a,b,c,d,e,f,g,h);
    T1 = X[9] = dataPtr[9];   ROUND_00_15(9,h,a,b,c,d,e,f,g);
    T1 = X[10] = dataPtr[10]; ROUND_00_15(10,g,h,a,b,c,d,e,f);
    T1 = X[11] = dataPtr[11]; ROUND_00_15(11,f,g,h,a,b,c,d,e);
    T1 = X[12] = dataPtr[12]; ROUND_00_15(12,e,f,g,h,a,b,c,d);
    T1 = X[13] = dataPtr[13]; ROUND_00_15(13,d,e,f,g,h,a,b,c);
    T1 = X[14] = dataPtr[14]; ROUND_00_15(14,c,d,e,f,g,h,a,b);
    T1 = X[15] = dataPtr[15]; ROUND_00_15(15,b,c,d,e,f,g,h,a);

    for(i = 16; i < 64; i+=8)
    {
        ROUND_16_63(i+0,a,b,c,d,e,f,g,h,X);
        ROUND_16_63(i+1,h,a,b,c,d,e,f,g,X);
        ROUND_16_63(i+2,g,h,a,b,c,d,e,f,X);
        ROUND_16_63(i+3,f,g,h,a,b,c,d,e,X);
        ROUND_16_63(i+4,e,f,g,h,a,b,c,d,X);
        ROUND_16_63(i+5,d,e,f,g,h,a,b,c,X);
        ROUND_16_63(i+6,c,d,e,f,g,h,a,b,X);
        ROUND_16_63(i+7,b,c,d,e,f,g,h,a,X);
    }

    digestPtr[0] += a;  
    digestPtr[1] += b;  
    digestPtr[2] += c;  
    digestPtr[3] += d;
    digestPtr[4] += e;  
    digestPtr[5] += f;  
    digestPtr[6] += g;  
    digestPtr[7] += h;
}

void tfSha256Final(tt8BitPtr     outputPtr,
                   ttVoidPtr     sha2ContextPtr)
{
    ttPktLen       count;
    tt8BitPtr      dataPtr;
    ttSha256CtxPtr shaContextPtr;

    shaContextPtr = (ttSha256CtxPtr)sha2ContextPtr;

/* Compute number of bytes mod 64 */
    count = (ttPktLen)((shaContextPtr->shaCountLo >> 3) & 0x3F);

/* Set the first char of padding to 0x80.  This is safe since there is
 * always at least one byte free 
 */
    dataPtr = (tt8BitPtr)shaContextPtr->shaData + count;
    *dataPtr++ = 0x80;

/* Bytes of padding needed to make 64 bytes */
    count = TM_HMAC_SHA256_BLOCK_SIZE - 1 - count;

/* Pad out to 56 mod 64 */
    if (count < 8)
    {
/* Two lots of padding:  Pad the first block to 64 bytes */
        tm_memset( dataPtr, 0, count );
        tfSha256LongReverse(shaContextPtr->shaData, TM_HMAC_SHA256_BLOCK_SIZE);
        tfSha256Transform(shaContextPtr->shaDigest, shaContextPtr->shaData);

/* Now fill the next block with 56 bytes */
        tm_memset((tt8BitPtr)shaContextPtr->shaData, 0, 
                   TM_HMAC_SHA256_BLOCK_SIZE - 8);
    }
    else
    {
/* Pad block to 56 bytes */
        tm_memset(dataPtr, 0, count - 8);
    }

/* Append length in bits and transform */
    shaContextPtr->shaData[14] = shaContextPtr->shaCountHi;
    shaContextPtr->shaData[15] = shaContextPtr->shaCountLo;

    tfSha256LongReverse(shaContextPtr->shaData, TM_HMAC_SHA256_BLOCK_SIZE - 8);
    tfSha256Transform(shaContextPtr->shaDigest, shaContextPtr->shaData);

/* Output to an array of bytes */
    tfSha2ToByte(outputPtr, shaContextPtr->shaDigest, 32);

/* Zeroise sensitive stuff */
    tm_memset((tt8BitPtr)shaContextPtr, 0, sizeof(ttSha256Ctx));
}

static void tfSha2ToByte(tt8BitPtr outputPtr, tt32BitPtr  input, ttPktLen len)
{    
/* Output SHA-256 outputPtr in tt8Bit array */
    ttPktLen i, j;

    for(i = 0, j = 0; j < len; i++, j += 4) 
    {
        outputPtr[j]   = tm_8bit(input[i] >> 24);
        outputPtr[j+1] = tm_8bit(input[i] >> 16);
        outputPtr[j+2] = tm_8bit(input[i] >> 8 );
        outputPtr[j+3] = tm_8bit(input[i]);
    }
}

/* When run on a little-endian CPU we need to perform byte reversal on an
 * array of long words. 
 */
static void tfSha256LongReverse(tt32BitPtr buffer, tt32Bit byteCount)
{
#ifdef TM_LITTLE_ENDIAN
    tt32Bit value;

    byteCount /= sizeof(tt32Bit);
    while(byteCount--)
    {
        value = *buffer;
        tm_ntohl(value, *buffer);
        buffer++;
    }
#else /* !TM_LITTLE_ENDIAN */
   TM_UNREF_IN_ARG(buffer);
   TM_UNREF_IN_ARG(byteCount);
#endif /* TM_LITTLE_ENDIAN */
    return;
}
#else /* !TM_USE_SHA256 */
LINT_UNUSED_HEADER

/* To allow link for builds when TM_USE_SHA256 is not defined */
int tvSha256Dummy = 0;
#endif /* TM_USE_SHA256 */

/***************** End Of File *****************/
