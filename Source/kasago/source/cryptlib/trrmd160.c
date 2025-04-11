/*
 * Description: Testing the AH algorithems: RIPEMD160-96
 *
 * Filename: trrmd160.c
 * Author: Jin Shang
 * Date Created: 06/06/2002
 * $Source: source/cryptlib/trrmd160.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:35:37JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


/*  $OpenBSD: rmd160.c,v 1.3 2001/09/26 21:40:13 markus Exp $   */
/*
 * Copyright (c) 2001 Markus Friedl.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Preneel, Bosselaers, Dobbertin, "The Cryptographic Hash Function RIPEMD-160",
 * RSA Laboratories, CryptoBytes, Volume 3, Number 2, Autumn 1997,
 * ftp://ftp.rsasecurity.com/pub/cryptobytes/crypto3n2.pdf
 */

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>

#ifdef TM_USE_RIPEMD

#define TM_HMAC_RIPEMD_BLOCK_SIZE   64

#define tm_rmd160_put_32bit_le(cp, value)  { \
    (cp)[3] = (tt8Bit)((value) >> 24); \
    (cp)[2] = (tt8Bit)((value) >> 16); \
    (cp)[1] = (tt8Bit)((value) >> 8); \
    (cp)[0] = (tt8Bit)(value); } 

#define H0  TM_UL(0x67452301)
#define H1  TM_UL(0xEFCDAB89)
#define H2  TM_UL(0x98BADCFE)
#define H3  TM_UL(0x10325476)
#define H4  TM_UL(0xC3D2E1F0)

#define K0  TM_UL(0x00000000)
#define K1  TM_UL(0x5A827999)
#define K2  TM_UL(0x6ED9EBA1)
#define K3  TM_UL(0x8F1BBCDC)
#define K4  TM_UL(0xA953FD4E)

#define KK0 TM_UL(0x50A28BE6)
#define KK1 TM_UL(0x5C4DD124)
#define KK2 TM_UL(0x6D703EF3)
#define KK3 TM_UL(0x7A6D76E9)
#define KK4 TM_UL(0x00000000)

/* rotate x left n bits.  */
#define tm_rmd160_rol(n, x) (((x) << (n)) | ((x) >> (32-(n))))
#define tm_rmd160_f0(x, y, z) ((x) ^ (y) ^ (z))
#define tm_rmd160_f1(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define tm_rmd160_f2(x, y, z) (((x) | (~y)) ^ (z))
#define tm_rmd160_f3(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define tm_rmd160_f4(x, y, z) ((x) ^ ((y) | (~z)))  

#define tm_rmd160_r_f0(a, b, c, d, e, Kj, sj, rj) \
        a = tm_rmd160_rol(sj, a + tm_rmd160_f0(b,c,d) + x[rj] + Kj) + e; \
        c = tm_rmd160_rol(10, c); 
#define tm_rmd160_r_f1(a, b, c, d, e, Kj, sj, rj) \
        a = tm_rmd160_rol(sj, a + tm_rmd160_f1(b,c,d) + x[rj] + Kj) + e; \
        c = tm_rmd160_rol(10, c); 
#define tm_rmd160_r_f2(a, b, c, d, e, Kj, sj, rj) \
        a = tm_rmd160_rol(sj, a + tm_rmd160_f2(b,c,d) + x[rj] + Kj) + e; \
        c = tm_rmd160_rol(10, c); 
#define tm_rmd160_r_f3(a, b, c, d, e, Kj, sj, rj) \
        a = tm_rmd160_rol(sj, a + tm_rmd160_f3(b,c,d) + x[rj] + Kj) + e; \
        c = tm_rmd160_rol(10, c); 
#define tm_rmd160_r_f4(a, b, c, d, e, Kj, sj, rj) \
        a = tm_rmd160_rol(sj, a + tm_rmd160_f4(b,c,d) + x[rj] + Kj) + e; \
        c = tm_rmd160_rol(10, c); 



