/*
 * Description: ttDSA (signature and sign)
 *
 * Filename: trdsa.c
 * Author: Jin Shang
 * Date Created: 5/20/2002
 * $Source: source/cryptlib/trdsa.c $
 *
 * Modification History
 * $Revision: 6.0.2.3 $
 * $Date: 2011/01/15 02:39:45JST $
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
#include <trproto.h>
#include <trglobal.h>

#include <trcrylib.h>
#include <tropnssl.h>

#ifdef TM_PUBKEY_USE_DSA
#include <trpki.h>

#ifdef TM_USE_MAPPED_CRYLIB
#include <trcrymap.h>
#endif /* TM_USE_MAPPED_CRYLIB */
#ifdef    __cplusplus
extern "C" {
#endif


#define HASH(shaPtr, from , length, to) \
    { \
        shaPtr->aalgInitFuncPtr((ttVoidPtr)&context);\
        shaPtr->aalgUpdateFuncPtr((ttVoidPtr)&context, from,length,0);\
        shaPtr->aalgFinalFuncPtr(to, (ttVoidPtr)&context); \
    }
            
#define TM_SHA_DIGEST_LENGTH                   20
#define TM_DSA_FLAG_CACHE_MONT_P               0x01

struct tsDsaMethod {
    ttConstCharPtr name;
    ttDsaSigPtr (TM_CODE_FAR *dsa_do_sign)(
        ttConst8BitPtr dgst, int dlen, ttDSAPtr dsa);
    int         (TM_CODE_FAR *dsa_sign_setup)(
        ttDSAPtr dsa, ttBN_CTXPtr ctx_in, ttBIGNUMBERPtrPtr kinvp,
        ttBIGNUMBERPtrPtr rp);
    int         (TM_CODE_FAR *dsa_do_verify)(
        ttConst8BitPtr dgst, int dgst_len, ttDsaSigPtr sig, ttDSAPtr dsa);
    int         (TM_CODE_FAR *dsa_mod_exp)(
        ttDSAPtr dsa, ttBIGNUMBERPtr rr, ttBIGNUMBERPtr a1, 
        ttBIGNUMBERPtr p1, ttBIGNUMBERPtr a2, ttBIGNUMBERPtr p2,
        ttBIGNUMBERPtr m, ttBN_CTXPtr ctx,
        ttBN_MONT_CTXPtr in_mont);
    int         (TM_CODE_FAR *bn_mod_exp)(
        ttDSAPtr dsa, ttBIGNUMBERPtr r, ttBIGNUMBERPtr a, 
        ttConstBIGNUMBERPtr p, ttConstBIGNUMBERPtr m, 
        ttBN_CTXPtr ctx, ttBN_MONT_CTXPtr m_ctx); 
    int         (TM_CODE_FAR *init)(ttDSAPtr dsa);
    int         (TM_CODE_FAR *finish)(ttDSAPtr dsa);
    int         flags;
    ttCharPtr   app_data;
};

/* inner api definition */

ttDsaSigPtr     tfDsaSigNew    (void);
void            tfDsaSigFree    (ttDsaSigPtr a);



/* dsa_ossl.c */
ttDsaSigPtr dsa_do_sign(ttConst8BitPtr dgst, int dlen, ttDSAPtr dsa);
int      dsa_sign_setup(
    ttDSAPtr dsa, ttBN_CTXPtr ctx_in, ttBIGNUMBERPtrPtr kinvp,
    ttBIGNUMBERPtrPtr rp);
int      dsa_do_verify(ttConst8BitPtr dgst, int dgst_len, 
                              ttDsaSigPtr sig, ttDSAPtr dsa);
int      dsa_init(ttDSAPtr dsa);
int      dsa_finish(ttDSAPtr dsa);
int      dsa_mod_exp(
    ttDSAPtr dsa, ttBIGNUMBERPtr rr, ttBIGNUMBERPtr a1, 
    ttBIGNUMBERPtr p1, ttBIGNUMBERPtr a2, 
    ttBIGNUMBERPtr p2, ttBIGNUMBERPtr m, ttBN_CTXPtr ctx,
    ttBN_MONT_CTXPtr in_mont);
int      dsa_bn_mod_exp(
    ttDSAPtr dsa, ttBIGNUMBERPtr r, ttBIGNUMBERPtr a, 
    ttConstBIGNUMBERPtr p,  ttConstBIGNUMBERPtr m, 
    ttBN_CTXPtr ctx, ttBN_MONT_CTXPtr m_ctx);


static int tfX509DsaSigGet( ttVoidPtr keyPtr, tt8BitPtr sigPtr);
static int tfX509DsaSigSet( ttVoidPtr keyPtr, tt8BitPtr sigPtr);

#ifdef TM_USE_MAPPED_CRYLIB
static const ttDsaMethod openssl_dsa_meth = {
                "Mapped ttDSA",
                tfDsaMapSign,           /* tm_dsa_map_sign */
                tfDsaMapSignSetup,      /* tm_dsa_map_sign_setup */
                tfDsaMapVerify,         /* tm_dsa_map_verify */
                tfDsaMapModExp,         /* tm_dsa_map_mod_exp */
                tfDsaMapBnModExpMont,   /* tm_dsa_map_bn_mod_exp_mont */
                tfDsaMapInit,           /* tm_dsa_map_init */
                tfDsaMapFinish,         /* tm_dsa_map_finish */
                0,
                TM_OPENSSL_NULL         };
#else /* TM_USE_MAPPED_CRYLIB */
static const ttDsaMethod openssl_dsa_meth = {
                "OpenSSL ttDSA method",
                dsa_do_sign,
                dsa_sign_setup,
                dsa_do_verify,
                dsa_mod_exp,
                dsa_bn_mod_exp,
                dsa_init,
                dsa_finish,
                0,
                TM_OPENSSL_NULL         };
#endif /* TM_USE_MAPPED_CRYLIB */



#ifdef    __cplusplus
}
#endif 




