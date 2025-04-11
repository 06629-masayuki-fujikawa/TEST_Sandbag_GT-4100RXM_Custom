/*
 * Description: OpenSSL big number lib (part 1 of 2)
 *
 * Filename: trbignum.c
 * Author: Jin Zhang
 * Date Created: 5/10/2002
 * $Source: source/cryptlib/trbignum.c $
 *
 * Modification History
 * $Revision: 6.0.2.10 $
 * $Date: 2012/11/29 04:45:46JST $
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
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
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


#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>
#include <trcrylib.h>
#include <tropnssl.h>


#if (defined(TM_PUBKEY_USE_DIFFIEHELLMAN) || \
     defined(TM_PUBKEY_USE_DSA)     || \
     defined(TM_PUBKEY_USE_RSA) )


/*Big number library API */

static int          tfbn_num_bits_word(ttBN_ULONG l);
/* library internal functions */
static void         tfbn_mul_normal(ttBN_ULONGPtr r,
                                    ttBN_ULONGPtr a,
                                    int na,
                                    ttBN_ULONGPtr b,
                                    int nb);
#ifdef TM_BN_MUL_COMBA
static void         tfbn_mul_comba8(ttBN_ULONGPtr r,
                                    ttBN_ULONGPtr a,
                                    ttBN_ULONGPtr b);
static void         tfbn_mul_comba4(ttBN_ULONGPtr r,
                                    ttBN_ULONGPtr a,
                                    ttBN_ULONGPtr b);
#endif /* TM_BN_MUL_COMBA */
static void         tfbn_sqr_normal(ttBN_ULONGPtr r,
                                    ttBN_ULONGPtr a,
                                    int n,
                                    ttBN_ULONGPtr tmp);
#ifdef TM_BN_SQR_COMBA
static void         tfbn_sqr_comba8(ttBN_ULONGPtr r,ttBN_ULONGPtr a);
static void         tfbn_sqr_comba4(ttBN_ULONGPtr r,ttBN_ULONGPtr a);
#endif /* TM_BN_SQR_COMBA */

#ifdef TM_BN_RECURSION
static void         tfbn_mul_recursive(ttBN_ULONGPtr r,
                                       ttBN_ULONGPtr a,
                                       ttBN_ULONGPtr b,
                                       int n2,
                                       ttBN_ULONGPtr t);
static void         tfbn_mul_part_recursive(ttBN_ULONGPtr r,
                                            ttBN_ULONGPtr a,
                                            ttBN_ULONGPtr b,
                                            int tn,
                                            int n,
                                            ttBN_ULONGPtr t);
static void         tfbn_sqr_recursive(ttBN_ULONGPtr r,
                                       ttBN_ULONGPtr a,
                                       int n2,
                                       ttBN_ULONGPtr t);
#endif /* TM_BN_RECURSION */

static ttBN_ULONG     tfbn_mul_words(ttBN_ULONGPtr rp,
                                     ttBN_ULONGPtr ap,
                                     int num,
                                     ttBN_ULONG w);
static void           tfbn_sqr_words(ttBN_ULONGPtr rp,
                                     ttBN_ULONGPtr ap,
                                     int num);
static ttBN_ULONG     tfbn_div_words(ttBN_ULONG h, ttBN_ULONG l, ttBN_ULONG d);

#ifdef TM_USE_PKI
static int            tfBN_pseudo_rand(ttBIGNUMBERPtr rnd,
                                       int bits,
                                       int top,
                                       int bottom);
static int            tfbn2_rand_range(int pseudo,
                                       ttBIGNUMBERPtr r,
                                       ttBIGNUMBERPtr range);
#endif /* TM_USE_PKI */

#ifndef TM_OPTIMIZE_SIZE
static int          tfbn_cmp_words(ttBN_ULONGPtr a,ttBN_ULONGPtr b,int n);
static ttBN_ULONG   tfbn_sub_words(ttBN_ULONGPtr rp,
                                   ttBN_ULONGPtr ap,
                                   ttBN_ULONGPtr bp,
                                   int num);
#endif /* TM_OPTIMIZE_SIZE */

#ifndef TM_OPTIMIZE_SIZE
static int tfbn_cmp_words(ttBN_ULONGPtr a, ttBN_ULONGPtr b, int n)
{
    int i;
    ttBN_ULONG aa,bb;

    aa=a[n-1];
    bb=b[n-1];
    if (aa != bb) return((aa > bb)?1:-1);
    for (i=n-2; i>=0; i--)
    {
        aa=a[i];
        bb=b[i];
        if (aa != bb) return((aa > bb)?1:-1);
    }
    return(0);
}

static ttBN_ULONG tfbn_sub_words(
    ttBN_ULONGPtr r, ttBN_ULONGPtr a, ttBN_ULONGPtr b, int n)
{
    ttBN_ULONG t1,t2;
    int c=0;


    if (n <= 0) return((ttBN_ULONG)0);

    for (;;)
    {
        t1=a[0]; t2=b[0];
        r[0]=(t1-t2-(ttBN_ULONG)c)&TM_BN_MASK2;
        if (t1 != t2) c=(t1 < t2);
        n--;
        if (n <= 0) break;

        t1=a[1]; t2=b[1];
        r[1]=(t1-t2-(ttBN_ULONG)c)&TM_BN_MASK2;
        if (t1 != t2) c=(t1 < t2);
        n--;
        if (n <= 0) break;

        t1=a[2]; t2=b[2];
        r[2]=(t1-t2-(ttBN_ULONG)c)&TM_BN_MASK2;
        if (t1 != t2) c=(t1 < t2);
        n--;
        if (n <= 0) break;

        t1=a[3]; t2=b[3];
        r[3]=(t1-t2-(ttBN_ULONG)c)&TM_BN_MASK2;
        if (t1 != t2) c=(t1 < t2);
        n--;
        if (n <= 0) break;

        a+=4;
        b+=4;
        r+=4;
    }
    return(c);
}

#endif /* TM_OPTIMIZE_SIZE */

/*
 * Big number <-> string interexchange
 */


/**BN_lib.c ------------------------------------------*/
/* For a 32 bit machine
 * 2 -   4 ==  128
 * 3 -   8 ==  256
 * 4 -  16 ==  512
 * 5 -  32 == 1024
 * 6 -  64 == 2048
 * 7 - 128 == 4096
 * 8 - 256 == 8192
 */

ttBIGNUMBERPtr  tfBN_value_one(void)
{
    static ttBN_ULONG data_one;
    static ttBIGNUMBER const_one;

    data_one = (ttBN_ULONG)1;
    const_one.d = &data_one;
    const_one.top = 1;
    const_one.dmax = 1;
    const_one.neg = 0;
    const_one.flags = 0;

    return(&const_one);
}

static const tt8Bit TM_CONST_QLF tlBnNumBitsWord[256]={
    0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
};
static int tfbn_num_bits_word(ttBN_ULONG l)
{
#if defined(TM_DH_SIXTY_FOUR_BIT_LONG)
    if (l & 0xffffffff00000000L)
    {
        if (l & 0xffff000000000000L)
        {
            if (l & 0xff00000000000000L)
            {
                return((int) tlBnNumBitsWord[(int)(l>>56)]+56);
            }
            else
            {
                return((int) tlBnNumBitsWord[(int)(l>>48)]+48);
            }
        }
        else
        {
            if (l & 0x0000ff0000000000L)
            {
                return((int) tlBnNumBitsWord[(int)(l>>40)]+40);
            }
            else
            {
                return((int) tlBnNumBitsWord[(int)(l>>32)]+32);
            }
        }
    }
    else
#else
#ifdef TM_DH_SIXTY_FOUR_BIT
    if (l & 0xffffffff00000000LL)
    {
        if (l & 0xffff000000000000LL)
        {
            if (l & 0xff00000000000000LL)
            {
                return((int) tlBnNumBitsWord[(int)(l>>56)]+56);
            }
            else
            {
                return((int) tlBnNumBitsWord[(int)(l>>48)]+48);
            }
        }
        else
        {
            if (l & 0x0000ff0000000000LL)
            {
                return((int) tlBnNumBitsWord[(int)(l>>40)]+40);
            }
            else
            {
                return((int) tlBnNumBitsWord[(int)(l>>32)]+32);
            }
        }
    }
    else
#endif
#endif
    {
#if defined(TM_DH_THIRTY_TWO_BIT) || defined(TM_DH_SIXTY_FOUR_BIT) || \
        defined(TM_DH_SIXTY_FOUR_BIT_LONG)
        if (l & TM_UL(0xffff0000))
        {
            if (l & TM_UL(0xff000000))
            {
                return((int) tlBnNumBitsWord[(int)(l>>(unsigned)24)]+24);
            }
            else
            {
                return((int) tlBnNumBitsWord[(int)(l>>(unsigned)16)]+16);
            }
        }
        else
#endif
        {
#if defined(TM_DH_SIXTEEN_BIT) || defined(TM_DH_THIRTY_TWO_BIT) || \
            defined(TM_DH_SIXTY_FOUR_BIT) || \
            defined(TM_DH_SIXTY_FOUR_BIT_LONG)

            if (l & (ttBN_ULONG) 0xff00)
            {
                return((int) tlBnNumBitsWord[(int)(l>>8)]+8);
            }
            else    
#endif
                return((int) tlBnNumBitsWord[(int)(l   )]  );
        }
    }
}

int tm_bn_num_bits(ttConstBIGNUMBERPtr a)
{
    ttBN_ULONG l;
    int i;


    if (a->top == 0) return(0);
    l=a->d[a->top-1];
    i=(a->top-1)*TM_BN_BITS2;
    return(i+tfbn_num_bits_word(l));
}

void tfBN_clear_free(ttBIGNUMBERPtr a)
{
    int i;

    if (a == TM_OPENSSL_NULL) return;
    if (a->d != TM_OPENSSL_NULL)
    {
        tm_bzero(a->d,(unsigned int)(a->dmax)*sizeof(a->d[0]));
        if (!(tm_bn_get_flags(a,TM_BN_FLG_STATIC_DATA)))
            tm_free_raw_buffer((tt8BitPtr)a->d);
    }
    i=tm_bn_get_flags(a,TM_BN_FLG_MALLOCED);
/* commented by jin shang, because bzero erase flag TM_BN_FLG_MALLOCED, 
 *           see tfBN_free */
   /* tm_bzero(a,sizeof(ttBIGNUMBER)); 
    if (i)
        tfBN_free(a);  
    */
    tm_bzero(a,sizeof(ttBIGNUMBER));
    if (i)
        tm_free_raw_buffer((tt8BitPtr)a); 
}

void tfBN_free(ttBIGNUMBERPtr a)
{
    if (a == TM_OPENSSL_NULL) return;
    if ((a->d != TM_OPENSSL_NULL)
        && !(tm_bn_get_flags(a,TM_BN_FLG_STATIC_DATA)))
        tm_free_raw_buffer((tt8BitPtr)a->d);

    if (a->flags & TM_BN_FLG_MALLOCED)
        tm_free_raw_buffer((tt8BitPtr)a);
}

void tfBN_init(ttBIGNUMBERPtr a)
{
    tm_bzero(a,sizeof(ttBIGNUMBER));
}

ttBIGNUMBERPtr  tfBN_new(void)
{
    ttBIGNUMBERPtr ret;

    ret=(ttBIGNUMBERPtr )tm_get_raw_buffer(sizeof(ttBIGNUMBER));
    if (ret == TM_OPENSSL_NULL)
    {
        return(TM_OPENSSL_NULL);
    }
    ret->flags=TM_BN_FLG_MALLOCED;
    ret->top=0;
    ret->neg=0;
    ret->dmax=0;
    ret->d=TM_OPENSSL_NULL;
    return(ret);
}

