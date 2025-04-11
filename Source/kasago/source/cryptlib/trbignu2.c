/*
 * Description: OpenSSL big number lib (part 2 of 2)
 *
 * Filename: trbignu2.c
 * Author: Jin Zhang
 * Date Created: 5/10/2002
 * $Source: source/cryptlib/trbignu2.c $
 *
 * Modification History
 * $Revision: 6.0.2.6 $
 * $Date: 2012/11/29 05:29:42JST $
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

/* Add for RSA */
#ifdef TM_USE_PKI
static int          tfBN_rshift1(ttBIGNUMBERPtr r, ttBIGNUMBERPtr a);

#ifndef TM_OPTIMIZE_SIZE
static int          tfBN_reciprocal(ttBIGNUMBERPtr r,
                                    ttBIGNUMBERPtr m,
                                    int len,
                                    ttBN_CTXPtr ctx);
static int          tfBN_mod_exp_recp(ttBIGNUMBERPtr r,
                                      ttConstBIGNUMBERPtr a,
                                      ttConstBIGNUMBERPtr p,
                                      ttConstBIGNUMBERPtr m,
                                      ttBN_CTXPtr ctx);
static int          tfBN_RECP_CTX_set(ttBN_RECP_CTXPtr recp,
                                      ttConstBIGNUMBERPtr rdiv,
                                      ttBN_CTXPtr ctx);
static int          tfBN_mod_mul_reciprocal(ttBIGNUMBERPtr r,
                                            ttBIGNUMBERPtr x,
                                            ttBIGNUMBERPtr y,
                                            ttBN_RECP_CTXPtr recp,
                                            ttBN_CTXPtr ctx);
static int          tfBN_div_recp(ttBIGNUMBERPtr dv,
                                  ttBIGNUMBERPtr rem,
                                  ttBIGNUMBERPtr m,
                                  ttBN_RECP_CTXPtr recp,
                                  ttBN_CTXPtr ctx);
static void         tfBN_RECP_CTX_free(ttBN_RECP_CTXPtr recp);
static void         tfBN_RECP_CTX_init(ttBN_RECP_CTXPtr recp);
#endif /* ! TM_OPTIMIZE_SIZE */
#endif /* TM_USE_PKI */


/*Big number library API */
static int          tfBN_uadd(ttBIGNUMBERPtr r,
                              ttConstBIGNUMBERPtr a,
                              ttConstBIGNUMBERPtr b);
static void         tfBN_MONT_CTX_init(ttBN_MONT_CTXPtr ctx);
static int          tfBN_from_montgomery(ttBIGNUMBERPtr r,
                                         ttBIGNUMBERPtr a,
                                         ttBN_MONT_CTXPtr mont,
                                         ttBN_CTXPtr ctx);
static int          tfbn_mod_mul_montgomery(ttBIGNUMBERPtr r,
                                            ttBIGNUMBERPtr a,
                                            ttBIGNUMBERPtr b,
                                            ttBN_MONT_CTXPtr mont,
                                            ttBN_CTXPtr ctx);


#define TABLE_SIZE    32

#ifdef TM_USE_PKI
static int tfBN_rshift1(ttBIGNUMBERPtr r, ttBIGNUMBERPtr a)
{
    ttBN_ULONG t,c;
    ttBN_ULONGPtr ap,rp;
    ttBIGNUMBERPtr bn;
    int i;
    int ret=0;

    if (tm_bn_is_zero(a))
    {
        ret = tm_bn_zero(r);
        if (!ret) return 0;
        return(1);
    }
    if (a != r)
    {
        bn = tm_bn_wexpand(r,a->top);
        if( bn == TM_OPENSSL_NULL ) return(0);
        r->top=a->top;
        r->neg=a->neg;
    }
    ap=a->d;
    rp=r->d;
    c=0;
    for (i=a->top-1; i>=0; i--)
    {
        t=ap[i];
        rp[i]=((t>>1)&TM_BN_MASK2)|c;
        c=(t&1)?TM_BN_TBIT:0;
    }
    tm_bn_fix_top(r);
    return(1);
}

ttBIGNUMBERPtr  tfBN_dup(ttConstBIGNUMBERPtr a)
{
    ttBIGNUMBERPtr r;

    if (a == TM_OPENSSL_NULL) return TM_OPENSSL_NULL;


    r= tfBN_new();
    if (r == TM_OPENSSL_NULL) return(TM_OPENSSL_NULL);
    return((ttBIGNUMBERPtr ) tfBN_copy(r,a));
}

int tfBN_mask_bits(ttBIGNUMBERPtr a, int n)
{
    int b,w;

    w=n/TM_BN_BITS2;
    b=n%TM_BN_BITS2;
    if (w >= a->top) return(0);
    if (b == 0)
        a->top=w;
    else
    {
        a->top=w+1;
        a->d[w]&= ~(TM_BN_MASK2<<b);
    }
    tm_bn_fix_top(a);
    return(1);
}

/*bn_gcd.c -------------------------------------------------*/
int tfBN_gcd(
    ttBIGNUMBERPtr r, ttBIGNUMBERPtr in_a, ttBIGNUMBERPtr in_b,
    ttBN_CTXPtr ctx)
{
    ttBIGNUMBERPtr a,b,t,bn;
    int ret=0,cmp;

    tm_bn_check_top(in_a);
    tm_bn_check_top(in_b);

    tfBN_CTX_start(ctx);
    a = tfBN_CTX_get(ctx);
    b = tfBN_CTX_get(ctx);
    if (a == TM_OPENSSL_NULL || b == TM_OPENSSL_NULL) goto err;

    bn = tfBN_copy(a,in_a);
    if( bn == TM_OPENSSL_NULL ) goto err;

    bn = tfBN_copy(b,in_b);
    if( bn == TM_OPENSSL_NULL ) goto err;

    cmp = tfBN_cmp(a,b);
    if( cmp < 0 )
    {
        t=a;
        a=b;
        b=t;
    }

    t= tfEuclid(a,b);
    if (t == TM_OPENSSL_NULL) goto err;

    bn = tfBN_copy(r,t);
    if( bn == TM_OPENSSL_NULL ) goto err;

    ret=1;
err:
    tfBN_CTX_end(ctx);
    return(ret);
}

ttBIGNUMBERPtr  tfEuclid(ttBIGNUMBERPtr a, ttBIGNUMBERPtr b)
{
    ttBIGNUMBERPtr t;
    int shifts=0,cmp;

    tm_bn_check_top(a);
    tm_bn_check_top(b);

    for (;;)
        {
        if (tm_bn_is_zero(b))
            break;

        if (tm_bn_is_odd(a))
        {
            if (tm_bn_is_odd(b))
            {
                cmp = tfBN_sub(a,a,b);
                if( !cmp ) goto err;
                cmp = tfBN_rshift1(a,a);
                if( !cmp ) goto err;
                cmp = tfBN_cmp(a,b);
                if( cmp < 0 )
                {
                    t=a;
                    a=b;
                    b=t;
                }
            }
            else        /* a odd - b even */
            {
                cmp = tfBN_rshift1(b,b);
                if( !cmp ) goto err;
                cmp = tfBN_cmp(a,b);
                if( cmp < 0 )
                {
                    t=a;
                    a=b;
                    b=t;
                }
            }
        }
        else            /* a is even */
        {
            if (tm_bn_is_odd(b))
            {
                cmp = tfBN_rshift1(a,a);
                if( !cmp ) goto err;
                cmp = tfBN_cmp(a,b);
                if( cmp < 0 )
                {
                    t=a;
                    a=b;
                    b=t;
                }
            }
            else        /* a even - b even */
            {
                cmp = tfBN_rshift1(a,a);
                if( !cmp ) goto err;
                cmp = tfBN_rshift1(b,b);
                if( !cmp ) goto err;

                shifts++;
            }
        }
    }
    if (shifts)
    {
        cmp = tfBN_lshift(a,a,shifts);
        if( !cmp ) goto err;
    }
    return(a);
err:
    return(TM_OPENSSL_NULL);
}

