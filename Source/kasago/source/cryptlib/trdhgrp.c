/*
 *
 * Description: Diffie-Hellman module
 *
 * Filename: trdhgrp.c
 * Author: Jin Zhang
 * Date Created: 4/18/2002
 * $Source: source/cryptlib/trdhgrp.c $
 *
 * Modification History
 * $Revision: 6.0.2.3 $
 * $Date: 2012/11/27 12:25:47JST $
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
#ifdef TM_ERROR_CHECKING
#include <trglobal.h>
#endif /* TM_ERROR_CHECKING */
#include <trcrylib.h>
#ifdef TM_PUBKEY_USE_DIFFIEHELLMAN

#include <tropnssl.h>


#ifdef TM_USE_CLIB
#include <stdlib.h> /*strtol*/
#endif /*TM_USE_CLIB */

#ifdef TM_USE_MAPPED_CRYLIB
#include <trcrymap.h>
#endif /* TM_USE_MAPPED_CRYLIB */

static ttDhInBnPtr tfDH_new(void);
static void       tfDH_free(ttDhInBnPtr dh);
static int        tfDH_generate_key(ttDhInBnPtr dh);
static int        tfDH_compute_key(
    tt8BitPtr key,ttBIGNUMBERPtr pub_key, ttDhInBnPtr dh);

int        tfDH_bn_mod_exp(
    ttBIGNUMBERPtr r, ttBIGNUMBERPtr a, ttConstBIGNUMBERPtr p,
    ttConstBIGNUMBERPtr m, ttBN_CTXPtr ctx, ttBN_MONT_CTXPtr m_ctx);





/**dh_lib.c ---------------------------------------------*/

static void    tfDH_free(ttDhInBnPtr dh)
{
    ttBN_MONT_CTXPtr mont;

    mont=(ttBN_MONT_CTXPtr)dh->method_mont_p;

    if(dh->p)
    {
        tfBN_free(dh->p);
    }

    if(dh->g)
    {
        tfBN_free(dh->g);
    }

    if(dh->pub_key)
    {
        tfBN_free(dh->pub_key);
    }

    if(dh->priv_key)
    {
        tfBN_free(dh->priv_key);
    }

    if(mont)
    {
        tfBN_MONT_CTX_free(mont);
    }

    tm_free_raw_buffer((tt8BitPtr)dh);
}


static ttDhInBnPtr tfDH_new(void)
{
    
    ttDhInBnPtr ret;
    ret=(ttDhInBnPtr)tm_get_raw_buffer(sizeof(ttDhInBn));

    if (ret == TM_OPENSSL_NULL)
    {
        return(TM_OPENSSL_NULL);
    }

    ret->p=TM_OPENSSL_NULL;
    ret->g=TM_OPENSSL_NULL;
    ret->length=0;
    ret->pub_key=TM_OPENSSL_NULL;
    ret->priv_key=TM_OPENSSL_NULL;
    ret->method_mont_p=TM_OPENSSL_NULL;
    ret->flags = TM_DH_FLAG_CACHE_MONT_P;
    return(ret);
}









static int tfDH_generate_key(ttDhInBnPtr dh)
{
    int ok=0;
    int generate_new_key=0;
    tt32Bit l;
    ttBN_CTX ctx;
    ttBN_MONT_CTXPtr mont;
    ttBIGNUMBERPtr pub_key=TM_OPENSSL_NULL,priv_key=TM_OPENSSL_NULL;

    tfBN_CTX_init(&ctx);

    if (dh->priv_key == TM_OPENSSL_NULL)
    {
        priv_key= tfBN_new();
        if (priv_key == TM_OPENSSL_NULL) goto err;
        generate_new_key=1;
    }
    else
        priv_key=dh->priv_key;

    if (dh->pub_key == TM_OPENSSL_NULL)
    {
        pub_key= tfBN_new();
        if (pub_key == TM_OPENSSL_NULL) goto err;
    }
    else
        pub_key=dh->pub_key;

    if ((dh->method_mont_p == TM_OPENSSL_NULL)
        && (dh->flags & TM_DH_FLAG_CACHE_MONT_P))
    {
        dh->method_mont_p = (ttCharPtr)tfBN_MONT_CTX_new( );
        if( dh->method_mont_p != TM_OPENSSL_NULL )
        {
            ok = tfBN_MONT_CTX_set( (ttBN_MONT_CTXPtr)dh->method_mont_p,
                                    dh->p,
                                    &ctx );
            if( !ok ) goto err;
        }
    }
    mont=(ttBN_MONT_CTXPtr)dh->method_mont_p;

     if (generate_new_key)
     {
         l = dh->length ? 
                    (tt32Bit)(dh->length) : (tt32Bit)tm_bn_num_bits(dh->p)-1;
         /* secret exponent length */
         
         ok = tfBN_rand( priv_key, (int)l, 0, 0 );
        if( !ok ) goto err;
     }

#ifdef TM_USE_MAPPED_CRYLIB
    ok = tfDhMapBnModExpMont(dh, pub_key, dh->g,
                priv_key,dh->p,&ctx,mont);
#else /* TM_USE_MAPPED_CRYLIB */
    ok = tfDH_bn_mod_exp( pub_key, dh->g, priv_key, dh->p, &ctx, mont );
#endif /* TM_USE_MAPPED_CRYLIB */
    if (!ok )
        goto err;
        
    dh->pub_key=pub_key;
    dh->priv_key=priv_key;
    ok=1;
err:

    if(   (pub_key != TM_OPENSSL_NULL)
       && (dh->pub_key == TM_OPENSSL_NULL) )
    {
        tfBN_free(pub_key);
    }
    if(   (priv_key != TM_OPENSSL_NULL)
       && (dh->priv_key == TM_OPENSSL_NULL))
    {
        tfBN_free(priv_key);
    }
    tfBN_CTX_free(&ctx);
    return(ok);
}




