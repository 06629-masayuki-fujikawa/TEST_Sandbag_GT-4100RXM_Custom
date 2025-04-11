/*
 * Description: Testing the ESP algorithem: blowfish algorithm
 *
 * Filename: trblwfsh.c
 * Author: Jin Zhang
 * Date Created: 08/01/2001
 * $Source: source/cryptlib/trblwfsh.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:35:21JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


/*  $KAME: bf_enc.c,v 1.5 2000/09/18 21:21:19 itojun Exp $  */
/* crypto/bf/bf_enc.c */
/* Copyright (C) 1995-1997 Eric Young (eay@mincom.oz.au)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@mincom.oz.au).
 * The implementation was written so as to conform with Netscapes SSL.
 *
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@mincom.oz.au).
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
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
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@mincom.oz.au)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@mincom.oz.au)"
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

#ifdef TM_USE_BLOWFISH
#include <trcrylib.h>

/* This is actually a big endian algorithm, the most significate byte
 * is used to lookup array 0 */

/* use BF_PTR2 for intel boxes,
 * BF_PTR for sparc and MIPS/SGI
 * use nothing for Alpha and HP.
 */

#undef    BF_PTR
#undef    BF_PTR2


#define BF_M    0x3fc
#define BF_0    22
#define BF_1    14
#define BF_2    6
#define BF_3    2 /* left shift */

#if defined(BF_PTR2)

/* This is basically a special pentium verson */
#define tm_blowfish_enc(LL,R,S,P) \
    { \
    tt32Bit t,u,v; \
    u=R>>(unsigned)BF_0; \
    v=R>>(unsigned)BF_1; \
    u&=BF_M; \
    v&=BF_M; \
    t=  *(tt32BitPtr)((tt8BitPtr)&(S[  0])+u); \
    u=R>>(unsigned)BF_2; \
    t+= *(tt32BitPtr)((tt8BitPtr)&(S[256])+v); \
    v=R<<(unsigned)BF_3; \
    u&=BF_M; \
    v&=BF_M; \
    t^= *(tt32BitPtr)((tt8BitPtr)&(S[512])+u); \
    LL^=P; \
    t+= *(tt32BitPtr)((tt8BitPtr)&(S[768])+v); \
    LL^=t; \
    }

#elif defined(BF_PTR)

/* This is normally very good */

#define tm_blowfish_enc(LL,R,S,P) \
    LL^=P; \
    LL^= (((*(tt32BitPtr)((tt8BitPtr)&(S[  0])+((R>>(unsigned)BF_0)&BF_M))+ \
        *(tt32BitPtr)((tt8BitPtr)&(S[256])+((R>>(unsigned)BF_1)&BF_M)))^ \
        *(tt32BitPtr)((tt8BitPtr)&(S[512])+((R>>(unsigned)BF_2)&BF_M)))+ \
        *(tt32BitPtr)((tt8BitPtr)&(S[768])+((R<<(unsigned)BF_3)&BF_M)));
#else

/* This will always work, even on 64 bit machines and strangly enough,
 * on the Alpha it is faster than the pointer versions (both 32 and 64
 * versions of tt32Bit) */

#define tm_blowfish_enc(LL,R,S,P) \
    LL^=P; \
    LL^=(tt32Bit) (((    S[ (tt32Bit)       (R>>(unsigned)24)      ] +   \
                         S[(tt32Bit)(0x0100+((R>>(unsigned)16)&0xff))])^ \
                    S[(tt32Bit)(0x0200+((R>>(unsigned) 8)&0xff))])+      \
                   S[(tt32Bit)(0x0300+((R     )&0xff))]);
#endif