int tfDSASign ( int dgst_len, tt8BitPtr dgst, tt8BitPtr sigbuf, 
                ttDSAPtr  dsa, ttIntPtr siglen )
{
    ttDsaSigPtr s;

#ifdef TM_USE_MAPPED_CRYLIB
    s=tfDsaMapSign(dgst,dgst_len,dsa);
#else /* TM_USE_MAPPED_CRYLIB */
    s=dsa_do_sign(dgst,dgst_len,dsa);
#endif /* TM_USE_MAPPED_CRYLIB */
    if (s == TM_OPENSSL_NULL)
    {
        *siglen=0;
        return(0);
    }
/* Use ASN1 to store signature -- sequence of r,s */
    *siglen = tfX509DsaSigSet((ttVoidPtr)s, sigbuf);
    tfDsaSigFree(s);
    return(0);
}

/* data has already been hashed (probably with SHA or SHA-1). */
int tfDSAVerify (
    int dgst_len, tt8BitPtr dgst, tt8BitPtr sigbuf, ttDSAPtr dsa,int siglen)
{
    ttDsaSigPtr s;
	int       ret=-1;
    int         tempInt;

    TM_UNREF_IN_ARG(siglen);
    
    s = tfDsaSigNew();
    if (s == TM_OPENSSL_NULL) 
        return(ret);

    tempInt = tfX509DsaSigGet((ttVoidPtr)s,sigbuf);
    if (tempInt != TM_ENOERROR)
    {
        return -1;
    };
#ifdef TM_USE_MAPPED_CRYLIB
    ret=tfDsaMapVerify(dgst,dgst_len,s,dsa);
#else /* TM_USE_MAPPED_CRYLIB */
    ret=dsa_do_verify(dgst,dgst_len,s,dsa);
#endif /* TM_USE_MAPPED_CRYLIB */
    tfDsaSigFree(s);

    if (ret == 1) 
    {
        ret = TM_ENOERROR;
    }
    else
    {
        ret = -1;
    }
    return ret;
}


    
ttDSAPtr tfDsaNew( void )
{
    ttDsaMethodPtr meth;
    ttDSAPtr ret;
    int tempInt;

    ret=(ttDSAPtr )tm_get_raw_buffer(sizeof(ttDSA));
    if (ret == TM_OPENSSL_NULL)
    {
        return(TM_OPENSSL_NULL);
    }

    
    ret->meth = &openssl_dsa_meth;
    meth = ret->meth;
/* right now , don't use following elements */
/*
    ret->pad=0;
    ret->version=0;
    ret->references=1;
    ret->write_params=1;
 */

    ret->p=TM_OPENSSL_NULL;
    ret->q=TM_OPENSSL_NULL;
    ret->g=TM_OPENSSL_NULL;

    ret->pub_key=TM_OPENSSL_NULL;
    ret->priv_key=TM_OPENSSL_NULL;

    ret->kinv=TM_OPENSSL_NULL;
    ret->r=TM_OPENSSL_NULL;
    ret->method_mont_p=TM_OPENSSL_NULL;

    ret->flags=meth->flags;

    if ((meth->init != TM_OPENSSL_NULL))
    {
        tempInt = meth->init(ret);
        if (!tempInt)
        {
            tm_free_raw_buffer((tt8BitPtr)ret);
            ret=TM_OPENSSL_NULL;
        }
    }
    
    return(ret);
}