int tfBN_mod_exp(ttBIGNUMBERPtr r, ttBIGNUMBERPtr a, ttConstBIGNUMBERPtr p, 
                      ttConstBIGNUMBERPtr m,
                      ttBN_CTXPtr ctx)
{
    int ret;

    ret = 0;
#if !defined(TM_RECP_MUL_MOD) && !defined(TM_MONT_MUL_MOD)
    TM_UNREF_IN_ARG(ctx);
    TM_UNREF_IN_ARG(m);
    TM_UNREF_IN_ARG(p);
    TM_UNREF_IN_ARG(a);
    TM_UNREF_IN_ARG(r);
#endif /* !defined(TM_RECP_MUL_MOD) && !defined(TM_MONT_MUL_MOD) */


#ifdef TM_MONT_MUL_MOD
    /* I have finally been able to take out this pre-condition of
     * the top bit being set.  It was caused by an error in tfBN_div
     * with negatives.  There was also another problem when for a^b%m
     * a >= m.  eay 07-May-97 */
/*    if ((m->d[m->top-1]&TM_BN_TBIT) && tm_bn_is_odd(m)) */

    if (tm_bn_is_odd(m))
    {
        if (a->top == 1)
        {
            ttBN_ULONG A = a->d[0];
            ret= tfBN_mod_exp_mont_word(r,A,p,m,ctx,TM_OPENSSL_NULL);
        }
        else
            ret= tfBN_mod_exp_mont(r,a,p,m,ctx,TM_OPENSSL_NULL);
    }
#ifdef TM_RECP_MUL_MOD
    else
#endif /* TM_RECP_MUL_MOD */
#endif /* TM_MOND_MUL_MOD */
#ifdef TM_RECP_MUL_MOD
    { ret= tfBN_mod_exp_recp(r,a,p,m,ctx); }
#endif /* TM_RECP_MUL_MOD */

    return(ret);
}

#ifdef TM_RECP_MUL_MOD
static int tfBN_mod_exp_recp(ttBIGNUMBERPtr r, ttConstBIGNUMBERPtr a,
                           ttConstBIGNUMBERPtr p, ttConstBIGNUMBERPtr m, 
                           ttBN_CTXPtr ctx)
{
    int i,j,bits,ret=0,wstart,wend,window,wvalue;
    int start=1,ts=0,cmp,errCode;
    ttBIGNUMBERPtr aa;
    ttBIGNUMBER TM_FAR * val;
    ttBN_RECP_CTX recp;

    val = (ttBIGNUMBER TM_FAR *)0;
    bits=tm_bn_num_bits(p);

    if (bits == 0)
    {
        errCode = tm_bn_one(r);
        if (!errCode) return 0;
        return(1);
    }
    val = tm_get_raw_buffer(TABLE_SIZE * sizeof(ttBIGNUMBER));
    if (val == (ttBIGNUMBER TM_FAR *)0)
    {
        goto err;
    }

    tfBN_CTX_start(ctx);
    aa = tfBN_CTX_get(ctx);
    if( aa == TM_OPENSSL_NULL ) goto err;


    tfBN_RECP_CTX_init(&recp);
    cmp = tfBN_RECP_CTX_set(&recp,m,ctx);
    if( cmp <= 0) goto err;

    tfBN_init(&(val[0]));
    ts=1;

    cmp = tfBN_mod(&(val[0]),a,m,ctx);
    if( !cmp ) goto err;        /* 1 */

    window = tm_bn_winbits_for_exponent_size(bits);
    if (window > 1)
    {
        cmp = tfBN_mod_mul_reciprocal(aa,&(val[0]),&(val[0]),&recp,ctx);
        if( !cmp ) goto err;                /* 2 */

        j=1<<(window-1);
        for (i=1; i<j; i++)
        {
            tfBN_init(&val[i]);
            cmp = tfBN_mod_mul_reciprocal( &(val[i]),
                                           &(val[i-1]),
                                           aa,
                                           &recp,ctx);
            if (!cmp )
                goto err;
        }
        ts=i;
    }
        
    start=1;    /* This is used to avoid multiplication etc
             * when there is only the value '1' in the
             * buffer. */
    wvalue=0;    /* The 'value' of the window */
    wstart=bits-1;    /* The top bit of the window */
    wend=0;        /* The bottom bit of the window */

    cmp = tm_bn_one(r);
    if (!cmp) goto err;

    for (;;)
    {
        cmp = tfBN_is_bit_set(p,wstart);
        if( cmp == 0)
        {
            if (!start)
            {
                cmp = tfBN_mod_mul_reciprocal(r,r,r,&recp,ctx);
                if( !cmp ) goto err;
            }

            if (wstart == 0) break;
            wstart--;
            continue;
        }
        /* We now have wstart on a 'set' bit, we now need to work out
         * how bit a window to do.  To do this we need to scan
         * forward until the last set bit before the end of the
         * window */
        j=wstart;
        wvalue=1;
        wend=0;
        for (i=1; i<window; i++)
        {
            if (wstart-i < 0) break;
            
            cmp = tfBN_is_bit_set(p,wstart-i);
            if( cmp )
            {
                wvalue<<=(i-wend);
                wvalue|=1;
                wend=i;
            }
        }

        /* wend is the size of the current window */
        j=wend+1;
        /* add the 'bytes above' */
        if (!start)
            for (i=0; i<j; i++)
            {
                cmp = tfBN_mod_mul_reciprocal(r,r,r,&recp,ctx);
                if( !cmp ) goto err;
            }
        
        /* wvalue will be an odd number < 2^window */
        cmp = tfBN_mod_mul_reciprocal(r,r,&(val[wvalue>>1]),&recp,ctx);
        if (!cmp ) goto err;

        /* move the 'window' down further */
        wstart-=wend+1;
        wvalue=0;
        start=0;
        if (wstart < 0) break;
    }
    ret=1;
err:
    if (val != (ttBIGNUMBER TM_FAR *)0)
    {
        tfBN_CTX_end(ctx);
        for (i=0; i<ts; i++)
            tfBN_clear_free(&(val[i]));
        tfBN_RECP_CTX_free(&recp);
        tm_free_raw_buffer(val);
    }
    return(ret);
}
#endif /* TM_RECP_MUL_MOD */

/**dh_recp.c------------------------------------------------------*/
#ifndef TM_OPTIMIZE_SIZE
static void tfBN_RECP_CTX_init(ttBN_RECP_CTXPtr recp)
{
    tfBN_init(&(recp->N));
    tfBN_init(&(recp->Nr));
    recp->num_bits=0;
    recp->flags=0;
}

static void tfBN_RECP_CTX_free(ttBN_RECP_CTXPtr recp)
{
    if(recp == TM_OPENSSL_NULL)
        return;

    tfBN_free(&(recp->N));
    tfBN_free(&(recp->Nr));
    if (recp->flags & TM_BN_FLG_MALLOCED)
        tm_free_raw_buffer((tt8BitPtr)recp);
}

static int tfBN_RECP_CTX_set(
    ttBN_RECP_CTXPtr recp, ttConstBIGNUMBERPtr d, ttBN_CTXPtr ctx)
{
    int errCode;
    ttBIGNUMBERPtr   bn;

    TM_UNREF_IN_ARG(ctx);

    bn = tfBN_copy(&(recp->N),d);
    if (!bn) return 0;
    errCode = tm_bn_zero(&(recp->Nr));
    if (!errCode) return 0;
    recp->num_bits=tm_bn_num_bits(d);
    recp->shift=0;
    return(1);
}



static int tfBN_mod_mul_reciprocal(
    ttBIGNUMBERPtr r, ttBIGNUMBERPtr x, ttBIGNUMBERPtr y,
    ttBN_RECP_CTXPtr recp, ttBN_CTXPtr ctx)
{
    int ret=0,cmp;
    ttBIGNUMBERPtr a;

    tfBN_CTX_start(ctx);
    a = tfBN_CTX_get(ctx);
    if (a == TM_OPENSSL_NULL) goto err;
    if (y != TM_OPENSSL_NULL)
    {
        if (x == y)
        {
            cmp = tfBN_sqr(a,x,ctx);
            if( !cmp ) goto err;
        }
        else
        {
            cmp = tfBN_mul(a,x,y,ctx);
            if( !cmp ) goto err;
        }
    }
    else
    {
        a=x; /* Just do the mod */
    }
    ret = tfBN_div_recp(TM_OPENSSL_NULL,r,a,recp,ctx);
err:
    tfBN_CTX_end(ctx);
    return(ret);
}