/* This is an internal function that should not be used in applications.
 * It ensures that 'b' has enough room for a 'words' word number number.
 * It is mostly used by the various ttBIGNUMBER routines. If there is an error,
 * TM_OPENSSL_NULL is returned. If not, 'b' is returned. */

ttBIGNUMBERPtr tfbn_expand2(ttBIGNUMBERPtr b, int words)
{
    ttBN_ULONGPtr A, a;
    ttConstBN_ULONGPtr B;
    int i;


    if (words > b->dmax)
    {
        if ((tt32Bit)words > (TM_INTEGER_MAX/(4*TM_BN_BITS2)))
        {
            return TM_OPENSSL_NULL;
        }
            
        if (tm_bn_get_flags(b,TM_BN_FLG_STATIC_DATA))
        {
            return(TM_OPENSSL_NULL);
        }
        A=(ttBN_ULONGPtr )tm_get_raw_buffer(sizeof(ttBN_ULONG)*(ttPktLen)(words+1));
        a = A;
        if (A == TM_OPENSSL_NULL)
        {
            return(TM_OPENSSL_NULL);
        }
        B=b->d;
        /* Check if the previous number needs to be copied */
        if (B != TM_OPENSSL_NULL)
        {
            for (i=b->top>>2; i>0; i--,A+=4,B+=4)
            {
                /*
                 * The fact that the loop is unrolled
                 * 4-wise is a tribute to Intel. It's
                 * the one that doesn't have enough
                 * registers to accomodate more data.
                 * I'd unroll it 8-wise otherwise:-)
                 *
                 *        <appro@fy.chalmers.se>
                 */
                ttBN_ULONG a0,a1,a2,a3;
                a0=B[0]; a1=B[1]; a2=B[2]; a3=B[3];
                A[0]=a0; A[1]=a1; A[2]=a2; A[3]=a3;
            }
            switch (b->top&3)
            {
                case 3:    A[2]=B[2];
                case 2:    A[1]=B[1];
                case 1:    A[0]=B[0];
                case 0:    ; /* ultrix cc workaround, see above */
                default:   break;
            }
            tm_free_raw_buffer((tt8BitPtr)b->d);
        }

        b->d=a;
        b->dmax=words;

        /* Now need to zero any data between b->top and b->max */

        A= &(b->d[b->top]);
        for (i=(b->dmax - b->top)>>3; i>0; i--,A+=8)
        {
            A[0]=0; A[1]=0; A[2]=0; A[3]=0;
            A[4]=0; A[5]=0; A[6]=0; A[7]=0;
        }
        for (i=(b->dmax - b->top)&7; i>0; i--,A++)
            A[0]=0;
        

    }
    return(b);
}

ttBIGNUMBERPtr  tfBN_copy(ttBIGNUMBERPtr a, ttConstBIGNUMBERPtr b)
{
    int i;
    ttBN_ULONGPtr A;
    ttConstBN_ULONGPtr B;
    ttBIGNUMBERPtr     bn;


    if (a == b) return(a);
    bn = tm_bn_wexpand(a,b->top);
    if (bn == TM_OPENSSL_NULL) return(TM_OPENSSL_NULL);

    A=a->d;
    B=b->d;
    for (i=b->top>>2; i>0; i--,A+=4,B+=4)
    {
        ttBN_ULONG a0,a1,a2,a3;
        a0=B[0]; a1=B[1]; a2=B[2]; a3=B[3];
        A[0]=a0; A[1]=a1; A[2]=a2; A[3]=a3;
    }
    switch (b->top&3)
    {
        case 3: A[2]=B[2];
        case 2: A[1]=B[1];
        case 1: A[0]=B[0];
        case 0: ; /* ultrix cc workaround, see comments in tfbn_expand2 */
        default: break;
    }


    a->top=b->top;
    if ((a->top == 0) && (a->d != TM_OPENSSL_NULL))
        a->d[0]=0;
    a->neg=b->neg;
    return(a);
}

int tm_bn_set_word(ttBIGNUMBERPtr a, ttBN_ULONG w)
{

    ttBIGNUMBERPtr bn;

    bn = tm_bn_expand( a, (int)(sizeof(ttBN_ULONG)*8) );
    if( bn == TM_OPENSSL_NULL )
    {
        return(0);
    }

    a->neg=0;
    a->top=0;
    a->d[0]=(ttBN_ULONG)w&TM_BN_MASK2;
    if (a->d[0] != 0) a->top=1;

    return(1);
}

/* ignore negative */
ttBIGNUMBERPtr  tfBN_bin2bn(ttConst8BitPtr s, int len, ttBIGNUMBERPtr ret)
{
    ttPktLen    i,m;
    ttPktLen    n;
    ttBN_ULONG  l;
    ttBIGNUMBERPtr bn;

    if (ret == TM_OPENSSL_NULL) ret= tfBN_new();
    if (ret == TM_OPENSSL_NULL) return(TM_OPENSSL_NULL);
    l= (ttBN_ULONG) 0;
    n= (ttPktLen) len;
    if (n == (ttPktLen) 0)
    {
        ret->top=0;
        return(ret);
    }
    bn = tm_bn_expand(ret,(int)(n)*8);
    if (bn == TM_OPENSSL_NULL)
    {
        tfBN_free(ret);
        return(TM_OPENSSL_NULL);
    }
    i= (ttPktLen) (((n-1)/TM_BN_BYTES)+1);
    m= (ttPktLen) ((n-1)%(TM_BN_BYTES));
    ret->top = (int)i;

    while( n > (ttPktLen)0 )
    {
        n--;
        l= (ttBN_ULONG) ((l << 8)| *(s++));
        if (m == (ttPktLen) 0)
        {
            m--;
            ret->d[--i]=l;
            l= (ttBN_ULONG) 0;
            m= (ttPktLen) (TM_BN_BYTES-1);
        }
        else
        {
            m--;
        }
    }
    /* need to call this due to clear byte at top if avoiding
     * having the top bit set (-ve number) */
    tm_bn_fix_top(ret);
    return(ret);
}

/* ignore negative */
int tfBN_bn2bin(ttConstBIGNUMBERPtr a, tt8BitPtr to)
{
    int n,i;
    ttBN_ULONG l;

    n=i=tm_bn_num_bytes(a);
    while( i > 0 )
    {
        i--;
        l=a->d[i/TM_BN_BYTES];
        *(to++)=tm_8bit(l>>(8*(i%TM_BN_BYTES)));
    }
    return(n);
}

int tfBN_ucmp(ttConstBIGNUMBERPtr a, ttConstBIGNUMBERPtr b)
{
    int i;
    ttBN_ULONG t1,t2;
    ttBN_ULONGPtr ap,bp;


    i=a->top-b->top;
    if (i != 0) return(i);
    ap=a->d;
    bp=b->d;
    for (i=a->top-1; i>=0; i--)
    {
        t1= ap[i];
        t2= bp[i];
        if (t1 != t2)
            return(t1 > t2?1:-1);
    }
    return(0);
}

int tfBN_cmp(ttConstBIGNUMBERPtr a, ttConstBIGNUMBERPtr b)
{
    int i;
    int gt,lt;
    ttBN_ULONG t1,t2;

    if ((a == TM_OPENSSL_NULL) || (b == TM_OPENSSL_NULL))
    {
        if (a != TM_OPENSSL_NULL)
            return(-1);
        else if (b != TM_OPENSSL_NULL)
            return(1);
        else
            return(0);
    }


    if (a->neg != b->neg)
    {
        if (a->neg)
            return(-1);
        else    return(1);
    }
    if (a->neg == 0)
    {
        gt=1; 
        lt= -1; 
    }
    else    { gt= -1; lt=1; }

    if (a->top > b->top) return(gt);
    if (a->top < b->top) return(lt);
    for (i=a->top-1; i>=0; i--)
    {
        t1=a->d[i];
        t2=b->d[i];
        if (t1 > t2) return(gt);
        if (t1 < t2) return(lt);
    }
    return(0);
}

int tfBN_set_bit(ttBIGNUMBERPtr a, int n)
{
    ttBN_ULONGPtr   dataPtr;
    int             i,j,k;
    ttBIGNUMBERPtr  bn;

    i=n/TM_BN_BITS2;
    j=n%TM_BN_BITS2;
    if (a->top <= i)
    {
        dataPtr = a->d;
        bn = tm_bn_wexpand(a,i+1);
        if (bn == TM_OPENSSL_NULL) return(0);
        if (dataPtr == a->d)
/*
 * No new re-allocation: need to initialize data memory between old top and
 * new top to zero.
 * (If we had to re-allocate, the data area between old top and new top has been
 *  initialized to zero in tfbn_expand2().)
 */
        {
            for(k=a->top; k<i+1; k++)
                a->d[k]=0;
        }
        a->top=i+1;
    }

    a->d[i]|=(((ttBN_ULONG)1)<<j);
    return(1);
}


int tfBN_is_bit_set(ttConstBIGNUMBERPtr a, int n)
{
    int i,j;

    if (n < 0) return(0);
    i=n/TM_BN_BITS2;
    j=n%TM_BN_BITS2;
    if (a->top <= i) return(0);
    return((a->d[i]&(((ttBN_ULONG)1)<<j))?1:0);
}

/* Add for RSA by jshang */
ttBN_CTXPtr  tfBN_CTX_new(void)
{
    ttBN_CTXPtr ret;

    ret=(ttBN_CTXPtr )tm_get_raw_buffer(sizeof(ttBN_CTX));
    if (ret == TM_OPENSSL_NULL)
    {
        return(TM_OPENSSL_NULL);
    }

    tfBN_CTX_init(ret);
    ret->flags=TM_BN_FLG_MALLOCED;
    return(ret);
}

void tfBN_CTX_init(ttBN_CTXPtr ctx)
{
    int i;
    ctx->tos = 0;
    ctx->flags = 0;
    ctx->depth = 0;
    ctx->too_many = 0;
    for (i = 0; i < TM_BN_CTX_NUM; i++)
        tfBN_init(&(ctx->bn[i]));
}

void tfBN_CTX_free(ttBN_CTXPtr ctx)
{
    int i;

    if (ctx == TM_OPENSSL_NULL) return;


    for (i=0; i < TM_BN_CTX_NUM; i++)
        tfBN_clear_free(&(ctx->bn[i]));
    if (ctx->flags & TM_BN_FLG_MALLOCED)
        tm_free_raw_buffer((tt8BitPtr)ctx);
}

void tfBN_CTX_start(ttBN_CTXPtr ctx)
{
    if (ctx->depth < TM_BN_CTX_NUM_POS)
        ctx->pos[ctx->depth] = ctx->tos;
    ctx->depth++;
}

ttBIGNUMBERPtr  tfBN_CTX_get(ttBN_CTXPtr ctx)
{
    int errCode;
    ttBIGNUMBERPtr retPtr;
    if (ctx->depth > TM_BN_CTX_NUM_POS || ctx->tos >= TM_BN_CTX_NUM)
    {
        if (!ctx->too_many)
        {
            ctx->too_many = 1;
        }
        return TM_OPENSSL_NULL;
    }
    retPtr = &(ctx->bn[ctx->tos++]);
    errCode = tm_bn_zero(retPtr);
    if (!errCode)
    {
        retPtr = TM_OPENSSL_NULL;
    }
    return retPtr;
}