void tfDsaFree(ttDSAPtr r)
{
    ttDsaMethodPtr meth;

    if (r == TM_OPENSSL_NULL) return;
    meth = r->meth; 
    if(meth->finish) meth->finish(r);
    
    if (r->p != TM_OPENSSL_NULL) tfBN_clear_free(r->p);
    if (r->q != TM_OPENSSL_NULL) tfBN_clear_free(r->q);
    if (r->g != TM_OPENSSL_NULL) tfBN_clear_free(r->g);
    if (r->pub_key != TM_OPENSSL_NULL) tfBN_clear_free(r->pub_key);
    if (r->priv_key != TM_OPENSSL_NULL) tfBN_clear_free(r->priv_key);
    if (r->kinv != TM_OPENSSL_NULL) tfBN_clear_free(r->kinv);
    if (r->r != TM_OPENSSL_NULL) tfBN_clear_free(r->r);
    tm_free_raw_buffer((tt8BitPtr)r);
}


static int tfX509DsaSigSet( ttVoidPtr keyPtr, tt8BitPtr sigPtr)
{
    ttBIGNUMBERPtr  num[2];
    ttAsn1Integer   aInt;
    int             tlen;
    int             clen;
    
    int             i;
    int             size;
    int             retLen;
    
    if ( keyPtr == (ttVoidPtr)0)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfX509DsaSigSet", "null pointer!\n");
#endif /* TM_ERROR_CHECKING */
        retLen = -1;
        goto COMMONRETURN;
    }

    num[0]  = ((ttDsaSigPtr)keyPtr)->r;
    num[1]  = ((ttDsaSigPtr)keyPtr)->s;
    
    size    = 0;
    clen    = 0;
    aInt.asn1Data = (ttVoidPtr)0;
 
    for (i=0; i<2; i++)
    {
        tlen        =   tm_bn_num_bits(num[i]);
        tlen        =   (tlen == 0)?0:tlen/8+1;
        aInt.asn1Type   = TM_ASN1_TAG_INTEGER;
        if ( num[i]->neg)
        {
            aInt.asn1Type   = TM_ASN1_TAG_NEG_INTEGER;
        }
        aInt.asn1Length =   tlen;
        if ( tlen > size)
        {
            size = tlen;
        }
        clen += tm_asn1_integer_enc (&aInt, (ttVoidPtr)0,i);
    }
    
    aInt.asn1Type = TM_ASN1_TAG_INTEGER;
    aInt.asn1Data = tm_get_raw_buffer((ttPktLen)size+4);
    if (aInt.asn1Data == (ttVoidPtr)0)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfX509RsaPubKeyEnc", "out of memory!\n");
#endif /* TM_ERROR_CHECKING */
        retLen = -1;
        goto COMMONRETURN;
    }
    tlen = tm_asn1_sequence_enc ( (ttVoidPtr)0, &sigPtr, clen);
    for (i=0; i<2; i++)
    {
        aInt.asn1Length   = tfBN_bn2bin(num[i],aInt.asn1Data);
        tm_asn1_integer_enc ( &aInt, &sigPtr, i);
    }
    tm_free_raw_buffer(aInt.asn1Data);
    retLen = tlen + clen;

COMMONRETURN:
    return  (retLen);
}

