/*
 * Description: OpenSSL header files 
 *
 * Filename: tropnssl.h
 * Author: Jin Zhang
 * Date Created: 5/10/2002
 * $Source: include/tropnssl.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:33JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/* Modified from OPENSSL crypto/bn/bn_**.c  and crypto/dh/dh_**.c 
 * Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is tm_kernel_free for commercial and non-commercial use as
 * long as the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given 
 * attribution as the author of the parts of the library used.
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
 *    the apps directory (application code) you must include an 
 *    acknowledgement:
 *    "This product includes software written by Tim Hudson 
 *     (tjh@cryptsoft.com)"
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



/*$KAME: crypto_openssl.c, eaytest.c v 1.70 2001/09/11 14:21:12 sakane Exp $*/

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

#ifndef TM_OPTIMIZE_SIZE
#define TM_BN_MUL_COMBA
#define TM_BN_SQR_COMBA
#define TM_BN_RECURSION
#define TM_RECP_MUL_MOD
#define TM_MONT_MUL_MOD
#endif /*TM_OPTIMIZE_SIZE */



/* Pentium pro 16,16,16,32,64 */
/* Alpha       16,16,16,16.64 */
#define TM_BN_MULL_SIZE_NORMAL            (16) /* 32 */
#define TM_BN_MUL_RECURSIVE_SIZE_NORMAL   (16) /* 32 less than */
#define TM_BN_SQR_RECURSIVE_SIZE_NORMAL   (16) /* 32 */
#define TM_BN_MUL_LOW_RECUR_SIZE_NORMAL   (32) /* 32 */



#define TM_OPENSSL_NULL     (ttVoidPtr)0
#define tm_bn_winbits_for_exponent_size(b) \
        ((b) > 671 ? 6 : \
         (b) > 239 ? 5 : \
         (b) >  79 ? 4 : \
         (b) >  23 ? 3 : 1)
/*bn.h*/

#define     tm_bn_lbits(a)    ((a)&TM_BN_MASK2l)
#define     tm_bn_hbits(a)    (((a)>>TM_BN_BITS4)&TM_BN_MASK2l)
#define     tm_bn_l2hbits(a)    ((ttBN_ULONG)((a)&TM_BN_MASK2l)<<TM_BN_BITS4)


#define     tm_bn_mul64(l,h,bl,bh) \
{ \
    lt=l; \
    ht=h; \
    m =(bh)*(lt); \
    lt=(bl)*(lt); \
    m1=(bl)*(ht); \
    ht =(bh)*(ht); \
    m=(m+m1)&TM_BN_MASK2; if (m < m1) ht+=tm_bn_l2hbits(1L); \
    ht+=tm_bn_hbits(m); \
    m1=tm_bn_l2hbits(m); \
    lt=(lt+m1)&TM_BN_MASK2; if (lt < m1) ht++; \
    (l)=lt; \
    (h)=ht; \
}


#define tm_bn_sqr64(lo,ho,in) \
{ \
    h=(in); \
    l=tm_bn_lbits(h); \
    h=tm_bn_hbits(h); \
    m =(l)*(h); \
    l*=l; \
    h*=h; \
    h+=(m&TM_BN_MASK2h1)>>(TM_BN_BITS4-1); \
    m =(m&TM_BN_MASK2l)<<(TM_BN_BITS4+1); \
    l=(l+m)&TM_BN_MASK2; if (l < m) h++; \
    (lo)=l; \
    (ho)=h; \
}

#define tm_bn_mul_add(r,a,bl,bh,c) { \
    h= (a); \
    l=tm_bn_lbits(h); \
    h=tm_bn_hbits(h); \
    tm_bn_mul64(l,h,(bl),(bh)); \
 \
    /* non-multiply part */ \
    l=(l+(c))&TM_BN_MASK2; if (l < (c)) h++; \
    (c)=(r); \
    l=(l+(c))&TM_BN_MASK2; if (l < (c)) h++; \
    (c)=h&TM_BN_MASK2; \
    (r)=l; \
}

#define tm_bn_mul(r,a,bl,bh,c) { \
    h= (a); \
    l=tm_bn_lbits(h); \
    h=tm_bn_hbits(h); \
    tm_bn_mul64(l,h,(bl),(bh)); \
 \
    /* non-multiply part */ \
    l+=(c); if ((l&TM_BN_MASK2) < (c)) h++; \
    (c)=h&TM_BN_MASK2; \
    (r)=l&TM_BN_MASK2; \
}