void tfBN_CTX_end(ttBN_CTXPtr ctx)
{
    if (ctx == TM_OPENSSL_NULL) return;

    if (ctx->depth == 0)
        /* should never happen, but we can tolerate it if not in
         * debug mode (could be a 'goto err' in the calling function
         * before tfBN_CTX_start was reached) */
        tfBN_CTX_start(ctx);

    ctx->too_many = 0;
    ctx->depth--;
    if (ctx->depth < TM_BN_CTX_NUM_POS)
        ctx->tos = ctx->pos[ctx->depth];
}

static int tfbnrand(ttBIGNUMBERPtr rnd, int bits, int top, int bottom)
{
    tt8BitPtr buf=TM_OPENSSL_NULL;
    int ret=0,oneBit,bytes,mask,errCode;
    ttBIGNUMBERPtr bn;

    if (bits == 0)
    {
        errCode = tm_bn_zero(rnd);
        if (errCode)
        {
/* Success */
            ret = 1;
        }
        return ret;
    }

    bytes=(bits+7)/8;
    oneBit=(bits-1)%8;
    mask=0xff<<(oneBit+1);

    buf=(tt8BitPtr)tm_get_raw_buffer((ttPktLen)bytes);
    if (buf == TM_OPENSSL_NULL)
    {
        goto err;
    }

    /* make a random number and set the top and bottom bits */

    {
        tfGetRandomBytes(buf, bytes);
    }

    if (top != -1)
    {
        if (top)
        {
            if (oneBit == 0)
            {
                buf[0]=1;
                buf[1]|=(tt8Bit)0x80;
            }
            else
            {
                buf[0]|=(3<<(oneBit-1));
            }
        }
        else
        {
            buf[0]|=(1<<oneBit);
        }
    }
    buf[0] &= (tt8Bit)(~mask);
    if (bottom) /* set bottom bit if requested */
        buf[bytes-1]|=1;
    bn = tfBN_bin2bn(buf,bytes,rnd);
    if (!bn ) goto err;
    ret=1;
err:
    if (buf != TM_OPENSSL_NULL)
    {
        tm_bzero(buf,bytes);
        tm_free_raw_buffer((tt8BitPtr)buf);
    }
    return(ret);
}

int     tfBN_rand(ttBIGNUMBERPtr rnd, int bits, int top, int bottom)
{
    return tfbnrand(rnd, bits, top, bottom);
}

#ifdef TM_USE_PKI
static int tfBN_pseudo_rand(ttBIGNUMBERPtr rnd, int bits, int top, int bottom)
{
    return tfbnrand(rnd, bits, top, bottom);
}

/* random number r:  0 <= r < range */
static int tfbn2_rand_range(int pseudo, ttBIGNUMBERPtr r, ttBIGNUMBERPtr range)
{
    int (*ttbn_rand_func)(ttBIGNUMBERPtr , int, int, int) =
        pseudo ? tfBN_pseudo_rand : tfBN_rand;
    int n;
    int cmp,cmp2;
    
    if (range->neg || tm_bn_is_zero(range))
    {
        return 0;
    }

    n = tm_bn_num_bits(range); /* n > 0 */

    /* tfBN_is_bit_set(range, n - 1) always holds */

    if (n == 1)
    {
        cmp = tm_bn_zero(r);
        if (!cmp) return 0;
    }
    else 
        cmp  = tfBN_is_bit_set(range, n - 2);
        cmp2 = tfBN_is_bit_set(range, n - 3);
        if (!cmp  && !cmp2 )
        {
        /* range = 100..._2,
         * so  3*range (= 11..._2)  is exactly one bit longer than  range */
            do
            {
                cmp = ttbn_rand_func(r, n + 1, -1, 0);
                if (!cmp) return 0;
            /* If  r < 3*range,  use  r := r MOD range
             * (which is either  r, r - range,  or  r - 2*range).
             * Otherwise, iterate once more.
             * Since  3*range = 11..._2, each iteration succeeds with
             * probability >= .75. */
                cmp = tfBN_cmp(r ,range);
                if ( cmp >= 0)
                {
                    cmp = tfBN_sub(r, r, range);
                    if (!cmp) return 0;
                    cmp = tfBN_cmp(r, range);
                    if (cmp >= 0)
                    {
                        cmp = tfBN_sub(r, r, range);
                        if (!cmp) return 0;
                    }
                }
                cmp = tfBN_cmp(r, range);
            }
            while ( cmp >= 0);
        }
        else
        {
            do
            {
            /* range = 11..._2  or  range = 101..._2 */
                cmp = ttbn_rand_func(r, n, -1, 0);
                if (!cmp) return 0;

                cmp = tfBN_cmp(r, range);
            }
            while (cmp >= 0);
        }

    return 1;
}

int    tfBN_rand_range(ttBIGNUMBERPtr r, ttBIGNUMBERPtr range)
{
    return tfbn2_rand_range(0, r, range);
}

int    tfBN_pseudo_rand_range(ttBIGNUMBERPtr r, ttBIGNUMBERPtr range)
{
    return tfbn2_rand_range(1, r, range);
}
#endif /* TM_USE_PKI */

int tfeay_bn2v(ttCharVectorPtrPtr var, ttBIGNUMBERPtr bn)
{
    *var = (ttCharVectorPtr)tm_get_raw_buffer(sizeof(ttCharVector));

    if (*var == TM_OPENSSL_NULL)
        return(-1);

    (*var)->cvValuePtr = (char*)tm_get_raw_buffer((ttPktLen)
                                                   bn->top * TM_BN_BYTES);
    if ((*var)->cvValuePtr == (char*)0)
    {
        tm_free_raw_buffer((tt8BitPtr)*var);
        return(-1);
    }
    (*var)->cvLength = (unsigned int)tfBN_bn2bin( 
                                              bn, 
                                              (tt8BitPtr)(*var)->cvValuePtr);

    return 0;
}

int tfeay_v2bn(ttBIGNUMBERPtrPtr bn, ttCharVectorPtr var)
{
    *bn = tfBN_bin2bn(
             (tt8BitPtr)var->cvValuePtr, (int)var->cvLength, TM_OPENSSL_NULL);
    if (*bn == TM_OPENSSL_NULL)
        return -1;

    return 0;
}

/*BN_div.c*/
int tfBN_div(ttBIGNUMBERPtr dv, ttBIGNUMBERPtr rm, ttConstBIGNUMBERPtr num, 
                  ttConstBIGNUMBERPtr divisor, ttBN_CTXPtr ctx)
{
    int norm_shift,i,j,loop;
    ttBIGNUMBER wnum;
    ttBIGNUMBERPtr tmp,snum,sdiv,res,bn;
    ttBN_ULONGPtr resp,wnump;
    ttBN_ULONG d0,d1;
#ifndef TM_BN_UMULT_HIGH
    ttBN_ULONG m, m1, lt, ht; /* for tm_bn_mul64 */
#endif /* !TM_BN_UMULT_HIGH */
    int num_n,div_n,cmp;
    int ret;


    ret = 0;
    if (tm_bn_is_zero(divisor))
    {
        return(ret);
    }

    cmp = tfBN_ucmp(num,divisor);
    if( cmp < 0 )
    {
        if (rm != TM_OPENSSL_NULL)
        {
            bn = tfBN_copy(rm,num);
            if( bn == TM_OPENSSL_NULL ) 
            {
                return(ret); 
            }
        }
        ret = 1;
        if (dv != TM_OPENSSL_NULL)
        {
            cmp = tm_bn_zero(dv);
            if (!cmp)
            {
                ret = 0;
            }
        }
        return(ret);
    }

    tfBN_CTX_start(ctx);
    tmp= tfBN_CTX_get(ctx);
    snum= tfBN_CTX_get(ctx);
    sdiv= tfBN_CTX_get(ctx);
    if ( (tmp == TM_OPENSSL_NULL) || (snum == TM_OPENSSL_NULL) ||
         (sdiv == TM_OPENSSL_NULL) )
    {
        goto err;
    }
    if (dv == TM_OPENSSL_NULL)
    {
        res= tfBN_CTX_get(ctx);
        if (res == TM_OPENSSL_NULL)
        {
            goto err;
        }
    }
    else
    {
        res=dv;
    }
    if (sdiv==TM_OPENSSL_NULL || res == TM_OPENSSL_NULL) goto err;
    tmp->neg=0;

    /* First we normalise the numbers */
    norm_shift=TM_BN_BITS2-((tm_bn_num_bits(divisor))%TM_BN_BITS2);
    cmp = tfBN_lshift(sdiv,divisor,norm_shift);
    if (!cmp) goto err;
    sdiv->neg=0;
    norm_shift+=TM_BN_BITS2;
    cmp = tfBN_lshift(snum,num,norm_shift);
    if (!cmp) goto err;
    snum->neg=0;
    div_n=sdiv->top;
    num_n=snum->top;
    loop=num_n-div_n;

    /* Lets setup a 'window' into snum
     * This is the part that corresponds to the current
     * 'area' being divided */
    tfBN_init(&wnum);
    wnum.d=     &(snum->d[loop]);
    wnum.top= div_n;
    wnum.dmax= snum->dmax+1; /* a bit of a lie */

    /* Get the top 2 words of sdiv */
    /* i=sdiv->top; */
    d0=sdiv->d[div_n-1];
    d1=(div_n == 1)?0:sdiv->d[div_n-2];

    /* pointer to the 'top' of snum */
    wnump= &(snum->d[num_n-1]);

    /* Setup to 'res' */
    res->neg= (num->neg^divisor->neg);
    bn = tm_bn_wexpand(res,(loop+1));
    if (!bn) goto err;
    res->top=loop;
    resp= &(res->d[loop-1]);

    /* space for temp */
    bn = tm_bn_wexpand(tmp,(div_n+1));
    if (!bn) goto err;
    cmp =  tfBN_ucmp(&wnum,sdiv);
    if (cmp >= 0)
    {
        cmp = tfBN_usub(&wnum,&wnum,sdiv);
        if (!cmp) goto err;
        *resp=1;
        res->d[res->top-1]=1;
    }
    else
        res->top--;
    resp--;

    for (i=0; i<loop-1; i++)
    {
        ttBN_ULONG q,l0;
#if defined( tfBN_DIV3W) && !defined(NO_ASM)
        ttBN_ULONG bn_div_3_words(ttBN_ULONGPtr,ttBN_ULONG,ttBN_ULONG);
        q=bn_div_3_words(wnump,d1,d0);
#else
        ttBN_ULONG n0,n1,rem=0;

        n0=wnump[0];
        n1=wnump[-1];
        if (n0 == d0)
            q=TM_BN_MASK2;
        else             /* n0 < d0 */
        {
#ifdef TM_BN_LLONG
            ttBN_ULLONG t2;

#if defined(TM_BN_LLONG) && defined(ttBN_DIV2W) && !defined(tfbn_div_words)
            q=(ttBN_ULONG)(((((ttBN_ULLONG)n0)<<TM_BN_BITS2)|n1)/d0);
#else
            q=tfbn_div_words(n0,n1,d0);
#endif

#ifndef REMAINDER_IS_ALREADY_CALCULATED
            /*
             * rem doesn't have to be ttBN_ULLONG. The least we
             * know it's less that d0, isn't it?
             */
            rem=(n1-q*d0)&TM_BN_MASK2;
#endif
            t2=(ttBN_ULLONG)d1*q;

            for (;;)
            {
                if (t2 <= ((((ttBN_ULLONG)rem)<<TM_BN_BITS2)|wnump[-2]))
                    break;
                q--;
                rem += d0;
                if (rem < d0) break; /* don't let rem overflow */
                t2 -= d1;
            }
#else /* !TM_BN_LLONG */
            ttBN_ULONG t2l,t2h;
#ifndef TM_BN_UMULT_HIGH
            ttBN_ULONG ql,qh;
#endif /* !TM_BN_UMULT_HIGH */

            q=tfbn_div_words(n0,n1,d0);
#ifndef REMAINDER_IS_ALREADY_CALCULATED
            rem=(n1-q*d0)&TM_BN_MASK2;
#endif

#ifdef TM_BN_UMULT_HIGH
            t2l = d1 * q;
            t2h = TM_BN_UMULT_HIGH(d1,q);
#else /* !TM_BN_UMULT_HIGH */
            t2l=tm_bn_lbits(d1); t2h=tm_bn_hbits(d1);
            ql =tm_bn_lbits(q);  qh =tm_bn_hbits(q);
            tm_bn_mul64(t2l,t2h,ql,qh); /* t2=(ttBN_ULLONG)d1*q; */
#endif /* !TM_BN_UMULT_HIGH */

            for (;;)
            {
                if ((t2h < rem) ||
                    ((t2h == rem) && (t2l <= wnump[-2])))
                    break;
                q--;
                rem += d0;
                if (rem < d0) break; /* don't let rem overflow */
                if (t2l < d1) t2h--; t2l -= d1;
            }
#endif /* !TM_BN_LLONG */
        }
#endif /* ! tfBN_DIV3W */

        l0= tfbn_mul_words(tmp->d,sdiv->d,div_n,q);
        wnum.d--; wnum.top++;
        tmp->d[div_n]=l0;
        for (j=div_n+1; j>0; j--)
            if (tmp->d[j-1]) break;
        tmp->top=j;

        j=wnum.top;
        cmp = tfBN_sub(&wnum,&wnum,tmp);
        if (!cmp) goto err;

        snum->top=snum->top+wnum.top-j;

        if (wnum.neg)
        {
            q--;
            j=wnum.top;
            cmp = tfBN_add(&wnum,&wnum,sdiv);
            if (!cmp) goto err;

            snum->top+=wnum.top-j;
        }
        *(resp--)=q;
        wnump--;
    }
    if (rm != TM_OPENSSL_NULL)
    {
        cmp = tfBN_rshift(rm,snum,norm_shift);
        if (!cmp) goto err;

        rm->neg=num->neg;
    }
    tfBN_CTX_end(ctx);
    return(1);
err:
    tfBN_CTX_end(ctx);
    return(0);
}

