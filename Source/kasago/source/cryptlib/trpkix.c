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
 * Description: PKI
 *
 * Filename: trpkix.c
 * Author: Jin Shang
 * Date Created: 5/20/2002
 * $Source: source/cryptlib/trpkix.c $
 *
 * Modification History
 * $Revision: 6.0.2.17 $
 * $Date: 2012/11/08 05:48:25JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>
#include <trcrylib.h>
#ifdef TM_USE_PKI
#include <trpki.h>

#if defined(TM_USE_RAM_FS) || defined(TM_USE_DOS_FS) || defined(TM_USE_EMU_UNIX_FS)
#define TM_PKI_USE_FILE
#endif /* defined(TM_USE_RAM_FS) || defined(TM_USE_DOS_FS) */

static int tfX509CertTransfer(tt8BitPtr           certDerPtr,
                              ttPkiCertListPtr    certPtr   );

static int tfX509CrlTransfer (tt8BitPtr          crlDerPtr,
                              ttPkiCertListPtr   crlPtr   );

#ifdef TM_PUBKEY_USE_DSA
static void tfPkiGeneralDsaFree(ttGeneralDsaPtr dsaPtr);
#endif /* TM_PUBKEY_USE_DSA */

static int tfPkiCertificateClearLocked(int    certType);

static int tfPkiCheckCertSerial( ttX509RevokedCertPtr revokedCertPtr,
                                 ttPkiCertListPtr     certListPtr );

static int tfPkiLockedCheckRevokedCert( ttPkiCertListPtr certListPtr );

static int tfPkiLockedRevokedCertByList ( ttPkiCertListPtr  crlListPtr,
                                          ttX509CertListPtr crlTxtPtr,
                                          int               certType );

static int tfPkiCertCommonNameCompare(ttConstCharPtr myCommonName,
                                      ttConstCharPtr certCommonName);
#ifdef TM_USE_IKEV2
static int tfPkiBuildSubPubkeyInfoHash(ttX509SubPubKeyInfoPtr subPkPtr,
                                       tt8BitPtr              outHashPtr);
#endif /* TM_USE_IKEV2 */

static ttPkiCertListPtrPtr tfPkiCertTypeToList(int certType);

static int tfPkiOwnKeyPairDeleteLocked(void);

/* FUNCTION: tfUsePki
 *
 * PURPOSE:
 *  allocation initilization
 * PARAMETERS:
 *
 * RETURNS:
 *   TM_ENOERROR:
 *       Successful.
 *   TM_ENOBUFS:
 *       no bufs
 *   TM_EALREADY:
 *       already allocated
 */

int tfUsePki(void)
{
    int     errorCode;

    errorCode = TM_ENOERROR;

    if(tm_context(tvPkiPtr) != (ttVoidPtr)0)
    {
/* tm_context(tvPkiPtr) already allocated */
        errorCode = TM_EALREADY;
        goto pkiInitExit;
    }
    else
    {
        tm_context(tvPkiPtr) =
            (ttVoidPtr) tm_get_raw_buffer( sizeof(ttPkiGlobal));
        if(!tm_context(tvPkiPtr))
        {
            errorCode = TM_ENOBUFS;
            goto pkiInitExit;
        }
    }

/* initilize 0 */
    tm_bzero(tm_context(tvPkiPtr), sizeof(ttPkiGlobal));
/* initilize objects as algorithm objects */
    (void)tfX509ObjectInit();
#ifdef TM_PKI_CERT_CHECK_ALIVE
    ((ttPkiGlobalPtr)(tm_context(tvPkiPtr)))->pkiCertCheckAlive = 1;
#endif /* TM_PKI_CERT_CHECK_ALIVE */

#ifdef TM_PKI_CERT_NOT_VERIFY
    ((ttPkiGlobalPtr)(tm_context(tvPkiPtr)))->pkiCertNotVerify = 1;
#endif /* TTM_PKI_CERT_NOT_VERIFY */

    errorCode = tfCryptoEngineInit();

pkiInitExit:
    return errorCode;
}


/* Set PKI options
 *                   TM_PKIOPT_PKICERTCHECKALIVE        1
 *                   TM_PKIOPT_PKICERTNONVERIFY         2
 */
int tfPkiSetOptions(ttUser32Bit       option,
                    ttUser32Bit       value)
{
/* When TM_USE_PKI is defined, we are going to send PKI authentication
 * as proposed method. Set this option to be zero if we still want to
 * propose Preshared key method even if we define TM_USE_PKI.
 */
    int        errorCode;

    tm_call_lock_wait(&(tm_context(tvPkiLockEntry)));
    if (tm_context(tvPkiPtr))
    {
        errorCode = TM_ENOERROR;
        switch(option)
        {
        case TM_PKIOPT_PKICERTCHECKALIVE:
            ((ttPkiGlobalPtr)(tm_context(tvPkiPtr)))->pkiCertCheckAlive
                           = (tt8Bit)value;
            break;
        case TM_PKIOPT_PKICERTNONVERIFY:
            ((ttPkiGlobalPtr)(tm_context(tvPkiPtr)))->pkiCertNotVerify
                           = (tt8Bit)value;
            break;
        case TM_PKIOPT_PKIMAXKEYSIZE:
            ((ttPkiGlobalPtr)(tm_context(tvPkiPtr)))->pkiMaxKeySize
                           = (tt16Bit)value;
            break;
        default:
            errorCode = TM_EINVAL;
            break;
        }
    }
    else
    {
        errorCode = TM_ENOENT;
    }
    tm_call_unlock(&(tm_context(tvPkiLockEntry)));
    return errorCode;
}

/* un-initialize PKI */
void tfPkiUninitialize( void )
{
/*If IPsec/IKE, or SSL is still running, you can not close PKI
 * once you start them
 */
#if (defined(TM_USE_IPSEC) || defined(TM_USE_SSL_SERVER) || \
     defined(TM_USE_SSL_CLIENT))
    if(
#ifdef TM_USE_IPSEC
        (tm_context(tvIpsecPtr) != (ttVoidPtr)0)
#endif /* TM_USE_IPSEC */
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
#ifdef TM_USE_IPSEC
        ||
#endif /* TM_USE_IPSEC */
        (tm_context(tvSslTlsPtr) != (ttVoidPtr)0)
#endif /* TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */
        )
    {
/* don't do anything */
        goto PkiUninitializeExit;
    }
#endif /* TM_USE_SSL.. || TM_USE_ISPEC */

    tm_call_lock_wait( &(tm_context(tvPkiLockEntry)) );
    if(tm_context(tvPkiPtr))
    {
        (void)tfPkiCertificateClearLocked( TM_PKI_CERT_LOCAL );
        (void)tfPkiCertificateClearLocked( TM_PKI_CERT_NONLOCAL );
        (void)tfPkiCertificateClearLocked( TM_PKI_CERT_CRL);
        (void)tfPkiOwnKeyPairDeleteLocked();
        (void)tfX509ObjectFree();
        tm_free_raw_buffer(tm_context(tvPkiPtr));
        tm_context(tvPkiPtr) = 0;

        tfCryptoEngineUninitialize();
    }
    tm_call_unlock( &(tm_context(tvPkiLockEntry)) );
/* free object id and dec */
#if (defined(TM_USE_IPSEC) || defined(TM_USE_SSL_SERVER) || \
     defined(TM_USE_SSL_CLIENT))
PkiUninitializeExit:
#endif /* IPSEC || SSL */
    return;
}

/* FUNCTION: tfPkiCertificateDelete
 *
 * PURPOSE:
 *  delete a certificate by its id
 * PARAMETERS:
 *  idPtr   : pointer to id
 *  idLength: length of id
 *  certType: certificate type - LOCAL, NONLOCAL or CRL
 * RETURNS:
 *   TM_ENOERROR:
 *       Successful.
 *   TM_EINVAL:
 *       incorrect parameter
 */
int tfPkiCertificateDelete (ttUserConstVoidPtr   idPtr,
                            int                  idLength,
                            int                  certType)
{
    ttPkiCertListPtr        tmpCertListPtr;
    ttPkiCertListPtr        prevCertListPtr;
    ttPkiCertListPtrPtr     hdrPtrPtr;
    int                     result;
    int                     errorCode;

    hdrPtrPtr = 0;

    errorCode = TM_EINVAL;
    tm_call_lock_wait(&(tm_context(tvPkiLockEntry)));

    if (idLength < 0)
    {
        goto DELETERETURN;
    }

    if(!tm_context(tvPkiPtr))
    {
/* user didn't call tfUsePki before, exit */
        goto DELETERETURN;
    }

    hdrPtrPtr = tfPkiCertTypeToList(certType);
    if (hdrPtrPtr == (ttPkiCertListPtrPtr)0)
    {
        goto DELETERETURN;
    }
    tmpCertListPtr = *hdrPtrPtr;
    prevCertListPtr = (ttPkiCertListPtr)0;

    while ( tmpCertListPtr != (ttVoidPtr)0)
    {
        if ((tt16Bit)idLength != tmpCertListPtr->certIdLength)
        {
            result = -1;
        }
        else
        {
            result = tm_memcmp(tmpCertListPtr->certIdPtr,
                               idPtr,
                               tmpCertListPtr->certIdLength);
        }
        if ( result == 0 )
        {
/* Some other applications, such as IKE, SSL are checking out this
 * certificate, you can not delete it right now
 */
            if(tmpCertListPtr->certCheckOut)
            {
                goto DELETERETURN; /* should be EPERM */
            }

/* find the certicate we want to delete */
            tfPkiCertCleanup(tmpCertListPtr);
/* if the deleted node is header */
            if ( tmpCertListPtr == *hdrPtrPtr)
            {
                *hdrPtrPtr = (*hdrPtrPtr)->certLinkNextPtr;
            }
            else
            {
                prevCertListPtr->certLinkNextPtr
                        = tmpCertListPtr->certLinkNextPtr;
            }
            tm_free_raw_buffer((tt8BitPtr)tmpCertListPtr);


            errorCode = TM_ENOERROR;
            break;
        }
        else
        {
            prevCertListPtr = tmpCertListPtr;
            tmpCertListPtr = tmpCertListPtr->certLinkNextPtr;
        }
    }

DELETERETURN:
    tm_call_unlock(&(tm_context(tvPkiLockEntry)));
    return errorCode;
}

int tfPkiCertificateDeleteByHandle(ttCertHandle certHandle,
                                   int          certType)
{
    ttPkiCertListPtr        certListPtr;
    ttPkiCertListPtr        prevCertListPtr;
    ttPkiCertListPtrPtr     hdrPtrPtr;
    int                     errorCode;

    errorCode = TM_ENOERROR;

    if (certHandle != (ttCertHandle)0)
    {
        if (tm_context(tvPkiPtr) != 0)
        {
            tm_call_lock_wait(&(tm_context(tvPkiLockEntry)));
            hdrPtrPtr = tfPkiCertTypeToList(certType);
            if (hdrPtrPtr != (ttPkiCertListPtrPtr)0)
            {
                certListPtr = *hdrPtrPtr;
                prevCertListPtr = certListPtr;
                while (certListPtr != (ttPkiCertListPtr)0)
                {
                    if ((ttPkiCertListPtr)certHandle == certListPtr)
                    {
                        /* Found */
                        break;
                    }
                    prevCertListPtr = certListPtr;
                    certListPtr = certListPtr->certLinkNextPtr;
                }
                if (certListPtr != (ttPkiCertListPtr)0)
                {
                    /* Remove the certificate form list */
                    if (certListPtr->certCheckOut == 0)
                    {
                        tfPkiCertCleanup(certListPtr);
                        if (certListPtr == prevCertListPtr)
                        {
                            /* Head of the list */
                            *hdrPtrPtr = (*hdrPtrPtr)->certLinkNextPtr;
                        }
                        else
                        {
                            prevCertListPtr->certLinkNextPtr =
                                certListPtr->certLinkNextPtr;
                        }
                        tm_free_raw_buffer((tt8BitPtr)certListPtr);
                    }
                    else
                    {
                        /* Cannot delete it since owner count is not zero */
                        errorCode = TM_EPERM;
                    }
                }
                else
                {
                    /* Not found */
                    errorCode = TM_ENOENT;
                }
            }
            else
            {
                /* certType is invalid */
                errorCode = TM_EINVAL;
            }
            tm_call_unlock(&(tm_context(tvPkiLockEntry)));
        }
        else
        {
            /* PKI has not been initialized */
            errorCode = TM_EACCES;
        }   
    }
    else
    {
        /* Invalid arugument */
        errorCode = TM_EINVAL;
    }

    return errorCode;
}

/*
 * tfPkiCertificateClear function description
 * Lock PKI
 * call tfPkiCertificateClearLocked
 * UnLock PKI
 */
int tfPkiCertificateClear ( int    certType)
{
    int errorCode;

    tm_call_lock_wait( &(tm_context(tvPkiLockEntry)) );
    errorCode = tfPkiCertificateClearLocked(certType);
    tm_call_unlock( &(tm_context(tvPkiLockEntry)) );
    return errorCode;
}

/* FUNCTION: tfPkiCertificateClearLocked
 *
 * PURPOSE:
 *  clear a kind of certificates
 * PARAMETERS:
 *  certType: certificate type - LOCAL, NONLOCAL or CRL
 * RETURNS:
 *   TM_ENOERROR:
 *       Successful.
 */