#ifdef SIXTY_FOUR_BIT_LONG
/* assuming long is 64bit - this is the DEC Alpha
 * unsigned long long is only 64 bits :-(, don't define
 * TM_BN_LLONG for the DEC Alpha */
#ifdef  TM_BN_LLONG
typedef unsigned long long ttBN_ULLONG;
#endif /* TM_BN_LLONG */
typedef unsigned long ttBN_ULONG;
#define TM_BN_BYTES     8
#define TM_BN_BITS2     64
#define TM_BN_BITS4     32
#define TM_BN_MASK     (0xffffffffffffffffffffffffffffffffLL)
#define TM_BN_MASK2    (0xffffffffffffffffL)
#define TM_BN_MASK2l   (0xffffffffL)
#define TM_BN_MASK2h   (0xffffffff00000000L)
#define TM_BN_MASK2h1  (0xffffffff80000000L)
#define TM_BN_TBIT     (0x8000000000000000L)
#endif /* SIXTY_FOUR_BIT_LONG */

#ifdef TM_DH_SIXTY_FOUR_BIT
/* This is where the long long data type is 64 bits, but long is 32.
 * For machines where there are 64bit registers, this is the mode to use.
 * IRIX, on R4000 and above should use this mode, along with the relevant
 * assembler code :-).  Do NOT define TM_BN_LLONG.
 */
#undef  TM_BN_LLONG
typedef unsigned long long ttBN_ULONG;
#define TM_BN_BYTES     8
#define TM_BN_BITS2     64
#define TM_BN_BITS4     32
#define TM_BN_MASK2    (0xffffffffffffffffLL)
#define TM_BN_MASK2l   (0xffffffffL)
#define TM_BN_MASK2h   (0xffffffff00000000LL)
#define TM_BN_MASK2h1  (0xffffffff80000000LL)
#define TM_BN_TBIT     (0x8000000000000000LL)
#endif /* TM_DH_SIXTY_FOUR_BIT */

#ifdef TM_DH_THIRTY_TWO_BIT
#if defined(WIN32) && !defined(__GNUC__)
#ifdef  TM_BN_LLONG
typedef unsigned _int64 ttBN_ULLONG;
#endif /* TM_BN_LLONG */
#else
#ifdef  TM_BN_LLONG
typedef unsigned long long ttBN_ULLONG;
#endif /* TM_BN_LLONG */
#endif
typedef tt32Bit ttBN_ULONG;
#define TM_BN_BYTES    4
#define TM_BN_BITS2    32
#define TM_BN_BITS4    16
#ifdef WIN32
/* VC++ doesn't like the LL suffix */
#define TM_BN_MASK        (0xffffffffffffffffL)
#else
#define TM_BN_MASK        (0xffffffffffffffffLL)
#endif
#define TM_BN_MASK2       TM_UL(0xffffffff)
#define TM_BN_MASK2l      TM_UL(0xffff)
#define TM_BN_MASK2h1     TM_UL(0xffff8000)
#define TM_BN_MASK2h      TM_UL(0xffff0000)
#define TM_BN_TBIT        TM_UL(0x80000000)
#endif

#ifdef TM_DH_SIXTEEN_BIT
#ifndef ttBN_DIV2W
#define ttBN_DIV2W
#endif
#ifdef  TM_BN_LLONG
typedef tt32Bit ttBN_ULLONG;
#endif /* TM_BN_LLONG */
typedef tt16Bit ttBN_ULONG;
#define TM_BN_BYTES     2
#define TM_BN_BITS2     16
#define TM_BN_BITS4     8
#define TM_BN_MASK      TM_UL(0xffffffff)
#define TM_BN_MASK2     TM_UL(0xffff)
#define TM_BN_MASK2l    TM_UL(0xff)
#define TM_BN_MASK2h1   TM_UL(0xff80)
#define TM_BN_MASK2h    TM_UL(0xff00)
#define TM_BN_TBIT      TM_UL(0x8000)
#endif

#ifdef TM_DH_EIGHT_BIT
#ifndef ttBN_DIV2W
#define ttBN_DIV2W
#endif
#ifdef  TM_BN_LLONG
typedef tt16Bit ttBN_ULLONG;
#endif /* TM_BN_LLONG */
typedef tt8Bit ttBN_ULONG;
#define TM_BN_BYTES     1
#define TM_BN_BITS2     8
#define TM_BN_BITS4     4
#define TM_BN_MASK      TM_UL(0xffff)
#define TM_BN_MASK2     TM_UL(0xff)
#define TM_BN_MASK2l    TM_UL(0xf)
#define TM_BN_MASK2h1   TM_UL(0xf8)
#define TM_BN_MASK2h    TM_UL(0xf0)
#define TM_BN_TBIT      TM_UL(0x80)
#endif

