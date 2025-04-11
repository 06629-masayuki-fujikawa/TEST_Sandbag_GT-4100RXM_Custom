/*
 * Description: ttRSA (signature and sign)
 *
 * Filename: trrsa.c
 * Author: Jin Shang
 * Date Created: 5/10/2002
 * $Source: source/cryptlib/trrsa.c $
 *
 * Modification History
 * $Revision: 6.0.2.9 $
 * $Date: 2012/12/12 23:25:15JST $
 * $Author: pcarney $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
* All rights reserved.
*
* This package is an SSL implementation written
* by Eric Young (eay@cryptsoft.com).
* The implementation was written so as to conform with Netscapes SSL.
* 
* This library is free for commercial and non-commercial use as long as
* the following conditions are aheared to.    The following conditions
* apply to all code found in this distribution, be it the RC4, ttRSA,
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
*     notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
* 3. All advertising materials mentioning features or use of this software
*     must display the following acknowledgement:
*     "This product includes cryptographic software written by
*      Eric Young (eay@cryptsoft.com)"
*     The word 'cryptographic' can be left out if the rouines from the library
*     being used are not cryptographic related :-).
* 4. If you include any Windows specific code (or a derivative thereof) from 
*     the apps directory (application code) you must include an acknowledgement:
*     "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
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

#ifdef TM_PUBKEY_USE_RSA
#include <trcrylib.h>
#include <tropnssl.h>

#include <trpki.h>    
#ifdef TM_USE_MAPPED_CRYLIB
#include <trcrymap.h>
#endif /* TM_USE_MAPPED_CRYLIB */

#ifdef    __cplusplus
extern "C" {
#endif
  

/* following structure is copied from openssl */

#define TM_RSA_FLAG_CACHE_PUBLIC           0x02
#define TM_RSA_FLAG_CACHE_PRIVATE          0x04
#define TM_RSA_FLAG_BLINDING               0x08
#define TM_RSA_FLAG_THREAD_SAFE            0x10
/* This flag means the private key operations will be handled by rsa_mod_exp
 * and that they do not depend on the private key components being present:
 * for example a key stored in external hardware. Without this flag bn_mod_exp
 * gets called when private key components are absent.
 */
#define TM_RSA_FLAG_EXT_PKEY               0x20
    
/* This flag in the ttRsaMethod enables the new rsa_sign, rsa_verify functions.
 */
#define TM_RSA_FLAG_SIGN_VER               0x40
    
#define TM_RSA_PKCS1_PADDING               1
#define TM_RSA_SSLV23_PADDING              2
#define TM_RSA_NO_PADDING                  3
#define TM_RSA_PKCS1_OAEP_PADDING          4

    


struct tsRsaMethSt
{
    ttConstCharPtr name;
    int (TM_CODE_FAR *rsa_pub_enc)(
        int flen, tt8BitPtr from, tt8BitPtr to, ttRSAPtr rsa, int padding );
    int (TM_CODE_FAR *rsa_pub_dec)(
        int flen, tt8BitPtr from, tt8BitPtr to, ttRSAPtr rsa, int padding);
    int (TM_CODE_FAR *rsa_priv_enc)(
        int flen, tt8BitPtr from, tt8BitPtr to, ttRSAPtr rsa, int padding);
    int (TM_CODE_FAR *rsa_priv_dec)(
        int flen, tt8BitPtr from, tt8BitPtr to, ttRSAPtr rsa, int padding);
/* Can be null */
    int (TM_CODE_FAR *rsa_mod_exp)(
        ttBIGNUMBERPtr r0, ttBIGNUMBERPtr I, ttRSAPtr rsa); 
    
    int (TM_CODE_FAR *bn_mod_exp)(
        ttBIGNUMBERPtr r, ttBIGNUMBERPtr a, ttConstBIGNUMBERPtr p,
        ttConstBIGNUMBERPtr m, ttBN_CTXPtr ctx,
/* Can be null */
        ttBN_MONT_CTXPtr m_ctx); 
/* called at new */
    int (TM_CODE_FAR *init)     (ttRSAPtr rsa);        
/* called at free */
    int (TM_CODE_FAR *finish)   (ttRSAPtr rsa);
/* RSA_METHOD_FLAG_* things */
    int flags;            
/* may be needed! */
    ttCharPtr app_data;
/* New sign and verify functions: some libraries don't allow arbitrary data
 * to be signed/verified: this allows them to be used. Note: for this to work
 * the RSA_public_decrypt() and RSA_private_encrypt() should *NOT* be used
 * RSA_sign(), RSA_verify() should be used instead. Note: for backwards
 * compatibility this functionality is only enabled if the TM_RSA_FLAG_SIGN_VER
 * option is set in 'flags'.
 */
    int (TM_CODE_FAR *rsa_sign) (   int type, 
                        tt8BitPtr m, 
                        tt32Bit m_len,
                        tt8BitPtr sigret, 
                        tt32BitPtr siglen, 
                        ttRSAPtr rsa);
    int (TM_CODE_FAR *rsa_verify)(  int dtype, 
                        tt8BitPtr m, 
                        tt32Bit m_len,
                        tt8BitPtr sigbuf, 
                        tt32Bit siglen, 
                        ttRSAPtr rsa);
};
    
    
ttRSAPtr    tfRsaNewMethod (   ttRsaMethodPtr method );

 
    
static int  RSA_blinding_on(ttRSAPtr rsa, ttBN_CTXPtr ctx);
    
static int  RSA_padding_add_PKCS1_type_1(   tt8BitPtr to,
                                            int tlen,
                                            tt8BitPtr f,
                                            int fl  );
static int  RSA_padding_check_PKCS1_type_1( tt8BitPtr to,
                                            int tlen,
                                            tt8BitPtr f,
                                            int fl,
                                            int rsa_len );
static int  RSA_padding_add_PKCS1_type_2(   tt8BitPtr to,
                                            int tlen,
                                            tt8BitPtr f,
                                            int fl  );
static int  RSA_padding_check_PKCS1_type_2( tt8BitPtr to,
                                            int tlen,
                                            tt8BitPtr f,
                                            int fl,
                                            int rsa_len );

static int  RSA_padding_add_SSLv23(         tt8BitPtr to,
                                            int tlen,
                                            tt8BitPtr f,
                                            int fl  );
static int  RSA_padding_check_SSLv23(       tt8BitPtr to,
                                            int tlen,
                                            tt8BitPtr f,
                                            int fl,
                                            int rsa_len );
static int  RSA_padding_add_none(           tt8BitPtr to,
                                            int tlen,
                                            tt8BitPtr f,
                                            int fl  );
static int  RSA_padding_check_none(         tt8BitPtr to,
                                            int tlen,
                                            tt8BitPtr f,
                                            int fl,
                                            int rsa_len );
int  RSA_eay_public_encrypt(         int flen, 
                                            tt8BitPtr from,
                                            tt8BitPtr to, 
                                            ttRSAPtr rsa,
                                            int padding );
int  RSA_eay_private_encrypt(        int flen, 
                                            tt8BitPtr from,
                                            tt8BitPtr to, 
                                            ttRSAPtr rsa,
                                            int padding );
int  RSA_eay_public_decrypt(         int flen, 
                                            tt8BitPtr from,
                                            tt8BitPtr to, 
                                            ttRSAPtr rsa,
                                            int padding );
int  RSA_eay_private_decrypt(        int flen, 
                                            tt8BitPtr from,
                                            tt8BitPtr to, 
                                            ttRSAPtr rsa,
                                            int padding );

int  RSA_eay_mod_exp (
    ttBIGNUMBERPtr r0, ttBIGNUMBERPtr i, ttRSAPtr rsa);
int  RSA_eay_init    (ttRSAPtr rsa);
int  RSA_eay_finish  (ttRSAPtr rsa);


static int RSA_padding_add_PKCS1_OAEP( tt8BitPtr to, 
                                int tlen,
	                            tt8BitPtr from, 
                                int flen,
	                            tt8BitPtr param, 
                                int plen);

static int RSA_padding_check_PKCS1_OAEP(tt8BitPtr to, 
                                 int tlen,
	                             tt8BitPtr from, 
                                 int flen, 
                                 int num, 
                                 tt8BitPtr param,
                                 int plen);


#ifdef TM_USE_MAPPED_CRYLIB
static const ttRsaMethod TM_CONST_QLF rsa_pkcs1_map_meth={
        "Mapped ttRSA",
        tfRsaMapPublicEncrypt,      /* tm_rsa_map_public_encrypt */
        tfRsaMapPublicDecrypt,      /* tm_rsa_map_public_decrypt */
        tfRsaMapPrivateEncrypt,     /* tm_rsa_map_private_encrypt */
        tfRsaMapPrivateDecrypt,     /* tm_rsa_map_private_decrypt */
        tfRsaMapModExp,             /* tm_rsa_map_mod_exp */
        tfRsaMapBnModExpMont,       /* tm_rsa_map_bn_mod_exp_mont */
        tfRsaMapInit,               /* tm_rsa_map_init */
        tfRsaMapFinish,             /* tm_rsa_map_finish */
        0,
        TM_OPENSSL_NULL,
    };
#else /* TM_USE_MAPPED_CRYLIB */

static const ttRsaMethod TM_CONST_QLF rsa_pkcs1_eay_meth={
        "Eric Young's PKCS#1 ttRSA",
        RSA_eay_public_encrypt,
        RSA_eay_public_decrypt, /* signature verification */
        RSA_eay_private_encrypt, /* signing */
        RSA_eay_private_decrypt,
        RSA_eay_mod_exp,
        tfBN_mod_exp_mont,
        RSA_eay_init,
        RSA_eay_finish,
        0,
        TM_OPENSSL_NULL,
        0,
        0
    };
#endif /* TM_USE_MAPPED_CRYLIB */
#ifdef    __cplusplus
}
#endif