static int tfX509DsaSigGet( ttVoidPtr keyPtr, tt8BitPtr sigPtr)
{
    ttDsaSigPtr         dsaSigPtr;
    tt8BitPtr           tmpPtr;
        
    int                 totalLen;
    int                 errorCode;
    
    ttAsn1IntegerPtr    intPtr;

    errorCode = TM_ENOERROR;
    if ((sigPtr == (ttVoidPtr)0) || (keyPtr == (ttVoidPtr)0))
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfX509DsaSigGet", "null pointer!\n");
#endif /* TM_ERROR_CHECKING */
        errorCode = TM_EINVAL;
        goto dsaSigGetExit;
    }
    
    tmpPtr  = sigPtr;
    totalLen = tm_asn1_sequence_dec((ttVoidPtr)0,    &tmpPtr);
    

    dsaSigPtr = (ttDsaSigPtr )keyPtr;
    intPtr  = (ttVoidPtr)0;
    
    tm_asn1_integer_dec ( &intPtr, &tmpPtr);
    dsaSigPtr->r  = tfBN_bin2bn(
        intPtr->asn1Data, intPtr->asn1Length, dsaSigPtr->r);
    if (dsaSigPtr->r == (ttVoidPtr)0) 
    {
        errorCode = TM_ENOBUFS;
        goto dsaSigGetExit;
    }
    
    tm_asn1_integer_dec ( &intPtr, &tmpPtr);
    dsaSigPtr->s  = tfBN_bin2bn(
        intPtr->asn1Data, intPtr->asn1Length, dsaSigPtr->s);
    if (dsaSigPtr->s == (ttVoidPtr)0) 
    {
        errorCode = TM_ENOBUFS;
        goto dsaSigGetExit;
    }

    tfAsn1StringFree(intPtr);

    if (totalLen != (int)(tmpPtr - sigPtr))
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfX509DsaSigDecode", "length error!\n");
#endif /* TM_ERROR_CHECKING */
        errorCode = TM_EINVAL;
        goto dsaSigGetExit;
    }
dsaSigGetExit:
    return errorCode; 
}


ttVoidPtr tfDsaKeyAsn1ToBn( ttGeneralDsaPtr dsaPtr)
{
    ttDSAPtr    keyPtr;
    ttVoidPtr   retPtr;

    retPtr = (ttVoidPtr)0;

    if ( dsaPtr == (ttGeneralDsaPtr)0)
    {
        goto dsaKeyAsn1ToBn;
    }
    keyPtr = tfDsaNew();
    
    if ( dsaPtr->dsaP != (ttAsn1IntegerPtr)0)
    {
        keyPtr->p  = tfBN_bin2bn( dsaPtr->dsaP->asn1Data, 
                                dsaPtr->dsaP->asn1Length, keyPtr->p);
    }
    if ( dsaPtr->dsaQ != (ttAsn1IntegerPtr)0)
    {
        keyPtr->q  = tfBN_bin2bn( dsaPtr->dsaQ->asn1Data, 
                                dsaPtr->dsaQ->asn1Length, keyPtr->q);
    }
    if ( dsaPtr->dsaG != (ttAsn1IntegerPtr)0)
    {
        keyPtr->g  = tfBN_bin2bn( dsaPtr->dsaG->asn1Data, 
                                dsaPtr->dsaG->asn1Length, keyPtr->g);
    }
    if ( dsaPtr->dsaPubKey != (ttAsn1IntegerPtr)0)
    {
        keyPtr->pub_key  = tfBN_bin2bn( 
                        dsaPtr->dsaPubKey->asn1Data, 
                        dsaPtr->dsaPubKey->asn1Length, keyPtr->pub_key);
    }
    if ( dsaPtr->dsaPriKey != (ttAsn1IntegerPtr)0)
    {
        keyPtr->priv_key  = tfBN_bin2bn( 
                        dsaPtr->dsaPriKey->asn1Data, 
                        dsaPtr->dsaPriKey->asn1Length, keyPtr->priv_key);
    }
    
    retPtr = (ttVoidPtr)keyPtr;

dsaKeyAsn1ToBn:
    return retPtr;
}




/* tfDsaSize just gave an estimate value */
int tfDsaSize (ttDSAPtr d)
{
	return(20+2*tm_bn_num_bytes(d->q));
}
ttDsaSigPtr tfDsaSigNew(void)
{
    ttDsaSigPtr ret;

    ret = (ttDsaSigPtr )tm_get_raw_buffer(sizeof(ttDsaSig));
    if (ret == TM_OPENSSL_NULL)
    {
        return(TM_OPENSSL_NULL);
    }
    ret->r = TM_OPENSSL_NULL;
    ret->s = TM_OPENSSL_NULL;
    return(ret);
}