#ifdef  TM_BN_LLONG
typedef ttBN_ULLONG TM_FAR * ttBN_ULLONGPtr;
#endif /* TM_BN_LLONG */
typedef ttBN_ULONG  TM_FAR * ttBN_ULONGPtr;
typedef const ttBN_ULONG TM_FAR * ttConstBN_ULONGPtr;

#define TM_BN_FLG_MALLOCED         0x01
#define TM_BN_FLG_STATIC_DATA      0x02
#define tm_bn_get_flags(b,n)       ((b)->flags&(n))
#define TM_BN_CTX_NUM                 16
#define TM_BN_CTX_NUM_POS             12



#define tm_bn_to_montgomery(r,a,mont,ctx)    tfbn_mod_mul_montgomery(\
    r,a,&((mont)->RR),(mont),ctx)

#define tm_bn_num_bytes(a)    ((tm_bn_num_bits(a)+7)/8)
#define tm_bn_is_word(a,w)    (((a)->top == 1) && ((a)->d[0] == (ttBN_ULONG)(w)))
#define tm_bn_is_zero(a)      (((a)->top == 0) || tm_bn_is_word(a,0))
#define tm_bn_is_one(a)       (tm_bn_is_word((a),1))
#define tm_bn_is_odd(a)       (((a)->top > 0) && ((a)->d[0] & 1))
#define tm_bn_one(a)          (tm_bn_set_word((a),1))
#define tm_bn_zero(a)         (tm_bn_set_word((a),0))



#define tm_bn_expand(a,bits) \
    (((((((bits)+TM_BN_BITS2-1))/TM_BN_BITS2)) <= (a)->dmax)?   \
    (a):tfbn_expand2((a),(bits)/TM_BN_BITS2+1))

#define tm_bn_wexpand(a,words) \
    (((words) <= (a)->dmax)?(a):tfbn_expand2((a),(words)))

#define tm_bn_wexpand2(a,words) \
    if ((words) > (a)->dmax) tfbn_expand2((a),(words))


#define TM_DH_FLAG_CACHE_MONT_P    0x01
/* may need to set the DH_new()->flags */




#define tm_bn_fix_top(a) \
{ \
    ttBN_ULONGPtr ftl;      \
    if ((a)->top > 0)     \
    { \
            for (ftl= &((a)->d[(a)->top-1]); (a)->top > 0; (a)->top--) \
            { \
                if( *ftl ) \
                { \
                    break; \
                } \
                ftl--; \
            } \
    } \
}



typedef struct tsbignumber_st
{
    ttBN_ULONGPtr d;    /* Pointer to an array of 'TM_BN_BITS2' bit chunks. */
    int top;    /* Index of last used d +1. */
    /* The next are internal book keeping for tm_bn_expand. */
    int dmax;    /* Size of the d array. */
    int neg;    /* one if the number is negative */
    int flags;
}ttBIGNUMBER;
typedef ttBIGNUMBER TM_FAR * ttBIGNUMBERPtr;
typedef const ttBIGNUMBER TM_FAR * ttConstBIGNUMBERPtr;
typedef ttBIGNUMBERPtr TM_FAR * ttBIGNUMBERPtrPtr;

typedef struct tsbn_ctx_st
{
    int tos;
    ttBIGNUMBER bn[TM_BN_CTX_NUM];
    int flags;
    int depth;
    int pos[TM_BN_CTX_NUM_POS];
    int too_many;
} ttBN_CTX; /*big number context*/
typedef ttBN_CTX TM_FAR * ttBN_CTXPtr;

typedef struct tsbn_blinding_st
{
    int init;
    ttBIGNUMBERPtr A;
    ttBIGNUMBERPtr Ai;
    ttBIGNUMBERPtr mod; /* just a reference */
} ttBN_BLINDING;
typedef ttBN_BLINDING TM_FAR * ttBN_BLINDINGPtr;

/* Used for montgomery multiplication */
typedef struct tsbn_mont_ctx_st
{
    int ri;        /* number of bits in R */
    ttBIGNUMBER RR;     /* used to convert to montgomery form */
    ttBIGNUMBER N;      /* The modulus */
    ttBIGNUMBER Ni;     /* R*(1/R mod N) - N*Ni = 1
                    * (Ni is only stored for ttBIGNUMBER algorithm) */
    ttBN_ULONG n0;   /* least significant word of Ni */
    int flags;
}ttBN_MONT_CTX;
typedef ttBN_MONT_CTX TM_FAR * ttBN_MONT_CTXPtr;

