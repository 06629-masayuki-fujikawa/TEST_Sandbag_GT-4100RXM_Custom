/*
 * Description: SHA-512 and SHA-384 alg from modified Mozilla
 * sha512.c implementation
 *
 * Filename: trsha5.c
 * Author: E Crockett
 * Date Created: 06/19/2008
 * $Source: source/cryptlib/trsha5.c $
 *
 * Modification History
 * $Revision: 6.0.2.3 $
 * $Date: 2010/07/31 00:37:24JST $
 * $Author: pcarney $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * sha512.c - implementation of SHA256, SHA384 and SHA512
 *
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Netscape security libraries.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
/*
 * FIPS pub 180-2: Secure Hash Standard
 * based on: http://csrc.nist.gov/publications/fips/fips180-2/fips180-2.pdf
 */

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>

/* For SHA384 and SHA512 */
#if (defined(TM_USE_SHA384) || defined(TM_USE_SHA512))
#define W ctx->u.w
#define H ctx->shaDigest

#define TM_HMAC_SHA512_BLOCK_SIZE 128

#define SWAP4MASK  0x00FF00FF
#define SHA_HTONL(x) (t1 = (x), t1 = (t1 << 16) | (t1 >> 16), \
                      ((t1 & SWAP4MASK) << 8) | ((t1 >> 8) & SWAP4MASK))
#define BYTESWAP4(x)  x = SHA_HTONL(x)

#define BYTESWAP8(x) \
            { \
                tt32Bit tmp; \
                BYTESWAP4(x.lo); \
                BYTESWAP4(x.hi); \
                tmp = x.lo; \
                x.lo = x.hi; \
                x.hi = tmp; \
            }

#ifdef TM_LITTLE_ENDIAN
#define ULLC(hi,lo) { TM_UL(0x ## lo), TM_UL(0x ## hi) }
#else
#define ULLC(hi,lo) { TM_UL(0x ## hi), TM_UL(0x ## lo) }
#endif

/* use only 32-bit variables, and don't unroll loops */
#define ADDTO(x,y) y.lo += x.lo; y.hi += x.hi + (x.lo > y.lo)

#define ROTR64a(x,n,lo,hi) (x.lo >> n | x.hi << (32-n))
#define ROTR64A(x,n,lo,hi) (x.lo << (64-n) | x.hi >> (n-32))
#define SHR64a(x,n,lo,hi)  (x.lo >> n | x.hi << (32-n))

/* Capitol Sigma and lower case sigma functions */
#define s0lo(x) (ROTR64a(x,1,lo,hi)  ^ ROTR64a(x,8,lo,hi)  ^ \
                SHR64a(x,7,lo,hi))
#define s0hi(x) (ROTR64a(x,1,hi,lo)  ^ ROTR64a(x,8,hi,lo)  ^ (x.hi >> 7))

#define s1lo(x) (ROTR64a(x,19,lo,hi) ^ ROTR64A(x,61,lo,hi) ^ \
                SHR64a(x,6,lo,hi))
#define s1hi(x) (ROTR64a(x,19,hi,lo) ^ ROTR64A(x,61,hi,lo) ^ (x.hi >> 6))

#define S0lo(x) (ROTR64a(x,28,lo,hi) ^ ROTR64A(x,34,lo,hi) ^ \
                ROTR64A(x,39,lo,hi))
#define S0hi(x) (ROTR64a(x,28,hi,lo) ^ ROTR64A(x,34,hi,lo) ^ \
                ROTR64A(x,39,hi,lo))

#define S1lo(x) (ROTR64a(x,14,lo,hi) ^ ROTR64a(x,18,lo,hi) ^ \
                ROTR64A(x,41,lo,hi))
#define S1hi(x) (ROTR64a(x,14,hi,lo) ^ ROTR64a(x,18,hi,lo) ^ \
                ROTR64A(x,41,hi,lo))

/* 32-bit versions of Ch and Maj */
#define Chxx(x,y,z,lo) ((x.lo & y.lo) ^ (~x.lo & z.lo))
#define Majx(x,y,z,lo) ((x.lo & y.lo) ^ (x.lo & z.lo) ^ (y.lo & z.lo))

