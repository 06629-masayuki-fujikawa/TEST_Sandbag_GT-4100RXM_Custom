/*
 * Description: Testing the ESP algorithem: twofish
 *
 * Filename: trtwofsh.c
 * Author: Jin Zhang
 * Date Created: 08/01/2001
 * $Source: source/cryptlib/trtwofsh.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:35:42JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/***************************************************************************
 Copyright (c) 2002 by Niels Ferguson.

  The author hereby grants a perpetual license to everybody to use this
  code for any purpose as long as the copyright message is included in the
  source code of this or any derived work.

  Yes, this means that you, your company, your club, and anyone else can
  use this code anywhere you want.  You can change it and distribute it
  under the GPL, include it in your commercial product without releasing
  the source code, put it on the web, etc.  The only thing you cannot do is
  remove my copyright message, or distribute any source code based on this
  implementation that does not include my copyright message.

  I appreciate a mention in the documentation or credits, but I understand
  if that is difficult to do.  I also appreciate it if you tell me where
  and why you used my code.

  DISCLAIMER: As I'm giving away my work for free, I'm of course not going
  to accept any liability of any form.  This code, or the Twofish cipher,
  might very well be flawed; you have been warned.  This software is
  provided as-is, without any kind of warrenty or guarantee.  And that is
  really all you can expect when you download code for free from the
  Internet.
***************************************************************************/


#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trcrylib.h>
#include <trglobal.h> /* tvTwoFishInit */

#ifdef TM_USE_TWOFISH



/* Generic rotation ops (Use intrinsic rotate for performance, if possible) */
#define    tm_twofish_rol(x,n) \
          (((x) << ((n) & 0x1F)) | ((x) >> (32-((n) & 0x1F))))
#define    tm_twofish_ror(x,n) \
          (((x) >> ((n) & 0x1F)) | ((x) << (32-((n) & 0x1F))))



/* XXX should use ALIGNBYTES, however, we cannot do that at runtime */

/*  Build higher-level constructs based on endianness setting  */
#ifdef  TM_LITTLE_ENDIAN
#define        tm_twofish_bswap(x)            (x) 
/* NOP for little-endian machines */
#define        TM_TWOFISH_ADDR_XOR            0        
/* NOP for little-endian machines */
#else
#define        tm_twofish_bswap(x)           \
 ((tm_twofish_ror(x,8) & TM_UL(0xFF00FF00)) | \
 (tm_twofish_rol(x,8) & TM_UL(0x00FF00FF)))
#define        TM_TWOFISH_ADDR_XOR            3        
/* convert byte address in dword */
#endif

/*    Macros for extracting bytes from dwords (correct for endianness) */
#define    _b(x,N)    (((tt8BitPtr)&x)[((N) & 3) ^ TM_TWOFISH_ADDR_XOR]) 
/* pick bytes out of a dword */

#define        b0(x)            _b(x,0)        /* extract LSB of tt32Bit */
#define        b1(x)            _b(x,1)
#define        b2(x)            _b(x,2)
#define        b3(x)            _b(x,3)        /* extract MSB of tt32Bit */

/*    Defines:
        Add any additional defines you need
*/

/* CHANGE POSSIBLE: inclusion of algorithm specific defines */
/* TWOFISH specific definitions */


/* default number of rounds for 128-bit keys*/
#define        TM_TWOFISH_ROUNDS_128             16    
/* default number of rounds for 192-bit keys*/
#define        TM_TWOFISH_ROUNDS_192             16    
/* default number of rounds for 256-bit keys*/
#define        TM_TWOFISH_ROUNDS_256             16    

/* initialization signature ('FISH') */
#define        TM_TWOFISH_VALID_SIG     TM_UL(0x48534946)




/* Typedefs:
    Typedef'ed data storage elements. Add any algorithm specific
    parameters at the bottom of the structs as appropriate.
*/



/* for computing subkeys */
#define    TM_TF_SK_STEP            TM_UL(0x02020202)
#define    TM_TF_SK_BUMP            TM_UL(0x01010101)
#define    TM_TF_SK_ROTL            9

/* Reed-Solomon code parameters: (12,8) reversible code
    g(x) = x**4 + (a + 1/a) x**3 + a x**2 + (a + 1/a) x + 1
   where a = primitive root of field generator 0x14D */
#define    TM_TF_RS_GF_FDBK        0x14D        /* field generator */
#define    tm_twofish_rs_rem(x)        \
    { tt8Bit  b  = (tt8Bit) (x >> 24);          \
      tt32Bit g2 = \
           (tt32Bit)(((b << 1) ^ (((b & 0x80)!= \
           (tt8Bit)0 ) ? TM_TF_RS_GF_FDBK : 0 )) & 0xFF);  \
      tt32Bit g3 = \
           (tt32Bit)(((b >> 1) & 0x7F) ^ (((b & 1) != \
           (tt8Bit)0) ? TM_TF_RS_GF_FDBK >> 1 : 0 ) ^ g2 ); \
      x = (x << 8) ^ (g3 << 24) ^ (g2 << 16) ^ (g3 << 8) ^ b; \
    }

/*    Macros for the MDS matrix
*    The MDS matrix is (using primitive polynomial 169):
*      01  EF  5B  5B
*      5B  EF  EF  01
*      EF  5B  01  EF
*      EF  01  EF  5B
*----------------------------------------------------------------
* More statistical properties of this matrix (from MDS.EXE output):
*
* Min Hamming weight (one byte difference) =  8. Max=26.  Total =  1020.
* Prob[8]:      7    23    42    20    52    95    88    94   121   128    91
*             102    76    41    24     8     4     1     3     0     0     0
* Runs[8]:      2     4     5     6     7     8     9    11
* MSBs[8]:      1     4    15     8    18    38    40    43
* HW= 8: 05040705 0A080E0A 14101C14 28203828 50407050 01499101 A080E0A0 
* HW= 9: 04050707 080A0E0E 10141C1C 20283838 40507070 80A0E0E0 C6432020 07070504 
*        0E0E0A08 1C1C1410 38382820 70705040 E0E0A080 202043C6 05070407 0A0E080E 
*        141C101C 28382038 50704070 A0E080E0 4320C620 02924B02 089A4508 
* Min Hamming weight (two byte difference) =  3. Max=28.  Total = 390150.
* Prob[3]:      7    18    55   149   270   914  2185  5761 11363 20719 32079
*           43492 51612 53851 52098 42015 31117 20854 11538  6223  2492  1033
* MDS OK, tm_twofish_ror:   6+  7+  8+  9+ 10+ 11+ 12+ 13+ 14+ 15+ 16+
*               17+ 18+ 19+ 20+ 21+ 22+ 23+ 24+ 25+ 26+
*/
#define    TM_TF_MDS_GF_FDBK        0x169 /* primitive polynomial for GF(256)*/
#define    tm_twofish_lfsr1(x) \
           ( ((x) >> 1)  ^ ((((x) & 0x01) != \
           (tt8Bit)0) ?   TM_TF_MDS_GF_FDBK/2 : 0))
#define    tm_twofish_lfsr2(x)  \
           ( ((x) >> 2)  ^ ((((x) & 0x02) != \
           (tt8Bit)0) ?   TM_TF_MDS_GF_FDBK/2 : 0)  \
           ^ ((((x) & 0x01) != (tt8Bit)0) \
           ?   TM_TF_MDS_GF_FDBK/4 : 0))

#define    tm_twofish_mx_1(x) ((tt32Bit)  (x))        
/* force result to dword so << will work */
#define    tm_twofish_mx_x(x)  \
           ((tt32Bit) ((x) ^            tm_twofish_lfsr2(x)))    /* 5B */