/* rem != m */
int tfBN_mod(ttBIGNUMBERPtr rem, ttConstBIGNUMBERPtr m, ttConstBIGNUMBERPtr d, 
                  ttBN_CTXPtr ctx)
{
    return( tfBN_div(TM_OPENSSL_NULL,rem,m,d,ctx));
}

/* tfBN_word.c --------------------------------------*/
int tfBN_add_word(ttBIGNUMBERPtr a, ttBN_ULONG w)
{
    ttBN_ULONG l;
    int i;
    ttBIGNUMBERPtr bn;

    if (a->neg)
    { 
        a->neg=0;
        i= tfBN_sub_word(a,w);
        if (!tm_bn_is_zero(a))
            a->neg=!(a->neg);
        return(i);
    }
    w&=TM_BN_MASK2;
    bn = tm_bn_wexpand(a,a->top+1);
    if (bn == TM_OPENSSL_NULL) return(0);
    i=0;
    for (;;)
    { 
        l=(a->d[i]+(ttBN_ULONG)w)&TM_BN_MASK2;
        a->d[i]=l;
        if (w > l)
            w=1;
        else
            break;
        i++;
    }
    if (i >= a->top)
        a->top++;
    return(1);
}

int tfBN_sub_word(ttBIGNUMBERPtr a, ttBN_ULONG w)
{
    int i;

    if (tm_bn_is_zero(a) || a->neg)
    { 
        a->neg=0;
        i= tfBN_add_word(a,w);
        a->neg=1;
        return(i);
    }

    w&=TM_BN_MASK2;
    if ((a->top == 1) && (a->d[0] < w))
    { 
        a->d[0]=w-a->d[0];
        a->neg=1;
        return(1);
    }
    i=0;
    for (;;)
    { 
        if (a->d[i] >= w)
        { 
            a->d[i]-=w;
            break;
        }
        else
        { 
            a->d[i]=(a->d[i]-w)&TM_BN_MASK2;
            i++;
            w=1;
        }
    }
    if ((a->d[i] == 0) && (i == (a->top-1)))
        a->top--;
    return(1);
}

int tfBN_mul_word(ttBIGNUMBERPtr a, ttBN_ULONG w)
{
    ttBN_ULONG ll;
    ttBIGNUMBERPtr bn;

    w&=TM_BN_MASK2;
    if (a->top)
    {
        if (w == 0)
            tm_bn_zero(a); /* cannot fail since a->top is non zero */
        else
        {
            ll= tfbn_mul_words(a->d,a->d,a->top,w);
            if (ll)
            {
                bn = tm_bn_wexpand(a,a->top+1);
                if (bn == TM_OPENSSL_NULL) return(0);
                a->d[a->top++]=ll;
            }
        }
    }
    return(1);
}

/*bn_sqr.c  ---------------------------------------------------*/

/* r must not be a */
/* I've just gone over this and it is now %20 faster on x86 - eay - 27 Jun 96*/
int tfBN_sqr(ttBIGNUMBERPtr r, ttBIGNUMBERPtr a, ttBN_CTXPtr ctx)
{
    int max,al;
    int ret = 0;
    ttBIGNUMBERPtr tmp,rr,bn;


    al=a->top;
    if (al <= 0)
    {
        r->top=0;
        return(1);
    }

    tfBN_CTX_start(ctx);
    rr=(a != r) ? r : tfBN_CTX_get(ctx);
    tmp= tfBN_CTX_get(ctx);
    if ((rr == TM_OPENSSL_NULL) || (tmp == TM_OPENSSL_NULL)) goto err;

    max=(al+al);
    bn = tm_bn_wexpand(rr,max+1);
    if (bn == TM_OPENSSL_NULL) goto err;

    r->neg=0;
    if (al == 4)
    {
#ifndef TM_BN_SQR_COMBA
        ttBN_ULONG t[8];
        tfbn_sqr_normal(rr->d,a->d,4,t);
#else
        tfbn_sqr_comba4(rr->d,a->d);
#endif
    }
    else if (al == 8)
    {
#ifndef TM_BN_SQR_COMBA
        ttBN_ULONG t[16];
        tfbn_sqr_normal(rr->d,a->d,8,t);
#else
        tfbn_sqr_comba8(rr->d,a->d);
#endif
    }
    else 
    {
#if defined(TM_BN_RECURSION)
        if (al < TM_BN_SQR_RECURSIVE_SIZE_NORMAL)
        {
            ttBN_ULONG t[TM_BN_SQR_RECURSIVE_SIZE_NORMAL*2];
            tfbn_sqr_normal(rr->d,a->d,al,t);
        }
        else
        {
            int j,k;

            j=tfbn_num_bits_word((ttBN_ULONG)al);
            j=1<<(j-1);
            k=j+j;
            if (al == j)
            {
                bn = tm_bn_wexpand(a,k*2);
                if (bn == TM_OPENSSL_NULL) goto err;
                bn = tm_bn_wexpand(tmp,k*2);
                if (bn == TM_OPENSSL_NULL) goto err;
                tfbn_sqr_recursive(rr->d,a->d,al,tmp->d);
            }
            else
            {
                bn = tm_bn_wexpand(tmp,max);
                if (bn == TM_OPENSSL_NULL) goto err;
                tfbn_sqr_normal(rr->d,a->d,al,tmp->d);
            }
        }
#else
        bn = tm_bn_wexpand(tmp,max);
        if (bn == TM_OPENSSL_NULL) goto err;
        tfbn_sqr_normal(rr->d,a->d,al,tmp->d);
#endif
    }

    rr->top=max;
    if ((max > 0) && (rr->d[max-1] == 0)) rr->top--;
    ret = 1;
    if (r != rr)
    {
        bn = tfBN_copy(r, rr);
        if (bn == TM_OPENSSL_NULL) 
        {
            ret = 0;
        }
    }
 err:
    tfBN_CTX_end(ctx);
    return(ret);
}

/* tmp must have 2*n words */
static void tfbn_sqr_normal(
    ttBN_ULONGPtr r, ttBN_ULONGPtr a, int n, ttBN_ULONGPtr tmp)
{
    int i,j,max;
    ttBN_ULONGPtr ap,rp;

    max=n*2;
    ap=a;
    rp=r;
    rp[0]=rp[max-1]=0;
    rp++;
    j=n;

    j--;
    if( j > 0 )
    {
        ap++;
        rp[j]= tfbn_mul_words(rp,ap,j,ap[-1]);
        rp+=2;
    }

    for (i=n-2; i>0; i--)
    {
        j--;
        ap++;
        rp[j]= tfbn_mul_add_words(rp,ap,j,ap[-1]);
        rp+=2;
    }

    (void)tfbn_add_words(r,r,r,max);

    /* There will not be a carry */

    (void)tfbn_sqr_words(tmp,a,n);

    (void)tfbn_add_words(r,r,tmp,max);
}

#ifdef TM_BN_RECURSION
/* r is 2*n words in size,
 * a and b are both n words in size.    (There's not actually a 'b' here ...)
 * n must be a power of 2.
 * We multiply and return the result.
 * t must be 2*n words in size
 * We calculate
 * a[0]*b[0]
 * a[0]*b[0]+a[1]*b[1]+(a[0]-a[1])*(b[1]-b[0])
 * a[1]*b[1]
 */
static void tfbn_sqr_recursive(
    ttBN_ULONGPtr r, ttBN_ULONGPtr a, int n2, ttBN_ULONGPtr t)
{
    int n=n2/2;
    int zero,c1;
    ttBN_ULONGPtr p;
    ttBN_ULONG ln,lo;

    if (n2 == 4)
    {
#ifndef TM_BN_SQR_COMBA
        tfbn_sqr_normal(r,a,4,t);
#else
        tfbn_sqr_comba4(r,a);
#endif
        return;
    }
    else if (n2 == 8)
    {
#ifndef TM_BN_SQR_COMBA
        tfbn_sqr_normal(r,a,8,t);
#else
        tfbn_sqr_comba8(r,a);
#endif
        return;
    }
    if (n2 < TM_BN_SQR_RECURSIVE_SIZE_NORMAL)
    {
        tfbn_sqr_normal(r,a,n2,t);
        return;
    }
    /* r=(a[0]-a[1])*(a[1]-a[0]) */
    c1=tfbn_cmp_words(a,&(a[n]),n);
    zero=0;
    if (c1 > 0)
        tfbn_sub_words(t,a,&(a[n]),n);
    else if (c1 < 0)
        tfbn_sub_words(t,&(a[n]),a,n);
    else
        zero=1;

    /* The result will always be negative unless it is zero */
    p= &(t[n2*2]);

    if (!zero)
        tfbn_sqr_recursive(&(t[n2]),t,n,p);
    else
    {
        tm_bzero(&(t[n2]),(unsigned int)n2*sizeof(ttBN_ULONG));
    }
    tfbn_sqr_recursive(r,a,n,p);
    tfbn_sqr_recursive(&(r[n2]),&(a[n]),n,p);

    /* t[32] holds (a[0]-a[1])*(a[1]-a[0]), it is negative or zero
     * r[10] holds (a[0]*b[0])
     * r[32] holds (b[1]*b[1])
     */

    c1=(int)(tfbn_add_words(t,r,&(r[n2]),n2));

    /* t[32] is negative */
    c1-=(int)(tfbn_sub_words(&(t[n2]),t,&(t[n2]),n2));

    /* t[32] holds (a[0]-a[1])*(a[1]-a[0])+(a[0]*a[0])+(a[1]*a[1])
     * r[10] holds (a[0]*a[0])
     * r[32] holds (a[1]*a[1])
     * c1 holds the carry bits
     */
    c1+=(int)(tfbn_add_words(&(r[n]),&(r[n]),&(t[n2]),n2));
    if (c1)
    {
        p= &(r[n+n2]);
        lo= *p;
        ln=(lo+(ttBN_ULONG)c1)&TM_BN_MASK2;
        *p=ln;

        /* The overflow will stop before we over write
         * words we should not overwrite */
        if (ln < (ttBN_ULONG)c1)
        {
            do    {
                p++;
                lo= *p;
                ln=(lo+1)&TM_BN_MASK2;
                *p=ln;
            } while (ln == 0);
        }
    }
}
#endif