static int tfBN_div_recp(
    ttBIGNUMBERPtr dv, ttBIGNUMBERPtr rem, ttBIGNUMBERPtr m,
    ttBN_RECP_CTXPtr recp, ttBN_CTXPtr ctx)
{
    int i,j,ret=0,cmp,errCode;
    ttBIGNUMBERPtr a,b,d,r,bn;

    tfBN_CTX_start(ctx);
    a= tfBN_CTX_get(ctx);
    b= tfBN_CTX_get(ctx);
    if (dv != TM_OPENSSL_NULL)
        d=dv;
    else
        d= tfBN_CTX_get(ctx);
    if (rem != TM_OPENSSL_NULL)
        r=rem;
    else
        r= tfBN_CTX_get(ctx);
    if (a == TM_OPENSSL_NULL || b == TM_OPENSSL_NULL || d == TM_OPENSSL_NULL
        || r == TM_OPENSSL_NULL) goto err;

    cmp = tfBN_ucmp(m,&(recp->N));
    if( cmp < 0 )
    {
        errCode = tm_bn_zero(d);
        if (!errCode) goto err;
        bn = tfBN_copy(r,m);
        if (!bn) goto err;
        tfBN_CTX_end(ctx);
        return(1);
    }

    /* We want the remainder
     * Given input of ABCDEF / ab
     * we need multiply ABCDEF by 3 digests of the reciprocal of ab
     *
     */
    i=tm_bn_num_bits(m);

    j=recp->num_bits<<1;
    if (j>i) i=j;
    j>>=1;

    if (i != recp->shift)
    {
        recp->shift= tfBN_reciprocal(&(recp->Nr),&(recp->N),
            i,ctx);
        if (recp->shift < 0)
        {
            goto err;
        }
    }

    cmp = tfBN_rshift(a,m,j);
    if( !cmp ) goto err;

    cmp = tfBN_mul(b,a,&(recp->Nr),ctx);
    if( !cmp ) goto err;

    cmp = tfBN_rshift(d,b,i-j);
    if( !cmp ) goto err;

    d->neg=0;
    cmp = tfBN_mul(b,&(recp->N),d,ctx);
    if( !cmp ) goto err;
    cmp = tfBN_usub(r,m,b);
    if( !cmp ) goto err;

    r->neg=0;

    j=0;
    cmp = tfBN_ucmp(r,&(recp->N));
    while ( cmp >= 0)
    {
        if (j > 2)
        {
            goto err;
        }
        j++;
        cmp = tfBN_usub(r,r,&(recp->N));
        if( !cmp ) goto err;
        cmp = tfBN_add_word(d,1);
        if( !cmp ) goto err;
        cmp = tfBN_ucmp(r,&(recp->N));
    }

    r->neg=tm_bn_is_zero(r)?0:m->neg;
    d->neg=m->neg^recp->N.neg;
    ret=1;
err:
    tfBN_CTX_end(ctx);
    return(ret);
}

/* len is the expected size of the result
 * We actually calculate with an extra word of precision, so
 * we can do faster division if the remainder is not required.
 */
static int tfBN_reciprocal(
    ttBIGNUMBERPtr r, ttBIGNUMBERPtr m, int len, ttBN_CTXPtr ctx)
{
    int ret= -1,cmp,errCode;
    ttBIGNUMBER t;

    tfBN_init(&t);

    errCode = tm_bn_zero(&t);
    if (!errCode) goto err;

    cmp = tfBN_set_bit(&t,len);
    if( !cmp ) goto err;

    cmp = tfBN_div(r,TM_OPENSSL_NULL,&t,m,ctx);
    if( !cmp ) goto err;

    ret=len;
err:
    tfBN_free(&t);
    return(ret);
}
#endif /* TM_OPTIMIZE_SIZE */

int tfBN_BLINDING_update(ttBN_BLINDINGPtr b, ttBN_CTXPtr ctx)
{
    int ret=0,cmp;

    if ((b->A == TM_OPENSSL_NULL) || (b->Ai == TM_OPENSSL_NULL))
    {
        tm_BNerr( tfBN_F_BN_BLINDING_UPDATE,BN_R_NOT_INITIALIZED);
        goto err;
    }

    cmp = tfBN_mod_mul(b->A,b->A,b->A,b->mod,ctx);
    if( !cmp ) goto err;

    cmp = tfBN_mod_mul(b->Ai,b->Ai,b->Ai,b->mod,ctx);
    if( !cmp ) goto err;

    ret=1;
err:
    return(ret);
}

int tfBN_BLINDING_convert(
    ttBIGNUMBERPtr n, ttBN_BLINDINGPtr b, ttBN_CTXPtr ctx)
{
    int ret;
    tm_bn_check_top(n);

    if ((b->A == TM_OPENSSL_NULL) || (b->Ai == TM_OPENSSL_NULL))
    {
        tm_BNerr( tfBN_F_BN_BLINDING_CONVERT,BN_R_NOT_INITIALIZED);
        ret = 0;
    }
    else
    {
        ret = tfBN_mod_mul(n,n,b->A,b->mod,ctx);
    }
    return ret;
}

int tfBN_BLINDING_invert(ttBIGNUMBERPtr n, ttBN_BLINDINGPtr b, ttBN_CTXPtr ctx)
{
    int ret;

    tm_bn_check_top(n);
    if ((b->A == TM_OPENSSL_NULL) || (b->Ai == TM_OPENSSL_NULL))
    {
        tm_BNerr( tfBN_F_BN_BLINDING_INVERT,BN_R_NOT_INITIALIZED);
        ret = 0;
    }
    else
    {
        ret= tfBN_mod_mul(n,n,b->Ai,b->mod,ctx);
        if (ret != 0)
        {
            ret = tfBN_BLINDING_update(b,ctx);
        }
    }
    return(ret);
}

void tfBN_BLINDING_free(ttBN_BLINDINGPtr r)
{
    if (r != TM_OPENSSL_NULL)
    {
        if (r->A  != TM_OPENSSL_NULL) tfBN_free(r->A );
        if (r->Ai != TM_OPENSSL_NULL) tfBN_free(r->Ai);
        tm_free_raw_buffer((tt8BitPtr)r);
    }
    return;
}

ttBN_BLINDINGPtr tfBN_BLINDING_new(
    ttBIGNUMBERPtr A, ttBIGNUMBERPtr Ai, ttBIGNUMBERPtr mod)
{
    ttBN_BLINDINGPtr ret=TM_OPENSSL_NULL;
    ttBIGNUMBERPtr   bn;

    tm_bn_check_top(Ai);
    tm_bn_check_top(mod);

    ret=(ttBN_BLINDINGPtr)tm_get_raw_buffer(sizeof(ttBN_BLINDING));
    if (ret == TM_OPENSSL_NULL)
    {
        tm_BNerr( tfBN_F_BN_BLINDING_NEW,ERR_R_MALLOC_FAILURE);
        return(TM_OPENSSL_NULL);
    }
    tm_memset(ret,0,sizeof(ttBN_BLINDING));
    ret->A= tfBN_new();
    if(ret->A == TM_OPENSSL_NULL) goto err;
    ret->Ai= tfBN_new();
    if(ret->Ai == TM_OPENSSL_NULL) goto err;
    bn = tfBN_copy(ret->A,A);
    if(!bn ) goto err;
    bn = tfBN_copy(ret->Ai,Ai);
    if(!bn ) goto err;

    ret->mod=mod;
    return(ret);
err:
    tfBN_BLINDING_free(ret);
    return(TM_OPENSSL_NULL);
}