#define    tm_twofish_mx_y(x) \
           ((tt32Bit) ((x) ^ tm_twofish_lfsr1(x) ^ tm_twofish_lfsr2(x))) /* EF */

/* use the following macro for matrix */
#define    MMATRIX00        m1_matrix
#define    MMATRIX01        mY_matrix
#define    MMATRIX02        mX_matrix
#define    MMATRIX03        mX_matrix

#define    MMATRIX10        mX_matrix
#define    MMATRIX11        mY_matrix
#define    MMATRIX12        mY_matrix
#define    MMATRIX13        m1_matrix

#define    MMATRIX20        mY_matrix
#define    MMATRIX21        mX_matrix
#define    MMATRIX22        m1_matrix
#define    MMATRIX23        mY_matrix

#define    MMATRIX30        mY_matrix
#define    MMATRIX31        m1_matrix
#define    MMATRIX32        mY_matrix
#define    MMATRIX33        mX_matrix


/*    Define the fixed p0/p1 permutations used in keyed S-box lookup.  
    By changing the following constant definitions for P_ij, the S-boxes will
    automatically get changed in all the Twofish source code. Note that P_i0 is
    the "outermost" 8x8 permutation applied.  See the f32() function to see
    how these constants are to be  used.
*/
#define    P_00    1                    /* "outermost" permutation */
#define    P_01    0
#define    P_02    0
#define    P_03    (P_01^1)            /* "extend" to larger key sizes */
#define    P_04    1

#define    P_10    0
#define    P_11    0
#define    P_12    1
#define    P_13    (P_11^1)
#define    P_14    0

#define    P_20    1
#define    P_21    1
#define    P_22    0
#define    P_23    (P_21^1)
#define    P_24    0

#define    P_30    0
#define    P_31    1
#define    P_32    1
#define    P_33    (P_31^1)
#define    P_34    1

#define    p8(N)    P8x8[(int)(P_##N)]
/* some syntax shorthand */

/* fixed 8x8 permutation S-boxes */

/***********************************************************************
*  07:07:14  05/30/98  [4x4]  TestCnt=256. keySize=128. CRC=4BD14D9E.
* maxKeyed:  dpMax = 18. lpMax =100. fixPt =  8. skXor =  0. skDup =  6. 
* log2(dpMax[ 6..18])=   --- 15.42  1.33  0.89  4.05  7.98 12.05
* log2(lpMax[ 7..12])=  9.32  1.01  1.16  4.23  8.02 12.45
* log2(fixPt[ 0.. 8])=  1.44  1.44  2.44  4.06  6.01  8.21 11.07 14.09 17.00
* log2(skXor[ 0.. 0])
* log2(skDup[ 0.. 6])=   ---  2.37  0.44  3.94  8.36 13.04 17.99
***********************************************************************/
static const tt8Bit TM_CONST_QLF P8x8[2][256]=
{
/*  p0:   */
/*  dpMax      = 10.  lpMax      = 64.  cycleCnt=   1  1  1  0.         */
/* 817D6F320B59ECA4.ECB81235F4A6709D.BA5E6D90C8F32471.D7F4126E9B3085CA. */
/* Karnaugh maps:
0111 0001 0011 1010. 0001 1001 1100 1111. 1001 1110 0011 1110. 1101 0101 1111 1001. 
0101 1111 1100 0100. 1011 0101 0010 0000. 0101 1000 1100 0101. 1000 0111 0011 0010. 
0000 1001 1110 1101. 1011 1000 1010 0011. 0011 1001 0101 0000. 0100 0010 0101 1011. 
0111 0100 0001 0110. 1000 1011 1110 1001. 0011 0011 1001 1101. 1101 0101 0000 1100. 
*/
    {
    0xA9, 0x67, 0xB3, 0xE8, 0x04, 0xFD, 0xA3, 0x76, 
    0x9A, 0x92, 0x80, 0x78, 0xE4, 0xDD, 0xD1, 0x38, 
    0x0D, 0xC6, 0x35, 0x98, 0x18, 0xF7, 0xEC, 0x6C, 
    0x43, 0x75, 0x37, 0x26, 0xFA, 0x13, 0x94, 0x48, 
    0xF2, 0xD0, 0x8B, 0x30, 0x84, 0x54, 0xDF, 0x23, 
    0x19, 0x5B, 0x3D, 0x59, 0xF3, 0xAE, 0xA2, 0x82, 
    0x63, 0x01, 0x83, 0x2E, 0xD9, 0x51, 0x9B, 0x7C, 
    0xA6, 0xEB, 0xA5, 0xBE, 0x16, 0x0C, 0xE3, 0x61, 
    0xC0, 0x8C, 0x3A, 0xF5, 0x73, 0x2C, 0x25, 0x0B, 
    0xBB, 0x4E, 0x89, 0x6B, 0x53, 0x6A, 0xB4, 0xF1, 
    0xE1, 0xE6, 0xBD, 0x45, 0xE2, 0xF4, 0xB6, 0x66, 
    0xCC, 0x95, 0x03, 0x56, 0xD4, 0x1C, 0x1E, 0xD7, 
    0xFB, 0xC3, 0x8E, 0xB5, 0xE9, 0xCF, 0xBF, 0xBA, 
    0xEA, 0x77, 0x39, 0xAF, 0x33, 0xC9, 0x62, 0x71, 
    0x81, 0x79, 0x09, 0xAD, 0x24, 0xCD, 0xF9, 0xD8, 
    0xE5, 0xC5, 0xB9, 0x4D, 0x44, 0x08, 0x86, 0xE7, 
    0xA1, 0x1D, 0xAA, 0xED, 0x06, 0x70, 0xB2, 0xD2, 
    0x41, 0x7B, 0xA0, 0x11, 0x31, 0xC2, 0x27, 0x90, 
    0x20, 0xF6, 0x60, 0xFF, 0x96, 0x5C, 0xB1, 0xAB, 
    0x9E, 0x9C, 0x52, 0x1B, 0x5F, 0x93, 0x0A, 0xEF, 
    0x91, 0x85, 0x49, 0xEE, 0x2D, 0x4F, 0x8F, 0x3B, 
    0x47, 0x87, 0x6D, 0x46, 0xD6, 0x3E, 0x69, 0x64, 
    0x2A, 0xCE, 0xCB, 0x2F, 0xFC, 0x97, 0x05, 0x7A, 
    0xAC, 0x7F, 0xD5, 0x1A, 0x4B, 0x0E, 0xA7, 0x5A, 
    0x28, 0x14, 0x3F, 0x29, 0x88, 0x3C, 0x4C, 0x02, 
    0xB8, 0xDA, 0xB0, 0x17, 0x55, 0x1F, 0x8A, 0x7D, 
    0x57, 0xC7, 0x8D, 0x74, 0xB7, 0xC4, 0x9F, 0x72, 
    0x7E, 0x15, 0x22, 0x12, 0x58, 0x07, 0x99, 0x34, 
    0x6E, 0x50, 0xDE, 0x68, 0x65, 0xBC, 0xDB, 0xF8, 
    0xC8, 0xA8, 0x2B, 0x40, 0xDC, 0xFE, 0x32, 0xA4, 
    0xCA, 0x10, 0x21, 0xF0, 0xD3, 0x5D, 0x0F, 0x00, 
    0x6F, 0x9D, 0x36, 0x42, 0x4A, 0x5E, 0xC1, 0xE0
    },
/*  p1:   */
/*  dpMax      = 10.  lpMax      = 64.  cycleCnt=   2  0  0  1.         */
/* 28BDF76E31940AC5.1E2B4C376DA5F908.4C75169A0ED82B3F.B951C3DE647F208A. */
/* Karnaugh maps:
0011 1001 0010 0111. 1010 0111 0100 0110. 0011 0001 1111 0100. 1111 1000 0001 1100. 
1100 1111 1111 1010. 0011 0011 1110 0100. 1001 0110 0100 0011. 0101 0110 1011 1011. 
0010 0100 0011 0101. 1100 1000 1000 1110. 0111 1111 0010 0110. 0000 1010 0000 0011. 
1101 1000 0010 0001. 0110 1001 1110 0101. 0001 0100 0101 0111. 0011 1011 1111 0010. 
*/
    {
    0x75, 0xF3, 0xC6, 0xF4, 0xDB, 0x7B, 0xFB, 0xC8, 
    0x4A, 0xD3, 0xE6, 0x6B, 0x45, 0x7D, 0xE8, 0x4B, 
    0xD6, 0x32, 0xD8, 0xFD, 0x37, 0x71, 0xF1, 0xE1, 
    0x30, 0x0F, 0xF8, 0x1B, 0x87, 0xFA, 0x06, 0x3F, 
    0x5E, 0xBA, 0xAE, 0x5B, 0x8A, 0x00, 0xBC, 0x9D, 
    0x6D, 0xC1, 0xB1, 0x0E, 0x80, 0x5D, 0xD2, 0xD5, 
    0xA0, 0x84, 0x07, 0x14, 0xB5, 0x90, 0x2C, 0xA3, 
    0xB2, 0x73, 0x4C, 0x54, 0x92, 0x74, 0x36, 0x51, 
    0x38, 0xB0, 0xBD, 0x5A, 0xFC, 0x60, 0x62, 0x96, 
    0x6C, 0x42, 0xF7, 0x10, 0x7C, 0x28, 0x27, 0x8C, 
    0x13, 0x95, 0x9C, 0xC7, 0x24, 0x46, 0x3B, 0x70, 
    0xCA, 0xE3, 0x85, 0xCB, 0x11, 0xD0, 0x93, 0xB8, 
    0xA6, 0x83, 0x20, 0xFF, 0x9F, 0x77, 0xC3, 0xCC, 
    0x03, 0x6F, 0x08, 0xBF, 0x40, 0xE7, 0x2B, 0xE2, 
    0x79, 0x0C, 0xAA, 0x82, 0x41, 0x3A, 0xEA, 0xB9, 
    0xE4, 0x9A, 0xA4, 0x97, 0x7E, 0xDA, 0x7A, 0x17, 
    0x66, 0x94, 0xA1, 0x1D, 0x3D, 0xF0, 0xDE, 0xB3, 
    0x0B, 0x72, 0xA7, 0x1C, 0xEF, 0xD1, 0x53, 0x3E, 
    0x8F, 0x33, 0x26, 0x5F, 0xEC, 0x76, 0x2A, 0x49, 
    0x81, 0x88, 0xEE, 0x21, 0xC4, 0x1A, 0xEB, 0xD9, 
    0xC5, 0x39, 0x99, 0xCD, 0xAD, 0x31, 0x8B, 0x01, 
    0x18, 0x23, 0xDD, 0x1F, 0x4E, 0x2D, 0xF9, 0x48, 
    0x4F, 0xF2, 0x65, 0x8E, 0x78, 0x5C, 0x58, 0x19, 
    0x8D, 0xE5, 0x98, 0x57, 0x67, 0x7F, 0x05, 0x64, 
    0xAF, 0x63, 0xB6, 0xFE, 0xF5, 0xB7, 0x3C, 0xA5, 
    0xCE, 0xE9, 0x68, 0x44, 0xE0, 0x4D, 0x43, 0x69, 
    0x29, 0x2E, 0xAC, 0x15, 0x59, 0xA8, 0x0A, 0x9E, 
    0x6E, 0x47, 0xDF, 0x34, 0x35, 0x6A, 0xCF, 0xDC, 
    0x22, 0xC9, 0xC0, 0x9B, 0x89, 0xD4, 0xED, 0xAB, 
    0x12, 0xA2, 0x0D, 0x52, 0xBB, 0x02, 0x2F, 0xA9, 
    0xD7, 0x61, 0x1E, 0xB4, 0x50, 0x04, 0xF6, 0xC2, 
    0x16, 0x25, 0x86, 0x56, 0x55, 0x09, 0xBE, 0x91
    }
};

