/*
 *
 * Description: Testing the ESP algorithems-- des
 *
 * Filename: trdes.c
 * Author: Jin Zhang
 * Date Created: 08/01/2001
 * $Source: source/cryptlib/trdes.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:35:27JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*  $KAME: des_ecb.c,v 1.5 2000/11/06 13:58:08 itojun Exp $ */

/* crypto/des/ecb_enc.c */
/* Copyright (C) 1995-1996 Eric Young (eay@mincom.oz.au)
 * All rights reserved.
 *
 * This file is part of an SSL implementation written
 * by Eric Young (eay@mincom.oz.au).
 * The implementation was written so as to conform with Netscapes SSL
 * specification.  This library and applications are
 * FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the following conditions are aheared to.
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.  If this code is used in a product,
 * Eric Young should be given attribution as the author of the parts used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Eric Young (eay@mincom.oz.au)
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>

#if (defined(TM_USE_DES) || defined(TM_USE_3DES))
#include <trcrylib.h>

typedef tt8Bit ttDesCblock[8];
typedef struct des_ks_struct
{
    union    {
        ttDesCblock     cblock;
        /* make sure things are correct size on machines with
         * 8 byte longs 
         */
        tt32Bit         pad[2];
    } ks;
} ttDesKeySchedule[16];
typedef ttDesKeySchedule TM_FAR * ttDesKeySchedulePtr;


#define TM_DES_KEY_SIZE    (sizeof(ttDesCblock))


/* tm_des_ip and tm_des_fp
 *     * The problem is more of a geometric problem that random bit fiddling.
 *      0  1  2  3  4  5  6  7      62 54 46 38 30 22 14  6
 *     8  9 10 11 12 13 14 15      60 52 44 36 28 20 12  4
 *    16 17 18 19 20 21 22 23      58 50 42 34 26 18 10  2
 *    24 25 26 27 28 29 30 31  to  56 48 40 32 24 16  8  0
 *
 *    32 33 34 35 36 37 38 39      63 55 47 39 31 23 15  7
 *    40 41 42 43 44 45 46 47      61 53 45 37 29 21 13  5
 *    48 49 50 51 52 53 54 55      59 51 43 35 27 19 11  3
 *    56 57 58 59 60 61 62 63      57 49 41 33 25 17  9  1
 *
 *    The output has been subject to swaps of the form
 *    0 1 -> 3 1 but the odd and even bits have been put into
 *    2 3    2 0
 *    different words.  The main trick is to remember that
 *    t=((l>>size)^r)&(mask);
 *    r^=t;
 *    l^=(t<<size);
 *    can be used to swap and move bits between words.
 *
 *    So l =  0  1  2  3  r = 16 17 18 19
 *            4  5  6  7      20 21 22 23
 *            8  9 10 11      24 25 26 27
 *           12 13 14 15      28 29 30 31
 *    becomes (for size == 2 and mask == 0x3333)
 *  t =   2^16  3^17 -- --   l =  0  1 16 17  r =  2  3 18 19
 *         6^20  7^21 -- --        4  5 20 21       6  7 22 23
 *        10^24 11^25 -- --        8  9 24 25      10 11 24 25
 *        14^28 15^29 -- --       12 13 28 29      14 15 28 29
 *
 *    Thanks for hints from Richard Outerbridge - he told me tm_des_ip&tm_des_fp
 *    could be done in 15 xor, 10 shifts and 5 ands.
 *    When I finally started to think of the problem in 2D
 *    I first got ~42 operations without xors.  When I remembered
 *    how to use xors :-) I got it to its final state.
 */
#define tm_des_perm_op(a,b,t,n,m) \
    ((t)=((((a)>>(unsigned)(n))^(b))&((tt32Bit)m)),\
    (b)^=(t),\
    (a)^=((t)<<(unsigned)(n)))

#define tm_des_ip(l,r) \
    { \
        register tt32Bit tt; \
        tm_des_perm_op(r,l,tt, 4, TM_UL(0x0f0f0f0f)); \
        tm_des_perm_op(l,r,tt,16, TM_UL(0x0000ffff)); \
        tm_des_perm_op(r,l,tt, 2, TM_UL(0x33333333)); \
        tm_des_perm_op(l,r,tt, 8, TM_UL(0x00ff00ff)); \
        tm_des_perm_op(r,l,tt, 1, TM_UL(0x55555555)); \
    }

#define tm_des_fp(l,r) \
    { \
        register tt32Bit tt; \
        tm_des_perm_op(l,r,tt, 1, TM_UL(0x55555555)); \
        tm_des_perm_op(r,l,tt, 8, TM_UL(0x00ff00ff)); \
        tm_des_perm_op(l,r,tt, 2, TM_UL(0x33333333)); \
        tm_des_perm_op(r,l,tt,16, TM_UL(0x0000ffff)); \
        tm_des_perm_op(l,r,tt, 4, TM_UL(0x0f0f0f0f)); \
    }

#define tm_des_hperm_op(a,t,n,m) \
    ((t)=((((a)<<(unsigned)(16-(n)))^(a))&((tt32Bit)m)),\
    (a)=(a)^(t)^(t>>(unsigned)(16-(n))))

static void tfDesEncrypt (tt32BitPtr, tt32BitPtr schedule, int);


#ifdef TM_DES_CHECK_KEY_PARITY
static int tfDesCheckParity (tt8BitPtr);
#endif /* TM_DES_CHECK_KEY_PARITY */

/* Those with odd 1's have a right parity */
static const tt8Bit TM_CONST_QLF oddParity[256] = 
{
     1,  1,  2,  2,  4,  4,  7,  7,  8,  8, 11, 11, 13, 13, 14, 14,
     16, 16, 19, 19, 21, 21, 22, 22, 25, 25, 26, 26, 28, 28, 31, 31,
     32, 32, 35, 35, 37, 37, 38, 38, 41, 41, 42, 42, 44, 44, 47, 47,
     49, 49, 50, 50, 52, 52, 55, 55, 56, 56, 59, 59, 61, 61, 62, 62,
     64, 64, 67, 67, 69, 69, 70, 70, 73, 73, 74, 74, 76, 76, 79, 79,
     81, 81, 82, 82, 84, 84, 87, 87, 88, 88, 91, 91, 93, 93, 94, 94,
     97, 97, 98, 98,100,100,103,103,104,104,107,107,109,109,110,110,
    112,112,115,115,117,117,118,118,121,121,122,122,124,124,127,127,
    128,128,131,131,133,133,134,134,137,137,138,138,140,140,143,143,
    145,145,146,146,148,148,151,151,152,152,155,155,157,157,158,158,
    161,161,162,162,164,164,167,167,168,168,171,171,173,173,174,174,
    176,176,179,179,181,181,182,182,185,185,186,186,188,188,191,191,
    193,193,194,194,196,196,199,199,200,200,203,203,205,205,206,206,
    208,208,211,211,213,213,214,214,217,217,218,218,220,220,223,223,
    224,224,227,227,229,229,230,230,233,233,234,234,236,236,239,239,
    241,241,242,242,244,244,247,247,248,248,251,251,253,253,254,254
};

#ifdef TM_DES_CHECK_KEY_WEAK
static int tfDesIsWeakKey (tt8BitPtr key);