static int tfPkiCertificateClearLocked ( int    certType)
{
    ttPkiCertListPtr        tmpCertListPtr;
    ttPkiCertListPtrPtr     hdrPtrPtr;
    int                     errorCode;

    errorCode = TM_EINVAL;
    hdrPtrPtr = 0;
    if(!tm_context(tvPkiPtr))
    {
/* user didn't call tfUsePki before, exit */
        goto pkiClearCertificateExit;
    }
    hdrPtrPtr = tfPkiCertTypeToList(certType);
    if (hdrPtrPtr == (ttPkiCertListPtrPtr)0)
    {
        goto pkiClearCertificateExit;
    }

    errorCode = TM_ENOERROR;
    while ( *hdrPtrPtr != (ttVoidPtr)0)
    {
        tfPkiCertCleanup(*hdrPtrPtr);
        tmpCertListPtr = *hdrPtrPtr;
        *hdrPtrPtr = (*hdrPtrPtr)->certLinkNextPtr;
        tm_free_raw_buffer((tt8BitPtr)tmpCertListPtr);
    }
pkiClearCertificateExit:
    return errorCode;
}

/*
 * The certificate is compared with the serial number of CRL. 
 * It certificate revoke when matching it.
 * Details of argument is following.
 *     revokedCertPtr - One of revocation list
 *     certListPtr - Certificate data
 * When the certificate revoke, TM_CERTFLAG_REVOKED is returned. 
 */
static int tfPkiCheckCertSerial( ttX509RevokedCertPtr revokedCertPtr,
                                 ttPkiCertListPtr     certListPtr )
{
    int compVal;
    int certFlag;

    certFlag = 0;
    /* Check serial number length */
    if( revokedCertPtr->revUserCertPtr->asn1Length == 
        *((tt8BitPtr)certListPtr->certSerNumPtr + 1) )
    {
        /* Compare serial number */
        compVal = tm_memcmp( 
                        ((tt8BitPtr)
                            certListPtr->certSerNumPtr + 2 ),
                        revokedCertPtr->revUserCertPtr->
                            asn1Data,
                        revokedCertPtr->revUserCertPtr->
                            asn1Length);
        if ( compVal == 0)
        {
            certListPtr->certRevoked = TM_8BIT_YES;
            certFlag = TM_CERTFLAG_REVOKED;
        }
    }
    return certFlag;
}
                   
/*
 * Whether the input certificate is effective is confirmed.
 * If CRL that the CA of the certificate issued is registered,
 * the content of CRL is confirmed.
 * The list of the Serial number is included in CRL.
 * The certificate with the same Serial number revoked already.
 * The certificate to which the Serial number is corresponding is revoked.
 * This function's processing is following.
 * 1. CRL of the Der format is taken out of the list,
 *    and the ASN1 decipherment is done.
 * 2. Issur of the certificate is compared with Issur of CRL. 
 * 3. The serial number of CRL is compared with the serial number 
 *    in the certificate when matching of issur. 
 * 4. When the serial number is corresponding, the certRevoked flag is set to 
 *    certListPtr, and TM_CERTFLAG_REVOKED is set to the return value. 
 * 5. When the certificate revoked, processing is ended. 
 */
static int tfPkiLockedCheckRevokedCert( ttPkiCertListPtr certListPtr )
{
    ttX509CertListPtr       crlTxtPtr;
    ttPkiCertListPtr        crlListPtr;
    tt8BitPtr               tmpDerPtr;
    ttX509RevokedCertPtr    revokedCertPtr;
    int                     length;
    int                     retCode;
    int                     errorCode;
    int                     i;

    length = 0;
    retCode = TM_ENOERROR;

    /* Get CRL List Head */
    crlListPtr = (ttPkiCertListPtr)((ttPkiGlobalPtr)
                     tm_context(tvPkiPtr))->
                         pkiCertListHdrPtrArr[TM_PKI_CERT_CRL_INDX];

    /* All registered CRL is confirmed */
    while( crlListPtr != 0 )
    {
        crlTxtPtr = tfX509CRLNew();
        if( crlTxtPtr == 0 )
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfPkiLockedCheckRevokedCert", "allocation error!\n");
#endif /* TM_ERROR_CHECKING */
#ifdef TM_LOG_CERT_WARNING
            tfEnhancedLogWrite(
                TM_LOG_MODULE_CERT,
                TM_LOG_LEVEL_WARNING,
                "tfPkiLockedCheckRevokedCert: "\
                "error decoding certificate no buffer");
#endif /* TM_LOG_CERT_WARNING */
            retCode |= TM_CERTFLAG_BADCERT;
            goto lockedCheckRevokedCert;
        }

        /* 1. Decode DER(ASN1) format */
        tmpDerPtr = crlListPtr->certContentPtr;
        length = tm_x509_generic_dec (  crlTxtPtr, 
                                        (tt8BitPtrPtr)&tmpDerPtr,
                                        (ttIntPtr)&length,
                                        TM_X509_CERTLIST);
        /* 2. compare Issur */
        if (crlListPtr->certDNLength == certListPtr->certDNLength)
        {
            errorCode = tm_memcmp(certListPtr->certDNPtr, 
                                  crlListPtr->certDNPtr,
                                  crlListPtr->certDNLength);
            if ( errorCode == 0 )
            {
                for ( i = 0 ; i < TM_PKI_MAX_STACK; i++)
                {
                    revokedCertPtr = 
                        crlTxtPtr->crlTbsCertListPtr->tclRevokedPtr[i];

                    /*
                     * 3. compare serial number 
                     * 4. certRevoked flag is set and TM_CERTFLAG_REVOKED
                     *    is set to the return value
                     */
                    retCode = 
                        tfPkiCheckCertSerial( revokedCertPtr,certListPtr );
                    if( retCode != TM_ENOERROR )
                    {
                        /* certificate is revoked */
                        break;
                    }
                }
            }
        }

        tfX509CRLFree( crlTxtPtr );

        /* 5. When the certificate revoked, processing is ended */
        if( certListPtr->certRevoked == TM_8BIT_YES )
        {
            /* Stop checking sence we found out certificate is revoked */
            break;
        }
        
        crlListPtr = crlListPtr->certLinkNextPtr;
    }

lockedCheckRevokedCert:

    return retCode;
}

/*
 * The certificate that matches to the serial number of input CRL is revoked.
 * crlTxtPtr is data that has already done DER decoding.
 * certType is a retrieved certificate type.
 * TM_PKI_CERT_LOCAL or TM_PKI_CERT_NONLOCAL is specified for certType.
 * 1. The certificate of specified certType is compared
 *    with the content of CRL.
 * 1.1 The certificate that has already revoked is not checked.
 * 1.2 The certificate is compared with Issur of CRL.
 * 1.3. compare serial number 
 * 1.4. certRevoked flag is set and TM_CERTFLAG_REVOKED
 *      is set to the return value
 */
static int tfPkiLockedRevokedCertByList ( ttPkiCertListPtr  crlListPtr,
                                          ttX509CertListPtr crlTxtPtr,
                                          int               certType )
{
    int i;
    int retCode;
    int compVal;
    ttPkiCertListPtr    hdrPtr;
    ttX509RevokedCertPtr revokedCertPtr;

    retCode = TM_ENOERROR;

    for ( i = 0 ; i < TM_PKI_MAX_STACK; i++)
    {
        if ( crlTxtPtr->crlTbsCertListPtr
                 ->tclRevokedPtr[i]->revUserCertPtr != 0)
        {
            revokedCertPtr = crlTxtPtr->crlTbsCertListPtr->tclRevokedPtr[i];

            hdrPtr = ((ttPkiGlobalPtr)
                        tm_context(tvPkiPtr))->
                            pkiCertListHdrPtrArr[certType];
            /*
             * 1. The certificate of specified certType is compared
             *    with the content of CRL.
             */
            while ( hdrPtr != (ttVoidPtr)0)
            {
                /*
                 * 1.1 The certificate that has already revoked is 
                 *     not checked. 
                 */
                if ( hdrPtr->certRevoked == TM_8BIT_NO )
                {
                    /* 1.2 The certificate is compared with Issur of CRL. */
                    if (crlListPtr->certDNLength == (int)hdrPtr->certDNLength)
                    {
                        compVal = tm_memcmp(hdrPtr->certDNPtr, 
                                              crlListPtr->certDNPtr,
                                              crlListPtr->certDNLength);

                        if ( compVal == 0 )
                        {
                            /*
                             * 1.3. compare serial number 
                             * 1.4. certRevoked flag is set and 
                             *      TM_CERTFLAG_REVOKED is set to 
                             *      the return value
                             */
                            retCode = tfPkiCheckCertSerial( 
                                                  revokedCertPtr,
                                                  hdrPtr );

                            if ( retCode != 0 )
                            {
                                /* certificate is revoked */
                                break;
                            }
                        }
                    }
                }
                hdrPtr = hdrPtr->certLinkNextPtr; 
            }
        }
        else
        {
            break;
        }
    }
    return retCode;
}

/*
 * The reliability of the certificate and CRL is confirmed.
 * It is confirmed that it is a certificate that trusted CA issued.
 * 1.The certificate with corresponding Subject to Issur of the certificate
 *   is retrieved. And, it sets it as a certificate of CA.
 * 2.The signature of the certificate is verified. 
 * 3.If the certificate of CA cannot be retrieved, 
 *   TM_CERTFLAG_UNKNOWNCA is returned. 
 * 4.If reliability cannot be verified, TM_CERTFLAG_NOTVERIFIED is returned.
 * Details of argument is following.
 *     checkListPtr - Certificate that verifies reliability(DER format)
 *     checkTxtPtr - Certificate that verifies reliability
 *                   (already ASN1 decode)
 *     typeHeader - If it is ROOT CA, TM_PKI_ROOT_CA is set. 
 */
int tfPkiCertVerify( ttVoidPtr        checkListPtr,
                     ttVoidPtr        checkTxtPtr,
                     ttConstVoidPtr   caIdPtr,
                     int              typeHeader )
{
    int                 errorCode;
    int                 certFlag;
#if (defined(TM_PUBKEY_USE_RSA) || defined(TM_PUBKEY_USE_DSA))
    ttPkiCertListPtr    checkCertListPtr;
    ttX509CertPtr       checkCertTxtPtr;
    ttVoidPtr           checkCaPtr;
#endif /* TM_PUBKEY_USE_RSA ||  TM_PUBKEY_USE_DSA */

    certFlag = 0;     

    /* If it is ROOT_CA, it doesn't check it. */
    if((typeHeader & TM_PKI_CERT_ROOTCA) != 0)
    {
        goto pkiCertVerifyEnd;
    }

#if (defined(TM_PUBKEY_USE_RSA) || defined(TM_PUBKEY_USE_DSA))
#ifdef TM_PKI_CERT_NOT_VERIFY
/* still need to verify, if set ipsecPkiCertNotVerify as 0 */
    if (((ttPkiGlobalPtr)tm_context(tvPkiPtr))->pkiCertNotVerify == 0)
#endif /* !TM_PKI_CERT_NOT_VERIFY */
    {
        /*
         * 1.The certificate with corresponding Subject to Issur of the 
         *   certificate is retrieved. And, it sets it as a certificate of CA.
         */
        checkCertListPtr = (ttPkiCertListPtr)checkListPtr;
        checkCertTxtPtr = (ttX509CertPtr)checkTxtPtr;
/* should search CA with Issuer's DN  */

        if (caIdPtr != (ttVoidPtr)0)
        {
            checkCertListPtr->certCAPtr = 
                tfPkiCertificateSearch(TM_PKI_CERTSEARCH_ID,    
                                       caIdPtr,
                                       (int) tm_strlen(caIdPtr),
                                       TM_PKI_CERT_OTHER_INDX,
                                       TM_8BIT_NO); /* is locked */
        }
        else
        {
/* should search CA with Issuer's DN  */
            checkCertListPtr->certCAPtr = 
                tfPkiCertificateSearch(TM_PKI_CERTSEARCH_ISSUER,    
                                       checkCertListPtr->certDNPtr,
                                       checkCertListPtr->certDNLength, 
                                       TM_PKI_CERT_OTHER_INDX,
                                       TM_8BIT_NO); /* is locked */
        }
        checkCaPtr = (ttVoidPtr)checkCertListPtr->certCAPtr;
    
        /*
         * 2.If the certificate of CA cannot be retrieved, 
         *   TM_CERTFLAG_UNKNOWNCA is returned. 
         */
        if ( checkCaPtr == 0 )
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfPkiCertificateCheckAdd", "NO CA found");
#endif /* TM_ERROR_CHECKING */
#ifdef TM_LOG_CERT_WARNING
            tfEnhancedLogWrite(
                TM_LOG_MODULE_CERT,
                TM_LOG_LEVEL_WARNING,
                "tfPkiCertificateCheckAdd: "\
                "no certification authority (CA) found");
#endif /* TM_LOG_CERT_WARNING */
            certFlag |= TM_CERTFLAG_UNKNOWNCA;
        }
        else
        {
            /* 
             * 3.The signature of the certificate is verified. 
             */
            errorCode = tfX509CertVerify(
                checkCertListPtr->certContentPtr,
                (ttVoidPtr)checkCertTxtPtr, 
                ((ttPkiCertListPtr)(checkCaPtr))->certKeyPtr );
            /*
             * 4.If reliability cannot be verified, 
             *   TM_CERTFLAG_NOTVERIFIED is returned.
             */
            if ( errorCode != TM_ENOERROR)
            {
/* certificate verify error */
                certFlag |= TM_CERTFLAG_NOTVERIFIED;
#ifdef TM_LOG_CERT_WARNING
            tfEnhancedLogWrite(
                TM_LOG_MODULE_CERT,
                TM_LOG_LEVEL_WARNING,
                "tfPkiCertificateCheckAdd: "\
                "certificate could not be verified");
#endif /* TM_LOG_CERT_WARNING */
            }

        }
    }
#endif /* TM_PUBKEY_USE_RSA ||  TM_PUBKEY_USE_DSA */

pkiCertVerifyEnd:
    return certFlag;
}