/** tfBN_asm.c *----------------------------------------------------*/

#if defined(TM_BN_LLONG) || defined(TM_BN_UMULT_HIGH)

ttBN_ULONG tfbn_mul_add_words(
    ttBN_ULONGPtr rp, ttBN_ULONGPtr ap, int num, ttBN_ULONG w)
{
    ttBN_ULONG c1;
    ttBN_ULONG l, h; /* for tm_bn_mul_add */
    ttBN_ULONG m, m1, lt, ht; /* for tm_bn_mul64 */


    c1 = 0;
    tm_assert(tfbn_mul_add_words, num >= 0);
    if (num <= 0) return(c1);

    while (num&~3)
    {
        tm_bn_mul_add(rp[0],ap[0],w,c1);
        tm_bn_mul_add(rp[1],ap[1],w,c1);
        tm_bn_mul_add(rp[2],ap[2],w,c1);
        tm_bn_mul_add(rp[3],ap[3],w,c1);
        ap+=4; rp+=4; num-=4;
    }
    if (num)
    {
        tm_bn_mul_add(rp[0],ap[0],w,c1); if (--num==0) return c1;
        tm_bn_mul_add(rp[1],ap[1],w,c1); if (--num==0) return c1;
        tm_bn_mul_add(rp[2],ap[2],w,c1); return c1;
    }
    
    return(c1);
} 

static ttBN_ULONG tfbn_mul_words(
    ttBN_ULONGPtr rp, ttBN_ULONGPtr ap, int num, ttBN_ULONG w)
{
    ttBN_ULONG c1=0;

    tm_assert(tfbn_mul_words, num >= 0);
    if (num <= 0) return(c1);

    while (num&~3)
    {
        tm_bn_mul(rp[0],ap[0],w,c1);
        tm_bn_mul(rp[1],ap[1],w,c1);
        tm_bn_mul(rp[2],ap[2],w,c1);
        tm_bn_mul(rp[3],ap[3],w,c1);
        ap+=4; rp+=4; num-=4;
    }
    if (num)
    {
        tm_bn_mul(rp[0],ap[0],w,c1); if (--num == 0) return c1;
        tm_bn_mul(rp[1],ap[1],w,c1); if (--num == 0) return c1;
        tm_bn_mul(rp[2],ap[2],w,c1);
    }
    return(c1);
} 

static void tfbn_sqr_words(ttBN_ULONGPtr r, ttBN_ULONGPtr a, int n)
{
    tm_assert(tfbn_sqr_words, n >= 0);
    if (n <= 0) return;
    while (n&~3)
    {
        sqr(r[0],r[1],a[0]);
        sqr(r[2],r[3],a[1]);
        sqr(r[4],r[5],a[2]);
        sqr(r[6],r[7],a[3]);
        a+=4; r+=8; n-=4;
    }
    if (n)
    {
        sqr(r[0],r[1],a[0]); if (--n == 0) return;
        sqr(r[2],r[3],a[1]); if (--n == 0) return;
        sqr(r[4],r[5],a[2]);
    }
}

#else /* !(defined(TM_BN_LLONG) || defined(TM_BN_UMULT_HIGH)) */

ttBN_ULONG tfbn_mul_add_words(
    ttBN_ULONGPtr rp, ttBN_ULONGPtr ap, int num, ttBN_ULONG w)
{
    ttBN_ULONG c=0;
    ttBN_ULONG bl,bh;
    ttBN_ULONG m, m1, lt, ht; /* for tm_bn_mul64 */
    ttBN_ULONG l, h; /* for tm_bn_mul_add */

    if (num <= 0) return((ttBN_ULONG)0);

    bl=tm_bn_lbits(w);
    bh=tm_bn_hbits(w);

    for (;;)
    {
        tm_bn_mul_add(rp[0],ap[0],bl,bh,c);
        num--;
        if (num == 0) break;
        tm_bn_mul_add(rp[1],ap[1],bl,bh,c);
        num--;
        if (num == 0) break;
        tm_bn_mul_add(rp[2],ap[2],bl,bh,c);
        num--;
        if (num == 0) break;
        tm_bn_mul_add(rp[3],ap[3],bl,bh,c);
        num--;
        if (num == 0) break;
        ap+=4;
        rp+=4;
    }
    return(c);
} 

static ttBN_ULONG tfbn_mul_words(
    ttBN_ULONGPtr rp, ttBN_ULONGPtr ap, int num, ttBN_ULONG w)
{
    ttBN_ULONG carry=0;
    ttBN_ULONG bl,bh;
    ttBN_ULONG m, m1, lt, ht; /* for tm_bn_mul64 */
    ttBN_ULONG l, h; /* for tm_bn_mul */

    if (num <= 0) return((ttBN_ULONG)0);

    bl=tm_bn_lbits(w);
    bh=tm_bn_hbits(w);

    for (;;)
    {
        tm_bn_mul(rp[0],ap[0],bl,bh,carry);
        num--;
        if (num == 0) break;
        tm_bn_mul(rp[1],ap[1],bl,bh,carry);
        num--;
        if (num == 0) break;
        tm_bn_mul(rp[2],ap[2],bl,bh,carry);
        num--;
        if (num == 0) break;
        tm_bn_mul(rp[3],ap[3],bl,bh,carry);
        num--;
        if (num == 0) break;
        ap+=4;
        rp+=4;
    }
    return(carry);
} 

static void tfbn_sqr_words(ttBN_ULONGPtr r, ttBN_ULONGPtr a, int n)
{
    ttBN_ULONG l, h, m; /* for tm_bn_sqr64 */

    if (n <= 0) return;
    for (;;)
    {
        tm_bn_sqr64(r[0],r[1],a[0]);
        n--;
        if (n == 0) break;

        tm_bn_sqr64(r[2],r[3],a[1]);
        n--;
        if (n == 0) break;

        tm_bn_sqr64(r[4],r[5],a[2]);
        n--;
        if (n == 0) break;

        tm_bn_sqr64(r[6],r[7],a[3]);
        n--;
        if (n == 0) break;

        a+=4;
        r+=8;
    }
}

#endif /* !(defined(TM_BN_LLONG) || defined(TM_BN_UMULT_HIGH)) */

#if defined(TM_BN_LLONG) && defined(ttBN_DIV2W)

static ttBN_ULONG tfbn_div_words(ttBN_ULONG h, ttBN_ULONG l, ttBN_ULONG d)
{
    return((ttBN_ULONG)(((((ttBN_ULLONG)h)<<TM_BN_BITS2)|l)/(ttBN_ULLONG)d));
}

#else

/* Divide h,l by d and return the result. */
/* I need to test this some more :-( */
static ttBN_ULONG tfbn_div_words(ttBN_ULONG h, ttBN_ULONG l, ttBN_ULONG d)
{
    ttBN_ULONG dh,dl,q,ret=0,th,tl,t;
    int i,count=2;

    if (d == 0) return(TM_BN_MASK2);

    i=tfbn_num_bits_word(d);


    i=TM_BN_BITS2-i;
    if (h >= d) h-=d;

    if (i)
    {
        d<<=i;
        h=(h<<i)|(l>>(TM_BN_BITS2-i));
        l<<=i;
    }
    dh=(d&TM_BN_MASK2h)>>TM_BN_BITS4;
    dl=(d&TM_BN_MASK2l);
    for (;;)
    {
        if ((h>>TM_BN_BITS4) == dh)
            q=TM_BN_MASK2l;
        else
            q=h/dh;

        th=q*dh;
        tl=dl*q;
        for (;;)
        {
            t=h-th;
            if ((t&TM_BN_MASK2h) ||
                ((tl) <= (
                    (t<<TM_BN_BITS4)|
                    ((l&TM_BN_MASK2h)>>TM_BN_BITS4))))
                break;
            q--;
            th-=dh;
            tl-=dl;
        }
        t=(tl>>TM_BN_BITS4);
        tl=(tl<<TM_BN_BITS4)&TM_BN_MASK2h;
        th+=t;

        if (l < tl) th++;
        l-=tl;
        if (h < th)
        {
            h+=d;
            q--;
        }
        h-=th;

        count--;
        if (count == 0) break;

        ret=q<<TM_BN_BITS4;
        h=((h<<TM_BN_BITS4)|(l>>TM_BN_BITS4))&TM_BN_MASK2;
        l=(l&TM_BN_MASK2l)<<TM_BN_BITS4;
    }
    ret|=q;
    return(ret);
}
#endif /* !defined(TM_BN_LLONG) && defined(ttBN_DIV2W) */

#ifdef TM_BN_LLONG
ttBN_ULONG tfbn_add_words(
    ttBN_ULONGPtr r, ttBN_ULONGPtr a, ttBN_ULONGPtr b, int n)
{
    ttBN_ULLONG ll=0;

    tm_assert(tfbn_add_words, n >= 0);
    if (n <= 0) return((ttBN_ULONG)0);

    for (;;)
    {
        ll+=(ttBN_ULLONG)a[0]+b[0];
        r[0]=(ttBN_ULONG)ll&TM_BN_MASK2;
        ll>>=TM_BN_BITS2;
        n--;
        if (n <= 0) break;

        ll+=(ttBN_ULLONG)a[1]+b[1];
        r[1]=(ttBN_ULONG)ll&TM_BN_MASK2;
        ll>>=TM_BN_BITS2;
        n--;
        if (n <= 0) break;

        ll+=(ttBN_ULLONG)a[2]+b[2];
        r[2]=(ttBN_ULONG)ll&TM_BN_MASK2;
        ll>>=TM_BN_BITS2;
        n--;
        if (n <= 0) break;

        ll+=(ttBN_ULLONG)a[3]+b[3];
        r[3]=(ttBN_ULONG)ll&TM_BN_MASK2;
        ll>>=TM_BN_BITS2;
        n--;
        if (n <= 0) break;

        a+=4;
        b+=4;
        r+=4;
    }
    return((ttBN_ULONG)ll);
}
#else /* !TM_BN_LLONG */
ttBN_ULONG tfbn_add_words(
    ttBN_ULONGPtr r, ttBN_ULONGPtr a, ttBN_ULONGPtr b, int n)
{
    ttBN_ULONG c,l,t;


    if (n <= 0) return((ttBN_ULONG)0);

    c=0;
    for (;;)
    {
        t=a[0];
        t=(t+c)&TM_BN_MASK2;
        c=(t < c);
        l=(t+b[0])&TM_BN_MASK2;
        c+=(l < t);
        r[0]=l;
        n--;
        if (n <= 0) break;

        t=a[1];
        t=(t+c)&TM_BN_MASK2;
        c=(t < c);
        l=(t+b[1])&TM_BN_MASK2;
        c+=(l < t);
        r[1]=l;
        n--;
        if (n <= 0) break;

        t=a[2];
        t=(t+c)&TM_BN_MASK2;
        c=(t < c);
        l=(t+b[2])&TM_BN_MASK2;
        c+=(l < t);
        r[2]=l;
        n--;
        if (n <= 0) break;

        t=a[3];
        t=(t+c)&TM_BN_MASK2;
        c=(t < c);
        l=(t+b[3])&TM_BN_MASK2;
        c+=(l < t);
        r[3]=l;
        n--;
        if (n <= 0) break;

        a+=4;
        b+=4;
        r+=4;
    }
    return((ttBN_ULONG)c);
}
#endif /* !TM_BN_LLONG */