int tfRSAPublicEncrypt(int flen, tt8BitPtr from, tt8BitPtr to,
	     ttVoidPtr rsa, ttIntPtr siglen)
{
#ifdef TM_USE_MAPPED_CRYLIB
    *siglen = tfRsaMapPublicEncrypt(
        flen, from, to, (ttRSAPtr)rsa, TM_RSA_PKCS1_PADDING);
#else /* TM_USE_MAPPED_CRYLIB */
    *siglen = RSA_eay_public_encrypt(
        flen, from, to, (ttRSAPtr)rsa, TM_RSA_PKCS1_PADDING);
#endif /* TM_USE_MAPPED_CRYLIB */
    return TM_ENOERROR;
}

int tfRSAPublicDecrypt(int flen, tt8BitPtr from, tt8BitPtr to,
	     ttVoidPtr rsa, ttIntPtr siglen)
{   
#ifdef TM_USE_MAPPED_CRYLIB
    *siglen = tfRsaMapPublicDecrypt(
        flen, from, to, (ttRSAPtr)rsa, TM_RSA_PKCS1_PADDING);
#else /* TM_USE_MAPPED_CRYLIB */
    *siglen = RSA_eay_public_decrypt(
        flen, from, to, (ttRSAPtr)rsa, TM_RSA_PKCS1_PADDING);
#endif /* TM_USE_MAPPED_CRYLIB */

    return TM_ENOERROR;
}

int tfRSAPrivateEncrypt(int flen, tt8BitPtr from, tt8BitPtr to,
	     ttVoidPtr rsa, ttIntPtr siglen)
{
#ifdef TM_USE_MAPPED_CRYLIB
    *siglen = tfRsaMapPrivateEncrypt(
        flen, from, to, (ttRSAPtr)rsa, TM_RSA_PKCS1_PADDING);
#else /* TM_USE_MAPPED_CRYLIB */
    *siglen = RSA_eay_private_encrypt(
        flen, from, to, (ttRSAPtr)rsa, TM_RSA_PKCS1_PADDING);
#endif /* TM_USE_MAPPED_CRYLIB */
    return TM_ENOERROR;
}

int tfRSAPrivateDecrypt(int flen, tt8BitPtr from, tt8BitPtr to,
	     ttVoidPtr rsa, ttIntPtr siglen)
{
#ifdef TM_USE_MAPPED_CRYLIB
    *siglen = tfRsaMapPrivateDecrypt(
        flen, from, to, (ttRSAPtr)rsa, TM_RSA_PKCS1_PADDING);
#else /* TM_USE_MAPPED_CRYLIB */
    *siglen = RSA_eay_private_decrypt(
        flen, from, to, (ttRSAPtr)rsa, TM_RSA_PKCS1_PADDING);
#endif /* TM_USE_MAPPED_CRYLIB */
    return TM_ENOERROR;
}

