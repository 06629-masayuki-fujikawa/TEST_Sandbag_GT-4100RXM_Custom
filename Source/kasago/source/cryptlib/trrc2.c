/*
 * Description: RC2 implementation
 *
 * Filename: trrc2.c
 * Author: Jin Zhang
 * Date Created: 10/2/2004
 * $Source: source/cryptlib/trrc2.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:35:36JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
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
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
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

#ifdef TM_USE_RC2
#include <trcrylib.h>

#define TM_RC2_KEY_SIZE   16

#define tm_rc2_c2l(c,l) (l =((tt32Bit)(*((c)++))), \
             l|=((tt32Bit)(*((c)++)))<< 8, \
             l|=((tt32Bit)(*((c)++)))<<16, \
             l|=((tt32Bit)(*((c)++)))<<24)

#define tm_rc2_l2c(l,c) (*((c)++)=tm_8bit(l), \
             *((c)++)=tm_8bit((l)>> 8), \
             *((c)++)=tm_8bit((l)>>16), \
             *((c)++)=tm_8bit((l)>>24))

static const tt8Bit TM_CONST_QLF key_table[256]={
    0xd9,0x78,0xf9,0xc4,0x19,0xdd,0xb5,0xed,0x28,0xe9,0xfd,0x79,
    0x4a,0xa0,0xd8,0x9d,0xc6,0x7e,0x37,0x83,0x2b,0x76,0x53,0x8e,
    0x62,0x4c,0x64,0x88,0x44,0x8b,0xfb,0xa2,0x17,0x9a,0x59,0xf5,
    0x87,0xb3,0x4f,0x13,0x61,0x45,0x6d,0x8d,0x09,0x81,0x7d,0x32,
    0xbd,0x8f,0x40,0xeb,0x86,0xb7,0x7b,0x0b,0xf0,0x95,0x21,0x22,
    0x5c,0x6b,0x4e,0x82,0x54,0xd6,0x65,0x93,0xce,0x60,0xb2,0x1c,
    0x73,0x56,0xc0,0x14,0xa7,0x8c,0xf1,0xdc,0x12,0x75,0xca,0x1f,
    0x3b,0xbe,0xe4,0xd1,0x42,0x3d,0xd4,0x30,0xa3,0x3c,0xb6,0x26,
    0x6f,0xbf,0x0e,0xda,0x46,0x69,0x07,0x57,0x27,0xf2,0x1d,0x9b,
    0xbc,0x94,0x43,0x03,0xf8,0x11,0xc7,0xf6,0x90,0xef,0x3e,0xe7,
    0x06,0xc3,0xd5,0x2f,0xc8,0x66,0x1e,0xd7,0x08,0xe8,0xea,0xde,
    0x80,0x52,0xee,0xf7,0x84,0xaa,0x72,0xac,0x35,0x4d,0x6a,0x2a,
    0x96,0x1a,0xd2,0x71,0x5a,0x15,0x49,0x74,0x4b,0x9f,0xd0,0x5e,
    0x04,0x18,0xa4,0xec,0xc2,0xe0,0x41,0x6e,0x0f,0x51,0xcb,0xcc,
    0x24,0x91,0xaf,0x50,0xa1,0xf4,0x70,0x39,0x99,0x7c,0x3a,0x85,
    0x23,0xb8,0xb4,0x7a,0xfc,0x02,0x36,0x5b,0x25,0x55,0x97,0x31,
    0x2d,0x5d,0xfa,0x98,0xe3,0x8a,0x92,0xae,0x05,0xdf,0x29,0x10,
    0x67,0x6c,0xba,0xc9,0xd3,0x00,0xe6,0xcf,0xe1,0x9e,0xa8,0x2c,
    0x63,0x16,0x01,0x3f,0x58,0xe2,0x89,0xa9,0x0d,0x38,0x34,0x1b,
    0xab,0x33,0xff,0xb0,0xbb,0x48,0x0c,0x5f,0xb9,0xb1,0xcd,0x2e,
    0xc5,0xf3,0xdb,0x47,0xe5,0xa5,0x9c,0x77,0x0a,0xa6,0x20,0x68,
    0xfe,0x7f,0xc1,0xad,
    };

/* It has come to my attention that there are 2 versions of the RC2
 * key schedule.  One which is normal, and anther which has a hook to
 * use a reduced key length.
 * BSAFE uses the 'retarded' version.  What I previously shipped is
 * the same as specifying 1024 for the 'bits' parameter.  Bsafe uses
 * a version where the bits parameter is the same as len*8 */