#define INITW(t) \
    { \
	    tt32Bit lo; \
        tt32Bit tm; \
	    tt32Bit cy = 0; \
	    lo = s1lo(W[t-2]); \
	    lo += (tm = W[t-7].lo); \
        if (lo < tm) \
        { \
            cy++; \
        } \
	    lo += (tm = s0lo(W[t-15])); \
        if (lo < tm) \
        {  \
            cy++; \
        } \
	    lo += (tm = W[t-16].lo); \
        if (lo < tm) \
        { \
            cy++; \
        } \
	    W[t].lo = lo; \
	    W[t].hi = cy + s1hi(W[t-2]) + W[t-7].hi + s0hi(W[t-15]) + W[t-16].hi; \
    }

#define ROUND(n,a,b,c,d,e,f,g,h) \
    { \
	    tt32Bit lo; \
        tt32Bit tm; \
        tt32Bit cy; \
	    lo  = S1lo(e); \
	    lo += (tm = Chxx(e,f,g,lo)); \
        cy = (lo < tm); \
	    lo += (tm = K512[n].lo); \
        if (lo < tm) \
        { \
            cy++; \
        } \
	    lo += (tm =    W[n].lo); \
        if (lo < tm) \
        { \
            cy++; \
        } \
	    h.lo += lo;	\
        if (h.lo < lo) \
        { \
            cy++; \
        } \
	    h.hi += cy + S1hi(e) + Chxx(e,f,g,hi) + K512[n].hi + W[n].hi; \
	    d.lo += h.lo; \
	    d.hi += h.hi + (d.lo < h.lo); \
	    lo = S0lo(a);  \
	    lo += (tm = Majx(a,b,c,lo)); \
        cy = (lo < tm); \
	    h.lo += lo;	\
        if (h.lo < lo) \
        { \
            cy++; \
        } \
	    h.hi += cy + S0hi(a) + Majx(a,b,c,hi); \
    }