#define TM_DES_NUM_WEAK_KEY    16
static const ttDesCblock TM_CONST_QLF weakKeys[TM_DES_NUM_WEAK_KEY] = 
{
    /* weak keys */
    {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01},
    {0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE},
    {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F},
    {0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0},
    /* semi-weak keys */
    {0x01,0xFE,0x01,0xFE,0x01,0xFE,0x01,0xFE},
    {0xFE,0x01,0xFE,0x01,0xFE,0x01,0xFE,0x01},
    {0x1F,0xE0,0x1F,0xE0,0x0E,0xF1,0x0E,0xF1},
    {0xE0,0x1F,0xE0,0x1F,0xF1,0x0E,0xF1,0x0E},
    {0x01,0xE0,0x01,0xE0,0x01,0xF1,0x01,0xF1},
    {0xE0,0x01,0xE0,0x01,0xF1,0x01,0xF1,0x01},
    {0x1F,0xFE,0x1F,0xFE,0x0E,0xFE,0x0E,0xFE},
    {0xFE,0x1F,0xFE,0x1F,0xFE,0x0E,0xFE,0x0E},
    {0x01,0x1F,0x01,0x1F,0x01,0x0E,0x01,0x0E},
    {0x1F,0x01,0x1F,0x01,0x0E,0x01,0x0E,0x01},
    {0xE0,0xFE,0xE0,0xFE,0xF1,0xFE,0xF1,0xFE},
    {0xFE,0xE0,0xFE,0xE0,0xFE,0xF1,0xFE,0xF1}
};

#endif /* TM_DES_CHECK_KEY_WEAK */