int tfBN_mod_exp2_mont(
    ttBIGNUMBERPtr rr, ttBIGNUMBERPtr a1, ttBIGNUMBERPtr p1, ttBIGNUMBERPtr a2,
    ttBIGNUMBERPtr p2, ttBIGNUMBERPtr m, ttBN_CTXPtr ctx,
    ttBN_MONT_CTXPtr in_mont)
{
    int i,j;
    int bits,b,bits1,bits2,ret=0,wpos1,wpos2,window1,window2,wvalue1,wvalue2;
    int r_is_one=1,ts1=0,ts2=0,errCode;
    ttBIGNUMBERPtr d,r;
    ttBIGNUMBERPtr a_mod_m;
    ttBIGNUMBER TM_FAR * val1;
    ttBIGNUMBER TM_FAR * val2;
    ttBN_MONT_CTXPtr mont=TM_OPENSSL_NULL;

    val1 = (ttBIGNUMBER TM_FAR *)0;
    val2 = (ttBIGNUMBER TM_FAR *)0;
    tm_bn_check_top(a1);
    tm_bn_check_top(p1);
    tm_bn_check_top(a2);
    tm_bn_check_top(p2);
    tm_bn_check_top(m);

    if (!(m->d[0] & 1))
    {
        tm_BNerr( tfBN_F_BN_MOD_EXP2_MONT,BN_R_CALLED_WITH_EVEN_MODULUS);
        return(0);
    }
    bits1=tm_bn_num_bits(p1);
    bits2=tm_bn_num_bits(p2);
    if ((bits1 == 0) && (bits2 == 0))
    {
        errCode = tm_bn_one(rr);
        if (!errCode) return 0;
        return(1);
    }
    bits=(bits1 > bits2)?bits1:bits2;
    val1 = tm_get_raw_buffer(TABLE_SIZE * sizeof(ttBIGNUMBER));
    if (val1 == (ttBIGNUMBER TM_FAR *)0)
    {
        goto err;
    }
    val2 = tm_get_raw_buffer(TABLE_SIZE * sizeof(ttBIGNUMBER));
    if (val2 == (ttBIGNUMBER TM_FAR *)0)
    {
        goto err;
    }

    tfBN_CTX_start(ctx);
    d = tfBN_CTX_get(ctx);
    r = tfBN_CTX_get(ctx);
    if (d == TM_OPENSSL_NULL || r == TM_OPENSSL_NULL) goto err;

    if (in_mont != TM_OPENSSL_NULL)
        mont=in_mont;
    else
    {
        mont= tfBN_MONT_CTX_new();
        if (mont == TM_OPENSSL_NULL) goto err;
        errCode = tfBN_MONT_CTX_set(mont,m,ctx);
        if (!errCode ) goto err;
    }

    window1 = tm_bn_winbits_for_exponent_size(bits1);
    window2 = tm_bn_winbits_for_exponent_size(bits2);

/*
 * Build table for a1: val1[i] := a1^(2*i + 1) mod m  for i=0..2^(window1-1)
 */
    tfBN_init(&val1[0]);
    ts1=1;
    errCode = tfBN_ucmp(a1,m);
    if (errCode  >= 0)
    {
        errCode = tfBN_mod(&(val1[0]),a1,m,ctx);
        if (!errCode )
            goto err;
        a_mod_m = &(val1[0]);
    }
    else
    {
        a_mod_m = a1;
    }
    errCode = tm_bn_to_montgomery(&(val1[0]),a_mod_m,mont,ctx);
    if (!errCode) goto err;
    if (window1 > 1)
    {
        errCode = tfbn_mod_mul_montgomery(d,&(val1[0]),&(val1[0]),mont,ctx);
        if (!errCode) goto err;

        j=1<<(window1-1);
        for (i=1; i<j; i++)
        {
            tfBN_init(&(val1[i]));
            errCode = tfbn_mod_mul_montgomery(&(val1[i]),&(val1[i-1]),d,mont,ctx);
            if (!errCode)
                goto err;
        }
        ts1=i;
    }


/*
 * Build table for a2: val2[i] := a2^(2*i + 1) mod m  for i=0..2^(window2-1)
 */
    tfBN_init(&val2[0]);
    ts2=1;
    errCode = tfBN_ucmp(a2,m);
    if (errCode >= 0)
    {
        errCode = tfBN_mod(&(val2[0]),a2,m,ctx);
        if (!errCode )
            goto err;
        a_mod_m = &(val2[0]);
    }
    else
        a_mod_m = a2;
    errCode = tm_bn_to_montgomery(&(val2[0]),a_mod_m,mont,ctx);
    if (!errCode) goto err;
    if (window2 > 1)
    {
        errCode = tfbn_mod_mul_montgomery(d,&(val2[0]),&(val2[0]),mont,ctx);
        if (!errCode) goto err;

        j=1<<(window2-1);
        for (i=1; i<j; i++)
        {
            tfBN_init(&(val2[i]));
            errCode = tfbn_mod_mul_montgomery(&(val2[i]),&(val2[i-1]),d,mont,ctx);
            if (!errCode)
                goto err;
        }
        ts2=i;
    }


    /* Now compute the power product, using independent windows. */
    r_is_one=1;
    wvalue1=0;  /* The 'value' of the first window */
    wvalue2=0;  /* The 'value' of the second window */
    wpos1=0;    /* If wvalue1 > 0, the bottom bit of the first window */
    wpos2=0;    /* If wvalue2 > 0, the bottom bit of the second window */

    errCode = tm_bn_to_montgomery(r,tfBN_value_one(),mont,ctx);
    if (!errCode) goto err;
    for (b=bits-1; b>=0; b--)
    {
        if (!r_is_one)
        {
            errCode = tfbn_mod_mul_montgomery(r,r,r,mont,ctx);
            if (!errCode)
                goto err;
        }
        
        if (!wvalue1)
        {
            errCode = tfBN_is_bit_set(p1, b);
            if (errCode)
            {
                /* consider bits b-window1+1 .. b for this window */
                i = b-window1+1;
                errCode = tfBN_is_bit_set(p1, i);
                while (!errCode) /* works for i<0 */
                {
                    i++;
                    errCode = tfBN_is_bit_set(p1, i);
                }
                wpos1 = i;
                wvalue1 = 1;
                for (i = b-1; i >= wpos1; i--)
                {
                    wvalue1 <<= 1;
                    errCode = tfBN_is_bit_set(p1, i);
                    if (errCode)
                        wvalue1++;
                }
            }
        }
        
        if (!wvalue2)
        {
            errCode = tfBN_is_bit_set(p2, b);
            if (errCode)
            {
                /* consider bits b-window2+1 .. b for this window */
                i = b-window2+1;
                errCode = tfBN_is_bit_set(p2, i);
                while (!errCode)
                {
                    i++;
                    errCode = tfBN_is_bit_set(p2, i);
                }
                wpos2 = i;
                wvalue2 = 1;
                for (i = b-1; i >= wpos2; i--)
                {
                    wvalue2 <<= 1;
                    errCode = tfBN_is_bit_set(p2, i);
                    if (errCode)
                        wvalue2++;
                }
            }
        }

        if (wvalue1 && b == wpos1)
        {
            /* wvalue1 is odd and < 2^window1 */
            errCode = tfbn_mod_mul_montgomery(r,r,&(val1[wvalue1>>1]),mont,ctx);
            if (!errCode)
                goto err;
            wvalue1 = 0;
            r_is_one = 0;
        }
        
        if (wvalue2 && b == wpos2)
        {
            /* wvalue2 is odd and < 2^window2 */
            errCode = tfbn_mod_mul_montgomery(r,r,&(val2[wvalue2>>1]),mont,ctx);
            if (!errCode)
                goto err;
            wvalue2 = 0;
            r_is_one = 0;
        }
    }
    ret = tfBN_from_montgomery(rr,r,mont,ctx);
err:
    if ((val1 != (ttBIGNUMBER TM_FAR *)0) && (val2 != (ttBIGNUMBER TM_FAR *)0))
    {
        if ((in_mont == TM_OPENSSL_NULL) && (mont != TM_OPENSSL_NULL))
            tfBN_MONT_CTX_free(mont);
        tfBN_CTX_end(ctx);
        for (i=0; i<ts1; i++)
            tfBN_clear_free(&(val1[i]));
        for (i=0; i<ts2; i++)
            tfBN_clear_free(&(val2[i]));
    }
    if (val1 != (ttBIGNUMBER TM_FAR *)0)
    {
        tm_free_raw_buffer(val1);
    }
    if (val2 != (ttBIGNUMBER TM_FAR *)0)
    {
        tm_free_raw_buffer(val2);
    }
    return(ret);
}