/* Used for reciprocal division/mod functions
 * It cannot be shared between threads
 */
typedef struct tsbn_recp_ctx_st
{
    ttBIGNUMBER N;    /* the divisor */
    ttBIGNUMBER Nr;    /* the reciprocal */
    int num_bits;
    int shift;
    int flags;
} ttBN_RECP_CTX;/*ttBN_RECP_CTX;*/
typedef ttBN_RECP_CTX TM_FAR * ttBN_RECP_CTXPtr;


/*
 * Diffie Hellman structure
 */


typedef struct tsDhInBn /* Diffie Hellman in Big number format */
{
    /* This first argument is used to pick up errors when
     * a ttDhInBn is passed instead of a EVP_PKEY */
    int     length;
    int     flags;

    ttBIGNUMBERPtr p;   /* prime number */
    ttBIGNUMBERPtr g;   /* generator */
    ttBIGNUMBERPtr pub_key;    /* g^x */
    ttBIGNUMBERPtr priv_key;    /* x */

    ttCharPtr method_mont_p;

} ttDhInBn;
typedef ttDhInBn TM_FAR * ttDhInBnPtr;


#ifdef TM_BN_MUL_COMBA
#ifdef TM_BN_LLONG
#define tm_mul_add_c(a,b,c0,c1,c2) \
    t=(ttBN_ULLONG)a*b; \
    t1=(ttBN_ULONG)Lw(t); \
    t2=(ttBN_ULONG)Hw(t); \
    c0=(c0+t1)&TM_BN_MASK2; if ((c0) < t1) t2++; \
    c1=(c1+t2)&TM_BN_MASK2; if ((c1) < t2) c2++;

#define tm_mul_add_c2(a,b,c0,c1,c2) \
    t=(ttBN_ULLONG)a*b; \
    tt=(t+t)&TM_BN_MASK; \
    if (tt < t) c2++; \
    t1=(ttBN_ULONG)Lw(tt); \
    t2=(ttBN_ULONG)Hw(tt); \
    c0=(c0+t1)&TM_BN_MASK2;  \
    if ((c0 < t1) && (((++t2)&TM_BN_MASK2) == 0)) c2++; \
    c1=(c1+t2)&TM_BN_MASK2; if ((c1) < t2) c2++;

#define tm_sqr_add_c(a,i,c0,c1,c2) \
    t=(ttBN_ULLONG)a[i]*a[i]; \
    t1=(ttBN_ULONG)Lw(t); \
    t2=(ttBN_ULONG)Hw(t); \
    c0=(c0+t1)&TM_BN_MASK2; if ((c0) < t1) t2++; \
    c1=(c1+t2)&TM_BN_MASK2; if ((c1) < t2) c2++;

#define tm_sqr_add_c2(a,i,j,c0,c1,c2) \
    tm_mul_add_c2((a)[i],(a)[j],c0,c1,c2)

#elif defined(TM_BN_UMULT_HIGH)

#define tm_mul_add_c(a,b,c0,c1,c2){   \
    ta=(a);                           \
    tb=(b);                           \
    t1 = ta * tb;                     \
    t2 = TM_BN_UMULT_HIGH(ta,tb);     \
    c0 += t1; t2 += (c0<t1)?1:0;      \
    c1 += t2; c2 += (c1<t2)?1:0;      \
}

#define tm_mul_add_c2(a,b,c0,c1,c2) { \
    ta = (a);                         \
    tb = (b);                         \
    t1 = TM_BN_UMULT_HIGH(ta,tb);     \
    t0 = ta * tb;                     \
    t2 = t1+t1; c2 += (t2<t1)?1:0;    \
    t1 = t0+t0; t2 += (t1<t0)?1:0;    \
    c0 += t1; t2 += (c0<t1)?1:0;      \
    c1 += t2; c2 += (c1<t2)?1:0;      \
}

#define tm_sqr_add_c(a,i,c0,c1,c2){   \
    ta = (a)[i];                      \
    t1 = ta * ta;                     \
    t2 = TM_BN_UMULT_HIGH(ta,ta);     \
    c0 += t1; t2 += (c0<t1)?1:0;      \
    c1 += t2; c2 += (c1<t2)?1:0;      \
}