/* compare myCommonName and the common name contained in certificate.
 * We will seperate the common name in a few parts, each part
 * will be seperated by character '.'. Each parts will contain at
 * most one wild * character '*'. We compare part by part.
 * Error returns when the following happens:
 * 1) Total part numbers are different
 * 2) Any part doesn't match
 *
 * we don't support *@*.foobar.com format, '.' is the only seperator
 * we suppport, then '*@*' will contain two wild cards.
 * We will also treat "foofoo.com" as a matched example of
 * "foo*bar.com", because anything follows * will be ignored.
 */
static int tfPkiCertCommonNameCompare(ttConstCharPtr myCommonName,
                                      ttConstCharPtr certCommonName)
{
    ttCharPtr myCommonBuffer;
    ttCharPtr certCommonBuffer;
    ttCharPtr myNPtr; /* next part */
    ttCharPtr certNPtr;
    ttCharPtr myCPtr; /* current part */
    ttCharPtr certCPtr;
    ttCharPtr certWildPtr;
    int       retCode;
    int       tempInt;

    retCode = TM_EINVAL;
    myCommonBuffer = tm_get_raw_buffer((ttPktLen)tm_strlen(myCommonName) + 1);
    certCommonBuffer = tm_get_raw_buffer((ttPktLen)tm_strlen(certCommonName) + 1);
    if(!myCommonBuffer || !certCommonBuffer)
    {
        retCode = TM_ENOBUFS;
        goto CommonNameCompareExit;
    }
    tm_strcpy(myCommonBuffer, myCommonName);
    tm_strcpy(certCommonBuffer, certCommonName);

    myNPtr = myCommonBuffer;
    certNPtr = certCommonBuffer;

    while( (myNPtr != (ttCharPtr)0) &&
           (certNPtr != (ttCharPtr)0))
    {
/* Step 1. Search for next seperator '.' */
        myCPtr = myNPtr;
        certCPtr = certNPtr;
        myNPtr = tm_strchr(myNPtr, '.');
        if(myNPtr)
        {
            *(myNPtr ++ ) = '\0';
        }
        certNPtr = tm_strchr(certNPtr, '.');
        if(certNPtr)
        {
            *(certNPtr ++) = '\0';
        }
/* Step 2. Now compare myCPtr and certCPtr to see if they are legal
 * match
 */
        tempInt = tm_strcmp(myCPtr, certCPtr);
        if(tempInt != 0)
        {
            certWildPtr = tm_strchr(certCPtr, '*');
            if(!certWildPtr)
            {
/* no wild card exists. We don't match */
                goto CommonNameCompareExit;
            }
            else
            {
                *certWildPtr = '\0';
                tempInt = tm_memcmp(myCPtr, certCPtr, tm_strlen(certCPtr));
                if(tempInt != 0)
                {
                    goto CommonNameCompareExit;
                }
            }
        }
    }

    if( (myNPtr == (ttCharPtr)0) &&
           (certNPtr == (ttCharPtr)0))
    {
/* if both are NULL, then the comparison is done, and we are satisfied */
        retCode = TM_ENOERROR;
    }

CommonNameCompareExit:
    if(myCommonBuffer)
    {
        tm_free_raw_buffer(myCommonBuffer);
    }

    if(certCommonBuffer)
    {
        tm_free_raw_buffer(certCommonBuffer);
    }
    return retCode;
}

/* Following functions for:
 *   1. Read Certificate File -- tfPkiCertificateCheckAdd:PEM and DER
 *   2. Read Public Key files -- tfPkiPubKeyAdd: PEM and DER
 *   3. Read Key Pair files (openssl) -- tfPkiOwnKeyPairAdd: PEM and DER
 */

/* API: tfPkiCertificateCheckAdd
 * Purpose: to obtain a Certificate from a file or memory pointer
 *             file format: PEM, DER; extracted public key
 * Input: file fileNamePtr+name, file type
 * Output: updated tvPkiPtr
 */
int tfPkiCertificateAdd (ttUserConstVoidPtr  fileNamePtr,
                         int                 typeFormat,
                         int                 typeHeader,
                         ttUserConstVoidPtr  idPtr,
                         int                 idLength,
                         ttUserConstVoidPtr  caIdPtr)
{
    return tfPkiCertificateCheckAdd(
                            (ttPkiCertListPtrPtr)0,
                            (ttCharPtr)0,
                            fileNamePtr,
                            typeFormat,
                            typeHeader,
                            idPtr,
                            idLength,
                            caIdPtr);

}

#ifdef TM_USE_IKEV2
static int tfPkiBuildSubPubkeyInfoHash(ttX509SubPubKeyInfoPtr subPkPtr,
                                       tt8BitPtr              outHashPtr)
{
    ttCryptoRequest           request;
    ttCryptoRequestHash       hashRequest;
    ttCryptoChainData         chainData;
    ttX509SubPubKeyInfo       pubKeyInfo;
    struct tsCryptoEngine *   cryptoEnginePtr;
    tt8BitPtr                 derPtr;
    ttAsn1StringPtr           tmpAsn1StringPtr;
    int                       derLen;
    int                       tempLen;
    int                       errorCode;
    
    errorCode = TM_ENOERROR;
    derPtr = (tt8BitPtr)0;
    tmpAsn1StringPtr = subPkPtr->subPkAlgorPtr->algorParPtr;

    cryptoEnginePtr = tfCryptoEngineGetFirst();
    if (cryptoEnginePtr == (struct tsCryptoEngine *)0)
    {
        errorCode = TM_EOPNOTSUPP;
        goto PKIBUILDSUBPUBKEYINFOHASH_EXIT;
    }

    tm_bcopy(subPkPtr, &pubKeyInfo, sizeof(ttX509SubPubKeyInfo));
    pubKeyInfo.subPkAlgorPtr->algorParPtr = (ttAsn1StringPtr)0;
    /* Get DER length */
    derLen = tm_x509_generic_enc(subPkPtr, 
                                 (tt8BitPtrPtr)0, 
                                 &tempLen,
                                 TM_X509_SUBPKINFO);
    if (derLen == 0)
    {
        errorCode = TM_EINVAL;
        goto PKIBUILDSUBPUBKEYINFOHASH_EXIT;

    }
    derPtr = tm_get_raw_buffer((ttPktLen)derLen);
    if (derPtr == (tt8BitPtr)0)
    {
        errorCode = TM_ENOBUFS;
        goto PKIBUILDSUBPUBKEYINFOHASH_EXIT;
    }
    /* Generate DER from Subject Public Key Info in TBS cert informain */
    (void)tm_x509_generic_enc(subPkPtr,
                              &derPtr,
                              &tempLen,
                              TM_X509_SUBPKINFO);
    derPtr -= tempLen;
    /*
     * Make a SHA-1 hash of Subject Public Key Info
     */
    tm_bzero(&request, sizeof(request));
    request.crType = TM_CEREQUEST_CRYPTO_HASH;
    request.crParamUnion.crHashParamPtr = &hashRequest;
    hashRequest.crhAlgPtr = (ttAhAlgorithmPtr)0;
    hashRequest.crhHashAlg = SADB_AALG_SHA1HMAC;
    hashRequest.crhOutPtr = outHashPtr;
    hashRequest.crhInDataPtr = &chainData;
    chainData.ccdDataPtr = derPtr;
    chainData.ccdDataLen = (ttPktLen)derLen;
    chainData.ccdNextPtr = (ttCryptoChainDataPtr)0;
    hashRequest.crhDataLength = (ttUser16Bit)chainData.ccdDataLen;

    errorCode = cryptoEnginePtr->ceSessionProcessFuncPtr(&request);

PKIBUILDSUBPUBKEYINFOHASH_EXIT:
    subPkPtr->subPkAlgorPtr->algorParPtr = tmpAsn1StringPtr;
    if (derPtr != (tt8BitPtr)0)
    {
        tm_free_raw_buffer(derPtr);
    }
    return errorCode;
}
#endif /* TM_USE_IKEV2 */

/* tfPkiCertificateCheckAdd returns a bitwised error information, if
 * no bit is set, then no error happens. If the first bit is set,
 * i.e. TM_CERTFLAG_BADCERT is set, it means the certificate is
 * invalid.
 * TM_CERTFLAG_BADCERT      0x01  critical error
 * TM_CERTFLAG_REVOKED      0x02  revoked cert
 * TM_CERTFLAG_EXPIRED      0x04  expired cert
 * TM_CERTFLAG_UNMATCHED    0x08  identity not matched
 * TM_CERTFLAG_UNKNOWNCA    0x10  ca unknown
 *
 */
int tfPkiCertificateCheckAdd(
                            ttPkiCertListPtrPtr retCertPtrPtr,
                            ttCharPtr           commonName,
                            ttUserConstVoidPtr  fileNamePtr,
                            int                 typeFormat,
                            int                 typeHeader,
                            ttUserConstVoidPtr  idPtr,
                            int                 idLength,
                            ttUserConstVoidPtr  caIdPtr)
{
    tt8BitPtr               certStrPtr;
    tt8BitPtr               certDerPtr;
    tt8BitPtr               tmpDerPtr;
    ttX509CertPtr           certTxtPtr;
    ttX509CertListPtr       crlTxtPtr;
    ttPkiCertListPtrPtr     hdrCertListPtrPtr;
    ttPkiCertListPtr        tmpCertListPtr;
    ttPkiCertListPtr        tmpCrlListPtr;
    ttX509AttrTypeValuePtr  nextAttrValuePtr;
#if (defined(TM_PUBKEY_USE_RSA) || defined(TM_PUBKEY_USE_DSA))
    ttX509ExtensionPtr      nextExtValuePtr;
#endif /* TM_PUBKEY_USE_RSA || TM_PUBKEY_USE_DSA */
#ifdef TM_DEBUG_LOGGING
    ttPkiCertListPtr        tempCertPtr;
#endif /* TM_DEBUG_LOGGING */
    ttCharPtr               nullChrCheckResult;
#if (defined(TM_PUBKEY_USE_RSA) || defined(TM_PUBKEY_USE_DSA))
    tt8BitPtr               temp8BitPtr;
#endif /* TM_PUBKEY_USE_RSA || TM_PUBKEY_USE_DSA */
    tt16Bit                 temp16;
    int                     length;
#ifdef TM_USE_IKEV2
    int                     errorCode;
#endif /* TM_USE_IKEV2 */
    int                     i;
    int                     pkType;
    int                     retCode;
    int                     checkCode;
    int                     tempInt;
#ifndef TM_PKI_BCCRITICAL_CHECK_DISABLE
    tt8Bit                  foundBcField;
#endif /* #ifndef TM_PKI_BCCRITICAL_CHECK_DISABLE */

    tm_trace_func_entry(tfPkiCertificateCheckAdd);

/* obtain PEM or DER format from file */
    length      = 0;
    certStrPtr  = 0;
#ifdef TM_USE_IKEV2
    errorCode   = TM_ENOERROR;
#endif /* TM_USE_IKEV2 */
    retCode     = 0;
    certTxtPtr  = (ttX509CertPtr)0;
    crlTxtPtr   = (ttX509CertListPtr)0;
    pkType      = 0;
    tmpCertListPtr      = (ttPkiCertListPtr)0;
#ifndef TM_PKI_BCCRITICAL_CHECK_DISABLE
    foundBcField = TM_8BIT_NO;
#endif /* #ifndef TM_PKI_BCCRITICAL_CHECK_DISABLE */
    tempInt     = 0;

    tm_call_lock_wait( &(tm_context(tvPkiLockEntry)) );
    if(!tm_context(tvPkiPtr))
    {
/* user didn't call tfUsePki before, exit */
        retCode |= TM_CERTFLAG_BADCERT;
#ifdef TM_LOG_CERT_WARNING
        tfEnhancedLogWrite(
            TM_LOG_MODULE_CERT,
            TM_LOG_LEVEL_WARNING,
            "tfPkiCertificateCheckAdd: "\
            "tfUsePki() must be called prior to adding a certificate");
#endif /* TM_LOG_CERT_WARNING */
        goto addCertificateExit;
    }
    if (fileNamePtr == (tt8BitPtr)0)
    {
#ifdef TM_LOG_CERT_WARNING
        tfEnhancedLogWrite(
            TM_LOG_MODULE_CERT,
            TM_LOG_LEVEL_WARNING,
            "tfPkiCertificateCheckAdd: "\
            "Null fileNamePtr parameter");
#endif /* TM_LOG_CERT_WARNING */
        retCode |= TM_CERTFLAG_BADCERT;
        goto addCertificateExit;
    }

    if (tm_8bit_bits_not_set(typeFormat, TM_PKI_CERTIFICATE_STRING))
    {
#ifdef TM_PKI_USE_FILE
        certStrPtr = tfPkiGetStringInFile(fileNamePtr,typeFormat,
                                                &temp16);
#else /* !TM_PKI_USE_FILE */
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfPkiCertificateCheckAdd", "NO file I/O");
#endif /* TM_ERROR_CHECKING */
#ifdef TM_LOG_CERT_WARNING
        tfEnhancedLogWrite(
            TM_LOG_MODULE_CERT,
            TM_LOG_LEVEL_WARNING,
            "tfPkiCertificateCheckAdd: "\
            "File I/O error");
#endif /* TM_LOG_CERT_WARNING */
        retCode |= TM_CERTFLAG_BADCERT;
        goto addCertificateExit;
#endif /* TM_PKI_USE_FILE */
    }
    else
    {
        certStrPtr = (tt8BitPtr)fileNamePtr;
    }

    if (certStrPtr == (tt8BitPtr)0)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfPkiCertificateCheckAdd", "string error");
#endif /* TM_ERROR_CHECKING */
#ifdef TM_LOG_CERT_WARNING
        tfEnhancedLogWrite(
            TM_LOG_MODULE_CERT,
            TM_LOG_LEVEL_WARNING,
            "tfPkiCertificateCheckAdd: "\
            "invalid input certificate string");
#endif /* TM_LOG_CERT_WARNING */
        retCode |= TM_CERTFLAG_BADCERT;
        goto addCertificateExit;
    }

    if (tm_8bit_one_bit_set(typeFormat, TM_PKI_CERTIFICATE_PEM))
    {
        certDerPtr = tfX509PemToDer ( certStrPtr, &temp16);
        if ( tm_8bit_bits_not_set(  typeFormat,
                                TM_PKI_CERTIFICATE_STRING))
        {
            tm_free_raw_buffer(certStrPtr);
        }
        if (certDerPtr == (ttVoidPtr)0)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfPkiCertificateCheckAdd", "wrong PEM");
#endif /* TM_ERROR_CHECKING */
#ifdef TM_LOG_CERT_WARNING
        tfEnhancedLogWrite(
            TM_LOG_MODULE_CERT,
            TM_LOG_LEVEL_WARNING,
            "tfPkiCertificateCheckAdd: "\
            "wrong PEM");
#endif /* TM_LOG_CERT_WARNING */
            retCode |= TM_CERTFLAG_BADCERT;
            goto addCertificateExit;
        }
    }
    else if (tm_8bit_one_bit_set(typeFormat, TM_PKI_CERTIFICATE_DER))
    {
/* get buffer for certificate */
        tmpDerPtr = certStrPtr;
        length = tm_asn1_sequence_dec((ttVoidPtr)0, &tmpDerPtr);
        if( length == -1 )
        {
            retCode |= TM_CERTFLAG_BADCERT;
            goto addCertificateExit;
        }
        length += (int)(tmpDerPtr - certStrPtr);
        certDerPtr = tm_get_raw_buffer((ttPktLen)length);
        if( certDerPtr == (ttVoidPtr)0 )
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfPkiCertificateCheckAdd", "no buffer");
#endif /* TM_ERROR_CHECKING */
            retCode |= TM_CERTFLAG_BADCERT;
            if ( tm_8bit_bits_not_set(  typeFormat,
                                        TM_PKI_CERTIFICATE_STRING))
            {
                tm_free_raw_buffer(certStrPtr);
            }
            goto addCertificateExit;
        }