static const tt32Bit TM_CONST_QLF desSkb[8][64] = {
{
/* for C bits (numbered as per FIPS 46) 1 2 3 4 5 6 */
    TM_UL(0x00000000),TM_UL(0x00000010),TM_UL(0x20000000),TM_UL(0x20000010),
    TM_UL(0x00010000),TM_UL(0x00010010),TM_UL(0x20010000),TM_UL(0x20010010),
    TM_UL(0x00000800),TM_UL(0x00000810),TM_UL(0x20000800),TM_UL(0x20000810),
    TM_UL(0x00010800),TM_UL(0x00010810),TM_UL(0x20010800),TM_UL(0x20010810),
    TM_UL(0x00000020),TM_UL(0x00000030),TM_UL(0x20000020),TM_UL(0x20000030),
    TM_UL(0x00010020),TM_UL(0x00010030),TM_UL(0x20010020),TM_UL(0x20010030),
    TM_UL(0x00000820),TM_UL(0x00000830),TM_UL(0x20000820),TM_UL(0x20000830),
    TM_UL(0x00010820),TM_UL(0x00010830),TM_UL(0x20010820),TM_UL(0x20010830),
    TM_UL(0x00080000),TM_UL(0x00080010),TM_UL(0x20080000),TM_UL(0x20080010),
    TM_UL(0x00090000),TM_UL(0x00090010),TM_UL(0x20090000),TM_UL(0x20090010),
    TM_UL(0x00080800),TM_UL(0x00080810),TM_UL(0x20080800),TM_UL(0x20080810),
    TM_UL(0x00090800),TM_UL(0x00090810),TM_UL(0x20090800),TM_UL(0x20090810),
    TM_UL(0x00080020),TM_UL(0x00080030),TM_UL(0x20080020),TM_UL(0x20080030),
    TM_UL(0x00090020),TM_UL(0x00090030),TM_UL(0x20090020),TM_UL(0x20090030),
    TM_UL(0x00080820),TM_UL(0x00080830),TM_UL(0x20080820),TM_UL(0x20080830),
    TM_UL(0x00090820),TM_UL(0x00090830),TM_UL(0x20090820),TM_UL(0x20090830)
},
{
/* for C bits (numbered as per FIPS 46) 7 8 10 11 12 13 */
    TM_UL(0x00000000),TM_UL(0x02000000),TM_UL(0x00002000),TM_UL(0x02002000),
    TM_UL(0x00200000),TM_UL(0x02200000),TM_UL(0x00202000),TM_UL(0x02202000),
    TM_UL(0x00000004),TM_UL(0x02000004),TM_UL(0x00002004),TM_UL(0x02002004),
    TM_UL(0x00200004),TM_UL(0x02200004),TM_UL(0x00202004),TM_UL(0x02202004),
    TM_UL(0x00000400),TM_UL(0x02000400),TM_UL(0x00002400),TM_UL(0x02002400),
    TM_UL(0x00200400),TM_UL(0x02200400),TM_UL(0x00202400),TM_UL(0x02202400),
    TM_UL(0x00000404),TM_UL(0x02000404),TM_UL(0x00002404),TM_UL(0x02002404),
    TM_UL(0x00200404),TM_UL(0x02200404),TM_UL(0x00202404),TM_UL(0x02202404),
    TM_UL(0x10000000),TM_UL(0x12000000),TM_UL(0x10002000),TM_UL(0x12002000),
    TM_UL(0x10200000),TM_UL(0x12200000),TM_UL(0x10202000),TM_UL(0x12202000),
    TM_UL(0x10000004),TM_UL(0x12000004),TM_UL(0x10002004),TM_UL(0x12002004),
    TM_UL(0x10200004),TM_UL(0x12200004),TM_UL(0x10202004),TM_UL(0x12202004),
    TM_UL(0x10000400),TM_UL(0x12000400),TM_UL(0x10002400),TM_UL(0x12002400),
    TM_UL(0x10200400),TM_UL(0x12200400),TM_UL(0x10202400),TM_UL(0x12202400),
    TM_UL(0x10000404),TM_UL(0x12000404),TM_UL(0x10002404),TM_UL(0x12002404),
    TM_UL(0x10200404),TM_UL(0x12200404),TM_UL(0x10202404),TM_UL(0x12202404)
},
{
/* for C bits (numbered as per FIPS 46) 14 15 16 17 19 20 */
    TM_UL(0x00000000),TM_UL(0x00000001),TM_UL(0x00040000),TM_UL(0x00040001),
    TM_UL(0x01000000),TM_UL(0x01000001),TM_UL(0x01040000),TM_UL(0x01040001),
    TM_UL(0x00000002),TM_UL(0x00000003),TM_UL(0x00040002),TM_UL(0x00040003),
    TM_UL(0x01000002),TM_UL(0x01000003),TM_UL(0x01040002),TM_UL(0x01040003),
    TM_UL(0x00000200),TM_UL(0x00000201),TM_UL(0x00040200),TM_UL(0x00040201),
    TM_UL(0x01000200),TM_UL(0x01000201),TM_UL(0x01040200),TM_UL(0x01040201),
    TM_UL(0x00000202),TM_UL(0x00000203),TM_UL(0x00040202),TM_UL(0x00040203),
    TM_UL(0x01000202),TM_UL(0x01000203),TM_UL(0x01040202),TM_UL(0x01040203),
    TM_UL(0x08000000),TM_UL(0x08000001),TM_UL(0x08040000),TM_UL(0x08040001),
    TM_UL(0x09000000),TM_UL(0x09000001),TM_UL(0x09040000),TM_UL(0x09040001),
    TM_UL(0x08000002),TM_UL(0x08000003),TM_UL(0x08040002),TM_UL(0x08040003),
    TM_UL(0x09000002),TM_UL(0x09000003),TM_UL(0x09040002),TM_UL(0x09040003),
    TM_UL(0x08000200),TM_UL(0x08000201),TM_UL(0x08040200),TM_UL(0x08040201),
    TM_UL(0x09000200),TM_UL(0x09000201),TM_UL(0x09040200),TM_UL(0x09040201),
    TM_UL(0x08000202),TM_UL(0x08000203),TM_UL(0x08040202),TM_UL(0x08040203),
    TM_UL(0x09000202),TM_UL(0x09000203),TM_UL(0x09040202),TM_UL(0x09040203)
},
{
/* for C bits (numbered as per FIPS 46) 21 23 24 26 27 28 */
    TM_UL(0x00000000),TM_UL(0x00100000),TM_UL(0x00000100),TM_UL(0x00100100),
    TM_UL(0x00000008),TM_UL(0x00100008),TM_UL(0x00000108),TM_UL(0x00100108),
    TM_UL(0x00001000),TM_UL(0x00101000),TM_UL(0x00001100),TM_UL(0x00101100),
    TM_UL(0x00001008),TM_UL(0x00101008),TM_UL(0x00001108),TM_UL(0x00101108),
    TM_UL(0x04000000),TM_UL(0x04100000),TM_UL(0x04000100),TM_UL(0x04100100),
    TM_UL(0x04000008),TM_UL(0x04100008),TM_UL(0x04000108),TM_UL(0x04100108),
    TM_UL(0x04001000),TM_UL(0x04101000),TM_UL(0x04001100),TM_UL(0x04101100),
    TM_UL(0x04001008),TM_UL(0x04101008),TM_UL(0x04001108),TM_UL(0x04101108),
    TM_UL(0x00020000),TM_UL(0x00120000),TM_UL(0x00020100),TM_UL(0x00120100),
    TM_UL(0x00020008),TM_UL(0x00120008),TM_UL(0x00020108),TM_UL(0x00120108),
    TM_UL(0x00021000),TM_UL(0x00121000),TM_UL(0x00021100),TM_UL(0x00121100),
    TM_UL(0x00021008),TM_UL(0x00121008),TM_UL(0x00021108),TM_UL(0x00121108),
    TM_UL(0x04020000),TM_UL(0x04120000),TM_UL(0x04020100),TM_UL(0x04120100),
    TM_UL(0x04020008),TM_UL(0x04120008),TM_UL(0x04020108),TM_UL(0x04120108),
    TM_UL(0x04021000),TM_UL(0x04121000),TM_UL(0x04021100),TM_UL(0x04121100),
    TM_UL(0x04021008),TM_UL(0x04121008),TM_UL(0x04021108),TM_UL(0x04121108)
},
{
/* for D bits (numbered as per FIPS 46) 1 2 3 4 5 6 */
    TM_UL(0x00000000),TM_UL(0x10000000),TM_UL(0x00010000),TM_UL(0x10010000),
    TM_UL(0x00000004),TM_UL(0x10000004),TM_UL(0x00010004),TM_UL(0x10010004),
    TM_UL(0x20000000),TM_UL(0x30000000),TM_UL(0x20010000),TM_UL(0x30010000),
    TM_UL(0x20000004),TM_UL(0x30000004),TM_UL(0x20010004),TM_UL(0x30010004),
    TM_UL(0x00100000),TM_UL(0x10100000),TM_UL(0x00110000),TM_UL(0x10110000),
    TM_UL(0x00100004),TM_UL(0x10100004),TM_UL(0x00110004),TM_UL(0x10110004),
    TM_UL(0x20100000),TM_UL(0x30100000),TM_UL(0x20110000),TM_UL(0x30110000),
    TM_UL(0x20100004),TM_UL(0x30100004),TM_UL(0x20110004),TM_UL(0x30110004),
    TM_UL(0x00001000),TM_UL(0x10001000),TM_UL(0x00011000),TM_UL(0x10011000),
    TM_UL(0x00001004),TM_UL(0x10001004),TM_UL(0x00011004),TM_UL(0x10011004),
    TM_UL(0x20001000),TM_UL(0x30001000),TM_UL(0x20011000),TM_UL(0x30011000),
    TM_UL(0x20001004),TM_UL(0x30001004),TM_UL(0x20011004),TM_UL(0x30011004),
    TM_UL(0x00101000),TM_UL(0x10101000),TM_UL(0x00111000),TM_UL(0x10111000),
    TM_UL(0x00101004),TM_UL(0x10101004),TM_UL(0x00111004),TM_UL(0x10111004),
    TM_UL(0x20101000),TM_UL(0x30101000),TM_UL(0x20111000),TM_UL(0x30111000),
    TM_UL(0x20101004),TM_UL(0x30101004),TM_UL(0x20111004),TM_UL(0x30111004)
},
{
/* for D bits (numbered as per FIPS 46) 8 9 11 12 13 14 */
    TM_UL(0x00000000),TM_UL(0x08000000),TM_UL(0x00000008),TM_UL(0x08000008),
    TM_UL(0x00000400),TM_UL(0x08000400),TM_UL(0x00000408),TM_UL(0x08000408),
    TM_UL(0x00020000),TM_UL(0x08020000),TM_UL(0x00020008),TM_UL(0x08020008),
    TM_UL(0x00020400),TM_UL(0x08020400),TM_UL(0x00020408),TM_UL(0x08020408),
    TM_UL(0x00000001),TM_UL(0x08000001),TM_UL(0x00000009),TM_UL(0x08000009),
    TM_UL(0x00000401),TM_UL(0x08000401),TM_UL(0x00000409),TM_UL(0x08000409),
    TM_UL(0x00020001),TM_UL(0x08020001),TM_UL(0x00020009),TM_UL(0x08020009),
    TM_UL(0x00020401),TM_UL(0x08020401),TM_UL(0x00020409),TM_UL(0x08020409),
    TM_UL(0x02000000),TM_UL(0x0A000000),TM_UL(0x02000008),TM_UL(0x0A000008),
    TM_UL(0x02000400),TM_UL(0x0A000400),TM_UL(0x02000408),TM_UL(0x0A000408),
    TM_UL(0x02020000),TM_UL(0x0A020000),TM_UL(0x02020008),TM_UL(0x0A020008),
    TM_UL(0x02020400),TM_UL(0x0A020400),TM_UL(0x02020408),TM_UL(0x0A020408),
    TM_UL(0x02000001),TM_UL(0x0A000001),TM_UL(0x02000009),TM_UL(0x0A000009),
    TM_UL(0x02000401),TM_UL(0x0A000401),TM_UL(0x02000409),TM_UL(0x0A000409),
    TM_UL(0x02020001),TM_UL(0x0A020001),TM_UL(0x02020009),TM_UL(0x0A020009),
    TM_UL(0x02020401),TM_UL(0x0A020401),TM_UL(0x02020409),TM_UL(0x0A020409)
},
{
/* for D bits (numbered as per FIPS 46) 16 17 18 19 20 21 */
    TM_UL(0x00000000),TM_UL(0x00000100),TM_UL(0x00080000),TM_UL(0x00080100),
    TM_UL(0x01000000),TM_UL(0x01000100),TM_UL(0x01080000),TM_UL(0x01080100),
    TM_UL(0x00000010),TM_UL(0x00000110),TM_UL(0x00080010),TM_UL(0x00080110),
    TM_UL(0x01000010),TM_UL(0x01000110),TM_UL(0x01080010),TM_UL(0x01080110),
    TM_UL(0x00200000),TM_UL(0x00200100),TM_UL(0x00280000),TM_UL(0x00280100),
    TM_UL(0x01200000),TM_UL(0x01200100),TM_UL(0x01280000),TM_UL(0x01280100),
    TM_UL(0x00200010),TM_UL(0x00200110),TM_UL(0x00280010),TM_UL(0x00280110),
    TM_UL(0x01200010),TM_UL(0x01200110),TM_UL(0x01280010),TM_UL(0x01280110),
    TM_UL(0x00000200),TM_UL(0x00000300),TM_UL(0x00080200),TM_UL(0x00080300),
    TM_UL(0x01000200),TM_UL(0x01000300),TM_UL(0x01080200),TM_UL(0x01080300),
    TM_UL(0x00000210),TM_UL(0x00000310),TM_UL(0x00080210),TM_UL(0x00080310),
    TM_UL(0x01000210),TM_UL(0x01000310),TM_UL(0x01080210),TM_UL(0x01080310),
    TM_UL(0x00200200),TM_UL(0x00200300),TM_UL(0x00280200),TM_UL(0x00280300),
    TM_UL(0x01200200),TM_UL(0x01200300),TM_UL(0x01280200),TM_UL(0x01280300),
    TM_UL(0x00200210),TM_UL(0x00200310),TM_UL(0x00280210),TM_UL(0x00280310),
    TM_UL(0x01200210),TM_UL(0x01200310),TM_UL(0x01280210),TM_UL(0x01280310)
},
{
/* for D bits (numbered as per FIPS 46) 22 23 24 25 27 28 */
    TM_UL(0x00000000),TM_UL(0x04000000),TM_UL(0x00040000),TM_UL(0x04040000),
    TM_UL(0x00000002),TM_UL(0x04000002),TM_UL(0x00040002),TM_UL(0x04040002),
    TM_UL(0x00002000),TM_UL(0x04002000),TM_UL(0x00042000),TM_UL(0x04042000),
    TM_UL(0x00002002),TM_UL(0x04002002),TM_UL(0x00042002),TM_UL(0x04042002),
    TM_UL(0x00000020),TM_UL(0x04000020),TM_UL(0x00040020),TM_UL(0x04040020),
    TM_UL(0x00000022),TM_UL(0x04000022),TM_UL(0x00040022),TM_UL(0x04040022),
    TM_UL(0x00002020),TM_UL(0x04002020),TM_UL(0x00042020),TM_UL(0x04042020),
    TM_UL(0x00002022),TM_UL(0x04002022),TM_UL(0x00042022),TM_UL(0x04042022),
    TM_UL(0x00000800),TM_UL(0x04000800),TM_UL(0x00040800),TM_UL(0x04040800),
    TM_UL(0x00000802),TM_UL(0x04000802),TM_UL(0x00040802),TM_UL(0x04040802),
    TM_UL(0x00002800),TM_UL(0x04002800),TM_UL(0x00042800),TM_UL(0x04042800),
    TM_UL(0x00002802),TM_UL(0x04002802),TM_UL(0x00042802),TM_UL(0x04042802),
    TM_UL(0x00000820),TM_UL(0x04000820),TM_UL(0x00040820),TM_UL(0x04040820),
    TM_UL(0x00000822),TM_UL(0x04000822),TM_UL(0x00040822),TM_UL(0x04040822),
    TM_UL(0x00002820),TM_UL(0x04002820),TM_UL(0x00042820),TM_UL(0x04042820),
    TM_UL(0x00002822),TM_UL(0x04002822),TM_UL(0x00042822),TM_UL(0x04042822)
}
};
                    