static int tfDH_compute_key(tt8BitPtr key, ttBIGNUMBERPtr pub_key, 
                          ttDhInBnPtr dh)
{
    ttBN_CTX ctx;
    ttBN_MONT_CTXPtr mont;
    ttBIGNUMBERPtr tmp;
    int ret= -1;

    tfBN_CTX_init(&ctx);
    tfBN_CTX_start(&ctx);
    tmp = tfBN_CTX_get(&ctx);
    
    if (dh->priv_key == TM_OPENSSL_NULL)
    {

        goto err;
    }
    if ((dh->method_mont_p == TM_OPENSSL_NULL)
        && (dh->flags & TM_DH_FLAG_CACHE_MONT_P))
    {
        dh->method_mont_p = (ttCharPtr)tfBN_MONT_CTX_new( );
        if( dh->method_mont_p != TM_OPENSSL_NULL )
        {
            ret = tfBN_MONT_CTX_set( (ttBN_MONT_CTXPtr)dh->method_mont_p,
                                     dh->p,
                                     &ctx );
            if( !ret )
            {
                ret = -1;
                goto err;
            }
        }
    }

    mont=(ttBN_MONT_CTXPtr)dh->method_mont_p;
#ifdef TM_USE_MAPPED_CRYLIB
    ret = tfDhMapBnModExpMont(dh, tmp, pub_key, dh->priv_key,dh->p,&ctx,mont));
#else /* TM_USE_MAPPED_CRYLIB */
    ret = tfDH_bn_mod_exp( tmp, pub_key, dh->priv_key, dh->p, &ctx, mont );
#endif /* TM_USE_MAPPED_CRYLIB */
    if( !ret )
    {
        ret = -1;
        goto err;
    }

    ret= tfBN_bn2bin(tmp,key);
err:
    tfBN_CTX_end(&ctx);
    tfBN_CTX_free(&ctx);
    return(ret);
}



int tfDH_bn_mod_exp(
    ttBIGNUMBERPtr r, ttBIGNUMBERPtr a, ttConstBIGNUMBERPtr p,
    ttConstBIGNUMBERPtr m, ttBN_CTXPtr ctx, ttBN_MONT_CTXPtr m_ctx)
{
    int returnCode;
    if (a->top == 1)
    {
        ttBN_ULONG A = a->d[0];
        returnCode = tfBN_mod_exp_mont_word(r,A,p,m,ctx,m_ctx);
    }
    else
    {
        returnCode = tfBN_mod_exp_mont(r,a,p,m,ctx,m_ctx);
    }
    return returnCode;
}





/*----------------- public api------------------------------------*/



void tfCvMalloc(ttCharVectorPtrPtr cvPtrPtr, tt32Bit length)
{
    (*cvPtrPtr) = (ttCharVectorPtr)tm_get_raw_buffer(sizeof(ttCharVector));
    if(!*cvPtrPtr)
    {
        return;
    }
    else
    {
        (*cvPtrPtr)->cvValuePtr =
            (ttCharPtr) tm_get_raw_buffer((ttPktLen)length);
        if(!(*cvPtrPtr)->cvValuePtr)
        {
            tm_free_raw_buffer((tt8BitPtr)*cvPtrPtr);
            return;
        }
        (*cvPtrPtr)->cvLength = (unsigned int)length;
    }
}