static int tfTfParseHexDword (int, ttConst8BitPtr, tt32BitPtr, tt8BitPtr);
static tt32Bit tfTwofishRsmdsEncode (tt32Bit, tt32Bit);
static void tfTwofishBuildMds (void);
static void tfTfReverseRoundSubkeys (ttTwofishKeyInstancePtr, tt8Bit);
static int tfTwofishReKey(ttTwofishKeyInstancePtr key);


/*
+*****************************************************************************
*            Constants/Macros/Tables
-****************************************************************************/


static         tt32Bit MDStab[4][256];   /* not actually const, init once */

/* number of rounds for various key sizes:  128, 192, 256 */
/* (ignored for now in optimized code!) */
static const int  TM_CONST_QLF numRounds[4]= {0,
                            TM_TWOFISH_ROUNDS_128,
                            TM_TWOFISH_ROUNDS_192,
                            TM_TWOFISH_ROUNDS_256 };

#if TM_TWOFISH_REENTRANT
#define        _sBox_  key->sBox8x32
#else
static   tt32Bit      _sBox_[4][256];    /* permuted MDStab based on keys */
#endif

#define _sBox8_(N) (((tt8Bit *) _sBox_) + (N)*256)

/*------- see what level of S-box precomputation we need to do -----*/
#if   defined(ZERO_KEY)
#define    Fe32_128(x,R)    \
    (   MDStab[0][p8(01)[p8(02)[_b(x,R  )]^b0(SKEY[1])]^b0(SKEY[0])] ^    \
        MDStab[1][p8(11)[p8(12)[_b(x,R+1)]^b1(SKEY[1])]^b1(SKEY[0])] ^    \
        MDStab[2][p8(21)[p8(22)[_b(x,R+2)]^b2(SKEY[1])]^b2(SKEY[0])] ^    \
        MDStab[3][p8(31)[p8(32)[_b(x,R+3)]^b3(SKEY[1])]^b3(SKEY[0])] )
#define    Fe32_192(x,R)    \
    (   MDStab[0][p8(01)[p8(02)[p8(03)[_b(x,R  )]^ \
                  b0(SKEY[2])]^b0(SKEY[1])]^b0(SKEY[0])] ^ \
        MDStab[1][p8(11)[p8(12)[p8(13)[_b(x,R+1)]^ \
                  b1(SKEY[2])]^b1(SKEY[1])]^b1(SKEY[0])] ^ \
        MDStab[2][p8(21)[p8(22)[p8(23)[_b(x,R+2)]^ \
                  b2(SKEY[2])]^b2(SKEY[1])]^b2(SKEY[0])] ^ \
        MDStab[3][p8(31)[p8(32)[p8(33)[_b(x,R+3)]^ \
                  b3(SKEY[2])]^b3(SKEY[1])]^b3(SKEY[0])] )
#define    Fe32_256(x,R)    \
    (   MDStab[0][p8(01)[p8(02)[p8(03)[p8(04)[_b(x,R  )]^ \
                  b0(SKEY[3])]^b0(SKEY[2])]^b0(SKEY[1])]^b0(SKEY[0])] ^ \
        MDStab[1][p8(11)[p8(12)[p8(13)[p8(14)[_b(x,R+1)]^ \
                  b1(SKEY[3])]^b1(SKEY[2])]^b1(SKEY[1])]^b1(SKEY[0])] ^ \
        MDStab[2][p8(21)[p8(22)[p8(23)[p8(24)[_b(x,R+2)]^ \
                  b2(SKEY[3])]^b2(SKEY[2])]^b2(SKEY[1])]^b2(SKEY[0])] ^ \
        MDStab[3][p8(31)[p8(32)[p8(33)[p8(34)[_b(x,R+3)]^ \
                  b3(SKEY[3])]^b3(SKEY[2])]^b3(SKEY[1])]^b3(SKEY[0])] )