#define tm_sqr_add_c2(a,i,j,c0,c1,c2)    \
    tm_mul_add_c2((a)[i],(a)[j],c0,c1,c2)

#else /* !TM_BN_LLONG */
#define tm_mul_add_c(a,b,c0,c1,c2) \
    t1=tm_bn_lbits(a); t2=tm_bn_hbits(a); \
    bl=tm_bn_lbits(b); bh=tm_bn_hbits(b); \
    tm_bn_mul64(t1,t2,bl,bh); \
    c0=(c0+t1)&TM_BN_MASK2; if ((c0) < t1) t2++; \
    c1=(c1+t2)&TM_BN_MASK2; if ((c1) < t2) c2++;

#define tm_mul_add_c2(a,b,c0,c1,c2) \
    t1=tm_bn_lbits(a); t2=tm_bn_hbits(a); \
    bl=tm_bn_lbits(b); bh=tm_bn_hbits(b); \
    tm_bn_mul64(t1,t2,bl,bh); \
    if (t2 & TM_BN_TBIT) c2++; \
    t2=(t2+t2)&TM_BN_MASK2; \
    if (t1 & TM_BN_TBIT) t2++; \
    t1=(t1+t1)&TM_BN_MASK2; \
    c0=(c0+t1)&TM_BN_MASK2;  \
    if ((c0 < t1) && (((++t2)&TM_BN_MASK2) == 0)) c2++; \
    c1=(c1+t2)&TM_BN_MASK2; if ((c1) < t2) c2++;

#define tm_sqr_add_c(a,i,c0,c1,c2) \
    tm_bn_sqr64(t1,t2,(a)[i]); \
    c0=(c0+t1)&TM_BN_MASK2; if ((c0) < t1) t2++; \
    c1=(c1+t2)&TM_BN_MASK2; if ((c1) < t2) c2++;

#define tm_sqr_add_c2(a,i,j,c0,c1,c2) \
    tm_mul_add_c2((a)[i],(a)[j],c0,c1,c2)
#endif /* !TM_BN_LLONG */

#endif /* TM_BN_MUL_COMBA */


/* Moved here from trbignum.c */
TM_PROTO_EXTERN ttBIGNUMBERPtr      tfBN_value_one(void);
TM_PROTO_EXTERN void                tfBN_clear_free(ttBIGNUMBERPtr a);
TM_PROTO_EXTERN ttBIGNUMBERPtr      tfBN_copy(ttBIGNUMBERPtr a,
                                              ttConstBIGNUMBERPtr b);
TM_PROTO_EXTERN int                 tfBN_sub(ttBIGNUMBERPtr r,
                                             ttConstBIGNUMBERPtr a, 
                                             ttConstBIGNUMBERPtr b);
TM_PROTO_EXTERN int                 tfBN_usub(ttBIGNUMBERPtr r,
                                              ttConstBIGNUMBERPtr a,
                                              ttConstBIGNUMBERPtr b);
TM_PROTO_EXTERN int                 tfBN_add(ttBIGNUMBERPtr r,
                                             ttConstBIGNUMBERPtr a, 
                                             ttConstBIGNUMBERPtr b);
TM_PROTO_EXTERN int                 tfBN_mod(ttBIGNUMBERPtr rem,
                                             ttConstBIGNUMBERPtr m, 
                                             ttConstBIGNUMBERPtr d,
                                             ttBN_CTXPtr ctx);
TM_PROTO_EXTERN int                 tfBN_div(ttBIGNUMBERPtr dv,
                                             ttBIGNUMBERPtr rem,
                                             ttConstBIGNUMBERPtr m,
                                             ttConstBIGNUMBERPtr d,
                                             ttBN_CTXPtr ctx);
TM_PROTO_EXTERN int                 tfBN_mul(ttBIGNUMBERPtr r,
                                             ttBIGNUMBERPtr a,
                                             ttBIGNUMBERPtr b,
                                             ttBN_CTXPtr ctx);
TM_PROTO_EXTERN int                 tfBN_sqr(ttBIGNUMBERPtr r,
                                             ttBIGNUMBERPtr a,
                                             ttBN_CTXPtr ctx);
TM_PROTO_EXTERN int                 tfBN_mul_word(ttBIGNUMBERPtr a,
                                                  ttBN_ULONG w);
TM_PROTO_EXTERN int                 tfBN_add_word(ttBIGNUMBERPtr a,
                                                  ttBN_ULONG w);
TM_PROTO_EXTERN int                 tfBN_sub_word(ttBIGNUMBERPtr a,
                                                  ttBN_ULONG w);