static const ttBlowfishKey TM_CONST_QLF bfInit= {
    {
    TM_UL(0x243f6a88), TM_UL(0x85a308d3), TM_UL(0x13198a2e), TM_UL(0x03707344),
    TM_UL(0xa4093822), TM_UL(0x299f31d0), TM_UL(0x082efa98), TM_UL(0xec4e6c89),
    TM_UL(0x452821e6), TM_UL(0x38d01377), TM_UL(0xbe5466cf), TM_UL(0x34e90c6c),
    TM_UL(0xc0ac29b7), TM_UL(0xc97c50dd), TM_UL(0x3f84d5b5), TM_UL(0xb5470917),
    TM_UL(0x9216d5d9), TM_UL(0x8979fb1b)
    },{
    TM_UL(0xd1310ba6), TM_UL(0x98dfb5ac), TM_UL(0x2ffd72db), TM_UL(0xd01adfb7),
    TM_UL(0xb8e1afed), TM_UL(0x6a267e96), TM_UL(0xba7c9045), TM_UL(0xf12c7f99),
    TM_UL(0x24a19947), TM_UL(0xb3916cf7), TM_UL(0x0801f2e2), TM_UL(0x858efc16),
    TM_UL(0x636920d8), TM_UL(0x71574e69), TM_UL(0xa458fea3), TM_UL(0xf4933d7e),
    TM_UL(0x0d95748f), TM_UL(0x728eb658), TM_UL(0x718bcd58), TM_UL(0x82154aee),
    TM_UL(0x7b54a41d), TM_UL(0xc25a59b5), TM_UL(0x9c30d539), TM_UL(0x2af26013),
    TM_UL(0xc5d1b023), TM_UL(0x286085f0), TM_UL(0xca417918), TM_UL(0xb8db38ef),
    TM_UL(0x8e79dcb0), TM_UL(0x603a180e), TM_UL(0x6c9e0e8b), TM_UL(0xb01e8a3e),
    TM_UL(0xd71577c1), TM_UL(0xbd314b27), TM_UL(0x78af2fda), TM_UL(0x55605c60),
    TM_UL(0xe65525f3), TM_UL(0xaa55ab94), TM_UL(0x57489862), TM_UL(0x63e81440),
    TM_UL(0x55ca396a), TM_UL(0x2aab10b6), TM_UL(0xb4cc5c34), TM_UL(0x1141e8ce),
    TM_UL(0xa15486af), TM_UL(0x7c72e993), TM_UL(0xb3ee1411), TM_UL(0x636fbc2a),
    TM_UL(0x2ba9c55d), TM_UL(0x741831f6), TM_UL(0xce5c3e16), TM_UL(0x9b87931e),
    TM_UL(0xafd6ba33), TM_UL(0x6c24cf5c), TM_UL(0x7a325381), TM_UL(0x28958677),
    TM_UL(0x3b8f4898), TM_UL(0x6b4bb9af), TM_UL(0xc4bfe81b), TM_UL(0x66282193),
    TM_UL(0x61d809cc), TM_UL(0xfb21a991), TM_UL(0x487cac60), TM_UL(0x5dec8032),
    TM_UL(0xef845d5d), TM_UL(0xe98575b1), TM_UL(0xdc262302), TM_UL(0xeb651b88),
    TM_UL(0x23893e81), TM_UL(0xd396acc5), TM_UL(0x0f6d6ff3), TM_UL(0x83f44239),
    TM_UL(0x2e0b4482), TM_UL(0xa4842004), TM_UL(0x69c8f04a), TM_UL(0x9e1f9b5e),
    TM_UL(0x21c66842), TM_UL(0xf6e96c9a), TM_UL(0x670c9c61), TM_UL(0xabd388f0),
    TM_UL(0x6a51a0d2), TM_UL(0xd8542f68), TM_UL(0x960fa728), TM_UL(0xab5133a3),
    TM_UL(0x6eef0b6c), TM_UL(0x137a3be4), TM_UL(0xba3bf050), TM_UL(0x7efb2a98),
    TM_UL(0xa1f1651d), TM_UL(0x39af0176), TM_UL(0x66ca593e), TM_UL(0x82430e88),
    TM_UL(0x8cee8619), TM_UL(0x456f9fb4), TM_UL(0x7d84a5c3), TM_UL(0x3b8b5ebe),
    TM_UL(0xe06f75d8), TM_UL(0x85c12073), TM_UL(0x401a449f), TM_UL(0x56c16aa6),
    TM_UL(0x4ed3aa62), TM_UL(0x363f7706), TM_UL(0x1bfedf72), TM_UL(0x429b023d),
    TM_UL(0x37d0d724), TM_UL(0xd00a1248), TM_UL(0xdb0fead3), TM_UL(0x49f1c09b),
    TM_UL(0x075372c9), TM_UL(0x80991b7b), TM_UL(0x25d479d8), TM_UL(0xf6e8def7),
    TM_UL(0xe3fe501a), TM_UL(0xb6794c3b), TM_UL(0x976ce0bd), TM_UL(0x04c006ba),
    TM_UL(0xc1a94fb6), TM_UL(0x409f60c4), TM_UL(0x5e5c9ec2), TM_UL(0x196a2463),
    TM_UL(0x68fb6faf), TM_UL(0x3e6c53b5), TM_UL(0x1339b2eb), TM_UL(0x3b52ec6f),
    TM_UL(0x6dfc511f), TM_UL(0x9b30952c), TM_UL(0xcc814544), TM_UL(0xaf5ebd09),
    TM_UL(0xbee3d004), TM_UL(0xde334afd), TM_UL(0x660f2807), TM_UL(0x192e4bb3),
    TM_UL(0xc0cba857), TM_UL(0x45c8740f), TM_UL(0xd20b5f39), TM_UL(0xb9d3fbdb),
    TM_UL(0x5579c0bd), TM_UL(0x1a60320a), TM_UL(0xd6a100c6), TM_UL(0x402c7279),
    TM_UL(0x679f25fe), TM_UL(0xfb1fa3cc), TM_UL(0x8ea5e9f8), TM_UL(0xdb3222f8),
    TM_UL(0x3c7516df), TM_UL(0xfd616b15), TM_UL(0x2f501ec8), TM_UL(0xad0552ab),
    TM_UL(0x323db5fa), TM_UL(0xfd238760), TM_UL(0x53317b48), TM_UL(0x3e00df82),
    TM_UL(0x9e5c57bb), TM_UL(0xca6f8ca0), TM_UL(0x1a87562e), TM_UL(0xdf1769db),
    TM_UL(0xd542a8f6), TM_UL(0x287effc3), TM_UL(0xac6732c6), TM_UL(0x8c4f5573),
    TM_UL(0x695b27b0), TM_UL(0xbbca58c8), TM_UL(0xe1ffa35d), TM_UL(0xb8f011a0),
    TM_UL(0x10fa3d98), TM_UL(0xfd2183b8), TM_UL(0x4afcb56c), TM_UL(0x2dd1d35b),
    TM_UL(0x9a53e479), TM_UL(0xb6f84565), TM_UL(0xd28e49bc), TM_UL(0x4bfb9790),
    TM_UL(0xe1ddf2da), TM_UL(0xa4cb7e33), TM_UL(0x62fb1341), TM_UL(0xcee4c6e8),
    TM_UL(0xef20cada), TM_UL(0x36774c01), TM_UL(0xd07e9efe), TM_UL(0x2bf11fb4),
    TM_UL(0x95dbda4d), TM_UL(0xae909198), TM_UL(0xeaad8e71), TM_UL(0x6b93d5a0),
    TM_UL(0xd08ed1d0), TM_UL(0xafc725e0), TM_UL(0x8e3c5b2f), TM_UL(0x8e7594b7),
    TM_UL(0x8ff6e2fb), TM_UL(0xf2122b64), TM_UL(0x8888b812), TM_UL(0x900df01c),
    TM_UL(0x4fad5ea0), TM_UL(0x688fc31c), TM_UL(0xd1cff191), TM_UL(0xb3a8c1ad),
    TM_UL(0x2f2f2218), TM_UL(0xbe0e1777), TM_UL(0xea752dfe), TM_UL(0x8b021fa1),
    TM_UL(0xe5a0cc0f), TM_UL(0xb56f74e8), TM_UL(0x18acf3d6), TM_UL(0xce89e299),
    TM_UL(0xb4a84fe0), TM_UL(0xfd13e0b7), TM_UL(0x7cc43b81), TM_UL(0xd2ada8d9),
    TM_UL(0x165fa266), TM_UL(0x80957705), TM_UL(0x93cc7314), TM_UL(0x211a1477),
    TM_UL(0xe6ad2065), TM_UL(0x77b5fa86), TM_UL(0xc75442f5), TM_UL(0xfb9d35cf),
    TM_UL(0xebcdaf0c), TM_UL(0x7b3e89a0), TM_UL(0xd6411bd3), TM_UL(0xae1e7e49),
    TM_UL(0x00250e2d), TM_UL(0x2071b35e), TM_UL(0x226800bb), TM_UL(0x57b8e0af),
    TM_UL(0x2464369b), TM_UL(0xf009b91e), TM_UL(0x5563911d), TM_UL(0x59dfa6aa),
    TM_UL(0x78c14389), TM_UL(0xd95a537f), TM_UL(0x207d5ba2), TM_UL(0x02e5b9c5),
    TM_UL(0x83260376), TM_UL(0x6295cfa9), TM_UL(0x11c81968), TM_UL(0x4e734a41),
    TM_UL(0xb3472dca), TM_UL(0x7b14a94a), TM_UL(0x1b510052), TM_UL(0x9a532915),
    TM_UL(0xd60f573f), TM_UL(0xbc9bc6e4), TM_UL(0x2b60a476), TM_UL(0x81e67400),
    TM_UL(0x08ba6fb5), TM_UL(0x571be91f), TM_UL(0xf296ec6b), TM_UL(0x2a0dd915),
    TM_UL(0xb6636521), TM_UL(0xe7b9f9b6), TM_UL(0xff34052e), TM_UL(0xc5855664),
    TM_UL(0x53b02d5d), TM_UL(0xa99f8fa1), TM_UL(0x08ba4799), TM_UL(0x6e85076a),
    TM_UL(0x4b7a70e9), TM_UL(0xb5b32944), TM_UL(0xdb75092e), TM_UL(0xc4192623),
    TM_UL(0xad6ea6b0), TM_UL(0x49a7df7d), TM_UL(0x9cee60b8), TM_UL(0x8fedb266),
    TM_UL(0xecaa8c71), TM_UL(0x699a17ff), TM_UL(0x5664526c), TM_UL(0xc2b19ee1),
    TM_UL(0x193602a5), TM_UL(0x75094c29), TM_UL(0xa0591340), TM_UL(0xe4183a3e),
    TM_UL(0x3f54989a), TM_UL(0x5b429d65), TM_UL(0x6b8fe4d6), TM_UL(0x99f73fd6),
    TM_UL(0xa1d29c07), TM_UL(0xefe830f5), TM_UL(0x4d2d38e6), TM_UL(0xf0255dc1),
    TM_UL(0x4cdd2086), TM_UL(0x8470eb26), TM_UL(0x6382e9c6), TM_UL(0x021ecc5e),
    TM_UL(0x09686b3f), TM_UL(0x3ebaefc9), TM_UL(0x3c971814), TM_UL(0x6b6a70a1),
    TM_UL(0x687f3584), TM_UL(0x52a0e286), TM_UL(0xb79c5305), TM_UL(0xaa500737),
    TM_UL(0x3e07841c), TM_UL(0x7fdeae5c), TM_UL(0x8e7d44ec), TM_UL(0x5716f2b8),
    TM_UL(0xb03ada37), TM_UL(0xf0500c0d), TM_UL(0xf01c1f04), TM_UL(0x0200b3ff),
    TM_UL(0xae0cf51a), TM_UL(0x3cb574b2), TM_UL(0x25837a58), TM_UL(0xdc0921bd),
    TM_UL(0xd19113f9), TM_UL(0x7ca92ff6), TM_UL(0x94324773), TM_UL(0x22f54701),
    TM_UL(0x3ae5e581), TM_UL(0x37c2dadc), TM_UL(0xc8b57634), TM_UL(0x9af3dda7),
    TM_UL(0xa9446146), TM_UL(0x0fd0030e), TM_UL(0xecc8c73e), TM_UL(0xa4751e41),
    TM_UL(0xe238cd99), TM_UL(0x3bea0e2f), TM_UL(0x3280bba1), TM_UL(0x183eb331),
    TM_UL(0x4e548b38), TM_UL(0x4f6db908), TM_UL(0x6f420d03), TM_UL(0xf60a04bf),
    TM_UL(0x2cb81290), TM_UL(0x24977c79), TM_UL(0x5679b072), TM_UL(0xbcaf89af),
    TM_UL(0xde9a771f), TM_UL(0xd9930810), TM_UL(0xb38bae12), TM_UL(0xdccf3f2e),
    TM_UL(0x5512721f), TM_UL(0x2e6b7124), TM_UL(0x501adde6), TM_UL(0x9f84cd87),
    TM_UL(0x7a584718), TM_UL(0x7408da17), TM_UL(0xbc9f9abc), TM_UL(0xe94b7d8c),
    TM_UL(0xec7aec3a), TM_UL(0xdb851dfa), TM_UL(0x63094366), TM_UL(0xc464c3d2),
    TM_UL(0xef1c1847), TM_UL(0x3215d908), TM_UL(0xdd433b37), TM_UL(0x24c2ba16),
    TM_UL(0x12a14d43), TM_UL(0x2a65c451), TM_UL(0x50940002), TM_UL(0x133ae4dd),
    TM_UL(0x71dff89e), TM_UL(0x10314e55), TM_UL(0x81ac77d6), TM_UL(0x5f11199b),
    TM_UL(0x043556f1), TM_UL(0xd7a3c76b), TM_UL(0x3c11183b), TM_UL(0x5924a509),
    TM_UL(0xf28fe6ed), TM_UL(0x97f1fbfa), TM_UL(0x9ebabf2c), TM_UL(0x1e153c6e),
    TM_UL(0x86e34570), TM_UL(0xeae96fb1), TM_UL(0x860e5e0a), TM_UL(0x5a3e2ab3),
    TM_UL(0x771fe71c), TM_UL(0x4e3d06fa), TM_UL(0x2965dcb9), TM_UL(0x99e71d0f),
    TM_UL(0x803e89d6), TM_UL(0x5266c825), TM_UL(0x2e4cc978), TM_UL(0x9c10b36a),
    TM_UL(0xc6150eba), TM_UL(0x94e2ea78), TM_UL(0xa5fc3c53), TM_UL(0x1e0a2df4),
    TM_UL(0xf2f74ea7), TM_UL(0x361d2b3d), TM_UL(0x1939260f), TM_UL(0x19c27960),
    TM_UL(0x5223a708), TM_UL(0xf71312b6), TM_UL(0xebadfe6e), TM_UL(0xeac31f66),
    TM_UL(0xe3bc4595), TM_UL(0xa67bc883), TM_UL(0xb17f37d1), TM_UL(0x018cff28),
    TM_UL(0xc332ddef), TM_UL(0xbe6c5aa5), TM_UL(0x65582185), TM_UL(0x68ab9802),
    TM_UL(0xeecea50f), TM_UL(0xdb2f953b), TM_UL(0x2aef7dad), TM_UL(0x5b6e2f84),
    TM_UL(0x1521b628), TM_UL(0x29076170), TM_UL(0xecdd4775), TM_UL(0x619f1510),
    TM_UL(0x13cca830), TM_UL(0xeb61bd96), TM_UL(0x0334fe1e), TM_UL(0xaa0363cf),
    TM_UL(0xb5735c90), TM_UL(0x4c70a239), TM_UL(0xd59e9e0b), TM_UL(0xcbaade14),
    TM_UL(0xeecc86bc), TM_UL(0x60622ca7), TM_UL(0x9cab5cab), TM_UL(0xb2f3846e),
    TM_UL(0x648b1eaf), TM_UL(0x19bdf0ca), TM_UL(0xa02369b9), TM_UL(0x655abb50),
    TM_UL(0x40685a32), TM_UL(0x3c2ab4b3), TM_UL(0x319ee9d5), TM_UL(0xc021b8f7),
    TM_UL(0x9b540b19), TM_UL(0x875fa099), TM_UL(0x95f7997e), TM_UL(0x623d7da8),
    TM_UL(0xf837889a), TM_UL(0x97e32d77), TM_UL(0x11ed935f), TM_UL(0x16681281),
    TM_UL(0x0e358829), TM_UL(0xc7e61fd6), TM_UL(0x96dedfa1), TM_UL(0x7858ba99),
    TM_UL(0x57f584a5), TM_UL(0x1b227263), TM_UL(0x9b83c3ff), TM_UL(0x1ac24696),
    TM_UL(0xcdb30aeb), TM_UL(0x532e3054), TM_UL(0x8fd948e4), TM_UL(0x6dbc3128),
    TM_UL(0x58ebf2ef), TM_UL(0x34c6ffea), TM_UL(0xfe28ed61), TM_UL(0xee7c3c73),
    TM_UL(0x5d4a14d9), TM_UL(0xe864b7e3), TM_UL(0x42105d14), TM_UL(0x203e13e0),
    TM_UL(0x45eee2b6), TM_UL(0xa3aaabea), TM_UL(0xdb6c4f15), TM_UL(0xfacb4fd0),
    TM_UL(0xc742f442), TM_UL(0xef6abbb5), TM_UL(0x654f3b1d), TM_UL(0x41cd2105),
    TM_UL(0xd81e799e), TM_UL(0x86854dc7), TM_UL(0xe44b476a), TM_UL(0x3d816250),
    TM_UL(0xcf62a1f2), TM_UL(0x5b8d2646), TM_UL(0xfc8883a0), TM_UL(0xc1c7b6a3),
    TM_UL(0x7f1524c3), TM_UL(0x69cb7492), TM_UL(0x47848a0b), TM_UL(0x5692b285),
    TM_UL(0x095bbf00), TM_UL(0xad19489d), TM_UL(0x1462b174), TM_UL(0x23820e00),
    TM_UL(0x58428d2a), TM_UL(0x0c55f5ea), TM_UL(0x1dadf43e), TM_UL(0x233f7061),
    TM_UL(0x3372f092), TM_UL(0x8d937e41), TM_UL(0xd65fecf1), TM_UL(0x6c223bdb),
    TM_UL(0x7cde3759), TM_UL(0xcbee7460), TM_UL(0x4085f2a7), TM_UL(0xce77326e),
    TM_UL(0xa6078084), TM_UL(0x19f8509e), TM_UL(0xe8efd855), TM_UL(0x61d99735),
    TM_UL(0xa969a7aa), TM_UL(0xc50c06c2), TM_UL(0x5a04abfc), TM_UL(0x800bcadc),
    TM_UL(0x9e447a2e), TM_UL(0xc3453484), TM_UL(0xfdd56705), TM_UL(0x0e1e9ec9),
    TM_UL(0xdb73dbd3), TM_UL(0x105588cd), TM_UL(0x675fda79), TM_UL(0xe3674340),
    TM_UL(0xc5c43465), TM_UL(0x713e38d8), TM_UL(0x3d28f89e), TM_UL(0xf16dff20),
    TM_UL(0x153e21e7), TM_UL(0x8fb03d4a), TM_UL(0xe6e39f2b), TM_UL(0xdb83adf7),
    TM_UL(0xe93d5a68), TM_UL(0x948140f7), TM_UL(0xf64c261c), TM_UL(0x94692934),
    TM_UL(0x411520f7), TM_UL(0x7602d4f7), TM_UL(0xbcf46b2e), TM_UL(0xd4a20068),
    TM_UL(0xd4082471), TM_UL(0x3320f46a), TM_UL(0x43b7d4b7), TM_UL(0x500061af),
    TM_UL(0x1e39f62e), TM_UL(0x97244546), TM_UL(0x14214f74), TM_UL(0xbf8b8840),
    TM_UL(0x4d95fc1d), TM_UL(0x96b591af), TM_UL(0x70f4ddd3), TM_UL(0x66a02f45),
    TM_UL(0xbfbc09ec), TM_UL(0x03bd9785), TM_UL(0x7fac6dd0), TM_UL(0x31cb8504),
    TM_UL(0x96eb27b3), TM_UL(0x55fd3941), TM_UL(0xda2547e6), TM_UL(0xabca0a9a),
    TM_UL(0x28507825), TM_UL(0x530429f4), TM_UL(0x0a2c86da), TM_UL(0xe9b66dfb),
    TM_UL(0x68dc1462), TM_UL(0xd7486900), TM_UL(0x680ec0a4), TM_UL(0x27a18dee),
    TM_UL(0x4f3ffea2), TM_UL(0xe887ad8c), TM_UL(0xb58ce006), TM_UL(0x7af4d6b6),
    TM_UL(0xaace1e7c), TM_UL(0xd3375fec), TM_UL(0xce78a399), TM_UL(0x406b2a42),
    TM_UL(0x20fe9e35), TM_UL(0xd9f385b9), TM_UL(0xee39d7ab), TM_UL(0x3b124e8b),
    TM_UL(0x1dc9faf7), TM_UL(0x4b6d1856), TM_UL(0x26a36631), TM_UL(0xeae397b2),
    TM_UL(0x3a6efa74), TM_UL(0xdd5b4332), TM_UL(0x6841e7f7), TM_UL(0xca7820fb),
    TM_UL(0xfb0af54e), TM_UL(0xd8feb397), TM_UL(0x454056ac), TM_UL(0xba489527),
    TM_UL(0x55533a3a), TM_UL(0x20838d87), TM_UL(0xfe6ba9b7), TM_UL(0xd096954b),
    TM_UL(0x55a867bc), TM_UL(0xa1159a58), TM_UL(0xcca92963), TM_UL(0x99e1db33),
    TM_UL(0xa62a4a56), TM_UL(0x3f3125f9), TM_UL(0x5ef47e1c), TM_UL(0x9029317c),
    TM_UL(0xfdf8e802), TM_UL(0x04272f70), TM_UL(0x80bb155c), TM_UL(0x05282ce3),
    TM_UL(0x95c11548), TM_UL(0xe4c66d22), TM_UL(0x48c1133f), TM_UL(0xc70f86dc),
    TM_UL(0x07f9c9ee), TM_UL(0x41041f0f), TM_UL(0x404779a4), TM_UL(0x5d886e17),
    TM_UL(0x325f51eb), TM_UL(0xd59bc0d1), TM_UL(0xf2bcc18f), TM_UL(0x41113564),
    TM_UL(0x257b7834), TM_UL(0x602a9c60), TM_UL(0xdff8e8a3), TM_UL(0x1f636c1b),
    TM_UL(0x0e12b4c2), TM_UL(0x02e1329e), TM_UL(0xaf664fd1), TM_UL(0xcad18115),
    TM_UL(0x6b2395e0), TM_UL(0x333e92e1), TM_UL(0x3b240b62), TM_UL(0xeebeb922),
    TM_UL(0x85b2a20e), TM_UL(0xe6ba0d99), TM_UL(0xde720c8c), TM_UL(0x2da2f728),
    TM_UL(0xd0127845), TM_UL(0x95b794fd), TM_UL(0x647d0862), TM_UL(0xe7ccf5f0),
    TM_UL(0x5449a36f), TM_UL(0x877d48fa), TM_UL(0xc39dfd27), TM_UL(0xf33e8d1e),
    TM_UL(0x0a476341), TM_UL(0x992eff74), TM_UL(0x3a6f6eab), TM_UL(0xf4f8fd37),
    TM_UL(0xa812dc60), TM_UL(0xa1ebddf8), TM_UL(0x991be14c), TM_UL(0xdb6e6b0d),
    TM_UL(0xc67b5510), TM_UL(0x6d672c37), TM_UL(0x2765d43b), TM_UL(0xdcd0e804),
    TM_UL(0xf1290dc7), TM_UL(0xcc00ffa3), TM_UL(0xb5390f92), TM_UL(0x690fed0b),
    TM_UL(0x667b9ffb), TM_UL(0xcedb7d9c), TM_UL(0xa091cf0b), TM_UL(0xd9155ea3),
    TM_UL(0xbb132f88), TM_UL(0x515bad24), TM_UL(0x7b9479bf), TM_UL(0x763bd6eb),
    TM_UL(0x37392eb3), TM_UL(0xcc115979), TM_UL(0x8026e297), TM_UL(0xf42e312d),
    TM_UL(0x6842ada7), TM_UL(0xc66a2b3b), TM_UL(0x12754ccc), TM_UL(0x782ef11c),
    TM_UL(0x6a124237), TM_UL(0xb79251e7), TM_UL(0x06a1bbe6), TM_UL(0x4bfb6350),
    TM_UL(0x1a6b1018), TM_UL(0x11caedfa), TM_UL(0x3d25bdd8), TM_UL(0xe2e1c3c9),
    TM_UL(0x44421659), TM_UL(0x0a121386), TM_UL(0xd90cec6e), TM_UL(0xd5abea2a),
    TM_UL(0x64af674e), TM_UL(0xda86a85f), TM_UL(0xbebfe988), TM_UL(0x64e4c3fe),
    TM_UL(0x9dbc8057), TM_UL(0xf0f7c086), TM_UL(0x60787bf8), TM_UL(0x6003604d),
    TM_UL(0xd1fd8346), TM_UL(0xf6381fb0), TM_UL(0x7745ae04), TM_UL(0xd736fccc),
    TM_UL(0x83426b33), TM_UL(0xf01eab71), TM_UL(0xb0804187), TM_UL(0x3c005e5f),
    TM_UL(0x77a057be), TM_UL(0xbde8ae24), TM_UL(0x55464299), TM_UL(0xbf582e61),
    TM_UL(0x4e58f48f), TM_UL(0xf2ddfda2), TM_UL(0xf474ef38), TM_UL(0x8789bdc2),
    TM_UL(0x5366f9c3), TM_UL(0xc8b38e74), TM_UL(0xb475f255), TM_UL(0x46fcd9b9),
    TM_UL(0x7aeb2661), TM_UL(0x8b1ddf84), TM_UL(0x846a0e79), TM_UL(0x915f95e2),
    TM_UL(0x466e598e), TM_UL(0x20b45770), TM_UL(0x8cd55591), TM_UL(0xc902de4c),
    TM_UL(0xb90bace1), TM_UL(0xbb8205d0), TM_UL(0x11a86248), TM_UL(0x7574a99e),
    TM_UL(0xb77f19b6), TM_UL(0xe0a9dc09), TM_UL(0x662d09a1), TM_UL(0xc4324633),
    TM_UL(0xe85a1f02), TM_UL(0x09f0be8c), TM_UL(0x4a99a025), TM_UL(0x1d6efe10),
    TM_UL(0x1ab93d1d), TM_UL(0x0ba5a4df), TM_UL(0xa186f20f), TM_UL(0x2868f169),
    TM_UL(0xdcb7da83), TM_UL(0x573906fe), TM_UL(0xa1e2ce9b), TM_UL(0x4fcd7f52),
    TM_UL(0x50115e01), TM_UL(0xa70683fa), TM_UL(0xa002b5c4), TM_UL(0x0de6d027),
    TM_UL(0x9af88c27), TM_UL(0x773f8641), TM_UL(0xc3604c06), TM_UL(0x61a806b5),
    TM_UL(0xf0177a28), TM_UL(0xc0f586e0), TM_UL(0x006058aa), TM_UL(0x30dc7d62),
    TM_UL(0x11e69ed7), TM_UL(0x2338ea63), TM_UL(0x53c2dd94), TM_UL(0xc2c21634),
    TM_UL(0xbbcbee56), TM_UL(0x90bcb6de), TM_UL(0xebfc7da1), TM_UL(0xce591d76),
    TM_UL(0x6f05e409), TM_UL(0x4b7c0188), TM_UL(0x39720a3d), TM_UL(0x7c927c24),
    TM_UL(0x86e3725f), TM_UL(0x724d9db9), TM_UL(0x1ac15bb4), TM_UL(0xd39eb8fc),
    TM_UL(0xed545578), TM_UL(0x08fca5b5), TM_UL(0xd83d7cd3), TM_UL(0x4dad0fc4),
    TM_UL(0x1e50ef5e), TM_UL(0xb161e6f8), TM_UL(0xa28514d9), TM_UL(0x6c51133c),
    TM_UL(0x6fd5c7e7), TM_UL(0x56e14ec4), TM_UL(0x362abfce), TM_UL(0xddc6c837),
    TM_UL(0xd79a3234), TM_UL(0x92638212), TM_UL(0x670efa8e), TM_UL(0x406000e0),
    TM_UL(0x3a39ce37), TM_UL(0xd3faf5cf), TM_UL(0xabc27737), TM_UL(0x5ac52d1b),
    TM_UL(0x5cb0679e), TM_UL(0x4fa33742), TM_UL(0xd3822740), TM_UL(0x99bc9bbe),
    TM_UL(0xd5118e9d), TM_UL(0xbf0f7315), TM_UL(0xd62d1c7e), TM_UL(0xc700c47b),
    TM_UL(0xb78c1b6b), TM_UL(0x21a19045), TM_UL(0xb26eb1be), TM_UL(0x6a366eb4),
    TM_UL(0x5748ab2f), TM_UL(0xbc946e79), TM_UL(0xc6a376d2), TM_UL(0x6549c2c8),
    TM_UL(0x530ff8ee), TM_UL(0x468dde7d), TM_UL(0xd5730a1d), TM_UL(0x4cd04dc6),
    TM_UL(0x2939bbdb), TM_UL(0xa9ba4650), TM_UL(0xac9526e8), TM_UL(0xbe5ee304),
    TM_UL(0xa1fad5f0), TM_UL(0x6a2d519a), TM_UL(0x63ef8ce2), TM_UL(0x9a86ee22),
    TM_UL(0xc089c2b8), TM_UL(0x43242ef6), TM_UL(0xa51e03aa), TM_UL(0x9cf2d0a4),
    TM_UL(0x83c061ba), TM_UL(0x9be96a4d), TM_UL(0x8fe51550), TM_UL(0xba645bd6),
    TM_UL(0x2826a2f9), TM_UL(0xa73a3ae1), TM_UL(0x4ba99586), TM_UL(0xef5562e9),
    TM_UL(0xc72fefd3), TM_UL(0xf752f7da), TM_UL(0x3f046f69), TM_UL(0x77fa0a59),
    TM_UL(0x80e4a915), TM_UL(0x87b08601), TM_UL(0x9b09e6ad), TM_UL(0x3b3ee593),
    TM_UL(0xe990fd5a), TM_UL(0x9e34d797), TM_UL(0x2cf0b7d9), TM_UL(0x022b8b51),
    TM_UL(0x96d5ac3a), TM_UL(0x017da67d), TM_UL(0xd1cf3ed6), TM_UL(0x7c7d2d28),
    TM_UL(0x1f9f25cf), TM_UL(0xadf2b89b), TM_UL(0x5ad6b472), TM_UL(0x5a88f54c),
    TM_UL(0xe029ac71), TM_UL(0xe019a5e6), TM_UL(0x47b0acfd), TM_UL(0xed93fa9b),
    TM_UL(0xe8d3c48d), TM_UL(0x283b57cc), TM_UL(0xf8d56629), TM_UL(0x79132e28),
    TM_UL(0x785f0191), TM_UL(0xed756055), TM_UL(0xf7960e44), TM_UL(0xe3d35e8c),
    TM_UL(0x15056dd4), TM_UL(0x88f46dba), TM_UL(0x03a16125), TM_UL(0x0564f0bd),
    TM_UL(0xc3eb9e15), TM_UL(0x3c9057a2), TM_UL(0x97271aec), TM_UL(0xa93a072a),
    TM_UL(0x1b3f6d9b), TM_UL(0x1e6321f5), TM_UL(0xf59c66fb), TM_UL(0x26dcf319),
    TM_UL(0x7533d928), TM_UL(0xb155fdf5), TM_UL(0x03563482), TM_UL(0x8aba3cbb),
    TM_UL(0x28517711), TM_UL(0xc20ad9f8), TM_UL(0xabcc5167), TM_UL(0xccad925f),
    TM_UL(0x4de81751), TM_UL(0x3830dc8e), TM_UL(0x379d5862), TM_UL(0x9320f991),
    TM_UL(0xea7a90c2), TM_UL(0xfb3e7bce), TM_UL(0x5121ce64), TM_UL(0x774fbe32),
    TM_UL(0xa8b6e37e), TM_UL(0xc3293d46), TM_UL(0x48de5369), TM_UL(0x6413e680),
    TM_UL(0xa2ae0810), TM_UL(0xdd6db224), TM_UL(0x69852dfd), TM_UL(0x09072166),
    TM_UL(0xb39a460a), TM_UL(0x6445c0dd), TM_UL(0x586cdecf), TM_UL(0x1c20c8ae),
    TM_UL(0x5bbef7dd), TM_UL(0x1b588d40), TM_UL(0xccd2017f), TM_UL(0x6bb4e3bb),
    TM_UL(0xdda26a7e), TM_UL(0x3a59ff45), TM_UL(0x3e350a44), TM_UL(0xbcb4cdd5),
    TM_UL(0x72eacea8), TM_UL(0xfa6484bb), TM_UL(0x8d6612ae), TM_UL(0xbf3c6f47),
    TM_UL(0xd29be463), TM_UL(0x542f5d9e), TM_UL(0xaec2771b), TM_UL(0xf64e6370),
    TM_UL(0x740e0d8d), TM_UL(0xe75b1357), TM_UL(0xf8721671), TM_UL(0xaf537d5d),
    TM_UL(0x4040cb08), TM_UL(0x4eb4e2cc), TM_UL(0x34d2466a), TM_UL(0x0115af84),
    TM_UL(0xe1b00428), TM_UL(0x95983a1d), TM_UL(0x06b89fb4), TM_UL(0xce6ea048),
    TM_UL(0x6f3f3b82), TM_UL(0x3520ab82), TM_UL(0x011a1d4b), TM_UL(0x277227f8),
    TM_UL(0x611560b1), TM_UL(0xe7933fdc), TM_UL(0xbb3a792b), TM_UL(0x344525bd),
    TM_UL(0xa08839e1), TM_UL(0x51ce794b), TM_UL(0x2f32c9b7), TM_UL(0xa01fbac9),
    TM_UL(0xe01cc87e), TM_UL(0xbcc7d1f6), TM_UL(0xcf0111c3), TM_UL(0xa1e8aac7),
    TM_UL(0x1a908749), TM_UL(0xd44fbd9a), TM_UL(0xd0dadecb), TM_UL(0xd50ada38),
    TM_UL(0x0339c32a), TM_UL(0xc6913667), TM_UL(0x8df9317c), TM_UL(0xe0b12b4f),
    TM_UL(0xf79e59b7), TM_UL(0x43f5bb3a), TM_UL(0xf2d519ff), TM_UL(0x27d9459c),
    TM_UL(0xbf97222c), TM_UL(0x15e6fc2a), TM_UL(0x0f91fc71), TM_UL(0x9b941525),
    TM_UL(0xfae59361), TM_UL(0xceb69ceb), TM_UL(0xc2a86459), TM_UL(0x12baa8d1),
    TM_UL(0xb6c1075e), TM_UL(0xe3056a0c), TM_UL(0x10d25065), TM_UL(0xcb03a442),
    TM_UL(0xe0ec6e0e), TM_UL(0x1698db3b), TM_UL(0x4c98a0be), TM_UL(0x3278e964),
    TM_UL(0x9f1f9532), TM_UL(0xe0d392df), TM_UL(0xd3a0342b), TM_UL(0x8971f21e),
    TM_UL(0x1b0a7441), TM_UL(0x4ba3348c), TM_UL(0xc5be7120), TM_UL(0xc37632d8),
    TM_UL(0xdf359f8d), TM_UL(0x9b992f2e), TM_UL(0xe60b6f47), TM_UL(0x0fe3f11d),
    TM_UL(0xe54cda54), TM_UL(0x1edad891), TM_UL(0xce6279cf), TM_UL(0xcd3e7e6f),
    TM_UL(0x1618b166), TM_UL(0xfd2c1d05), TM_UL(0x848fd2c5), TM_UL(0xf6fb2299),
    TM_UL(0xf523f357), TM_UL(0xa6327623), TM_UL(0x93a83531), TM_UL(0x56cccd02),
    TM_UL(0xacf08162), TM_UL(0x5a75ebb5), TM_UL(0x6e163697), TM_UL(0x88d273cc),
    TM_UL(0xde966292), TM_UL(0x81b949d0), TM_UL(0x4c50901b), TM_UL(0x71c65614),
    TM_UL(0xe6c6c7bd), TM_UL(0x327a140a), TM_UL(0x45e1d006), TM_UL(0xc3f27b9a),
    TM_UL(0xc9aa53fd), TM_UL(0x62a80f00), TM_UL(0xbb25bfe2), TM_UL(0x35bdd2f6),
    TM_UL(0x71126905), TM_UL(0xb2040222), TM_UL(0xb6cbcf7c), TM_UL(0xcd769c2b),
    TM_UL(0x53113ec0), TM_UL(0x1640e3d3), TM_UL(0x38abbd60), TM_UL(0x2547adf0),
    TM_UL(0xba38209c), TM_UL(0xf746ce76), TM_UL(0x77afa1c5), TM_UL(0x20756060),
    TM_UL(0x85cbfe4e), TM_UL(0x8ae88dd8), TM_UL(0x7aaaf9b0), TM_UL(0x4cf9aa7e),
    TM_UL(0x1948c25c), TM_UL(0x02fb8a8c), TM_UL(0x01c36ae4), TM_UL(0xd6ebe1f9),
    TM_UL(0x90d4f869), TM_UL(0xa65cdea0), TM_UL(0x3f09252d), TM_UL(0xc208e69f),
    TM_UL(0xb74e6132), TM_UL(0xce77e25b), TM_UL(0x578fdfe3), TM_UL(0x3ac372e6)
    }
    };