void tfDsaSigFree(ttDsaSigPtr r)
{
    if (r == TM_OPENSSL_NULL) return;
    if (r->r) tfBN_clear_free(r->r);
    if (r->s) tfBN_clear_free(r->s);
    tm_free_raw_buffer((tt8BitPtr)r);
}



ttDsaSigPtr dsa_do_sign(ttConst8BitPtr dgst, int dlen, ttDSAPtr dsa)
{
    ttBIGNUMBERPtr kinv=TM_OPENSSL_NULL,r=TM_OPENSSL_NULL,s=TM_OPENSSL_NULL;
    ttBIGNUMBER m;
    ttBIGNUMBER xr;
    ttBN_CTXPtr ctx=TM_OPENSSL_NULL;
    int i; 
    int tempInt;
    ttDsaSigPtr ret=TM_OPENSSL_NULL;
    ttBIGNUMBERPtr bigNumPtr;

    tfBN_init(&m);
    tfBN_init(&xr);
    if (!dsa->p || !dsa->q || !dsa->g)
    {
/*        reason= -1 ; */
        goto err;
    }
    s= tfBN_new();
    if (s == TM_OPENSSL_NULL) goto err;

    i=tm_bn_num_bytes(dsa->q); /* should be 20 */
    if ((dlen > i) || (dlen > 50))
    {
/*        reason= -1 ;*/
        goto err;
    }

    ctx= tfBN_CTX_new();
    if (ctx == TM_OPENSSL_NULL) goto err;

    if ((dsa->kinv == TM_OPENSSL_NULL) || (dsa->r == TM_OPENSSL_NULL))
    {
#ifdef TM_USE_MAPPED_CRYLIB
tempInt = tfDsaMapSignSetup(dsa,ctx,&kinv,&r);
if (!tempInt) goto err;
#else /* TM_USE_MAPPED_CRYLIB */
tempInt = dsa_sign_setup(dsa,ctx,&kinv,&r);
if (!tempInt) goto err;
#endif /* TM_USE_MAPPED_CRYLIB */
    }
    else
    {
        kinv=dsa->kinv;
        dsa->kinv=TM_OPENSSL_NULL;
        r=dsa->r;
        dsa->r=TM_OPENSSL_NULL;
    }

    bigNumPtr = tfBN_bin2bn(dgst,dlen,&m);
    if ( bigNumPtr == TM_OPENSSL_NULL) goto err;

    /* Compute  s = inv(k) (m + xr) mod q */
    tempInt = tfBN_mod_mul(&xr,dsa->priv_key,r,dsa->q,ctx);
    if (!tempInt) goto err;/* s = xr */
    tempInt = tfBN_add(s, &xr, &m);
    if (!tempInt) goto err;        /* s = m + xr */
    tempInt = tfBN_cmp(s,dsa->q);
    if (tempInt > 0)
        tfBN_sub(s,s,dsa->q);
    tempInt = tfBN_mod_mul(s,s,kinv,dsa->q,ctx);
    if (!tempInt) goto err;

    ret=tfDsaSigNew();
    if (ret == TM_OPENSSL_NULL) goto err;
    ret->r = r;
    ret->s = s;
    
err:
    if (!ret)
    {
        tfBN_free(r);
        tfBN_free(s);
    }
    if (ctx != TM_OPENSSL_NULL) tfBN_CTX_free(ctx);
    tfBN_clear_free(&m);
    tfBN_clear_free(&xr);
/* dsa->kinv is TM_OPENSSL_NULL now if we used it */
    if (kinv != TM_OPENSSL_NULL)
        tfBN_clear_free(kinv);
    return(ret);
}