/* Padding used with all flavors of SHA */
static const tt8Bit pad[128] = {
                0x80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

/* SHA-384 and SHA-512 constants, K512. */
static const ttSHAint64 K512[80] = {
    ULLC(428a2f98,d728ae22), ULLC(71374491,23ef65cd),
    ULLC(b5c0fbcf,ec4d3b2f), ULLC(e9b5dba5,8189dbbc),
    ULLC(3956c25b,f348b538), ULLC(59f111f1,b605d019),
    ULLC(923f82a4,af194f9b), ULLC(ab1c5ed5,da6d8118),
    ULLC(d807aa98,a3030242), ULLC(12835b01,45706fbe),
    ULLC(243185be,4ee4b28c), ULLC(550c7dc3,d5ffb4e2),
    ULLC(72be5d74,f27b896f), ULLC(80deb1fe,3b1696b1),
    ULLC(9bdc06a7,25c71235), ULLC(c19bf174,cf692694),
    ULLC(e49b69c1,9ef14ad2), ULLC(efbe4786,384f25e3),
    ULLC(0fc19dc6,8b8cd5b5), ULLC(240ca1cc,77ac9c65),
    ULLC(2de92c6f,592b0275), ULLC(4a7484aa,6ea6e483),
    ULLC(5cb0a9dc,bd41fbd4), ULLC(76f988da,831153b5),
    ULLC(983e5152,ee66dfab), ULLC(a831c66d,2db43210),
    ULLC(b00327c8,98fb213f), ULLC(bf597fc7,beef0ee4),
    ULLC(c6e00bf3,3da88fc2), ULLC(d5a79147,930aa725),
    ULLC(06ca6351,e003826f), ULLC(14292967,0a0e6e70),
    ULLC(27b70a85,46d22ffc), ULLC(2e1b2138,5c26c926),
    ULLC(4d2c6dfc,5ac42aed), ULLC(53380d13,9d95b3df),
    ULLC(650a7354,8baf63de), ULLC(766a0abb,3c77b2a8),
    ULLC(81c2c92e,47edaee6), ULLC(92722c85,1482353b),
    ULLC(a2bfe8a1,4cf10364), ULLC(a81a664b,bc423001),
    ULLC(c24b8b70,d0f89791), ULLC(c76c51a3,0654be30),
    ULLC(d192e819,d6ef5218), ULLC(d6990624,5565a910),
    ULLC(f40e3585,5771202a), ULLC(106aa070,32bbd1b8),
    ULLC(19a4c116,b8d2d0c8), ULLC(1e376c08,5141ab53),
    ULLC(2748774c,df8eeb99), ULLC(34b0bcb5,e19b48a8),
    ULLC(391c0cb3,c5c95a63), ULLC(4ed8aa4a,e3418acb),
    ULLC(5b9cca4f,7763e373), ULLC(682e6ff3,d6b2b8a3),
    ULLC(748f82ee,5defb2fc), ULLC(78a5636f,43172f60),
    ULLC(84c87814,a1f0ab72), ULLC(8cc70208,1a6439ec),
    ULLC(90befffa,23631e28), ULLC(a4506ceb,de82bde9),
    ULLC(bef9a3f7,b2c67915), ULLC(c67178f2,e372532b),
    ULLC(ca273ece,ea26619c), ULLC(d186b8c7,21c0c207),
    ULLC(eada7dd6,cde0eb1e), ULLC(f57d4f7f,ee6ed178),
    ULLC(06f067aa,72176fba), ULLC(0a637dc5,a2c898a6),
    ULLC(113f9804,bef90dae), ULLC(1b710b35,131c471b),
    ULLC(28db77f5,23047d84), ULLC(32caab7b,40c72493),
    ULLC(3c9ebe0a,15c9bebc), ULLC(431d67c4,9c100d4c),
    ULLC(4cc5d4be,cb3e42b6), ULLC(597f299c,fc657e2a),
    ULLC(5fcb6fab,3ad6faec), ULLC(6c44198c,4a475817)
};

#ifdef TM_USE_SHA512
/* SHA-512 initial hash values */
static const ttSHAint64 H512[8] = {
    ULLC(6a09e667,f3bcc908), ULLC(bb67ae85,84caa73b),
    ULLC(3c6ef372,fe94f82b), ULLC(a54ff53a,5f1d36f1),
    ULLC(510e527f,ade682d1), ULLC(9b05688c,2b3e6c1f),
    ULLC(1f83d9ab,fb41bd6b), ULLC(5be0cd19,137e2179)
};
#endif /* TM_USE_SHA512 */

#ifdef TM_USE_SHA384
/* SHA-384 initial hash values */
static const ttSHAint64 H384[8] = {
    ULLC(cbbb9d5d,c1059ed8), ULLC(629a292a,367cd507),
    ULLC(9159015a,3070dd17), ULLC(152fecd8,f70e5939),
    ULLC(67332667,ffc00b31), ULLC(8eb44a87,68581511),
    ULLC(db0c2e0d,64f98fa7), ULLC(47b5481d,befa4fa4)
};
#endif /* TM_USE_SHA384 */

static void tfSha2Final(tt8BitPtr outputPtr,
                        ttVoidPtr sha512ContextPtr,
                        tt8Bit    truncate);

#ifdef TM_USE_SHA512
void tfSha512Init(ttVoidPtr sha512ContextPtr)
{
    ttSha512CtxPtr shaContextPtr;

    shaContextPtr = (ttSha512CtxPtr)sha512ContextPtr;

/* clear entire pointer */
    tm_memset(shaContextPtr, 0, sizeof(ttSha512Ctx));
/* set initialization vectors */

    tm_memcpy(shaContextPtr->shaDigest, H512, sizeof(H512));
}
#endif /* TM_USE_SHA512 */

static void tfSha512Transform(ttSha512CtxPtr ctx)
{
    ttSHAint64 a;
    ttSHAint64 b;
    ttSHAint64 c;
    ttSHAint64 d;
    ttSHAint64 e;
    ttSHAint64 f;
    ttSHAint64 g;
    ttSHAint64 h;
    ttS32Bit t;
#ifdef TM_LITTLE_ENDIAN
    tt32Bit t1;
#endif /* TM_LITTLE_ENDIAN */

#ifdef TM_LITTLE_ENDIAN
    {
        BYTESWAP8(W[0]);
        BYTESWAP8(W[1]);
        BYTESWAP8(W[2]);
        BYTESWAP8(W[3]);
        BYTESWAP8(W[4]);
        BYTESWAP8(W[5]);
        BYTESWAP8(W[6]);
        BYTESWAP8(W[7]);
        BYTESWAP8(W[8]);
        BYTESWAP8(W[9]);
        BYTESWAP8(W[10]);
        BYTESWAP8(W[11]);
        BYTESWAP8(W[12]);
        BYTESWAP8(W[13]);
        BYTESWAP8(W[14]);
        BYTESWAP8(W[15]);
    }
#endif

    {
/* prepare the "message schedule"   */
	    for (t = 16; t < 80; ++t)
        {
	        INITW(t);
	    }
    }

    {
        a = H[0];
        b = H[1];
        c = H[2];
        d = H[3];
        e = H[4];
        f = H[5];
        g = H[6];
        h = H[7];

        {
	        for (t = 0; t < 80; t+= 8)
            {
	            ROUND(t+0,a,b,c,d,e,f,g,h)
	            ROUND(t+1,h,a,b,c,d,e,f,g)
	            ROUND(t+2,g,h,a,b,c,d,e,f)
	            ROUND(t+3,f,g,h,a,b,c,d,e)
	            ROUND(t+4,e,f,g,h,a,b,c,d)
	            ROUND(t+5,d,e,f,g,h,a,b,c)
	            ROUND(t+6,c,d,e,f,g,h,a,b)
	            ROUND(t+7,b,c,d,e,f,g,h,a)
	        }
        }

        ADDTO(a,H[0]);
        ADDTO(b,H[1]);
        ADDTO(c,H[2]);
        ADDTO(d,H[3]);
        ADDTO(e,H[4]);
        ADDTO(f,H[5]);
        ADDTO(g,H[6]);
        ADDTO(h,H[7]);
    }
}

void tfSha512Update(ttVoidPtr sha512ContextPtr,
                    tt8BitPtr buffer,
                    ttPktLen  count,
                    ttPktLen  offset)
{
    tt8BitPtr      p;
    ttSha512CtxPtr shaContextPtr;
    ttPktLen       dataCount;

    TM_UNREF_IN_ARG(offset); /* for 16Bit or 32Bit DSP usage */

    shaContextPtr = (ttSha512CtxPtr)sha512ContextPtr;

/* Get count of bytes already in data, buffer
 * This is the data passed in % BLOCK_SIZE,
 * the amount of information that has not yet been digested
 * We convert the seen bits to bytes, then do manual modulus on (block size)
 */
    dataCount = (ttPktLen)((shaContextPtr->sizeLo.lo>>3) & (0x7f));

/* Update bitcount, total bits in message, not used in Update */
/* Add number of bits in the buffer to shaCountLo, test for 32-bit overflow */
    shaContextPtr->sizeLo.lo += (count<<3);
    if (shaContextPtr->sizeLo.lo < (count<<3))
    {
/* Carry from low to high */
        shaContextPtr->sizeLo.hi++;
    }
    shaContextPtr->sizeLo.hi += count >> 29;

/* Handle any leading odd-sized chunks */
    if (dataCount)
    {
        p = (tt8BitPtr)shaContextPtr->u.shaData + dataCount;
/* Find how much space is left in shaData */
    	dataCount = TM_HMAC_SHA512_BLOCK_SIZE - dataCount;
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
/* The data to be hashed is in shaData, the buffer we just filled */
        tfSha512Transform(shaContextPtr);
/* next time we put anything into buffer, it goes at the end */
	    buffer += dataCount;
/* we have count-dataCount bytes left in the buffer to process */
	    count  -= dataCount;
    }

/* Process data in TM_HMAC_SHA_BLOCK_SIZE chunks */
    while (count >= TM_HMAC_SHA512_BLOCK_SIZE)
    {
/* copy a full block from buffer to shaData, the process it */
    	tm_memcpy(shaContextPtr->u.shaData,
                  buffer,
                  TM_HMAC_SHA512_BLOCK_SIZE);
        tfSha512Transform(shaContextPtr);
/* adjust buffer pointer and byte count again */
	    buffer += TM_HMAC_SHA512_BLOCK_SIZE;
	    count  -= TM_HMAC_SHA512_BLOCK_SIZE;
    }
/* Handle any remaining bytes of data. */
/* completely empty the buffer of the remaining COUNT bytes,
 * which is less than the block size
 */
    tm_memcpy((tt8BitPtr)shaContextPtr->u.shaData, buffer, count);
UPDATE_RETURN:
    return;
}

#ifdef TM_USE_SHA512
void tfSha512Final(tt8BitPtr outputPtr,
                   ttVoidPtr sha512ContextPtr)
{
    tfSha2Final(outputPtr, sha512ContextPtr, TM_8BIT_NO);
}
#endif /* TM_USE_SHA512 */

static void tfSha2Final(tt8BitPtr outputPtr,
                        ttVoidPtr sha512ContextPtr,
                        tt8Bit    sha384truncate)
{
    ttSha512CtxPtr shaContextPtr;
    ttPktLen       count;
    ttPktLen       padLen;
#ifdef TM_LITTLE_ENDIAN
    tt32Bit        t1;
#endif /* TM_LITTLE_ENDIAN */
    tt8Bit         copyLen;

    shaContextPtr = (ttSha512CtxPtr)sha512ContextPtr;

/* Compute number of bytes mod 128 */
    count = (ttPktLen)((shaContextPtr->sizeLo.lo>>3) & 0x7f);

/* either pad this block out to 112 bytes, or if necessary, pad to the end
 * then create a new block padded to 112 bytes, leaving 16 bytes for the
 * number.  16+112 = 128 the block size
 */

    if (count < 112)
    {
        padLen = 112-count;
    }
    else
    {
        padLen = 112+128-count;
    }

    tfSha512Update(shaContextPtr, (tt8BitPtr)pad, padLen, 0);

/* Get the number of bytes in the message.  We do not allow for messages
 * less than 2^128 as specified, only 2^64, so we ignore the upper 64 bits
 */
    padLen <<= 3;
    if (shaContextPtr->sizeLo.lo < padLen)
    {
        shaContextPtr->sizeLo.hi--;
    }
    shaContextPtr->sizeLo.lo-=padLen;
/* Messages are allowed to be less than 2^64 bits in length,
 * not the full 2^128 as stated in FIPS180-2
 */
    shaContextPtr->u.w[14].lo = 0;
    shaContextPtr->u.w[14].hi = 0;
    shaContextPtr->u.w[15].lo = shaContextPtr->sizeLo.lo;
    shaContextPtr->u.w[15].hi = shaContextPtr->sizeLo.hi;
#ifdef TM_LITTLE_ENDIAN
    BYTESWAP8(shaContextPtr->u.w[15]);
#endif /* TM_LITTLE_ENDIAN */
    tfSha512Transform(shaContextPtr);

    /* now output the answer */
#ifdef TM_LITTLE_ENDIAN
    BYTESWAP8(shaContextPtr->shaDigest[0]);
    BYTESWAP8(shaContextPtr->shaDigest[1]);
    BYTESWAP8(shaContextPtr->shaDigest[2]);
    BYTESWAP8(shaContextPtr->shaDigest[3]);
    BYTESWAP8(shaContextPtr->shaDigest[4]);
    BYTESWAP8(shaContextPtr->shaDigest[5]);
#endif /* TM_LITTLE_ENDIAN */

/* truncate determines the size of the digest.  if(truncate) only uses
 * 384 bits of the digest, while !truncate uses the full 512 bits.
 */
    if (!sha384truncate)
    {
        copyLen = 64;
#ifdef TM_LITTLE_ENDIAN
        BYTESWAP8(shaContextPtr->shaDigest[6]);
        BYTESWAP8(shaContextPtr->shaDigest[7]);
#endif /* TM_LITTLE_ENDIAN */
    }
    else
    {
        copyLen = 48;
    }
    tm_memcpy(outputPtr, shaContextPtr->shaDigest, copyLen);
/* Zeroise sensitive stuff */
    tm_memset((tt8BitPtr)shaContextPtr, 0, sizeof(ttSha512Ctx));
}

#ifdef TM_USE_SHA384
/* SHA384 uses a SHA512Context as the real context.
 * The only differences between SHA384 an SHA512 are:
 * a) the intialization values for the context, and
 * b) the number of bytes of data produced as output.
 */

void tfSha384Init(ttVoidPtr sha384ContextPtr)
{
    ttSha384CtxPtr shaContextPtr;
    shaContextPtr = (ttSha384CtxPtr)sha384ContextPtr;
    tm_memset(shaContextPtr, 0, sizeof(ttSha512Ctx));
    tm_memcpy(shaContextPtr->shaDigest, H384, sizeof(H384));
}

void tfSha384Update(ttVoidPtr  sha384ContextPtr,
                    tt8BitPtr  input,
		            ttPktLen   count,
                    ttPktLen   offset)
{
    tfSha512Update(sha384ContextPtr, input, count, offset);
}

void tfSha384Final(tt8BitPtr  outputPtr,
                   ttVoidPtr  sha384ContextPtr)
{
    tfSha2Final(outputPtr, sha384ContextPtr, TM_8BIT_YES);
}
#endif /* TM_USE_SHA384 */
#else /* !(TM_USE_SHA384 || TM_USE_SHA512) */
LINT_UNUSED_HEADER

/* To allow link for builds when none of TM_USE_SHA256, TM_USE_SHA384,
 * and TM_USE_SHA512 are defined
 */
int tvSha512Dummy = 0;
#endif /* TM_USE_SHA384 || TM_USE_SHA512 */

/***************** End Of File *****************/