void
tfBlowfishSetKey(ttBlowfishKeyPtr key, tt32Bit len, tt8BitPtr data)
{
    int i;
    tt32Bit ri, in[2];
    tt32BitPtr p;
    tt8BitPtr d, end;

    tm_memcpy((char *)key, (char *)&bfInit, sizeof(ttBlowfishKey));
    p = key->P;

    if (len > ((TM_BLOWFISH_ROUNDS + 2) * 4))
        len = (TM_BLOWFISH_ROUNDS + 2) * 4;

    d = data;
    end= &(data[len]);
    for (i = 0; i < TM_BLOWFISH_ROUNDS + 2; i++) {
        ri = (tt32Bit) *(d++);
        if (d >= end) d = data;

        ri <<=(unsigned) 8;
        ri |= *(d++);
        if (d >= end) d = data;

        ri <<=(unsigned) 8;
        ri |= *(d++);
        if (d >= end) d = data;

        ri <<=(unsigned) 8;
        ri |= *(d++);
        if (d >= end) d = data;

        p[i] ^= ri;
    }

    in[0] = (tt32Bit) 0;
    in[1] = (tt32Bit) 0;
    for (i = 0; i < TM_BLOWFISH_ROUNDS + 2; i += 2) {
        tfBlowfishEncrypt(in, key, TM_ESP_ENCRYPT);
        p[i  ] = in[0];
        p[i+1] = in[1];
    }

    p = key->S;
    for (i = 0; i < 4 * 256; i += 2) {
        tfBlowfishEncrypt(in, key, TM_ESP_ENCRYPT);
        p[i  ] = in[0];
        p[i+1] = in[1];
    }
}