#ifdef TM_DES_CHECK_KEY_WEAK
static int tfDesIsWeakKey(tt8BitPtr key)
{
    int i;

    for (i=0; i<TM_DES_NUM_WEAK_KEY; i++)
    {
        /* Added == 0 to comparision, I obviously don't run
         * this section very often :-(, thanks to
         * engineering@MorningStar.Com for the fix
         * eay 93/06/29 */
        if (tm_memcmp(weakKeys[i],key,sizeof(ttDesCblock)) == 0) 
            return(1);
    }

    return(0);
}
#endif /* TM_DES_CHECK_KEY_WEAK */


#define TM_DES_ITERATIONS 16

/* used in des_read and des_write */
/*#define MAXWRITE    (1024*16)
#define BSIZE        (MAXWRITE+4)
*/
#define tm_des_c2l(c,l)    (l =((tt32Bit)(*((c)++)))    ,  \
                            l|=((tt32Bit)(*((c)++)))<<(unsigned) 8, \
                            l|=((tt32Bit)(*((c)++)))<<(unsigned)16, \
                            l|=((tt32Bit)(*((c)++)))<<(unsigned)24)

#define tm_des_l2c(l,c) (*((c)++)=tm_8bit((l)), \
                         *((c)++)=tm_8bit((l)>>(unsigned) 8), \
                         *((c)++)=tm_8bit((l)>>(unsigned)16), \
                         *((c)++)=tm_8bit((l)>>(unsigned)24))

#define    tm_des_rotate(a,n)   \
       (((a)>>(unsigned)(n))+((a)<<(unsigned)(32-(n))))