#endif /* TM_USE_PKI */

/* tfBN_add.c ----------------------------------------------------*/
int tfBN_add(ttBIGNUMBERPtr r, ttConstBIGNUMBERPtr a, ttConstBIGNUMBERPtr b)
{
    ttConstBIGNUMBERPtr tmp;
    int cmp;

    /*  a +  b    a+b
     *  a + -b    a-b
     * -a +  b    b-a
     * -a + -b    -(a+b)
     */
    if (a->neg ^ b->neg)
    {
        /* only one is negative */
        if (a->neg)
        {
            tmp=a; a=b; b=tmp; 
        }

        /* we are now a - b */
        cmp = tfBN_ucmp(a,b);
        if( cmp < 0 )
        {
            cmp = tfBN_usub(r,b,a);
            if( !cmp ) return(0);
            r->neg=1;
        }
        else
        {
            cmp = tfBN_usub(r,a,b);
            if( !cmp ) return(0);
            r->neg=0;
        }
        return(1);
    }

    if (a->neg) /* both are neg */
        r->neg=1;
    else
        r->neg=0;

    cmp = tfBN_uadd(r,a,b);
    if( !cmp ) return(0);
    return(1);
}

/* unsigned add of b to a, r must be large enough */
static int tfBN_uadd(
    ttBIGNUMBERPtr r, ttConstBIGNUMBERPtr a, ttConstBIGNUMBERPtr b)
{
    register int i;
    int max,min;
    ttBN_ULONGPtr ap,bp,rp;
    ttBN_ULONG carry,t1;
    ttConstBIGNUMBERPtr tmp;
    ttBIGNUMBERPtr bn;

    if (a->top < b->top)
    {
        tmp=a; a=b; b=tmp; 
    }
    max=a->top;
    min=b->top;

    bn = tm_bn_wexpand(r,max+1);
    if( bn == TM_OPENSSL_NULL )
        return(0);

    r->top=max;


    ap=a->d;
    bp=b->d;
    rp=r->d;
    carry=0;

    carry=tfbn_add_words(rp,ap,bp,min);
    rp+=min;
    ap+=min;
    bp+=min;
    i=min;

    if (carry)
    {
        while (i < max)
        {
            i++;
            t1= *(ap++);
            
            *rp = (t1+1) & TM_BN_MASK2;
            
            if( *rp >= t1 )
            {
                rp++;
                carry=0;
                break;
            }
            rp++;
        }
        if ((i >= max) && carry)
        {
            *(rp++)=1;
            r->top++;
        }
    }
    if (rp != ap)
    {
        for (; i<max; i++)
            *(rp++)= *(ap++);
    }
    /* memcpy(rp,ap,sizeof(*ap)*(max-i));*/
    return(1);
}

/* unsigned subtraction of b from a, a must be larger than b. */
int tfBN_usub(
    ttBIGNUMBERPtr r, ttConstBIGNUMBERPtr a, ttConstBIGNUMBERPtr b)
{
    int max,min;
    register ttBN_ULONG t1,t2;
    register ttBN_ULONGPtr ap,bp,rp;
    ttBIGNUMBERPtr bn;
    int i,carry;
#if defined(IRIX_CC_BUG) && !defined(LINT)
    int dummy;
#endif

    if (a->top < b->top) /* hmm... should not be happening */
    {
        return(0);
    }

    max=a->top;
    min=b->top;
    
    bn = tm_bn_wexpand(r,max);
    if( bn == TM_OPENSSL_NULL ) return(0);

    ap=a->d;
    bp=b->d;
    rp=r->d;

    carry=0;
    for (i=0; i<min; i++)
    {
        t1= *(ap++);
        t2= *(bp++);
        if (carry)
        {
            carry=(t1 <= t2);
            t1=(t1-t2-1)&TM_BN_MASK2;
        }
        else
        {
            carry=(t1 < t2);
            t1=(t1-t2)&TM_BN_MASK2;
        }
#if defined(IRIX_CC_BUG) && !defined(LINT)
        dummy=t1;
#endif
        *(rp++)=t1&TM_BN_MASK2;
    }

    if (carry) /* subtracted */
    {
        while (i < max)
        {
            i++;
            t1= *(ap++);
            t2=(t1-1)&TM_BN_MASK2;
            *(rp++)=t2;
            if (t1 > t2) break;
        }
    }

    if (rp != ap)
    {
        for (;;)
        {
            if( i >= max ) break;
            i++;
            rp[0]=ap[0];
            if( i >= max ) break;
            i++;
            rp[1]=ap[1];
            if( i >= max ) break;
            i++;
            rp[2]=ap[2];
            if( i >= max ) break;
            i++;
            rp[3]=ap[3];
            rp+=4;
            ap+=4;
        }
    }

    r->top=max;
    tm_bn_fix_top(r);
    return(1);
}


int tfBN_sub(ttBIGNUMBERPtr r, ttConstBIGNUMBERPtr a, ttConstBIGNUMBERPtr b)
{
    int max,cmp;
    int add=0,neg=0;
    ttConstBIGNUMBERPtr tmp;
    ttBIGNUMBERPtr bn;

    /*  a -  b    a-b
     *  a - -b    a+b
     * -a -  b    -(a+b)
     * -a - -b    b-a
     */
    if (a->neg)
    {
        if (b->neg)
        {
            tmp=a; a=b; b=tmp; 
        }
        else
        {
            add=1; neg=1; 
        }
    }
    else
    { 
        if (b->neg) { add=1; neg=0; }
    }

    if (add)
    { 
        cmp = tfBN_uadd(r,a,b);
        if( !cmp ) return(0);
        r->neg=neg;
        return(1);
    }

    /* We are actually doing a - b :-) */

    max=(a->top > b->top)?a->top:b->top;
    
    bn = tm_bn_wexpand(r,max);
    if( bn == TM_OPENSSL_NULL ) return(0);
    cmp = tfBN_ucmp(a,b);
    if( cmp < 0)
    {
        cmp = tfBN_usub(r,b,a);
        if( !cmp ) return(0);
        r->neg=1;
    }
    else
    {
        cmp = tfBN_usub(r,a,b);
        if( !cmp ) return(0);
        r->neg=0;
    }
    return(1);
}

#ifdef TM_USE_PKI
/* tfBN_shift.c -----------------------------------------------------------*/

int tfBN_lshift1(ttBIGNUMBERPtr r, ttBIGNUMBERPtr a)
{
    register ttBN_ULONGPtr ap,rp;
    register ttBN_ULONG t,c;
    ttBIGNUMBERPtr bn;
    int i;

    if (r != a)
    {
        r->neg=a->neg;
        bn = tm_bn_wexpand(r,a->top+1);
        if( bn == TM_OPENSSL_NULL ) return(0);
        r->top=a->top;
    }
    else
    {
        bn = tm_bn_wexpand(r,a->top+1);
        if( bn == TM_OPENSSL_NULL) return(0);
    }
    ap=a->d;
    rp=r->d;
    c=0;
    for (i=0; i<a->top; i++)
    {
        t= *(ap++);
        *(rp++)=((t<<1)|c)&TM_BN_MASK2;
        c=(t & TM_BN_TBIT)?1:0;
    }
    if (c)
    {
        *rp=1;
        r->top++;
    }
    return(1);
}
#endif /* TM_USE_PKI */