TM_PROTO_EXTERN int                 tfBN_cmp(ttConstBIGNUMBERPtr a,
                                             ttConstBIGNUMBERPtr b);
TM_PROTO_EXTERN int                 tfBN_is_bit_set(ttConstBIGNUMBERPtr a,
                                                    int n);
TM_PROTO_EXTERN int                 tfBN_lshift(ttBIGNUMBERPtr r,
                                                ttConstBIGNUMBERPtr a,
                                                int n);
TM_PROTO_EXTERN int                 tfBN_lshift1(ttBIGNUMBERPtr r,
                                                 ttBIGNUMBERPtr a);
TM_PROTO_EXTERN int                 tfBN_rshift(ttBIGNUMBERPtr r,
                                                ttBIGNUMBERPtr a,
                                                int n);
TM_PROTO_EXTERN int                 tfBN_ucmp(ttConstBIGNUMBERPtr a,
                                              ttConstBIGNUMBERPtr b);
TM_PROTO_EXTERN int                 tfBN_set_bit(ttBIGNUMBERPtr a,
                                                 int n);
TM_PROTO_EXTERN ttBIGNUMBERPtr      tfBN_mod_inverse(ttBIGNUMBERPtr ret,
                                                     ttBIGNUMBERPtr a,
                                                     ttConstBIGNUMBERPtr n,
                                                     ttBN_CTXPtr ctx);
TM_PROTO_EXTERN ttBIGNUMBERPtr      tfbn_expand2(ttBIGNUMBERPtr a, int words);
TM_PROTO_EXTERN ttBN_ULONG          tfbn_mul_add_words(ttBN_ULONGPtr rp,
                                                       ttBN_ULONGPtr ap,
                                                       int num,
                                                       ttBN_ULONG w);
TM_PROTO_EXTERN ttBN_ULONG          tfbn_add_words(ttBN_ULONGPtr rp,
                                                   ttBN_ULONGPtr ap,
                                                   ttBN_ULONGPtr bp,
                                                   int num);
/* Moved here from trbignu2.c */
TM_PROTO_EXTERN int                 tfBN_mod_exp(ttBIGNUMBERPtr r,
                                                 ttBIGNUMBERPtr a,
                                                 ttConstBIGNUMBERPtr p,
                                                 ttConstBIGNUMBERPtr m,
                                                 ttBN_CTXPtr ctx);
TM_PROTO_EXTERN int                 tfBN_mod_exp2_mont(ttBIGNUMBERPtr r,
                                                       ttBIGNUMBERPtr a1,
                                                       ttBIGNUMBERPtr p1,
                                                       ttBIGNUMBERPtr a2,
                                                       ttBIGNUMBERPtr p2,
                                                       ttBIGNUMBERPtr m,
                                                       ttBN_CTXPtr ctx,
                                                       ttBN_MONT_CTXPtr m_ctx);
TM_PROTO_EXTERN int                 tfBN_mod_mul(ttBIGNUMBERPtr ret,
                                                 ttBIGNUMBERPtr a,
                                                 ttBIGNUMBERPtr b,
                                                 ttConstBIGNUMBERPtr m,
                                                 ttBN_CTXPtr ctx);
TM_PROTO_EXTERN void                tfBN_MONT_CTX_free(ttBN_MONT_CTXPtr mont);
TM_PROTO_EXTERN ttBIGNUMBERPtr      tfBN_new(void);
TM_PROTO_EXTERN void                tfBN_init(ttBIGNUMBERPtr);
TM_PROTO_EXTERN void                tfBN_free(ttBIGNUMBERPtr a);
TM_PROTO_EXTERN void                tfBN_CTX_init(ttBN_CTXPtr c);
ttBN_MONT_CTXPtr                    tfBN_MONT_CTX_new(void);
TM_PROTO_EXTERN int                 tfBN_MONT_CTX_set(
                                            ttBN_MONT_CTXPtr mont,
                                            ttConstBIGNUMBERPtr modulus,
                                            ttBN_CTXPtr ctx);
TM_PROTO_EXTERN int                 tm_bn_set_word(ttBIGNUMBERPtr a,
                                                   ttBN_ULONG w);
TM_PROTO_EXTERN int                 tm_bn_num_bits(ttConstBIGNUMBERPtr a);
TM_PROTO_EXTERN int                 tfBN_rand(ttBIGNUMBERPtr rnd,
                                              int bits,
                                              int top,
                                              int bottom);