/* Bits means effective key length */
static void RC2_set_key(ttRc2KeyPtr key, int len, 
                        ttConst8BitPtr data, int bits)
{
    int i,j;
    tt8BitPtr k;
    tt32BitPtr ki;
    unsigned int c,d;

    k= (tt8BitPtr)&(key->rc2KeyData[0]);
    *k=0; /* for if there is a zero length key */

    if (len > 128) len=128;
    if (bits <= 0) bits=1024;
    if (bits > 1024) bits=1024;
    
    for (i=0; i<len; i++)
        k[i]=data[i];

    /* expand table */
    d=k[len-1];
    j=0;
    for (i=len; i < 128; i++,j++)
    {
        d=key_table[(k[j]+d)&0xff];
        k[i] = tm_8bit(d);
    }

    /* hmm.... key reduction to 'bits' bits */

    j=(bits+7)>>3;
    i=128-j;
    c= (unsigned int)(0xff>>(-bits & 0x07));

    d=key_table[k[i]&c];
    k[i] = tm_8bit(d);

    while( i )
    {
        i--;
        d=key_table[k[i+j]^d];
        k[i] = tm_8bit(d);
    }

    /* copy from bytes into tt32Bit's */
    ki= &(key->rc2KeyData[63]);
    for (i=127; i>=0; i-=2)
    {
        *(ki--)=(tt32Bit)(((k[i]<<8)|k[i-1])&0xffff);
    }
}


static void RC2_encrypt(tt32BitPtr d, ttRc2KeyPtr key)
{
    int i,n;
    register tt32BitPtr p0,p1;
    register tt32Bit x0,x1,x2,x3,t;
    tt32Bit l;

    l=d[0];
    x0=(tt32Bit)l&0xffff;
    x1=(tt32Bit)(l>>16);
    l=d[1];
    x2=(tt32Bit)l&0xffff;
    x3=(tt32Bit)(l>>16);

    n=3;
    i=5;

    p0=p1= &(key->rc2KeyData[0]);
    for (;;)
    {
        t=(x0+(x1& ~x3)+(x2&x3)+ *(p0++))&0xffff;
        x0=(t<<1)|(t>>15);
        t=(x1+(x2& ~x0)+(x3&x0)+ *(p0++))&0xffff;
        x1=(t<<2)|(t>>14);
        t=(x2+(x3& ~x1)+(x0&x1)+ *(p0++))&0xffff;
        x2=(t<<3)|(t>>13);
        t=(x3+(x0& ~x2)+(x1&x2)+ *(p0++))&0xffff;
        x3=(t<<5)|(t>>11);

        i--;
        if(i == 0)
        {
            n--;
            if(n == 0) break;
            i=(n == 2)?6:5;

            x0+=p1[x3&0x3f];
            x1+=p1[x0&0x3f];
            x2+=p1[x1&0x3f];
            x3+=p1[x2&0x3f];
        }
    }

    d[0]=(tt32Bit)(x0&0xffff)|((tt32Bit)(x1&0xffff)<<16);
    d[1]=(tt32Bit)(x2&0xffff)|((tt32Bit)(x3&0xffff)<<16);
}

