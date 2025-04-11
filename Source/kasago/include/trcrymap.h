/*
 * Description: Macro Definitions for mapped crypto functions
 * Filename: trcrymap.h
 * Author: Tim
 * Date Created: 3/27/2006
 * $Source: include/trcrymap.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:19JST $
 * $Author: odile $
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
#ifdef TM_USE_MAPPED_CRYLIB

#ifdef TM_PUBKEY_USE_DSA
#define tm_dsa_map_sign(dgst,dlen,dsa) \
            dsa_do_sign((dgst),(dlen),(dsa))

#define tm_dsa_map_sign_setup(dsa,ctx_in,kinvp,rp) \
            dsa_sign_setup((dsa),(ctx_in),(kinvp),(rp))

#define tm_dsa_map_verify(dgst,dlen,sig,dsa) \
            dsa_do_verify((dgst),(dlen),(sig),(dsa))

#define tm_dsa_map_mod_exp(dsa,rr,a1,p1,a2,p2,m,ctx,in_mont) \
            dsa_mod_exp((dsa),(rr),(a1),(p1),(a2),(p2),(m),(ctx),(in_mont))

/* The pointer to the DSA object (first argument) is unused */
#define tm_dsa_map_bn_mod_exp_mont(dsa,r,a,p,m,ctx,m_ctx) \
    tfBN_mod_exp_mont((r),(a),(p),(m),(ctx),(m_ctx))

#define tm_dsa_map_init(dsa) \
            dsa_init((dsa))

#define tm_dsa_map_finish(dsa) \
            dsa_finish((dsa))
#endif /* TM_PUBKEY_USE_DSA */

#ifdef TM_PUBKEY_USE_RSA
#define tm_rsa_map_public_encrypt(flen,from,to,rsa,padding) \
            RSA_eay_public_encrypt((flen),(from),(to),(rsa),(padding))

#define tm_rsa_map_public_decrypt(flen,from,to,rsa,padding) \
            RSA_eay_public_decrypt((flen),(from),(to),(rsa),(padding))

#define tm_rsa_map_private_encrypt(flen,from,to,rsa,padding) \
            RSA_eay_private_encrypt((flen),(from),(to),(rsa),(padding))

#define tm_rsa_map_private_decrypt(flen,from,to,rsa,padding) \
            RSA_eay_private_decrypt((flen),(from),(to),(rsa),(padding))

#define tm_rsa_map_mod_exp(r0,I,rsa) \
            RSA_eay_mod_exp((r0),(I),(rsa))

/* The pointer to the DSA object (first argument) is unused */
#define tm_rsa_map_bn_mod_exp_mont(r,a,p,m,ctx,m_ctx) \
    tfBN_mod_exp_mont((r),(a),(p),(m),(ctx),(m_ctx)); 

#define tm_rsa_map_init(rsa) \
            RSA_eay_init((rsa))

#define tm_rsa_map_finish(rsa) \
            RSA_eay_finish((rsa))
#endif /* TM_PUBKEY_USE_RSA */

#ifdef TM_PUBKEY_USE_DIFFIEHELLMAN
/* Diffie-Hellman */
/* The pointer to the DH object (first argument) is unused */
#define tm_dh_map_bn_mod_exp_mont(dh,r,a,p,m,ctx,m_ctx) \
    tfDH_bn_mod_exp((r),(a),(p),(m),(ctx),(m_ctx))
#endif /* TM_PUBKEY_USE_DIFFIEHELLMAN */

/* Random Number Generator (RNG) */
#define tm_map_get_random_bytes(rPtr, size) \
{ \
    int i; \
    for(i = 0; i < (size); i++) \
    { \
        *((rPtr) + i) = tfGetRandomByte(); \
    } \
}

#ifdef TM_PUBKEY_USE_DSA
/* DSA */
ttDsaSigPtr dsa_do_sign            (ttConst8BitPtr dgst, 
                                    int dlen, 
                                    ttDSAPtr dsa);

int dsa_sign_setup                 (ttDSAPtr dsa, 
                                    ttBN_CTXPtr ctx_in, 
                                    ttBIGNUMBERPtrPtr kinvp,
                                    ttBIGNUMBERPtrPtr rp);

int dsa_do_verify                  (ttConst8BitPtr dgst, 
                                    int dgst_len, 
                                    ttDsaSigPtr sig, 
                                    ttDSAPtr dsa);

int dsa_init                       (ttDSAPtr dsa);

int dsa_finish                     (ttDSAPtr dsa);

int dsa_mod_exp                    (ttDSAPtr dsa, 
                                    ttBIGNUMBERPtr rr, 
                                    ttBIGNUMBERPtr a1, 
                                    ttBIGNUMBERPtr p1, 
                                    ttBIGNUMBERPtr a2, 
                                    ttBIGNUMBERPtr p2, 
                                    ttBIGNUMBERPtr m, 
                                    ttBN_CTXPtr ctx,
                                    ttBN_MONT_CTXPtr in_mont);

int dsa_bn_mod_exp                 (ttDSAPtr dsa, 
                                    ttBIGNUMBERPtr r, 
                                    ttBIGNUMBERPtr a, 
                                    ttConstBIGNUMBERPtr p,  
                                    ttConstBIGNUMBERPtr m, 
                                    ttBN_CTXPtr ctx, 
                                    ttBN_MONT_CTXPtr m_ctx);