static const tt32Bit TM_CONST_QLF desSPtrans[8][64] = {
{
/* nibble 0 */
    TM_UL(0x00820200), TM_UL(0x00020000), TM_UL(0x80800000), TM_UL(0x80820200),
    TM_UL(0x00800000), TM_UL(0x80020200), TM_UL(0x80020000), TM_UL(0x80800000),
    TM_UL(0x80020200), TM_UL(0x00820200), TM_UL(0x00820000), TM_UL(0x80000200),
    TM_UL(0x80800200), TM_UL(0x00800000), TM_UL(0x00000000), TM_UL(0x80020000),
    TM_UL(0x00020000), TM_UL(0x80000000), TM_UL(0x00800200), TM_UL(0x00020200),
    TM_UL(0x80820200), TM_UL(0x00820000), TM_UL(0x80000200), TM_UL(0x00800200),
    TM_UL(0x80000000), TM_UL(0x00000200), TM_UL(0x00020200), TM_UL(0x80820000),
    TM_UL(0x00000200), TM_UL(0x80800200), TM_UL(0x80820000), TM_UL(0x00000000),
    TM_UL(0x00000000), TM_UL(0x80820200), TM_UL(0x00800200), TM_UL(0x80020000),
    TM_UL(0x00820200), TM_UL(0x00020000), TM_UL(0x80000200), TM_UL(0x00800200),
    TM_UL(0x80820000), TM_UL(0x00000200), TM_UL(0x00020200), TM_UL(0x80800000),
    TM_UL(0x80020200), TM_UL(0x80000000), TM_UL(0x80800000), TM_UL(0x00820000),
    TM_UL(0x80820200), TM_UL(0x00020200), TM_UL(0x00820000), TM_UL(0x80800200),
    TM_UL(0x00800000), TM_UL(0x80000200), TM_UL(0x80020000), TM_UL(0x00000000),
    TM_UL(0x00020000), TM_UL(0x00800000), TM_UL(0x80800200), TM_UL(0x00820200),
    TM_UL(0x80000000), TM_UL(0x80820000), TM_UL(0x00000200), TM_UL(0x80020200)
    },{
/* nibble 1 */
    TM_UL(0x10042004), TM_UL(0x00000000), TM_UL(0x00042000), TM_UL(0x10040000),
    TM_UL(0x10000004), TM_UL(0x00002004), TM_UL(0x10002000), TM_UL(0x00042000),
    TM_UL(0x00002000), TM_UL(0x10040004), TM_UL(0x00000004), TM_UL(0x10002000),
    TM_UL(0x00040004), TM_UL(0x10042000), TM_UL(0x10040000), TM_UL(0x00000004),
    TM_UL(0x00040000), TM_UL(0x10002004), TM_UL(0x10040004), TM_UL(0x00002000),
    TM_UL(0x00042004), TM_UL(0x10000000), TM_UL(0x00000000), TM_UL(0x00040004),
    TM_UL(0x10002004), TM_UL(0x00042004), TM_UL(0x10042000), TM_UL(0x10000004),
    TM_UL(0x10000000), TM_UL(0x00040000), TM_UL(0x00002004), TM_UL(0x10042004),
    TM_UL(0x00040004), TM_UL(0x10042000), TM_UL(0x10002000), TM_UL(0x00042004),
    TM_UL(0x10042004), TM_UL(0x00040004), TM_UL(0x10000004), TM_UL(0x00000000),
    TM_UL(0x10000000), TM_UL(0x00002004), TM_UL(0x00040000), TM_UL(0x10040004),
    TM_UL(0x00002000), TM_UL(0x10000000), TM_UL(0x00042004), TM_UL(0x10002004),
    TM_UL(0x10042000), TM_UL(0x00002000), TM_UL(0x00000000), TM_UL(0x10000004),
    TM_UL(0x00000004), TM_UL(0x10042004), TM_UL(0x00042000), TM_UL(0x10040000),
    TM_UL(0x10040004), TM_UL(0x00040000), TM_UL(0x00002004), TM_UL(0x10002000),
    TM_UL(0x10002004), TM_UL(0x00000004), TM_UL(0x10040000), TM_UL(0x00042000)
    },{
/* nibble 2 */
    TM_UL(0x41000000), TM_UL(0x01010040), TM_UL(0x00000040), TM_UL(0x41000040),
    TM_UL(0x40010000), TM_UL(0x01000000), TM_UL(0x41000040), TM_UL(0x00010040),
    TM_UL(0x01000040), TM_UL(0x00010000), TM_UL(0x01010000), TM_UL(0x40000000),
    TM_UL(0x41010040), TM_UL(0x40000040), TM_UL(0x40000000), TM_UL(0x41010000),
    TM_UL(0x00000000), TM_UL(0x40010000), TM_UL(0x01010040), TM_UL(0x00000040),
    TM_UL(0x40000040), TM_UL(0x41010040), TM_UL(0x00010000), TM_UL(0x41000000),
    TM_UL(0x41010000), TM_UL(0x01000040), TM_UL(0x40010040), TM_UL(0x01010000),
    TM_UL(0x00010040), TM_UL(0x00000000), TM_UL(0x01000000), TM_UL(0x40010040),
    TM_UL(0x01010040), TM_UL(0x00000040), TM_UL(0x40000000), TM_UL(0x00010000),
    TM_UL(0x40000040), TM_UL(0x40010000), TM_UL(0x01010000), TM_UL(0x41000040),
    TM_UL(0x00000000), TM_UL(0x01010040), TM_UL(0x00010040), TM_UL(0x41010000),
    TM_UL(0x40010000), TM_UL(0x01000000), TM_UL(0x41010040), TM_UL(0x40000000),
    TM_UL(0x40010040), TM_UL(0x41000000), TM_UL(0x01000000), TM_UL(0x41010040),
    TM_UL(0x00010000), TM_UL(0x01000040), TM_UL(0x41000040), TM_UL(0x00010040),
    TM_UL(0x01000040), TM_UL(0x00000000), TM_UL(0x41010000), TM_UL(0x40000040),
    TM_UL(0x41000000), TM_UL(0x40010040), TM_UL(0x00000040), TM_UL(0x01010000)
    },{
/* nibble 3 */
    TM_UL(0x00100402), TM_UL(0x04000400), TM_UL(0x00000002), TM_UL(0x04100402),
    TM_UL(0x00000000), TM_UL(0x04100000), TM_UL(0x04000402), TM_UL(0x00100002),
    TM_UL(0x04100400), TM_UL(0x04000002), TM_UL(0x04000000), TM_UL(0x00000402),
    TM_UL(0x04000002), TM_UL(0x00100402), TM_UL(0x00100000), TM_UL(0x04000000),
    TM_UL(0x04100002), TM_UL(0x00100400), TM_UL(0x00000400), TM_UL(0x00000002),
    TM_UL(0x00100400), TM_UL(0x04000402), TM_UL(0x04100000), TM_UL(0x00000400),
    TM_UL(0x00000402), TM_UL(0x00000000), TM_UL(0x00100002), TM_UL(0x04100400),
    TM_UL(0x04000400), TM_UL(0x04100002), TM_UL(0x04100402), TM_UL(0x00100000),
    TM_UL(0x04100002), TM_UL(0x00000402), TM_UL(0x00100000), TM_UL(0x04000002),
    TM_UL(0x00100400), TM_UL(0x04000400), TM_UL(0x00000002), TM_UL(0x04100000),
    TM_UL(0x04000402), TM_UL(0x00000000), TM_UL(0x00000400), TM_UL(0x00100002),
    TM_UL(0x00000000), TM_UL(0x04100002), TM_UL(0x04100400), TM_UL(0x00000400),
    TM_UL(0x04000000), TM_UL(0x04100402), TM_UL(0x00100402), TM_UL(0x00100000),
    TM_UL(0x04100402), TM_UL(0x00000002), TM_UL(0x04000400), TM_UL(0x00100402),
    TM_UL(0x00100002), TM_UL(0x00100400), TM_UL(0x04100000), TM_UL(0x04000402),
    TM_UL(0x00000402), TM_UL(0x04000000), TM_UL(0x04000002), TM_UL(0x04100400)
    },{
/* nibble 4 */
    TM_UL(0x02000000), TM_UL(0x00004000), TM_UL(0x00000100), TM_UL(0x02004108),
    TM_UL(0x02004008), TM_UL(0x02000100), TM_UL(0x00004108), TM_UL(0x02004000),
    TM_UL(0x00004000), TM_UL(0x00000008), TM_UL(0x02000008), TM_UL(0x00004100),
    TM_UL(0x02000108), TM_UL(0x02004008), TM_UL(0x02004100), TM_UL(0x00000000),
    TM_UL(0x00004100), TM_UL(0x02000000), TM_UL(0x00004008), TM_UL(0x00000108),
    TM_UL(0x02000100), TM_UL(0x00004108), TM_UL(0x00000000), TM_UL(0x02000008),
    TM_UL(0x00000008), TM_UL(0x02000108), TM_UL(0x02004108), TM_UL(0x00004008),
    TM_UL(0x02004000), TM_UL(0x00000100), TM_UL(0x00000108), TM_UL(0x02004100),
    TM_UL(0x02004100), TM_UL(0x02000108), TM_UL(0x00004008), TM_UL(0x02004000),
    TM_UL(0x00004000), TM_UL(0x00000008), TM_UL(0x02000008), TM_UL(0x02000100),
    TM_UL(0x02000000), TM_UL(0x00004100), TM_UL(0x02004108), TM_UL(0x00000000),
    TM_UL(0x00004108), TM_UL(0x02000000), TM_UL(0x00000100), TM_UL(0x00004008),
    TM_UL(0x02000108), TM_UL(0x00000100), TM_UL(0x00000000), TM_UL(0x02004108),
    TM_UL(0x02004008), TM_UL(0x02004100), TM_UL(0x00000108), TM_UL(0x00004000),
    TM_UL(0x00004100), TM_UL(0x02004008), TM_UL(0x02000100), TM_UL(0x00000108),
    TM_UL(0x00000008), TM_UL(0x00004108), TM_UL(0x02004000), TM_UL(0x02000008)
    },{
/* nibble 5 */
    TM_UL(0x20000010), TM_UL(0x00080010), TM_UL(0x00000000), TM_UL(0x20080800),
    TM_UL(0x00080010), TM_UL(0x00000800), TM_UL(0x20000810), TM_UL(0x00080000),
    TM_UL(0x00000810), TM_UL(0x20080810), TM_UL(0x00080800), TM_UL(0x20000000),
    TM_UL(0x20000800), TM_UL(0x20000010), TM_UL(0x20080000), TM_UL(0x00080810),
    TM_UL(0x00080000), TM_UL(0x20000810), TM_UL(0x20080010), TM_UL(0x00000000),
    TM_UL(0x00000800), TM_UL(0x00000010), TM_UL(0x20080800), TM_UL(0x20080010),
    TM_UL(0x20080810), TM_UL(0x20080000), TM_UL(0x20000000), TM_UL(0x00000810),
    TM_UL(0x00000010), TM_UL(0x00080800), TM_UL(0x00080810), TM_UL(0x20000800),
    TM_UL(0x00000810), TM_UL(0x20000000), TM_UL(0x20000800), TM_UL(0x00080810),
    TM_UL(0x20080800), TM_UL(0x00080010), TM_UL(0x00000000), TM_UL(0x20000800),
    TM_UL(0x20000000), TM_UL(0x00000800), TM_UL(0x20080010), TM_UL(0x00080000),
    TM_UL(0x00080010), TM_UL(0x20080810), TM_UL(0x00080800), TM_UL(0x00000010),
    TM_UL(0x20080810), TM_UL(0x00080800), TM_UL(0x00080000), TM_UL(0x20000810),
    TM_UL(0x20000010), TM_UL(0x20080000), TM_UL(0x00080810), TM_UL(0x00000000),
    TM_UL(0x00000800), TM_UL(0x20000010), TM_UL(0x20000810), TM_UL(0x20080800),
    TM_UL(0x20080000), TM_UL(0x00000810), TM_UL(0x00000010), TM_UL(0x20080010)
    },{
/* nibble 6 */
    TM_UL(0x00001000), TM_UL(0x00000080), TM_UL(0x00400080), TM_UL(0x00400001),
    TM_UL(0x00401081), TM_UL(0x00001001), TM_UL(0x00001080), TM_UL(0x00000000),
    TM_UL(0x00400000), TM_UL(0x00400081), TM_UL(0x00000081), TM_UL(0x00401000),
    TM_UL(0x00000001), TM_UL(0x00401080), TM_UL(0x00401000), TM_UL(0x00000081),
    TM_UL(0x00400081), TM_UL(0x00001000), TM_UL(0x00001001), TM_UL(0x00401081),
    TM_UL(0x00000000), TM_UL(0x00400080), TM_UL(0x00400001), TM_UL(0x00001080),
    TM_UL(0x00401001), TM_UL(0x00001081), TM_UL(0x00401080), TM_UL(0x00000001),
    TM_UL(0x00001081), TM_UL(0x00401001), TM_UL(0x00000080), TM_UL(0x00400000),
    TM_UL(0x00001081), TM_UL(0x00401000), TM_UL(0x00401001), TM_UL(0x00000081),
    TM_UL(0x00001000), TM_UL(0x00000080), TM_UL(0x00400000), TM_UL(0x00401001),
    TM_UL(0x00400081), TM_UL(0x00001081), TM_UL(0x00001080), TM_UL(0x00000000),
    TM_UL(0x00000080), TM_UL(0x00400001), TM_UL(0x00000001), TM_UL(0x00400080),
    TM_UL(0x00000000), TM_UL(0x00400081), TM_UL(0x00400080), TM_UL(0x00001080),
    TM_UL(0x00000081), TM_UL(0x00001000), TM_UL(0x00401081), TM_UL(0x00400000),
    TM_UL(0x00401080), TM_UL(0x00000001), TM_UL(0x00001001), TM_UL(0x00401081),
    TM_UL(0x00400001), TM_UL(0x00401080), TM_UL(0x00401000), TM_UL(0x00001001)
    },{
/* nibble 7 */
    TM_UL(0x08200020), TM_UL(0x08208000), TM_UL(0x00008020), TM_UL(0x00000000),
    TM_UL(0x08008000), TM_UL(0x00200020), TM_UL(0x08200000), TM_UL(0x08208020),
    TM_UL(0x00000020), TM_UL(0x08000000), TM_UL(0x00208000), TM_UL(0x00008020),
    TM_UL(0x00208020), TM_UL(0x08008020), TM_UL(0x08000020), TM_UL(0x08200000),
    TM_UL(0x00008000), TM_UL(0x00208020), TM_UL(0x00200020), TM_UL(0x08008000),
    TM_UL(0x08208020), TM_UL(0x08000020), TM_UL(0x00000000), TM_UL(0x00208000),
    TM_UL(0x08000000), TM_UL(0x00200000), TM_UL(0x08008020), TM_UL(0x08200020),
    TM_UL(0x00200000), TM_UL(0x00008000), TM_UL(0x08208000), TM_UL(0x00000020),
    TM_UL(0x00200000), TM_UL(0x00008000), TM_UL(0x08000020), TM_UL(0x08208020),
    TM_UL(0x00008020), TM_UL(0x08000000), TM_UL(0x00000000), TM_UL(0x00208000),
    TM_UL(0x08200020), TM_UL(0x08008020), TM_UL(0x08008000), TM_UL(0x00200020),
    TM_UL(0x08208000), TM_UL(0x00000020), TM_UL(0x00200020), TM_UL(0x08008000),
    TM_UL(0x08208020), TM_UL(0x00200000), TM_UL(0x08200000), TM_UL(0x08000020),
    TM_UL(0x00208000), TM_UL(0x00008020), TM_UL(0x08008020), TM_UL(0x08200000),
    TM_UL(0x00000020), TM_UL(0x08208000), TM_UL(0x00208020), TM_UL(0x00000000),
    TM_UL(0x08000000), TM_UL(0x08200020), TM_UL(0x00008000), TM_UL(0x00208020)
}
};

