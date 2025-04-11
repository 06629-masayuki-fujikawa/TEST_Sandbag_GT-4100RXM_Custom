/*
 * $Source: source/cryptlib/trmd5.c $
 *
 * Modification History
 * $Revision: 6.0.2.4 $
 * $Date: 2010/08/17 05:38:12JST $
 * $Author: pcarney $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


/*
 ***********************************************************************
 ** md5.c -- the source code for MD5 routines                         **
 ** RSA Data Security, Inc. MD5 Message-Digest Algorithm              **
 ** Created: 2/17/90 RLR                                              **
 ** Revised: 1/91 SRD,AJ,BSK,JT Reference C ver., 7/10 constant corr. **
 ***********************************************************************
 */

/*
 ***********************************************************************
 ** Copyright (C) 1990, RSA Data Security, Inc. All rights reserved.  **
 **                                                                   **
 ** License to copy and use this software is granted provided that    **
 ** it is identified as the "RSA Data Security, Inc. MD5 Message-     **
 ** Digest Algorithm" in all material mentioning or referencing this  **
 ** software or this function.                                        **
 **                                                                   **
 ** License is also granted to make and use derivative works          **
 ** provided that such works are identified as "derived from the RSA  **
 ** Data Security, Inc. MD5 Message-Digest Algorithm" in all          **
 ** material mentioning or referencing the derived work.              **
 **                                                                   **
 ** RSA Data Security, Inc. makes no representations concerning       **
 ** either the merchantability of this software or the suitability    **
 ** of this software for any particular purpose.  It is provided "as  **
 ** is" without express or implied warranty of any kind.              **
 **                                                                   **
 ** These notices must be retained in any copies of any part of this  **
 ** documentation and/or software.                                    **
 ***********************************************************************
 */


/*
 ***********************************************************************
 **  Message-digest routines:                                         **
 **  To form the message digest for a message M                       **
 **    (1) Initialize a context buffer mdContextPtr using tfMd5Init   **
 **    (2) Call tfMd5Update on mdContextPtr and M                     **
 **    (3) Call tfMd5Final on mdContextPtr                            **
 **  The message digest is now in mdContext->digest[0...15]           **
 ***********************************************************************
 */

#include <trsocket.h>
#include <trmacro.h>

/*
 * Modified by Treck Inc. to comply with coding standards
 */

#ifdef TM_USE_MD5
#include <trtype.h>
#include <trproto.h>

/* Define unreferenced macro for PC-LINT compile error suppression */
#ifdef TM_LINT
LINT_UNREF_MACRO(UL)
#endif /* TM_LINT */


/* forward declaration */
static void tfMd5Transform (tt32BitPtr bufPtr,
                            tt32BitPtr inPtr);