#define    GetSboxKey    tt32Bit SKEY[4];    /* local copy */ \
                    tm_bcopy(key->sboxKeys,SKEY,sizeof(SKEY));
/* MIN_KEY*/
#elif defined(MIN_KEY)


#define    Fe32_0(x,R)(MDStab[0][p8(01)[_sBox8_(0)[_b(x,R  )]] ^ b0(SKEY0)]^\
                   MDStab[1][p8(11)[_sBox8_(1)[_b(x,R+1)]] ^ b1(SKEY0)] ^ \
                   MDStab[2][p8(21)[_sBox8_(2)[_b(x,R+2)]] ^ b2(SKEY0)] ^ \
                   MDStab[3][p8(31)[_sBox8_(3)[_b(x,R+3)]] ^ b3(SKEY0)])
#define sbSet(N,i,J,v) { _sBox8_(N)[i+J] = v; }
#define    GetSboxKey    tt32Bit SKEY0    = key->sboxKeys[0]  /* local copy */
/*--PART_KEY */
#elif defined(PART_KEY)    

#define    Fe32_0(x,R)(MDStab[0][_sBox8_(0)[_b(x,R  )]] ^ \
                   MDStab[1][_sBox8_(1)[_b(x,R+1)]] ^ \
                   MDStab[2][_sBox8_(2)[_b(x,R+2)]] ^ \
                   MDStab[3][_sBox8_(3)[_b(x,R+3)]])
#define sbSet(N,i,J,v) { _sBox8_(N)[i+J] = v; }
#define    GetSboxKey    
/*--FULL KEY */
#else    /* default is FULL_KEY */
#ifndef FULL_KEY
#define    FULL_KEY    1
#endif



/* Fe32_0 does a full S-box + MDS lookup.  Need to #define _sBox_ before use.
   Note that we "interleave" 0,1, and 2,3 to avoid cache bank collisions
   in optimized assembly language.
*/
#define    Fe32_0(x,R) (_sBox_[0][2*_b(x,R  )] ^ _sBox_[0][2*_b(x,R+1)+1] ^  \
                    _sBox_[2][2*_b(x,R+2)] ^ _sBox_[2][2*_b(x,R+3)+1]) 

        /* set a single S-box value, given the input byte */
#define sbSet(N,i,J,v) { _sBox_[N&2][2*i+(N&1)+2*J] = \
        MDStab[(tt32Bit)N][(tt32Bit)v]; }
#define    GetSboxKey    
#endif



#define        VALIDATE_PARMS    0        /* disable for full speed */

/* end of debug macros */



/*
+*****************************************************************************
*
* Function Name:    tfTfParseHexDword
*
* Function:            Parse ASCII hex nibbles and fill in key/iv dwords
*
* Arguments:        bit            =    # bits to read
*                    srcTxt        =    ASCII source
*                    d            =    ptr to dwords to fill in
*                    dstTxt        =    where to make a copy of ASCII source
*                                    (NULL ok)
*
* Return:            Zero if no error.  Nonzero --> invalid hex or length
*
* Notes:  Note that the parameter d is a tt32Bit array, not a byte array.
*    This routine is coded to work both for little-endian and big-endian
*    architectures.  The character stream is interpreted as a LITTLE-ENDIAN
*    byte stream, since that is how the Pentium works, but the conversion
*    happens automatically below. 
*
-****************************************************************************/
static int tfTfParseHexDword(int bits,
                             ttConst8BitPtr srcTxt,
                             tt32BitPtr d,
                             tt8BitPtr dstTxt)
{
    int i;
    tt8Bit c;
    tt32Bit b;

#if VALIDATE_PARMS
  #if TM_TWOFISH_ALIGN32
    if (((int)d) & 3)
        return TM_ETWOFISHBADALIGN32;    
  #endif
#endif

    for (i=0;i*32<bits;i++)
        d[i]=0;                    /* first, zero the field */

    for (i=0;i*8<bits;i++)        /* parse one nibble at a time */
    {                        /* case out the hexadecimal characters */
        c = (tt8Bit)srcTxt[i];
        if (dstTxt) dstTxt[i]=srcTxt[i];
        b = (tt32Bit)c;
        /* works for big and little endian! */
        d[i/4] |= b << ((i % 4) * 8);
    }

    return 0;                    /* no error */
}




/*
+*****************************************************************************
*
* Function Name:    RS_MDS_encode
*
* Function:            Use (12,8) Reed-Solomon code over GF(256) to produce
*                    a key S-box dword from two key material dwords.
*
* Arguments:        k0    =    1st dword
*                    k1    =    2nd dword
*
* Return:            Remainder polynomial generated using RS code
*
* Notes:
     Since this computation is done only once per reKey per 64 bits of key,
     the performance impact of this routine is imperceptible. The RS code
     chosen has "simple" coefficients to allow smartcard/hardware
     implementation without lookup tables.
*
-****************************************************************************/
static tt32Bit tfTwofishRsmdsEncode(tt32Bit k0,tt32Bit k1)
{
    int i,j;
    tt32Bit r;

    for (i=0, r=(tt32Bit)0;i<2;i++)
    {
        r ^= (i != 0 )? k0 : k1;            /* merge in 32 more key bits */
/* r ^= (i) ? k0 : k1; */
        for (j=0;j<4;j++)            /* shift one byte at a time */
/* PRQA: QAC Message 3762: Implicit conversion: int to unsigned long. */
/* PRQA S 3762 L1 */
            tm_twofish_rs_rem(r);                
/* PRQA L:L1 */
    }
    return r;
}


/*
+*****************************************************************************
*
* Function Name:    tfTwofishBuildMds
*
* Function:            Initialize the MDStab array
*
* Arguments:        None.
*
* Return:            None.
*
* Notes:
*    Here we precompute all the fixed MDS table.  This only needs to be done
*    one time at initialization, after which the table is "const".
*
-****************************************************************************/
static void tfTwofishBuildMds(void)
{
    int i;
    tt32Bit d;
    tt8Bit m1_matrix[2],mX_matrix[2],mY_matrix[4];

    d = 0;
    for (i=0;i<256;i++)
    {
        m1_matrix[0]=P8x8[0][i];        /* compute all the matrix elements */
        mX_matrix[0]=(tt8Bit) tm_twofish_mx_x(m1_matrix[0]);
        mY_matrix[0]=(tt8Bit) tm_twofish_mx_y(m1_matrix[0]);

        m1_matrix[1]=P8x8[1][i];
        mX_matrix[1]=(tt8Bit) tm_twofish_mx_x(m1_matrix[1]);
        mY_matrix[1]=(tt8Bit) tm_twofish_mx_y(m1_matrix[1]);


#define    SetMDS(N)                    \
        b0(d) = MMATRIX0##N[P_##N##0];    \
        b1(d) = MMATRIX1##N[P_##N##0];    \
        b2(d) = MMATRIX2##N[P_##N##0];    \
        b3(d) = MMATRIX3##N[P_##N##0];    \
        MDStab[N][i] = d;

        SetMDS(0);        /* fill in the matrix with elements computed above */
        SetMDS(1);
        SetMDS(2);
        SetMDS(3);
    }
    

    tm_global(tvTwoFishInit) = TM_8BIT_YES; /* NEVER modify the table again! */
}