/* copy certificate from user buffer */
        tm_bcopy(certStrPtr, certDerPtr, length );
        if ( tm_8bit_bits_not_set(  typeFormat,
                                    TM_PKI_CERTIFICATE_STRING))
        {
            tm_free_raw_buffer(certStrPtr);
        }
    }
    else
    {
        /* Wrong format (neither PEM nor DER) */
        if (tm_8bit_bits_not_set(typeFormat, TM_PKI_CERTIFICATE_STRING))
        {
            tm_free_raw_buffer(certStrPtr);
        }
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfPkiCertificateCheckAdd", "wrong Format");
#endif /* TM_ERROR_CHECKING */
#ifdef TM_LOG_CERT_WARNING
        tfEnhancedLogWrite(
            TM_LOG_MODULE_CERT,
            TM_LOG_LEVEL_WARNING,
            "tfPkiCertificateCheckAdd: "\
            "unsupported certificate format 0x%x",
            typeFormat);
#endif /* TM_LOG_CERT_WARNING */
        retCode |= TM_CERTFLAG_BADCERT;
        goto addCertificateExit;
    }


/* PEM changed to Txt, Extract Public/Private Key */
/* Right now, support multi IKE id, and multi own Certificate for each ID */
    hdrCertListPtrPtr = tfPkiCertTypeToList(typeHeader);
    if (hdrCertListPtrPtr == (ttPkiCertListPtrPtr)0)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfPkiCertificateCheckAdd", "type header wrong");
#endif /* TM_ERROR_CHECKING */
#ifdef TM_LOG_CERT_WARNING
        tfEnhancedLogWrite(
            TM_LOG_MODULE_CERT,
            TM_LOG_LEVEL_WARNING,
            "tfPkiCertificateCheckAdd: "\
            "invalid type header");
#endif /* TM_LOG_CERT_WARNING */
        retCode |= TM_CERTFLAG_BADCERT;
        goto addCertificateExit;
    }

    tmpCertListPtr = (ttPkiCertListPtr)tm_get_raw_buffer(
        sizeof(ttPkiCertList));
    if ( tmpCertListPtr== (ttVoidPtr)0)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfPkiCertificateCheckAdd", "no buffer");
#endif /* TM_ERROR_CHECKING */
        retCode |= TM_CERTFLAG_BADCERT;
#ifdef TM_LOG_CERT_WARNING
        tfEnhancedLogWrite(
            TM_LOG_MODULE_CERT,
            TM_LOG_LEVEL_WARNING,
            "tfPkiCertificateCheckAdd: "\
            "unable to alocate %d bytes for certificate",
            sizeof(ttPkiCertList));
#endif /* TM_LOG_CERT_WARNING */
        goto addCertificateExit;
    }

    tm_bzero(tmpCertListPtr, sizeof(ttPkiCertList));
    if ( (typeHeader & TM_PKI_CERT_CRL) == 0)
    {
/* for all certificates, find the parameter pointers in certificate */
        if ( (idLength > 0) && (idPtr != (ttUserVoidPtr)0) )
        {
/* that can not be ASN1_DN or ASN1_GN */
            tmpCertListPtr->certIdLength    = (tt16Bit)idLength;
            tmpCertListPtr->certIdPtr = tm_get_raw_buffer((ttPktLen)idLength);
            if ( tmpCertListPtr->certIdPtr == (ttVoidPtr)0)
            {
#ifdef TM_ERROR_CHECKING
                tfKernelWarning("tfPkiCertificateCheckAdd", "out of memory");
#endif /* TM_ERROR_CHECKING */
                retCode |= TM_CERTFLAG_BADCERT;
#ifdef TM_LOG_CERT_WARNING
                tfEnhancedLogWrite(
                    TM_LOG_MODULE_CERT,
                    TM_LOG_LEVEL_WARNING,
                    "tfPkiCertificateCheckAdd: "\
                    "unable to allocate %d bytes for certificate ID",
                    idLength);
#endif /* TM_LOG_CERT_WARNING */
                goto addCertificateExit;
            }
            tm_bcopy(idPtr, tmpCertListPtr->certIdPtr,idLength);
        }
        tmpCertListPtr->certLinkNextPtr = *hdrCertListPtrPtr;

/* initilize certDNPtr, certSubPtr fields */
        (void)tfX509CertTransfer( certDerPtr, tmpCertListPtr);


/* At this moment, certDerPtr still points to the starting point.
 * need to extract public key info, even for selfcert, because
 * we need to extract pub key type
 */
/* DER decoding, not yet verify */

        certTxtPtr = tfX509CertNew();
        if ( certTxtPtr == (ttX509CertPtr)0)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfPkiCertificateCheckAdd", "allocation error!\n");
#endif /* TM_ERROR_CHECKING */
#ifdef TM_LOG_CERT_WARNING
            tfEnhancedLogWrite(
                TM_LOG_MODULE_CERT,
                TM_LOG_LEVEL_WARNING,
                "tfPkiCertificateCheckAdd: "\
                "error decoding certificate no buffer");
#endif /* TM_LOG_CERT_WARNING */
            retCode |= TM_CERTFLAG_BADCERT;
            goto addCertificateExit;
        }
        length = tm_x509_generic_dec (  certTxtPtr,
                                        (tt8BitPtrPtr)&certDerPtr,
                                        (ttIntPtr)&length,
                                        TM_X509_CERT);

        tmpCertListPtr->certLength = (tt16Bit)length;

        if ( length <0)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfPkiCertificateCheckAdd", "decode error!\n");
#endif /* TM_ERROR_CHECKING */
#ifdef TM_LOG_CERT_WARNING
            tfEnhancedLogWrite(
                TM_LOG_MODULE_CERT,
                TM_LOG_LEVEL_WARNING,
                "tfPkiCertificateCheckAdd: "\
                "error decoding certificate");
#endif /* TM_LOG_CERT_WARNING */
            retCode |= TM_CERTFLAG_BADCERT;
            goto addCertificateExit;
        }
        if (typeHeader & TM_PKI_CERT_NONLOCAL)
        {
            tmpCertListPtr->certKeyPtr = tfX509PubKeyInfoGet(
                                    certTxtPtr->certTbsPtr->tbsSubPkInfoPtr);
            if ( tmpCertListPtr->certKeyPtr == (ttVoidPtr)0)
            {
#ifdef TM_ERROR_CHECKING
                tfKernelWarning("tfPkiCertificateCheckAdd", "pubkey error!\n");
#endif /* TM_ERROR_CHECKING */
                retCode |= TM_CERTFLAG_BADCERT;
#ifdef TM_LOG_CERT_WARNING
                tfEnhancedLogWrite(
                    TM_LOG_MODULE_CERT,
                    TM_LOG_LEVEL_WARNING,
                    "tfPkiCertificateCheckAdd: "\
                    "error getting info from public key");
#endif /* TM_LOG_CERT_WARNING */
                goto addCertificateExit;
            }
#ifdef TM_USE_IKEV2
            errorCode = tfPkiBuildSubPubkeyInfoHash(
                certTxtPtr->certTbsPtr->tbsSubPkInfoPtr,
                &tmpCertListPtr->certSubPubkeyInfoHash[0]);
            if (errorCode != TM_ENOERROR)
            {
                retCode |= TM_CERTFLAG_BADCERT;
                goto addCertificateExit;
            }
#endif /* TM_USE_IKEV2 */
        }
        else
        {
/* find the key type of own */
            pkType = tfX509GetObjId((tt8BitPtr)certTxtPtr->certTbsPtr
                            ->tbsSubPkInfoPtr->subPkAlgorPtr->algorObjPtr);
        }

        retCode = tfPkiCertVerify( (ttVoidPtr)tmpCertListPtr,
                                   (ttVoidPtr)certTxtPtr,
                                   caIdPtr,
                                   typeHeader );

/* revocation check of this certificate */
        checkCode = tfPkiLockedCheckRevokedCert( tmpCertListPtr );
        retCode |= checkCode;
    }
    else
    {
        /* CRL */
        tmpCrlListPtr = tmpCertListPtr;
/*
 * Initilize crlContentPtr so that we can free crlDerPtr when CRL is deleted
 */
        tmpCrlListPtr->certContentPtr = certDerPtr;

         (void)tfX509CrlTransfer( certDerPtr, tmpCrlListPtr);
/* CRL, right now did not verify CRL */

/* for all CRL, find the parameter pointers in CRL */
        if ( (idLength > 0) && (idPtr != (ttUserVoidPtr)0) )
        {
/* that can not be ASN1_DN or ASN1_GN */
            tmpCrlListPtr->certIdLength = (tt16Bit)idLength;
            tmpCrlListPtr->certIdPtr = tm_get_raw_buffer((ttPktLen)idLength);
            if ( tmpCrlListPtr->certIdPtr == (ttVoidPtr)0)
            {
#ifdef TM_ERROR_CHECKING
                tfKernelWarning("tfPkiCertificateCheckAdd", "out of memory");
#endif /* TM_ERROR_CHECKING */
                retCode |= TM_CERTFLAG_BADCERT;
                goto addCertificateExit;
            }
            tm_bcopy(idPtr, tmpCrlListPtr->certIdPtr,idLength);
        }
        
        tmpCrlListPtr->certLinkNextPtr = 
            (struct tsPkiCertListSt *)*hdrCertListPtrPtr;
/* DER decoding, not yet verify */
        crlTxtPtr = tfX509CRLNew();
        if (crlTxtPtr == (ttX509CertListPtr)0)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfPkiCertificateCheckAdd", "allocation error!\n");
#endif /* TM_ERROR_CHECKING */
#ifdef TM_LOG_CERT_WARNING
            tfEnhancedLogWrite(
                TM_LOG_MODULE_CERT,
                TM_LOG_LEVEL_WARNING,
                "tfPkiCertificateCheckAdd: "\
                "error decoding certificate no buffer");
#endif /* TM_LOG_CERT_WARNING */
            retCode |= TM_CERTFLAG_BADCERT;
            goto addCertificateExit;
        }
        length = tm_x509_generic_dec (  crlTxtPtr,
                                        (tt8BitPtrPtr)&certDerPtr,
                                        (ttIntPtr)&length,
                                        TM_X509_CERTLIST);
        tmpCertListPtr->certLength = (tt16Bit)length;
        if ( length <0)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfPkiCertificateCheckAdd", "decode error!\n");
#endif /* TM_ERROR_CHECKING */
#ifdef TM_LOG_CERT_WARNING
            tfEnhancedLogWrite(
                TM_LOG_MODULE_CERT,
                TM_LOG_LEVEL_WARNING,
                "tfPkiCertificateCheckAdd: "\
                "error decoding certificate");
#endif /* TM_LOG_CERT_WARNING */
            retCode |= TM_CERTFLAG_BADCERT;
            goto addCertificateExit;
        }

        retCode = tfPkiCertVerify( (ttVoidPtr)tmpCrlListPtr,
                                   (ttVoidPtr)crlTxtPtr,
                                   caIdPtr,
                                   typeHeader );
        if(retCode != 0)
        {
            retCode |= TM_CERTFLAG_BADCERT;
            goto addCertificateExit;
        }
/* match certificate to existing cert list, no verify */
        tfPkiLockedRevokedCertByList ( tmpCrlListPtr,
                                       crlTxtPtr,
                                       TM_PKI_CERT_OTHER_INDX );

        tfPkiLockedRevokedCertByList ( tmpCrlListPtr,
                                       crlTxtPtr,
                                       TM_PKI_CERT_SELF_INDX );
    }
