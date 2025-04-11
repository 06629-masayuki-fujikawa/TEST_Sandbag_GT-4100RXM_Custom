/*
 * Description:  Alleged RC4 algorithm
 *
 * Filename: trarcfor.c
 * Author: Jin Zhang
 * Date Created: 07/22/2003
 * $Source: source/cryptlib/trarcfor.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:35:16JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


/*        This code illustrates a sample implementation
 *                 of the Arcfour algorithm
 *         Copyright (c) April 29, 1997 Kalle Kaukonen.
 *                    All Rights Reserved.
 * 
 *  draft-kaukonen-cipher-arcfour-03.txt
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that this copyright
 * notice and disclaimer are retained.
 *
 * THIS SOFTWARE IS PROVIDED BY KALLE KAUKONEN AND CONTRIBUTORS ``AS
 * IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL KALLE
 * KAUKONEN OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>

#ifdef TM_USE_ARCFOUR
#include <trcrylib.h>
 
 

static void tfArc4Init(ttArc4ContextPtr ctxPtr, 
                       ttConst8BitPtr   keyPtr, 
                       ttPktLen         keyLength);

/* as AES uses assymetric scheduled keys, we need to do it twice. */
int tfEspArc4SchedLen(void)
{
    return sizeof(ttArc4Context);
}

int tfEspArc4Schedule(ttEspAlgorithmPtr algoPtr,
                      ttGenericKeyPtr   gkeyPtr)
{
    TM_UNREF_IN_ARG(algoPtr);
    tfArc4Init(gkeyPtr->keySchedulePtr,
               gkeyPtr->keyDataPtr,
               (ttPktLen) (gkeyPtr->keyBits >>3));
    return TM_ENOERROR;
}
             

static void tfArc4Init(ttArc4ContextPtr ctxPtr, 
                       ttConst8BitPtr   keyPtr, 
                       ttPktLen         keyLength)
{
    ttPktLen  keyIndex;
    tt8Bit    stateIndex;
    tt16Bit   counter;
    tt8BitPtr statePtr;
    tt8Bit    t;
    tt8Bit    u;
    
    statePtr = ctxPtr->arc4State;
    ctxPtr->arc4X = TM_8BIT_ZERO;
    ctxPtr->arc4Y = TM_8BIT_ZERO;
    for (counter = TM_16BIT_ZERO; counter < (tt16Bit) TM_ARC4_STATE_SIZE;
         counter++)
    {
        statePtr[counter] = (tt8Bit)counter;
    }
    keyIndex = (ttPktLen) 0;
    stateIndex = TM_8BIT_ZERO;
    for (counter = TM_16BIT_ZERO; counter < (tt16Bit) TM_ARC4_STATE_SIZE;
         counter++)
    {
        t = statePtr[counter];
        stateIndex = tm_8bit(stateIndex + keyPtr[keyIndex] + t);
        u = statePtr[stateIndex];
        statePtr[stateIndex] = t;
        statePtr[counter] = u;
        keyIndex++;
        if( keyIndex >= keyLength )
        {
            keyIndex = (ttPktLen) 0;
        }
    }
}


void tfArc4Encrypt(ttArc4ContextPtr ctxPtr, 
                   tt8BitPtr        destPtr,
                   ttConst8BitPtr   srcPtr,
                   ttPktLen         len)
{
    ttPktLen    i;
    tt8BitPtr   statePtr;
    tt8Bit      x;
    tt8Bit      y;
    tt8Bit      sx;
    tt8Bit      sy;

    statePtr = ctxPtr->arc4State;

    for (i = (ttPktLen)0; i < len; i++)
    {
        x             = tm_8bit(ctxPtr->arc4X + 1);
        sx            = statePtr[x];
        y             = tm_8bit(sx + ctxPtr->arc4Y);
        sy            = statePtr[y];
        ctxPtr->arc4X = x;
        ctxPtr->arc4Y = y;
        statePtr[y]   = sx;
        statePtr[x]   = sy;

        destPtr[i] = tm_8bit(srcPtr[i] ^ statePtr[tm_8bit(sx + sy)]);
    }
}


/*  For either encryption or decryption, the input text is processed one
 *  byte at a time.           
 *  To encrypt, XOR the value K with the next byte of the plaintext.  To 
 *  decrypt, XOR the value K with the next byte of the ciphertext.
 */

void tfArc4EncryptMessage(ttConst8BitPtr    srcPtr,
                          ttPktLen          srcLength,
                          tt8BitPtr         keyPtr,
                          ttPktLen          keyLength,
                          tt8BitPtr         destPtr)
{
    ttArc4Context     arc4Context;
    tfArc4Init(&arc4Context,
               keyPtr,
               keyLength);
    tfArc4Encrypt(&arc4Context,
                  destPtr,
                  srcPtr,
                  srcLength);
}




#else /* !TM_USE_ARCFOUR */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_ARCFOUR is not defined */
int tlArcfourDummy = 0;

#endif /* TM_USE_ARCFOUR */