#define tm_des_encrypt(Q,R,S) {\
    u = (R^s[S  ]); \
    t = R^s[S+1]; \
    t = tm_des_rotate(t,4); \
    Q ^=desSPtrans[1][(tt32Bit)((t     )&0x3f)]| \
        desSPtrans[3][(tt32Bit)((t>>(unsigned) 8)&0x3f)]| \
        desSPtrans[5][(tt32Bit)((t>>(unsigned)16)&0x3f)]| \
        desSPtrans[7][(tt32Bit)((t>>(unsigned)24)&0x3f)]| \
        desSPtrans[0][(tt32Bit)((u     )&0x3f)]| \
        desSPtrans[2][(tt32Bit)((u>>(unsigned) 8)&0x3f)]| \
        desSPtrans[4][(tt32Bit)((u>>(unsigned)16)&0x3f)]| \
        desSPtrans[6][(tt32Bit)((u>>(unsigned)24)&0x3f)]; }

#ifdef TM_USE_DES
/* return 0 if key parity is odd (correct),
 * return -1 if key parity error,
 * return -2 if illegal weak key.
 */
int tfDesSetKey(tt8BitPtr key, tt32BitPtr schedule)
{
    static const int shifts2[16]={0,0,1,1,1,1,1,1,0,1,1,1,1,1,1,0};
    register tt32Bit c,d,t,s;
    register tt8BitPtr in;
    register tt32BitPtr k;
    register int i;
    int      errorCode;

#ifdef TM_DES_CHECK_KEY_PARITY
    if (tfDesCheckParity(key) == 0) /*1 is ok*/
    {
        errorCode = TM_EKEYPARITY;
        goto SETKEYRETURN;
    }
#endif /*TM_DES_CHECK_KEY_PARITY*/

#ifdef TM_DES_CHECK_KEY_WEAK
     if (tfDesIsWeakKey(key) == 1) /*0 is good*/
     {
        errorCode = TM_EWEEKKEY;
        goto SETKEYRETURN;
     }
#endif /* TM_DES_CHECK_KEY_WEAK */

    k=(tt32BitPtr)schedule;
    in=key;

    tm_des_c2l(in,c);
    tm_des_c2l(in,d);

    tm_des_perm_op (d,c,t,4, TM_UL(0x0f0f0f0f));
    tm_des_hperm_op(c,t,-2, TM_UL(0xcccc0000));
    tm_des_hperm_op(d,t,-2, TM_UL(0xcccc0000));
    tm_des_perm_op (d,c,t,1, TM_UL(0x55555555));
    tm_des_perm_op (c,d,t,8, TM_UL(0x00ff00ff));
    tm_des_perm_op (d,c,t,1, TM_UL(0x55555555));
    d = (tt32Bit)(((tt32Bit)(d & TM_UL(0x000000ff))<<(unsigned)16)
                  | (d & TM_UL(0x0000ff00))
                  | ((tt32Bit)(d & TM_UL(0x00ff0000))>>(unsigned)16)
                  | ((tt32Bit)(c & TM_UL(0xf0000000))>>(unsigned)4));

    c &= TM_UL(0x0fffffff);

    for (i=0; i<TM_DES_ITERATIONS; i++)
    {
        if (shifts2[i] != 0)
        { 
            c=(tt32Bit)((c>>(unsigned)2)|(c<<(unsigned)26));
            d=(tt32Bit)((d>>(unsigned)2)|(d<<(unsigned)26)); 
        }
        else
        { 
            c=(tt32Bit)((c>>(unsigned)1)|(c<<(unsigned)27)); 
            d=(tt32Bit)((d>>(unsigned)1)|(d<<(unsigned)27)); 
        }
        c &= TM_UL(0x0fffffff);
        d &= TM_UL(0x0fffffff);
        /* could be a few less shifts but I am to lazy at this
         * point in time to investigate */
        s=  (tt32Bit)(desSkb[0][(tt32Bit)( (c    )&0x3f)]|
            desSkb[1][(tt32Bit)(((c>>(unsigned) 6)&0x03)|
            ((c>>(unsigned) 7)&0x3c))]|
            desSkb[2][(tt32Bit)(((c>>(unsigned)13)&0x0f)|
            ((c>>(unsigned)14)&0x30))]|
            desSkb[3][(tt32Bit)(((c>>(unsigned)20)&0x01)|
            ((c>>(unsigned)21)&0x06) |
            ((c>>(unsigned)22)&0x38))]);
        t=  (tt32Bit)(desSkb[4][(tt32Bit)( (d    )&0x3f)]|
            desSkb[5][(tt32Bit)(((d>>(unsigned) 7)&0x03)|
            ((d>>(unsigned) 8)&0x3c))]|
            desSkb[6][(tt32Bit)((d>>(unsigned)15)&0x3f  )]|
            desSkb[7][(tt32Bit)(((d>>(unsigned)21)&0x0f)|
            ((d>>(unsigned)22)&0x30))]);

        /* table contained 0213 4657 */
        *(k++)= (tt32Bit) ((t<<(unsigned)16)|(s & TM_UL(0x0000ffff)));
        s = (tt32Bit)((s>>(unsigned)16)|(t & TM_UL(0xffff0000)));
        
        s = (tt32Bit)((s<<(unsigned)4)|(s>>(unsigned)28));
        *(k++) = s;
    }
    errorCode = TM_ENOERROR;

#if (defined(TM_DES_CHECK_KEY_PARITY) || \
       defined(TM_DES_CHECK_KEY_WEAK))