ttVoidPtr tfRsaKeyAsn1ToBn( ttGeneralRsaPtr rsaPtr)
{
    ttRSAPtr    keyPtr;
    ttVoidPtr   retPtr;

    retPtr = (ttVoidPtr)0;

    if ( rsaPtr == (ttGeneralRsaPtr)0)
    {
        goto rsaKeyAsn1ToBn;
    }
    keyPtr = tfRsaNew();
    
    if ( rsaPtr->rsaN != (ttAsn1IntegerPtr)0)
    {
        keyPtr->n  = tfBN_bin2bn( rsaPtr->rsaN->asn1Data, 
                            rsaPtr->rsaN->asn1Length, keyPtr->n);
    }
    if ( rsaPtr->rsaE != (ttAsn1IntegerPtr)0)
    {
        keyPtr->e  = tfBN_bin2bn( rsaPtr->rsaE->asn1Data, 
                            rsaPtr->rsaE->asn1Length, keyPtr->e);
    }
    if ( rsaPtr->rsaD != (ttAsn1IntegerPtr)0)
    {
        keyPtr->d  = tfBN_bin2bn( rsaPtr->rsaD->asn1Data, 
                            rsaPtr->rsaD->asn1Length, keyPtr->d);
    }
    if ( rsaPtr->rsaP != (ttAsn1IntegerPtr)0)
    {
        keyPtr->p  = tfBN_bin2bn( rsaPtr->rsaP->asn1Data, 
                            rsaPtr->rsaP->asn1Length, keyPtr->p);
    }
    if ( rsaPtr->rsaQ != (ttAsn1IntegerPtr)0)
    {
        keyPtr->q  = tfBN_bin2bn( rsaPtr->rsaQ->asn1Data, 
                            rsaPtr->rsaQ->asn1Length, keyPtr->q);
    }
    if ( rsaPtr->rsaPm != (ttAsn1IntegerPtr)0)
    {
        keyPtr->dmp1  = tfBN_bin2bn( rsaPtr->rsaPm->asn1Data, 
                            rsaPtr->rsaPm->asn1Length, keyPtr->dmp1);
    }
    if ( rsaPtr->rsaQm != (ttAsn1IntegerPtr)0)
    {
        keyPtr->dmq1  = tfBN_bin2bn( rsaPtr->rsaQm->asn1Data, 
                            rsaPtr->rsaQm->asn1Length, keyPtr->dmq1);
    }
    if ( rsaPtr->rsaPinv != (ttAsn1IntegerPtr)0)
    {
        keyPtr->iqmp  = tfBN_bin2bn( rsaPtr->rsaPinv->asn1Data,
                            rsaPtr->rsaPinv->asn1Length, keyPtr->iqmp);
    }

    retPtr = (ttVoidPtr)keyPtr;

rsaKeyAsn1ToBn:
    return retPtr;
}



/* RSA sign is RSAPrivateEncrypt */
int tfRSASign(int           flen,
              tt8BitPtr     strPtr,
              tt8BitPtr     sigPtr,
              ttVoidPtr     rsaPtr,
              ttIntPtr      siglenPtr)
{
    int errorCode;

    errorCode = tfRSAPrivateEncrypt(flen,
                                    strPtr,
                                    sigPtr,
                                    rsaPtr,
                                    siglenPtr);

    return errorCode;
}

/* RSA verify is RSAPublicDecrypt */
int tfRSAVerify (int        signedDataLength,/*  signed data length*/
                 tt8BitPtr  signedDataPtr,   /* signed data*/
                 tt8BitPtr  sigPtr,          /* signature */
                 ttVoidPtr  rsaPtr,          /* rsaKey ttRsaPtr */
                 int        siglen )         /* signature length*/
{
    int num;
    int errorCode;
    tt8BitPtr aPtr;
/* generally the signedDataLength and signedDataPtr is the output
 * of a hashing calculation
 */
    aPtr = tm_get_raw_buffer((ttPktLen)(signedDataLength+100));
    if ( aPtr == (tt8BitPtr )0 )
        return -1;
    (void)tfRSAPublicDecrypt(siglen,sigPtr,aPtr,rsaPtr,&num);
    errorCode = TM_ENOERROR;
    if (num != signedDataLength)
        errorCode = -1;
    else
    {
        errorCode = tm_memcmp(signedDataPtr,aPtr , num);
        if (errorCode != 0)
        errorCode = -1;
    }
    tm_free_raw_buffer(aPtr);
    return errorCode;
}





static int RSA_blinding_on(ttRSAPtr rsa, ttBN_CTXPtr p_ctx)
{
    ttBIGNUMBERPtr A,Ai;
    ttBN_CTXPtr ctx;
    int ret=0;
    int tempInt;

    if (p_ctx == TM_OPENSSL_NULL)
    {
        ctx= tfBN_CTX_new();
        if (ctx == TM_OPENSSL_NULL) goto err;
    }
    else
        ctx=p_ctx;
    
    if (rsa->blinding != TM_OPENSSL_NULL)
        tfBN_BLINDING_free(rsa->blinding);
    
    tfBN_CTX_start(ctx);
    A = tfBN_CTX_get(ctx);
    tempInt = tfBN_rand_range(A,rsa->n);
    if (!tempInt) goto err;
    Ai= tfBN_mod_inverse(TM_OPENSSL_NULL,A,rsa->n,ctx);
    if (Ai == TM_OPENSSL_NULL) goto err;

    tempInt = rsa->meth->bn_mod_exp(A,A,rsa->e,rsa->n,ctx,rsa->_method_mod_n);
    if (!tempInt)
        goto err;
    rsa->blinding= tfBN_BLINDING_new(A,Ai,rsa->n);
    rsa->flags|=TM_RSA_FLAG_BLINDING;
    tfBN_free(Ai);
    ret=1;
err:
    tfBN_CTX_end(ctx);
    if (ctx != p_ctx) tfBN_CTX_free(ctx);
    return(ret);
}