int tfBN_lshift(ttBIGNUMBERPtr r, ttConstBIGNUMBERPtr a, int n)
{
    int i,nw,lb,rb,newTop;
    ttBN_ULONGPtr t,f;
    ttBN_ULONG l;
    ttBIGNUMBERPtr bn;

    r->neg=a->neg;
    nw=n/TM_BN_BITS2;
    lb=n%TM_BN_BITS2;
    rb=TM_BN_BITS2-lb;
    if (lb)
    {
        newTop = a->top + nw + 1;
    }
    else
    {
        newTop = a->top + nw;
    }
    bn = tm_bn_wexpand(r, newTop);
    if( bn == TM_OPENSSL_NULL )
    {
    	return(0);
    }
    f=a->d;
    t=r->d;
    if (lb)
    {
/* needed if lb is non zero, because we start ORing at newTop -1 . */
        t[newTop-1]=0;
    }
    if (lb == 0)
    {
        for (i=a->top-1; i>=0; i--)
            t[nw+i]=f[i];
    }
    else
    {
        for (i=a->top-1; i>=0; i--)
        {
            l=f[i];
            t[nw+i+1]|=(l>>rb)&TM_BN_MASK2;
            t[nw+i]=(l<<lb)&TM_BN_MASK2;
        }
    }
    tm_bzero(t,(unsigned int)nw*sizeof(t[0]));
    r->top=newTop;
    tm_bn_fix_top(r);
    return(1);
}

int tfBN_rshift(ttBIGNUMBERPtr r, ttBIGNUMBERPtr a, int n)
{
    int i,j,nw,lb,rb,errCode;
    ttBN_ULONGPtr t,f;
    ttBN_ULONG l,tmp;
    ttBIGNUMBERPtr bn;

    nw=n/TM_BN_BITS2;
    rb=n%TM_BN_BITS2;
    lb=TM_BN_BITS2-rb;
    if (nw > a->top || a->top == 0)
    {
        errCode = tm_bn_zero(r);
        if (!errCode) return 0;
        return(1);
    }
    j=a->top-nw;
    if (r != a)
    {
        r->neg=a->neg;
        bn = tm_bn_wexpand(r,j);
        if( bn == TM_OPENSSL_NULL ) return(0);
    }
    else
    {
        if (n == 0)
            return 1; /* or the copying loop will go berserk */
    }

    f= &(a->d[nw]);
    t=r->d;
    r->top=j;

    if (rb == 0)
    {
        for (i=j; i > 0; i--)
            *(t++)= *(f++);
    }
    else
    {
        l= *(f++);
        for (i=1; i<j; i++)
        {
            tmp =(l>>rb)&TM_BN_MASK2;
            l= *(f++);
            *(t++) =(tmp|(l<<lb))&TM_BN_MASK2;
        }
        *(t++) =(l>>rb)&TM_BN_MASK2;
    }
    tm_bn_fix_top(r);
    return(1);
}

/* bn_exp.c -----------------------------------------------*/

/* slow but works */
int tfBN_mod_mul(ttBIGNUMBERPtr ret, ttBIGNUMBERPtr a, ttBIGNUMBERPtr b, 
                      ttConstBIGNUMBERPtr m, ttBN_CTXPtr ctx)
{
    ttBIGNUMBERPtr t;
    int r=0,cmp;


    tfBN_CTX_start(ctx);
    t = tfBN_CTX_get(ctx);
    if( t == TM_OPENSSL_NULL ) goto err;
    if (a == b)
    {
        cmp = tfBN_sqr(t,a,ctx);
        if( !cmp ) goto err;
    }
    else
    {
        cmp = tfBN_mul(t,a,b,ctx);
        if( !cmp ) goto err;
    }
    r = tfBN_mod(ret,t,m,ctx);
err:
    tfBN_CTX_end(ctx);
    return(r);
}

int tfBN_mod_exp_mont(
    ttBIGNUMBERPtr rr, ttBIGNUMBERPtr a, ttConstBIGNUMBERPtr p,
    ttConstBIGNUMBERPtr m, ttBN_CTXPtr ctx, ttBN_MONT_CTXPtr in_mont)
{
    int i,j,bits,ret=0,wstart,wend,window,wvalue,errCode;
    int start=1,ts=0,cmp;
    ttBIGNUMBERPtr d,r;
    ttBIGNUMBERPtr aa;
    ttBIGNUMBER val[TABLE_SIZE];
    ttBN_MONT_CTXPtr mont=TM_OPENSSL_NULL;


    if (!(m->d[0] & 1))
    {

        return(0);
    }
    bits=tm_bn_num_bits(p);
    if (bits == 0)
    {
        errCode = tm_bn_one(rr);
        if (!errCode) return 0;
        return(1);
    }
    tfBN_CTX_start(ctx);
    d = tfBN_CTX_get(ctx);
    r = tfBN_CTX_get(ctx);
    if (d == TM_OPENSSL_NULL || r == TM_OPENSSL_NULL) goto err;

    /* If this is not done, things will break in the montgomery
     * part */

    if (in_mont != TM_OPENSSL_NULL)
        mont=in_mont;
    else
    {
        mont = tfBN_MONT_CTX_new();
        if( mont == TM_OPENSSL_NULL ) goto err;
        cmp = tfBN_MONT_CTX_set(mont,m,ctx);
        if( !cmp ) goto err;
    }

    tfBN_init(&val[0]);
    ts=1;
    cmp = tfBN_ucmp(a,m);
    if( cmp >= 0 )
    {
        cmp = tfBN_mod(&(val[0]),a,m,ctx);
        if( !cmp ) goto err;
        aa= &(val[0]);
    }
    else
        aa=a;
    
    cmp = tm_bn_to_montgomery(&(val[0]),aa,mont,ctx);
    if( !cmp )
        goto err; /* 1 */

    window = tm_bn_winbits_for_exponent_size(bits);
    if (window > 1)
    {
        cmp = tfbn_mod_mul_montgomery(d,&(val[0]),&(val[0]),mont,ctx);
        if( !cmp ) goto err; 
        /* 2 */
        j=1<<(window-1);
        for (i=1; i<j; i++)
        {
            tfBN_init(&(val[i]));
            cmp = tfbn_mod_mul_montgomery(&(val[i]),&(val[i-1]),d,mont,ctx);
            if( !cmp ) goto err;
        }
        ts=i;
    }

    start=1;    /* This is used to avoid multiplication etc
             * when there is only the value '1' in the
             * buffer. */
    wvalue=0;    /* The 'value' of the window */
    wstart=bits-1;    /* The top bit of the window */
    wend=0;        /* The bottom bit of the window */

    cmp = tm_bn_to_montgomery(r,tfBN_value_one(),mont,ctx);
    if( !cmp ) goto err;
    for (;;)
    {
        cmp = tfBN_is_bit_set(p,wstart);
        if( cmp == 0 )
        {
            if (!start)
            {
                cmp = tfbn_mod_mul_montgomery(r,r,r,mont,ctx);
                if( !cmp ) goto err;
            }
            if (wstart == 0) break;
            wstart--;
            continue;
        }
        /* We now have wstart on a 'set' bit, we now need to work out
         * how bit a window to do.  To do this we need to scan
         * forward until the last set bit before the end of the
         * window */
        j=wstart;
        wvalue=1;
        wend=0;
        for (i=1; i<window; i++)
        {
            if (wstart-i < 0) break;
            cmp = tfBN_is_bit_set(p,wstart-i);
            if( cmp )
            {
                wvalue<<=(i-wend);
                wvalue|=1;
                wend=i;
            }
        }

        /* wend is the size of the current window */
        j=wend+1;
        /* add the 'bytes above' */
        if (!start)
            for (i=0; i<j; i++)
            {
                cmp = tfbn_mod_mul_montgomery(r,r,r,mont,ctx);
                if( !cmp )
                    goto err;
            }
        
        /* wvalue will be an odd number < 2^window */
        cmp = tfbn_mod_mul_montgomery(r,r,&(val[wvalue>>1]),mont,ctx);
        if( !cmp )
            goto err;

        /* move the 'window' down further */
        wstart-=wend+1;
        wvalue=0;
        start=0;
        if (wstart < 0) break;
    }
    ret = tfBN_from_montgomery(rr,r,mont,ctx);
err:
    if ((in_mont == TM_OPENSSL_NULL)
        && (mont != TM_OPENSSL_NULL))
    {
        tfBN_MONT_CTX_free(mont);
    }
    tfBN_CTX_end(ctx);
    for (i=0; i<ts; i++)
    {
        tfBN_clear_free(&(val[i]));
    }
    return(ret);
}