TM_PROTO_EXTERN void                tfBN_CTX_start(ttBN_CTXPtr ctx);
TM_PROTO_EXTERN ttBIGNUMBERPtr      tfBN_CTX_get(ttBN_CTXPtr ctx);
TM_PROTO_EXTERN ttBN_CTXPtr         tfBN_CTX_new(void);
TM_PROTO_EXTERN void                tfBN_CTX_free(ttBN_CTXPtr c);
TM_PROTO_EXTERN void                tfBN_CTX_end(ttBN_CTXPtr ctx);
TM_PROTO_EXTERN int                 tfBN_bn2bin(ttConstBIGNUMBERPtr a,
                                                tt8BitPtr to);
TM_PROTO_EXTERN ttBIGNUMBERPtr tfBN_bin2bn(ttConst8BitPtr s,
                                           int len,
                                           ttBIGNUMBERPtr ret);
TM_PROTO_EXTERN int                 tfeay_v2bn(ttBIGNUMBERPtrPtr bn,
                                               ttCharVectorPtr var);
TM_PROTO_EXTERN int                 tfeay_bn2v(ttCharVectorPtrPtr var,
                                               ttBIGNUMBERPtr bn);
TM_PROTO_EXTERN int                 tfBN_mod_exp_mont_word(
                                                ttBIGNUMBERPtr r,
                                                ttBN_ULONG a,
                                                ttConstBIGNUMBERPtr p,
                                                ttConstBIGNUMBERPtr m,
                                                ttBN_CTXPtr ctx,
                                                ttBN_MONT_CTXPtr m_ctx);
TM_PROTO_EXTERN int                 tfBN_mod_exp_mont(ttBIGNUMBERPtr r,
                                                      ttBIGNUMBERPtr a,
                                                      ttConstBIGNUMBERPtr p,
                                                      ttConstBIGNUMBERPtr m,
                                                      ttBN_CTXPtr ctx,
                                                      ttBN_MONT_CTXPtr m_ctx);

#ifdef TM_USE_PKI
TM_PROTO_EXTERN int                 tfBN_BLINDING_update(ttBN_BLINDINGPtr b,
                                                         ttBN_CTXPtr ctx);
TM_PROTO_EXTERN int                 tfBN_BLINDING_convert(ttBIGNUMBERPtr n,
                                                          ttBN_BLINDINGPtr r,
                                                          ttBN_CTXPtr ctx);
TM_PROTO_EXTERN int                 tfBN_BLINDING_invert(ttBIGNUMBERPtr n,
                                                         ttBN_BLINDINGPtr b,
                                                         ttBN_CTXPtr ctx);
TM_PROTO_EXTERN void                tfBN_BLINDING_free(ttBN_BLINDINGPtr r);
TM_PROTO_EXTERN ttBN_BLINDINGPtr    tfBN_BLINDING_new(ttBIGNUMBERPtr A,
                                                      ttBIGNUMBERPtr Ai,
                                                      ttBIGNUMBERPtr mod);

#define     tm_bn_check_top(a)
#define     tm_BNerr(a, b)

TM_PROTO_EXTERN ttBIGNUMBERPtr      tfEuclid(ttBIGNUMBERPtr a,
                                             ttBIGNUMBERPtr b);
TM_PROTO_EXTERN int                 tfBN_gcd(ttBIGNUMBERPtr r,
                                             ttBIGNUMBERPtr in_a,
                                             ttBIGNUMBERPtr in_b,
                                             ttBN_CTXPtr ctx);
TM_PROTO_EXTERN int                 tfBN_pseudo_rand_range(
                                             ttBIGNUMBERPtr rnd,
                                             ttBIGNUMBERPtr range);
TM_PROTO_EXTERN int                 tfBN_rand_range(ttBIGNUMBERPtr r,
                                                    ttBIGNUMBERPtr range);
TM_PROTO_EXTERN int                 tfBN_mask_bits(ttBIGNUMBERPtr a,int n);
TM_PROTO_EXTERN ttBIGNUMBERPtr      tfBN_dup(ttConstBIGNUMBERPtr a);

/* definition for RSA, DSA, ASN1 and X509, may move to a new head file as 
 * it is very different from openssl */


