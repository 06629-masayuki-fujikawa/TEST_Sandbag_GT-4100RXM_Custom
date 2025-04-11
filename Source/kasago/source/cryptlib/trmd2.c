/*
 * Filename: trmd2.c
 * Author: Jin Zhang
 * Date Created: 12/23/2004
 * $Source: source/cryptlib/trmd2.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:35:32JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


/* crypto/md2/md2_dgst.c */
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
#include <trproto.h>
#include <trglobal.h>

#ifdef TM_USE_MD2
/* Implemented from RFC1319 The MD2 Message-Digest Algorithm
 */

#define TM_MD2_BLOCK_SIZE   16

static void tfMd2Block(ttMd2CtxPtr md2CtxPtr, const tt8BitPtr d);
/* The magic tlMd2SValue table - I have converted it to hex since it is
 * basically just a random byte string. */
static const tt32Bit TM_CONST_QLF tlMd2SValue[256] = {
    0x29, 0x2E, 0x43, 0xC9, 0xA2, 0xD8, 0x7C, 0x01,
    0x3D, 0x36, 0x54, 0xA1, 0xEC, 0xF0, 0x06, 0x13,
    0x62, 0xA7, 0x05, 0xF3, 0xC0, 0xC7, 0x73, 0x8C,
    0x98, 0x93, 0x2B, 0xD9, 0xBC, 0x4C, 0x82, 0xCA,
    0x1E, 0x9B, 0x57, 0x3C, 0xFD, 0xD4, 0xE0, 0x16,
    0x67, 0x42, 0x6F, 0x18, 0x8A, 0x17, 0xE5, 0x12,
    0xBE, 0x4E, 0xC4, 0xD6, 0xDA, 0x9E, 0xDE, 0x49,
    0xA0, 0xFB, 0xF5, 0x8E, 0xBB, 0x2F, 0xEE, 0x7A,
    0xA9, 0x68, 0x79, 0x91, 0x15, 0xB2, 0x07, 0x3F,
    0x94, 0xC2, 0x10, 0x89, 0x0B, 0x22, 0x5F, 0x21,
    0x80, 0x7F, 0x5D, 0x9A, 0x5A, 0x90, 0x32, 0x27,
    0x35, 0x3E, 0xCC, 0xE7, 0xBF, 0xF7, 0x97, 0x03,
    0xFF, 0x19, 0x30, 0xB3, 0x48, 0xA5, 0xB5, 0xD1,
    0xD7, 0x5E, 0x92, 0x2A, 0xAC, 0x56, 0xAA, 0xC6,
    0x4F, 0xB8, 0x38, 0xD2, 0x96, 0xA4, 0x7D, 0xB6,
    0x76, 0xFC, 0x6B, 0xE2, 0x9C, 0x74, 0x04, 0xF1,
    0x45, 0x9D, 0x70, 0x59, 0x64, 0x71, 0x87, 0x20,
    0x86, 0x5B, 0xCF, 0x65, 0xE6, 0x2D, 0xA8, 0x02,
    0x1B, 0x60, 0x25, 0xAD, 0xAE, 0xB0, 0xB9, 0xF6,
    0x1C, 0x46, 0x61, 0x69, 0x34, 0x40, 0x7E, 0x0F,
    0x55, 0x47, 0xA3, 0x23, 0xDD, 0x51, 0xAF, 0x3A,
    0xC3, 0x5C, 0xF9, 0xCE, 0xBA, 0xC5, 0xEA, 0x26,
    0x2C, 0x53, 0x0D, 0x6E, 0x85, 0x28, 0x84, 0x09,
    0xD3, 0xDF, 0xCD, 0xF4, 0x41, 0x81, 0x4D, 0x52,
    0x6A, 0xDC, 0x37, 0xC8, 0x6C, 0xC1, 0xAB, 0xFA,
    0x24, 0xE1, 0x7B, 0x08, 0x0C, 0xBD, 0xB1, 0x4A,
    0x78, 0x88, 0x95, 0x8B, 0xE3, 0x63, 0xE8, 0x6D,
    0xE9, 0xCB, 0xD5, 0xFE, 0x3B, 0x00, 0x1D, 0x39,
    0xF2, 0xEF, 0xB7, 0x0E, 0x66, 0x58, 0xD0, 0xE4,
    0xA6, 0x77, 0x72, 0xF8, 0xEB, 0x75, 0x4B, 0x0A,
    0x31, 0x44, 0x50, 0xB4, 0x8F, 0xED, 0x1F, 0x1A,
    0xDB, 0x99, 0x8D, 0x33, 0x9F, 0x11, 0x83, 0x14,
    };

void tfMd2Init(ttVoidPtr contextPtr)
{
    ttMd2CtxPtr md2CtxPtr;
    md2CtxPtr = (ttMd2CtxPtr)contextPtr;
    md2CtxPtr->md2Num = 0;
    tm_bzero(md2CtxPtr->md2State,sizeof(md2CtxPtr->md2State));
    tm_bzero(md2CtxPtr->md2Cksm,sizeof(md2CtxPtr->md2Cksm));
    tm_bzero(md2CtxPtr->md2Data,sizeof(md2CtxPtr->md2Data));
    return;
}