int tfBN_mod_exp_mont_word(
    ttBIGNUMBERPtr rr, ttBN_ULONG a, ttConstBIGNUMBERPtr p,
    ttConstBIGNUMBERPtr m, ttBN_CTXPtr ctx, ttBN_MONT_CTXPtr in_mont)
{
    ttBN_MONT_CTXPtr mont = TM_OPENSSL_NULL;
    int b, bits, ret=0, cmp, errCode;
    int r_is_one;
    ttBN_ULONG w, next_w;
    ttBIGNUMBERPtr d, r, t;
    ttBIGNUMBERPtr swap_tmp;
#define tfBN_MOD_MUL_WORD(r, w, m) \
        ( tfBN_mul_word(r, (w)) && \
        (/* tfBN_ucmp(r, (m)) < 0 ? 1 :*/  \
          ( tfBN_mod(t, r, m, ctx) && (swap_tmp = r, r = t, t = swap_tmp, 1))))
        /* tfBN_MOD_MUL_WORD is only used with 'w' large,
          * so the tfBN_ucmp test is probably more overhead
          * than always using tfBN_mod (which uses tfBN_copy if
          * a similar test returns true). */
#define tfBN_TO_MONTGOMERY_WORD(r, w, mont) \
        (tm_bn_set_word(r, (w)) && tm_bn_to_montgomery(r, r, (mont), ctx))

    if (!(m->d[0] & 1))
    {

        return(0);
    }
    bits = tm_bn_num_bits(p);
    if (bits == 0)
    {
        errCode = tm_bn_one(rr);
        if (!errCode) return 0;
        return(1);
    }
    tfBN_CTX_start(ctx);
    d = tfBN_CTX_get(ctx);
    r = tfBN_CTX_get(ctx);
    t = tfBN_CTX_get(ctx);
    if (d == TM_OPENSSL_NULL || r == TM_OPENSSL_NULL
        || t == TM_OPENSSL_NULL) goto err;

    if (in_mont != TM_OPENSSL_NULL)
        mont=in_mont;
    else
    {
        mont = tfBN_MONT_CTX_new();
        if( mont == TM_OPENSSL_NULL ) goto err;
        cmp = tfBN_MONT_CTX_set(mont, m, ctx);
        if (!cmp ) goto err;
    }

    r_is_one = 1; /* except for Montgomery factor */

    /* bits-1 >= 0 */

    /* The result is accumulated in the product r*w. */
    w = a; /* bit 'bits-1' of 'p' is always set */
    for (b = bits-2; b >= 0; b--)
    {
        /* First, square r*w. */
        next_w = w*w;
        if ((next_w/w) != w) /* overflow */
        {
            if (r_is_one)
            {
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the reference processing */
/* PRQA S 3416 L1 */
                if( !tfBN_TO_MONTGOMERY_WORD(r, w, mont) ) goto err;
/* PRQA L:L1 */
                r_is_one = 0;
            }
            else
            {
/* PRQA: QAC Message 3416: It is a code intended by initialization and */
/* PRQA: QAC Message 3416: the reference processing of the variable */
/* PRQA S 3416 L1 */
                if (! tfBN_MOD_MUL_WORD(r, w, m)) goto err;
/* PRQA L:L1 */
            }
            next_w = 1;
        }
        w = next_w;
        if (!r_is_one)
        {
            cmp = tfbn_mod_mul_montgomery(r, r, r, mont, ctx);
            if( !cmp ) goto err;
        }

        /* Second, multiply r*w by 'a' if exponent bit is set. */
        cmp = tfBN_is_bit_set(p, b);
        if( cmp )
        {
            next_w = w*a;
            if ((next_w/a) != w) /* overflow */
            {
                if (r_is_one)
                {
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the reference processing */
/* PRQA S 3416 L1 */
                    if( !tfBN_TO_MONTGOMERY_WORD(r, w, mont) ) goto err;
/* PRQA L:L1 */
                    r_is_one = 0;
                }
                else
                {
/* PRQA: QAC Message 3416: It is a code intended by initialization and  */
/* PRQA: QAC Message 3416: the reference processing of the variable  */
/* PRQA S 3416 L1 */
                    if( !tfBN_MOD_MUL_WORD(r, w, m) ) goto err;
/* PRQA L:L1 */
                }
                next_w = a;
            }
            w = next_w;
        }
    }

    /* Finally, set r:=r*w. */
    if (w != 1)
    {
        if (r_is_one)
        {
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the reference processing */
/* PRQA S 3416 L1 */
            if( !tfBN_TO_MONTGOMERY_WORD(r, w, mont) ) goto err;
/* PRQA L:L1 */
            r_is_one = 0;
        }
        else
        {
/* PRQA: QAC Message 3416: It is a code intended by initialization and  */
/* PRQA: QAC Message 3416: the reference processing of the variable  */
/* PRQA S 3416 L1 */
            if( !tfBN_MOD_MUL_WORD(r, w, m) ) goto err;
/* PRQA L:L1 */
        }
    }

    if (r_is_one) /* can happen only if a == 1*/
    {
        cmp = tm_bn_one(rr);
        if( !cmp ) goto err;
    }
    else
    {
        cmp = tfBN_from_montgomery(rr, r, mont, ctx);
        if( !cmp ) goto err;
    }
    ret = 1;
err:
    if ((in_mont == TM_OPENSSL_NULL)
        && (mont != TM_OPENSSL_NULL)) tfBN_MONT_CTX_free(mont);
    tfBN_CTX_end(ctx);
    return(ret);
}

/**bn_mont.c --------------------------------------------------------*/
#define MONT_WORD /* use the faster word-based algorithm */

/*
 * tfbn_mod_mul_montgomery
 *
 * Returns:
 *  0: error
 *  1: success
 */
static int tfbn_mod_mul_montgomery(ttBIGNUMBERPtr   r,
                                   ttBIGNUMBERPtr   a,
                                   ttBIGNUMBERPtr   b,
                                   ttBN_MONT_CTXPtr mont,
                                   ttBN_CTXPtr      ctx)
{
    ttBIGNUMBERPtr  tmpBnPtr;
    int             retVal;

    retVal = 0;

    tfBN_CTX_start(ctx);
    tmpBnPtr = tfBN_CTX_get(ctx);
    if (tmpBnPtr != TM_OPENSSL_NULL)
    {
        if (a == b)
        {
            retVal = tfBN_sqr(tmpBnPtr, a, ctx);
        }
        else
        {
            retVal = tfBN_mul(tmpBnPtr, a, b, ctx);
        }
        if (retVal != 0)
        {
/* reduce from aRR to aR */
            retVal = tfBN_from_montgomery(r, tmpBnPtr, mont, ctx);
        }
    }

    tfBN_CTX_end(ctx);
    return retVal;
}