/*
+*****************************************************************************
*
* Function Name:    tfTfReverseRoundSubkeys
*
* Function:  Reverse order of round subkeys to switch between encrypt/decrypt
*
* Arguments:        key        =    ptr to ttTwofishKeyInstance to be reversed
*                    newDir    =    new direction value
*
* Return:            None.
*
* Notes:
     This optimization allows both blockEncrypt and blockDecrypt to use the
     same "fallthru" switch statement based on the number of rounds.
     Note that key->numRounds must be even and >= 2 here.
*
-****************************************************************************/
static void tfTfReverseRoundSubkeys(ttTwofishKeyInstancePtr key,tt8Bit newDir)
{
    tt32Bit t0,t1;
    register tt32BitPtr r0=key->subKeys+TM_TWOFISH_ROUND_SUBKEYS;
    register tt32BitPtr r1=r0 + 2*key->numRounds - 2;

    for (;r0 < r1;r0+=2,r1-=2)
    {
        t0=r0[0];            /* swap the order */
        t1=r0[1];
        r0[0]=r1[0];        /* but keep relative order within pairs */
        r0[1]=r1[1];
        r1[0]=t0;
        r1[1]=t1;
    }

    key->direction=newDir;
}

/*
+*****************************************************************************
*
* Function Name:    Xor256
*
* Function:  Copy an 8-bit permutation (256 bytes), xoring with a byte
*
* Arguments:        dst        =    where to put result
*                    src        =    where to get data (can be same asa dst)
*                    b        =    byte to xor
*
* Return:            None
*
* Notes:
     BorlandC's optimization is terrible!  When we put the code inline,
     it generates fairly good code in the *following* segment (not in the
     Xor256 code itself).  If the call is made, the code following the call
     is awful! The penalty is nearly 50%!  So we take the code size hit for
     inlining for Borland, while Microsoft happily works with a call.
*
-****************************************************************************/
#if defined(__BORLANDC__)    /* do it inline */
#define Xor32(dst,src,i) {((tt32BitPtr)dst)[i] = ((tt32BitPtr)src)[i] ^ tmpX;}
#define    Xor256(dst,src,b)                \
    {                                    \
    register tt32Bit tmpX=TM_UL(0x01010101) * b;\
    for (i=0;i<64;i+=4)                    \
    { \
        Xor32(dst,src,i  ); \
        Xor32(dst,src,i+1); \
        Xor32(dst,src,i+2); \
        Xor32(dst,src,i+3); \
    } \
    }
#else                        /* do it as a function call */
static void Xor256 (ttVoidPtr dst, ttConstVoidPtr src, tt8Bit b);

static void Xor256(ttVoidPtr dst, ttConstVoidPtr src,tt8Bit b)
    {
    register tt32BitPtr d=(tt32BitPtr)dst;
    register tt32BitPtr s=(tt32BitPtr)src;
    /* replicate byte to all four bytes */
    register tt32Bit    x=(tt32Bit)b;
    x |= x << 16;
    x |= x << 8;

#define X_8(N)    { d[N]=s[N] ^ x; d[N+1]=s[N+1] ^ x; }
#define X_32(N)    { X_8(N); X_8(N+2); X_8(N+4); X_8(N+6); }
    X_32(0 ); X_32( 8); X_32(16); X_32(24);    /* all inline */
    d+=32;    /* keep offsets small! */
    s+=32;
    X_32(0 ); X_32( 8); X_32(16); X_32(24);    /* all inline */
    }
#endif