int dsa_sign_setup(
    ttDSAPtr dsa, ttBN_CTXPtr ctx_in, ttBIGNUMBERPtrPtr kinvp,
    ttBIGNUMBERPtrPtr rp)
{
    ttBN_CTXPtr ctx;
    ttBIGNUMBER k;
    ttBIGNUMBERPtr kinv=TM_OPENSSL_NULL,r=TM_OPENSSL_NULL;
    int tempInt;
    int ret=0;

    tfBN_init(&k);
    if (!dsa->p || !dsa->q || !dsa->g)
    {
        return 0;
    }
    if (ctx_in == TM_OPENSSL_NULL)
    {
        ctx= tfBN_CTX_new();
        if (ctx == TM_OPENSSL_NULL) goto err;
    }
    else
        ctx=ctx_in;

    r= tfBN_new();
    if (r == TM_OPENSSL_NULL) goto err;
    kinv=TM_OPENSSL_NULL;

    /* Get random k */
    do
    {
        tempInt = tfBN_rand_range(&k, dsa->q);
        if (!tempInt) goto err;
    }
    while (tm_bn_is_zero(&k));

    if ((dsa->method_mont_p == TM_OPENSSL_NULL)
        && (dsa->flags & TM_DSA_FLAG_CACHE_MONT_P))
    {
        dsa->method_mont_p=(ttCharPtr) tfBN_MONT_CTX_new();
        if (dsa->method_mont_p != TM_OPENSSL_NULL)
        {
            tempInt = tfBN_MONT_CTX_set((ttBN_MONT_CTXPtr )dsa->method_mont_p,
                                        dsa->p,ctx);
            if (!tempInt ) goto err;
        }
    }

    /* Compute r = (g^k mod p) mod q */
    tempInt = dsa_bn_mod_exp(dsa, r,dsa->g,&k,dsa->p,ctx,
                             (ttBN_MONT_CTXPtr )dsa->method_mont_p);
    if (!tempInt) goto err;

    tempInt = tfBN_mod(r,r,dsa->q,ctx);
    if (!tempInt ) goto err;

    /* Compute  part of 's = inv(k) (m + xr) mod q' */
    kinv= tfBN_mod_inverse(TM_OPENSSL_NULL,&k,dsa->q,ctx);
    if (kinv == TM_OPENSSL_NULL) goto err;

    if (*kinvp != TM_OPENSSL_NULL) tfBN_clear_free(*kinvp);
    *kinvp=kinv;
    kinv=TM_OPENSSL_NULL;
    if (*rp != TM_OPENSSL_NULL) tfBN_clear_free(*rp);
    *rp=r;
    ret=1;
err:
    if (!ret)
    {
        if (kinv != TM_OPENSSL_NULL) tfBN_clear_free(kinv);
        if (r != TM_OPENSSL_NULL) tfBN_clear_free(r);
    }
    if (ctx_in == TM_OPENSSL_NULL) tfBN_CTX_free(ctx);
    if (kinv != TM_OPENSSL_NULL) tfBN_clear_free(kinv);
    tfBN_clear_free(&k);
    return(ret);
}