#ifdef TM_BN_MUL_COMBA

static void tfbn_mul_comba8(ttBN_ULONGPtr r, ttBN_ULONGPtr a, ttBN_ULONGPtr b)
{
#ifdef TM_BN_LLONG
    ttBN_ULLONG t;
#else
    ttBN_ULONG bl,bh;
#endif
    ttBN_ULONG t1,t2;
    ttBN_ULONG c1,c2,c3;
#ifdef TM_BN_UMULT_HIGH
    ttBN_ULONG ta, tb; /* for tm_mul_add_c */
#else /* !TM_BN_UMULT_HIGH */
    ttBN_ULONG m, m1, lt, ht; /* for tm_bn_mul64 */
#endif /* !TM_BN_UMULT_HIGH */

    c1=0;
    c2=0;
    c3=0;
    tm_mul_add_c(a[0],b[0],c1,c2,c3);
    r[0]=c1;
    c1=0;
    tm_mul_add_c(a[0],b[1],c2,c3,c1);
    tm_mul_add_c(a[1],b[0],c2,c3,c1);
    r[1]=c2;
    c2=0;
    tm_mul_add_c(a[2],b[0],c3,c1,c2);
    tm_mul_add_c(a[1],b[1],c3,c1,c2);
    tm_mul_add_c(a[0],b[2],c3,c1,c2);
    r[2]=c3;
    c3=0;
    tm_mul_add_c(a[0],b[3],c1,c2,c3);
    tm_mul_add_c(a[1],b[2],c1,c2,c3);
    tm_mul_add_c(a[2],b[1],c1,c2,c3);
    tm_mul_add_c(a[3],b[0],c1,c2,c3);
    r[3]=c1;
    c1=0;
    tm_mul_add_c(a[4],b[0],c2,c3,c1);
    tm_mul_add_c(a[3],b[1],c2,c3,c1);
    tm_mul_add_c(a[2],b[2],c2,c3,c1);
    tm_mul_add_c(a[1],b[3],c2,c3,c1);
    tm_mul_add_c(a[0],b[4],c2,c3,c1);
    r[4]=c2;
    c2=0;
    tm_mul_add_c(a[0],b[5],c3,c1,c2);
    tm_mul_add_c(a[1],b[4],c3,c1,c2);
    tm_mul_add_c(a[2],b[3],c3,c1,c2);
    tm_mul_add_c(a[3],b[2],c3,c1,c2);
    tm_mul_add_c(a[4],b[1],c3,c1,c2);
    tm_mul_add_c(a[5],b[0],c3,c1,c2);
    r[5]=c3;
    c3=0;
    tm_mul_add_c(a[6],b[0],c1,c2,c3);
    tm_mul_add_c(a[5],b[1],c1,c2,c3);
    tm_mul_add_c(a[4],b[2],c1,c2,c3);
    tm_mul_add_c(a[3],b[3],c1,c2,c3);
    tm_mul_add_c(a[2],b[4],c1,c2,c3);
    tm_mul_add_c(a[1],b[5],c1,c2,c3);
    tm_mul_add_c(a[0],b[6],c1,c2,c3);
    r[6]=c1;
    c1=0;
    tm_mul_add_c(a[0],b[7],c2,c3,c1);
    tm_mul_add_c(a[1],b[6],c2,c3,c1);
    tm_mul_add_c(a[2],b[5],c2,c3,c1);
    tm_mul_add_c(a[3],b[4],c2,c3,c1);
    tm_mul_add_c(a[4],b[3],c2,c3,c1);
    tm_mul_add_c(a[5],b[2],c2,c3,c1);
    tm_mul_add_c(a[6],b[1],c2,c3,c1);
    tm_mul_add_c(a[7],b[0],c2,c3,c1);
    r[7]=c2;
    c2=0;
    tm_mul_add_c(a[7],b[1],c3,c1,c2);
    tm_mul_add_c(a[6],b[2],c3,c1,c2);
    tm_mul_add_c(a[5],b[3],c3,c1,c2);
    tm_mul_add_c(a[4],b[4],c3,c1,c2);
    tm_mul_add_c(a[3],b[5],c3,c1,c2);
    tm_mul_add_c(a[2],b[6],c3,c1,c2);
    tm_mul_add_c(a[1],b[7],c3,c1,c2);
    r[8]=c3;
    c3=0;
    tm_mul_add_c(a[2],b[7],c1,c2,c3);
    tm_mul_add_c(a[3],b[6],c1,c2,c3);
    tm_mul_add_c(a[4],b[5],c1,c2,c3);
    tm_mul_add_c(a[5],b[4],c1,c2,c3);
    tm_mul_add_c(a[6],b[3],c1,c2,c3);
    tm_mul_add_c(a[7],b[2],c1,c2,c3);
    r[9]=c1;
    c1=0;
    tm_mul_add_c(a[7],b[3],c2,c3,c1);
    tm_mul_add_c(a[6],b[4],c2,c3,c1);
    tm_mul_add_c(a[5],b[5],c2,c3,c1);
    tm_mul_add_c(a[4],b[6],c2,c3,c1);
    tm_mul_add_c(a[3],b[7],c2,c3,c1);
    r[10]=c2;
    c2=0;
    tm_mul_add_c(a[4],b[7],c3,c1,c2);
    tm_mul_add_c(a[5],b[6],c3,c1,c2);
    tm_mul_add_c(a[6],b[5],c3,c1,c2);
    tm_mul_add_c(a[7],b[4],c3,c1,c2);
    r[11]=c3;
    c3=0;
    tm_mul_add_c(a[7],b[5],c1,c2,c3);
    tm_mul_add_c(a[6],b[6],c1,c2,c3);
    tm_mul_add_c(a[5],b[7],c1,c2,c3);
    r[12]=c1;
    c1=0;
    tm_mul_add_c(a[6],b[7],c2,c3,c1);
    tm_mul_add_c(a[7],b[6],c2,c3,c1);
    r[13]=c2;
    c2=0;
    tm_mul_add_c(a[7],b[7],c3,c1,c2);
    r[14]=c3;
    r[15]=c1;
}

static void tfbn_mul_comba4(ttBN_ULONGPtr r, ttBN_ULONGPtr a, ttBN_ULONGPtr b)
{
#ifdef TM_BN_LLONG
    ttBN_ULLONG t;
#else
    ttBN_ULONG bl,bh;
#endif
    ttBN_ULONG t1,t2;
    ttBN_ULONG c1,c2,c3;
#ifdef TM_BN_UMULT_HIGH
    ttBN_ULONG ta, tb; /* for tm_mul_add_c */
#else /* !TM_BN_UMULT_HIGH */
    ttBN_ULONG m, m1, lt, ht; /* for tm_bn_mul64 */
#endif /* !TM_BN_UMULT_HIGH */

    c1=0;
    c2=0;
    c3=0;
    tm_mul_add_c(a[0],b[0],c1,c2,c3);
    r[0]=c1;
    c1=0;
    tm_mul_add_c(a[0],b[1],c2,c3,c1);
    tm_mul_add_c(a[1],b[0],c2,c3,c1);
    r[1]=c2;
    c2=0;
    tm_mul_add_c(a[2],b[0],c3,c1,c2);
    tm_mul_add_c(a[1],b[1],c3,c1,c2);
    tm_mul_add_c(a[0],b[2],c3,c1,c2);
    r[2]=c3;
    c3=0;
    tm_mul_add_c(a[0],b[3],c1,c2,c3);
    tm_mul_add_c(a[1],b[2],c1,c2,c3);
    tm_mul_add_c(a[2],b[1],c1,c2,c3);
    tm_mul_add_c(a[3],b[0],c1,c2,c3);
    r[3]=c1;
    c1=0;
    tm_mul_add_c(a[3],b[1],c2,c3,c1);
    tm_mul_add_c(a[2],b[2],c2,c3,c1);
    tm_mul_add_c(a[1],b[3],c2,c3,c1);
    r[4]=c2;
    c2=0;
    tm_mul_add_c(a[2],b[3],c3,c1,c2);
    tm_mul_add_c(a[3],b[2],c3,c1,c2);
    r[5]=c3;
    c3=0;
    tm_mul_add_c(a[3],b[3],c1,c2,c3);
    r[6]=c1;
    r[7]=c2;
}

static void tfbn_sqr_comba8(ttBN_ULONGPtr r, ttBN_ULONGPtr a)
{
#ifdef TM_BN_LLONG
    ttBN_ULLONG t,tt;
#else
    ttBN_ULONG bl,bh;
#endif
    ttBN_ULONG t1,t2;
    ttBN_ULONG c1,c2,c3;
#ifdef TM_BN_UMULT_HIGH
    ttBN_ULONG ta; /* for tm_sqr_add_c/tm_sqr_add_c2 */
    ttBN_ULONG tb, t0; /* for tm_sqr_add_c2 */
#else /* !TM_BN_UMULT_HIGH */
    ttBN_ULONG l,h,m; /* for tm_bn_sqr64 (tm_sqr_add_c)*/
    ttBN_ULONG m1, lt, ht; /* for tm_bn_mul64 (tm_sqr_add_c2)*/
#endif /* !TM_BN_UMULT_HIGH */

    c1=0;
    c2=0;
    c3=0;
    tm_sqr_add_c(a,0,c1,c2,c3);
    r[0]=c1;
    c1=0;
    tm_sqr_add_c2(a,1,0,c2,c3,c1);
    r[1]=c2;
    c2=0;
    tm_sqr_add_c(a,1,c3,c1,c2);
    tm_sqr_add_c2(a,2,0,c3,c1,c2);
    r[2]=c3;
    c3=0;
    tm_sqr_add_c2(a,3,0,c1,c2,c3);
    tm_sqr_add_c2(a,2,1,c1,c2,c3);
    r[3]=c1;
    c1=0;
    tm_sqr_add_c(a,2,c2,c3,c1);
    tm_sqr_add_c2(a,3,1,c2,c3,c1);
    tm_sqr_add_c2(a,4,0,c2,c3,c1);
    r[4]=c2;
    c2=0;
    tm_sqr_add_c2(a,5,0,c3,c1,c2);
    tm_sqr_add_c2(a,4,1,c3,c1,c2);
    tm_sqr_add_c2(a,3,2,c3,c1,c2);
    r[5]=c3;
    c3=0;
    tm_sqr_add_c(a,3,c1,c2,c3);
    tm_sqr_add_c2(a,4,2,c1,c2,c3);
    tm_sqr_add_c2(a,5,1,c1,c2,c3);
    tm_sqr_add_c2(a,6,0,c1,c2,c3);
    r[6]=c1;
    c1=0;
    tm_sqr_add_c2(a,7,0,c2,c3,c1);
    tm_sqr_add_c2(a,6,1,c2,c3,c1);
    tm_sqr_add_c2(a,5,2,c2,c3,c1);
    tm_sqr_add_c2(a,4,3,c2,c3,c1);
    r[7]=c2;
    c2=0;
    tm_sqr_add_c(a,4,c3,c1,c2);
    tm_sqr_add_c2(a,5,3,c3,c1,c2);
    tm_sqr_add_c2(a,6,2,c3,c1,c2);
    tm_sqr_add_c2(a,7,1,c3,c1,c2);
    r[8]=c3;
    c3=0;
    tm_sqr_add_c2(a,7,2,c1,c2,c3);
    tm_sqr_add_c2(a,6,3,c1,c2,c3);
    tm_sqr_add_c2(a,5,4,c1,c2,c3);
    r[9]=c1;
    c1=0;
    tm_sqr_add_c(a,5,c2,c3,c1);
    tm_sqr_add_c2(a,6,4,c2,c3,c1);
    tm_sqr_add_c2(a,7,3,c2,c3,c1);
    r[10]=c2;
    c2=0;
    tm_sqr_add_c2(a,7,4,c3,c1,c2);
    tm_sqr_add_c2(a,6,5,c3,c1,c2);
    r[11]=c3;
    c3=0;
    tm_sqr_add_c(a,6,c1,c2,c3);
    tm_sqr_add_c2(a,7,5,c1,c2,c3);
    r[12]=c1;
    c1=0;
    tm_sqr_add_c2(a,7,6,c2,c3,c1);
    r[13]=c2;
    c2=0;
    tm_sqr_add_c(a,7,c3,c1,c2);
    r[14]=c3;
    r[15]=c1;
}