ttRSAPtr tfRsaNew()
{
    ttRsaMethodPtr meth;
    ttRSAPtr ret;
    int      tempInt;

    ret=(ttRSAPtr )tm_get_raw_buffer(sizeof(ttRSA));
    if (ret == TM_OPENSSL_NULL)
    {
        return(TM_OPENSSL_NULL);
    }
    
#ifdef TM_USE_MAPPED_CRYLIB
    ret->meth = (ttRsaMethodPtr)&rsa_pkcs1_map_meth;
#else
    ret->meth = (ttRsaMethodPtr)&rsa_pkcs1_eay_meth;
#endif /* TM_USE_MAPPED_CRYLIB */
    meth = ret->meth;
/*
    ret->pad=0;
    ret->version=0;
    ret->references=1;
    ret->bignum_data=TM_OPENSSL_NULL;
 */ 
    ret->n=TM_OPENSSL_NULL;
    ret->e=TM_OPENSSL_NULL;
    ret->d=TM_OPENSSL_NULL;
    ret->p=TM_OPENSSL_NULL;
    ret->q=TM_OPENSSL_NULL;
    ret->dmp1=TM_OPENSSL_NULL;
    ret->dmq1=TM_OPENSSL_NULL;
    ret->iqmp=TM_OPENSSL_NULL;
    ret->_method_mod_n=TM_OPENSSL_NULL;
    ret->_method_mod_p=TM_OPENSSL_NULL;
    ret->_method_mod_q=TM_OPENSSL_NULL;
    ret->blinding=TM_OPENSSL_NULL;
    
    ret->flags=meth->flags;
    
    if (meth->init != TM_OPENSSL_NULL)
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

void tfRsaFree(ttRSAPtr r)
{
    ttRsaMethodPtr meth;
    
    if (r == TM_OPENSSL_NULL) return;

    meth = r->meth;
    if (meth->finish != TM_OPENSSL_NULL)
        meth->finish(r);
    
    
    if (r->n != TM_OPENSSL_NULL) tfBN_clear_free(r->n);
    if (r->e != TM_OPENSSL_NULL) tfBN_clear_free(r->e);
    if (r->d != TM_OPENSSL_NULL) tfBN_clear_free(r->d);
    if (r->p != TM_OPENSSL_NULL) tfBN_clear_free(r->p);
    if (r->q != TM_OPENSSL_NULL) tfBN_clear_free(r->q);
    if (r->dmp1 != TM_OPENSSL_NULL) tfBN_clear_free(r->dmp1);
    if (r->dmq1 != TM_OPENSSL_NULL) tfBN_clear_free(r->dmq1);
    if (r->iqmp != TM_OPENSSL_NULL) tfBN_clear_free(r->iqmp);
    if (r->blinding != TM_OPENSSL_NULL) tfBN_BLINDING_free(r->blinding);
    tm_free_raw_buffer((tt8BitPtr)r);
}

int tfRsaSize(ttRSAPtr r)
{
	return(tm_bn_num_bytes(r->n));
}
int RSA_eay_public_encrypt(int flen, tt8BitPtr from,
            tt8BitPtr to, ttRSAPtr rsa, int padding)
{
    ttRsaMethodPtr meth;
    ttBIGNUMBER f,ret;
    int i,j,k,num=0,r= -1;
    int tempInt;
    ttBIGNUMBERPtr tempBn;
    tt8BitPtr buf=TM_OPENSSL_NULL;
    ttBN_CTXPtr ctx=TM_OPENSSL_NULL;
    

    meth = rsa->meth;
    tfBN_init(&f);
    tfBN_init(&ret);
    ctx = tfBN_CTX_new();
    if (ctx == TM_OPENSSL_NULL) goto err;
    num=tm_bn_num_bytes(rsa->n);
    buf=(tt8BitPtr )tm_get_raw_buffer((ttPktLen)num);
    if (buf == TM_OPENSSL_NULL)
    {
        goto err;
    }

    switch (padding)
    {
        case TM_RSA_PKCS1_PADDING:
            i=RSA_padding_add_PKCS1_type_2(buf,num,from,flen);
            break;
        case TM_RSA_PKCS1_OAEP_PADDING:
            i=RSA_padding_add_PKCS1_OAEP(buf,num,from,flen,TM_OPENSSL_NULL,0);
            break;
        case TM_RSA_SSLV23_PADDING:
            i=RSA_padding_add_SSLv23(buf,num,from,flen);
            break;
        case TM_RSA_NO_PADDING:
            i=RSA_padding_add_none(buf,num,from,flen);
            break;
        default:
            goto err;
    }
    if (i <= 0) goto err;
    tempBn = tfBN_bin2bn(buf,num,&f);
    if (tempBn == TM_OPENSSL_NULL) goto err;
    tempInt = tfBN_ucmp(&f, rsa->n);
    if (tempInt >= 0)
    {    
/* usually the padding functions would catch this */
        goto err;
    }
        
    if ((rsa->_method_mod_n == TM_OPENSSL_NULL)
        && (rsa->flags & TM_RSA_FLAG_CACHE_PUBLIC))
    {
        ttBN_MONT_CTXPtr bn_mont_ctx;
        bn_mont_ctx = tfBN_MONT_CTX_new();
        if (bn_mont_ctx == TM_OPENSSL_NULL)
            goto err;
        tempInt = tfBN_MONT_CTX_set(bn_mont_ctx,rsa->n,ctx);
        if (!tempInt)
        {
            tfBN_MONT_CTX_free(bn_mont_ctx);
            goto err;
        }
/* other thread may have finished first */
        if (rsa->_method_mod_n == TM_OPENSSL_NULL)
        {
            if (rsa->_method_mod_n == TM_OPENSSL_NULL)
            {
                rsa->_method_mod_n = bn_mont_ctx;
                bn_mont_ctx = TM_OPENSSL_NULL;
            }
        }
        if (bn_mont_ctx)
            tfBN_MONT_CTX_free(bn_mont_ctx);
    }
    tempInt = meth->bn_mod_exp(&ret,&f,rsa->e,rsa->n,ctx,rsa->_method_mod_n);
    if (!tempInt) goto err;
        
/* put in leading 0 bytes if the number is less than the length of the
   modulus */
    j=tm_bn_num_bytes(&ret);
    i= tfBN_bn2bin(&ret,&(to[num-j]));
    for (k=0; k<(num-i); k++)
        to[k]=0;
        
    r=num;
err:
    if (ctx != TM_OPENSSL_NULL) tfBN_CTX_free(ctx);
    tfBN_clear_free(&f);
    tfBN_clear_free(&ret);
    if (buf != TM_OPENSSL_NULL) 
    {
        tm_memset(buf,0,num);
        tm_free_raw_buffer(buf);
    }
    return(r);
}
    
/* signing */
int RSA_eay_private_encrypt(int flen, tt8BitPtr from,
        tt8BitPtr to, ttRSAPtr rsa, int padding)
{
    ttRsaMethodPtr meth;
    ttBIGNUMBER f,ret;
    int i,j,k,num=0,r= -1;
    int tempInt;
    ttBIGNUMBERPtr tempBn;
    tt8BitPtr buf=TM_OPENSSL_NULL;
    ttBN_CTXPtr ctx=TM_OPENSSL_NULL;
        
    meth = rsa->meth;
    tfBN_init(&f);
    tfBN_init(&ret);
        
    ctx = tfBN_CTX_new();
    if (ctx == TM_OPENSSL_NULL) goto err;
    num=tm_bn_num_bytes(rsa->n);
    buf=(tt8BitPtr )tm_get_raw_buffer((ttPktLen)num);
    if (buf == TM_OPENSSL_NULL)
    {
        goto err;
    }
        
    switch (padding)
    {
        case TM_RSA_PKCS1_PADDING:
            i=RSA_padding_add_PKCS1_type_1(buf,num,from,flen);
            break;
        case TM_RSA_NO_PADDING:
            i=RSA_padding_add_none(buf,num,from,flen);
            break;
        case TM_RSA_SSLV23_PADDING:
        default:
            goto err;
    }
    if (i <= 0) goto err;
        
    tempBn = tfBN_bin2bn(buf,num,&f);
    if (tempBn == TM_OPENSSL_NULL) goto err;
        
    tempInt = tfBN_ucmp(&f, rsa->n);
    if (tempInt >= 0)
    {    
/* usually the padding functions would catch this */
        goto err;
    }
        
    if ((rsa->flags & TM_RSA_FLAG_BLINDING)
        && (rsa->blinding == TM_OPENSSL_NULL))
        RSA_blinding_on(rsa,ctx);
    if (rsa->flags & TM_RSA_FLAG_BLINDING)
    {
        tempInt = tfBN_BLINDING_convert(&f,rsa->blinding,ctx);
        if (!tempInt) goto err;
    }
            
    if ( (rsa->flags & TM_RSA_FLAG_EXT_PKEY) ||
                ((rsa->p != TM_OPENSSL_NULL) &&
                (rsa->q != TM_OPENSSL_NULL) &&
                (rsa->dmp1 != TM_OPENSSL_NULL) &&
                (rsa->dmq1 != TM_OPENSSL_NULL) &&
                (rsa->iqmp != TM_OPENSSL_NULL)) )
    { 
        tempInt = meth->rsa_mod_exp(&ret,&f,rsa);
        if (!tempInt) goto err; 
    }
    else
    {
        tempInt = meth->bn_mod_exp(&ret,&f,rsa->d,rsa->n,ctx,TM_OPENSSL_NULL);
        if (!tempInt)
            goto err;
    }
            
    if (rsa->flags & TM_RSA_FLAG_BLINDING)
    {
        tempInt = tfBN_BLINDING_invert(&ret,rsa->blinding,ctx);
        if (!tempInt) goto err;
    }
                
/* put in leading 0 bytes if the number is less than the length of the
   modulus */
    j=tm_bn_num_bytes(&ret);
    i= tfBN_bn2bin(&ret,&(to[num-j]));
    for (k=0; k<(num-i); k++)
        to[k]=0;
                
    r=num;
err:
    if (ctx != TM_OPENSSL_NULL) tfBN_CTX_free(ctx);
    tfBN_clear_free(&ret);
    tfBN_clear_free(&f);
    if (buf != TM_OPENSSL_NULL)
    {
        tm_memset(buf,0,num);
        tm_free_raw_buffer(buf);
    }
    return(r);
}
    
int RSA_eay_private_decrypt(int flen, tt8BitPtr from,
       tt8BitPtr to, ttRSAPtr rsa, int padding)
{
    ttRsaMethodPtr meth;
    ttBIGNUMBER f,ret;
    int j,num=0,r= -1;
    int tempInt;
    tt8BitPtr p;
    ttBIGNUMBERPtr tempBn;
    tt8BitPtr buf=TM_OPENSSL_NULL;
    ttBN_CTXPtr ctx=TM_OPENSSL_NULL;
        
    meth = rsa->meth;
    tfBN_init(&f);
    tfBN_init(&ret);
    ctx = tfBN_CTX_new();
    if (ctx == TM_OPENSSL_NULL)
    {
        goto err;
    }
        
    num=tm_bn_num_bytes(rsa->n);
    buf=(tt8BitPtr )tm_get_raw_buffer((ttPktLen)num);
    if (buf == TM_OPENSSL_NULL)
    {
        goto err;
    }
        
/* This check was for equality but PGP does evil things
 * and chops off the top '0' bytes */
    if (flen > num)
    {
        goto err;
    }
        
/* make data into a big number */
    tempBn = tfBN_bin2bn(from,(int)flen,&f);
    if ( tempBn == TM_OPENSSL_NULL)
    {
        goto err;
    }
    
    tempInt = tfBN_ucmp(&f, rsa->n);
    if (tempInt >= 0)
    {
        goto err;
    }
        
    if ((rsa->flags & TM_RSA_FLAG_BLINDING) && (rsa->blinding == TM_OPENSSL_NULL))
        RSA_blinding_on(rsa,ctx);
    if (rsa->flags & TM_RSA_FLAG_BLINDING)
    {
        tempInt = tfBN_BLINDING_convert(&f,rsa->blinding,ctx);
        if (!tempInt)
        {
            goto err;
        }
    }
            
/* do the decrypt */
    if ( (rsa->flags & TM_RSA_FLAG_EXT_PKEY) ||
                ((rsa->p != TM_OPENSSL_NULL) &&
                (rsa->q != TM_OPENSSL_NULL) &&
                (rsa->dmp1 != TM_OPENSSL_NULL) &&
                (rsa->dmq1 != TM_OPENSSL_NULL) &&
                (rsa->iqmp != TM_OPENSSL_NULL)) )
    { 
        tempInt = meth->rsa_mod_exp(&ret,&f,rsa);
        if (!tempInt)
        {
            goto err; 
        }
    }
    else
    {
        tempInt = meth->bn_mod_exp(&ret,&f,rsa->d,rsa->n,ctx,TM_OPENSSL_NULL);
        if (!tempInt)
        {
            goto err;
        }
    }
            
    if (rsa->flags & TM_RSA_FLAG_BLINDING)
    {
        tempInt = tfBN_BLINDING_invert(&ret,rsa->blinding,ctx);
        if (!tempInt)
        {
            goto err;
        }
    }
                
    p=buf;
    j= tfBN_bn2bin(&ret,p); /* j is only used with no-padding mode */
                
    switch (padding)
    {
        case TM_RSA_PKCS1_PADDING:
            r=RSA_padding_check_PKCS1_type_2(to,num,buf,j,num);
            break;
        case TM_RSA_PKCS1_OAEP_PADDING:
            r=RSA_padding_check_PKCS1_OAEP(to,num,buf,j,num,TM_OPENSSL_NULL,0);
            break;
        case TM_RSA_SSLV23_PADDING:
            r=RSA_padding_check_SSLv23(to,num,buf,j,num);
            break;
        case TM_RSA_NO_PADDING:
            r=RSA_padding_check_none(to,num,buf,j,num);
            break;
        default:
            goto err;
    }
                
err:
    if (ctx != TM_OPENSSL_NULL)
    {
        tfBN_CTX_free(ctx);
    }
    tfBN_clear_free(&f);
    tfBN_clear_free(&ret);
    if (buf != TM_OPENSSL_NULL)
    {
        tm_memset(buf,0,num);
        tm_free_raw_buffer(buf);
    }
    return(r);
}
        
/* signature verification */
int RSA_eay_public_decrypt(int flen, tt8BitPtr from,
        tt8BitPtr to, ttRSAPtr rsa, int padding)
{
    ttRsaMethodPtr meth;
    ttBIGNUMBER f,ret;
    ttBIGNUMBERPtr tempBn;
    int i,num=0,r= -1;
    int tempInt;
    tt8BitPtr p;
    tt8BitPtr buf=TM_OPENSSL_NULL;
    ttBN_CTXPtr ctx=TM_OPENSSL_NULL;
        
    meth = rsa->meth;
    tfBN_init(&f);
    tfBN_init(&ret);
    ctx = tfBN_CTX_new();
    if (ctx == TM_OPENSSL_NULL)
    {
        goto err;
    }
        
    num=tm_bn_num_bytes(rsa->n);
    buf=(tt8BitPtr )tm_get_raw_buffer((ttPktLen)num);
    if (buf == TM_OPENSSL_NULL)
    {
        goto err;
    }
        
/* This check was for equality but PGP does evil things
 * and chops off the top '0' bytes */
    if (flen > num)
    {
        goto err;
    }
        
    tempBn = tfBN_bin2bn(from,flen,&f);
    if (tempBn == TM_OPENSSL_NULL)
    {
        goto err;
    }
        
    tempInt = tfBN_ucmp(&f, rsa->n);
    if ( tempInt >= 0)
    {
        goto err;
    }
        
/* do the decrypt */
    if ((rsa->_method_mod_n == TM_OPENSSL_NULL)
        && (rsa->flags & TM_RSA_FLAG_CACHE_PUBLIC))
    {
        ttBN_MONT_CTXPtr bn_mont_ctx;
        bn_mont_ctx = tfBN_MONT_CTX_new();
        if (bn_mont_ctx == TM_OPENSSL_NULL)
        {
            goto err;
        }
        tempInt = tfBN_MONT_CTX_set(bn_mont_ctx,rsa->n,ctx);
        if (!tempInt)
        {
            tfBN_MONT_CTX_free(bn_mont_ctx);
            goto err;
        }
/* other thread may have finished first */
        if (rsa->_method_mod_n == TM_OPENSSL_NULL)
        {
            if (rsa->_method_mod_n == TM_OPENSSL_NULL)
            {
                rsa->_method_mod_n = bn_mont_ctx;
                bn_mont_ctx = TM_OPENSSL_NULL;
            }
        }
        if (bn_mont_ctx)
            tfBN_MONT_CTX_free(bn_mont_ctx);
    }
    tempInt = meth->bn_mod_exp(&ret,&f,rsa->e,rsa->n,ctx,rsa->_method_mod_n);
    if (!tempInt)
    {
        goto err;
    }
        
    p=buf;
    i= tfBN_bn2bin(&ret,p);
        
    switch (padding)
    {
        case TM_RSA_PKCS1_PADDING:
            r=RSA_padding_check_PKCS1_type_1(to,num,buf,i,num);
            break;
        case TM_RSA_NO_PADDING:
            r=RSA_padding_check_none(to,num,buf,i,num);
            break;
        default:
            goto err;
    }
        
err:
    if (ctx != TM_OPENSSL_NULL)
    {
        tfBN_CTX_free(ctx);
    }
    tfBN_clear_free(&f);
    tfBN_clear_free(&ret);
    if (buf != TM_OPENSSL_NULL)
    {
        tm_memset(buf,0,num);
        tm_free_raw_buffer(buf);
    }

    return(r);
}
    
int RSA_eay_mod_exp(ttBIGNUMBERPtr r0, ttBIGNUMBERPtr I, ttRSAPtr rsa)
{
    ttRsaMethodPtr meth;
    ttBIGNUMBER r1,m1,vrfy;
    int ret=0;
    int tempInt;
    ttBN_CTXPtr ctx;
        
    meth = rsa->meth;
    tfBN_init(&m1);
    tfBN_init(&r1);
    tfBN_init(&vrfy);
    ctx = tfBN_CTX_new();
    if (ctx == TM_OPENSSL_NULL) goto err;
        
    if (rsa->flags & TM_RSA_FLAG_CACHE_PRIVATE)
    {
        if (rsa->_method_mod_p == TM_OPENSSL_NULL)
        {
            ttBN_MONT_CTXPtr bn_mont_ctx;
            bn_mont_ctx = tfBN_MONT_CTX_new();
            if (bn_mont_ctx == TM_OPENSSL_NULL)
                goto err;
            tempInt = tfBN_MONT_CTX_set(bn_mont_ctx,rsa->p,ctx);
            if (!tempInt)
            {
                tfBN_MONT_CTX_free(bn_mont_ctx);
                goto err;
            }
/* other thread may have finished first */
            if (rsa->_method_mod_p == TM_OPENSSL_NULL)
            {
                if (rsa->_method_mod_p == TM_OPENSSL_NULL)
                {
                    rsa->_method_mod_p = bn_mont_ctx;
                    bn_mont_ctx = TM_OPENSSL_NULL;
                }
            }
            if (bn_mont_ctx)
                tfBN_MONT_CTX_free(bn_mont_ctx);
        }
            
        if (rsa->_method_mod_q == TM_OPENSSL_NULL)
        {
            ttBN_MONT_CTXPtr bn_mont_ctx;
            bn_mont_ctx = tfBN_MONT_CTX_new();
            if (bn_mont_ctx == TM_OPENSSL_NULL)
                goto err;
            tempInt = tfBN_MONT_CTX_set(bn_mont_ctx,rsa->q,ctx);
            if (!tempInt)
            {
                tfBN_MONT_CTX_free(bn_mont_ctx);
                goto err;
            }
/* other thread may have finished first */            
            if (rsa->_method_mod_q == TM_OPENSSL_NULL)
            {
                if (rsa->_method_mod_q == TM_OPENSSL_NULL)
                {
                    rsa->_method_mod_q = bn_mont_ctx;
                    bn_mont_ctx = TM_OPENSSL_NULL;
                }
            }
            if (bn_mont_ctx)
                tfBN_MONT_CTX_free(bn_mont_ctx);
        }
    }
        
    tempInt = tfBN_mod(&r1,I,rsa->q,ctx);
    if (!tempInt) goto err;
    tempInt = meth->bn_mod_exp(&m1,&r1,rsa->dmq1,rsa->q,ctx,rsa->_method_mod_q);
    if (!tempInt) goto err;
        
    tempInt = tfBN_mod(&r1,I,rsa->p,ctx);
    if (!tempInt) goto err;
    tempInt = meth->bn_mod_exp(r0,&r1,rsa->dmp1,rsa->p,ctx,rsa->_method_mod_p);
    if (!tempInt) goto err;
        
    tempInt = tfBN_sub(r0,r0,&m1);
    if (!tempInt) goto err;
/* This will help stop the size of r0 increasing, which does
 * affect the multiply if it optimised for a power of 2 size */
    if (r0->neg)
    {
        tempInt = tfBN_add(r0,r0,rsa->p);
        if (!tempInt) goto err;
    }
    tempInt = tfBN_mul(&r1,r0,rsa->iqmp,ctx);
    if (!tempInt ) goto err;
    tempInt = tfBN_mod(r0,&r1,rsa->p,ctx);
    if (!tempInt) goto err;
/* If p < q it is occasionally possible for the correction of
 * adding 'p' if r0 is negative above to leave the result still
 * negative. This can break the private key operations: the following
 * second correction should *always* correct this rare occurrence.
 * This will *never* happen with OpenSSL generated keys because
 * they ensure p > q [steve]
 */
    if (r0->neg)
    {
        tempInt = tfBN_add(r0,r0,rsa->p);
        if (!tempInt) goto err;
    }
    tempInt = tfBN_mul(&r1,r0,rsa->q,ctx);
    if (!tempInt ) goto err;
    tempInt = tfBN_add(r0,&r1,&m1);
    if (!tempInt ) goto err;
                
    if (rsa->e && rsa->n)
    {
        tempInt = meth->bn_mod_exp(&vrfy,r0,rsa->e,rsa->n,ctx,TM_OPENSSL_NULL);
        if (!tempInt)
            goto err;
        tempInt = tfBN_cmp(I, &vrfy);
        if (tempInt != 0)
        {
            tempInt = meth->bn_mod_exp(r0,I,rsa->d,rsa->n,ctx,TM_OPENSSL_NULL);
            if (!tempInt)
                goto err;
        }
    }
    ret=1;
err:
    tfBN_clear_free(&m1);
    tfBN_clear_free(&r1);
    tfBN_clear_free(&vrfy);
    tfBN_CTX_free(ctx);
    return(ret);
}
int RSA_eay_init(ttRSAPtr rsa)
{
    rsa->flags|=TM_RSA_FLAG_CACHE_PUBLIC|TM_RSA_FLAG_CACHE_PRIVATE;
    return(1);
}

int RSA_eay_finish(ttRSAPtr rsa)
{
    if (rsa->_method_mod_n != TM_OPENSSL_NULL)
        tfBN_MONT_CTX_free(rsa->_method_mod_n);
    if (rsa->_method_mod_p != TM_OPENSSL_NULL)
        tfBN_MONT_CTX_free(rsa->_method_mod_p);
    if (rsa->_method_mod_q != TM_OPENSSL_NULL)
        tfBN_MONT_CTX_free(rsa->_method_mod_q);
    return(1);
}

    
/* Original rsa_none.c */
    
    
static int RSA_padding_add_none(tt8BitPtr to, int tlen, tt8BitPtr from,
        int flen)
{
    if (flen != tlen)
    {
        return(0);
    }
        
    tm_memcpy(to,from,(tt32Bit)flen);
    return(1);
}
    
static int RSA_padding_check_none(tt8BitPtr to, int tlen, tt8BitPtr from,
        int flen, int num)
{
	TM_UNREF_IN_ARG(num);
    if (flen > tlen)
    {
        return(-1);
    }
        
    tm_memset(to,0,tlen-flen);
    tm_memcpy(to+tlen-flen,from,flen);
    return(tlen);
}
   
/* Original rsa_pk1.c */
    
static int RSA_padding_add_PKCS1_type_1(tt8BitPtr to, int tlen,
        tt8BitPtr from, int flen)
{
    int j;
    tt8BitPtr p;
        
    if (flen > (tlen-11))
    {
        return(0);
    }
        
    p=(tt8BitPtr )to;
        
    *(p++)=0;
/* Private Key BT (Block Type) */
    *(p++)=1; 
        
/* pad out with 0xff data */
    j=tlen-3-flen;
    tm_memset(p,0xff,j);
    p+=j;
    *(p++)='\0';
    tm_memcpy(p,from,(tt32Bit)flen);
    return(1);
}
    
static int RSA_padding_check_PKCS1_type_1(tt8BitPtr to, int tlen,
        tt8BitPtr from, int flen, int num)
{
    int i,j;
    tt8BitPtr p;
        
    p=from;
    if ((num != (flen+1)) || (*p != 01))
    {
        return(-1);
    }
    p++;
/* scan over padding data */
    j=flen-1; /* one for type. */
    for (i=0; i<j; i++)
    {
        if (*p != 0xff) /* should decrypt to 0xff */
        {
            if (*p == 0)
            { 
                p++; 
                break; 
            }
            else    
            {
                return(-1);
            }
        }
        p++;
    }
        
    if (i == j)
    {
        return(-1);
    }
        
    if (i < 8)
    {
        return(-1);
    }
    i++; /* Skip over the '\0' */
    j-=i;
    if (j > tlen)
    {
        return(-1);
    }
    tm_memcpy(to,p,(tt32Bit)j);
        
    return(j);
}
    
static int RSA_padding_add_PKCS1_type_2(tt8BitPtr to, int tlen,
        tt8BitPtr from, int flen)
{
    int i,j;
    tt8BitPtr p;
        
    if (flen > (tlen-11))
    {
        return(0);
    }
        
    p=(tt8BitPtr)to;
        
    *(p++)=0;
/* Public Key BT (Block Type) */
    *(p++)=2;         
/* pad out with non-zero random data */
    j=tlen-3-flen;
        
    tfGetRandomBytes(p,j);
    for (i=0; i<j; i++)
    {
        if (*p == '\0')
        do
        {
            tfGetRandomBytes(p,1);
        } 
        while (*p == '\0');
        p++;
    }
        
    *(p++)='\0';
        
    tm_memcpy(p,from,(tt32Bit)flen);
    return(1);
}
    
static int RSA_padding_check_PKCS1_type_2(tt8BitPtr to, int tlen,
        tt8BitPtr from, int flen, int num)
{
    int i,j;
    tt8BitPtr p;
        
    p=from;
    if ((num != (flen+1)) || (*p != 02))
    {
        return(-1);
    }
    p++;
#ifdef PKCS1_CHECK
    return(num-11);
#endif
        
/* scan over padding data */
    j=flen-1; /* one for type. */
    for (i=0; i<j; i++)
    {
        if (*p == 0)
        {
            p++;
            break;
        }
        p++;
    }
            
    if (i == j)
    {
        return(-1);
    }
            
    if (i < 8)
    {
        return(-1);
    }
    i++; /* Skip over the '\0' */
    j-=i;
    if (j > tlen)
    {
        return(-1);
    }
    tm_memcpy(to,p,(tt32Bit)j);
            
    return(j);
}
    
/* Original rsa_ssl.c */
    
static int RSA_padding_add_SSLv23(tt8BitPtr to, int tlen, tt8BitPtr from,
        int flen)
{
    int i,j;
    tt8BitPtr p;
        
    if (flen > (tlen-11))
    {
        return(0);
    }
        
    p=(tt8BitPtr)to;
        
    *(p++)=0;
    *(p++)=2; /* Public Key BT (Block Type) */
        
/* pad out with non-zero random data */
    j=tlen-3-8-flen;
        
    tfGetRandomBytes(p,j);
    for (i=0; i<j; i++)
    {
        if (*p == '\0')
        do
        {
            tfGetRandomBytes(p,1);
        } 
        while (*p == '\0');
        p++;
    }
        
    tm_memset(p,3,8);
    p+=8;
    *(p++)='\0';
        
    tm_memcpy(p,from,(tt32Bit)flen);
    return(1);
}
    
static int RSA_padding_check_SSLv23(tt8BitPtr to, int tlen, tt8BitPtr from,
        int flen, int num)
{
    int i,j,k;
    tt8BitPtr p;
        
    p=from;
    if (flen < 10)
    {
        return(-1);
    }
    if ((num != (flen+1)) || (*p != 02))
    {
        return(-1);
    }
    p++;
/* scan over padding data */
    j=flen-1; /* one for type */
    for (i=0; i<j; i++)
    {
        if (*p == 0)
        {
            p++;
            break;
        }
        p++;
    }
            
    if ((i == j) || (i < 8))
    {
        return(-1);
    }
    for (k= -8; k<0; k++)
    {
        if (p[k] !=  0x03) break;
    }
    if (k == -1)
    {
        return(-1);
    }
            
    i++; /* Skip over the '\0' */
    j-=i;
    if (j > tlen)
    {
        return(-1);
    }
    tm_memcpy(to,p,(tt32Bit)j);
    return(j);
}

/* RSA_padding_add_PKCS1_OAEP that is for RSA encryption, may
 * be not usefule for signature
 */
static int tfRsaMGF1(tt8BitPtr mask, ttPktLen len,
	tt8BitPtr seed, ttPktLen seedlen);

static int RSA_padding_add_PKCS1_OAEP( tt8BitPtr to, 
                                int tlen,
	                            tt8BitPtr from, 
                                int flen,
	                            tt8BitPtr param, 
                                int plen)
{
    int i, emlen = tlen - 1;
    tt8BitPtr db, seed;
    tt8BitPtr dbmask;
    tt8Bit    seedmask[TM_SHA1_HASH_SIZE];

    ttSha1Ctx     c;

	if (flen > emlen - 2 * TM_SHA1_HASH_SIZE - 1)
    {
        return 0;
    }

    if (emlen < 2 * TM_SHA1_HASH_SIZE + 1)
    {
        return 0;
    }

    dbmask = tm_get_raw_buffer((ttPktLen)emlen - TM_SHA1_HASH_SIZE);
    if (dbmask == TM_OPENSSL_NULL)
	{
	    return 0;
    }
    
    to[0] = 0;
    seed = to + 1;
    db = to + TM_SHA1_HASH_SIZE + 1;

/*    SHA1(param, plen, db);*/
    tfSha1Init(&c);
    tfSha1Update(&c, param, (ttPktLen)plen, 0);
    tfSha1Final(db, &c);

    tm_memset(  db + TM_SHA1_HASH_SIZE, 0,
                emlen - flen - 2 * TM_SHA1_HASH_SIZE - 1);
    db[emlen - flen - TM_SHA1_HASH_SIZE - 1] = 0x01;
    tm_memcpy(db + emlen - flen - TM_SHA1_HASH_SIZE, from, (tt32Bit) flen);

    tfGetRandomBytes(seed, TM_SHA1_HASH_SIZE);

    tfRsaMGF1(dbmask, (ttPktLen)emlen - TM_SHA1_HASH_SIZE, 
              seed, TM_SHA1_HASH_SIZE);
    for (i = 0; i < emlen - TM_SHA1_HASH_SIZE; i++)
    {
        db[i] ^= dbmask[i];
    }

    tfRsaMGF1(seedmask, TM_SHA1_HASH_SIZE, db, 
              (ttPktLen)emlen - TM_SHA1_HASH_SIZE);
    for (i = 0; i < TM_SHA1_HASH_SIZE; i++)
    {
        seed[i] ^= seedmask[i];
    }

	tm_free_raw_buffer(dbmask);
    return 1;
}

static int RSA_padding_check_PKCS1_OAEP(tt8BitPtr to, 
                                 int tlen,
	                             tt8BitPtr from, 
                                 int flen, 
                                 int num, 
                                 tt8BitPtr param,
                                 int plen)
{
    int i, dblen, mlen = -1;
    tt8BitPtr maskeddb;
    int lzero;
    tt8Bit seed[TM_SHA1_HASH_SIZE]; 
    tt8BitPtr db = TM_OPENSSL_NULL; 
    tt8Bit phash[TM_SHA1_HASH_SIZE];
    int bad = 0;
    int tempInt;

    ttSha1Ctx     c;

    num--;
    if (num < 2 * TM_SHA1_HASH_SIZE + 1)
		/* 'num' is the length of the modulus, i.e. does not depend on the
		 * particular ciphertext. */
    {
        goto decoding_err;
    }

    lzero = num - flen;
    if (lzero < 0)
    {
		/* lzero == -1 */

		/* signalling this error immediately after detection might allow
		 * for side-channel attacks (e.g. timing if 'plen' is huge
		 * -- cf. James H. Manger, "A Chosen Ciphertext Attack on RSA Optimal
		 * Asymmetric Encryption Padding (OAEP) [...]", CRYPTO 2001),
		 * so we use a 'bad' flag */
        bad = 1;
        lzero = 0;
    }
    maskeddb = from - lzero + TM_SHA1_HASH_SIZE;

    dblen = num - TM_SHA1_HASH_SIZE;
    db = tm_get_raw_buffer((ttPktLen)dblen);
    if (db == TM_OPENSSL_NULL)
    {
        return -1;
    }

	tfRsaMGF1(seed, TM_SHA1_HASH_SIZE, maskeddb, (ttPktLen)dblen);
	for (i = lzero; i < TM_SHA1_HASH_SIZE; i++)
    {
        seed[i] ^= from[i - lzero];
    }

    tfRsaMGF1(db, (ttPktLen)dblen, seed, TM_SHA1_HASH_SIZE);
    for (i = 0; i < dblen; i++)
    {
        db[i] ^= maskeddb[i];
    }

/*    SHA1(param, plen, phash);*/
    tfSha1Init(&c);
    tfSha1Update(&c, param, (ttPktLen)plen, 0);
    tfSha1Final(phash, &c);

    tempInt = tm_memcmp(db, phash, TM_SHA1_HASH_SIZE);
    if (tempInt != 0 || bad)
    {
        goto decoding_err;
    }
	else
	{
		for (i = TM_SHA1_HASH_SIZE; i < dblen; i++)
        {
            if (db[i] != 0x00)
            {
				break;
            }
        }
		if (db[i] != 0x01 || i >= dblen)
        {
			goto decoding_err;
        }
        else
        {
            i++;
			/* everything looks OK */
            mlen = dblen - i;
            if (tlen < mlen)
            {
                mlen = -1;
            }
            else
            {
                tm_memcpy(to, db + i, mlen);
            }
        }
    }
	tm_free_raw_buffer(db);
	return mlen;

decoding_err:
	/* to avoid chosen ciphertext attacks, the error message should not reveal
	 * which kind of decoding error happened */
	if (db != TM_OPENSSL_NULL) 
    {
        tm_free_raw_buffer(db);
    }
	return -1;
}

static int tfRsaMGF1(
    tt8BitPtr mask, ttPktLen len, tt8BitPtr seed, ttPktLen seedlen)
{
    ttPktLen i, outlen = 0;
    tt8Bit cnt[4];
    ttSha1Ctx     c;
    tt8Bit md[TM_SHA1_HASH_SIZE];

    for (i = 0; outlen < len; i++)
    {
        cnt[0] = (tt8Bit )(i >> 24);
        cnt[1] = (tt8Bit )(i >> 16);
        cnt[2] = (tt8Bit )(i >> 8); 
        cnt[3] = (tt8Bit )i;
        tfSha1Init(&c);
        tfSha1Update(&c, seed, seedlen,0);
        tfSha1Update(&c, cnt, 4, 0);
        if (outlen + TM_SHA1_HASH_SIZE <= len)
        {
            tfSha1Final(mask + outlen, &c);
            outlen += TM_SHA1_HASH_SIZE;
        }
        else
        {
		    tfSha1Final(md, &c);
            tm_memcpy(mask + outlen, md, len - outlen);
            outlen = len;
        }
    }
    return 0;
}


#else /* ! TM_PUBKEY_USE_RSA */
/* To allow link for builds when TM_USE_PKI is not defined */
int tvRsaDummy = 0;

#endif /* TM_PUBKEY_USE_RSA */