ttDsaSigPtr tfDsaMapSign           (ttConst8BitPtr dgst, 
                                    int dlen, 
                                    ttDSAPtr dsa);

int tfDsaMapSignSetup              (ttDSAPtr dsa, 
                                    ttBN_CTXPtr ctx_in, 
                                    ttBIGNUMBERPtrPtr kinvp, 
                                    ttBIGNUMBERPtrPtr rp);

int tfDsaMapVerify                 (ttConst8BitPtr dgst, 
                                    int dgst_len, 
                                    ttDsaSigPtr sig, 
                                    ttDSAPtr dsa);

int tfDsaMapModExp                 (ttDSAPtr dsa, 
                                    ttBIGNUMBERPtr rr, 
                                    ttBIGNUMBERPtr a1, 
                                    ttBIGNUMBERPtr p1, 
                                    ttBIGNUMBERPtr a2, 
                                    ttBIGNUMBERPtr p2, 
                                    ttBIGNUMBERPtr m, 
                                    ttBN_CTXPtr ctx, 
                                    ttBN_MONT_CTXPtr in_mont);

int tfDsaMapBnModExpMont           (ttDSAPtr dsa, 
                                    ttBIGNUMBERPtr r, 
                                    ttBIGNUMBERPtr a, 
                                    ttConstBIGNUMBERPtr p, 
                                    ttConstBIGNUMBERPtr m, 
                                    ttBN_CTXPtr ctx, 
                                    ttBN_MONT_CTXPtr m_ctx); 

int tfDsaMapInit                   (ttDSAPtr dsa);

int tfDsaMapFinish                 (ttDSAPtr dsa);
#endif /* TM_PUBKEY_USE_DSA */

#ifdef TM_PUBKEY_USE_RSA
/* RSA */
int tfRsaMapPublicEncrypt          (int flen, 
                                    tt8BitPtr from, 
                                    tt8BitPtr to, 
                                    ttRSAPtr rsa, 
                                    int padding);

int tfRsaMapPublicDecrypt          (int flen, 
                                    tt8BitPtr fom, 
                                    tt8BitPtr to, 
                                    ttRSAPtr rsa, 
                                    int padding);

int tfRsaMapPrivateEncrypt         (int flen, 
                                    tt8BitPtr from, 
                                    tt8BitPtr to, 
                                    ttRSAPtr rsa, 
                                    int padding);

int tfRsaMapPrivateDecrypt         (int flen, 
                                    tt8BitPtr from, 
                                    tt8BitPtr to, 
                                    ttRSAPtr rsa, 
                                    int padding);

int tfRsaMapModExp                 (ttBIGNUMBERPtr r0, 
                                    ttBIGNUMBERPtr I, 
                                    ttRSAPtr rsa); 

int tfRsaMapBnModExpMont           (ttBIGNUMBERPtr r, 
                                    ttBIGNUMBERPtr a, 
                                    ttConstBIGNUMBERPtr p,
                                    ttConstBIGNUMBERPtr m, 
                                    ttBN_CTXPtr ctx,
                                    ttBN_MONT_CTXPtr m_ctx); 

int tfRsaMapInit                   (ttRSAPtr rsa);        

int tfRsaMapFinish                 (ttRSAPtr rsa);

/* EAY RSA definitions */
int RSA_eay_public_encrypt         (int flen, 
                                    tt8BitPtr from,
                                    tt8BitPtr to, 
                                    ttRSAPtr rsa,
                                    int padding);

int RSA_eay_private_encrypt        (int flen, 
                                    tt8BitPtr from,
                                    tt8BitPtr to, 
                                    ttRSAPtr rsa,
                                    int padding);

int RSA_eay_public_decrypt         (int flen, 
                                    tt8BitPtr from,
                                    tt8BitPtr to, 
                                    ttRSAPtr rsa,
                                    int padding);

int RSA_eay_private_decrypt        (int flen, 
                                    tt8BitPtr from,
                                    tt8BitPtr to, 
                                    ttRSAPtr rsa,
                                    int padding);

int RSA_eay_mod_exp                (ttBIGNUMBERPtr r0, 
                                    ttBIGNUMBERPtr i, 
                                    ttRSAPtr rsa);

int RSA_eay_init                   (ttRSAPtr rsa);

int RSA_eay_finish                 (ttRSAPtr rsa);
#endif /* TM_PUBKEY_USE_RSA */


#ifdef TM_PUBKEY_USE_DIFFIEHELLMAN
/* Diffie-Hellman */
int tfDH_bn_mod_exp                (ttBIGNUMBERPtr r, 
                                    ttBIGNUMBERPtr a, 
                                    ttConstBIGNUMBERPtr p,
                                    ttConstBIGNUMBERPtr m, 
                                    ttBN_CTXPtr ctx, 
                                    ttBN_MONT_CTXPtr m_ctx);

int tfDhMapBnModExpMont            (ttDhInBnPtr dh,
                                    ttBIGNUMBERPtr r, 
                                    ttBIGNUMBERPtr a, 
                                    ttConstBIGNUMBERPtr p,
                                    ttConstBIGNUMBERPtr m, 
                                    ttBN_CTXPtr ctx,
                                    ttBN_MONT_CTXPtr m_ctx);

#endif /* TM_PUBKEY_USE_DIFFIEHELLMAN */

void tfMapGetRandomBytes           (tt8BitPtr rPtr,
                                    int size);

#endif /* TM_USE_MAPPED_CRYLIB */