int dsa_do_verify(
    ttConst8BitPtr dgst, int dgst_len, ttDsaSigPtr sig, ttDSAPtr dsa)
{
    ttBN_CTXPtr ctx;
    ttBIGNUMBER u1,u2,t1;
    ttBN_MONT_CTXPtr mont;
    int ret;
    int tempInt;
    ttBIGNUMBERPtr tmpPtr;
    ttBIGNUMBERPtr bigNumPtr;

    ret = -1;
    mont = TM_OPENSSL_NULL;
    ctx = TM_OPENSSL_NULL;
    tfBN_init(&u1);
    tfBN_init(&u2);
    tfBN_init(&t1);

    ctx= tfBN_CTX_new();
    if (ctx == TM_OPENSSL_NULL)
    {
        goto err;
    }
    tempInt = tfBN_ucmp(sig->r, dsa->q);
    if (tm_bn_is_zero(sig->r) || sig->r->neg || tempInt >= 0)
    {
        ret = 0;
        goto err;
    }
    tempInt = tfBN_ucmp(sig->s, dsa->q);
    if (tm_bn_is_zero(sig->s) || sig->s->neg || tempInt >= 0)
    {
        ret = 0;
        goto err;
    }

    /* Calculate W = inv(S) mod Q
     * save W in u2 */
    tmpPtr = tfBN_mod_inverse(&u2,sig->s,dsa->q,ctx);
    if (tmpPtr == TM_OPENSSL_NULL)
    {
        goto err;
    }

    /* save M in u1 */
    bigNumPtr = tfBN_bin2bn(dgst,dgst_len,&u1);
    if ( bigNumPtr == TM_OPENSSL_NULL)
    {
        goto err;
    }

    /* u1 = M * w mod q */
    tempInt = tfBN_mod_mul(&u1,&u1,&u2,dsa->q,ctx);
    if (!tempInt)
    {
        goto err;
    }

    /* u2 = r * w mod q */
    tempInt = tfBN_mod_mul(&u2,sig->r,&u2,dsa->q,ctx);
    if (!tempInt)
    {
        goto err;
    }

    if ((dsa->method_mont_p == TM_OPENSSL_NULL)
        && (dsa->flags & TM_DSA_FLAG_CACHE_MONT_P))
    {
        dsa->method_mont_p =(ttCharPtr)tfBN_MONT_CTX_new();
        if (dsa->method_mont_p == TM_OPENSSL_NULL)
        {
            goto err;
        }
        tempInt = tfBN_MONT_CTX_set((ttBN_MONT_CTXPtr )dsa->method_mont_p,
                                    dsa->p,ctx);
        if (!tempInt)
        {
            goto err;
        }
    }
    mont = (ttBN_MONT_CTXPtr )dsa->method_mont_p;

    {
#ifdef TM_USE_MAPPED_CRYLIB
    if (!tfDsaMapModExp(dsa, &t1,dsa->g,&u1,dsa->pub_key,&u2,
                        dsa->p,ctx,mont))
    {
        goto err;
    }
#else /* TM_USE_MAPPED_CRYLIB */
    tempInt = dsa_mod_exp(dsa, &t1,dsa->g,&u1,dsa->pub_key,&u2,
                        dsa->p,ctx,mont);
    if (!tempInt)
    {
        goto err;
    }
#endif /* TM_USE_MAPPED_CRYLIB */
    /* tfBN_copy(&u1,&t1); */
    /* let u1 = u1 mod q */
    tempInt = tfBN_mod(&u1,&t1,dsa->q,ctx);
    if (!tempInt )
    {
        goto err;
    }
    }
    /* V is now in u1.  If the signature is correct, it will be
     * equal to R. */
    ret = tfBN_ucmp(&u1, sig->r);
    if (ret == 0)
    {
        ret = 1;
    }
    else
    {
        ret = -1;
    }
err:
    if (ctx != TM_OPENSSL_NULL)
    {
        tfBN_CTX_free(ctx);
    }
    tfBN_free(&u1);
    tfBN_free(&u2);
    tfBN_free(&t1);
    return (ret);
}

int dsa_init(ttDSAPtr dsa)
{
    dsa->flags|=TM_DSA_FLAG_CACHE_MONT_P;
    return(1);
}

int dsa_finish(ttDSAPtr dsa)
{
    if(dsa->method_mont_p)
        tfBN_MONT_CTX_free((ttBN_MONT_CTXPtr )dsa->method_mont_p);
    return(1);
}

int dsa_mod_exp(
    ttDSAPtr dsa, ttBIGNUMBERPtr rr, ttBIGNUMBERPtr a1, ttBIGNUMBERPtr p1,
    ttBIGNUMBERPtr a2, ttBIGNUMBERPtr p2, ttBIGNUMBERPtr m, ttBN_CTXPtr ctx,
    ttBN_MONT_CTXPtr in_mont)
{
	TM_UNREF_IN_ARG(dsa);
    return tfBN_mod_exp2_mont(rr, a1, p1, a2, p2, m, ctx, in_mont);
}
    
int dsa_bn_mod_exp(
    ttDSAPtr dsa, ttBIGNUMBERPtr r, ttBIGNUMBERPtr a, ttConstBIGNUMBERPtr p,
    ttConstBIGNUMBERPtr m, ttBN_CTXPtr ctx,
    ttBN_MONT_CTXPtr m_ctx)
{
#ifdef TM_USE_MAPPED_CRYLIB
    return tfDsaMapBnModExpMont(dsa, r, a, p, m, ctx, m_ctx);
#else /* TM_USE_MAPPED_CRYLIB */
    TM_UNREF_IN_ARG(dsa);
    return tfBN_mod_exp_mont(r, a, p, m, ctx, m_ctx);
#endif /* TM_USE_MAPPED_CRYLIB */
}

#else /* ! TM_PUBKEY_USE_DSA */
/* To allow link for builds when TM_USE_PKI is not defined */
int tvDsaDummy = 0;

#endif /* TM_PUBKEY_USE_DSA */