/* XXX "data" is host endian */
void tfBlowfishEncrypt(tt32BitPtr data, ttBlowfishKeyPtr key, int encrypt)
{
    register tt32Bit l, r;
    register tt32BitPtr p, s;

    p = key->P;
    s= &key->S[0];
    l = data[0];
    r = data[1];

    if (encrypt == TM_ESP_ENCRYPT) {
        l^=p[0];
        tm_blowfish_enc(r, l, s, p[ 1]);
        tm_blowfish_enc(l, r, s, p[ 2]);
        tm_blowfish_enc(r, l, s, p[ 3]);
        tm_blowfish_enc(l, r, s, p[ 4]);
        tm_blowfish_enc(r, l, s, p[ 5]);
        tm_blowfish_enc(l, r, s, p[ 6]);
        tm_blowfish_enc(r, l, s, p[ 7]);
        tm_blowfish_enc(l, r, s, p[ 8]);
        tm_blowfish_enc(r, l, s, p[ 9]);
        tm_blowfish_enc(l, r, s, p[10]);
        tm_blowfish_enc(r, l, s, p[11]);
        tm_blowfish_enc(l, r, s, p[12]);
        tm_blowfish_enc(r, l, s, p[13]);
        tm_blowfish_enc(l, r, s, p[14]);
        tm_blowfish_enc(r, l, s, p[15]);
        tm_blowfish_enc(l, r, s, p[16]);
#if TM_BLOWFISH_ROUNDS == 20
        tm_blowfish_enc(r, l, s, p[17]);
        tm_blowfish_enc(l, r, s, p[18]);
        tm_blowfish_enc(r, l, s, p[19]);
        tm_blowfish_enc(l, r, s, p[20]);
#endif
        r ^= p[TM_BLOWFISH_ROUNDS + 1];
    } else {
        l ^= p[TM_BLOWFISH_ROUNDS + 1];
#if TM_BLOWFISH_ROUNDS == 20
        tm_blowfish_enc(r, l, s, p[20]);
        tm_blowfish_enc(l, r, s, p[19]);
        tm_blowfish_enc(r, l, s, p[18]);
        tm_blowfish_enc(l, r, s, p[17]);
#endif
        tm_blowfish_enc(r, l, s, p[16]);
        tm_blowfish_enc(l, r, s, p[15]);
        tm_blowfish_enc(r, l, s, p[14]);
        tm_blowfish_enc(l, r, s, p[13]);
        tm_blowfish_enc(r, l, s, p[12]);
        tm_blowfish_enc(l, r, s, p[11]);
        tm_blowfish_enc(r, l, s, p[10]);
        tm_blowfish_enc(l, r, s, p[ 9]);
        tm_blowfish_enc(r, l, s, p[ 8]);
        tm_blowfish_enc(l, r, s, p[ 7]);
        tm_blowfish_enc(r, l, s, p[ 6]);
        tm_blowfish_enc(l, r, s, p[ 5]);
        tm_blowfish_enc(r, l, s, p[ 4]);
        tm_blowfish_enc(l, r, s, p[ 3]);
        tm_blowfish_enc(r, l, s, p[ 2]);
        tm_blowfish_enc(l, r, s, p[ 1]);
        r ^= p[0];
    }
    data[1] = l;
    data[0] = r;
}