void tfCvFree(ttCharVectorPtr vptr)
{
    if(!vptr)
        return;
    if(vptr->cvValuePtr)
    {
        tm_free_raw_buffer((tt8BitPtr)vptr->cvValuePtr);
    }
    tm_free_raw_buffer((tt8BitPtr)vptr);
}



/* generate my private secret and the corresponding public value,
 * using the DH generator and prime.
 */
int tfDhGenerate(ttCharVectorPtr    prime, 
                 tt32Bit            g, 
                 tt32Bit            privLength, 
                 ttCharVectorPtrPtr pub, 
                 ttCharVectorPtrPtr priv)
{
    ttBIGNUMBERPtr p = TM_OPENSSL_NULL;
    ttDhInBnPtr dh = TM_OPENSSL_NULL;
    int error = -1;
    int ret;

    /* initialize */
    /* pre-process to generate number */
    ret = tfeay_v2bn( &p, prime );
    if( ret < 0 )
        goto end;
    dh = tfDH_new();
    if( dh == TM_OPENSSL_NULL )
        goto end;
    dh->p = p;
    p     = TM_OPENSSL_NULL;    /* p is now part of dh structure */
    dh->g = tfBN_new();
    if( dh->g == TM_OPENSSL_NULL )
        goto end;
    ret = tm_bn_set_word( dh->g, (ttBN_ULONG)g );
    if( !ret )
        goto end;

    if (privLength != 0)
        dh->length = (int)privLength;

    /* generate public and private number */
    ret = tfDH_generate_key(dh);
    if( !ret )
        goto end;

    /* copy results to buffers */
    ret = tfeay_bn2v( pub, dh->pub_key );
    if( ret < 0 )
        goto end;
    ret = tfeay_bn2v( priv, dh->priv_key );
    if( ret < 0 )
    {
        tfCvFree(*pub);
        goto end;
    }

    error = 0;

end:
    if (dh != TM_OPENSSL_NULL)
        tfDH_free(dh);
    if (p != 0)
        tfBN_free(p);
    return(error);
}





int tfDhComputeSharedSecret(ttCharVectorPtr prime,
                            ttCharVectorPtr priv, 
                            ttCharVectorPtr pub2, 
                            ttCharVectorPtr keyPtr)
{
    ttBIGNUMBERPtr dh_pub = TM_OPENSSL_NULL;
    ttDhInBnPtr dh = TM_OPENSSL_NULL;
    int l;
    tt8BitPtr v = TM_OPENSSL_NULL;
    int error = -1;
    int ret;

    ret = tfeay_v2bn( &dh_pub, pub2 );
    if( ret < 0 )
        goto end;

    /* make ttDhInBn structure */
    dh = tfDH_new();
    if( dh == TM_OPENSSL_NULL )
        goto end;
    ret = tfeay_v2bn(&dh->p, prime);
    if( ret < 0 )
        goto end;
/* this value is not used by DH computation. - Jin
    ret = tfeay_v2bn( &dh->pub_key, pub );
    if( ret < 0 )
        goto end;
*/
    ret = tfeay_v2bn( &dh->priv_key, priv );
    if( ret < 0 )
        goto end;
    dh->length = (int)pub2->cvLength * 8;

    v = (tt8BitPtr)tm_get_raw_buffer(prime->cvLength*sizeof(tt8Bit));
    if (v == TM_OPENSSL_NULL)
    {
        goto end;
    }
    else
    {
        tm_bzero(v, prime->cvLength*sizeof(tt8Bit));
    }

    l = tfDH_compute_key(v, dh_pub, dh);
    if (l == -1)
    {
        goto end;
    }
    keyPtr->cvLength = (unsigned int)l;
    keyPtr->cvValuePtr = (ttCharPtr)v;

    error = 0;

end:
    if (dh_pub != TM_OPENSSL_NULL)
        tfBN_free(dh_pub);
    if (dh != TM_OPENSSL_NULL)
        tfDH_free(dh);
    if ((error != 0) && (v != (tt8BitPtr)0))
        tm_free_raw_buffer(v);
    return(error);
}


#else /* ! TM_PUBKEY_USE_DIFFIEHELLMAN */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_PUBKEY_USE_DIFFIEHELLMAN is not defined */
int tlDhgrpDummy = 0;

#endif /* TM_PUBKEY_USE_DIFFIEHELLMAN */