SETKEYRETURN:
#endif /* TM_DES_CHECK_KEY_PARITY) || TM_DES_CHECK_KEY_WEAK */

    return errorCode;
}
#endif /* TM_USE_DES */

#ifdef TM_DES_CHECK_KEY_PARITY
static int tfDesCheckParity(tt8BitPtr key)
{
    int i;

    for (i=0; i<(int)TM_DES_KEY_SIZE; i++)
    {
        if (*(key+i) != oddParity[(int)*(key+i)])
            return(0);
    }
    return(1);
}

#endif

#ifdef TM_USE_DES
void tfDesSetParity(tt8BitPtr key)
{
    int i;

    for (i=0; i<(int)TM_DES_KEY_SIZE; i++)
    {
        *(key+i) = oddParity[(int)*(key+i)];
    }
}



void tfDesEcbEncrypt(tt8BitPtr inputblock,tt8BitPtr outputblock,
      tt32BitPtr ks, int encrypt)
{
    register tt32Bit l;
    register tt8BitPtr in,out;
    tt32Bit ll[2];

    in= inputblock;
    out= outputblock;
    tm_des_c2l(in,l); ll[0]=l;
    tm_des_c2l(in,l); ll[1]=l;
    tfDesEncrypt(ll,ks,encrypt);
    l=ll[0]; tm_des_l2c(l,out);
    l=ll[1]; tm_des_l2c(l,out);
    l=ll[0]=ll[1]=0;
}
#endif /* TM_USE_DES */