/*
+*****************************************************************************
*
* Function Name:    reKey
*
* Function:    Initialize the Twofish key schedule from key32
*
* Arguments:        key   =    ptr to ttTwofishKeyInstance to be initialized
*
* Return:            TM_ENOERROR on success
*
* Notes:
*    Here we precompute all the round subkeys, although that is not actually
*    required.  For example, on a smartcard, the round subkeys can 
*    be generated on-the-fly    using f32()
*
-****************************************************************************/
static int tfTwofishReKey(ttTwofishKeyInstancePtr key)
{
    tt32Bit    A=0,B=0,q;
    tt32Bit    sKey[TM_TWOFISH_MAX_KEY_BITS/64], \
                    k32e[TM_TWOFISH_MAX_KEY_BITS/64], \
                    k32o[TM_TWOFISH_MAX_KEY_BITS/64];
    int        i,j,k64Cnt,keyLen;
    int        subkeyCnt; 
    int        errorCode;
#if !defined(ZERO_KEY)
/* small local 8-bit permutations */
    tt8BitPtr  L0Ptr;
    tt8BitPtr  L1Ptr;
#endif /* ! ZERO_KEY */

    errorCode = TM_ENOERROR;

#if VALIDATE_PARMS
  #if TM_TWOFISH_ALIGN32
    if (((int)key) & 3)
        return TM_ETWOFISHBADALIGN32;
    if ((key->keyLen % 64) || (key->keyLen < TM_TWOFISH_MIN_KEY_BITS))
        return TM_EINVAL;
  #endif
#endif

    if (tm_global(tvTwoFishInit) == TM_8BIT_NO)
    {
/* do this one time only */
        tfTwofishBuildMds();
    }

#define    F32(res,x,k32)                                           \
    {                                                               \
    tt32Bit t=x;                                                    \
    switch (k64Cnt & 3)                                             \
    {                                                               \
        case 0:  /* same as 4 */                                    \
                 b0(t)   = (tt8Bit)(p8(04)[b0(t)] ^ b0(k32[3]));    \
                 b1(t)   = (tt8Bit)(p8(14)[b1(t)] ^ b1(k32[3]));    \
                 b2(t)   = (tt8Bit)(p8(24)[b2(t)] ^ b2(k32[3]));    \
                 b3(t)   = (tt8Bit)(p8(34)[b3(t)] ^ b3(k32[3]));    \
                 /* fall thru, having pre-processed t */            \
        case 3:  b0(t)   = (tt8Bit)(p8(03)[b0(t)] ^ b0(k32[2]));    \
                 b1(t)   = (tt8Bit)(p8(13)[b1(t)] ^ b1(k32[2]));    \
                 b2(t)   = (tt8Bit)(p8(23)[b2(t)] ^ b2(k32[2]));    \
                 b3(t)   = (tt8Bit)(p8(33)[b3(t)] ^ b3(k32[2]));    \
                 /* fall thru, having pre-processed t */            \
        case 2:     /* 128-bit keys (optimize for this case) */     \
            res= MDStab[0][p8(01)[p8(02)[b0(t)] ^ b0(k32[1])] ^ b0(k32[0])] ^ \
                 MDStab[1][p8(11)[p8(12)[b1(t)] ^ b1(k32[1])] ^ b1(k32[0])] ^ \
                 MDStab[2][p8(21)[p8(22)[b2(t)] ^ b2(k32[1])] ^ b2(k32[0])] ^ \
                 MDStab[3][p8(31)[p8(32)[b3(t)] ^ b3(k32[1])] ^ b3(k32[0])] ; \
        default:                                                    \
            break;                                                  \
    }\
    }


    subkeyCnt = TM_TWOFISH_ROUND_SUBKEYS + 2*key->numRounds;
    keyLen=key->keyLen;
    k64Cnt=(keyLen+63)/64;            /* number of 64-bit key words */
    for (i=0,j=k64Cnt-1;i<k64Cnt;i++,j--)
    {                            /* split into even/odd key dwords */
        k32e[i]=key->key32[2*i  ];
        k32o[i]=key->key32[2*i+1];
        /* compute S-box keys using (12,8) Reed-Solomon code over GF(256) */
        sKey[j]=key->sboxKeys[j]=tfTwofishRsmdsEncode(k32e[i],k32o[i]);    
        /* reverse order */
    }


    for (i=0,q=0;i<subkeyCnt/2;i++,q+=TM_TF_SK_STEP)    
    {                            /* compute round subkeys for PHT */
        F32(A,q        ,k32e);        /* A uses even key dwords */
        F32(B,q+TM_TF_SK_BUMP,k32o);        /* B uses odd  key dwords */
        B = tm_twofish_rol(B,8);
        key->subKeys[2*i  ] = A+B;    /* combine with a PHT */
        B = A + 2*B;
        key->subKeys[2*i+1] = tm_twofish_rol(B,TM_TF_SK_ROTL);
    }
#if !defined(ZERO_KEY)
    L0Ptr = (tt8BitPtr)0;
    L1Ptr = (tt8BitPtr)0;
    L0Ptr = tm_malloc(256);
    if (L0Ptr != (tt8BitPtr)0)
    {
/* case out key length for speed in generating S-boxes */
        switch (keyLen)
        {
            case 128:
            #if defined(FULL_KEY) || defined(PART_KEY)

                #define    one128(N,J)   \
                        sbSet(N,i,J,(tt32Bit)(p8(N##1)[L0Ptr[i+J]]^k0))
                #define    sb128(N) {                       \
                    Xor256(L0Ptr,p8(N##2),b##N(sKey[1]));      \
                    { register tt32Bit k0=b##N(sKey[0]);    \
                    for (i=0;i<256;i+=2) { one128(N,0); one128(N,1); } } }

            #elif defined(MIN_KEY)
                #define    sb128(N) Xor256(_sBox8_(N),p8(N##2),b##N(sKey[1]))
            #endif
                sb128(0); sb128(1); sb128(2); sb128(3);
                break;
            case 192:
            #if defined(FULL_KEY) || defined(PART_KEY)
                #define one192(N,J) \
                    sbSet(N,i,J,    \
                        (tt32Bit)(p8(N##1)[(int)(p8(N##2)[L0Ptr[i+J]]^k1)]^k0))
                #define    sb192(N) {                       \
                    Xor256(L0Ptr,p8(N##3),b##N(sKey[2]));   \
                    { register tt32Bit k0=b##N(sKey[0]);    \
                    register tt32Bit k1=b##N(sKey[1]);      \
                    for (i=0;i<256;i+=2) { one192(N,0); one192(N,1); } } }
            #elif defined(MIN_KEY)
                #define one192(N,J) \
                    sbSet(N,i,J,(tt32Bit)(p8(N##2)[L0Ptr[i+J]]^k1))
                #define    sb192(N) {                       \
                    Xor256(L0Ptr,p8(N##3),b##N(sKey[2]));   \
                    { register tt32Bit k1=b##N(sKey[1]);    \
                    for (i=0;i<256;i+=2) { one192(N,0); one192(N,1); } } }
            #endif
                sb192(0); sb192(1); sb192(2); sb192(3);
                break;
            case 256:
#if defined(FULL_KEY) || defined(PART_KEY)
#define one256(N,J) \
        sbSet(N,i,J,(tt32Bit)(p8(N##1)[(int)(p8(N##2)[L0Ptr[i+J]]^k1)]^k0))
#define sb256(N)                                                    \
        {                                                           \
            Xor256(L1Ptr,p8(N##4),b##N(sKey[3]));                   \
            for (i=0;i<256;i+=2)                                    \
            {                                                       \
                L0Ptr[i  ]=p8(N##3)[L1Ptr[i]];                      \
                L0Ptr[i+1]=p8(N##3)[L1Ptr[i+1]];                    \
            }                                                       \
            Xor256(L0Ptr,L0Ptr,b##N(sKey[2]));                      \
            {                                                       \
                register tt32Bit k0=b##N(sKey[0]);                  \
                register tt32Bit k1=b##N(sKey[1]);                  \
                for (i=0;i<256;i+=2)                                \
                {                                                   \
                    one256(N,0);                                    \
                    one256(N,1);                                    \
                }                                                   \
            }                                                       \
        }
#elif defined(MIN_KEY)
#define one256(N,J) sbSet(N,i,J,(tt32Bit)(p8(N##2)[L0Ptr[i+J]]^k1))
#define sb256(N)                                                    \
        {                                                           \
            Xor256(L1Ptr,p8(N##4),b##N(sKey[3]));                   \
            for (i=0;i<256;i+=2)                                    \
            {                                                       \
                L0Ptr[i  ]=p8(N##3)[L1Ptr[i]];                      \
                L0Ptr[i+1]=p8(N##3)[L1Ptr[i+1]];                    \
            }                                                       \
            Xor256(L0Ptr,L0Ptr,b##N(sKey[2]));                      \
            {                                                       \
                register tt32Bit k1=b##N(sKey[1]);                  \
                for (i=0;i<256;i+=2)                                \
                {                                                   \
                    one256(N,0);                                    \
                    one256(N,1);                                    \
                }                                                   \
            }                                                       \
        }
#endif
                L1Ptr = tm_malloc(256);
                if (L1Ptr != (tt8BitPtr)0)
                {
                    sb256(0); sb256(1);    sb256(2); sb256(3);
                    tm_free(L1Ptr);
                }
                else
                {
                    errorCode = TM_ENOBUFS;
                }
                break;
            default:
                break;
        }
        tm_free(L0Ptr);
    }
    else
    {
        errorCode = TM_ENOBUFS;
    }
#endif


    if (errorCode == TM_ENOERROR)
    {

        if (key->direction == TM_ESP_ENCRYPT)
        {
    /* reverse the round subkey order */
            tfTfReverseRoundSubkeys(key,TM_ESP_ENCRYPT);
        }
    }

    return errorCode;
}
/*
+*****************************************************************************
*
* Function Name:    makeKey
*
* Function:            Initialize the Twofish key schedule
*
* Arguments:        key  =    ptr to ttTwofishKeyInstance to be initialized
*                   direction   = TM_ESP_ENCRYPT or TM_ESP_DECRYPT
*                   keyLen      = # bits of key text at *keyMaterial
*                   keyMaterial = ptr to hex ASCII chars representing key bits
*
* Return:            TM_ENOERROR on success
*                    else error code (e.g., TM_EINVAL)
*
* Notes: This parses the key bits from keyMaterial.  Zeroes out unused key bits
*
-****************************************************************************/
int tfTwofishMakeKey(ttTwofishKeyInstancePtr key, 
                     tt8Bit direction, 
                     int keyLen,
                     tt8BitPtr keyMaterial)
{
    int retCode;

#if VALIDATE_PARMS                /* first, sanity check on parameters */
    if (key == (ttTwofishKeyInstancePtr)0 )            
        return TM_EINVAL;/* must have a ttTwofishKeyInstance to initialize */
    if ((direction != TM_ESP_ENCRYPT) && (direction != TM_ESP_DECRYPT))
        return TM_EINVAL;        /* must have valid direction */
    if ((keyLen > TM_TWOFISH_MAX_KEY_BITS) || (keyLen < 8) || (keyLen & 0x3F))
        return TM_EINVAL;        /* length must be valid */
    key->keySig = TM_TWOFISH_VALID_SIG;    /* show that we are initialized */
  #if TM_TWOFISH_ALIGN32
    if ((((int)key) & 3) || (((int)key->key32) & 3))
        return TM_ETWOFISHBADALIGN32;
  #endif
#endif

    key->direction    = direction;/* set our cipher direction */
    key->keyLen        = (keyLen+63) & ~63;  /* round up to multiple of 64 */
    key->numRounds    = numRounds[(keyLen-1)/64];
    tm_bzero(key->key32,sizeof(key->key32));    /* zero unused bits */
    key->keyMaterial[TM_TWOFISH_MAX_KEY_SIZE]=0;/* terminate ASCII string */

    if (keyMaterial == TM_8BIT_NULL_PTR)
        return TM_ENOERROR;            /* allow a "dummy" call */
        
    retCode = tfTfParseHexDword(keyLen,keyMaterial,key->key32,key->keyMaterial);
    if (retCode)
        return TM_EINVAL;    

    return tfTwofishReKey(key);            /* generate round subkeys */
}


/*
+*****************************************************************************
*
* Function Name:    blockEncrypt
*
* Function:            Encrypt block(s) of data using Twofish
*
* Arguments:         key = ptr to already initialized ttTwofishKeyInstance
*                    input    = ptr to data blocks to be encrypted
*                    inputLen = # bits to encrypt (multiple of blockSize)
*                    outBuffer= ptr to where to put encrypted blocks
*
* Return:            # bits ciphered (>= 0)
*                    else error code (e.g., TM_EINVALIDKEYR, BAD_KEY_MATERIAL)
*
* Notes: The only supported block size for ECB/CBC modes is
    TM_TWOFISH_BLOCK_SIZE bits. If inputLen is not a multiple of
    TM_TWOFISH_BLOCK_SIZE bits in those modes, an error
    TM_ETWOFISHBADINPUTLEN is returned.  In CFB1 mode, all block sizes can be
    supported.
*
-****************************************************************************/

int tfTwofishBlockEncrypt(ttTwofishKeyInstancePtr key,ttConst8BitPtr input,
                int inputLen, tt8BitPtr outBuffer)
{
    tt8BitPtr       temp8BitPtr;
    ttConst8BitPtr  tempConst8BitPtr;
/* make local copies of things for faster access */
    tt32Bit         sk[TM_TWOFISH_TOTAL_SUBKEYS];
    tt32Bit         x[TM_TWOFISH_BLOCK_SIZE/32]; /* block being encrypted */
    tt32Bit         t0, t1;                      /* temp variables */
    int             rounds=key->numRounds;       /* number of rounds */
    int             n;                           /* loop counters */
#ifdef ZERO_KEY
    int             i;
#endif /* ZERO_KEY */

    GetSboxKey;

#if VALIDATE_PARMS
    if ((key == (ttTwofishKeyInstancePtr) 0) || \
        (key->keySig != TM_TWOFISH_VALID_SIG))
        return TM_EINVAL;
    if ((rounds < 2) || (rounds > TM_TWOFISH_MAX_ROUNDS) || (rounds&1))
        return TM_EINVAL;
    if (inputLen != TM_TWOFISH_BLOCK_SIZE)
        return TM_ETWOFISHBADINPUTLEN;
  #if TM_TWOFISH_ALIGN32
/* it is possible that input and outBuffer are not 32-bit aligned, because
   of TCP scattered buffers - the TCP data segment may not start on a 32-bit
   boundary if TCP data we sent has already been ACK'ed */
    if  ( (((int)key      ) & 3) )
        return TM_ETWOFISHBADALIGN32;
  #endif
#endif

    /* here for ECB, CBC modes */
    if (key->direction != TM_ESP_ENCRYPT)
    {
/* reverse the round subkey order */
        tfTfReverseRoundSubkeys(key,TM_ESP_ENCRYPT);
    }

    /* make local copy of subkeys for speed */
    tm_bcopy(key->subKeys,
        sk,
        (int)sizeof(tt32Bit)*(TM_TWOFISH_ROUND_SUBKEYS+2*rounds));

    for (n=0;n<inputLen;
         n+=TM_TWOFISH_BLOCK_SIZE,
         input+=TM_TWOFISH_BLOCK_SIZE/8,
         outBuffer+=TM_TWOFISH_BLOCK_SIZE/8)
    {

#define LoadBlockE(N)                                     \
        {                                                 \
            tempConst8BitPtr = (ttConst8BitPtr) (input + (N << 2)); \
            t0 = (((tt32Bit) tempConst8BitPtr[3] << 24)   \
                  | ((tt32Bit) tempConst8BitPtr[2] << 16) \
                  | ((tt32Bit) tempConst8BitPtr[1] <<  8) \
                  | ((tt32Bit) tempConst8BitPtr[0]));     \
            x[N]= t0 ^ sk[TM_TWOFISH_INPUT_WHITEN+N];     \
        }

        LoadBlockE(0);
        LoadBlockE(1);    
        LoadBlockE(2);    
        LoadBlockE(3);

#define    EncryptRound(K,R,id)                              \
        {                                                    \
            t0       = Fe32##id(x[K  ],0);                   \
            t1       = Fe32##id(x[K^1],3);                   \
            x[K^3] = tm_twofish_rol(x[K^3],1);                            \
            x[K^2]^= t0 +   t1 + sk[TM_TWOFISH_ROUND_SUBKEYS+2*(R)  ];    \
            x[K^3]^= t0 + 2*t1 + sk[TM_TWOFISH_ROUND_SUBKEYS+2*(R)+1];    \
            x[K^2] = tm_twofish_ror(x[K^2],1);                            \
        } \


#define        Encrypt2(R,id)   \
        { EncryptRound(0,R+1,id); EncryptRound(2,R,id); }

#ifdef ZERO_KEY
        switch (key->keyLen)
        {
            case 128:
                for (i=rounds-2;i>=0;i-=2)
                    Encrypt2(i, _128);
                break;
            case 192:
                for (i=rounds-2;i>=0;i-=2)
                    Encrypt2(i, _192);
                break;
            case 256:
                for (i=rounds-2;i>=0;i-=2)
                    Encrypt2(i, _256);
                break;
        }
#else /* ZERO_KEY */
        Encrypt2(14, _0);
        Encrypt2(12, _0);
        Encrypt2(10, _0);
        Encrypt2( 8, _0);
        Encrypt2( 6, _0);
        Encrypt2( 4, _0);
        Encrypt2( 2, _0);
        Encrypt2( 0, _0);
#endif /* ZERO_KEY */

        /* need to do (or undo, depending on your point of view) final swap */
#define    StoreBlockE(N)                               \
        {                                               \
            temp8BitPtr = outBuffer + (N << 2);         \
            t0=x[N^2] ^ sk[TM_TWOFISH_OUTPUT_WHITEN+N]; \
            temp8BitPtr[3] = (tt8Bit)(t0 >> 24);        \
            temp8BitPtr[2] = (tt8Bit)(t0 >> 16);        \
            temp8BitPtr[1] = (tt8Bit)(t0 >>  8);        \
            temp8BitPtr[0] = (tt8Bit)t0;                \
        }

        StoreBlockE(0);    
        StoreBlockE(1);    
        StoreBlockE(2);    
        StoreBlockE(3);
    }


    return inputLen;
}

/*
+*****************************************************************************
*
* Function Name:    blockDecrypt
*
* Function:            Decrypt block(s) of data using Twofish
*
* Arguments:        
*                    key  =    ptr to already initialized ttTwofishKeyInstance
*                    input        =    ptr to data blocks to be decrypted
*                    inputLen    =    # bits to encrypt (multiple of blockSize)
*                    outBuffer    =    ptr to where to put decrypted blocks
*
* Return:            # bits ciphered (>= 0)
*                    else error code (e.g., TM_EINVAL, BAD_KEY_MATERIAL)
*
* Notes: The only supported block size for ECB/CBC modes is
    TM_TWOFISH_BLOCK_SIZE bits. If inputLen is not a multiple of
    TM_TWOFISH_BLOCK_SIZE bits in those modes, an error
    TM_ETWOFISHBADINPUTLEN is returned.  In CFB1 mode, all block sizes can be
    supported.
*
-****************************************************************************/
int tfTwofishBlockDecrypt(ttTwofishKeyInstancePtr key,
                          ttConst8BitPtr input,
                          int inputLen, 
                          tt8BitPtr outBuffer)
{
/* make local copies of things for faster access */
    tt32Bit sk[TM_TWOFISH_TOTAL_SUBKEYS];
    tt32Bit x[TM_TWOFISH_BLOCK_SIZE/32];    /* block being encrypted */
    tt32Bit t0,t1;                          /* temp variables */
    int     rounds=key->numRounds;          /* number of rounds */
    int     n;                              /* loop counters */
#ifdef ZERO_KEY
    int     i;
#endif /* ZERO_KEY */

    GetSboxKey;

#if VALIDATE_PARMS
    if ((key == (ttTwofishKeyInstancePtr)0) || \
        (key->keySig != TM_TWOFISH_VALID_SIG))
        return TM_EINVAL;
    if ((rounds < 2) || (rounds > TM_TWOFISH_MAX_ROUNDS) || (rounds&1))
        return TM_EINVAL;
    if (inputLen != TM_TWOFISH_BLOCK_SIZE)
        return TM_ETWOFISHBADINPUTLEN;
  #if TM_TWOFISH_ALIGN32
    if ( (((int)key      ) & 3) ||
         (((int)input)  & 3) || (((int)outBuffer) & 3))
        return TM_ETWOFISHBADALIGN32;
  #endif
#endif

    /* here for ECB, CBC modes */
    if (key->direction != TM_ESP_DECRYPT)
    {
/* reverse the round subkey order */
        tfTfReverseRoundSubkeys(key,TM_ESP_DECRYPT);
    }

    /* make local copy of subkeys for speed */
    tm_bcopy(key->subKeys,
             sk,
             (int)sizeof(tt32Bit)*(TM_TWOFISH_ROUND_SUBKEYS+2*rounds));

    for (n=0;
         n<inputLen;
         n+=TM_TWOFISH_BLOCK_SIZE,
         input+=TM_TWOFISH_BLOCK_SIZE/8,
         outBuffer+=TM_TWOFISH_BLOCK_SIZE/8)
    {
#define LoadBlockD(N) x[N^2]=tm_twofish_bswap(((tt32BitPtr)input)[N]) ^ \
             sk[TM_TWOFISH_OUTPUT_WHITEN+N]

        LoadBlockD(0);    
        LoadBlockD(1);    
        LoadBlockD(2);    
        LoadBlockD(3);

#define    DecryptRound(K,R,id)                                \
            t0       = Fe32##id(x[K  ],0);                    \
            t1       = Fe32##id(x[K^1],3);                    \
            x[K^2] = tm_twofish_rol (x[K^2],1);                        \
            x[K^2]^= t0 +   t1 + sk[TM_TWOFISH_ROUND_SUBKEYS+2*(R)  ];    \
            x[K^3]^= t0 + 2*t1 + sk[TM_TWOFISH_ROUND_SUBKEYS+2*(R)+1];    \
            x[K^3] = tm_twofish_ror (x[K^3],1);                        \

#define  Decrypt2(R,id)    \
        { \
            DecryptRound(2,R+1,id); \
            DecryptRound(0,R,id);  \
        }

#if defined(ZERO_KEY)
        switch (key->keyLen)
        {
            case 128:
                for (i=rounds-2;i>=0;i-=2)
                    Decrypt2(i, _128);
                break;
            case 192:
                for (i=rounds-2;i>=0;i-=2)
                    Decrypt2(i, _192);
                break;
            case 256:
                for (i=rounds-2;i>=0;i-=2)
                    Decrypt2(i, _256);
                break;
        }
#else
        {
        Decrypt2(14, _0);
        Decrypt2(12, _0);
        Decrypt2(10, _0);
        Decrypt2( 8, _0);
        Decrypt2( 6, _0);
        Decrypt2( 4, _0);
        Decrypt2( 2, _0);
        Decrypt2( 0, _0);
        }
#endif

        {
#ifdef TM_LITTLE_ENDIAN
#define    StoreBlockD(N)    \
            ((tt32BitPtr)outBuffer)[N] = x[N] ^ sk[TM_TWOFISH_INPUT_WHITEN+N]
#else
#define    StoreBlockD(N)    \
            { \
               t0=x[N]^sk[TM_TWOFISH_INPUT_WHITEN+N]; \
               ((tt32BitPtr)outBuffer)[N] = tm_twofish_bswap(t0); \
            }
#endif
            StoreBlockD(0);    
            StoreBlockD(1);    
            StoreBlockD(2);    
            StoreBlockD(3);
#undef  StoreBlockD

            continue;
        }
    }

    return inputLen;
}

/*-----------------------  esp-twofish ----------------------- */

/*
 * twofish actually do not use assymetric scheduled keys, however, AES C API
 * suggests assymetric key setup.
 */
int tfEspTwofishSchedLen(void)
{

    return sizeof(ttTwofishKeyInstance) * 2;
}

int tfEspTwofishSchedule(
    ttEspAlgorithmPtr algoPtr, ttGenericKeyPtr gkeyPtr)
{
    ttTwofishKeyInstancePtr   k;
    int                       retCode;

    TM_UNREF_IN_ARG(algoPtr);
    k = (ttTwofishKeyInstancePtr)gkeyPtr->keySchedulePtr;
    retCode = tfTwofishMakeKey(&k[0], TM_ESP_DECRYPT, (int)gkeyPtr->keyBits ,
                               gkeyPtr->keyDataPtr);
    if (retCode != TM_ENOERROR)
        return TM_EINVAL;
    retCode = tfTwofishMakeKey(&k[1], TM_ESP_ENCRYPT, (int)gkeyPtr->keyBits ,
                               gkeyPtr->keyDataPtr);
    if (retCode != TM_ENOERROR)
        return TM_EINVAL;
    return TM_ENOERROR;
}






int tfEspTwofishBlockDecrypt(ttEspAlgorithmPtr algoPtr,
                             ttGenericKeyPtr   gkeyPtr,
                             tt8BitPtr         s,
                             tt8BitPtr         d)
{
    ttTwofishKeyInstancePtr   p;
    int                       retCode;

/* does not take advantage of CBC mode support */
    p = (ttTwofishKeyInstancePtr)gkeyPtr->keySchedulePtr;
    retCode = tfTwofishBlockDecrypt(&p[0], s, algoPtr->ealgBlockSize * 8, d);
    if (retCode != TM_ENOERROR)
    {
        return TM_EINVAL;
    }
    return TM_ENOERROR;
}

int tfEspTwofishBlockEncrypt( ttEspAlgorithmPtr algoPtr,
                              ttGenericKeyPtr   gkeyPtr,
                              tt8BitPtr         s,
                              tt8BitPtr         d)
{
    ttTwofishKeyInstancePtr   p;
    int                       retCode;

/* does not take advantage of CBC mode support */
    p = (ttTwofishKeyInstancePtr)gkeyPtr->keySchedulePtr;
    retCode = tfTwofishBlockEncrypt(&p[1], s, algoPtr->ealgBlockSize * 8, d);
    if (retCode != TM_ENOERROR)
    {
        return TM_EINVAL;
    }
    return TM_ENOERROR;
}



#else /* ! TM_USE_TWOFISH */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_TWOFISH is not defined */
int tlTwofshDummy = 0;

#endif /* TM_USE_TWOFISH */