/*-----------------------  esp-blowfish ----------------------- */



int tfEspBlowfishBlockDecrypt(ttEspAlgorithmPtr algoPtr,
                              ttGenericKeyPtr   gkeyPtr,
                              tt8BitPtr         s,
                              tt8BitPtr         d)

{
/*  tfBlowfishEncrypt() takes values in host byteorder */
    tt32Bit t[2];

    TM_UNREF_IN_ARG(algoPtr);
    tm_bcopy(s, t, sizeof(t));
    t[0] = ntohl(t[0]);
    t[1] = ntohl(t[1]);
    tfBlowfishEncrypt(t, 
                      (ttBlowfishKeyPtr)gkeyPtr->keySchedulePtr, 
        TM_ESP_DECRYPT);
    t[0] = htonl(t[0]);
    t[1] = htonl(t[1]);
    tm_bcopy(t, d, sizeof(t));
    return TM_ENOERROR;
}



int tfEspBlowfishSchedLen(void)
{

    return sizeof(ttBlowfishKey);
}

int tfEspBlowfishSchedule(
            ttEspAlgorithmPtr     algoPtr,
            ttGenericKeyPtr       gkeyPtr)
{

    TM_UNREF_IN_ARG(algoPtr);
    tfBlowfishSetKey((ttBlowfishKeyPtr)gkeyPtr->keySchedulePtr,
             (tt32Bit)(gkeyPtr->keyBits>>3),
             gkeyPtr->keyDataPtr);
    return TM_ENOERROR;
}



int tfEspBlowfishBlockEncrypt( ttEspAlgorithmPtr algoPtr,
                               ttGenericKeyPtr   gkeyPtr,
                               tt8BitPtr         s,
                               tt8BitPtr         d)
{
/*  tfBlowfishEncrypt() takes values in host byteorder */
    tt32Bit t[2];

    TM_UNREF_IN_ARG(algoPtr);
    tm_bcopy(s, t, sizeof(t));
    t[0] = ntohl(t[0]);
    t[1] = ntohl(t[1]);
    tfBlowfishEncrypt(t, 
        (ttBlowfishKeyPtr)gkeyPtr->keySchedulePtr, 
        TM_ESP_ENCRYPT);
    t[0] = htonl(t[0]);
    t[1] = htonl(t[1]);
    tm_bcopy(t, d, sizeof(t));
    return 0;
}


#else /* ! TM_USE_BLOWFISH */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_BLOWFISH is not defined */
int tvBlwfshDummy = 0;

#endif /* TM_USE_BLOWFISH*/