static void tfbn_sqr_comba4(ttBN_ULONGPtr r, ttBN_ULONGPtr a)
{
#ifdef TM_BN_LLONG
    ttBN_ULLONG t,tt;
#else
    ttBN_ULONG bl,bh;
#endif
    ttBN_ULONG t1,t2;
    ttBN_ULONG c1,c2,c3;
#ifdef TM_BN_UMULT_HIGH
    ttBN_ULONG ta; /* for tm_sqr_add_c/tm_sqr_add_c2 */
    ttBN_ULONG tb, t0; /* for tm_sqr_add_c2 */
#else /* !TM_BN_UMULT_HIGH */
    ttBN_ULONG l,h,m; /* for tm_bn_sqr64 (tm_sqr_add_c)*/
    ttBN_ULONG m1, lt, ht; /* for tm_bn_mul64 (tm_sqr_add_c2)*/
#endif /* !TM_BN_UMULT_HIGH */

    c1=0;
    c2=0;
    c3=0;
    tm_sqr_add_c(a,0,c1,c2,c3);
    r[0]=c1;
    c1=0;
    tm_sqr_add_c2(a,1,0,c2,c3,c1);
    r[1]=c2;
    c2=0;
    tm_sqr_add_c(a,1,c3,c1,c2);
    tm_sqr_add_c2(a,2,0,c3,c1,c2);
    r[2]=c3;
    c3=0;
    tm_sqr_add_c2(a,3,0,c1,c2,c3);
    tm_sqr_add_c2(a,2,1,c1,c2,c3);
    r[3]=c1;
    c1=0;
    tm_sqr_add_c(a,2,c2,c3,c1);
    tm_sqr_add_c2(a,3,1,c2,c3,c1);
    r[4]=c2;
    c2=0;
    tm_sqr_add_c2(a,3,2,c3,c1,c2);
    r[5]=c3;
    c3=0;
    tm_sqr_add_c(a,3,c1,c2,c3);
    r[6]=c1;
    r[7]=c2;
}

#endif /* !TM_BN_MUL_COMBA */

/* solves ax == 1 (mod n) */
ttBIGNUMBERPtr  tfBN_mod_inverse(ttBIGNUMBERPtr in, ttBIGNUMBERPtr a, 
                                 ttConstBIGNUMBERPtr n, ttBN_CTXPtr ctx)
{
    ttBIGNUMBERPtr A,B,X,Y,M,D,R=TM_OPENSSL_NULL;
    ttBIGNUMBERPtr T,bn,ret=TM_OPENSSL_NULL;
    int sign,cmp;



    tfBN_CTX_start(ctx);
    A = tfBN_CTX_get(ctx);
    B = tfBN_CTX_get(ctx);
    X = tfBN_CTX_get(ctx);
    D = tfBN_CTX_get(ctx);
    M = tfBN_CTX_get(ctx);
    Y = tfBN_CTX_get(ctx);
    if ( (A == TM_OPENSSL_NULL) || (B == TM_OPENSSL_NULL) ||
         (X == TM_OPENSSL_NULL) || (D == TM_OPENSSL_NULL) ||
         (M == TM_OPENSSL_NULL) || (Y == TM_OPENSSL_NULL) )
    {
        goto err;
    }
    if (in == TM_OPENSSL_NULL)
        R= tfBN_new();
    else
        R=in;
    if (R == TM_OPENSSL_NULL) goto err;

    (void)tm_bn_zero(X); /* cannot fail because of tfBN_CTX_get() */
    (void)tm_bn_one(Y); /* cannot fail because of tfBN_CTX_get() */
    bn = tfBN_copy(A,a);
    if ( bn == TM_OPENSSL_NULL) goto err;
    bn = tfBN_copy(B,n);
    if ( bn == TM_OPENSSL_NULL) goto err;
    sign=1;

    while (!tm_bn_is_zero(B))
    {
        cmp = tfBN_div(D,M,A,B,ctx);
        if (!cmp) goto err;
        T=A;
        A=B;
        B=M;
        /* T has a struct, M does not */

        cmp = tfBN_mul(T,D,X,ctx);
        if (!cmp) goto err;
        cmp = tfBN_add(T,T,Y);
        if (!cmp) goto err;
        M=Y;
        Y=X;
        X=T;
        sign= -sign;
    }
    if (sign < 0)
    {
        cmp =tfBN_sub(Y,n,Y);
        if (!cmp) goto err;
    }

    if (tm_bn_is_one(A))
    { 
        cmp = tfBN_mod(R,Y,n,ctx);
        if (!cmp) goto err;
     }
    else
    {
        goto err;
    }
    ret=R;
err:
    if ((ret == TM_OPENSSL_NULL) && (in == TM_OPENSSL_NULL)) tfBN_free(R);
    tfBN_CTX_end(ctx);
    return(ret);
}

/*bn_mul.c -----------------------------------------------------*/
#ifdef TM_BN_RECURSION
/* Karatsuba recursive multiplication algorithm
 * (cf. Knuth, The Art of Computer Programming, Vol. 2) */

/* r is 2*n2 words in size,
 * a and b are both n2 words in size.
 * n2 must be a power of 2.
 * We multiply and return the result.
 * t must be 2*n2 words in size
 * We calculate
 * a[0]*b[0]
 * a[0]*b[0]+a[1]*b[1]+(a[0]-a[1])*(b[1]-b[0])
 * a[1]*b[1]
 */
static void tfbn_mul_recursive(
    ttBN_ULONGPtr r, ttBN_ULONGPtr a, ttBN_ULONGPtr b, int n2, ttBN_ULONGPtr t)
{
    int n=n2/2,c1,c2;
    tt32Bit neg,zero;
    ttBN_ULONG ln,lo;
    ttBN_ULONGPtr p;

# ifdef TM_BN_MUL_COMBA

    if (n2 == 8)
    {
        tfbn_mul_comba8(r,a,b);
        return; 
    }
# endif /* TM_BN_MUL_COMBA */
    if (n2 < TM_BN_MUL_RECURSIVE_SIZE_NORMAL)
    {
        /* This should not happen */
        tfbn_mul_normal(r,a,n2,b,n2);
        return;
    }
    /* r=(a[0]-a[1])*(b[1]-b[0]) */
    c1=tfbn_cmp_words(a,&(a[n]),n);
    c2=tfbn_cmp_words(&(b[n]),b,n);
    zero=neg=0;
    switch (c1*3+c2)
    {
    case -4:
        (void)tfbn_sub_words(t,      &(a[n]),a,      n); /* - */
        (void)tfbn_sub_words(&(t[n]),b,      &(b[n]),n); /* - */
        break;
    case -3:
        zero=1;
        break;
    case -2:
        (void)tfbn_sub_words(t,      &(a[n]),a,      n); /* - */
        (void)tfbn_sub_words(&(t[n]),&(b[n]),b,      n); /* + */
        neg=1;
        break;
    case -1:
    case 0:
    case 1:
        zero=1;
        break;
    case 2:
        (void)tfbn_sub_words(t,      a,      &(a[n]),n); /* + */
        (void)tfbn_sub_words(&(t[n]),b,      &(b[n]),n); /* - */
        neg=1;
        break;
    case 3:
        zero=1;
        break;
    case 4:
        (void)tfbn_sub_words(t,      a,      &(a[n]),n);
        (void)tfbn_sub_words(&(t[n]),&(b[n]),b,      n);
        break;
    default:
        break;
    }

# ifdef TM_BN_MUL_COMBA
    if (n == 4)
    {
        if (!zero)
            tfbn_mul_comba4(&(t[n2]),t,&(t[n]));
        else
            tm_bzero(&(t[n2]),8*sizeof(ttBN_ULONG));
        
        tfbn_mul_comba4(r,a,b);
        tfbn_mul_comba4(&(r[n2]),&(a[n]),&(b[n]));
    }
    else if (n == 8)
    {
        if (!zero)
            tfbn_mul_comba8(&(t[n2]),t,&(t[n]));
        else
            tm_bzero(&(t[n2]),16*sizeof(ttBN_ULONG));
        
        tfbn_mul_comba8(r,a,b);
        tfbn_mul_comba8(&(r[n2]),&(a[n]),&(b[n]));
    }
    else
# endif /* TM_BN_MUL_COMBA */
    {
        p= &(t[n2*2]);
        if (!zero)
            tfbn_mul_recursive(&(t[n2]),t,&(t[n]),n,p);
        else
            tm_bzero(&(t[n2]),(unsigned int)n2*sizeof(ttBN_ULONG));
        tfbn_mul_recursive(r,a,b,n,p);
        tfbn_mul_recursive(&(r[n2]),&(a[n]),&(b[n]),n,p);
    }

    /* t[32] holds (a[0]-a[1])*(b[1]-b[0]), c1 is the sign
     * r[10] holds (a[0]*b[0])
     * r[32] holds (b[1]*b[1])
     */

    c1=(int)(tfbn_add_words(t,r,&(r[n2]),n2));

    if (neg) /* if t[32] is negative */
    {
        c1-=(int)(tfbn_sub_words(&(t[n2]),t,&(t[n2]),n2));
    }
    else
    {
        /* Might have a carry */
        c1+=(int)(tfbn_add_words(&(t[n2]),&(t[n2]),t,n2));
    }

    /* t[32] holds (a[0]-a[1])*(b[1]-b[0])+(a[0]*b[0])+(a[1]*b[1])
     * r[10] holds (a[0]*b[0])
     * r[32] holds (b[1]*b[1])
     * c1 holds the carry bits
     */
    c1+=(int)(tfbn_add_words(&(r[n]),&(r[n]),&(t[n2]),n2));
    if (c1)
    {
        p= &(r[n+n2]);
        lo= *p;
        ln=(lo+(ttBN_ULONG)c1)&TM_BN_MASK2;
        *p=ln;

        /* The overflow will stop before we over write
         * words we should not overwrite */
        if (ln < (ttBN_ULONG)c1)
        {
            do    {
                p++;
                lo= *p;
                ln=(lo+1)&TM_BN_MASK2;
                *p=ln;
            } while (ln == 0);
        }
    }
}