static void RC2_decrypt(tt32BitPtr d, ttRc2KeyPtr key)
{
    int i,n;
    register tt32BitPtr p0,p1;
    register tt32Bit x0,x1,x2,x3,t;
    tt32Bit l;

    l=d[0];
    x0=(tt32Bit)l&0xffff;
    x1=(tt32Bit)(l>>16);
    l=d[1];
    x2=(tt32Bit)l&0xffff;
    x3=(tt32Bit)(l>>16);

    n=3;
    i=5;

    p0= &(key->rc2KeyData[63]);
    p1= &(key->rc2KeyData[0]);
    for (;;)
    {
        t=((x3<<11)|(x3>>5))&0xffff;
        x3=(t-(x0& ~x2)-(x1&x2)- *(p0--))&0xffff;
        t=((x2<<13)|(x2>>3))&0xffff;
        x2=(t-(x3& ~x1)-(x0&x1)- *(p0--))&0xffff;
        t=((x1<<14)|(x1>>2))&0xffff;
        x1=(t-(x2& ~x0)-(x3&x0)- *(p0--))&0xffff;
        t=((x0<<15)|(x0>>1))&0xffff;
        x0=(t-(x1& ~x3)-(x2&x3)- *(p0--))&0xffff;

        i--;
        if(i == 0)
        {
            n--;
            if(n == 0) break;
            i=(n == 2)?6:5;

            x3=(x3-p1[x2&0x3f])&0xffff;
            x2=(x2-p1[x1&0x3f])&0xffff;
            x1=(x1-p1[x0&0x3f])&0xffff;
            x0=(x0-p1[x3&0x3f])&0xffff;
        }
    }

    d[0]=(tt32Bit)(x0&0xffff)|((tt32Bit)(x1&0xffff)<<16);
    d[1]=(tt32Bit)(x2&0xffff)|((tt32Bit)(x3&0xffff)<<16);
}

static void tfRc2EcbTransform(ttConst8BitPtr       in, 
                              tt8BitPtr            out, 
                              ttRc2KeyPtr          keyPtr,
                              int                  direction)
{
    tt32Bit l,d[2];

    tm_rc2_c2l(in,l); 
    d[0]=l;
    tm_rc2_c2l(in,l); 
    d[1]=l;
    if (direction == TM_ESP_ENCRYPT)
    {
        RC2_encrypt(d,keyPtr);
    }
    else
    {
        RC2_decrypt(d,keyPtr);
    }
    l=d[0]; 
    tm_rc2_l2c(l,out);
    l=d[1]; 
    tm_rc2_l2c(l,out);
    d[1] = 0;
    d[0] = 0;
    l    = 0;

}




int tfEspRc2SchedLen(void)
{
    return sizeof(ttRc2Key);
}

int tfEspRc2Schedule(ttEspAlgorithmPtr algoPtr,
                     ttGenericKeyPtr   gkeyPtr)
{
    ttRc2KeyPtr           k;
/* we use the keyround field to indicate the effective key bits */
    int                   bits;

    TM_UNREF_IN_ARG(algoPtr);
    if(gkeyPtr->keyRounds)
    {
        bits = gkeyPtr->keyRounds;
    }
    else
    {
/* if user doesn't specify effective key bits, we use keyLength * 8 */
        bits = (int)gkeyPtr->keyBits;
    }
    k = (ttRc2KeyPtr)gkeyPtr->keySchedulePtr;
    RC2_set_key(k, 
        (int)(gkeyPtr->keyBits>>3), 
        gkeyPtr->keyDataPtr,
        bits);
    return TM_ENOERROR;
}

int tfEspRc2BlockEncrypt(ttEspAlgorithmPtr algoPtr,
                         ttGenericKeyPtr   gkeyPtr,
                         tt8BitPtr         s,
                         tt8BitPtr         d)
{
    ttRc2KeyPtr    p;

    TM_UNREF_IN_ARG(algoPtr);
    p = (ttRc2KeyPtr)gkeyPtr->keySchedulePtr;

    tfRc2EcbTransform(s, d, p, TM_ESP_ENCRYPT);

    return TM_ENOERROR;
}

int tfEspRc2BlockDecrypt( ttEspAlgorithmPtr algoPtr,
                          ttGenericKeyPtr   gkeyPtr,
                          tt8BitPtr         s,
                          tt8BitPtr         d)
{
    ttRc2KeyPtr    p;

    TM_UNREF_IN_ARG(algoPtr);
    p = (ttRc2KeyPtr)gkeyPtr->keySchedulePtr;

    tfRc2EcbTransform(s, d, p, TM_ESP_DECRYPT);

    return TM_ENOERROR;
}

#else /* ! TM_USE_RC2 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_RC2 is not defined */
int tvRc2Dummy = 0;

#endif /* TM_USE_RC2 */