typedef struct tsRsaMethSt ttRsaMethod;
typedef const ttRsaMethod TM_FAR * ttRsaMethodPtr;
typedef struct tsRsaSt
{
    ttRsaMethodPtr meth;
    ttBIGNUMBERPtr n;
    ttBIGNUMBERPtr e;
    ttBIGNUMBERPtr d;
    ttBIGNUMBERPtr p;
    ttBIGNUMBERPtr q;
    ttBIGNUMBERPtr dmp1;
    ttBIGNUMBERPtr dmq1;
    ttBIGNUMBERPtr iqmp;

    int flags;

/* Used to cache montgomery values */
    ttBN_MONT_CTXPtr _method_mod_n;
    ttBN_MONT_CTXPtr _method_mod_p;
    ttBN_MONT_CTXPtr _method_mod_q;
        
/* all ttBIGNUMBER values are actually in the following data, if it is not
 * TM_OPENSSL_NULL */
    ttBN_BLINDINGPtr blinding;
} ttRSA;

typedef ttRSA  TM_FAR * ttRSAPtr;

typedef struct tsDsaMethod ttDsaMethod;
typedef const ttDsaMethod TM_FAR * ttDsaMethodPtr;
typedef struct tsDsaSt
{
    ttBIGNUMBERPtr p;
    ttBIGNUMBERPtr q;    /* == 20 */
    ttBIGNUMBERPtr g;

    ttBIGNUMBERPtr pub_key;  /* y public key */
    ttBIGNUMBERPtr priv_key; /* x private key */

    ttBIGNUMBERPtr kinv;    /* Signing pre-calc */
    ttBIGNUMBERPtr r;    /* Signing pre-calc */

    int flags;
    /* Normally used to cache montgomery values */
    ttCharPtr method_mont_p;
    
    ttDsaMethodPtr   meth;

} ttDSA;
typedef ttDSA  TM_FAR * ttDSAPtr;

typedef struct tsDSASIGst
{
    ttBIGNUMBERPtr r;
    ttBIGNUMBERPtr s;
} ttDsaSig;
typedef ttDsaSig TM_FAR * ttDsaSigPtr;


/* new functions */
TM_PROTO_EXTERN ttRSAPtr    tfRsaNew(void);
TM_PROTO_EXTERN void        tfRsaFree (ttRSAPtr r);
TM_PROTO_EXTERN int         tfRsaSize(ttRSAPtr r);
TM_PROTO_EXTERN ttDSAPtr    tfDsaNew(void);
TM_PROTO_EXTERN void        tfDsaFree(ttDSAPtr r);
TM_PROTO_EXTERN int         tfDsaSize(ttDSAPtr r);

/* API for RSA and DSA */
/* RSA signature and verify, return:0- verify success */

TM_PROTO_EXTERN int tfRSASign           (   int             flen, 
                                            tt8BitPtr       strPtr, 
                                            tt8BitPtr       sigPtr, 
                                            ttVoidPtr       rsaPtr,
                                            ttIntPtr        siglenPtr );
TM_PROTO_EXTERN int tfRSAVerify         (   int             flen, 
                                            tt8BitPtr       strPtr, 
                                            tt8BitPtr       sigPtr, 
                                            ttVoidPtr       rsaPtr,
                                            int             siglen ); 

TM_PROTO_EXTERN int tfRSAPublicEncrypt(     int        flen, 
                                            tt8BitPtr  from, 
                                            tt8BitPtr  to,
                                            ttVoidPtr  rsa, 
                                            ttIntPtr   siglen);
TM_PROTO_EXTERN int tfRSAPrivateDecrypt(    int       flen, 
                                            tt8BitPtr from, 
                                            tt8BitPtr to,
                                            ttVoidPtr rsa, 
                                            ttIntPtr  siglen);
TM_PROTO_EXTERN int tfRSAPublicDecrypt(     int flen,
                                            tt8BitPtr from,
                                            tt8BitPtr to,
                                            ttVoidPtr rsa,
                                            ttIntPtr siglen);
TM_PROTO_EXTERN int tfRSAPrivateEncrypt(    int flen,
                                            tt8BitPtr from,
                                            tt8BitPtr to,
                                            ttVoidPtr rsa,
                                            ttIntPtr siglen);
TM_PROTO_EXTERN int tfDSASign (             int             dgst_len, 
                                            tt8BitPtr       dgst, 
                                            tt8BitPtr       sigbuf, 
                                            ttDSAPtr        dsa,
                                            ttIntPtr        siglenPtr );
TM_PROTO_EXTERN int tfDSAVerify (           int             dgst_len, 
                                            tt8BitPtr       dgst, 
                                            tt8BitPtr       sigbuf, 
                                            ttDSAPtr        dsa,
                                            int             siglen);

#endif /* TM_USE_PKI */