#if (defined(TM_PUBKEY_USE_RSA) || defined(TM_PUBKEY_USE_DSA))

#ifdef TM_PKI_CERT_NOT_VERIFY
/* still need to verify, if set pkiCertNotVerify as 0 */
    if (((ttPkiGlobalPtr)tm_context(tvPkiPtr))->pkiCertNotVerify == 0)
#endif /* TM_PKI_CERT_NOT_VERIFY */
    {
/*
 * Basic Constraints check
 * http://www.thoughtcrime.org/ie-ssl-chain.txt
 */
        if ((typeHeader & TM_PKI_CERT_ROOTCA) != 0)
        {
            if (certTxtPtr)
            {
                if (certTxtPtr->certTbsPtr)
                {
                    i = 0;
                    nextExtValuePtr =
                            certTxtPtr->certTbsPtr->tbsExtensionsPtr[i];
                    while (nextExtValuePtr && (i < TM_PKI_MAX_STACK))
                    {
                        if (nextExtValuePtr->extnIDPtr)
                        {
/* Search for the "BasicConstraints" extension */
                            tempInt = tm_memcmp(
                                        nextExtValuePtr->extnIDPtr->objIdStr,
                                        "2.5.29.19",
                                        tm_strlen("2.5.29.19"));
                            if (tempInt == 0)
                            {
#ifndef TM_PKI_BCCRITICAL_CHECK_DISABLE
                                foundBcField = TM_8BIT_YES;
#endif /* #ifndef TM_PKI_BCCRITICAL_CHECK_DISABLE */
/*
 * The value of the "BasicConstraints" extension is DER encoded at this point.
 * BasicConstraints ::= SEQUENCE {
 *   cA                      BOOLEAN OPTIONAL, --DEFAULT FALSE,
 *   pathLenConstraint       INTEGER OPTIONAL }
 * Typically, this will be: 0x 30 03 01 01 ff
 * 0x30: Sequence
 * 0x03: Total length of Sequence is 3 (no pathLenConstraint)
 * 0x01: Type Boolean
 * 0x01: Length of Boolean is 1
 * 0xff: Non-zero -- indicating True
 */
/* First, we validate the BasicConstraints extension */
                                temp8BitPtr =
                                        nextExtValuePtr->extnValuePtr->asn1Data;
                                if (   (temp8BitPtr[0] != 0x30)
                                    || (temp8BitPtr[1] < 0x03)
                                    || (temp8BitPtr[2] != 0x01)
                                    || (temp8BitPtr[3] != 0x01) )
                                {
#ifdef TM_LOG_CERT_WARNING
                                    tfEnhancedLogWrite(
                                        TM_LOG_MODULE_CERT,
                                        TM_LOG_LEVEL_WARNING,
                                        "tfPkiCertificateCheckAdd: "\
                                        "Root CA's BasicConstraints field "\
                                        "is malformed!");
#endif /* TM_LOG_CERT_WARNING */
                                    retCode |= TM_CERTFLAG_BADCERT;
                                    goto addCertificateExit;
                                }
                                if (temp8BitPtr[4] == 0)
                                {
#ifdef TM_LOG_CERT_WARNING
                                    tfEnhancedLogWrite(
                                        TM_LOG_MODULE_CERT,
                                        TM_LOG_LEVEL_WARNING,
                                        "tfPkiCertificateCheckAdd: "\
                                        "Root CA's BasicConstraints field "\
                                        "indicates it cannot be a CA!");
#endif /* TM_LOG_CERT_WARNING */
                                    retCode |= TM_CERTFLAG_BADCERT;
                                    goto addCertificateExit;
                                }
#ifndef TM_PKI_ROOTCA_IGNORE_BCCRITICAL
                                if (nextExtValuePtr->extnCritical == 0)
                                {
#ifdef TM_LOG_CERT_WARNING
                                    tfEnhancedLogWrite(
                                        TM_LOG_MODULE_CERT,
                                        TM_LOG_LEVEL_WARNING,
                                        "tfPkiCertificateCheckAdd: "\
                                        "Root CA's BasicConstraints field "\
                                        "is not marked as CRITICAL!");
#endif /* TM_LOG_CERT_WARNING */
                                    retCode |= TM_CERTFLAG_BADCERT;
                                    goto addCertificateExit;
                                }
#endif /* TM_PKI_ROOTCA_IGNORE_BCCRITICAL */
                                break;
                            }
                        }
                        i++;
                        nextExtValuePtr =
                            certTxtPtr->certTbsPtr->tbsExtensionsPtr[i];
                    }
                }
            }
        }
#ifndef TM_PKI_BCCRITICAL_CHECK_DISABLE
        if (   ((typeHeader & TM_PKI_CERT_ROOTCA) != 0)
            && (foundBcField == TM_8BIT_NO) )
        {
#ifdef TM_LOG_CERT_WARNING
            tfEnhancedLogWrite(
                TM_LOG_MODULE_CERT,
                TM_LOG_LEVEL_WARNING,
                "tfPkiCertificateCheckAdd: "\
                "Root CA's BasicConstraints field "\
                "is not present!");
#endif /* TM_LOG_CERT_WARNING */
            retCode |= TM_CERTFLAG_BADCERT;
            goto addCertificateExit;
        }
#endif /* TM_PKI_BCCRITICAL_CHECK_DISABLE */
    }
#endif /* TM_PUBKEY_USE_RSA ||  TM_PUBKEY_USE_DSA */

/* check the common name against what we believe it to be */
    if(commonName != (ttCharPtr)0)
    {
#ifdef TM_USE_IKEV2
        errorCode = TM_EINVAL; /* assume it is a fake cert for somebody else */
#endif /* TM_USE_IKEV2 */
        for(i = 0; i < certTxtPtr->certTbsPtr->tbsSubjectPtr->nameNum;
            i ++)
        {
            nextAttrValuePtr =
                certTxtPtr->certTbsPtr->tbsSubjectPtr->nameRelativeDNPtr[i];
            if(!nextAttrValuePtr)
            {
                break;
            }
            else
            {
                tempInt = tm_memcmp(nextAttrValuePtr->atvObjPtr->objIdStr,
                               "2.5.4.3",
                               tm_strlen("2.5.4.3"));
                if (!tempInt) /*common name */
                {
/*
 * Null Prefix Attack prevention
 * http://www.thoughtcrime.org/papers/null-prefix-attacks.pdf
 */
                    nullChrCheckResult = tm_strchr(
                        (ttConstCharPtr)nextAttrValuePtr->atvValuePtr->asn1Data,
                        '\0');
                    if (nullChrCheckResult !=
                            (  ((ttCharPtr)
                                    (nextAttrValuePtr->atvValuePtr->asn1Data))
                             + (nextAttrValuePtr->atvValuePtr->asn1Length)) )
                    {
#ifdef TM_LOG_CERT_WARNING
                        tfEnhancedLogWrite(
                            TM_LOG_MODULE_CERT,
                            TM_LOG_LEVEL_WARNING,
                            "tfPkiCertificateCheckAdd: "\
                            "Certificate contains NULL character " \
                            "in the Common Name!");
#endif /* TM_LOG_CERT_WARNING */
                        retCode |= TM_CERTFLAG_BADCERT;
                        goto addCertificateExit;
                    }
                    else
                    {
                        tempInt = tfPkiCertCommonNameCompare(
                               commonName,
                               (ttConstCharPtr)nextAttrValuePtr->atvValuePtr->
                               asn1Data);
                        if(tempInt != TM_ENOERROR)
                        {
#ifdef TM_LOG_CERT_INFO
                            tfEnhancedLogWrite(
                                TM_LOG_MODULE_CERT,
                                TM_LOG_LEVEL_INFO,
                                "tfPkiCertificateCheckAdd: "\
                                "certificate did not match a common name"\
                                TM_LOG_DELIMITER \
                                "%s does not match the name in the certificate"\
                                "(%s)",
                                commonName,
                                (ttConstCharPtr)nextAttrValuePtr->
                                                        atvValuePtr->asn1Data);
#endif /* TM_LOG_CERT_INFO */
                            retCode |= TM_CERTFLAG_UNMATCHED;
                        }
                    }
                    break;
                }
            }
        }
    }

    if (typeHeader & TM_PKI_CERT_LOCAL)
    {
        tmpCertListPtr->certKeyPtr = ((ttPkiGlobalPtr)tm_context(tvPkiPtr))
                   ->pkiOwnKeyPtr;
        if(!tmpCertListPtr->certKeyPtr)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfPkiCertificateCheckAdd",
                "You must add private key-pair before add LOCAL certificate");
#endif /* TM_ERROR_CHECKING */
#ifdef TM_LOG_CERT_WARNING
            tfEnhancedLogWrite(
                TM_LOG_MODULE_CERT,
                TM_LOG_LEVEL_WARNING,
                "tfPkiCertificateCheckAdd: "\
                "private key-pair must be added prior to local certificate");
#endif /* TM_LOG_CERT_WARNING */
            retCode |= TM_CERTFLAG_BADCERT;
            goto addCertificateExit;
        }
/* Reset the global pointer, as the local certificate owns the key now */
        ((ttPkiGlobalPtr)tm_context(tvPkiPtr))->pkiOwnKeyPtr =
                                                         (ttPkiPublicKeyPtr)0;
        tmpCertListPtr->certKeyPtr->pkType =  pkType;
    }

#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
#ifdef TM_USE_SSL_VERSION_33
/* DigestAlgorithm Index */
    for (i = 0; i < 19; i++)
    {
        tempInt = tm_strcmp ( 
            tlPkiConstObject[i].objShortName,
            certTxtPtr->certTbsPtr->tbsSignaturePtr->algorObjPtr->objShortName);
        if (tempInt == 0)
        {
            tmpCertListPtr->certDigestIndex = (tt8Bit)i;
            break;
        }
    }
    if(tempInt)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfPkiCertificateCheckAdd",
            "DigestAlgorithm field is NULL");
#endif /* TM_ERROR_CHECKING */
#ifdef TM_LOG_CERT_WARNING
        tfEnhancedLogWrite(
            TM_LOG_MODULE_CERT,
            TM_LOG_LEVEL_WARNING,
            "tfPkiCertificateCheckAdd: "\
            "DigestAlgorithm field is NULL");
#endif /* TM_LOG_CERT_WARNING */
        retCode |= TM_CERTFLAG_BADCERT;
        goto addCertificateExit;
    }
#endif /* TM_USE_SSL_VERSION_33 */
#endif /* TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */

    if ((idLength == 0)
        && (typeHeader & TM_PKI_CERT_LOCAL))
    {
/* if idLength = 0, then its ID is ASN1_DN */
        idLength                = tmpCertListPtr->certSubLength;
        tmpCertListPtr->certIdLength    = (tt16Bit)idLength;
        if (idLength != 0)
        {
            tmpCertListPtr->certIdPtr  = tm_get_raw_buffer((ttPktLen)idLength);
            if (tmpCertListPtr->certIdPtr == (ttVoidPtr)0)
            {
#ifdef TM_LOG_CERT_WARNING
                tfEnhancedLogWrite(
                    TM_LOG_MODULE_CERT,
                    TM_LOG_LEVEL_WARNING,
                    "tfPkiCertificateCheckAdd: "\
                    "unable to allocate %d bytes for certificate ASN1_DN ID",
                    idLength);
#endif /* TM_LOG_CERT_WARNING */
                retCode |= TM_CERTFLAG_BADCERT;
                goto addCertificateExit;
            }
            tm_bcopy((tt8BitPtr)tmpCertListPtr->certSubPtr,
                     tmpCertListPtr->certIdPtr,idLength);
        }
    }

    if ( typeHeader != TM_PKI_CERT_CRL)
    {
        tempInt = tfX509CertAlive(tmpCertListPtr);
        if (tempInt != TM_ENOERROR)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfPkiCertificateCheckAdd", "Cert Expired!\n");
#endif /* TM_ERROR_CHECKING */
#ifdef TM_LOG_CERT_WARNING
            tfEnhancedLogWrite(
                TM_LOG_MODULE_CERT,
                TM_LOG_LEVEL_WARNING,
                "tfPkiCertificateCheckAdd: "\
                "the certificate is expired");
#endif /* TM_LOG_CERT_WARNING */
            retCode |= TM_CERTFLAG_EXPIRED;
        }
    }

    *hdrCertListPtrPtr = tmpCertListPtr;
#ifdef TM_DEBUG_LOGGING
    tempCertPtr = tmpCertListPtr;
    while(tempCertPtr != (ttPkiCertListPtr)0)
    {
#ifdef TM_AUTO_TEST
        tm_debug_log2("Context 0x%x, 0x%x Certificate list: ",
                           tvCurrentContext, typeHeader);
#else /* !TM_AUTO_TEST */
        tm_debug_log1("0x%x Certificate list: ", typeHeader);
#endif /* !TM_AUTO_TEST */
        tm_debug_log3("certPtr=%lx, certLength=%u, certContentPtr=%lx",
                      tempCertPtr,
                      tempCertPtr->certLength,
                      tempCertPtr->certContentPtr);
        tempCertPtr = tempCertPtr->certLinkNextPtr;
    }
#endif /* TM_DEBUG_LOGGING */