/* n+tn is the word length
 * t needs to be n*4 is size, as does r */
static void tfbn_mul_part_recursive(
    ttBN_ULONGPtr r, ttBN_ULONGPtr a, ttBN_ULONGPtr b,
    int tn, int n, ttBN_ULONGPtr t)
{
    int i,j,n2=n*2;
    tt32Bit c1,c2,neg;
    ttBN_ULONGPtr p;
    ttBN_ULONG ln,lo;

    if (n < 8)
    {
        i=tn+n;
        tfbn_mul_normal(r,a,i,b,i);
        return;
    }

    /* r=(a[0]-a[1])*(b[1]-b[0]) */
    c1=(tt32Bit)tfbn_cmp_words(a,&(a[n]),n);
    c2=(tt32Bit)tfbn_cmp_words(&(b[n]),b,n);
    neg=0;
    switch ((int)(c1*3+c2))
    {
    case -4:
        tfbn_sub_words(t,      &(a[n]),a,      n); /* - */
        tfbn_sub_words(&(t[n]),b,      &(b[n]),n); /* - */
        break;
    case -3:
        /*zero=1;*/
        /* break; */
    case -2:
        tfbn_sub_words(t,      &(a[n]),a,      n); /* - */
        tfbn_sub_words(&(t[n]),&(b[n]),b,      n); /* + */
        neg=1;
        break;
    case -1:
    case 0:
    case 1:
        /*zero=1;*/
        /* break; */
    case 2:
        tfbn_sub_words(t,      a,      &(a[n]),n); /* + */
        tfbn_sub_words(&(t[n]),b,      &(b[n]),n); /* - */
        neg=1;
        break;
    case 3:
        /*zero=1;*/
        /* break; */
    case 4:
        tfbn_sub_words(t,      a,      &(a[n]),n);
        tfbn_sub_words(&(t[n]),&(b[n]),b,      n);
        break;
    default:
        break;
    }
        /* The zero case isn't yet implemented here. The speedup
           would probably be negligible. */
    if (n == 8)
    {
        tfbn_mul_comba8(&(t[n2]),t,&(t[n]));
        tfbn_mul_comba8(r,a,b);
        tfbn_mul_normal(&(r[n2]),&(a[n]),tn,&(b[n]),tn);
        tm_bzero(&(r[n2+tn*2]),sizeof(ttBN_ULONG)*(unsigned int)(n2-tn*2));
    }
    else
    {
        p= &(t[n2*2]);
        tfbn_mul_recursive(&(t[n2]),t,&(t[n]),n,p);
        tfbn_mul_recursive(r,a,b,n,p);
        i=n/2;
        /* If there is only a bottom half to the number,
         * just do it */
        j=tn-i;
        if (j == 0)
        {
            tfbn_mul_recursive(&(r[n2]),&(a[n]),&(b[n]),i,p);
            tm_bzero(&(r[n2+i*2]),sizeof(ttBN_ULONG)*(unsigned int)(n2-i*2));
        }
        else if (j > 0) /* eg, n == 16, i == 8 and tn == 11 */
            {
                tfbn_mul_part_recursive(&(r[n2]),&(a[n]),&(b[n]),
                    j,i,p);
                tm_bzero(&(r[n2+tn*2]),
                         sizeof(ttBN_ULONG)*(unsigned int)(n2-tn*2));
        }
        else /* (j < 0) eg, n == 16, i == 8 and tn == 5 */
        {
            tm_bzero(&(r[n2]), sizeof(ttBN_ULONG)*(unsigned int)n2);
            if (tn < TM_BN_MUL_RECURSIVE_SIZE_NORMAL)
            {
                tfbn_mul_normal(&(r[n2]),&(a[n]),tn,&(b[n]),tn);
            }
            else
            {
                for (;;)
                {
                    i/=2;
                    if (i < tn)
                    {
                        tfbn_mul_part_recursive(&(r[n2]),
                            &(a[n]),&(b[n]),
                            tn-i,i,p);
                        break;
                    }
                    else if (i == tn)
                    {
                        tfbn_mul_recursive(&(r[n2]),
                            &(a[n]),&(b[n]),
                            i,p);
                        break;
                    }
                }
            }
        }
    }

    /* t[32] holds (a[0]-a[1])*(b[1]-b[0]), c1 is the sign
     * r[10] holds (a[0]*b[0])
     * r[32] holds (b[1]*b[1])
     */

    c1 = tfbn_add_words(t,r,&(r[n2]),n2);

    if (neg) /* if t[32] is negative */
    {
        c1 -= tfbn_sub_words(&(t[n2]),t,&(t[n2]),n2);
    }
    else
    {
        /* Might have a carry */
        c1 += tfbn_add_words(&(t[n2]),&(t[n2]),t,n2);
    }

    /* t[32] holds (a[0]-a[1])*(b[1]-b[0])+(a[0]*b[0])+(a[1]*b[1])
     * r[10] holds (a[0]*b[0])
     * r[32] holds (b[1]*b[1])
     * c1 holds the carry bits
     */
    c1 += tfbn_add_words(&(r[n]),&(r[n]),&(t[n2]),n2);
    if (c1)
    {
        p= &(r[n+n2]);
        lo= *p;
        ln=(lo+c1)&TM_BN_MASK2;
        *p=ln;

        /* The overflow will stop before we over write
         * words we should not overwrite */
        if (ln < c1)
        {
            do    {
                p++;
                lo= *p;
                ln=(lo+1)&TM_BN_MASK2;
                *p=ln;
            } while (ln == 0);
        }
    }
}



#endif /* TM_BN_RECURSION */

int tfBN_mul(
    ttBIGNUMBERPtr r, ttBIGNUMBERPtr a, ttBIGNUMBERPtr b, ttBN_CTXPtr ctx)
{
    int top,al,bl;
    ttBIGNUMBERPtr rr,bn;
    int ret = 0,errCode;
#if defined(TM_BN_MUL_COMBA) || defined(TM_BN_RECURSION)
    int i;
#endif
#ifdef TM_BN_RECURSION
    ttBIGNUMBERPtr t;
    int j,k;
#endif

    al=a->top;
    bl=b->top;

    if ((al == 0) || (bl == 0))
    {
        errCode = tm_bn_zero(r);
        if (!errCode) return 0;
        return(1);
    }
    top=al+bl;

    tfBN_CTX_start(ctx);
    if ((r == a) || (r == b))
    {
        rr = tfBN_CTX_get(ctx);
        if (rr == TM_OPENSSL_NULL) goto err;
    }
    else
    {
        rr = r;
    }
    rr->neg=a->neg^b->neg;

#if defined(TM_BN_MUL_COMBA) || defined(TM_BN_RECURSION)
    i = al-bl;
#endif
#ifdef TM_BN_MUL_COMBA
    if (i == 0)
    {
        if (al == 8)
        {
            bn = tm_bn_wexpand(rr,16);
            if (bn == TM_OPENSSL_NULL) goto err;
            rr->top=16;
            tfbn_mul_comba8(rr->d,a->d,b->d);
            goto end;
        }
    }
#endif /* TM_BN_MUL_COMBA */
#ifdef TM_BN_RECURSION
    if ((al >= TM_BN_MULL_SIZE_NORMAL) && (bl >= TM_BN_MULL_SIZE_NORMAL))
    {
        if (i == 1 && !tm_bn_get_flags(b,TM_BN_FLG_STATIC_DATA))
        {
            bn = tm_bn_wexpand(b,al);
            if (bn == TM_OPENSSL_NULL) goto err;

            b->d[bl]=0;
            bl++;
            i--;
        }
        else if (i == -1 && !tm_bn_get_flags(a,TM_BN_FLG_STATIC_DATA))
        {
            bn = tm_bn_wexpand(a,bl);
            if (bn == TM_OPENSSL_NULL) goto err;

            a->d[al]=0;
            al++;
            i++;
        }
        if (i == 0)
        {
            /* symmetric and > 4 */
            /* 16 or larger */
            j=tfbn_num_bits_word((ttBN_ULONG)al);
            j=1<<(j-1);
            k=j+j;
            t = tfBN_CTX_get(ctx);
            if (t == TM_OPENSSL_NULL)
            {
                goto err;
            }
            if (al == j) /* exact multiple */
            {
                bn = tm_bn_wexpand(t,k*2);
                if (bn == TM_OPENSSL_NULL) goto err;
                bn = tm_bn_wexpand(rr,k*2);
                if (bn == TM_OPENSSL_NULL) goto err;

                tfbn_mul_recursive(rr->d,a->d,b->d,al,t->d);
            }
            else
            {
                bn = tm_bn_wexpand(a,k);
                if (bn == TM_OPENSSL_NULL) goto err;
                bn = tm_bn_wexpand(b,k);
                if (bn == TM_OPENSSL_NULL) goto err;
                bn = tm_bn_wexpand(t,k*4);
                if (bn == TM_OPENSSL_NULL) goto err;
                bn = tm_bn_wexpand(rr,k*4);
                if (bn == TM_OPENSSL_NULL) goto err;

                for (i=a->top; i<k; i++)
                    a->d[i]=0;
                for (i=b->top; i<k; i++)
                    b->d[i]=0;
                tfbn_mul_part_recursive(rr->d,a->d,b->d,al-j,j,t->d);
            }
            rr->top=top;
            goto end;
        }
    }
#endif /* TM_BN_RECURSION */
    bn = tm_bn_wexpand(rr,top);
    if (bn == TM_OPENSSL_NULL) goto err;
    rr->top=top;
    tfbn_mul_normal(rr->d,a->d,al,b->d,bl);

#if defined(TM_BN_MUL_COMBA) || defined(TM_BN_RECURSION)
end:
#endif
    tm_bn_fix_top(rr);
    ret=1;
    if (r != rr)
    {
        bn = tfBN_copy(r, rr);
        if (bn == TM_OPENSSL_NULL) 
        {
            ret = 0;
        }
    }
err:
    tfBN_CTX_end(ctx);
    return(ret);
}

static void tfbn_mul_normal(
    ttBN_ULONGPtr r, ttBN_ULONGPtr a, int na, ttBN_ULONGPtr b, int nb)
{
    ttBN_ULONGPtr rr;


    if (na < nb)
    {
        int itmp;
        ttBN_ULONGPtr ltmp;

        itmp=na; na=nb; nb=itmp;
        ltmp=a;   a=b;   b=ltmp;

    }
    rr= &(r[na]);
    rr[0]= tfbn_mul_words(r,a,na,b[0]);

    for (;;)
    {
        nb--;
        if (nb <= 0) return;
        rr[1]= tfbn_mul_add_words(&(r[1]),a,na,b[1]);
        nb--;
        if (nb <= 0) return;
        rr[2]= tfbn_mul_add_words(&(r[2]),a,na,b[2]);
        nb--;
        if (nb <= 0) return;
        rr[3]= tfbn_mul_add_words(&(r[3]),a,na,b[3]);
        nb--;
        if (nb <= 0) return;
        rr[4]= tfbn_mul_add_words(&(r[4]),a,na,b[4]);
        rr+=4;
        r+=4;
        b+=4;
    }
}

#else /* ! (TM_PUBKEY_USE_DIFFIEHELLMAN || TM_PUBKEY_USE_DSA || RSA */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when none of the macroes is not defined */
int tvBignumDummy = 0;

#endif /* TM_PUBKEY_USE_DIFFIEHELLMAN || TM_PUBKEY_USE_DSA || RSA */