static void tfDesEncrypt(tt32BitPtr data,tt32BitPtr ks, int encrypt)
{
    /*ks = key schedule */
    register tt32Bit l,r,t,u;
    register int i;
    register tt32BitPtr s;

    u=data[0];
    r=data[1];

    tm_des_ip(u,r);
    /* Things have been modified so that the initial rotate is
     * done outside the loop.  This required the
     * desSPtrans values in sp.h to be rotated 1 bit to the right.
     * One perl script later and things have a 5% speed up on a sparc2.
     * Thanks to Richard Outerbridge <71755.204@CompuServe.COM>
     * for pointing this out. */
    l=(r<<(unsigned)1)|(r>>(unsigned)31);
    r=(u<<(unsigned)1)|(u>>(unsigned)31);

    s=(tt32BitPtr)ks;
    /* I don't know if it is worth the effort of loop unrolling the
     * inner loop
     */
    if (encrypt != 0)
    {
        for (i=0; i<32; i+=8)
        {
            tm_des_encrypt(l,r,i+0); /*  1 */
            tm_des_encrypt(r,l,i+2); /*  2 */
            tm_des_encrypt(l,r,i+4); /*  3 */
            tm_des_encrypt(r,l,i+6); /*  4 */
        }
    }
    else
    {
        for (i=30; i>0; i-=8)
        {
            tm_des_encrypt(l,r,i-0); /* 16 */
            tm_des_encrypt(r,l,i-2); /* 15 */
            tm_des_encrypt(l,r,i-4); /* 14 */
            tm_des_encrypt(r,l,i-6); /* 13 */
        }
    }
    l=(l>>(unsigned)1)|(l<<(unsigned)31);
    r=(r>>(unsigned)1)|(r<<(unsigned)31);

    tm_des_fp(r,l);
    data[0]=l;
    data[1]=r;
    l=r=t=u=0;
}

#ifdef TM_USE_DES
int tfEspDesSchedLen(void)
{

    return sizeof(ttDesKeySchedule);
}

int tfEspDesSchedule(ttEspAlgorithmPtr  algoPtr,
                     ttGenericKeyPtr    gkeyPtr)
{
    TM_UNREF_IN_ARG(algoPtr);
    
    return tfDesSetKey(gkeyPtr->keyDataPtr,
                       (tt32BitPtr)gkeyPtr->keySchedulePtr);
}



int tfEspDesBlockDecrypt( ttEspAlgorithmPtr algoPtr,
                          ttGenericKeyPtr   gkeyPtr,
                          tt8BitPtr         s,
                          tt8BitPtr         d)
{
    TM_UNREF_IN_ARG(algoPtr);
/* assumption: d has a good alignment */
    tm_bcopy(s, d, sizeof(tt32Bit) * 2);
    tfDesEcbEncrypt(d, d, 
            (tt32BitPtr)gkeyPtr->keySchedulePtr, TM_ESP_DECRYPT);
    return 0;
}

int tfEspDesBlockEncrypt( ttEspAlgorithmPtr algoPtr,
                          ttGenericKeyPtr   gkeyPtr,
                          tt8BitPtr         s,
                          tt8BitPtr         d)
{
    TM_UNREF_IN_ARG(algoPtr);
/* assumption: d has a good alignment */
    tm_bcopy(s, d, sizeof(tt32Bit) * 2);
    tfDesEcbEncrypt(d, d,
        (tt32BitPtr)gkeyPtr->keySchedulePtr, TM_ESP_ENCRYPT);
    return 0;
}
#endif /* TM_USE_DES */

/*-----------------------  esp-3des ----------------------- */
#ifdef TM_USE_3DES
int tfEsp3DesSchedLen(void)
{

    return sizeof(ttDesKeySchedule) * 3;
}

int tfEsp3DesSchedule(ttEspAlgorithmPtr   algoPtr,
                      ttGenericKeyPtr     gkeyPtr)
{
    int                   error;
    tt8BitPtr             schPtr;
    int                   i;
    tt8BitPtr             k;

    error = TM_ENOERROR;
    TM_UNREF_IN_ARG(algoPtr);
    schPtr = (tt8BitPtr)gkeyPtr->keySchedulePtr;
    k = (tt8BitPtr)(gkeyPtr->keyDataPtr);

    error = tm_memcmp( k, k+8, 8 );
    if( error == 0 )
    {
        error = TM_EINVAL;/*TM_E3DESWEEKKEY;*/
        goto DES3_RETURN;
    }
    error = tm_memcmp( k+8, k+16, 8 );
    if( error == 0 )
    {
        error = TM_EINVAL;/*TM_E3DESWEEKKEY;*/
        goto DES3_RETURN;
    }
    for (i = 0; i < 3; i++) 
    {
        error = tfDesSetKey(k + 8 * i, 
            (tt32BitPtr)(schPtr + (int)sizeof(ttDesKeySchedule) *i));
        if (error)
        {
            error = TM_EINVAL; 
            goto DES3_RETURN;
        }
    }
DES3_RETURN:
    return error;
}


int tfEsp3DesBlockDecrypt( ttEspAlgorithmPtr algoPtr,
                           ttGenericKeyPtr   gkeyPtr,
                           tt8BitPtr         s,
                           tt8BitPtr         d)
{
    ttDesKeySchedulePtr   p;

    TM_UNREF_IN_ARG(algoPtr);
/* assumption: d has a good alignment */
    p = (ttDesKeySchedulePtr)gkeyPtr->keySchedulePtr;
    tm_bcopy(s, d, sizeof(tt32Bit) * 2);
    tfDesEcbEncrypt(d, d, (tt32BitPtr)p[2], TM_ESP_DECRYPT);
    tfDesEcbEncrypt(d, d, (tt32BitPtr)p[1], TM_ESP_ENCRYPT);
    tfDesEcbEncrypt(d, d, (tt32BitPtr)p[0], TM_ESP_DECRYPT);
    return 0;
}

int tfEsp3DesBlockEncrypt( ttEspAlgorithmPtr algoPtr,
                           ttGenericKeyPtr   gkeyPtr,
                           tt8BitPtr         s,
                           tt8BitPtr         d)
{
    ttDesKeySchedulePtr    p;

    TM_UNREF_IN_ARG(algoPtr);
/* assumption: d has a good alignment */
    p = (ttDesKeySchedulePtr)gkeyPtr->keySchedulePtr;
    tm_bcopy(s, d, sizeof(tt32Bit) * 2);
    tfDesEcbEncrypt(d, d, (tt32BitPtr)p[0], TM_ESP_ENCRYPT);
    tfDesEcbEncrypt(d, d, (tt32BitPtr)p[1], TM_ESP_DECRYPT);
    tfDesEcbEncrypt(d, d, (tt32BitPtr)p[2], TM_ESP_ENCRYPT);
    return 0;
}
#endif /* TM_USE_3DES */


#else /* ! TM_USE_DES || TM_USE_3DES*/

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_DES is not defined */
int tlDesDummy = 0;

#endif /* TM_USE_DES || TM_USE_3DES */