addCertificateExit:

    if (typeHeader != TM_PKI_CERT_CRL)
    {
        (void)tfX509CertFree(certTxtPtr);
    }
    else
    {
        (void)tfX509CRLFree(crlTxtPtr);
    }
    tm_call_unlock( &(tm_context(tvPkiLockEntry)) );

    if (retCode & TM_CERTFLAG_BADCERT )
    {
        if (tmpCertListPtr != (ttPkiCertListPtr)0)
        {
            if (tmpCertListPtr->certIdPtr != 0)
            {
                tm_free_raw_buffer(tmpCertListPtr->certIdPtr);
            }
            if (   (tmpCertListPtr->certContentPtr != 0)
                && (tmpCertListPtr->certContentPtr != fileNamePtr) )
            {
                tm_free_raw_buffer(tmpCertListPtr->certContentPtr);
            }
            if (    (typeHeader & TM_PKI_CERT_NONLOCAL )
                 && (tmpCertListPtr->certKeyPtr != 0) )
            {
#ifdef TM_PUBKEY_USE_RSA
                if (tmpCertListPtr->certKeyPtr->pkType == TM_PKI_OBJ_RSA)
                {
                    tfPkiGeneralRsaFree(
                        (ttGeneralRsaPtr)
                            (tmpCertListPtr->certKeyPtr->pkKeyPtr));
                }
#endif /* TM_PUBKEY_USE_RSA */
#ifdef TM_PUBKEY_USE_DSA
                if (tmpCertListPtr->certKeyPtr->pkType == TM_PKI_OBJ_DSA)
                {
                    tfPkiGeneralDsaFree(
                        (ttGeneralDsaPtr)
                            (tmpCertListPtr->certKeyPtr->pkKeyPtr));
                }
#endif /* TM_PUBKEY_USE_DSA */
                tm_free_raw_buffer(tmpCertListPtr->certKeyPtr);
            }
            tm_free_raw_buffer((tt8BitPtr)tmpCertListPtr);
            tmpCertListPtr = (ttPkiCertListPtr)0;
        }
    }

    if (retCertPtrPtr)
    {
        *retCertPtrPtr = tmpCertListPtr;
    }

    tm_trace_func_exit_rc(tfPkiCertificateCheckAdd, retCode);
    return retCode;
}

/* FUNCTION: tfX509CrlTransfer
 *
 * PURPOSE:
 *  extract useful information from a crl (DER format)
 * PARAMETERS:
 *  crlDerPtr : pointer to a (CRL)Certificate Revocation List (DER format)
 *  crlPtr : pointer to the Certificate Revocation List structure
 * 
 * RETURNS:
 *   TM_ENOERROR:
 *      success
 */
static int tfX509CrlTransfer (     tt8BitPtr          crlDerPtr,
                                   ttPkiCertListPtr   crlPtr   )
{
    ttAsn1IntegerPtr    intTempPtr;
    tt8BitPtr           temp8Ptr;
    int                 length;
    
#ifdef TM_ERROR_CHECKING
    if ((crlDerPtr == (tt8BitPtr)0 
         || (crlPtr == (ttPkiCertListPtr)0)))
    {
        tfKernelError("tfX509CrlTransfer", "Invalid argument");

    }
#endif /* TM_ERROR_CHECKING */
    
    intTempPtr  = (ttAsn1IntegerPtr)0;
    crlPtr->certContentPtr = crlDerPtr;
/* cert list*/
    length      = tm_asn1_sequence_dec((ttVoidPtr)0,    &crlDerPtr);
/* tbs cert list */
    length      = tm_asn1_sequence_dec((ttVoidPtr)0,    &crlDerPtr);

/* version (X509 V1 CRL did not have version field ) */
    if ( *crlDerPtr == 0x02 )
    {
        (void)tm_asn1_integer_dec (  &intTempPtr ,   &crlDerPtr);
    }

/* signature algorithm, another sequence, generally, we ignore the
 * value here
 */
    temp8Ptr = crlDerPtr;
/* This length also includes the tag and length part*/
    length = tm_asn1_sequence_dec((ttVoidPtr)0, &crlDerPtr);
    crlDerPtr = (tt8BitPtr)(temp8Ptr + length);
/* issuer */
    crlPtr->certDNPtr = crlDerPtr;
    temp8Ptr = crlDerPtr;
    crlPtr->certDNLength   = (tt16Bit)
                    tm_asn1_sequence_dec((ttVoidPtr)0, &crlDerPtr);
    crlDerPtr = (tt8BitPtr)(temp8Ptr + crlPtr->certDNLength);
/* This Update */
    crlPtr->certValidBeforePtr = (ttVoidPtr)crlDerPtr;
    crlDerPtr += *(crlDerPtr + 1) + 2;
/* Next Update */
    crlPtr->certValidAfterPtr = crlDerPtr;

    return TM_ENOERROR;
}

int tfPkiOwnKeyPairAdd     (    ttUserConstVoidPtr  fileNamePtr,
                                int                 typeFormat,
                                int                 keyType)
{
    int errorCode;

    tm_call_lock_wait( &(tm_context(tvPkiLockEntry)) );
    if(!tm_context(tvPkiPtr))
    {
/* user didn't call tfUsePki before, exit */
        errorCode = TM_EINVAL;
    }
    else
    {
        errorCode = tfPkiOwnKeyPairAddTo (fileNamePtr,
                                 typeFormat,
                                 keyType,
                                 (ttVoidPtrPtr)(
                                  &((ttPkiGlobalPtr)tm_context(tvPkiPtr))
                                          ->pkiOwnKeyPtr),
                                 0) ;
    }
    tm_call_unlock( &(tm_context(tvPkiLockEntry)) );
    return errorCode;
}

int tfPkiOwnKeyPairDelete (void)
{
    int                     errorCode;

    tm_call_lock_wait(&(tm_context(tvPkiLockEntry)));
    if ((tm_context(tvPkiPtr)) != (ttVoidPtr)0)
/* Make sure tfUsePki has been called */
    {
        errorCode = tfPkiOwnKeyPairDeleteLocked();
    }
    else
    {
/* Failure case: no key to delete */
        errorCode = TM_ENOENT;
    }
    tm_call_unlock(&(tm_context(tvPkiLockEntry)));
    return errorCode;

}

static int tfPkiOwnKeyPairDeleteLocked (void)
{
    int                     errorCode;

    if ( (((ttPkiGlobalPtr)tm_context(tvPkiPtr))->pkiOwnKeyPtr) != 
                                            (ttPkiPublicKeyPtr)0)
    {
/* Local key was set, and not in use. */
        errorCode = TM_ENOERROR;
        tm_free_raw_buffer(
                    ((ttPkiGlobalPtr)tm_context(tvPkiPtr))->pkiOwnKeyPtr);
        ((ttPkiGlobalPtr)tm_context(tvPkiPtr))->pkiOwnKeyPtr =
                                            (ttPkiPublicKeyPtr)0;
    }
    else
    {
        errorCode = TM_ENOENT;
    }
    return errorCode;
}

int tfPkiOwnKeyPairAddTo (ttConstVoidPtr    fileNamePtr,
                          int               typeFormat,
                          int               keyType,
                          ttVoidPtrPtr      toAddrPtr,
                          int               checkExport)
{

    tt8BitPtr            temp8Ptr;
    tt8BitPtr            pkDerPtr;
    ttPkiPublicKeyPtr    pkPtr;
    ttAsn1IntegerPtr     intPtr;
#ifdef TM_PUBKEY_USE_RSA
    ttGeneralRsaPtr      rsaPtr;
#endif /* TM_PUBKEY_USE_RSA */
#ifdef TM_PUBKEY_USE_DSA
    ttGeneralDsaPtr      dsaPtr;
#endif /* TM_PUBKEY_USE_DSA */
    tt16Bit              totalLen;
    tt16Bit              length;
    int                  errorCode;

#if (!defined(TM_PUBKEY_USE_RSA) && !defined(TM_PUBKEY_USE_DSA))
    TM_UNREF_IN_ARG(keyType);
#endif /* !TM_PUBKEY_USE_RSA && !TM_PUBKEY_USE_DSA */
    errorCode = TM_EINVAL;
#ifndef TM_PUBKEY_USE_RSA
    TM_UNREF_IN_ARG(checkExport);
#endif /* TM_PUBKEY_USE_RSA */
    intPtr = (ttAsn1IntegerPtr)0;
    pkDerPtr = (tt8BitPtr)0;

    if (tm_8bit_bits_not_set(typeFormat, TM_PKI_CERTIFICATE_STRING))
    {
#ifdef TM_PKI_USE_FILE
        temp8Ptr = tfPkiGetStringInFile(fileNamePtr, typeFormat, &length);
#else /* !TM_PKI_USE_FILE */
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfPkiOwnKeyPairAddTo", "NO file I/O");
#endif /* TM_ERROR_CHECKING */
        goto pkiAddOwnKeyPairExit;
#endif /* !TM_PKI_USE_FILE */
    }
    else
    {
        temp8Ptr = (tt8BitPtr)fileNamePtr;
    }

    if (temp8Ptr == TM_8BIT_NULL_PTR)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfPkiOwnKeyPairAddTo", "string error");
#endif /* TM_ERROR_CHECKING */
        errorCode = TM_EINVAL;
        goto pkiAddOwnKeyPairExit;
    }

    if (tm_8bit_one_bit_set(typeFormat, TM_PKI_CERTIFICATE_PEM))
    {
        pkDerPtr = tfX509PemToDer(temp8Ptr, &length);
        if (tm_8bit_bits_not_set(typeFormat, TM_PKI_CERTIFICATE_STRING))
        {
            tm_free_raw_buffer(temp8Ptr);
        }

        if (pkDerPtr == TM_VOID_NULL_PTR)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfPkiOwnKeyPairAddTo", "wrong PEM");
#endif /* TM_ERROR_CHECKING */
            goto pkiAddOwnKeyPairExit;
        }
    }
    else if (tm_8bit_one_bit_set(typeFormat, TM_PKI_CERTIFICATE_DER))
    {
        pkDerPtr = temp8Ptr;
    }
    else
    {
        if (tm_8bit_bits_not_set(typeFormat, TM_PKI_CERTIFICATE_STRING))
        {
            tm_free_raw_buffer(temp8Ptr);
        }
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfPkiCertificateCheckAdd", "wrong Format");
#endif
        goto pkiAddOwnKeyPairExit;
    }


    pkPtr = (ttPkiPublicKeyPtr)tm_get_raw_buffer(sizeof(ttPkiPublicKey));
    if (pkPtr == TM_VOID_NULL_PTR)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfPkiOwnKeyPairAddTo", "null pointer!\n");
#endif /* TM_ERROR_CHECKING */
        errorCode = TM_ENOBUFS;
        goto pkiAddOwnKeyPairExit;
    }

    totalLen = 0;
/* sequence of ....... */
#ifdef TM_PUBKEY_USE_RSA
    if (keyType == TM_PKI_RSA_KEY)
    {
/* RSA, from Openssl:  */
        temp8Ptr = pkDerPtr;

        rsaPtr = (ttGeneralRsaPtr)tm_get_raw_buffer(sizeof(ttGeneralRsa));
        if (rsaPtr == (ttGeneralRsaPtr)0)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfPkiOwnKeyPairAddTo", "no memory!\n");
#endif /* TM_ERROR_CHECKING */
            goto pkiAddOwnKeyPairExit;
        }
        tm_bzero((tt8BitPtr)rsaPtr, sizeof(*rsaPtr));

        totalLen = (tt16Bit)tm_asn1_sequence_dec((ttVoidPtr)0, &temp8Ptr);
/* follow the openssl's format */
        (void)tm_asn1_integer_dec(&intPtr, &temp8Ptr);
        (void)tm_asn1_integer_dec(&rsaPtr->rsaN, &temp8Ptr);
        (void)tm_asn1_integer_dec(&rsaPtr->rsaE, &temp8Ptr);
        (void)tm_asn1_integer_dec(&rsaPtr->rsaD, &temp8Ptr);
        (void)tm_asn1_integer_dec(&rsaPtr->rsaP, &temp8Ptr);
        (void)tm_asn1_integer_dec(&rsaPtr->rsaQ, &temp8Ptr);
        (void)tm_asn1_integer_dec(&rsaPtr->rsaPm, &temp8Ptr);
        (void)tm_asn1_integer_dec(&rsaPtr->rsaQm, &temp8Ptr);
        (void)tm_asn1_integer_dec(&rsaPtr->rsaPinv, &temp8Ptr);

        pkPtr->pkType = TM_PKI_OBJ_RSA;
        pkPtr->pkKeyPtr = (ttVoidPtr)rsaPtr;
        if (rsaPtr->rsaN != 0)
        {
            pkPtr->pkSigLen = tm_pki_rsa_size(rsaPtr);
            if (checkExport && (pkPtr->pkSigLen > (512 >> 3)))
            {
#ifdef TM_ERROR_CHECKING
                tfKernelWarning("tfPkiOwnKeyPairAddTo", "Not exportable!\n");
#endif /* TM_ERROR_CHECKING */
                goto pkiAddOwnKeyPairExit;
            }
            if (    (((ttPkiGlobalPtr)(tm_context(tvPkiPtr)))->
                            pkiMaxKeySize != (tt16Bit)0)
                 && (pkPtr->pkSigLen > ((ttPkiGlobalPtr)
                          (tm_context(tvPkiPtr)))->pkiMaxKeySize >> 3)
               )
/* pkSigLen is in bytes, and pkiMaxKeySize in bits */
            {
#ifdef TM_ERROR_CHECKING
                tfKernelWarning("tfPkiOwnKeyPairAddTo",
                                "Key size exceeds set maximum!\n");
#endif /* TM_ERROR_CHECKING */
                errorCode = TM_ERANGE;
                goto pkiAddOwnKeyPairExit;
            }
        }
        else
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfPkiOwnKeyPairAddTo", "dec error!\n");
#endif /* TM_ERROR_CHECKING */
            goto pkiAddOwnKeyPairExit;
        }
    }
#endif /* TM_PUBKEY_USE_RSA */