static int tfBN_from_montgomery(
    ttBIGNUMBERPtr ret, ttBIGNUMBERPtr a, ttBN_MONT_CTXPtr mont,
    ttBN_CTXPtr ctx)
{
    int retn=0;

#ifdef MONT_WORD
    ttBIGNUMBERPtr n,r, bn;
    ttBN_ULONG n0,v;
    ttBN_ULONGPtr ap,np,rp,nrp;
    int al,nl,max,i,x,ri,cmp;

    tfBN_CTX_start(ctx);
    r = tfBN_CTX_get(ctx);
    if( r == TM_OPENSSL_NULL ) goto err;
    bn = tfBN_copy(r,a);
    if( bn == TM_OPENSSL_NULL ) goto err;
    n= &(mont->N);

    ap=a->d;
    /* mont->ri is the size of mont->N in bits (rounded up
       to the word size) */
    al=ri=mont->ri/TM_BN_BITS2;
    
    nl=n->top;
    if ((al == 0) || (nl == 0)) { r->top=0; return(1); }

    max=(nl+al+1); /* allow for overflow (no?) XXX */
    bn = tm_bn_wexpand(r,max);
    if( bn == TM_OPENSSL_NULL ) goto err;
    bn = tm_bn_wexpand(ret,max);
    if( bn == TM_OPENSSL_NULL) goto err;

    r->neg=a->neg^n->neg;
    np=n->d;
    rp=r->d;
    nrp= &(r->d[nl]);

    /* clear the top words of T */
    for (i=r->top; i<max; i++) /* tm_bzero? XXX */
        r->d[i]=0;

    r->top=max;
    n0=mont->n0;

    for (i=0; i<nl; i++)
    {

        v= tfbn_mul_add_words(rp,np,nl,(rp[0]*n0)&TM_BN_MASK2);
        nrp++;
        rp++;
        if (((nrp[-1]+=v)&TM_BN_MASK2) >= v)
            continue;
        else
        {
            nrp[0]++;
            if (((nrp[0])&TM_BN_MASK2) != 0) continue;
            nrp[1]++;
            if (((nrp[1])&TM_BN_MASK2) != 0) continue;
            for (x=2; (((++nrp[x])&TM_BN_MASK2) == 0); x++) ;
        }
    }
    tm_bn_fix_top(r);
    
    /* mont->ri will be a multiple of the word size */
    ret->neg = r->neg;
    x=ri;
    rp=ret->d;
    ap= &(r->d[x]);
    if (r->top < x)
        al=0;
    else
        al=r->top-x;
    ret->top=al;
    al-=4;
    for (i=0; i<al; i+=4)
    {
        ttBN_ULONG t1,t2,t3,t4;
        
        t1=ap[i+0];
        t2=ap[i+1];
        t3=ap[i+2];
        t4=ap[i+3];
        rp[i+0]=t1;
        rp[i+1]=t2;
        rp[i+2]=t3;
        rp[i+3]=t4;
    }
    al+=4;
    for (; i<al; i++)
        rp[i]=ap[i];
#else /* !MONT_WORD */ 
/*
    ttBIGNUMBERPtr t1,t2;

    tfBN_CTX_start(ctx);
    t1 = tfBN_CTX_get(ctx);
    t2 = tfBN_CTX_get(ctx);
    if (t1 == TM_OPENSSL_NULL || t2 == TM_OPENSSL_NULL) goto err;
    
    if (! tfBN_copy(t1,a)) goto err;
    tfBN_mask_bits(t1,mont->ri);

    if (! tfBN_mul(t2,t1,&mont->Ni,ctx)) goto err;
    tfBN_mask_bits(t2,mont->ri);

    if (! tfBN_mul(t1,t2,&mont->N,ctx)) goto err;
    if (! tfBN_add(t2,a,t1)) goto err;
    if (! tfBN_rshift(ret,t2,mont->ri)) goto err;
*/
#endif /* MONT_WORD */

    cmp = tfBN_ucmp(ret, &(mont->N));
    if( cmp >= 0 )
    {
        tfBN_usub(ret,ret,&(mont->N));
    }
    retn=1;
 err:
    tfBN_CTX_end(ctx);
    return(retn);
}

ttBN_MONT_CTXPtr  tfBN_MONT_CTX_new(void)
{
    ttBN_MONT_CTXPtr ret;
    
    ret=(ttBN_MONT_CTXPtr )tm_get_raw_buffer(sizeof(ttBN_MONT_CTX));
    if (ret == TM_OPENSSL_NULL)
    {
        return(TM_OPENSSL_NULL);
    }

    tfBN_MONT_CTX_init(ret);
    ret->flags=TM_BN_FLG_MALLOCED;
    return(ret);
}

static void tfBN_MONT_CTX_init(ttBN_MONT_CTXPtr ctx)
{
    ctx->ri=0;
    tfBN_init(&(ctx->RR));
    tfBN_init(&(ctx->N));
    tfBN_init(&(ctx->Ni));
    ctx->flags=0;
}

void tfBN_MONT_CTX_free(ttBN_MONT_CTXPtr mont)
{
    if(mont == TM_OPENSSL_NULL)
        return;

    tfBN_free(&(mont->RR));
    tfBN_free(&(mont->N));
    tfBN_free(&(mont->Ni));
    if (mont->flags & TM_BN_FLG_MALLOCED)
        tm_free_raw_buffer((tt8BitPtr)mont);
}

int tfBN_MONT_CTX_set(ttBN_MONT_CTXPtr mont, ttConstBIGNUMBERPtr mod, 
                           ttBN_CTXPtr ctx)
{
    ttBIGNUMBER     Ri;
    ttBIGNUMBERPtr  R,bn;
    int             retVal,cmp,errCode;

    retVal = 0;
    tfBN_init(&Ri);
    R= &(mont->RR);                    /* grab RR as a temp */
    bn = tfBN_copy(&(mont->N),mod);
    if (!bn) goto err; /* Set N */

#ifdef MONT_WORD
    {
        ttBIGNUMBER tmod;
        ttBN_ULONG buf[2];

        mont->ri=(tm_bn_num_bits(mod)+(TM_BN_BITS2-1))/TM_BN_BITS2*TM_BN_BITS2;
        errCode = tm_bn_zero(R);
        if (!errCode) goto err;
        cmp = tfBN_set_bit(R,TM_BN_BITS2);
        if (!cmp) goto err;            /* R */

        buf[0]=mod->d[0]; /* tmod = N mod word size */
        buf[1]=0;
        tmod.d=buf;
        tmod.top=1;
        tmod.dmax=2;
        tmod.neg=mod->neg;
                            /* Ri = R^-1 mod N*/
        bn = tfBN_mod_inverse(&Ri,R,&tmod,ctx);
        if( bn == TM_OPENSSL_NULL )
        {
            goto err;
        }
        cmp = tfBN_lshift(&Ri,&Ri,TM_BN_BITS2);        /* R*Ri */
        if (!cmp)
        {
            goto err;
        }
        if (!tm_bn_is_zero(&Ri))
        {
            cmp = tfBN_sub_word(&Ri,1);
            if (!cmp)
            {
                goto err;
            }
        }
        else /* if N mod word size == 1 */
        {
            errCode = tm_bn_set_word(&Ri,TM_BN_MASK2);
            if (!errCode)  /* Ri-- (mod word size) */
            {
                goto err;
            }
        }
        cmp = tfBN_div(&Ri,TM_OPENSSL_NULL,&Ri,&tmod,ctx);
        if (!cmp)    /* Ni = (R*Ri-1)/N,
                                         * keep only least significant word: */
        {
            goto err;
        }
        mont->n0=Ri.d[0];
    }
#else /* !MONT_WORD */
    { /* ttBIGNUMBER version */
        mont->ri=tm_bn_num_bits(mod);
        errCode = tm_bn_zero(R);
        if (!errCode) goto err;
/* R = 2^ri */
        cmp = tfBN_set_bit(R,mont->ri);
        if (!cmp) goto err;
/* Ri = R^-1 mod N*/
        bn = tfBN_mod_inverse(&Ri,R,mod,ctx);
        if( bn == TM_OPENSSL_NULL )
            goto err;
        if (!tfBN_lshift(&Ri,&Ri,mont->ri)) goto err; /* R*Ri */
        if (!tfBN_sub_word(&Ri,1)) goto err;
                            /* Ni = (R*Ri-1) / N */
        cmp = tfBN_div(&(mont->Ni),TM_OPENSSL_NULL,&Ri,mod,ctx);
        if (!cmp) goto err;
    }
#endif

    /* setup RR for conversions */
    errCode = tm_bn_zero(&(mont->RR));
    if (!errCode) goto err;
    cmp = tfBN_set_bit(&(mont->RR),mont->ri*2);
    if (!cmp) goto err;
    cmp = tfBN_mod(&(mont->RR),&(mont->RR),&(mont->N),ctx);
    if (!cmp) goto err;

    retVal = 1;
err:
    tfBN_free(&Ri);
    return retVal;
}


#else /* ! (TM_PUBKEY_USE_DIFFIEHELLMAN || TM_PUBKEY_USE_DSA || RSA */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when none of the macroes is not defined */
int tvBignum2Dummy = 0;

#endif /* TM_PUBKEY_USE_DIFFIEHELLMAN || TM_PUBKEY_USE_DSA || RSA */
