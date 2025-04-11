/*
 * Copyright Notice:
 * Copyright Treck Incorporated  1997 - 2016
 * Copyright Zuken Elmic Japan   1997 - 2016
 * No portions or the complete contents of this file may be copied or
 * distributed in any form (including but not limited to printed or electronic
 * forms) without the expressed written consent of Treck Incorporated OR
 * Zuken Elmic.  Copyright laws and International Treaties protect the
 * contents of this file.  Unauthorized use is prohibited.
 * All rights reserved.
 *
 * Description: trcrypam.c - Mapping of Crypto library functions if they are
 *   to be used with an alternative software or hardware implementation
 *
 * Filename: trcrymap.c
 * Author: Tim
 * Date Created: 3/27/2006
 * $Source: source/cryptlib/trcrymap.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:35:26JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

#ifdef TM_USE_MAPPED_CRYLIB

#include <trcrylib.h>
#include <tropnssl.h>
#include <trpki.h> 
#include <trcrymap.h>

#ifdef TM_PUBKEY_USE_DSA
/* Common mapped DSA functions */
ttDsaSigPtr tfDsaMapSign    (ttConst8BitPtr dgst, 
                             int dlen, 
                             ttDSAPtr dsa)
{
    return tm_dsa_map_sign(dgst, dlen, dsa);
}
int tfDsaMapSignSetup       (ttDSAPtr dsa, 
                             ttBN_CTXPtr ctx_in, 
                             ttBIGNUMBERPtrPtr kinvp, 
                             ttBIGNUMBERPtrPtr rp)
{
    return tm_dsa_map_sign_setup(dsa, ctx_in, kinvp, rp);
}
int tfDsaMapVerify          (ttConst8BitPtr dgst, 
                             int dgst_len, 
                             ttDsaSigPtr sig, 
                             ttDSAPtr dsa)
{
    return tm_dsa_map_verify(dgst, dgst_len, sig, dsa);
}
int tfDsaMapModExp          (ttDSAPtr dsa, 
                             ttBIGNUMBERPtr rr, 
                             ttBIGNUMBERPtr a1, 
                             ttBIGNUMBERPtr p1, 
                             ttBIGNUMBERPtr a2, 
                             ttBIGNUMBERPtr p2, 
                             ttBIGNUMBERPtr m, 
                             ttBN_CTXPtr ctx, 
                             ttBN_MONT_CTXPtr in_mont)
{
    return tm_dsa_map_mod_exp(dsa, rr, a1, p1, a2, p2, m, ctx, in_mont);
}
int tfDsaMapBnModExpMont    (ttDSAPtr dsa, 
                             ttBIGNUMBERPtr r, 
                             ttBIGNUMBERPtr a, 
                             ttConstBIGNUMBERPtr p, 
                             ttConstBIGNUMBERPtr m, 
                             ttBN_CTXPtr ctx, 
                             ttBN_MONT_CTXPtr m_ctx)
{
    TM_UNREF_IN_ARG(dsa);
    return tm_dsa_map_bn_mod_exp_mont(dsa, r, a, p, m, ctx, m_ctx);
}
int tfDsaMapInit            (ttDSAPtr dsa)
{
    return tm_dsa_map_init(dsa);
}
int tfDsaMapFinish          (ttDSAPtr dsa)
{
    return tm_dsa_map_finish(dsa);
}
#endif /* TM_PUBKEY_USE_DSA */
#ifdef TM_PUBKEY_USE_RSA
/* Common mapped RSA functions */
int tfRsaMapPublicEncrypt   (int flen, 
                             tt8BitPtr from, 
                             tt8BitPtr to, 
                             ttRSAPtr rsa, 
                             int padding )
{
    return tm_rsa_map_public_encrypt(flen, from, to, rsa, padding);
}
int tfRsaMapPublicDecrypt   (int flen, 
                             tt8BitPtr from, 
                             tt8BitPtr to, 
                             ttRSAPtr rsa, 
                             int padding)
{
    return tm_rsa_map_public_decrypt(flen, from, to, rsa, padding);
}
int tfRsaMapPrivateEncrypt  (int flen, 
                             tt8BitPtr from, 
                             tt8BitPtr to, 
                             ttRSAPtr rsa, 
                             int padding)
{
    return tm_rsa_map_private_encrypt(flen, from, to, rsa, padding);
}
int tfRsaMapPrivateDecrypt  (int flen, 
                             tt8BitPtr from, 
                             tt8BitPtr to, 
                             ttRSAPtr rsa, 
                             int padding)
{
    return tm_rsa_map_private_decrypt(flen, from, to, rsa, padding);
}
int tfRsaMapModExp          (ttBIGNUMBERPtr r0, 
                             ttBIGNUMBERPtr I, 
                             ttRSAPtr rsa)
{
    return tm_rsa_map_mod_exp(r0, I, rsa);
}
int tfRsaMapBnModExpMont    (ttBIGNUMBERPtr r, 
                             ttBIGNUMBERPtr a, 
                             ttConstBIGNUMBERPtr p,
                             ttConstBIGNUMBERPtr m, 
                             ttBN_CTXPtr ctx,
                             ttBN_MONT_CTXPtr m_ctx)
{
    return tm_rsa_map_bn_mod_exp_mont(r, a, p, m, ctx, m_ctx);
}
int tfRsaMapInit            (ttRSAPtr rsa)
{
    return tm_rsa_map_init(rsa);
}
int tfRsaMapFinish          (ttRSAPtr rsa)
{
    return tm_rsa_map_finish(rsa);
}
#endif /* TM_PUBKEY_USE_RSA */
#ifdef TM_PUBKEY_USE_DIFFIEHELLMAN
/* Common  mapped Diffie-Hellman function */
int tfDhMapBnModExpMont     (ttDhInBnPtr dh,
                             ttBIGNUMBERPtr r, 
                             ttBIGNUMBERPtr a, 
                             ttConstBIGNUMBERPtr p,
                             ttConstBIGNUMBERPtr m, 
                             ttBN_CTXPtr ctx,
                             ttBN_MONT_CTXPtr m_ctx)
{
    TM_UNREF_IN_ARG(dh);
    return tm_dh_map_bn_mod_exp_mont(dh, r, a, p, m, ctx, m_ctx);
}
#endif /* TM_PUBKEY_USE_DIFFIEHELLMAN */
/* Common mapped Random Number Generator function */
void tfMapGetRandomBytes    (tt8BitPtr rPtr,
                             int size)
{
    tm_map_get_random_bytes(rPtr, size);
}
#endif /* TM_USE_MAPPED_CRYLIB */