#ifdef TM_PUBKEY_USE_DSA
    if (keyType == TM_PKI_DSA_KEY)
    {
/* DSA: key or parameter */
        temp8Ptr = pkDerPtr;

        dsaPtr = (ttGeneralDsaPtr)tm_get_raw_buffer(sizeof(ttGeneralDsa));
        if (dsaPtr == (ttGeneralDsaPtr)0)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfPkiOwnKeyPairAddTo", "no memory!\n");
#endif /* TM_ERROR_CHECKING */
            goto pkiAddOwnKeyPairExit;
        }
        tm_bzero((tt8BitPtr)dsaPtr, sizeof(*dsaPtr));


        totalLen = (tt16Bit)tm_asn1_sequence_dec((ttVoidPtr)0, &temp8Ptr);
/* follow the openssl's format */
        (void)tm_asn1_integer_dec(&intPtr, &temp8Ptr);
        (void)tm_asn1_integer_dec(&dsaPtr->dsaP, &temp8Ptr);
        (void)tm_asn1_integer_dec(&dsaPtr->dsaQ, &temp8Ptr);
        (void)tm_asn1_integer_dec(&dsaPtr->dsaG, &temp8Ptr);
        (void)tm_asn1_integer_dec(&dsaPtr->dsaPubKey, &temp8Ptr);
        (void)tm_asn1_integer_dec(&dsaPtr->dsaPriKey, &temp8Ptr);

        pkPtr->pkKeyPtr = (ttVoidPtr)dsaPtr;
        if (dsaPtr->dsaQ != 0)
        {
/* estimate value */
            pkPtr->pkSigLen = tm_pki_dsa_size(dsaPtr);
        }
        else
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfPkiOwnKeyPairAddTo", "dec q error!\n");
#endif /* TM_ERROR_CHECKING */
            pkPtr->pkSigLen = 100;
        }
    }
#endif /* TM_PUBKEY_USE_DSA */

    if (totalLen != (tt16Bit)(temp8Ptr - pkDerPtr))
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfPkiOwnKeyAddPair", "length error!\n");
#endif /* TM_ERROR_CHECKING */
        goto pkiAddOwnKeyPairExit;
    }
    if (*toAddrPtr == ((ttPkiGlobalPtr)tm_context(tvPkiPtr))->pkiOwnKeyPtr)
/* Setting global private key. Free previous one if any. */
    {
        (void)tfPkiOwnKeyPairDeleteLocked();
    }
    *toAddrPtr = pkPtr;
    errorCode = TM_ENOERROR;

pkiAddOwnKeyPairExit:
    if (intPtr != (ttAsn1IntegerPtr)0)
    {
        (void)tfAsn1StringFree(intPtr);
    }
    if (tm_8bit_one_bit_set(typeFormat, TM_PKI_CERTIFICATE_PEM ))
    {
        if (pkDerPtr != (tt8BitPtr)0)
        {
            tm_free_raw_buffer(pkDerPtr);
        }
    }
    return errorCode;
}
                            
int tfPkiOwnCertificateAdd(int              typeFormat,
                           ttUserVoidPtr    keyPairDataPtr,
                           int              keyType,
                           ttUserVoidPtr    certDataPtr,
                           ttUserVoidPtr    caIdPtr,
                           ttCertHandlePtr  certHandlePtr,
                           ttUserVoidPtr    idPtr,
                           int              idLength,
                           int             *certFlagPtr)
{

    ttPkiCertListPtr  certListPtr;
    int               certFlag;
    int               errorCode;
    int               isKeyTypeValid;

#ifdef TM_PUBKEY_USE_RSA
    if (keyType == TM_PKI_RSA_KEY)
    {
        isKeyTypeValid = 1;
    }
    else
#endif /* TM_PUBKEY_USE_RSA */
#ifdef TM_PUBKEY_USE_DSA
    if (keyType == TM_PKI_DSA_KEY)
    {
        isKeyTypeValid = 1;
    }
    else
#endif /* TM_PUBKEY_USE_DSA */
    {
        isKeyTypeValid = 0;
    }

    if ((tm_8bit_one_bit_set(typeFormat, (TM_PKI_CERTIFICATE_PEM
                                          | TM_PKI_CERTIFICATE_DER
                                          | TM_PKI_CERTIFICATE_STRING))
         && (keyPairDataPtr != (ttUserVoidPtr)0)
         && (isKeyTypeValid == 1)
         && (certDataPtr != (ttUserVoidPtr)0)
         && (certHandlePtr != (ttCertHandlePtr)0)
         && (certFlagPtr != (int*)0))
        )
    {
        if (tm_context(tvPkiPtr) != (ttVoidPtr)0)
        {
            tm_call_lock_wait(&(tm_context(tvPkiLockEntry)));
            errorCode = 
                tfPkiOwnKeyPairAddTo(keyPairDataPtr,
                                     typeFormat,
                                     keyType,
                                     (ttVoidPtrPtr)
                                     (&((ttPkiGlobalPtr)tm_context(tvPkiPtr))
                                      ->pkiOwnKeyPtr),
                                     (int)0);
            tm_call_unlock(&(tm_context(tvPkiLockEntry)));
            if (errorCode == TM_ENOERROR)
            {
                certFlag = tfPkiCertificateCheckAdd(&certListPtr,
                                                    (ttCharPtr)0,
                                                    certDataPtr,
                                                    typeFormat,
                                                    TM_PKI_CERT_LOCAL,
                                                    idPtr,
                                                    idLength,
                                                    caIdPtr);
                if (certFlag == (int)0)
                {
                    /* Hand cert handle to user */
                    *certHandlePtr = (ttCertHandle)certListPtr;
                }
                else
                {
                    /* Invalid certificate */
                    errorCode = TM_EPERM;
                }
                *certFlagPtr = certFlag;
            }
        }
        else
        {
            /* User didn't call tfUsePki before, exit */
            errorCode = TM_EACCES;
        }
    }
    else
    {
        /* Invalid argument */
        errorCode = TM_EINVAL;
    }

    return errorCode;
}

/* FUNCTION: tfPkiCertCleanup
 *
 * PURPOSE:
 *  free a certificate
 * PARAMETERS:
 *  ttPkiCertListPtr  certPtr
 * RETURNS:
 *   TM_ENOERROR:
 *       Successful.
 */

void tfPkiCertCleanup(ttPkiCertListPtr certPtr)
{
    int    pkiType;

    if ( certPtr == (ttPkiCertListPtr)0)
    {
        return;
    }
    if ( certPtr->certContentPtr != 0)
    {
        tm_free_raw_buffer((tt8BitPtr)certPtr->certContentPtr);
        certPtr->certContentPtr = (ttVoidPtr)0;
    }
    if ( certPtr->certIdPtr != 0)
    {
        tm_free_raw_buffer((tt8BitPtr)certPtr->certIdPtr);
        certPtr->certIdPtr = (ttVoidPtr)0;
    }
    if ( certPtr->certKeyPtr != 0)
    {
        pkiType = certPtr->certKeyPtr->pkType;
#ifdef TM_PUBKEY_USE_DSA
        if ( pkiType == TM_PKI_OBJ_DSASHA1
            || pkiType == TM_PKI_OBJ_DSA)
        {
            tfPkiGeneralDsaFree((ttGeneralDsaPtr)
                        (certPtr->certKeyPtr->pkKeyPtr));
            certPtr->certKeyPtr->pkKeyPtr = (ttVoidPtr)0;
        }
#endif /* TM_PUBKEY_USE_DSA */

#ifdef TM_PUBKEY_USE_RSA
        if(pkiType == TM_PKI_OBJ_RSA    ||
                pkiType == TM_PKI_OBJ_RSAMD5 ||
                pkiType == TM_PKI_OBJ_RSASHA1||
                pkiType == TM_PKI_OBJ_RSASHA256||
                pkiType == TM_PKI_OBJ_RSASHA384||
                pkiType == TM_PKI_OBJ_RSASHA512||
                pkiType == TM_PKI_OBJ_RSARIPEMD)
        {
            tfPkiGeneralRsaFree((ttGeneralRsaPtr)
                        (certPtr->certKeyPtr->pkKeyPtr));
            certPtr->certKeyPtr->pkKeyPtr = (ttVoidPtr)0;
        }
#endif /* TM_PUBKEY_USE_RSA */
/* if the certificate is local, set keyPtr as 0 in order not to free twice */
        ((ttPkiGlobalPtr)tm_context(tvPkiPtr))->pkiOwnKeyPtr = 0;
        tm_free_raw_buffer((tt8BitPtr)certPtr->certKeyPtr);
        certPtr->certKeyPtr = (ttPkiPublicKeyPtr)0;
    }
    return;
}

/* FUNCTION: tfPkiGeneralRsaFree
 *
 * PURPOSE:
 *  free a general RSA
 * PARAMETERS:
 *  ttGeneralRsaPtr  rsaPtr
 * RETURNS:
 *   TM_ENOERROR:
 *       Successful.
 */
#ifdef TM_PUBKEY_USE_RSA
void tfPkiGeneralRsaFree(ttGeneralRsaPtr rsaPtr)
{
    if ( rsaPtr == (ttGeneralRsaPtr)0)
    {
        return;
    }
    if ( rsaPtr->rsaN != (ttAsn1IntegerPtr)0)
    {
        (void)tfAsn1StringFree(rsaPtr->rsaN);
    }
    if ( rsaPtr->rsaE != (ttAsn1IntegerPtr)0)
    {
        (void)tfAsn1StringFree(rsaPtr->rsaE);
    }
    if ( rsaPtr->rsaD != (ttAsn1IntegerPtr)0)
    {
        (void)tfAsn1StringFree(rsaPtr->rsaD);
    }
    if ( rsaPtr->rsaP != (ttAsn1IntegerPtr)0)
    {
        (void)tfAsn1StringFree(rsaPtr->rsaP);
    }
    if ( rsaPtr->rsaQ != (ttAsn1IntegerPtr)0)
    {
        (void)tfAsn1StringFree(rsaPtr->rsaQ);
    }
    if ( rsaPtr->rsaPm != (ttAsn1IntegerPtr)0)
    {
        (void)tfAsn1StringFree(rsaPtr->rsaPm);
    }
    if ( rsaPtr->rsaQm != (ttAsn1IntegerPtr)0)
    {
        (void)tfAsn1StringFree(rsaPtr->rsaQm);
    }
    if ( rsaPtr->rsaPinv != (ttAsn1IntegerPtr)0)
    {
        (void)tfAsn1StringFree(rsaPtr->rsaPinv);
    }
    tm_free_raw_buffer((tt8BitPtr)rsaPtr);
    return;
}
#endif /* TM_PUBKEY_USE_RSA */

/* FUNCTION: tfPkiGeneralDsaFree
 *
 * PURPOSE:
 *  free a general DSA
 * PARAMETERS:
 *  ttGeneralDsaPtr  dsaPtr
 * RETURNS:
 *   TM_ENOERROR:
 *       Successful.
 */

#ifdef TM_PUBKEY_USE_DSA
static void tfPkiGeneralDsaFree(ttGeneralDsaPtr dsaPtr)
{
    if ( dsaPtr == (ttGeneralDsaPtr)0)
    {
        return;
    }
    if ( dsaPtr->dsaP != (ttAsn1IntegerPtr)0)
    {
        (void)tfAsn1StringFree(dsaPtr->dsaP);
    }
    if ( dsaPtr->dsaQ != (ttAsn1IntegerPtr)0)
    {
        (void)tfAsn1StringFree(dsaPtr->dsaQ);
    }
    if ( dsaPtr->dsaG != (ttAsn1IntegerPtr)0)
    {
        (void)tfAsn1StringFree(dsaPtr->dsaG);
    }
    if ( dsaPtr->dsaPubKey != (ttAsn1IntegerPtr)0)
    {
        (void)tfAsn1StringFree(dsaPtr->dsaPubKey);
    }
    if ( dsaPtr->dsaPriKey != (ttAsn1IntegerPtr)0)
    {
        (void)tfAsn1StringFree(dsaPtr->dsaPriKey);
    }
    tm_free_raw_buffer((tt8BitPtr)dsaPtr);
    return;
}
#endif /* TM_PUBKEY_USE_DSA */

/* FUNCTION: tfPkiCertificateSearch
 *
 * PURPOSE:
 *  find a certificate
 * PARAMETERS:
 *  type   : search type, id/issuer/subject/serialnum
 *  keyPtr : key pointer corresponding to the type
 *  keyLength : key length
 *  hdrPtr : certifcate list header pointer ( maybe 0)
 *
 * RETURNS:
 *   pointer to the certificate
 */
ttPkiCertListPtr tfPkiCertificateSearch(int              type,
                                        ttConstVoidPtr   keyPtr,
                                        int              keyLength,
                                        int              listIndex,
                                        tt8Bit           needLock)
{
    ttPkiCertListPtr    retCertPtr;
    ttPkiCertListPtr    hdrPtr;
    int                 result;
    int                 tempInt;

    retCertPtr = (ttPkiCertListPtr)0;

    if (needLock)
    {
        tm_call_lock_wait(&(tm_context(tvPkiLockEntry)));
    }
    if (tm_context(tvPkiPtr))
    {
        hdrPtr = ((ttPkiGlobalPtr)
                    tm_context(tvPkiPtr))->pkiCertListHdrPtrArr[listIndex];
        result  = -1;


        while ( hdrPtr != (ttVoidPtr)0)
        {
            if ( hdrPtr->certRevoked == TM_8BIT_YES)
            {
                continue;
            }

            switch (type)
            {
                case TM_PKI_CERTSEARCH_ID:
                    if ( keyLength == (int)hdrPtr->certIdLength )
                    {
                        result =
                              tm_memcmp(hdrPtr->certIdPtr, keyPtr, keyLength);
                    }
                    break;
                case TM_PKI_CERTSEARCH_ISSUER:
                    if ( keyLength == (int)hdrPtr->certSubLength )
                    {
                        result = tm_memcmp(hdrPtr->certSubPtr, keyPtr,
                                           keyLength);
                    }
                    break;
                case TM_PKI_CERTSEARCH_SUBJECT:
                    if ( keyLength == (int)hdrPtr->certSubLength )
                    {
                        result = tm_memcmp(hdrPtr->certSubPtr, keyPtr,
                                           keyLength);
                    }
                    break;
                case TM_PKI_CERTSEARCH_SERIALNUM:
                    result = tm_memcmp(hdrPtr->certSerNumPtr , keyPtr,
                                                hdrPtr->certSerNumLength);
                    break;
                default:
                    break;
            }
            if ( result == 0)
            {
                tempInt = tfX509CertAlive(hdrPtr);
                if (tempInt == TM_ENOERROR)
                {
                    retCertPtr = hdrPtr;
                }
                break;
            }
            hdrPtr = hdrPtr->certLinkNextPtr;
        }
    }
    if (needLock)
    {
        tm_call_unlock(&(tm_context(tvPkiLockEntry)));
    }
    return retCertPtr;
}