static const tt8Bit  TM_CONST_QLF tlPadding[64] = {
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* F, G, H and I are basic MD5 functions */
#define tm_md5_f(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define tm_md5_g(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define tm_md5_h(x, y, z) ((x) ^ (y) ^ (z))
#define tm_md5_i(x, y, z) ((y) ^ ((x) | (~z)))

/* tm_md5_rotate_left rotates x left n bits */
#define tm_md5_rotate_left(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* tm_md5_ff, tm_md5_gg, tm_md5_hh, and tm_md5_ii transformations for rounds 1, 2, 3, and 4 */
/* Rotation is separate from addition to prevent recomputation */
#define tm_md5_ff(a, b, c, d, x, s, ac) \
  {(a) += tm_md5_f ((b), (c), (d)) + (x) + (tt32Bit)(ac); \
   (a) = tm_md5_rotate_left ((a), (s)); \
   (a) += (b); \
  }
#define tm_md5_gg(a, b, c, d, x, s, ac) \
  {(a) += tm_md5_g ((b), (c), (d)) + (x) + (tt32Bit)(ac); \
   (a) = tm_md5_rotate_left ((a), (s)); \
   (a) += (b); \
  }
#define tm_md5_hh(a, b, c, d, x, s, ac) \
  {(a) += tm_md5_h ((b), (c), (d)) + (x) + (tt32Bit)(ac); \
   (a) = tm_md5_rotate_left ((a), (s)); \
   (a) += (b); \
  }
#define tm_md5_ii(a, b, c, d, x, s, ac) \
  {(a) += tm_md5_i ((b), (c), (d)) + (x) + (tt32Bit)(ac); \
   (a) = tm_md5_rotate_left ((a), (s)); \
   (a) += (b); \
  }

#ifdef TM_DSP
#define tm_md5_get_char_next(buffer, offset) \
((*(buffer) >> ((TM_DSP_ROUND_PTR - \
               ((offset) % TM_DSP_BYTES_PER_WORD)) << 3)) & 0xff); \
{ \
    if ( (++(offset) % TM_DSP_BYTES_PER_WORD) == 0) \
    { \
        offset=0; \
        (buffer)++; \
    } \
}
#else /* !TM_DSP */
#define tm_md5_get_char_next(buffer, offset)  *(buffer)++;
#endif /* TM_DSP */

#ifdef __STDC__
#define UL(x)   x##U
#else
#define UL(x)   x
#endif

/* The routine tfMd5Init initializes the message-digest context
   mdContextPtr. All fields are set to zero.
 */
void tfMd5Init (ttVoidPtr contextPtr)
{
  ttMd5CtxPtr mdContextPtr;
  mdContextPtr = (ttMd5CtxPtr)contextPtr;

  mdContextPtr->md5I[0] = mdContextPtr->md5I[1] = TM_UL(0);

  /* Load magic initialization constants.
   */
  mdContextPtr->md5Buf[0] = TM_UL(0x67452301);
  mdContextPtr->md5Buf[1] = TM_UL(0xefcdab89);
  mdContextPtr->md5Buf[2] = TM_UL(0x98badcfe);
  mdContextPtr->md5Buf[3] = TM_UL(0x10325476);
}

/* The routine tfMd5Update updates the message-digest context to
   account for the presence of each of the characters inBufPtr[0..inLen-1]
   in the message whose digest is being computed.
 */
void tfMd5Update (ttVoidPtr contextPtr,
                  tt8BitPtr inBufPtr,
                  ttPktLen  inLen,
                  ttPktLen  offset)
{
  tt32Bit  in[16];
  unsigned mdi;
  unsigned i;
  unsigned ii;
  ttMd5CtxPtr mdContextPtr;

  mdContextPtr = (ttMd5CtxPtr)contextPtr;

  /* compute number of bytes mod 64 */
  mdi = (unsigned)((mdContextPtr->md5I[0] >> 3) & 0x3F);

  /* update number of bits */
  if ((mdContextPtr->md5I[0] + ((tt32Bit)inLen << 3)) < mdContextPtr->md5I[0])
    mdContextPtr->md5I[1]++;
  mdContextPtr->md5I[0] += ((tt32Bit)inLen << 3);
  mdContextPtr->md5I[1] += ((tt32Bit)inLen >> 29);

  while (inLen) {
    /* add new character to buffer, increment mdi */
      inLen--;
      if(offset == TM_DSP_UNPACKED_CHAR)
      {
          mdContextPtr->md5In[mdi++] = *inBufPtr++;
      }
      else
      {
          mdContextPtr->md5In[mdi++] = tm_md5_get_char_next(inBufPtr,offset);
      }

    /* transform if necessary */
    if (mdi == 0x40) {
      for (i = 0, ii = 0; i < 16; i++, ii += 4)
        in[i] = (((tt32Bit)mdContextPtr->md5In[ii+3]) << 24) |
                (((tt32Bit)mdContextPtr->md5In[ii+2]) << 16) |
                (((tt32Bit)mdContextPtr->md5In[ii+1]) << 8) |
                ((tt32Bit)mdContextPtr->md5In[ii]);
      tfMd5Transform (mdContextPtr->md5Buf, in);
      mdi = 0;
    }
  }
}

/* The routine MD5Final terminates the message-digest computation and
   ends with the desired message digest in digestPtr[0...15].
 */
void tfMd5Final (tt8BitPtr digestPtr, ttVoidPtr contextPtr)
{
  tt32Bit   in[16];
  unsigned  mdi;
  unsigned  i;
  unsigned  ii;
  ttPktLen  padLen;

  ttMd5CtxPtr mdContextPtr;
  mdContextPtr = (ttMd5CtxPtr)contextPtr;

  /* save number of bits */
  in[14] = mdContextPtr->md5I[0];
  in[15] = mdContextPtr->md5I[1];

  /* compute number of bytes mod 64 */
  mdi = (unsigned)((mdContextPtr->md5I[0] >> 3) & 0x3F);

  /* pad out to 56 mod 64 */
  padLen = (ttPktLen)((mdi < 56) ? (56 - mdi) : (120 - mdi));
  tfMd5Update ((ttVoidPtr)mdContextPtr,
               (tt8BitPtr)tlPadding,
               padLen,
               TM_DSP_UNPACKED_CHAR);

  /* append length in bits and transform */
  for (i = 0, ii = 0; i < 14; i++, ii += 4)
    in[i] = (((tt32Bit)mdContextPtr->md5In[ii+3]) << 24) |
            (((tt32Bit)mdContextPtr->md5In[ii+2]) << 16) |
            (((tt32Bit)mdContextPtr->md5In[ii+1]) << 8) |
            ((tt32Bit)mdContextPtr->md5In[ii]);
  tfMd5Transform (mdContextPtr->md5Buf, in);

  /* store buffer in digest */
  for (i = 0, ii = 0; i < 4; i++, ii += 4) {
      digestPtr[ii]   = tm_8bit(mdContextPtr->md5Buf[i]);
      digestPtr[ii+1] = tm_8bit(mdContextPtr->md5Buf[i] >> 8);
      digestPtr[ii+2] = tm_8bit(mdContextPtr->md5Buf[i] >> 16);
      digestPtr[ii+3] = tm_8bit(mdContextPtr->md5Buf[i] >> 24);
  }
}

/*
 * Basic MD5 step. tfMd5Transforms bufPtr based on inPtr.
 */
static void tfMd5Transform (tt32BitPtr bufPtr,
                            tt32BitPtr inPtr)
{
  tt32Bit a = bufPtr[0], b = bufPtr[1], c = bufPtr[2], d = bufPtr[3];

  /* Round 1 */
#define TM_MD5_S11 7
#define TM_MD5_S12 12
#define TM_MD5_S13 17
#define TM_MD5_S14 22
  tm_md5_ff ( a, b, c, d, inPtr[ 0], TM_MD5_S11, TM_UL(3614090360)); /* 1 */
  tm_md5_ff ( d, a, b, c, inPtr[ 1], TM_MD5_S12, TM_UL(3905402710)); /* 2 */
  tm_md5_ff ( c, d, a, b, inPtr[ 2], TM_MD5_S13,  TM_UL(606105819)); /* 3 */
  tm_md5_ff ( b, c, d, a, inPtr[ 3], TM_MD5_S14, TM_UL(3250441966)); /* 4 */
  tm_md5_ff ( a, b, c, d, inPtr[ 4], TM_MD5_S11, TM_UL(4118548399)); /* 5 */
  tm_md5_ff ( d, a, b, c, inPtr[ 5], TM_MD5_S12, TM_UL(1200080426)); /* 6 */
  tm_md5_ff ( c, d, a, b, inPtr[ 6], TM_MD5_S13, TM_UL(2821735955)); /* 7 */
  tm_md5_ff ( b, c, d, a, inPtr[ 7], TM_MD5_S14, TM_UL(4249261313)); /* 8 */
  tm_md5_ff ( a, b, c, d, inPtr[ 8], TM_MD5_S11, TM_UL(1770035416)); /* 9 */
  tm_md5_ff ( d, a, b, c, inPtr[ 9], TM_MD5_S12, TM_UL(2336552879)); /* 10 */
  tm_md5_ff ( c, d, a, b, inPtr[10], TM_MD5_S13, TM_UL(4294925233)); /* 11 */
  tm_md5_ff ( b, c, d, a, inPtr[11], TM_MD5_S14, TM_UL(2304563134)); /* 12 */
  tm_md5_ff ( a, b, c, d, inPtr[12], TM_MD5_S11, TM_UL(1804603682)); /* 13 */
  tm_md5_ff ( d, a, b, c, inPtr[13], TM_MD5_S12, TM_UL(4254626195)); /* 14 */
  tm_md5_ff ( c, d, a, b, inPtr[14], TM_MD5_S13, TM_UL(2792965006)); /* 15 */
  tm_md5_ff ( b, c, d, a, inPtr[15], TM_MD5_S14, TM_UL(1236535329)); /* 16 */

  /* Round 2 */
#define TM_MD5_S21 5
#define TM_MD5_S22 9
#define TM_MD5_S23 14
#define TM_MD5_S24 20
  tm_md5_gg ( a, b, c, d, inPtr[ 1], TM_MD5_S21, TM_UL(4129170786)); /* 17 */
  tm_md5_gg ( d, a, b, c, inPtr[ 6], TM_MD5_S22, TM_UL(3225465664)); /* 18 */
  tm_md5_gg ( c, d, a, b, inPtr[11], TM_MD5_S23,  TM_UL(643717713)); /* 19 */
  tm_md5_gg ( b, c, d, a, inPtr[ 0], TM_MD5_S24, TM_UL(3921069994)); /* 20 */
  tm_md5_gg ( a, b, c, d, inPtr[ 5], TM_MD5_S21, TM_UL(3593408605)); /* 21 */
  tm_md5_gg ( d, a, b, c, inPtr[10], TM_MD5_S22,   TM_UL(38016083)); /* 22 */
  tm_md5_gg ( c, d, a, b, inPtr[15], TM_MD5_S23, TM_UL(3634488961)); /* 23 */
  tm_md5_gg ( b, c, d, a, inPtr[ 4], TM_MD5_S24, TM_UL(3889429448)); /* 24 */
  tm_md5_gg ( a, b, c, d, inPtr[ 9], TM_MD5_S21,  TM_UL(568446438)); /* 25 */
  tm_md5_gg ( d, a, b, c, inPtr[14], TM_MD5_S22, TM_UL(3275163606)); /* 26 */
  tm_md5_gg ( c, d, a, b, inPtr[ 3], TM_MD5_S23, TM_UL(4107603335)); /* 27 */
  tm_md5_gg ( b, c, d, a, inPtr[ 8], TM_MD5_S24, TM_UL(1163531501)); /* 28 */
  tm_md5_gg ( a, b, c, d, inPtr[13], TM_MD5_S21, TM_UL(2850285829)); /* 29 */
  tm_md5_gg ( d, a, b, c, inPtr[ 2], TM_MD5_S22, TM_UL(4243563512)); /* 30 */
  tm_md5_gg ( c, d, a, b, inPtr[ 7], TM_MD5_S23, TM_UL(1735328473)); /* 31 */
  tm_md5_gg ( b, c, d, a, inPtr[12], TM_MD5_S24, TM_UL(2368359562)); /* 32 */

  /* Round 3 */
#define TM_MD5_S31 4
#define TM_MD5_S32 11
#define TM_MD5_S33 16
#define TM_MD5_S34 23
  tm_md5_hh ( a, b, c, d, inPtr[ 5], TM_MD5_S31, TM_UL(4294588738)); /* 33 */
  tm_md5_hh ( d, a, b, c, inPtr[ 8], TM_MD5_S32, TM_UL(2272392833)); /* 34 */
  tm_md5_hh ( c, d, a, b, inPtr[11], TM_MD5_S33, TM_UL(1839030562)); /* 35 */
  tm_md5_hh ( b, c, d, a, inPtr[14], TM_MD5_S34, TM_UL(4259657740)); /* 36 */
  tm_md5_hh ( a, b, c, d, inPtr[ 1], TM_MD5_S31, TM_UL(2763975236)); /* 37 */
  tm_md5_hh ( d, a, b, c, inPtr[ 4], TM_MD5_S32, TM_UL(1272893353)); /* 38 */
  tm_md5_hh ( c, d, a, b, inPtr[ 7], TM_MD5_S33, TM_UL(4139469664)); /* 39 */
  tm_md5_hh ( b, c, d, a, inPtr[10], TM_MD5_S34, TM_UL(3200236656)); /* 40 */
  tm_md5_hh ( a, b, c, d, inPtr[13], TM_MD5_S31,  TM_UL(681279174)); /* 41 */
  tm_md5_hh ( d, a, b, c, inPtr[ 0], TM_MD5_S32, TM_UL(3936430074)); /* 42 */
  tm_md5_hh ( c, d, a, b, inPtr[ 3], TM_MD5_S33, TM_UL(3572445317)); /* 43 */
  tm_md5_hh ( b, c, d, a, inPtr[ 6], TM_MD5_S34,   TM_UL(76029189)); /* 44 */
  tm_md5_hh ( a, b, c, d, inPtr[ 9], TM_MD5_S31, TM_UL(3654602809)); /* 45 */
  tm_md5_hh ( d, a, b, c, inPtr[12], TM_MD5_S32, TM_UL(3873151461)); /* 46 */
  tm_md5_hh ( c, d, a, b, inPtr[15], TM_MD5_S33,  TM_UL(530742520)); /* 47 */
  tm_md5_hh ( b, c, d, a, inPtr[ 2], TM_MD5_S34, TM_UL(3299628645)); /* 48 */

  /* Round 4 */
#define TM_MD5_S41 6
#define TM_MD5_S42 10
#define TM_MD5_S43 15
#define TM_MD5_S44 21
  tm_md5_ii ( a, b, c, d, inPtr[ 0], TM_MD5_S41, TM_UL(4096336452)); /* 49 */
  tm_md5_ii ( d, a, b, c, inPtr[ 7], TM_MD5_S42, TM_UL(1126891415)); /* 50 */
  tm_md5_ii ( c, d, a, b, inPtr[14], TM_MD5_S43, TM_UL(2878612391)); /* 51 */
  tm_md5_ii ( b, c, d, a, inPtr[ 5], TM_MD5_S44, TM_UL(4237533241)); /* 52 */
  tm_md5_ii ( a, b, c, d, inPtr[12], TM_MD5_S41, TM_UL(1700485571)); /* 53 */
  tm_md5_ii ( d, a, b, c, inPtr[ 3], TM_MD5_S42, TM_UL(2399980690)); /* 54 */
  tm_md5_ii ( c, d, a, b, inPtr[10], TM_MD5_S43, TM_UL(4293915773)); /* 55 */
  tm_md5_ii ( b, c, d, a, inPtr[ 1], TM_MD5_S44, TM_UL(2240044497)); /* 56 */
  tm_md5_ii ( a, b, c, d, inPtr[ 8], TM_MD5_S41, TM_UL(1873313359)); /* 57 */
  tm_md5_ii ( d, a, b, c, inPtr[15], TM_MD5_S42, TM_UL(4264355552)); /* 58 */
  tm_md5_ii ( c, d, a, b, inPtr[ 6], TM_MD5_S43, TM_UL(2734768916)); /* 59 */
  tm_md5_ii ( b, c, d, a, inPtr[13], TM_MD5_S44, TM_UL(1309151649)); /* 60 */
  tm_md5_ii ( a, b, c, d, inPtr[ 4], TM_MD5_S41, TM_UL(4149444226)); /* 61 */
  tm_md5_ii ( d, a, b, c, inPtr[11], TM_MD5_S42, TM_UL(3174756917)); /* 62 */
  tm_md5_ii ( c, d, a, b, inPtr[ 2], TM_MD5_S43,  TM_UL(718787259)); /* 63 */
  tm_md5_ii ( b, c, d, a, inPtr[ 9], TM_MD5_S44, TM_UL(3951481745)); /* 64 */

  bufPtr[0] += a;
  bufPtr[1] += b;
  bufPtr[2] += c;
  bufPtr[3] += d;
}

void tfHmacMd5(tt8BitPtr    dataPtr,
               int          dataLen,
               tt8BitPtr    hashKeyPtr,
               tt8BitPtr    digestPtr,
               int          digestLen)
{
    ttMd5Ctx    md5Ctx;
    tt8Bit      k1[64];
    tt8Bit      k2[64];
    tt8Bit      buf[16];
    int         i;

    tm_memset(k1, 0x36, 64);
    tm_memset(k2, 0x5c, 64);

    for (i = 0; i < 16; i++)
    {
        k1[i] ^= (tt8Bit) (hashKeyPtr[i]);
        k2[i] ^= (tt8Bit) (hashKeyPtr[i]);
    }

    tfMd5Init(&md5Ctx);
    tfMd5Update(&md5Ctx, k1, 64, 0);
    tfMd5Update(&md5Ctx, dataPtr, (ttPktLen)dataLen, 0);
    tfMd5Final(buf, &md5Ctx);

    tfMd5Init(&md5Ctx);
    tfMd5Update(&md5Ctx, k2, 64, 0);
    tfMd5Update(&md5Ctx, buf, 16, 0);
    tfMd5Final(buf, &md5Ctx);

    if (digestLen > 16)
    {
        digestLen = 16;
    }
    tm_memcpy(digestPtr, buf, digestLen);
    return;
}


void tfMd5Checksum(tt8BitPtr    dataPtr,
                   int          dataLen,
                   tt8BitPtr    digestPtr,
                   int          digestLen)
{
    ttMd5Ctx    md5Ctx;
    tt8Bit      buf[16];

    tfMd5Init(&md5Ctx);
    tfMd5Update(&md5Ctx, dataPtr, (ttPktLen)dataLen, 0);
    tfMd5Final(buf, &md5Ctx);

    if (digestLen > 16)
    {
        digestLen = 16;
    }
    tm_memcpy(digestPtr, buf, digestLen);
    return;
}


#else /* ! TM_USE_MD5 */

LINT_UNUSED_HEADER

/* To allow link for builds when TM_USE_MD5 is not defined */
int tvMd5Dummy = 0;
#endif /* TM_USE_MD5 */

/***************** End Of File *****************/