void tfMd2Update(ttVoidPtr      contextPtr,
                tt8BitPtr      inBufPtr, 
                ttPktLen       inLen,
                ttPktLen       offset)
{
    tt8BitPtr        p;
    ttMd2CtxPtr      md2CtxPtr;
    
    TM_UNREF_IN_ARG(offset);
    md2CtxPtr = (ttMd2CtxPtr)contextPtr;
    if (inLen == 0) 
    {
        goto md2UpdateExit;
    }
    p = md2CtxPtr->md2Data;
    if (md2CtxPtr->md2Num != 0)
    {
        if ((md2CtxPtr->md2Num+ (int)inLen) >= TM_MD2_BLOCK_SIZE)
        {
            tm_memcpy(&(p[md2CtxPtr->md2Num]),inBufPtr,TM_MD2_BLOCK_SIZE-md2CtxPtr->md2Num);
            tfMd2Block(md2CtxPtr,md2CtxPtr->md2Data);
            inBufPtr +=(TM_MD2_BLOCK_SIZE - md2CtxPtr->md2Num);
            inLen -= (unsigned) (TM_MD2_BLOCK_SIZE - md2CtxPtr->md2Num);
            md2CtxPtr->md2Num = 0;
            /* drop through and do the rest */
        }
        else
        {
            tm_memcpy(&(p[md2CtxPtr->md2Num]),inBufPtr,(int)inLen);
            md2CtxPtr->md2Num +=(int)inLen;
            goto md2UpdateExit;
        }
    }
    /* we now can process the input inBufPtr in blocks of TM_MD2_BLOCK_SIZE
     * chars and save the leftovers to md2CtxPtr->md2Data. */
    while (inLen >= TM_MD2_BLOCK_SIZE)
    {
        tfMd2Block(md2CtxPtr,inBufPtr);
        inBufPtr += TM_MD2_BLOCK_SIZE;
        inLen -= TM_MD2_BLOCK_SIZE;
    }
    tm_memcpy(p,inBufPtr,(int)inLen);
    md2CtxPtr->md2Num = (int)inLen;

md2UpdateExit:
    return;
}

static void tfMd2Block(ttMd2CtxPtr md2CtxPtr, const tt8BitPtr d)
{
    tt32Bit     t;
    tt32BitPtr  sp1;
    tt32BitPtr  sp2;
    int         i;
    int         j;
    tt32BitPtr  state;

    state = tm_get_raw_buffer(sizeof(tt32Bit) * 48);
    if(!state)
    {
        goto md2BlockExit;
    }

    sp1 = md2CtxPtr->md2State;
    sp2 = md2CtxPtr->md2Cksm;
    j = (int)sp2[TM_MD2_BLOCK_SIZE-1];
    for (i = 0; i < 16; i++)
    {
        state[i] = sp1[i];
        t = d[i];
        state[i+16] = t;
        state[i+32] = (t^sp1[i]);
        sp2[i] ^= tlMd2SValue[(int)t^j];
        j = (int)sp2[i];
    }
    t = 0;
    for (i = 0; i < 18; i++)
    {
        for (j = 0; j < 48; j += 8)
        {
            state[j + 0] ^= tlMd2SValue[t];
            t = state[j + 0];
            state[j + 1] ^= tlMd2SValue[t];
            t = state[j + 1];
            state[j + 2] ^= tlMd2SValue[t];
            t = state[j + 2];
            state[j + 3] ^= tlMd2SValue[t];
            t = state[j + 3];
            state[j + 4] ^= tlMd2SValue[t];
            t = state[j + 4];
            state[j + 5] ^= tlMd2SValue[t];
            t = state[j + 5];
            state[j + 6] ^= tlMd2SValue[t];
            t = state[j + 6];
            state[j + 7] ^= tlMd2SValue[t];
            t = state[j + 7];
        }
        t = (t+(tt32Bit)i)& 0xff;
    }
    tm_memcpy(sp1,state,16*sizeof(tt32Bit));

md2BlockExit:
    if(state)
    {
        tm_free_raw_buffer(state);
    }
}

void tfMd2Final(tt8BitPtr   digestPtr, 
                ttVoidPtr   contextPtr)

{
    ttMd2CtxPtr md2CtxPtr;
    int         i;
    int         v;
    tt8BitPtr   cp;
    tt32BitPtr  p1;
    tt32BitPtr  p2;

    md2CtxPtr = (ttMd2CtxPtr)contextPtr;

    cp = md2CtxPtr->md2Data;
    p1 = md2CtxPtr->md2State;
    p2 = md2CtxPtr->md2Cksm;
    v = TM_MD2_BLOCK_SIZE-md2CtxPtr->md2Num;
    for (i = md2CtxPtr->md2Num; i < TM_MD2_BLOCK_SIZE; i++)
    {
        cp[i] = tm_8bit(v);
    }

    tfMd2Block(md2CtxPtr,cp);

    for (i = 0; i < TM_MD2_BLOCK_SIZE; i++)
    {
        cp[i] = tm_8bit(p2[i]);
    }
    tfMd2Block(md2CtxPtr,cp);

    for (i = 0; i < 16; i++)
    {
        digestPtr[i] = tm_8bit(p1[i]);
    }
    tm_bzero((ttCharPtr)&md2CtxPtr,sizeof(md2CtxPtr));
    return;
}

#else /* ! TM_USE_MD2 */

LINT_UNUSED_HEADER

/* To allow link for builds when TM_USE_MD2 is not defined */
int tvMd2Dummy = 0;
#endif /* TM_USE_MD2 */

/***************** End Of File *****************/