/* FUNCTION: tfPkiCertificateReadKey
 *
 * PURPOSE:
 *  return the key pointer in a certificate
 * PARAMETERS:
 *  idPtr   : id pointer
 *  hdrPtr : certifcate list header pointer ( maybe 0)
 *
 * RETURNS:
 *   pointer to the public key
 */

ttPkiPublicKeyPtr tfPkiCertificateReadKey(
                                    ttVoidPtr idPtr,
                                    int       idLength,
                                    int       listIndex)
{
    ttPkiCertListPtr    retPtr;
    retPtr = tfPkiCertificateSearch(TM_PKI_CERTSEARCH_ID,
                                    idPtr,
                                    idLength,
                                    listIndex,
                                    TM_8BIT_YES); /* need lock */
    if ( retPtr == (ttVoidPtr)0)
    {
        return (ttVoidPtr)0;
    }
    return retPtr->certKeyPtr;
}

/* FUNCTION: tfX509CertTransfer
 *
 * PURPOSE:
 *  extract useful information from a certificate (DER format)
 * PARAMETERS:
 *  certDerPtr   :  pointer to a certificate
 *  certPtr : pointer to the certificate structure
 *
 * RETURNS:
 *   TM_ENOERROR:
 *      success
 */

static int tfX509CertTransfer (     tt8BitPtr           certDerPtr,
                                    ttPkiCertListPtr    certPtr   )
{
    ttAsn1IntegerPtr    intTempPtr;
    tt8BitPtr           temp8Ptr;
    int                 length;


    intTempPtr  = (ttAsn1IntegerPtr)0;
    certPtr->certContentPtr = certDerPtr;
/* find the subject name position and length */
/* cert*/
    length      = tm_asn1_sequence_dec((ttVoidPtr)0,    &certDerPtr);
/* tbs cert */
    length      = tm_asn1_sequence_dec((ttVoidPtr)0,    &certDerPtr);

/* version & serial number */
    if ( *certDerPtr != 0xa0 )
    {
/* some openssl certificate did not have serial number. Process the
 * intTempPtr for only once.
 */
        (void)tm_asn1_integer_dec (  &intTempPtr ,   &certDerPtr);
    }
    else
    {
        certDerPtr++;
        (void)tm_asn1_context_dec(&length ,&certDerPtr);
/* bypass the version */
        (void)tm_asn1_integer_dec (  &intTempPtr ,   &certDerPtr);
/* including two bytes for tag + len*/
        certPtr->certSerNumLength = (tt8Bit)
                            (*((tt8BitPtr)(certDerPtr + 1)) + 2);
/* points the begging of tag part of serialNumber*/
        certPtr->certSerNumPtr = (ttVoidPtr)certDerPtr;
        (void)tm_asn1_integer_dec (  &intTempPtr ,   &certDerPtr);
    }

/* signature algorithm, another sequence, generally, we ignore the
 * value here
 */
    temp8Ptr      = certDerPtr;
/* This length also includes the tag and length part*/
    length      = tm_asn1_sequence_dec((ttVoidPtr)0, &certDerPtr);
    if (length == -1)
    {
        goto X509_CERT_TRANSFER_EXIT;
    }
    certDerPtr  = (tt8BitPtr)(temp8Ptr + length);
/* issuer */
    certPtr->certDNPtr = certDerPtr;
    temp8Ptr      = certDerPtr;
    certPtr->certDNLength   = (tt16Bit)
                    tm_asn1_sequence_dec((ttVoidPtr)0, &certDerPtr);
    if (certPtr->certDNLength == (tt16Bit)-1)
    {
        goto X509_CERT_TRANSFER_EXIT;
    }
    certDerPtr  = (tt8BitPtr)(temp8Ptr + certPtr->certDNLength);
/* validity */
    temp8Ptr      = certDerPtr;
    length      = tm_asn1_sequence_dec((ttVoidPtr)0, &certDerPtr);
    if (length == -1)
    {
        goto X509_CERT_TRANSFER_EXIT;
    }
    certPtr->certValidBeforePtr = (ttVoidPtr)(certDerPtr + 2);
    certPtr->certValidAfterPtr  = (ttVoidPtr)(certDerPtr + 2 + 2 +
                                *(tt8BitPtr)((tt8BitPtr)certDerPtr + 1));
    certDerPtr  = (tt8BitPtr)(temp8Ptr + length);
/* subject  */
    certPtr->certSubPtr  = certDerPtr;
    temp8Ptr      = certDerPtr;
    certPtr->certSubLength= (tt16Bit)
                    tm_asn1_sequence_dec((ttVoidPtr)0, &certDerPtr);
    if (certPtr->certSubLength == (tt16Bit)-1)
    {
        goto X509_CERT_TRANSFER_EXIT;
    }
    certDerPtr  = (tt8BitPtr)(temp8Ptr + certPtr->certSubLength);

X509_CERT_TRANSFER_EXIT:
    if ( intTempPtr != (ttAsn1IntegerPtr)0)
    {
        (void)tfAsn1StringFree(intTempPtr);
    }
    return TM_ENOERROR;
}


static ttPkiCertListPtrPtr tfPkiCertTypeToList(int certType)
{
    ttPkiCertListPtrPtr     hdrPtrPtr;
    int                     index;

    if ( certType & TM_PKI_CERT_LOCAL)
    {
        index = TM_PKI_CERT_SELF_INDX;
    }
    else if ( certType & TM_PKI_CERT_NONLOCAL)
    {
        index = TM_PKI_CERT_OTHER_INDX;
    }
    else if ( certType & TM_PKI_CERT_CRL)
    {
        index = TM_PKI_CERT_CRL_INDX;
    }
    else
    {
        index = -1;
    }
    if (index != -1)
    {
        hdrPtrPtr = &(((ttPkiGlobalPtr)tm_context(tvPkiPtr))->
                                            pkiCertListHdrPtrArr[index]);
    }
    else
    {
        hdrPtrPtr = (ttPkiCertListPtrPtr)0;
    }
    return hdrPtrPtr;
}

#ifdef TM_PKI_USE_FILE

/* following two functions for FILE I/O */
/* need to add txftpfs.c, and trramfs.c if want to use RAM file
 * pass BEGIN,
 * pass END
 * if your file i/o command is different, you should replace fopen, fgetc,
 *, fseek, fclose,FILE
 */


tt8BitPtr   tfPkiGetStringInFile (ttConstVoidPtr    fileNamePtr,
                                  int               typeFormat,
                                  tt16BitPtr        lenPtr)
{
/* there is a method to get base64 betwwen BEGIN and END */
    ttVoidPtr       hdrPtr;
    ttCharPtr       retPtr;
    ttCharPtr       temp8Ptr;
    ttCharPtr       prevTemp8Ptr;
    int             length;
    int             totalLen;
    int             i;
    int             searchLineFeed;
    int             firstRead;
    int             firstOffset;
    int             lastOffset;

    ttVoidPtr       userPtr;

    retPtr      = (ttCharPtr)0;
    temp8Ptr    = (ttCharPtr)0;

    userPtr = tfFSUserLogin ("treck", "treck");
    if(userPtr == (ttVoidPtr)0)
    {
        goto pkiGetStringInFileExit;
    }

    hdrPtr      = tfFSOpenFile(userPtr, fileNamePtr,
                    TM_FS_READ, TM_TYPE_ASCII, TM_STRU_STREAM);
    if ( hdrPtr == (ttVoidPtr)0 )
    {
        goto pkiGetStringInFileExit;
    }
    searchLineFeed = 0;
    firstRead   = 0;
    firstOffset = 0;
    lastOffset  = 0;
    totalLen    = 0;
#ifndef TM_BLOCK_LENGTH
#define TM_BLOCK_LENGTH 128
#endif /* TM_BLOCK_LENGTH */
    do
    {
        prevTemp8Ptr = temp8Ptr;
        temp8Ptr = (ttCharPtr)tm_get_raw_buffer(TM_BLOCK_LENGTH + totalLen);
        if (temp8Ptr == (ttCharPtr)0)
        {
            if (prevTemp8Ptr != (ttCharPtr)0)
            {
                tm_free_raw_buffer(prevTemp8Ptr);
            }
            break;
        }
        if (prevTemp8Ptr != (ttCharPtr)0)
        {
            tm_bcopy(prevTemp8Ptr, temp8Ptr, totalLen);
            tm_free_raw_buffer(prevTemp8Ptr);
        }
        length = tfFSReadFile(userPtr, hdrPtr, temp8Ptr + totalLen,
                              TM_BLOCK_LENGTH);
        if (length > 0)
        {
            totalLen += length;
        }
    }
    while (length == TM_BLOCK_LENGTH);
    tfFSCloseFile((ttVoidPtr)0, hdrPtr);

    if ( temp8Ptr == (ttCharPtr)0)
    {
        goto pkiGetStringInFileExit;
    }
    if (totalLen <= 0)
    {
        tm_free_raw_buffer(temp8Ptr);
        goto pkiGetStringInFileExit;
    }

    if ( typeFormat == TM_PKI_CERTIFICATE_DER)
    {
        retPtr = temp8Ptr;
        goto pkiGetStringInFileExit;
    }
    for (i = 0; i < totalLen; i++)
    {
        if ( firstRead == 0)
        {
            if ( (searchLineFeed == 0)
                &&(tm_strncmp(temp8Ptr+i,"BEGIN",5)==0))
            {
                searchLineFeed  = 1;
                i+=5;
            }

            if ( searchLineFeed == 1)
            {
                if ( temp8Ptr[i] == 10 || temp8Ptr[i] == 13)
                {
                    if ( temp8Ptr[i+1] != 10)
                    {
                        firstOffset = i+1;
                    }
                    else
                    {
                        firstOffset = i+2;
                    }
                    searchLineFeed = 0;
                    firstRead       = 1;
                }
            }
        }
        else
        {
            if ( temp8Ptr[i] == 10 || temp8Ptr[i] == 13)
            {
                lastOffset = i + 1;
            }
            if (tm_strncmp(temp8Ptr+i,"END",3)==0)
            {
                break;
            }
        }
    }

    *lenPtr = (tt16Bit)(lastOffset - firstOffset);
    retPtr = (ttCharPtr)tm_get_raw_buffer(*lenPtr);
    if (retPtr != (ttCharPtr)0)
    {
        tm_bcopy( &temp8Ptr[firstOffset], retPtr, *lenPtr);
        tm_free_raw_buffer((tt8BitPtr)temp8Ptr);
    }
pkiGetStringInFileExit:
    if (userPtr)
    {
        tfFSUserLogout(userPtr);
    }
    return (tt8BitPtr)retPtr;
}


int tfPkiPutStringToFile (      ttConstVoidPtr  fileNamePtr,
                                int             typeFormat,
                                tt8BitPtr       bufferPtr,
                                tt16Bit         length,
                                ttCharPtr       title)
{
/* there is a method to get base64 betwwen BEGIN and END */
    ttVoidPtr         hdrPtr;
    ttVoidPtr         userPtr;

    userPtr = tfFSUserLogin ("treck", "treck");
    if(userPtr == (ttVoidPtr)0)
    {
        goto pkiPutStringToFileExit;
    }

    hdrPtr = tfFSOpenFile(userPtr, fileNamePtr,
                    TM_FS_WRITE, TM_TYPE_ASCII, TM_STRU_STREAM);
    if ( hdrPtr == (ttVoidPtr)0 )
    {
        goto pkiPutStringToFileExit;
    }

    if ( typeFormat == TM_PKI_CERTIFICATE_DER)
    {
        tfFSWriteFile(userPtr, hdrPtr,(ttCharPtr)bufferPtr, length);
    }
    else
    {
        tfFSWriteFile(userPtr, hdrPtr,
                    "-----BEGIN ", 12);
        tfFSWriteFile(userPtr, hdrPtr,
                    title, tm_strlen(title)-1);
        tfFSWriteFile(userPtr, hdrPtr,
                    "-----\n", 6);
        tfFSWriteFile(userPtr, hdrPtr,(ttCharPtr)bufferPtr, length);

        tfFSWriteFile(userPtr, hdrPtr,
                    "-----END ", 10);
        tfFSWriteFile(userPtr, hdrPtr,
                    title, tm_strlen(title)-1);
        tfFSWriteFile(userPtr, hdrPtr,
                    "-----\n", 6);

    }

    tfFSCloseFile((ttVoidPtr)0, hdrPtr);

pkiPutStringToFileExit:
    if (userPtr)
    {
        tfFSUserLogout(userPtr);
    }
    return TM_ENOERROR;
}
#endif /* TM_PKI_USE_FILE */

#else /* !TM_USE_PKI */
/* To allow link for builds when TM_USE_PKI is not defined */
int tlPkixDummy = 0;

#endif /* TM_USE_PKI */