static const tt8Bit  TM_CONST_QLF tlRmd160Padding[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void tfRmd160Transform(tt32Bit state[5], tt8BitPtr block);

void tfRmd160Init(ttVoidPtr ctx)
{
    ttRmd160CtxPtr rmd160CtxPtr;
    rmd160CtxPtr = (ttRmd160CtxPtr)ctx;
    
    rmd160CtxPtr->rmdCountLo     = 0;
    rmd160CtxPtr->rmdCountHi     = 0;
    rmd160CtxPtr->rmdDigest[0]   = H0;
    rmd160CtxPtr->rmdDigest[1]   = H1;
    rmd160CtxPtr->rmdDigest[2]   = H2;
    rmd160CtxPtr->rmdDigest[3]   = H3;
    rmd160CtxPtr->rmdDigest[4]   = H4;
}

void tfRmd160Update(ttVoidPtr  ctx, 
                    tt8BitPtr input, 
                    ttPktLen count , 
                    ttPktLen offset )
{
    tt32Bit tmp;
    ttPktLen have, off, need, len;
    ttRmd160CtxPtr rmd160CtxPtr;
    rmd160CtxPtr = ( ttRmd160CtxPtr ) ctx;

    len = count;
    TM_UNREF_IN_ARG(offset);
    have = (ttPktLen) ((rmd160CtxPtr->rmdCountLo/8) % 64);
    need = 64 - have;

    tmp = rmd160CtxPtr->rmdCountLo ;
    rmd160CtxPtr->rmdCountLo = tmp + ((tt32Bit)len << 3);
    if (rmd160CtxPtr->rmdCountLo < tmp)
        rmd160CtxPtr->rmdCountHi++;
    rmd160CtxPtr->rmdCountHi += len>>29; 
    
    off = 0;

    if (len >= need)
    {
        if (have) 
        {
            tm_memcpy((tt8BitPtr)rmd160CtxPtr->rmdData + have, input, need);
            tfRmd160Transform(
                rmd160CtxPtr->rmdDigest, (tt8BitPtr)rmd160CtxPtr->rmdData);
            off = need;
            have = 0;
        }
        /* now the buffer is empty */
        while (off + 64 <= len) 
        {
            tfRmd160Transform(rmd160CtxPtr->rmdDigest, input+off);
            off += 64;
        }
    }
    if (off < len)
        tm_memcpy(
            (tt8BitPtr)(rmd160CtxPtr->rmdData) + have, input+off, len-off);
}

void tfRmd160Final(tt8BitPtr digest, ttVoidPtr ctx)
{
    int i;
    tt8Bit size[8];
    tt32Bit padlen;
    
    ttRmd160CtxPtr rmd160CtxPtr;
    rmd160CtxPtr = ( ttRmd160CtxPtr ) ctx;

    tm_rmd160_put_32bit_le(size, rmd160CtxPtr->rmdCountLo);
    tm_rmd160_put_32bit_le(&size[4], rmd160CtxPtr->rmdCountHi);

    /*
     * pad to 64 byte blocks, at least one byte from PADDING plus 8 bytes
     * for the size
     */
    padlen = 64 - ((rmd160CtxPtr->rmdCountLo/8) % 64);
    if (padlen < 1 + 8)
        padlen += 64;
    tfRmd160Update(                             /* padlen - 8 <= 64 */
        rmd160CtxPtr, (tt8BitPtr)tlRmd160Padding, (ttPktLen)padlen - 8,0);
    tfRmd160Update(rmd160CtxPtr, size, 8,0);

    if (digest != (ttVoidPtr) 0)
        for (i = 0; i < 5; i++)
            tm_rmd160_put_32bit_le(
                digest + i*4, rmd160CtxPtr->rmdDigest[i]);

    tm_memset(rmd160CtxPtr, 0, sizeof (*rmd160CtxPtr));
}

void tfRmd160Transform(tt32Bit state[5], tt8BitPtr block)
{
    tt32Bit a, b, c, d, e, aa, bb, cc, dd, ee, t, x[16];

#ifdef TM_LITTLE_ENDIAN
    tm_memcpy(x, (tt8BitPtr)block, TM_HMAC_RIPEMD_BLOCK_SIZE);
#else
    int i;
    tt32Bit value;
    
    for (i = 0; i < 16; i++)
    {
        value = *((tt32BitPtr)(block+i*4));
        value = ( ( value & TM_UL(0xFF00FF00) ) >> 8  ) | \
                ( ( value & TM_UL(0x00FF00FF) ) << 8 );
        x[i] = ( value << 16 ) | ( value >> 16 );
    }
#endif

    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];

    /* Round 1 */
    tm_rmd160_r_f0(a, b, c, d, e, K0, 11,  0);
    tm_rmd160_r_f0(e, a, b, c, d, K0, 14,  1);
    tm_rmd160_r_f0(d, e, a, b, c, K0, 15,  2);
    tm_rmd160_r_f0(c, d, e, a, b, K0, 12,  3);
    tm_rmd160_r_f0(b, c, d, e, a, K0,  5,  4);
    tm_rmd160_r_f0(a, b, c, d, e, K0,  8,  5);
    tm_rmd160_r_f0(e, a, b, c, d, K0,  7,  6);
    tm_rmd160_r_f0(d, e, a, b, c, K0,  9,  7);
    tm_rmd160_r_f0(c, d, e, a, b, K0, 11,  8);
    tm_rmd160_r_f0(b, c, d, e, a, K0, 13,  9);
    tm_rmd160_r_f0(a, b, c, d, e, K0, 14, 10);
    tm_rmd160_r_f0(e, a, b, c, d, K0, 15, 11);
    tm_rmd160_r_f0(d, e, a, b, c, K0,  6, 12);
    tm_rmd160_r_f0(c, d, e, a, b, K0,  7, 13);
    tm_rmd160_r_f0(b, c, d, e, a, K0,  9, 14);
    tm_rmd160_r_f0(a, b, c, d, e, K0,  8, 15); /* #15 */
    /* Round 2 */
    tm_rmd160_r_f1(e, a, b, c, d, K1,  7,  7);
    tm_rmd160_r_f1(d, e, a, b, c, K1,  6,  4);
    tm_rmd160_r_f1(c, d, e, a, b, K1,  8, 13);
    tm_rmd160_r_f1(b, c, d, e, a, K1, 13,  1);
    tm_rmd160_r_f1(a, b, c, d, e, K1, 11, 10);
    tm_rmd160_r_f1(e, a, b, c, d, K1,  9,  6);
    tm_rmd160_r_f1(d, e, a, b, c, K1,  7, 15);
    tm_rmd160_r_f1(c, d, e, a, b, K1, 15,  3);
    tm_rmd160_r_f1(b, c, d, e, a, K1,  7, 12);
    tm_rmd160_r_f1(a, b, c, d, e, K1, 12,  0);
    tm_rmd160_r_f1(e, a, b, c, d, K1, 15,  9);
    tm_rmd160_r_f1(d, e, a, b, c, K1,  9,  5);
    tm_rmd160_r_f1(c, d, e, a, b, K1, 11,  2);
    tm_rmd160_r_f1(b, c, d, e, a, K1,  7, 14);
    tm_rmd160_r_f1(a, b, c, d, e, K1, 13, 11);
    tm_rmd160_r_f1(e, a, b, c, d, K1, 12,  8); /* #31 */
    /* Round 3 */
    tm_rmd160_r_f2(d, e, a, b, c, K2, 11,  3);
    tm_rmd160_r_f2(c, d, e, a, b, K2, 13, 10);
    tm_rmd160_r_f2(b, c, d, e, a, K2,  6, 14);
    tm_rmd160_r_f2(a, b, c, d, e, K2,  7,  4);
    tm_rmd160_r_f2(e, a, b, c, d, K2, 14,  9);
    tm_rmd160_r_f2(d, e, a, b, c, K2,  9, 15);
    tm_rmd160_r_f2(c, d, e, a, b, K2, 13,  8);
    tm_rmd160_r_f2(b, c, d, e, a, K2, 15,  1);
    tm_rmd160_r_f2(a, b, c, d, e, K2, 14,  2);
    tm_rmd160_r_f2(e, a, b, c, d, K2,  8,  7);
    tm_rmd160_r_f2(d, e, a, b, c, K2, 13,  0);
    tm_rmd160_r_f2(c, d, e, a, b, K2,  6,  6);
    tm_rmd160_r_f2(b, c, d, e, a, K2,  5, 13);
    tm_rmd160_r_f2(a, b, c, d, e, K2, 12, 11);
    tm_rmd160_r_f2(e, a, b, c, d, K2,  7,  5);
    tm_rmd160_r_f2(d, e, a, b, c, K2,  5, 12); /* #47 */
    /* Round 4 */
    tm_rmd160_r_f3(c, d, e, a, b, K3, 11,  1);
    tm_rmd160_r_f3(b, c, d, e, a, K3, 12,  9);
    tm_rmd160_r_f3(a, b, c, d, e, K3, 14, 11);
    tm_rmd160_r_f3(e, a, b, c, d, K3, 15, 10);
    tm_rmd160_r_f3(d, e, a, b, c, K3, 14,  0);
    tm_rmd160_r_f3(c, d, e, a, b, K3, 15,  8);
    tm_rmd160_r_f3(b, c, d, e, a, K3,  9, 12);
    tm_rmd160_r_f3(a, b, c, d, e, K3,  8,  4);
    tm_rmd160_r_f3(e, a, b, c, d, K3,  9, 13);
    tm_rmd160_r_f3(d, e, a, b, c, K3, 14,  3);
    tm_rmd160_r_f3(c, d, e, a, b, K3,  5,  7);
    tm_rmd160_r_f3(b, c, d, e, a, K3,  6, 15);
    tm_rmd160_r_f3(a, b, c, d, e, K3,  8, 14);
    tm_rmd160_r_f3(e, a, b, c, d, K3,  6,  5);
    tm_rmd160_r_f3(d, e, a, b, c, K3,  5,  6);
    tm_rmd160_r_f3(c, d, e, a, b, K3, 12,  2); /* #63 */
    /* Round 5 */
    tm_rmd160_r_f4(b, c, d, e, a, K4,  9,  4);
    tm_rmd160_r_f4(a, b, c, d, e, K4, 15,  0);
    tm_rmd160_r_f4(e, a, b, c, d, K4,  5,  5);
    tm_rmd160_r_f4(d, e, a, b, c, K4, 11,  9);
    tm_rmd160_r_f4(c, d, e, a, b, K4,  6,  7);
    tm_rmd160_r_f4(b, c, d, e, a, K4,  8, 12);
    tm_rmd160_r_f4(a, b, c, d, e, K4, 13,  2);
    tm_rmd160_r_f4(e, a, b, c, d, K4, 12, 10);
    tm_rmd160_r_f4(d, e, a, b, c, K4,  5, 14);
    tm_rmd160_r_f4(c, d, e, a, b, K4, 12,  1);
    tm_rmd160_r_f4(b, c, d, e, a, K4, 13,  3);
    tm_rmd160_r_f4(a, b, c, d, e, K4, 14,  8);
    tm_rmd160_r_f4(e, a, b, c, d, K4, 11, 11);
    tm_rmd160_r_f4(d, e, a, b, c, K4,  8,  6);
    tm_rmd160_r_f4(c, d, e, a, b, K4,  5, 15);
    tm_rmd160_r_f4(b, c, d, e, a, K4,  6, 13); /* #79 */

    aa = a ; bb = b; cc = c; dd = d; ee = e;

    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];

    /* Parallel round 1 */
    tm_rmd160_r_f4(a, b, c, d, e, KK0,  8,  5);
    tm_rmd160_r_f4(e, a, b, c, d, KK0,  9, 14);
    tm_rmd160_r_f4(d, e, a, b, c, KK0,  9,  7);
    tm_rmd160_r_f4(c, d, e, a, b, KK0, 11,  0);
    tm_rmd160_r_f4(b, c, d, e, a, KK0, 13,  9);
    tm_rmd160_r_f4(a, b, c, d, e, KK0, 15,  2);
    tm_rmd160_r_f4(e, a, b, c, d, KK0, 15, 11);
    tm_rmd160_r_f4(d, e, a, b, c, KK0,  5,  4);
    tm_rmd160_r_f4(c, d, e, a, b, KK0,  7, 13);
    tm_rmd160_r_f4(b, c, d, e, a, KK0,  7,  6);
    tm_rmd160_r_f4(a, b, c, d, e, KK0,  8, 15);
    tm_rmd160_r_f4(e, a, b, c, d, KK0, 11,  8);
    tm_rmd160_r_f4(d, e, a, b, c, KK0, 14,  1);
    tm_rmd160_r_f4(c, d, e, a, b, KK0, 14, 10);
    tm_rmd160_r_f4(b, c, d, e, a, KK0, 12,  3);
    tm_rmd160_r_f4(a, b, c, d, e, KK0,  6, 12); /* #15 */
    /* Parallel round 2 */
    tm_rmd160_r_f3(e, a, b, c, d, KK1,  9,  6);
    tm_rmd160_r_f3(d, e, a, b, c, KK1, 13, 11);
    tm_rmd160_r_f3(c, d, e, a, b, KK1, 15,  3);
    tm_rmd160_r_f3(b, c, d, e, a, KK1,  7,  7);
    tm_rmd160_r_f3(a, b, c, d, e, KK1, 12,  0);
    tm_rmd160_r_f3(e, a, b, c, d, KK1,  8, 13);
    tm_rmd160_r_f3(d, e, a, b, c, KK1,  9,  5);
    tm_rmd160_r_f3(c, d, e, a, b, KK1, 11, 10);
    tm_rmd160_r_f3(b, c, d, e, a, KK1,  7, 14);
    tm_rmd160_r_f3(a, b, c, d, e, KK1,  7, 15);
    tm_rmd160_r_f3(e, a, b, c, d, KK1, 12,  8);
    tm_rmd160_r_f3(d, e, a, b, c, KK1,  7, 12);
    tm_rmd160_r_f3(c, d, e, a, b, KK1,  6,  4);
    tm_rmd160_r_f3(b, c, d, e, a, KK1, 15,  9);
    tm_rmd160_r_f3(a, b, c, d, e, KK1, 13,  1);
    tm_rmd160_r_f3(e, a, b, c, d, KK1, 11,  2); /* #31 */
    /* Parallel round 3 */
    tm_rmd160_r_f2(d, e, a, b, c, KK2,  9, 15);
    tm_rmd160_r_f2(c, d, e, a, b, KK2,  7,  5);
    tm_rmd160_r_f2(b, c, d, e, a, KK2, 15,  1);
    tm_rmd160_r_f2(a, b, c, d, e, KK2, 11,  3);
    tm_rmd160_r_f2(e, a, b, c, d, KK2,  8,  7);
    tm_rmd160_r_f2(d, e, a, b, c, KK2,  6, 14);
    tm_rmd160_r_f2(c, d, e, a, b, KK2,  6,  6);
    tm_rmd160_r_f2(b, c, d, e, a, KK2, 14,  9);
    tm_rmd160_r_f2(a, b, c, d, e, KK2, 12, 11);
    tm_rmd160_r_f2(e, a, b, c, d, KK2, 13,  8);
    tm_rmd160_r_f2(d, e, a, b, c, KK2,  5, 12);
    tm_rmd160_r_f2(c, d, e, a, b, KK2, 14,  2);
    tm_rmd160_r_f2(b, c, d, e, a, KK2, 13, 10);
    tm_rmd160_r_f2(a, b, c, d, e, KK2, 13,  0);
    tm_rmd160_r_f2(e, a, b, c, d, KK2,  7,  4);
    tm_rmd160_r_f2(d, e, a, b, c, KK2,  5, 13); /* #47 */
    /* Parallel round 4 */
    tm_rmd160_r_f1(c, d, e, a, b, KK3, 15,  8);
    tm_rmd160_r_f1(b, c, d, e, a, KK3,  5,  6);
    tm_rmd160_r_f1(a, b, c, d, e, KK3,  8,  4);
    tm_rmd160_r_f1(e, a, b, c, d, KK3, 11,  1);
    tm_rmd160_r_f1(d, e, a, b, c, KK3, 14,  3);
    tm_rmd160_r_f1(c, d, e, a, b, KK3, 14, 11);
    tm_rmd160_r_f1(b, c, d, e, a, KK3,  6, 15);
    tm_rmd160_r_f1(a, b, c, d, e, KK3, 14,  0);
    tm_rmd160_r_f1(e, a, b, c, d, KK3,  6,  5);
    tm_rmd160_r_f1(d, e, a, b, c, KK3,  9, 12);
    tm_rmd160_r_f1(c, d, e, a, b, KK3, 12,  2);
    tm_rmd160_r_f1(b, c, d, e, a, KK3,  9, 13);
    tm_rmd160_r_f1(a, b, c, d, e, KK3, 12,  9);
    tm_rmd160_r_f1(e, a, b, c, d, KK3,  5,  7);
    tm_rmd160_r_f1(d, e, a, b, c, KK3, 15, 10);
    tm_rmd160_r_f1(c, d, e, a, b, KK3,  8, 14); /* #63 */
    /* Parallel round 5 */
    tm_rmd160_r_f0(b, c, d, e, a, KK4,  8, 12);
    tm_rmd160_r_f0(a, b, c, d, e, KK4,  5, 15);
    tm_rmd160_r_f0(e, a, b, c, d, KK4, 12, 10);
    tm_rmd160_r_f0(d, e, a, b, c, KK4,  9,  4);
    tm_rmd160_r_f0(c, d, e, a, b, KK4, 12,  1);
    tm_rmd160_r_f0(b, c, d, e, a, KK4,  5,  5);
    tm_rmd160_r_f0(a, b, c, d, e, KK4, 14,  8);
    tm_rmd160_r_f0(e, a, b, c, d, KK4,  6,  7);
    tm_rmd160_r_f0(d, e, a, b, c, KK4,  8,  6);
    tm_rmd160_r_f0(c, d, e, a, b, KK4, 13,  2);
    tm_rmd160_r_f0(b, c, d, e, a, KK4,  6, 13);
    tm_rmd160_r_f0(a, b, c, d, e, KK4,  5, 14);
    tm_rmd160_r_f0(e, a, b, c, d, KK4, 15,  0);
    tm_rmd160_r_f0(d, e, a, b, c, KK4, 13,  3);
    tm_rmd160_r_f0(c, d, e, a, b, KK4, 11,  9);
    tm_rmd160_r_f0(b, c, d, e, a, KK4, 11, 11); /* #79 */

    t =        state[1] + cc + d;
    state[1] = state[2] + dd + e;
    state[2] = state[3] + ee + a;
    state[3] = state[4] + aa + b;
    state[4] = state[0] + bb + c;
    state[0] = t;
}

#else /* ! TM_USE_RIPEMD */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_RIPEMD is not defined */
int tvRmd160Dummy = 0;

#endif /* TM_USE_RIPEMD*/
