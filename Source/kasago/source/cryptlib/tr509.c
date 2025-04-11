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
* Description:
*
* Filename: tr509.c
* Author: Jin Shang
* Date Created: 5/20/2002
 * $Source: source/cryptlib/tr509.c $
 *
 * Modification History
 * $Revision: 6.0.2.4 $
 * $Date: 2016/04/13 18:15:43JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

#ifdef TM_USE_PKI
#include <trpki.h>

/* Following Objects are related to X509 and PKI_IKE, The field objDerStr will be
 * added in tfX509ObjectInit when you call tfUsePki */
#ifndef TM_USE_SSL_VERSION_33
static const
#else
const
#endif /* TM_USE_SSL_VERSION_33 */
ttAsn1Object TM_CONST_QLF tlPkiConstObject[]={
{"UNDEF",       "undefined",                "0", 0},
{"trustRoot",   "Trust Root",               "1.3.6.1.5.5.7.48.1.11",0},
/* algorithms */
{"pkcs",        "RSA Data Security, Inc. PKCS", "1.2.840.113549.1",0},
{"rsaEncryption", "rsaEncryption",          "1.2.840.113549.1.1.1",0}, /* 3 */
{"RSA-MD2",     "md2WithRSAEncryption",     "1.2.840.113549.1.1.2",0}, /* 4 */
{"RSA-MD5",     "md5WithRSAEncryption",     "1.2.840.113549.1.1.4",0}, /* 5 */
{"RSA-SHA1",    "sha1WithRSAEncryption",    "1.2.840.113549.1.1.5",0}, /* 6 */
{"RSA-SHA256",  "sha256WithRSAEncryption",  "1.2.840.113549.1.1.11",0}, /* 7 */
{"RSA-SHA384",  "sha384WithRSAEncryption",  "1.2.840.113549.1.1.12",0}, /* 8 */
{"RSA-SHA512",  "sha512WithRSAEncryption",  "1.2.840.113549.1.1.13",0}, /* 9 */
{"RSA-RIPEMD160", "ripemd160WithRSA",       "1.3.36.3.3.1.2",0}, /* 10 */
{"DSA",         "dsaEncryption",            "1.2.840.10040.4.1",0}, /* 11 */
{"DSA-SHA1",    "dsaWithSHA1",              "1.2.840.10040.4.3",0}, /* 12 */
{"MD2",         "md2",                      "1.2.840.113549.2.2",0}, /* 13 */
{"MD5",         "md5",                      "1.2.840.113549.2.5",0}, /* 14 */
{"SHA1",        "sha1",                     "1.3.14.3.2.26",0}, /* 15 */
{"SHA256",      "sha256",                   "2.16.840.1.101.3.4.2.1",0}, 
/* 16 */
{"SHA384",      "sha384",                   "2.16.840.1.101.3.4.2.2",0},
/* 17 */
{"SHA512",      "sha512",                   "2.16.840.1.101.3.4.2.3",0},
/* 18 */

/* name attribute */
{"CN",          "commonName",               "2.5.4.3",0},
{"S",           "surname",                  "2.5.4.4",0},
{"SN",          "serialNumber",             "2.5.4.5",0},
{"C",           "countryName",              "2.5.4.6",0},
{"L",           "localityName",             "2.5.4.7",0},
{"ST",          "stateOrProvinceName",      "2.5.4.8",0},
{"O",           "organizationName",         "2.5.4.10",0},
{"OU",          "organizationalUnitName",   "2.5.4.11",0},
{"T",           "title",                    "2.5.4.12",0},
{"Email",       "emailAddress",             "1.2.840.113549.1.9.1",0},
{"E",           "emailAddress",             "1.2.840.113549.1.9.1",0},
{"unstructuredName",
    "PKCS #9 unstructured name",            "1.2.840.113549.1.9.2",0},
{"unstructuredAddress",
    "PKCS #9 unstructured address",         "1.2.840.113549.1.9.8",0},
{"name",        "name",                     "2.5.4.41",0},
{"G",           "givenName",                "2.5.4.42",0},
{"I",           "initials",                 "2.5.4.43",0},
{"gQualifier",  "generationQualifier",      "2.5.4.44",0},
{"dnQualifier", "dnQualifier",              "2.5.4.46",0},
{"PN",          "pseudonym",                "2.5.4.65",0},
{"DC",          "domainComponent",          "0.9.2342.19200300.100.1.25",0},
/* v3 extensions: standard */
/* no "Policy Mappings, Subject Directory Attributes, Name constrains"*
 * "Inhibit any-policy, freshest CRL " */
{"subjectKeyIdentifier",
    "X509v3 Subject Key Identifier",        "2.5.29.14",0},
{"authorityKeyIdentifier",
    "X509v3 Authority Key Identifier",      "2.5.29.35",0},
{"keyUsage", "X509v3 Key Usage",            "2.5.29.15",0},
{"privateKeyUsagePeriod",
    "X509v3 Private Key Usage Period",      "2.5.29.16",0},
{"certificatePolicies",
    "X509v3 Certificate Policies",          "2.5.29.32",0},
{"subjectAltName",
    "X509v3 Subject Alternative Name",      "2.5.29.17",0},
{"issuerAltName",
    "X509v3 Issuer Alternative Name",       "2.5.29.18",0},
{"basicConstraints",
    "X509v3 Basic Constraints",             "2.5.29.19",0},
{"policyConstraints",
    "X509v3 Policy Constraints",            "2.5.29.36",0},
{"extendedKeyUsage",
    "X509v3 Extended Key Usage",            "2.5.29.37",0},
{"crlDistributionPoints",
    "X509v3 CRL Distribution Points",       "2.5.29.31",0},
/* v3 extensions:private internet */
{"authorityInfoAccess",
    "Authority Information Access",         "1.3.6.1.5.5.7.1.1",0},
{"subjectInfoAccess",
    "Subject Information Access",           "1.3.6.1.5.5.7.1.11",0},
#ifdef TM_X509_SUPPORT_MICROSOFT_OID
/* Microsoft OID */
{"szOID_CERTSRV_CA_VERSION",  
    "1.3.6.1.4.1.311.21.1",                 "1.3.6.1.4.1.311.21.1",0},
{"szOID_CRL_NEXT_PUBLISH",  
    "1.3.6.1.4.1.311.21.4",                 "1.3.6.1.4.1.311.21.4",0},
{"szOID_CRL_SELF_CDP",  
    "1.3.6.1.4.1.311.21.14",                 "1.3.6.1.4.1.311.21.14",0}, 
#endif /* TM_X509_SUPPORT_MICROSOFT_OID */
/* crl extensions */
/* no " issuing distribution point, freshest crl */
{"crlNumber", "X509v3 CRL Number",          "2.5.29.20",0},
{"deltaCRL", "X509v3 Delta CRL Indicator",  "2.5.29.27",0},
/* crl entry extensions */
/* no " hold instruction code, certificate issuer"*/
{"CRLReason", "X509v3 CRL Reason Code",     "2.5.29.21",0},
{"invalidityDate", "Invalidity Date",       "2.5.29.24",0},
/* others */
{"challengePassword", "challengePassword",  "1.2.840.113549.1.9.7",0},
{"nsComment", "Netscape Comment",           "2.16.840.1.113730.1.13",0},
{"Entrust",   "Entrust Version Extension",  "1.2.840.113533.7.65.0",0}
};



#define tm_x509_objid(pkiPtr, x) \
        ((int)(ttUserPtrCastToInt)((tt8BitPtr)x - (tt8BitPtr)pkiPtr->pkiObjectPtr) \
         /(int)sizeof(ttAsn1Object))
#define TM_PKI_MAX_OBJECT       (sizeof(tlPkiConstObject)/sizeof(ttAsn1Object))

#define TM_PKI_OBJECT_NID_UNDEF         0


static tt8Bit   tfX509PemDec     (tt8Bit x);
static tt8Bit   tfX509PemEnc     (tt8Bit x);
static int      tfX509AlgorIdFree(ttX509AlgorIdenPtr aIdPtr);
static ttX509TBSCertPtr    tfX509TBSCertNew(void);
static int      tfX509TBSCertFree( ttX509TBSCertPtr tbsPtr);
static ttX509NamePtr tfX509NameNew (void);
static int      tfX509NameFree (ttX509NamePtr name);
#ifdef TM_PKI_CERT_CHECK_ALIVE
static tt32Bit tfX509TimeAbsDiff(tt8BitPtr timePtr, int type);
#endif /* TM_PKI_CERT_CHECK_ALIVE */
#ifdef TM_PUBKEY_USE_DSA
static int tfX509DsaPkParaGet( ttVoidPtr keyPtr, ttAsn1StringPtr paPtr);
#endif /* TM_PUBKEY_USE_DSA */

ttX509CertPtr   tfX509CertNew(void)
{
    ttX509CertPtr       certPtr;
    int                 errorCode;

    errorCode = TM_ENOBUFS;
    certPtr = (ttX509CertPtr)tm_get_raw_buffer(sizeof(ttX509Cert));
    if ( certPtr == (ttVoidPtr)0)
    {
        goto x509CertNewExit;
    }
    tm_bzero(certPtr, sizeof(ttX509Cert));
    certPtr->certTbsPtr   = tfX509TBSCertNew();
    if (certPtr->certTbsPtr == 0)
    {
        goto x509CertNewExit;
    }
    certPtr->certAlgorPtr = (ttX509AlgorIdenPtr)
                        tm_get_raw_buffer(sizeof(ttX509AlgorIden));
    if (certPtr->certAlgorPtr == 0)
    {
        goto x509CertNewExit;
    }
    tm_bzero(certPtr->certAlgorPtr, sizeof(ttX509AlgorIden));
    certPtr->certSigPtr = tfAsn1StringNew(TM_ASN1_TAG_BITSTRING);
    if (certPtr->certSigPtr == 0)
    {
        goto x509CertNewExit;
    }
    errorCode = TM_ENOERROR;
x509CertNewExit:
    if (errorCode != TM_ENOERROR)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfX509CertNew", "out of memory!\n");
#endif /* TM_ERROR_CHECKING */
        if (certPtr != (ttX509CertPtr)0)
        {
            tfX509CertFree(certPtr);
            certPtr = (ttX509CertPtr)0;
        }
    }
    return certPtr;
}

int tfX509CertFree(ttX509CertPtr certPtr)
{
    int errorCode;

    errorCode = TM_ENOERROR;
    if ( certPtr != (ttVoidPtr)0)
    {
        (void)tfX509TBSCertFree(certPtr->certTbsPtr);
/* free sigAlgor, and sigValue. tfX509AlgorIdFree checks for NULL pointer */
        (void)tfX509AlgorIdFree(certPtr->certAlgorPtr);
/* tfAsn1StringFree checks for NULL pointer */
        (void)tfAsn1StringFree(certPtr->certSigPtr);
        tm_free_raw_buffer((tt8BitPtr)certPtr);
    }
    else
    {
        errorCode = TM_EINVAL;
    }
    return errorCode;
}

ttX509CertListPtr   tfX509CRLNew(void)
{
    ttX509CertListPtr       cerListPtr;
    ttX509TBSCertListPtr    tbsCertListPtr;
    int                     errorCode;
    int                     i;

    errorCode = TM_ENOBUFS; /* assume failure */
    cerListPtr =(ttX509CertListPtr)tm_get_raw_buffer( sizeof(ttX509CertList));
    if ( cerListPtr == (ttVoidPtr)0)
    {
        goto x509CrlNewExit;
    }
    tm_bzero(cerListPtr, sizeof(ttX509CertList));
    cerListPtr->crlAlgorPtr  = (ttX509AlgorIdenPtr)
                        tm_get_raw_buffer(sizeof(ttX509AlgorIden));
    if ( cerListPtr->crlAlgorPtr == (ttX509AlgorIdenPtr)0)
    {
        goto x509CrlNewExit;
    }
    tm_bzero(cerListPtr->crlAlgorPtr, sizeof(ttX509AlgorIden));
    cerListPtr->crlSigPtr    = tfAsn1StringNew(TM_ASN1_TAG_BITSTRING);

    if (cerListPtr->crlSigPtr == (ttAsn1BitStringPtr)0)
    {
        goto x509CrlNewExit;
    }

    cerListPtr->crlTbsCertListPtr  = (ttX509TBSCertListPtr)
                            tm_get_raw_buffer(sizeof(ttX509TBSCertList));

    tbsCertListPtr = cerListPtr->crlTbsCertListPtr;
    if (tbsCertListPtr == (ttX509TBSCertListPtr)0)
    {
        goto x509CrlNewExit;
    }
    tm_bzero(tbsCertListPtr, sizeof(ttX509TBSCertList));

    tfAsn1StringInit(&tbsCertListPtr->tclVersion);

    tbsCertListPtr->tclSignaturePtr  = (ttX509AlgorIdenPtr)
                    tm_get_raw_buffer (sizeof(ttX509AlgorIden));
    if (tbsCertListPtr->tclSignaturePtr == (ttX509AlgorIdenPtr)0)
    {
        goto x509CrlNewExit;
    }
    tm_bzero(tbsCertListPtr->tclSignaturePtr, sizeof(ttX509AlgorIden));
    tbsCertListPtr->tclThisUpdatePtr = (ttAsn1TimePtr)
                    tm_get_raw_buffer (sizeof(ttAsn1Time));
    if (tbsCertListPtr->tclThisUpdatePtr == (ttAsn1TimePtr)0)
    {
        goto x509CrlNewExit;
    }
    tm_bzero(tbsCertListPtr->tclThisUpdatePtr, sizeof(ttAsn1Time));
    tbsCertListPtr->tclNextUpdatePtr = (ttAsn1TimePtr)
                    tm_get_raw_buffer (sizeof(ttAsn1Time));
    if (tbsCertListPtr->tclNextUpdatePtr == (ttAsn1TimePtr)0)
    {
        goto x509CrlNewExit;
    }
    tm_bzero(tbsCertListPtr->tclNextUpdatePtr, sizeof(ttAsn1Time));
    tbsCertListPtr->tclIssuerPtr     = tfX509NameNew();
    if (tbsCertListPtr->tclIssuerPtr == (ttX509NamePtr)0)
    {
        goto x509CrlNewExit;
    }

    for ( i = 0 ; i < TM_PKI_MAX_STACK; i++)
    {
        tbsCertListPtr->tclRevokedPtr[i] = (ttX509RevokedCertPtr)
            tm_get_raw_buffer(sizeof(ttX509RevokedCert));
        if (tbsCertListPtr->tclRevokedPtr[i]== (ttVoidPtr)0)
        {
            goto x509CrlNewExit;
        }
        tm_bzero(tbsCertListPtr->tclRevokedPtr[i], sizeof(ttX509RevokedCert));
    }


    for ( i = 0 ; i < TM_PKI_MAX_STACK; i++)
    {
        tbsCertListPtr->tclCrlExtensionsPtr[i] = (ttX509ExtensionPtr)
            tm_get_raw_buffer(sizeof(ttX509Extension));
        if (tbsCertListPtr->tclCrlExtensionsPtr[i]== (ttVoidPtr)0)
        {
            goto x509CrlNewExit;
        }
        tm_bzero(tbsCertListPtr->tclCrlExtensionsPtr[i],
                 sizeof(ttX509Extension));
    }
    errorCode = TM_ENOERROR;
x509CrlNewExit:
    if (errorCode != TM_ENOERROR)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfX509CRLNew", "out of memory!\n");
#endif /* TM_ERROR_CHECKING */
        if (cerListPtr != (ttX509CertListPtr)0)
        {
            tfX509CRLFree(cerListPtr);
            cerListPtr = (ttX509CertListPtr)0;
        }
    }
    return cerListPtr;
}

int tfX509CRLFree( ttX509CertListPtr cerListPtr)
{
    ttX509TBSCertListPtr    tbsCertListPtr;
    ttX509RevokedCertPtr    revokedCertPtr;
    int                     errorCode;
    int                     i;
    int                     j;

    if ( cerListPtr != (ttX509CertListPtr)0)
    {
        errorCode = TM_ENOERROR;
/* tfAsn1StringFree checks for NULL pointer */
        (void)tfAsn1StringFree(cerListPtr->crlSigPtr);
        if (cerListPtr->crlAlgorPtr != (ttX509AlgorIdenPtr)0)
        {
/* free sigAlgor, and sigValue. tfX509AlgorIdFree checks for NULL pointer */
            (void)tfX509AlgorIdFree(cerListPtr->crlAlgorPtr);
        }
        tbsCertListPtr = cerListPtr->crlTbsCertListPtr;
        if (tbsCertListPtr != (ttX509TBSCertListPtr)0)
        {
/* free sigAlgor, and sigValue. tfX509AlgorIdFree checks for NULL pointer */
            (void)tfX509AlgorIdFree(tbsCertListPtr->tclSignaturePtr);
/* tfAsn1StringFree checks for NULL pointer */
            tfAsn1StringFree(tbsCertListPtr->tclThisUpdatePtr);
/* tfAsn1StringFree checks for NULL pointer */
            tfAsn1StringFree(tbsCertListPtr->tclNextUpdatePtr);
            for ( i = 0 ; i < TM_PKI_MAX_STACK; i++)
            {
                revokedCertPtr = tbsCertListPtr->tclRevokedPtr[i];
                if (revokedCertPtr != (ttX509RevokedCertPtr)0)
                {
/* tfAsn1StringFree checks for NULL pointer */
                    tfAsn1StringFree(revokedCertPtr->revUserCertPtr);
/* tfAsn1StringFree checks for NULL pointer */
                    tfAsn1StringFree(revokedCertPtr->revRevocationDatePtr);
                    for (j = 0; j < TM_PKI_MAX_STACK; j++)
                    {
                        if (revokedCertPtr->revCrlEntryExtPtr[j] != 0)
                        {
/* tfAsn1StringFree checks for NULL pointer */
                            tfAsn1StringFree(
                              revokedCertPtr->revCrlEntryExtPtr[j]->extnValuePtr);
                            tm_free_raw_buffer((tt8BitPtr)(ttVoidPtr)
                                            revokedCertPtr->revCrlEntryExtPtr[j]);
                        }
                    }
                    tm_free_raw_buffer((tt8BitPtr)(ttVoidPtr)revokedCertPtr);
                }
            }
            for ( i = 0 ; i < TM_PKI_MAX_STACK; i++)
            {
                if (tbsCertListPtr->tclCrlExtensionsPtr[i]
                                                 != (ttX509ExtensionPtr)0)
                {
                    (void)tfAsn1StringFree(
                           tbsCertListPtr->tclCrlExtensionsPtr[i]->extnValuePtr);
                    tm_free_raw_buffer((tt8BitPtr)(ttVoidPtr)
                                       tbsCertListPtr->tclCrlExtensionsPtr[i]);
                }
            }
            if (tbsCertListPtr->tclIssuerPtr != (ttX509NamePtr)0)
            {
                tfX509NameFree(tbsCertListPtr->tclIssuerPtr);
            }
            tm_free_raw_buffer((tt8BitPtr)(ttVoidPtr)tbsCertListPtr);
        }
        tm_free_raw_buffer((tt8BitPtr)(ttVoidPtr)cerListPtr);
    }
    else
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfX509CRLFree", "null pointer!\n");
#endif /* TM_ERROR_CHECKING */
        errorCode = TM_EINVAL;
    }
    return errorCode;
}

/* DER encodng and decoding */
int tfX509GenericDecode (     ttVoidPtr     vPtr,
                              tt8BitPtrPtr  derPtrPtr,
                              ttIntPtr      lengthPtr,
                              int           type)
{
    tt8BitPtr           temp8Ptr;
    tt8BitPtr           listHeadPtr;
    tt8BitPtr           tempSeqHeadPtr;
    ttAsn1IntegerPtr    asn1IntPtr;
    int                 x509Version;
    int                 tlen;
    int                 totalLen;
    int                 listTotalLen;
    int                 retLen;
    int                 j;

    tlen    = 0;
    retLen  = 0;
    tempSeqHeadPtr = (tt8BitPtr)0;
/* Assume failure */
    *lengthPtr = -1;
    if (( vPtr == (ttVoidPtr)0) || (derPtrPtr == (ttVoidPtr)0) ||
            ( *derPtrPtr == (ttVoidPtr)0))
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfX509GenericDecode", "null pointer!\n");
#endif /* TM_ERROR_CHECKING */
        goto x509GeneralDecExit;
    }
    temp8Ptr  = *derPtrPtr;

    totalLen = tm_asn1_sequence_dec((ttVoidPtr)0, &temp8Ptr);
    if (totalLen < 0)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfX509GenericDecode", "error decoding length!\n");
#endif /* TM_ERROR_CHECKING */
        goto x509GeneralDecExit;
    }

/* recursive function call here */
    switch (type)
    {
        case TM_X509_CERT:
            retLen = tm_x509_generic_dec(((ttX509CertPtr)vPtr)->certTbsPtr,
                                         &temp8Ptr,
                                         &tlen,
                                         TM_X509_TBSCERT);
            if (retLen >= 0)
            {
                retLen =
                    tm_x509_generic_dec(((ttX509CertPtr)vPtr)->certAlgorPtr ,
                                        &temp8Ptr,  &tlen, TM_X509_ALGORID);
            }
            if (retLen >= 0)
            {
                retLen =
                      tm_asn1_bitstring_dec(&((ttX509CertPtr)vPtr)->certSigPtr,
                                            &temp8Ptr);
            }
            break;
        case TM_X509_CERTREQ:
            retLen = tm_x509_generic_dec(
                    ((ttX509CertReqPtr)vPtr)->certReqInfoPtr,
                    &temp8Ptr,
                    &tlen,
                    TM_X509_CERTREQINFO);
            if (retLen >= 0)
            {
                retLen = tm_x509_generic_dec(
                    ((ttX509CertReqPtr)vPtr)->certReqAlgorPtr,
                    &temp8Ptr,
                    &tlen,
                    TM_X509_ALGORID);
            }
            if (retLen >= 0)
            {
                retLen = tm_asn1_bitstring_dec(
                        &((ttX509CertReqPtr)vPtr)->certReqSigPtr,
                        &temp8Ptr);
            }
            break;
        case TM_X509_CERTLIST:
            retLen = tm_x509_generic_dec(
                    ((ttX509CertListPtr)vPtr)->crlTbsCertListPtr,
                    &temp8Ptr,
                    &tlen,
                    TM_X509_TBSCERTLIST);
            if (retLen >= 0)
            {
                retLen = tm_x509_generic_dec(
                    ((ttX509CertListPtr)vPtr)->crlAlgorPtr ,
                    &temp8Ptr,
                    &tlen,
                    TM_X509_ALGORID);
            }
            if (retLen >= 0)
            {
                retLen = tm_asn1_bitstring_dec(
                    &((ttX509CertListPtr)vPtr)->crlSigPtr,
                    &temp8Ptr);
            }
            break;
        case TM_X509_TBSCERT:

/* for Openssl, comment serialNumber and following line */
/* version & serial number */
            if ( *temp8Ptr != 0xa0 )
            {
/* some OpenSSL certificate did not have serial number and a0 tag */
                asn1IntPtr = &((ttX509TBSCertPtr)vPtr)->tbsVersion;
                retLen = tm_asn1_integer_dec ( &asn1IntPtr, &temp8Ptr);
            }
            else
            {
                temp8Ptr++;
                retLen = tm_asn1_context_dec(&j ,&temp8Ptr);
                if (retLen >= 0)
                {
/* version */
                    asn1IntPtr = &((ttX509TBSCertPtr)vPtr)->tbsVersion;
                    retLen = tm_asn1_integer_dec ( &asn1IntPtr, &temp8Ptr);
                }
                if (retLen >= 0)
                {
/* serialNumber */
                    asn1IntPtr = &((ttX509TBSCertPtr)vPtr)->tbsSerialNumber;
                    retLen = tm_asn1_integer_dec ( &asn1IntPtr, &temp8Ptr);
                }
            }
            if (retLen >= 0)
            {
/* signature */
                retLen = tm_x509_generic_dec(
                   ((ttX509TBSCertPtr)vPtr)->tbsSignaturePtr,
                   &temp8Ptr, &tlen, TM_X509_ALGORID);
            }
            if (retLen >= 0)
            {
/* issuer */
                retLen = tm_x509_generic_dec(
                   ((ttX509TBSCertPtr)vPtr)->tbsIssuerPtr,
                   &temp8Ptr, &tlen, TM_X509_NAME);
            }
            if (retLen >= 0)
            {
/* validity */
                retLen = tm_x509_generic_dec(
                   ((ttX509TBSCertPtr)vPtr)->tbsValidityPtr,
                   &temp8Ptr, &tlen, TM_X509_VALIDITY);
            }
            if (retLen >= 0)
            {
/* subject */
                retLen = tm_x509_generic_dec(
                   ((ttX509TBSCertPtr)vPtr)->tbsSubjectPtr,
                   &temp8Ptr, &tlen, TM_X509_NAME);
            }
            if (retLen >= 0)
            {
/* subjectPublicKeyInfo */
                retLen = tm_x509_generic_dec(
                   ((ttX509TBSCertPtr)vPtr)->tbsSubPkInfoPtr,
                   &temp8Ptr, &tlen, TM_X509_SUBPKINFO);
            }

            if (    ( totalLen == (int)(temp8Ptr - *derPtrPtr) )
                 || (retLen < 0) )
            {
                break;
            }
/* issuerUniqueId */
            if ( *temp8Ptr == 0x81)
            {
                retLen = tm_asn1_bitstring_dec     (
                            &((ttX509TBSCertPtr)vPtr)->tbsIssuerUIDPtr ,
                            &temp8Ptr);
                if (    ( totalLen == (int)(temp8Ptr - *derPtrPtr) )
                     || (retLen < 0) )
                {
                    break;
                }
            }
/* subjectUniqueId */
            if ( *temp8Ptr == 0x82)
            {
                retLen = tm_asn1_bitstring_dec     (
                            &((ttX509TBSCertPtr)vPtr)->tbsSubjectUIDPtr ,
                            &temp8Ptr);
                if (    ( totalLen == (int)(temp8Ptr - *derPtrPtr) )
                     || (retLen < 0) )
                {
                    break;
                }
            }
/* extensions */
            if ( *temp8Ptr == 0xa3)
            {
                temp8Ptr++;
                retLen = tm_asn1_context_dec(&j ,&temp8Ptr);
                if (retLen >= 0)
                {
/* sequence of extension */
                    retLen = tm_asn1_sequence_dec((ttVoidPtr)0, &temp8Ptr);
                    if (retLen >= 0)
                    {
                        for ( j = 0; j < TM_PKI_MAX_STACK; j++)
                        {
                            if ( totalLen == (int)(temp8Ptr - *derPtrPtr) )
                            {
                                break;
                            }

                            retLen = tm_x509_generic_dec    (
                                ((ttX509TBSCertPtr)vPtr)->tbsExtensionsPtr[j],
                                &temp8Ptr, &tlen, TM_X509_EXTENSION);
                            if ( retLen < 0)
                            {
                                break;
                            }
                        }
                    }
                }
            }
            break;
        case TM_X509_ALGORID:
/* object does not need to allocate memory */
            retLen = tm_asn1_object_dec (
                            &((ttX509AlgorIdenPtr)vPtr)->algorObjPtr ,
                            &temp8Ptr);
            if (    ( totalLen == (int)(temp8Ptr - *derPtrPtr) )
                 || (retLen < 0) )
            {
                break;
            }
            if (( *temp8Ptr == 0x05) || ( *temp8Ptr == 0x04))
            {
/* NULL type for RSA */
                retLen = tm_asn1_type_dec(
                                &((ttX509AlgorIdenPtr)vPtr)->algorParPtr,
                                &temp8Ptr);
            }
            else
            {
/* DSA grammer: seq of p,q,and g. */
                ((ttX509AlgorIdenPtr)vPtr)->algorParPtr =
                        tfAsn1StringNew(TM_ASN1_TAG_OCTETSTRING);
                if (((ttX509AlgorIdenPtr)vPtr)->algorParPtr ==
                                (ttAsn1StringPtr)0)
                {
#ifdef TM_ERROR_CHECKING
                    tfKernelWarning("tfX509GenericDecode", "no memory!\n");
#endif /* TM_ERROR_CHECKING */
                    goto x509GeneralDecExit;
                }
                tlen = (int)(totalLen - (int)(temp8Ptr - *derPtrPtr));
                if ( tlen < 0)
                {
#ifdef TM_ERROR_CHECKING
                    tfKernelWarning("tfX509GenericDecode",
                                    "error decoding length!\n");
#endif /* TM_ERROR_CHECKING */
                    goto x509GeneralDecExit;
                }
                ((ttX509AlgorIdenPtr)vPtr)->algorParPtr->asn1Data =
                        tm_get_raw_buffer( (ttPktLen)tlen);
                if (((ttX509AlgorIdenPtr)vPtr)->algorParPtr->asn1Data
                                == (tt8BitPtr)0)
                {
#ifdef TM_ERROR_CHECKING
                    tfKernelWarning("tfX509GenericDecode", "no memory!\n");
#endif /* TM_ERROR_CHECKING */
                    goto x509GeneralDecExit;
                }
                ((ttX509AlgorIdenPtr)vPtr)->algorParPtr->asn1Length = tlen;
                tm_bcopy(temp8Ptr,
                        ((ttX509AlgorIdenPtr)vPtr)->algorParPtr->asn1Data,
                        tlen);
                temp8Ptr += tlen;
            }
            break;
        case TM_X509_NAME:
            tlen = 0;
            for ( j = 0 ; j < TM_PKI_MAX_STACK; j++)
            {
                if ( totalLen == (int)(temp8Ptr - *derPtrPtr) )
                {
                    break;
                }
                retLen = tm_asn1_set_dec ((ttVoidPtr)0, &temp8Ptr);
                if ( retLen < 0)
                {
                    break;
                }
                if ( totalLen == (int)(temp8Ptr - *derPtrPtr) )
                {
                    break;
                }
                retLen = tm_x509_generic_dec(
                            ((ttX509NamePtr)vPtr)->nameRelativeDNPtr[j],
                            &temp8Ptr, &tlen, TM_X509_ATTRTYPEVALUE);
                if ( retLen < 0)
                {
                    break;
                }
            }
            ((ttX509NamePtr)vPtr)->nameNum = j;
            break;
        case TM_X509_VALIDITY:
            retLen = tm_asn1_time_dec(
                            &((ttX509ValidityPtr)vPtr)->notBeforePtr,
                            &temp8Ptr);
            if (retLen >= 0)
            {
                retLen = tm_asn1_time_dec(
                            &((ttX509ValidityPtr)vPtr)->notAfterPtr,
                            &temp8Ptr);
            }
            break;
        case TM_X509_SUBPKINFO:
            if ( ((ttX509SubPubKeyInfoPtr)vPtr)->subPkAlgorPtr
                    == (ttVoidPtr)0)
            {
                ((ttX509SubPubKeyInfoPtr)vPtr)->subPkAlgorPtr
                    = (ttX509AlgorIdenPtr)
                        tm_get_raw_buffer(sizeof(ttX509AlgorIden));
                if (((ttX509SubPubKeyInfoPtr)vPtr)->subPkAlgorPtr
                                == (ttX509AlgorIdenPtr)0)
                {
#ifdef TM_ERROR_CHECKING
                    tfKernelWarning("tfX509GenericDecode", "no memory!\n");
#endif /* TM_ERROR_CHECKING */
                    goto x509GeneralDecExit;
                }
                tm_bzero(((ttX509SubPubKeyInfoPtr)vPtr)->subPkAlgorPtr,
                        sizeof(ttX509AlgorIden));

            }
            retLen = tm_x509_generic_dec(
                            ((ttX509SubPubKeyInfoPtr)vPtr)->subPkAlgorPtr,
                            &temp8Ptr, &tlen, TM_X509_ALGORID);
            if (retLen >= 0)
            {
                retLen = tm_asn1_bitstring_dec(
                            &((ttX509SubPubKeyInfoPtr)vPtr)->subPkValuePtr,
                            &temp8Ptr);
            }
            break;
        case TM_X509_EXTENSION:
            retLen = tm_asn1_object_dec(
                            &((ttX509ExtensionPtr)vPtr)->extnIDPtr ,
                            &temp8Ptr);
            if (retLen >= 0)
            {
/* Boolean false is zero octet */
                retLen = tm_asn1_boolean_dec(
                                &((ttX509ExtensionPtr)vPtr)->extnCritical,
                                &temp8Ptr);
            }
            if (retLen >= 0)
            {
                retLen = tm_asn1_octetstring_dec(
                            &((ttX509ExtensionPtr)vPtr)->extnValuePtr ,
                            &temp8Ptr);
            }
            break;
        case TM_X509_ATTRTYPEVALUE:
            retLen = tm_asn1_object_dec(
                            &((ttX509AttrTypeValuePtr)vPtr)->atvObjPtr ,
                            &temp8Ptr);
            if (retLen >= 0)
            {
                retLen = tm_asn1_string_dec(
                            &((ttX509AttrTypeValuePtr)vPtr)->atvValuePtr,
                            &temp8Ptr);
            }
            break;
        case TM_X509_CERTREQINFO:
            asn1IntPtr = &((ttX509CertReqInfoPtr)vPtr)->reqInfoVersion;
            retLen = tm_asn1_integer_dec( &asn1IntPtr, &temp8Ptr);
            if (retLen >= 0)
            {
                retLen = tm_x509_generic_dec(
                    ((ttX509CertReqInfoPtr)vPtr)->reqInfoSubjectPtr,
                    &temp8Ptr, &tlen, TM_X509_NAME);
            }
            if (retLen >= 0)
            {
                retLen = tm_x509_generic_dec(
                    ((ttX509CertReqInfoPtr)vPtr)->reqInfoSubPkInfoPtr,
                    &temp8Ptr, &tlen,TM_X509_SUBPKINFO);
            }
            if (retLen >= 0)
            {
                for ( j = 0 ; j < TM_PKI_MAX_STACK; j++)
                {
                    if ( totalLen == (int)(temp8Ptr - *derPtrPtr) )
                    {
                        break;
                    }
                    retLen = tm_x509_generic_dec (
                                ((ttX509CertReqInfoPtr)vPtr)->reqInfoSubPkInfoPtr,
                                &temp8Ptr, &tlen,TM_X509_ATTRIBUTE);
                    if ( retLen < 0)
                    {
                        break;
                    }
                }
            }
            break;
        case TM_X509_ATTRIBUTE:
            retLen = tm_asn1_object_dec    (
                    &((ttX509AttributePtr)vPtr)->attObjPtr,
                    &temp8Ptr);
/* not clear: set, asn1string
 */
            break;
        case TM_X509_TBSCERTLIST:
            x509Version = TM_X509_CRL_VERSION_1; /* assume version is 1 */
            retLen = 0;
            if (*temp8Ptr == 0x02)
            {
                x509Version = TM_X509_CRL_VERSION_2;
                asn1IntPtr = &((ttX509TBSCertListPtr)vPtr)->tclVersion;
                retLen = tm_asn1_integer_dec( &asn1IntPtr, &temp8Ptr);
                if (retLen < 0)
                {
#ifdef TM_ERROR_CHECKING
                    tfKernelWarning("tfX509GenericDecode", 
                                    "Cannot decode CRL version!\n");
#endif /* TM_ERROR_CHECKING */
                    goto x509GeneralDecExit;
                }
            }
            retLen = tm_x509_generic_dec(
                        ((ttX509TBSCertListPtr)vPtr)->tclSignaturePtr,
                        &temp8Ptr, &tlen, TM_X509_ALGORID);
            if (retLen >= 0)
            {
                retLen = tm_x509_generic_dec(
                        ((ttX509TBSCertListPtr)vPtr)->tclIssuerPtr,
                        &temp8Ptr, &tlen, TM_X509_NAME);
            }
            if (retLen >= 0)
            {
                retLen = tm_asn1_time_dec(
                        &((ttX509TBSCertListPtr)vPtr)->tclThisUpdatePtr,
                        &temp8Ptr);
            }
            if (retLen >= 0)
            {
                retLen = tm_asn1_time_dec(
                        &((ttX509TBSCertListPtr)vPtr)->tclNextUpdatePtr,
                        &temp8Ptr);
            }
            if (retLen >= 0)
            {
                tempSeqHeadPtr = temp8Ptr;
                retLen = tm_asn1_sequence_dec((ttVoidPtr)0, &temp8Ptr);
            }
            if (retLen >= 0)
            {
                listTotalLen = retLen - (int)(temp8Ptr - tempSeqHeadPtr);
                listHeadPtr = temp8Ptr;
                for ( j = 0; j < TM_PKI_MAX_STACK; j++)
                {
                    if ( listTotalLen == (int)(temp8Ptr - listHeadPtr) )
                    {
                        break;
                    }
                    retLen = tm_x509_generic_dec    (
                            ((ttX509TBSCertListPtr)vPtr)->tclRevokedPtr[j],
                            &temp8Ptr, &tlen, TM_X509_REVOKEDCERT);
                    if ( retLen < 0)
                    {
                        break;
                    }
                }
            }
            if ((retLen >= 0)
                && (x509Version == TM_X509_CRL_VERSION_2))
            {
                if ((*temp8Ptr & 0xE0) == (TM_ASN1_CONTEXT_SPECIFIC_CLASS 
                                           | TM_ASN1_CONSTRUCTED_TYPE))  
                {
                    temp8Ptr++; /* skip head */
                    retLen = tm_asn1_context_dec((ttVoidPtr)&tlen, &temp8Ptr); 
                    if (retLen < 0)
                    {
#ifdef TM_ERROR_CHECKING
                        tfKernelWarning("tfX509GenericDecode", 
                                        "error decoding length!\n");
#endif /* TM_ERROR_CHECKING */
                        goto x509GeneralDecExit;
                    }
                    retLen = tm_asn1_sequence_dec((ttVoidPtr)0, &temp8Ptr);
                    if (retLen < 0)
                    {
#ifdef TM_ERROR_CHECKING
                        tfKernelWarning("tfX509GenericDecode", 
                                        "error decoding length!\n");
#endif /* TM_ERROR_CHECKING */
                        goto x509GeneralDecExit;
                    }
                }
                for ( j = 0; j < TM_PKI_MAX_STACK; j++)
                {
                    if ( totalLen == (int)(temp8Ptr - *derPtrPtr) )
                    {
                        break;
                    }
                    retLen = tm_x509_generic_dec    (
                        ((ttX509TBSCertListPtr)vPtr)->tclCrlExtensionsPtr[j],
                        &temp8Ptr, &tlen, TM_X509_EXTENSION);
                    if ( retLen < 0)
                    {
                        break;
                    }
                }
            }
            break;
        case TM_X509_REVOKEDCERT:
            retLen = tm_asn1_integer_dec    (
                        &((ttX509RevokedCertPtr)vPtr)->revUserCertPtr,
                        &temp8Ptr);
            if ( retLen >= 0)
            {
                retLen = tm_asn1_time_dec    (
                        &((ttX509RevokedCertPtr)vPtr)->revRevocationDatePtr,
                        &temp8Ptr);
            }
            if (retLen >= 0)
            {
                for ( j = 0; j < TM_PKI_MAX_STACK; j++)
                {
                    if ( totalLen == (int)(temp8Ptr - *derPtrPtr) )
                    {
                        break;
                    }
                    retLen = tm_x509_generic_dec    (
                            ((ttX509RevokedCertPtr)vPtr)->revCrlEntryExtPtr[j],
                            &temp8Ptr, &tlen, TM_X509_EXTENSION);
                    if ( retLen < 0)
                    {
                        break;
                    }
                }
            }
            break;
        default:
            break;
    }
    if (retLen != -1)
    {
        if ( totalLen != (int)(temp8Ptr - *derPtrPtr) )
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfX509GenericDecode", "length error!\n");
#endif /* TM_ERROR_CHECKING */
        }
        else
        {
            *derPtrPtr  = temp8Ptr;
            *lengthPtr  = totalLen;
        }
    }
x509GeneralDecExit:
    if (*lengthPtr == -1)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfX509GenericDecode", "Error decoding!\n");
#endif /* TM_ERROR_CHECKING */
    }
    return *lengthPtr;
}

/*
 * GenericFunc: Signature and Verify for Cert, CertRequest, CRL
 */
#if (defined(TM_PUBKEY_USE_RSA) || defined(TM_PUBKEY_USE_DSA))

int tfX509GenericFunc (     int          length,
                            ttVoidPtr    source,
                            ttIntPtr     sigLenPtr,
                            tt8BitPtrPtr sigDataPtrPtr,
                            ttVoidPtr    keyPtr,
                            int          method,
                            int          type )
{
    ttCryptoEnginePtr               cePtr;
    ttCryptoRequest                 request;
    ttCryptoRequestHash             hashreq;
    ttCryptoChainData               inhash;
#ifdef TM_PUBKEY_USE_RSA
    ttPkiGlobalPtr                  pkiPtr;
    ttCryptoRequestRsarequest       rsareq;
    ttX509AlgorIden                 aId;
    ttAsn1String                    aOct;
#endif /* TM_PUBKEY_USE_RSA */
#ifdef TM_PUBKEY_USE_DSA
    ttCryptoRequestDsarequest       dsareq;
#endif /* TM_PUBKEY_USE_DSA */
    tt8BitPtr                       sigPtr;
    tt8BitPtr                       aHash;
    tt8BitPtr                       allocAHash;
    ttAhAlgorithmPtr                hashAlgPtr;
    tt8Bit                          hash[32];
    int                             len;
    int                             tlen;
    int                             errorCode;
    int                             algorithm;
    tt16Bit                         hashAlg;



    aHash       = (tt8BitPtr)0;
    allocAHash  = (tt8BitPtr)0;
    tlen        = 0;
    sigPtr      = (tt8BitPtr)0;
    errorCode   = TM_ENOERROR;
    hashAlg     = 0;
/* RSA r DSA length */
    switch (method )
    {
#ifdef TM_PUBKEY_USE_RSA
        case TM_PKI_OBJ_RSAMD2     :
        case TM_PKI_OBJ_RSAMD5     :
        case TM_PKI_OBJ_RSASHA1    :
        case TM_PKI_OBJ_RSASHA256  :
        case TM_PKI_OBJ_RSASHA384  :
        case TM_PKI_OBJ_RSASHA512  :
            tlen = tm_pki_rsa_size(keyPtr);
            break;
#endif /* TM_PUBKEY_USE_RSA */
#ifdef TM_PUBKEY_USE_DSA
        case TM_PKI_OBJ_DSASHA1:
            tlen = tm_pki_dsa_size(keyPtr);
            break;
#endif /* TM_PUBKEY_USE_DSA */
        case TM_PKI_OBJ_RSARIPEMD  :
        default:
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfX509CenericSign", "algoirthm error!\n");
#endif /* TM_ERROR_CHECKING */
            errorCode = TM_EINVAL;
            goto x509GenericFuncExit;
    }

/* length of signature */
    sigPtr = tm_get_raw_buffer((ttPktLen)tlen);
    if ( sigPtr == (ttVoidPtr)0)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfX509CenericSign", "out of memory!\n");
#endif /* TM_ERROR_CHECKING */
        errorCode = TM_ENOBUFS;
        goto x509GenericFuncExit;
    }

/* Hash */
    switch (method)
    {
        case TM_PKI_OBJ_RSAMD2     :
            hashAlg = SADB_AALG_MD2HMAC;
            tlen = TM_MD2_HASH_SIZE;
            break;
        case TM_PKI_OBJ_RSAMD5     :
            hashAlg = SADB_AALG_MD5HMAC;
            tlen = TM_MD5_HASH_SIZE;
            break;
        case TM_PKI_OBJ_RSASHA1    :
        case TM_PKI_OBJ_DSASHA1    :
            hashAlg = SADB_AALG_SHA1HMAC;
            tlen = TM_SHA1_HASH_SIZE;
            break;
        case TM_PKI_OBJ_RSASHA256  :
            hashAlg = SADB_AALG_SHA256HMAC;
            tlen = TM_SHA256_HASH_SIZE;
            break;
        case TM_PKI_OBJ_RSASHA384  :
            hashAlg = SADB_AALG_SHA384HMAC;
            tlen = TM_SHA384_HASH_SIZE;
            break;
        case TM_PKI_OBJ_RSASHA512  :
            hashAlg = SADB_AALG_SHA512HMAC;
            tlen = TM_SHA512_HASH_SIZE;
            break;
        case TM_PKI_OBJ_RSARIPEMD  :
            break;
        default:
            break;
    }

    if(hashAlg != 0)
    {
        hashAlgPtr = tfAhAlgorithmLookup(hashAlg);
        if(!hashAlgPtr)
        {
            errorCode = -1;
            goto x509GenericFuncExit;
        }

        tm_bzero(&request, sizeof(request));
        request.crType          = TM_CEREQUEST_CRYPTO_HASH;
        request.crParamUnion.crHashParamPtr  = &hashreq;
        hashreq.crhAlgPtr       = hashAlgPtr;
        hashreq.crhHashAlg      = hashAlg;

        inhash.ccdDataLen       = (ttPktLen)length;
        inhash.ccdDataPtr       = (tt8BitPtr)source;
        inhash.ccdNextPtr       = (ttCryptoChainDataPtr)0;
        hashreq.crhInDataPtr    = &inhash;

        hashreq.crhOutPtr       = (tt8BitPtr)hash;
/* supposed to use first crypto engine to do hash */
        cePtr = tfCryptoEngineGet(hashAlg);
        if (cePtr)
        {
            cePtr->ceSessionProcessFuncPtr(&request);
        }
        else
        {
            errorCode = -1;
            goto x509GenericFuncExit;
        }
    }

#ifdef TM_PUBKEY_USE_RSA
    if( method == TM_PKI_OBJ_RSAMD2 ||
        method == TM_PKI_OBJ_RSAMD5 ||
        method == TM_PKI_OBJ_RSASHA1 ||
        method == TM_PKI_OBJ_RSASHA256 ||
        method == TM_PKI_OBJ_RSASHA384 ||
        method == TM_PKI_OBJ_RSASHA512)
    {
        pkiPtr = (ttPkiGlobalPtr)tm_context(tvPkiPtr);
        if (    (pkiPtr != (ttPkiGlobalPtr)0)
             && (pkiPtr->pkiObjectPtr != (ttAsn1ObjectPtr)0))
        {
/* sign : sequence of algor, octetstring of hash, according to RFC 2313
 * FOR RSA
 *   DigestInfo ::= SEQUENCE {
 *   digestAlgorithm DigestAlgorithmIdentifier,
 *   digest Digest }
 */
            len = 0;
            aHash = (tt8BitPtr)0;
            if ( method == TM_PKI_OBJ_RSAMD5)
            {
                aId.algorObjPtr  = (ttAsn1ObjectPtr)
                                        &pkiPtr->pkiObjectPtr[TM_PKI_OBJ_MD5];
            }
            else if (method == TM_PKI_OBJ_RSAMD2)
            {
                aId.algorObjPtr  = (ttAsn1ObjectPtr)
                                        &pkiPtr->pkiObjectPtr[TM_PKI_OBJ_MD2];
            }
            else if (method == TM_PKI_OBJ_RSASHA1)
            {
                aId.algorObjPtr  = (ttAsn1ObjectPtr)
                                        &pkiPtr->pkiObjectPtr[TM_PKI_OBJ_SHA1];
            }
            else if (method == TM_PKI_OBJ_RSASHA256)
            {
                aId.algorObjPtr  = 
                    (ttAsn1ObjectPtr)&pkiPtr->pkiObjectPtr[TM_PKI_OBJ_SHA256];
            }
            else if (method == TM_PKI_OBJ_RSASHA384)
            {
                aId.algorObjPtr  = 
                    (ttAsn1ObjectPtr)&pkiPtr->pkiObjectPtr[TM_PKI_OBJ_SHA384];
            }
            else
            {
                aId.algorObjPtr  = 
                    (ttAsn1ObjectPtr)&pkiPtr->pkiObjectPtr[TM_PKI_OBJ_SHA512];
            }
            aId.algorParPtr = 0;
            aOct.asn1Length = tlen;
            aOct.asn1Type   = TM_ASN1_TAG_OCTETSTRING;
            aOct.asn1Data   = &hash[0];
/* need to calculate twice to store */

            len += tm_x509_generic_enc(&aId,&aHash,&tlen,TM_X509_ALGORID);
            len += tm_asn1_octetstring_enc(&aOct,&aHash,0);
            tlen = tm_asn1_sequence_enc ( (ttVoidPtr)0, &aHash, len);
            tlen +=len;
            allocAHash = tm_get_raw_buffer((ttPktLen)tlen);
            if ( allocAHash == (ttVoidPtr)0)
            {
#ifdef TM_ERROR_CHECKING
                tfKernelWarning("tfX509CenericSign", "out of memory!\n");
#endif /* TM_ERROR_CHECKING */
                errorCode = TM_ENOBUFS;
                goto x509GenericFuncExit;
            }
            aHash = allocAHash;
            (void)tm_asn1_sequence_enc ((ttVoidPtr)0, &aHash,len);
            (void)tm_x509_generic_enc(&aId,&aHash,&len,TM_X509_ALGORID);
            (void)tm_asn1_octetstring_enc(&aOct,&aHash,0);
            aHash = allocAHash;
        }
        else
        {
            errorCode = TM_ENOENT;
            goto x509GenericFuncExit;
        }
    }
    else
#endif /* TM_PUBKEY_USE_RSA */
    if (method == TM_PKI_OBJ_DSASHA1)
    {
        aHash = &hash[0];
    }
    else
    {
        errorCode = TM_EINVAL;
        goto x509GenericFuncExit;
    }

/* signature */
/* check rsa/dsa engine */
    if ( (method == TM_PKI_OBJ_DSASHA1) || (method == TM_PKI_OBJ_DSA) )
    {
        algorithm = SADB_PUBKEY_DSA;
    }
    else
    {
        algorithm = SADB_PUBKEY_RSA;
    }
    cePtr  = tfCryptoEngineGet(algorithm);
    if (cePtr == (ttCryptoEnginePtr)0)
    {
/* this Diffie-Hellman group is not supported by any crypto engine */
        errorCode = -1;
        goto x509GenericFuncExit;
    }

    if ( type == TM_X509_SIGNATURE)
    {
        len = 0;
#ifdef TM_PUBKEY_USE_DSA
        if (( method == TM_PKI_OBJ_DSASHA1) ||
            ( method == TM_PKI_OBJ_DSA) )
        {
            request.crType          = TM_CEREQUEST_PUBKEY_DSASIGN;
            request.crParamUnion.crDsaParamPtr   = &dsareq;
            dsareq.crdDsainfoPtr    = (ttGeneralDsaPtr)keyPtr;
            dsareq.crdDsaDataLength = (tt16Bit)tlen;
            dsareq.crdDsaDataPtr    = aHash;
            dsareq.crdDsaTransPtr   = sigPtr;
            dsareq.crdDsaTransLenPtr = &len;

            (void)cePtr->ceSessionProcessFuncPtr(&request);

        }
#endif /* TM_PUBKEY_USE_DSA */

#ifdef TM_PUBKEY_USE_RSA
        if(method == TM_PKI_OBJ_RSA    ||
                method == TM_PKI_OBJ_RSAMD5 ||
                method == TM_PKI_OBJ_RSASHA1||
                method == TM_PKI_OBJ_RSASHA256||
                method == TM_PKI_OBJ_RSASHA384||
                method == TM_PKI_OBJ_RSASHA512||
                method == TM_PKI_OBJ_RSARIPEMD)
        {
            request.crType          = TM_CEREQUEST_PUBKEY_RSASIGN;
            request.crParamUnion.crRsaParamPtr   = &rsareq;
            rsareq.crrRsainfoPtr    = (ttGeneralRsaPtr)keyPtr;
            rsareq.crrRsaDataLength = (tt16Bit)tlen;
            rsareq.crrRsaDataPtr    = aHash;
            rsareq.crrRsaTransPtr   = sigPtr;
            rsareq.crrRsaTransLenPtr = &len;

            (void)cePtr->ceSessionProcessFuncPtr(&request);
         }
#endif /* TM_PUBKEY_USE_RSA */

        *sigLenPtr      = len;
        *sigDataPtrPtr  = sigPtr;
        sigPtr = (tt8BitPtr)0;
        goto x509GenericFuncExit;
    }
    else if ( type == TM_X509_VERIFY)
    {

#ifdef TM_PUBKEY_USE_DSA
        if (( method == TM_PKI_OBJ_DSASHA1)||
            ( method == TM_PKI_OBJ_DSA) )
        {
            request.crType          = TM_CEREQUEST_PUBKEY_DSAVERIFY;
            request.crParamUnion.crDsaParamPtr   = &dsareq;
            dsareq.crdDsainfoPtr    = (ttGeneralDsaPtr)keyPtr;
            dsareq.crdDsaDataLength = (tt16Bit)tlen;
            dsareq.crdDsaDataPtr    = aHash;
            dsareq.crdDsaTransPtr   = (tt8BitPtr)(*sigDataPtrPtr);
            dsareq.crdDsaTransLenPtr = sigLenPtr;
            (void)cePtr->ceSessionProcessFuncPtr(&request);
            errorCode = request.crResult;
        }
#endif /* TM_PUBKEY_USE_DSA */

#ifdef TM_PUBKEY_USE_RSA
        if(method == TM_PKI_OBJ_RSA    ||
                method == TM_PKI_OBJ_RSAMD2 ||
                method == TM_PKI_OBJ_RSAMD5 ||
                method == TM_PKI_OBJ_RSASHA1||
                method == TM_PKI_OBJ_RSASHA256||
                method == TM_PKI_OBJ_RSASHA384||
                method == TM_PKI_OBJ_RSASHA512||
                method == TM_PKI_OBJ_RSARIPEMD)
        {
            request.crType          = TM_CEREQUEST_PUBKEY_RSAVERIFY;
            request.crParamUnion.crRsaParamPtr   = &rsareq;
            rsareq.crrRsainfoPtr    = (ttGeneralRsaPtr)keyPtr;
            rsareq.crrRsaDataLength = (tt16Bit)tlen;
            rsareq.crrRsaDataPtr    = aHash;
            rsareq.crrRsaTransPtr   = (tt8BitPtr)(*sigDataPtrPtr);
            rsareq.crrRsaTransLenPtr = sigLenPtr;
            (void)cePtr->ceSessionProcessFuncPtr(&request);
            errorCode = request.crResult;
        }
#endif /* TM_PUBKEY_USE_RSA */
    }
x509GenericFuncExit:
    if (allocAHash != (tt8BitPtr)0)
    {
        tm_free_raw_buffer(allocAHash);
    }
    if (sigPtr != (tt8BitPtr)0)
    {
        tm_free_raw_buffer(sigPtr);
    }
    return errorCode;
}


int tfX509CertVerify(ttVoidPtr reqin, ttVoidPtr cPtr, ttVoidPtr keyin)
{
    ttX509CertPtr       certPtr;
    ttPkiPublicKeyPtr   pkey;
    ttPkiGlobalPtr      pkiPtr;
    tt8BitPtr           temp8Ptr;
    tt8BitPtr           tbsPtr;
    int                 length;
    int                 errorCode;

    errorCode   = TM_ENOERROR;
    pkey        = (ttPkiPublicKeyPtr)keyin;
    if ( reqin == (ttVoidPtr)0)
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        pkiPtr = (ttPkiGlobalPtr)tm_context(tvPkiPtr);
        if (    (pkiPtr != (ttPkiGlobalPtr)0)
             && (pkiPtr->pkiObjectPtr != (ttAsn1ObjectPtr)0))
        {
/* decode and verify DER of certificate  */
/* in the current, length is first four bytes in req_in */
            temp8Ptr = reqin;
            length = tm_asn1_sequence_dec( (ttVoidPtr)0, &temp8Ptr);
            tbsPtr = temp8Ptr;
            length = tm_asn1_sequence_dec( (ttVoidPtr)0, &temp8Ptr);

            if (length >= 0)
            {

                certPtr = (ttX509CertPtr)cPtr;

/* verify from tbsCertificate, need to remove length and sequence */
                errorCode = tfX509GenericFunc(
                                length,
                                tbsPtr,
                                &certPtr->certSigPtr->asn1Length,
                                &certPtr->certSigPtr->asn1Data,
                                pkey->pkKeyPtr ,
                                tm_x509_objid(pkiPtr,
                                          certPtr->certAlgorPtr->algorObjPtr),
                                TM_X509_VERIFY   );
            }
            else
            {
                errorCode = TM_EINVAL;
            }
#ifdef TM_ERROR_CHECKING
            if ( errorCode != TM_ENOERROR)
            {
                tfKernelWarning("tfX509CertVerify", "verify error!\n");
            }
#endif /* TM_ERROR_CHECKING */
        }
        else
        {
            errorCode = TM_ENOENT;
        }
    }
    return errorCode;
}

#endif /* TM_PUBKEY_USE_RSA || TM_PUBKEY_USE_DSA */



ttPkiPublicKeyPtr tfX509PubKeyInfoGet ( ttX509SubPubKeyInfoPtr subPkPtr)
{
    ttPkiPublicKeyPtr   keyPtr;
    tt8BitPtr           datPtr;
    ttPkiGlobalPtr      pkiPtr;
    int                 errorCode;

    pkiPtr = (ttPkiGlobalPtr)tm_context(tvPkiPtr);
    keyPtr = (ttPkiPublicKeyPtr)0;
    errorCode = TM_ENOERROR;
    if (    (subPkPtr != (ttVoidPtr)0 )
         && (pkiPtr != (ttPkiGlobalPtr)0)
         && (pkiPtr->pkiObjectPtr != (ttAsn1ObjectPtr)0))
    {
        keyPtr = (ttPkiPublicKeyPtr)tm_get_raw_buffer(sizeof(ttPkiPublicKey));
        if ( keyPtr != (ttPkiPublicKeyPtr)0 )
        {
            tm_bzero(keyPtr, sizeof(ttPkiPublicKey));
            keyPtr->pkType = tm_x509_objid(pkiPtr,
                                       subPkPtr->subPkAlgorPtr->algorObjPtr);
/* RSA , n+e*/
            datPtr = subPkPtr->subPkValuePtr->asn1Data;
#ifdef TM_PUBKEY_USE_DSA
            if (( keyPtr->pkType == TM_PKI_OBJ_DSASHA1 ) ||
                ( keyPtr->pkType == TM_PKI_OBJ_DSA ) )
            {
                (void)tm_x509_dsa_dec(&keyPtr->pkKeyPtr ,&datPtr);
            }
#ifdef TM_PUBKEY_USE_RSA
            else
#endif /* TM_PUBKEY_USE_RSA */
#endif /* TM_PUBKEY_USE_DSA */
#ifdef TM_PUBKEY_USE_RSA
            if((keyPtr->pkType == TM_PKI_OBJ_RSASHA1)||
               (keyPtr->pkType == TM_PKI_OBJ_RSASHA256)||
               (keyPtr->pkType == TM_PKI_OBJ_RSASHA384)||
               (keyPtr->pkType == TM_PKI_OBJ_RSASHA512)||
                (keyPtr->pkType == TM_PKI_OBJ_RSAMD5) ||
                (keyPtr->pkType == TM_PKI_OBJ_RSA))
            {
                (void)tm_x509_rsa_dec(&keyPtr->pkKeyPtr ,&datPtr);
            }
#endif /* TM_PUBKEY_USE_RSA */
#if (defined(TM_PUBKEY_USE_RSA) || defined(TM_PUBKEY_USE_DSA))
            else
#endif /* TM_PUBKEY_USE_RSA || TM_PUBKEY_USE_DSA */
            {
                errorCode = TM_EINVAL;
            }

#if (defined(TM_PUBKEY_USE_RSA) || defined(TM_PUBKEY_USE_DSA))
            if (errorCode == TM_ENOERROR)
            {
                if ( keyPtr->pkKeyPtr == (ttVoidPtr)0)
                {
#ifdef TM_ERROR_CHECKING
                    tfKernelWarning("tfX509PubKeyInfoGet", "keyPtr error!\n");
#endif /* TM_ERROR_CHECKING */
                    errorCode = TM_EINVAL;
                }
            }

#ifdef TM_PUBKEY_USE_DSA
            if (    (errorCode == TM_ENOERROR)
                 && (( keyPtr->pkType == TM_PKI_OBJ_DSASHA1)||
                     ( keyPtr->pkType == TM_PKI_OBJ_DSA) ) )
            {
                errorCode = tfX509DsaPkParaGet(keyPtr->pkKeyPtr ,
                                    subPkPtr->subPkAlgorPtr->algorParPtr);
#ifdef TM_ERROR_CHECKING
                if ( errorCode != TM_ENOERROR)
                {
                    tfKernelWarning("tfX509PubKeyInfoGet", "keyPtr error!\n");
                }
#endif /* TM_ERROR_CHECKING */
            }
#endif /* TM_PUBKEY_USE_DSA */
#endif /* TM_PUBKEY_USE_RSA || TM_PUBKEY_USE_DSA */
        }
    }
#ifdef TM_ERROR_CHECKING
    else
    {
        tfKernelWarning("tfX509PubKeyInfoGet", "null pointer!\n");
    }
#endif /* TM_ERROR_CHECKING */
    if (errorCode != TM_ENOERROR)
    {
        if (keyPtr != (ttPkiPublicKeyPtr)0)
        {
            tm_free_raw_buffer(keyPtr);
            keyPtr = (ttPkiPublicKeyPtr)0;
        }
    }
    return keyPtr;
}

#ifdef TM_PUBKEY_USE_RSA

int tfX509RsaPubKeyDecode (     ttVoidPtrPtr    vPtrPtr,
                                tt8BitPtrPtr    derPtrPtr)
{
    ttGeneralRsaPtr rsaPtr;
    tt8BitPtr       temp8Ptr;
    int             totalLen;

    totalLen = -1;
    if ((derPtrPtr != (ttVoidPtr)0) &&
        ( *derPtrPtr != (ttVoidPtr)0))
    {
        rsaPtr = (ttGeneralRsaPtr)tm_get_raw_buffer(sizeof(ttGeneralRsa));
        if ( rsaPtr != (ttGeneralRsaPtr)0)
        {
            temp8Ptr  = *derPtrPtr;
            totalLen = tm_asn1_sequence_dec((ttVoidPtr)0, &temp8Ptr);
            tm_bzero((tt8BitPtr)rsaPtr, sizeof(*rsaPtr));
            (void)tm_asn1_integer_dec ( &rsaPtr->rsaN, &temp8Ptr);
            (void)tm_asn1_integer_dec ( &rsaPtr->rsaE, &temp8Ptr);
            if (totalLen != (int)(temp8Ptr - *derPtrPtr))
            {
#ifdef TM_ERROR_CHECKING
                tfKernelWarning("tfX509RsaPubKeyDecode", "length error!\n");
#endif /* TM_ERROR_CHECKING */
                tm_free_raw_buffer(rsaPtr);
                totalLen = -1;
            }
            else
            {
                *derPtrPtr  = temp8Ptr;
                *vPtrPtr    = (ttVoidPtr)rsaPtr;
            }
        }
#ifdef TM_ERROR_CHECKING
        else
        {
            tfKernelWarning("tfX509RsaPubKeyDecode", "no memory!\n");
        }
#endif /* TM_ERROR_CHECKING */
    }
#ifdef TM_ERROR_CHECKING
    else
    {
        tfKernelWarning("tfX509RsaPubKeyDecode", "null pointer!\n");
    }
#endif /* TM_ERROR_CHECKING */
    return totalLen;
}
#endif /* TM_PUBKEY_USE_RSA */


/* DER encoding of DSA publick keyPtr, i.e.,
       pubkey: Y */

#ifdef TM_PUBKEY_USE_DSA

int tfX509DsaPubKeyDecode (     ttVoidPtrPtr    vPtrPtr,
                                tt8BitPtrPtr    derPtrPtr)
{
    ttGeneralDsaPtr     dsaPtr;
    tt8BitPtr           temp8Ptr;
    int                 totalLen;

    totalLen = -1;
    if ((derPtrPtr != (ttVoidPtr)0) && (*derPtrPtr != (ttVoidPtr)0))
    {
        dsaPtr = (ttGeneralDsaPtr)tm_get_raw_buffer(sizeof(ttGeneralDsa));
        if ( dsaPtr != (ttGeneralDsaPtr)0)
        {
            temp8Ptr  = *derPtrPtr;
            tm_bzero((tt8BitPtr)dsaPtr, sizeof(*dsaPtr));
            totalLen = tm_asn1_integer_dec ( &dsaPtr->dsaPubKey, &temp8Ptr);
            *derPtrPtr  = temp8Ptr;
            *vPtrPtr    = (ttVoidPtr)dsaPtr;
        }
#ifdef TM_ERROR_CHECKING
        else
        {
            tfKernelWarning("tfX509DsaPubKeyDecode", "no memory!\n");
        }
#endif /* TM_ERROR_CHECKING */
    }
#ifdef TM_ERROR_CHECKING
    else
    {
        tfKernelWarning("tfX509DsaPubKeyDecode", "null pointer!\n");
    }
#endif /* TM_ERROR_CHECKING */
    return totalLen;
}

static int tfX509DsaPkParaGet( ttVoidPtr keyPtr, ttAsn1StringPtr paPtr)
{
    ttGeneralDsaPtr     dsaPtr;
    tt8BitPtr           temp8Ptr;
    int                 totalLen;
    int                 errorCode;

    errorCode = TM_ENOERROR;
    if ((paPtr == (ttVoidPtr)0) || (keyPtr == (ttVoidPtr)0))
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfX509DsaPkParaGet", "null pointer!\n");
#endif /* TM_ERROR_CHECKING */
        errorCode = TM_EINVAL;
        goto dsaPkParaGetExit;
    }

    temp8Ptr  = (tt8BitPtr)paPtr->asn1Data;
    totalLen = tm_asn1_sequence_dec((ttVoidPtr)0,    &temp8Ptr);

    dsaPtr = (ttGeneralDsaPtr)keyPtr;
    (void)tm_asn1_integer_dec ( &dsaPtr->dsaP, &temp8Ptr);
    (void)tm_asn1_integer_dec ( &dsaPtr->dsaQ, &temp8Ptr);
    (void)tm_asn1_integer_dec ( &dsaPtr->dsaG, &temp8Ptr);

    if (totalLen != (int)(temp8Ptr - paPtr->asn1Data))
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfX509DsaPkParaGet", "length error!\n");
#endif /* TM_ERROR_CHECKING */
        errorCode = TM_EINVAL;
        goto dsaPkParaGetExit;
    }

dsaPkParaGetExit:
    return errorCode;
}

#endif /* TM_PUBKEY_USE_DSA */




/* Object Serach */

ttVoidPtr tfPkiObjSearch( ttVoidPtr keyPtr, int method, int len)
{
    ttPkiGlobalPtr  pkiPtr;
    ttVoidPtr       objectPtr;
    unsigned int    i;
    int             isEqual;
    tt8Bit          found;

    objectPtr = (ttVoidPtr)0;
    found = TM_8BIT_NO;
    pkiPtr = (ttPkiGlobalPtr)tm_context(tvPkiPtr);
    if (    (pkiPtr != (ttPkiGlobalPtr)0)
         && (pkiPtr->pkiObjectPtr != (ttAsn1ObjectPtr)0))
    {
/* did't consider UNDEF object */
        for ( i = 1; (i < TM_PKI_MAX_OBJECT); i ++)
        {
            switch (method)
            {
                case TM_PKI_OBJECT_SEARCH_SN:
                    isEqual = tm_strcmp ( 
                                    tlPkiConstObject[i].objShortName, keyPtr);
                    if ( isEqual ==0 )
                    {
                        found = TM_8BIT_YES;
                    }
                    break;
                case TM_PKI_OBJECT_SEARCH_LN:
                    isEqual = tm_strcmp ( 
                                    tlPkiConstObject[i].objLongName, keyPtr);
                    if ( isEqual ==0 )
                    {
                        found = TM_8BIT_YES;
                    }
                    break;
                case TM_PKI_OBJECT_SEARCH_OBJ:
                    isEqual = tm_strcmp ( tlPkiConstObject[i].objIdStr, keyPtr);
                    if ( isEqual ==0 )
                    {
                        found = TM_8BIT_YES;
                    }
                    break;
                case TM_PKI_OBJECT_SEARCH_DER:
                    isEqual = tm_memcmp ( 
                                pkiPtr->pkiObjectPtr[i].objDerStr + sizeof(int),
                                keyPtr,len );
                    if( ( len == *((ttIntPtr)pkiPtr->pkiObjectPtr[i].objDerStr) )
                     && ( isEqual == 0) )
                    {
                        found = TM_8BIT_YES;
                    }
                    break;
                default:
                    break;
            }
            if (found)
            {
                objectPtr = (ttVoidPtr)&pkiPtr->pkiObjectPtr[i];
                break; /* for loop */
            }
        }
    }
    return objectPtr;
}


ttVoidPtr tfPkiAsn1ObjIdSearch( ttVoidPtr keyPtr, int method, int len)
{
    ttPkiGlobalPtr  pkiPtr;
    ttVoidPtr       objectPtr;

    objectPtr = (ttVoidPtr)0;
    pkiPtr = (ttPkiGlobalPtr)tm_context(tvPkiPtr);

    if (    (pkiPtr != (ttPkiGlobalPtr)0)
         && (pkiPtr->pkiObjectPtr != (ttAsn1ObjectPtr)0))
    {
        objectPtr = tfPkiObjSearch(keyPtr, method, len);
        if(objectPtr == (ttVoidPtr)0)
        {
/* Set UNDEF object if object not found */
            objectPtr = (ttVoidPtr)&pkiPtr->pkiObjectPtr[0];
        }
    }
    return objectPtr;
}

int tfPkiObjIdSearch(ttCharPtr s)
{
    ttAsn1ObjectPtr op;
    ttPkiGlobalPtr  pkiPtr;
    int             retCode;

    op=(ttAsn1ObjectPtr)tfPkiObjSearch(
        (ttVoidPtr)s, TM_PKI_OBJECT_SEARCH_SN,0);
    if (op == (ttVoidPtr)0)
    {
        op=(ttAsn1ObjectPtr)tfPkiObjSearch(
            (ttVoidPtr)s, TM_PKI_OBJECT_SEARCH_LN,0);
    }
    if (op == (ttVoidPtr)0)
    {
        op=(ttAsn1ObjectPtr)tfPkiObjSearch(
            (ttVoidPtr)s, TM_PKI_OBJECT_SEARCH_OBJ,0);
    }
    if (op == (ttVoidPtr)0)
    {
        op=(ttAsn1ObjectPtr)tfPkiObjSearch(
            (ttVoidPtr)s, TM_PKI_OBJECT_SEARCH_DER, (int) tm_strlen(s));
    }
    if ( op != (ttVoidPtr)0)
    {
/*
 * If op is non null, then we have already verified that pkiPtr, and 
 * pkiPtr->pkiObjectPtr are non null
 */
        pkiPtr = (ttPkiGlobalPtr)tm_context(tvPkiPtr);
        retCode = tm_x509_objid(pkiPtr, op);
    }
    else
    {
        retCode = TM_PKI_OBJECT_NID_UNDEF;
    }
    return retCode;
}


int tfX509ObjectInit(void)
{
    tt8BitPtr      dataPtr;
    ttPkiGlobalPtr pkiPtr;
    unsigned int   i;
    int            length;
    int            errorCode;

    pkiPtr = (ttPkiGlobalPtr)tm_context(tvPkiPtr);
    if (pkiPtr == (ttPkiGlobalPtr)0)
    {
        errorCode = TM_ENOENT;
    }
    else
    {
        errorCode = TM_ENOERROR;
        if (pkiPtr->pkiObjectPtr == (ttAsn1ObjectPtr)0)
        {
            pkiPtr->pkiObjectPtr = tm_get_raw_buffer(sizeof(tlPkiConstObject));
            if (pkiPtr->pkiObjectPtr == (ttAsn1ObjectPtr)0)
            {
#ifdef TM_ERROR_CHECKING
                tfKernelWarning("tfX509ObjectInit", "OBJECT init error!\n");
#endif /* TM_ERROR_CHECKING */
                errorCode = TM_ENOBUFS;
            }
            else
            {
                tm_bcopy(tlPkiConstObject, pkiPtr->pkiObjectPtr,
                         sizeof(tlPkiConstObject));
            }
        }
        if (pkiPtr->pkiObjectPtr != (ttAsn1ObjectPtr)0)
        {
            for ( i = 0 ; i < TM_PKI_MAX_OBJECT ; i++)
            {
                if ( pkiPtr->pkiObjectPtr[i].objDerStr == (ttCharPtr)0 )
                {
                    length = tm_asn1_enc((ttVoidPtr)&pkiPtr->pkiObjectPtr[i] ,
                                        (ttVoidPtr)0, TM_ASN1_TAG_OBJECT);
                    dataPtr = tm_get_raw_buffer((ttPktLen)length + sizeof(int));
                    if ( dataPtr == (ttVoidPtr)0)
                    {
#ifdef TM_ERROR_CHECKING
                        tfKernelWarning("tfX509ObjectInit", "OBJECT init error!\n");
#endif /* TM_ERROR_CHECKING */
                        errorCode = TM_ENOBUFS;
                        break;
                    }
                    (void)tm_asn1_enc(
                        (ttVoidPtr)&pkiPtr->pkiObjectPtr[i],
                        (ttVoidPtr)(dataPtr + sizeof(int)),
                        TM_ASN1_TAG_OBJECT);
                    pkiPtr->pkiObjectPtr[i].objDerStr = (ttCharPtr)dataPtr;
                    *((ttIntPtr)(dataPtr)) = length;
                }
            }
            if (errorCode != TM_ENOERROR)
            {
                tfX509ObjectFree();
            }
        }
    }
    return errorCode;
}

void tfX509ObjectFree(void)
{
    ttPkiGlobalPtr pkiPtr;
    unsigned int   i;

    pkiPtr = (ttPkiGlobalPtr)tm_context(tvPkiPtr);
    if (pkiPtr != (ttPkiGlobalPtr)0)
    {
        if (pkiPtr->pkiObjectPtr != (ttAsn1ObjectPtr)0)
        {
            for ( i = 0 ; i < TM_PKI_MAX_OBJECT ; i++)
            {
                if ( pkiPtr->pkiObjectPtr[i].objDerStr != (ttCharPtr)0)
                {
                    tm_free_raw_buffer((tt8BitPtr)
                                        pkiPtr->pkiObjectPtr[i].objDerStr);
                    pkiPtr->pkiObjectPtr[i].objDerStr = (ttCharPtr)0;
                }
            }
            tm_free_raw_buffer(pkiPtr->pkiObjectPtr);
            pkiPtr->pkiObjectPtr = (ttAsn1ObjectPtr)0;
        }
    }
    return;
}

/****************************************************************************
* FUNCTION: tfX509CertAlive
*
* PURPOSE:
*   This function check certificate is alive or not.
*
* PARAMETERS:
*  certPtr  : pointer to certificate
* RETURNS:
*   0 (TM_ENOERROR ):
*       it is alive.
*   others (1 or -1 ):
*       not alive.
*
* NOTES:
*
****************************************************************************/
int tfX509CertAlive(ttPkiCertListPtr certPtr)
{
/* time is between before and after validity */
    int         ret;
#ifdef TM_PKI_CERT_CHECK_ALIVE
    tt32Bit     curTvTime;
    tt32Bit     curTime;
    tt32Bit     compareTime;
    char        curTimeStr[15];

    tt8Bit      type;

    ret = TM_EINVAL;
    if ( (((ttPkiGlobalPtr)tm_context(tvPkiPtr)))->pkiCertCheckAlive )
    {
        tm_bcopy(TM_PKI_TIME, curTimeStr,tm_strlen(TM_PKI_TIME));
        tm_kernel_set_critical;
        curTvTime = tvTime/1000;
        tm_kernel_release_critical;

        curTime = tfX509TimeAbsDiff((tt8BitPtr)curTimeStr,
                                    TM_ASN1_TAG_UTCTIME)  + curTvTime/1000;

        type = *((tt8BitPtr)certPtr->certValidBeforePtr - 2);
        compareTime =
            tfX509TimeAbsDiff((tt8BitPtr)certPtr->certValidBeforePtr,type);
        if ( compareTime > curTime )
        {
            goto certAliveExit;
        }

        compareTime =
            tfX509TimeAbsDiff((tt8BitPtr)certPtr->certValidAfterPtr,type);
        if ( compareTime < curTime )
        {
            goto certAliveExit;
        }
    }
#else /* !TM_PKI_CERT_CHECK_ALIVE */
    TM_UNREF_IN_ARG(certPtr);
#endif /* !TM_PKI_CERT_CHECK_ALIVE */

    ret = TM_ENOERROR;

#ifdef TM_PKI_CERT_CHECK_ALIVE
certAliveExit:
#endif /* TM_PKI_CERT_CHECK_ALIVE */
    return ret;
}
/****************************************************************************
* FUNCTION: tfX509TimeAbsDiff
*
* PURPOSE:
*   This function get absolute seconds since 1950
*
* PARAMETERS:
*  timePtr :  pointer to time string
*  type    : time format- TM_ASN1_TAG_UTCTIME, TM_ASN1_TAG_GENERALIZEDTIME
* RETURNS:
*   0 : wrong format
*   others: seconds
*
* NOTES:
*
****************************************************************************/
#ifdef TM_PKI_CERT_CHECK_ALIVE
static tt32Bit tfX509TimeAbsDiff(tt8BitPtr timePtr, int type)
{
    tt32Bit         retTime;
    static const int      dateofMonth[] =
    { 0, 31, 59, 90, 120, 151, 181, 212, 243,273,304, 334};
    tt8BitPtr       comPtr;
    int             lYear;
    int             value;

    lYear   = 0;
    comPtr  = timePtr-1;
/* year */
    value = (*++comPtr - '0')*10;
    value += (*++comPtr - '0');

    if ( type  ==  TM_ASN1_TAG_UTCTIME)
    {
        value += 1900;
        if ( value < 1950 )
        {
            value +=100;
        }
    }
    else if ( type == TM_ASN1_TAG_GENERALIZEDTIME)
    {
        value = value * 100 + (*comPtr++ - '0')*10 + (*comPtr++ - '0');
    }
    else
    {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfX509TimeAbsDiff", "time type error");
#endif /* TM_ERROR_CHECKING */
        retTime = 0;
        goto timeAbsDiffExit;
    }
    if (( value%4 == 0 ) && ((value%100 != 0)||(value%400 == 0)))
    {
        lYear = 1;
    }
/* day number, the first lYear is 1952 */
    retTime = 365*(value - 1950) + (int)((value+1 - 1950)/4);
/* adjust day number, 2100 is not lYear, did not consider beyond 2100 */
    if ( value > 2100 )
    {
        retTime--;
    }
/* month */
    value = (*++comPtr - '0')*10;
    value += (*++comPtr - '0');
    if ( value > 12 )
    {
        retTime = 0;
        goto timeAbsDiffExit;
    }
    retTime = retTime + dateofMonth[value-1];
    if ( lYear && value == 3 )
    {
        retTime++;
    }
/* days */
    value = (*++comPtr - '0')*10;
    value += (*++comPtr - '0');
    if ( value > 31 )
    {
        retTime = 0;
        goto timeAbsDiffExit;
    }
    retTime = retTime + value - 1;

/* hours */
    value = (*++comPtr - '0')*10;
    value += (*++comPtr - '0');
    if ( value > 24 )
    {
        retTime = 0;
        goto timeAbsDiffExit;
    }
    retTime = retTime*24 + value;
/* mins */
    value = (*++comPtr - '0')*10;
    value += (*++comPtr - '0');
    if ( value > 60 )
    {
        retTime = 0;
        goto timeAbsDiffExit;
    }
    retTime = retTime *60 + value;
/* seconds */
    value = (*++comPtr - '0')*10;
    value += (*++comPtr - '0');
    if ( value > 60 )
    {
        retTime = 0;
        goto timeAbsDiffExit;
    }
    retTime = retTime *60 + value;

timeAbsDiffExit:
    return retTime;
}

#endif /* TM_PKI_CERT_CHECK_ALIVE */

int tfX509GetObjId(tt8BitPtr x)
{
    ttPkiGlobalPtr pkiPtr;
    int            retCode;

    pkiPtr = (ttPkiGlobalPtr)tm_context(tvPkiPtr);
    if (    (pkiPtr != (ttPkiGlobalPtr)0)
         && (pkiPtr->pkiObjectPtr != (ttAsn1ObjectPtr)0))
    {
        retCode =  tm_x509_objid(pkiPtr, x);
    }
    else
    {
        retCode = TM_PKI_OBJECT_NID_UNDEF;
    }
    return retCode;
}

/* BASE64 encoding for PEM format
 * (1) the final quantum of encoding input is an integral
 *  multiple of 24 bits; here, the final unit of encoded output will be
 *  an integral multiple of 4 characters with no "=" padding, (2) the
 *  final quantum of encoding input is exactly 8 bits; here, the final
 *  unit of encoded output will be two characters followed by two "="
 *  padding characters, or (3) the final quantum of encoding input is
 *  exactly 16 bits; here, the final unit of encoded output will be three
 *  characters followed by one "=" padding character.

 * Value Encoding  Value Encoding  Value Encoding  Value Encoding
 *      0 A            17 R            34 i            51 z
 *      1 B            18 S            35 j            52 0
 *      2 C            19 T            36 k            53 1
 *      3 D            20 U            37 l            54 2
 *      4 E            21 V            38 m            55 3
 *      5 F            22 W            39 n            56 4
 *      6 G            23 X            40 o            57 5
 *      7 H            24 Y            41 p            58 6
 *      8 I            25 Z            42 q            59 7
 *      9 J            26 a            43 r            60 8
 *     10 K            27 b            44 s            61 9
 *     11 L            28 c            45 t            62 +
 *     12 M            29 d            46 u            63 /
 *     13 N            30 e            47 v
 *     14 O            31 f            48 w         (pad) =
 *     15 P            32 g            49 x
 *     16 Q            33 h            50 y
 */

static tt8Bit tfX509PemEnc(tt8Bit x)
{
    tt8Bit ret;

    ret = 0;
    if (x<26)
    {
        ret =  ( (tt8Bit)(x +'A'));
    }
    else if (x<52)
    {
        ret =  ( (tt8Bit)(x-26 +'a'));
    }
    else if (x<62)
    {
        ret =  ( (tt8Bit)(x -52+'0'));
    }
    else if ( x == 62)
    {
        ret =  '+';
    }
    else if ( x == 63)
    {
        ret =  '/';
    }
    return ret;
}

static tt8Bit tfX509PemDec(tt8Bit x)
{
    tt8Bit  ret;

    ret = 128;
    if ( (x>='A') && (x<='Z') )
    {
        ret =  ( (tt8Bit)(x -'A'));
    }
    else if ( (x>='a') && (x<='z') )
    {
        ret =  ( (tt8Bit)(x+26 -'a'));
    }
    else if ( (x>='0') && (x<='9') )
    {
        ret =  ( (tt8Bit)(x +52-'0'));
    }
    else if ( x == '+')
    {
        ret =  62;
    }
    else if ( x == '/')
    {
        ret =  63;
    }
    return ret;
}

tt8BitPtr  tfX509PemToDer ( tt8BitPtr pemPtr, tt16BitPtr lenPtr)
{
    tt8BitPtr   tempPtr;
    tt8Bit      t1,t2,t3,t4;
    int         dLen;
    int         length;
    int         j;
    int         i;

    if ( pemPtr == (tt8BitPtr)0)
    {
        tempPtr = (tt8BitPtr)0;
        goto x509PemToDerExit;
    }
    tempPtr = pemPtr;
    dLen = 0;
    while ( ( *tempPtr  != '=') && ( *tempPtr !='\0'))
    {
        if (( *tempPtr == 10) || ( *tempPtr == 13))
        {
            tempPtr++;
            continue;
        }
        dLen++;
        tempPtr++;
    }
    length = dLen;
    dLen = dLen/4*3;

    if ( *tempPtr == '=')
    {
        tempPtr++;
        dLen+=2;
        if ( *tempPtr == '=')
        {
            tempPtr++;
            dLen--;
        }
    }

    tempPtr = tm_get_raw_buffer((ttPktLen)dLen);
    *lenPtr = (tt16Bit)dLen;
    if ( tempPtr == (ttVoidPtr)0)
    {
        return (ttVoidPtr)0;
    }
    j = 0;
    for ( i = 0 ; i <length/4; i++)
    {
/* 6 + 2 ; 4+ 4: 2+6 */
        while(( *pemPtr == 10) || (*pemPtr == 13))
        {
            pemPtr++;
        }
        t1 = tfX509PemDec(*pemPtr++);

        while(( *pemPtr == 10) || (*pemPtr == 13))
        {
            pemPtr++;
        }
        t2 = tfX509PemDec(*pemPtr++);

        while(( *pemPtr == 10) || (*pemPtr == 13))
        {
            pemPtr++;
        }
        t3 = tfX509PemDec(*pemPtr++);

        while(( *pemPtr == 10) || (*pemPtr == 13))
        {
            pemPtr++;
        }
        t4 = tfX509PemDec(*pemPtr++);

        tempPtr[j++] = (tt8Bit)((t1<<2)|((t2 & 0x30)>>4));
        tempPtr[j++] = (tt8Bit)(((t2&0x0f)<<4)|((t3&0x3c)>>2));
        tempPtr[j++] = (tt8Bit)(((t3&0x03)<<6)| t4 );
    }

    if ( dLen > j )
    {
        while(( *pemPtr == 10) || (*pemPtr == 13))
        {
            pemPtr++;
        }
        t1 = tfX509PemDec(*pemPtr++);

        while(( *pemPtr == 10) || (*pemPtr == 13))
        {
            pemPtr++;
        }
        t2 = tfX509PemDec(*pemPtr++);

        tempPtr[j++] = (tt8Bit)((t1<<2)|((t2 & 0x30)>>4));

        if ( dLen > j )
        {
            while(( *pemPtr == 10) || (*pemPtr == 13))
            {
                pemPtr++;
            }
            t3 = tfX509PemDec(*pemPtr++);

            tempPtr[j++] = (tt8Bit)(((t2&0x0f)<<4) | ((t3&0x3c)>>2));
        }
    }
    if ( dLen != j)
    {
        return (ttVoidPtr)0;
    }

x509PemToDerExit:
    return tempPtr;
}

tt8BitPtr  tfX509DerToPem (  tt8BitPtr derPtr, int length,
                                    tt16BitPtr lenPtr)
{
    tt8BitPtr       tempPtr;
    int             dLen;
    int             i;
    int             j;
    tt8Bit          t1,t2,t3,t4;

    dLen = length/3*4;
    if ( length%3 == 1)
    {
        dLen+=4;
    }
    if ( length %3 == 2)
    {
        dLen+=4;
    }

    dLen += ( dLen/64 + 1) * 2 ;
    tempPtr = tm_get_raw_buffer((ttPktLen)dLen);
    if ( tempPtr == (ttVoidPtr)0)
    {
        tempPtr = (tt8BitPtr)0;
        goto x509DerToPemExit;
    }
    j = 0;
    for  ( i = 0 ; i < length/3; i++)
    {
        t1 = (tt8Bit)((derPtr[3*i]&0xfc)>>2);
        t2 = (tt8Bit)((((derPtr[3*i]&0x03)<<4)&0x3f)
                |((derPtr[3*i+1]&0xf0)>>4));
        t3 = (tt8Bit)((((derPtr[3*i+1]&0x0f)<<2)&0x3f)
                |((derPtr[3*i+2]&0xc0)>>6));
        t4 = (tt8Bit)(derPtr[3*i+2]&0x3f);

        tempPtr[j++] = tfX509PemEnc(t1);
        tempPtr[j++] = tfX509PemEnc(t2);
        tempPtr[j++] = tfX509PemEnc(t3);
        tempPtr[j++] = tfX509PemEnc(t4);

        if ( (i%16) == 15)
        {
            tempPtr[j++]= 10;
            tempPtr[j++]= 13;
        }
    }
    if ( length > 3*i)
    {
        tempPtr[j++] = tfX509PemEnc((tt8Bit)((derPtr[3*i]&0xfc)>>2));
        if ( length == 3*i + 1)
        {
            tempPtr[j++] = tfX509PemEnc((tt8Bit)(((derPtr[3*i]&0x03)<<4)
                            &0x3f));
            tempPtr[j++] = '=';
            tempPtr[j++] = '=';
        }
        else if ( length == 3*i+2)
        {
            tempPtr[j++] = tfX509PemEnc((tt8Bit)(((((derPtr[3*i])&0x03)<<4)
                            &0x3f)|((derPtr[3*i+1]&0xf0)>>4)));
            tempPtr[j++] = tfX509PemEnc((tt8Bit)(((derPtr[3*i+1]&0x0f)<<4)
                            &0x3f));
            tempPtr[j++] = '=';
        }
        else
        {
            return (ttVoidPtr)0;
        }
    }
    tempPtr[j++]= 10;
    tempPtr[j++]= 13;
    *lenPtr = (tt16Bit)dLen;

x509DerToPemExit:
    return tempPtr;
}

static int tfX509AlgorIdFree( ttX509AlgorIdenPtr aIdPtr)
{
    int errorCode;

    errorCode = TM_ENOERROR;
    if (aIdPtr != (ttX509AlgorIdenPtr) 0)
    {
/* tfAsn1StringFree checks for NULL pointer */
        (void)tfAsn1StringFree(aIdPtr->algorParPtr);
        tm_free_raw_buffer((tt8BitPtr)aIdPtr);
    }
    return errorCode;
}

static ttX509TBSCertPtr    tfX509TBSCertNew(void)
{
    ttX509TBSCertPtr        tbsPtr;
    int                     i;
    int                     errorCode;

    errorCode = TM_ENOBUFS; /* assume failure */
    tbsPtr = (ttX509TBSCertPtr)tm_get_raw_buffer( sizeof(ttX509TBSCert));
    if ( tbsPtr  == (ttX509TBSCertPtr)0)
    {
        goto x509TbsCertNewExit;
    }
    tm_bzero(tbsPtr, sizeof(ttX509TBSCert));
    tfAsn1StringInit(&tbsPtr->tbsVersion);
    tfAsn1StringInit(&tbsPtr->tbsSerialNumber);
    for ( i = 0 ; i < TM_PKI_MAX_STACK; i++)
    {
        tbsPtr->tbsExtensionsPtr[i] = (ttX509ExtensionPtr)
                            tm_get_raw_buffer(sizeof(ttX509Extension));
        if ( tbsPtr->tbsExtensionsPtr[i]== (ttVoidPtr)0)
        {
            goto x509TbsCertNewExit;
        }
        tm_bzero(tbsPtr->tbsExtensionsPtr[i], sizeof(ttX509Extension));
    }
    tbsPtr->tbsIssuerPtr        = tfX509NameNew();
    if (tbsPtr->tbsIssuerPtr == (ttVoidPtr)0)
    {
        goto x509TbsCertNewExit;
    }
    tbsPtr->tbsSubjectPtr       = tfX509NameNew();
    if (tbsPtr->tbsSubjectPtr == (ttVoidPtr)0)
    {
        goto x509TbsCertNewExit;
    }
    tbsPtr->tbsSubPkInfoPtr     = (ttX509SubPubKeyInfoPtr)
                            tm_get_raw_buffer (sizeof(ttX509SubPubKeyInfo));
    if (tbsPtr->tbsSubPkInfoPtr == (ttVoidPtr)0)
    {
        goto x509TbsCertNewExit;
    }
    tbsPtr->tbsSignaturePtr     = (ttX509AlgorIdenPtr)
                            tm_get_raw_buffer (sizeof(ttX509AlgorIden));
    if (tbsPtr->tbsSignaturePtr == (ttVoidPtr)0)
    {
        goto x509TbsCertNewExit;
    }
    tbsPtr->tbsValidityPtr      = (ttX509ValidityPtr)
                            tm_get_raw_buffer (sizeof(ttX509Validity));
    if (tbsPtr->tbsValidityPtr == (ttVoidPtr)0)
    {
        goto x509TbsCertNewExit;
    }
    tm_bzero(tbsPtr->tbsValidityPtr , sizeof(ttX509Validity));
    tm_bzero(tbsPtr->tbsSubPkInfoPtr , sizeof(ttX509SubPubKeyInfo));
    tm_bzero(tbsPtr->tbsSignaturePtr , sizeof(ttX509AlgorIden));

    errorCode = TM_ENOERROR;

x509TbsCertNewExit:
    if (errorCode != TM_ENOERROR)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfX509TbsCertNew", "out of memory!\n");
#endif /* TM_ERROR_CHECKING */
        if (tbsPtr != (ttX509TBSCertPtr)0)
        {
            tfX509TBSCertFree(tbsPtr);
            tbsPtr = (ttX509TBSCertPtr)0;
        }
    }
    return tbsPtr;
}

static int tfX509TBSCertFree( ttX509TBSCertPtr tbsPtr)
{
    int     errorCode ;
    int     i;

    errorCode = TM_ENOERROR;
    if (tbsPtr != (ttVoidPtr)0)
    {
        if ( tbsPtr->tbsVersion.asn1Data != (ttVoidPtr)0)
        {
            tm_free_raw_buffer( tbsPtr->tbsVersion.asn1Data);
        }
        if ( tbsPtr->tbsSerialNumber.asn1Data != (ttVoidPtr)0)
        {
            tm_free_raw_buffer( tbsPtr->tbsSerialNumber.asn1Data);
        }
        for ( i = 0 ; i < TM_PKI_MAX_STACK; i++)
        {
            if ( tbsPtr->tbsExtensionsPtr[i]  != (ttVoidPtr)0)
            {
/* tfAsn1StringFree checks for NULL pointer */
                (void)tfAsn1StringFree(tbsPtr->tbsExtensionsPtr[i]->extnValuePtr);
                tm_free_raw_buffer((tt8BitPtr)tbsPtr->tbsExtensionsPtr[i]);
            }
        }
        if ( tbsPtr->tbsIssuerPtr != (ttVoidPtr)0)
        {
            (void)tfX509NameFree(tbsPtr->tbsIssuerPtr);
        }
        if ( tbsPtr->tbsSubjectPtr != (ttVoidPtr)0)
        {
            (void)tfX509NameFree(tbsPtr->tbsSubjectPtr);
        }

/* free sigAlgor, and sigValue. tfX509AlgorIdFree checks for NULL pointer */
        (void)tfX509AlgorIdFree(tbsPtr->tbsSignaturePtr);

        if (tbsPtr->tbsValidityPtr != 0)
        {
/* tfAsn1StringFree checks for NULL pointer */
            (void)tfAsn1StringFree(tbsPtr->tbsValidityPtr->notAfterPtr);
/* tfAsn1StringFree checks for NULL pointer */
            (void)tfAsn1StringFree(tbsPtr->tbsValidityPtr->notBeforePtr);
            tm_free_raw_buffer((tt8BitPtr)tbsPtr->tbsValidityPtr);
        }
        if (tbsPtr->tbsSubPkInfoPtr != 0)
        {
/* free sigAlgor, and sigValue. tfX509AlgorIdFree checks for NULL pointer */
            (void)tfX509AlgorIdFree(tbsPtr->tbsSubPkInfoPtr->subPkAlgorPtr);
/* tfAsn1StringFree checks for NULL pointer */
            (void)tfAsn1StringFree(tbsPtr->tbsSubPkInfoPtr->subPkValuePtr);
            tm_free_raw_buffer((tt8BitPtr)tbsPtr->tbsSubPkInfoPtr);
        }
/* tfAsn1StringFree checks for NULL pointer */
        (void)tfAsn1StringFree(tbsPtr->tbsIssuerUIDPtr);
/* tfAsn1StringFree checks for NULL pointer */
        (void)tfAsn1StringFree(tbsPtr->tbsSubjectUIDPtr);
/* free issueUID, tbsSubjectUIDPtr */
        tm_free_raw_buffer((tt8BitPtr)tbsPtr);
    }
    else
    {
        errorCode = TM_EINVAL;
    }
    return errorCode;
}

/* Function: tfX509NameNew */
static ttX509NamePtr tfX509NameNew (void)
{
    ttX509NamePtr           name;
    ttX509AttrTypeValuePtr  attr;
    int                     i;

    name = (ttX509NamePtr)tm_get_raw_buffer(sizeof(ttX509Name));
    if ( name != (ttVoidPtr)0)
    {
        tm_bzero(name, sizeof(ttX509Name));
        for ( i = 0; i < TM_PKI_MAX_STACK; i++)
        {
            attr = (ttX509AttrTypeValuePtr)
                    tm_get_raw_buffer(sizeof(ttX509AttrTypeValue));
            if ( attr == (ttVoidPtr)0)
            {
#ifdef TM_ERROR_CHECKING
                tfKernelWarning("tfX509NameNew", "out of memory!\n");
#endif /* TM_ERROR_CHECKING */
                (void)tfX509NameFree(name);
                name = (ttX509NamePtr)0;
                break; /* for loop */
            }
/* Same as bzero */
            attr->atvObjPtr             = (ttVoidPtr)0;
            attr->atvValuePtr           = (ttVoidPtr)0;
            name->nameRelativeDNPtr[i]  = attr;
        }
    }
#ifdef TM_ERROR_CHECKING
    else
    {
        tfKernelWarning("tfX509NameNew", "out of memory!\n");
    }
#endif /* TM_ERROR_CHECKING */
    return name;

}

static int tfX509NameFree (ttX509NamePtr name)
{
    int i;
    int errorCode;

    errorCode = TM_ENOERROR;
    if (name != (ttVoidPtr)0)
    {
        for ( i = 0; i < TM_PKI_MAX_STACK; i++)
        {
            if ( name->nameRelativeDNPtr[i] != (ttVoidPtr)0)
            {
/* tfAsn1StringFree checks for NULL pointer */
                (void)tfAsn1StringFree(
                                   name->nameRelativeDNPtr[i]->atvValuePtr);
                tm_free_raw_buffer((tt8BitPtr)name->nameRelativeDNPtr[i]);
            }
        }
        tm_free_raw_buffer((tt8BitPtr)name);
    }
    else
    {
        errorCode = TM_EINVAL;
    }
    return errorCode;
}

/*** encoding part, it is not necessary for current PKI-IKE */

int tfX509GenericEncode (   ttVoidPtr       vPtr,
                            tt8BitPtrPtr    derPtrPtr,
                            ttIntPtr        lengthPtr,
                            int             type)
{
    tt8BitPtr       temp8Ptr;
    int             len;
    int             tlen;
    int             i;
    int             j;
    int             temp1Value;
    int             temp2Value;
    int             temp3Value;

    tlen = 0;
    temp1Value = 0;
    temp2Value = 0;
    temp3Value = 0;
    if ( vPtr == (ttVoidPtr)0)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfX509GenericEncode", "null pointer!\n");
#endif /* TM_ERROR_CHECKING */
        *lengthPtr = 0;
        goto x509GeneralEncExit;
    }
/* i =0 : check total length, i=1: put encode */
    temp8Ptr = (ttVoidPtr)0;
    for ( i = 0; i < 2;  i++)
    {
        len = 0;
        switch ( type )
        {
            case TM_X509_CERT:
                len +=tm_x509_generic_enc(
                                ((ttX509CertPtr)vPtr)->certTbsPtr,
                                &temp8Ptr,
                                &tlen,
                                TM_X509_TBSCERT);
                len +=tm_x509_generic_enc  (
                                ((ttX509CertPtr)vPtr)->certAlgorPtr ,
                                &temp8Ptr,
                                &tlen,
                                TM_X509_ALGORID);
                len +=tm_asn1_bitstring_enc(
                                ((ttX509CertPtr)vPtr)->certSigPtr,
                                &temp8Ptr,
                                tlen);
                break;
            case TM_X509_CERTREQ:
                len +=tm_x509_generic_enc(
                                ((ttX509CertReqPtr)vPtr)->certReqInfoPtr,
                                &temp8Ptr,
                                &tlen,
                                TM_X509_CERTREQINFO);
                len +=tm_x509_generic_enc  (
                                ((ttX509CertReqPtr)vPtr)->certReqAlgorPtr,
                                &temp8Ptr,
                                &tlen,
                                TM_X509_ALGORID);
                len +=tm_asn1_bitstring_enc(
                                ((ttX509CertReqPtr)vPtr)->certReqSigPtr,
                                &temp8Ptr,
                                tlen);
                break;
            case TM_X509_CERTLIST:
                len +=tm_x509_generic_enc(
                                ((ttX509CertListPtr)vPtr)->crlTbsCertListPtr,
                                &temp8Ptr,
                                &tlen,
                                TM_X509_TBSCERTLIST);
                len +=tm_x509_generic_enc  (
                                ((ttX509CertListPtr)vPtr)->crlAlgorPtr,
                                &temp8Ptr,
                                &tlen,
                                TM_X509_ALGORID);
                len +=tm_asn1_bitstring_enc(
                                ((ttX509CertListPtr)vPtr)->crlSigPtr,
                                &temp8Ptr,
                                tlen);
                break;
            case TM_X509_TBSCERT:
 /***********/
                if ( i == 1 )
                {
                    len+= tm_asn1_context_enc(0,&temp8Ptr,temp1Value);
                }

                temp1Value =tm_asn1_integer_enc     (
                                &((ttX509TBSCertPtr)vPtr)->tbsVersion ,
                                &temp8Ptr,
                                tlen);
                len += temp1Value;

                if ( i == 0)
                {
                    len+= tm_asn1_context_enc(0,&temp8Ptr,temp1Value);
                }

                len +=tm_asn1_integer_enc     (
                                &((ttX509TBSCertPtr)vPtr)->tbsSerialNumber,
                                &temp8Ptr,
                                tlen);

                len +=tm_x509_generic_enc(
                                ((ttX509TBSCertPtr)vPtr)->tbsSignaturePtr,
                                &temp8Ptr,
                                &tlen,
                                TM_X509_ALGORID);
                len +=tm_x509_generic_enc(
                                ((ttX509TBSCertPtr)vPtr)->tbsIssuerPtr,
                                &temp8Ptr,
                                &tlen,
                                TM_X509_NAME);
                len +=tm_x509_generic_enc(
                                ((ttX509TBSCertPtr)vPtr)->tbsValidityPtr,
                                &temp8Ptr,
                                &tlen,
                                TM_X509_VALIDITY);
                len +=tm_x509_generic_enc(
                                ((ttX509TBSCertPtr)vPtr)->tbsSubjectPtr,
                                &temp8Ptr,
                                &tlen,
                                TM_X509_NAME);
                len +=tm_x509_generic_enc(
                                ((ttX509TBSCertPtr)vPtr)->tbsSubPkInfoPtr,
                                &temp8Ptr,
                                &tlen,
                                TM_X509_SUBPKINFO);

                if ( ((ttX509TBSCertPtr)vPtr)->tbsIssuerUIDPtr
                        != (ttVoidPtr)0)
                {
                    len += tm_asn1_bitstring_enc(
                                ((ttX509TBSCertPtr)vPtr)->tbsIssuerUIDPtr ,
                                &temp8Ptr,
                                &tlen);
                }
                if ( ((ttX509TBSCertPtr)vPtr)->tbsSubjectUIDPtr
                        != (ttVoidPtr)0)
                {
                    len +=tm_asn1_bitstring_enc     (
                                ((ttX509TBSCertPtr)vPtr)->tbsSubjectUIDPtr ,
                                &temp8Ptr,
                                &tlen);
                }
/* [3], seq of seq, of seq.. */

                if ( i == 1)
                {
                    if ( temp2Value !=0)
                    {
                        len+= tm_asn1_context_enc(3,&temp8Ptr,temp2Value);
                    }
                }
                temp2Value=tm_x509_generic_enc    ((ttVoidPtr)
                                (((ttX509TBSCertPtr)vPtr)->tbsExtensionsPtr),
                                &temp8Ptr,
                                &tlen,
                                TM_X509_EXTENSION_ARRAY);
                if ( temp2Value >0)
                {
                    if ( i==0)
                    {
                        len+= tm_asn1_context_enc(3,&temp8Ptr,temp2Value);
                    }
                    else
                    {
                        len +=temp2Value;
                    }
                }
                break;
            case TM_X509_EXTENSION_ARRAY:
                for ( j = 0; j < TM_PKI_MAX_STACK; j++)
                {
                    if ((ttX509ExtensionPtr)(*((ttX509ExtensionPtrPtr)vPtr)
                            +j) == (ttVoidPtr)0)
                    {
                        break;
                    }
                    len+= tm_x509_generic_enc    (
                                (ttX509ExtensionPtr)
                                        (*((ttX509ExtensionPtrPtr)vPtr)+j),
                                &temp8Ptr,
                                &tlen,
                                TM_X509_EXTENSION);
                    if ( tlen == 0)
                    {
                        break;
                    }
                }
                break;

           case TM_X509_ALGORID:
                len +=tm_asn1_object_enc     (
                                ((ttX509AlgorIdenPtr)vPtr)->algorObjPtr ,
                                &temp8Ptr,
                                &tlen);
                if (( (ttX509AlgorIdenPtr)vPtr)->algorParPtr == 0)
                {
                    len += tm_asn1_null_enc(&temp8Ptr);
                }
                else
                {
                    len +=tm_asn1_type_enc     (
                                ((ttX509AlgorIdenPtr)vPtr)->algorParPtr ,
                                &temp8Ptr,
                                &tlen);
                }
                break;
            case TM_X509_NAME:
                for ( j = 0 ; j < ((ttX509NamePtr)vPtr)->nameNum ; j++)
                {
                    tlen = 0;
                    tlen =tm_x509_generic_enc(
                                ((ttX509NamePtr)vPtr)->nameRelativeDNPtr[j],
                                (ttVoidPtr)0,
                                &tlen,
                                TM_X509_ATTRTYPEVALUE);
                    if ( tlen == 0) break;
                    len +=tlen;
                    if ( i == 1)
                    {
                        len+=tm_asn1_set_enc ((ttVoidPtr)0, &temp8Ptr, tlen);
                        (void)tm_x509_generic_enc(
                                ((ttX509NamePtr)vPtr)->nameRelativeDNPtr[j],
                                &temp8Ptr,
                                &tlen,
                                TM_X509_ATTRTYPEVALUE);
                    }
                    else
                    {
                        len +=tm_asn1_set_enc ((ttVoidPtr)0, &temp8Ptr,tlen);
                    }
                }
                break;

            case TM_X509_VALIDITY:
                len +=tm_asn1_time_enc(
                                ((ttX509ValidityPtr)vPtr)->notBeforePtr,
                                &temp8Ptr,
                                &tlen);
                len +=tm_asn1_time_enc(
                                ((ttX509ValidityPtr)vPtr)->notAfterPtr,
                                &temp8Ptr,
                                &tlen);
                break;
            case TM_X509_SUBPKINFO:
                len +=tm_x509_generic_enc    (
                            ((ttX509SubPubKeyInfoPtr)vPtr)->subPkAlgorPtr,
                            &temp8Ptr,
                            &tlen,
                            TM_X509_ALGORID);
                len +=tm_asn1_bitstring_enc(
                            ((ttX509SubPubKeyInfoPtr)vPtr)->subPkValuePtr,
                            &temp8Ptr, &tlen);
                break;
            case TM_X509_EXTENSION:
                len +=tm_asn1_object_enc     (
                            ((ttX509ExtensionPtr)vPtr)->extnIDPtr ,
                            &temp8Ptr, &tlen);
                len +=tm_asn1_boolean_enc    (
                            ((ttX509ExtensionPtr)vPtr)->extnCritical ,
                            &temp8Ptr);
                len +=tm_asn1_octetstring_enc(
                            ((ttX509ExtensionPtr)vPtr)->extnValuePtr,
                            &temp8Ptr, &tlen);
                break;
            case TM_X509_ATTRTYPEVALUE:
                len +=tm_asn1_object_enc     (
                            ((ttX509AttrTypeValuePtr)vPtr)->atvObjPtr ,
                            &temp8Ptr, &tlen);
                len +=tm_asn1_string_enc(
                            ((ttX509AttrTypeValuePtr)vPtr)->atvValuePtr,
                            &temp8Ptr, &tlen);
                break;
            case TM_X509_CERTREQINFO:
                len +=tm_asn1_integer_enc    (
                            &((ttX509CertReqInfoPtr)vPtr)->reqInfoVersion,
                            &temp8Ptr, &tlen);
                len +=tm_x509_generic_enc    (
                            ((ttX509CertReqInfoPtr)vPtr)->reqInfoSubjectPtr,
                            &temp8Ptr, &tlen, TM_X509_NAME);
                len +=tm_x509_generic_enc    (
                        ((ttX509CertReqInfoPtr)vPtr)->reqInfoSubPkInfoPtr,
                        &temp8Ptr,
                        &tlen,
                        TM_X509_SUBPKINFO);
                for ( j = 0 ; j < TM_PKI_MAX_STACK; j++)
                {
                    tlen =tm_x509_generic_enc (
                        ((ttX509CertReqInfoPtr)vPtr)->reqInfoSubPkInfoPtr,
                        &temp8Ptr,
                        &tlen,
                        TM_X509_ATTRIBUTE);
                    if ( tlen <= 0)
                        break;
                    len += tlen;
                }
                break;
            case TM_X509_ATTRIBUTE:
                len +=tm_asn1_object_enc    (
                        ((ttX509AttributePtr)vPtr)->attObjPtr, &temp8Ptr, &tlen);
/* not clear: set, asn1string
 */
                break;
            case TM_X509_TBSCERTLIST:
                len +=tm_asn1_integer_enc    (
                        &((ttX509TBSCertListPtr)vPtr)->tclVersion,
                        &temp8Ptr,
                        &tlen);
                len +=tm_x509_generic_enc    (
                        ((ttX509TBSCertListPtr)vPtr)->tclSignaturePtr,
                        &temp8Ptr, &tlen, TM_X509_ALGORID);
                len +=tm_x509_generic_enc    (
                        ((ttX509TBSCertListPtr)vPtr)->tclIssuerPtr,
                        &temp8Ptr, &tlen, TM_X509_NAME);
                len +=tm_asn1_time_enc    (
                        ((ttX509TBSCertListPtr)vPtr)->tclThisUpdatePtr,
                        &temp8Ptr, &tlen);
                len +=tm_asn1_time_enc    (
                        ((ttX509TBSCertListPtr)vPtr)->tclNextUpdatePtr,
                        &temp8Ptr, &tlen);

                if ( i == 1)
                {
                    if ( temp3Value !=0)
                    {
                        len+= tm_asn1_context_enc(0,&temp8Ptr,temp3Value);
                    }
                }
                temp3Value=tm_x509_generic_enc    ((ttVoidPtr)
                    (((ttX509TBSCertListPtr)vPtr)->tclCrlExtensionsPtr),
                    &temp8Ptr, &tlen, TM_X509_EXTENSION_ARRAY);
                if ( temp3Value >0)
                {
                    if ( i==0)
                    {
                        len+= tm_asn1_context_enc(0,&temp8Ptr,temp3Value);
                    }
                    else
                    {
                        len +=temp3Value;
                    }
                }

                break;
            case TM_X509_REVOKEDCERT:
                len +=tm_asn1_integer_enc    (
                        ((ttX509RevokedCertPtr)vPtr)->revUserCertPtr,
                        &temp8Ptr, &tlen);
                len +=tm_asn1_time_enc    (
                        ((ttX509RevokedCertPtr)vPtr)->revRevocationDatePtr,
                        &temp8Ptr, &tlen);
                len += tm_x509_generic_enc    ((ttVoidPtr)
                        (((ttX509RevokedCertPtr)vPtr)->revCrlEntryExtPtr),
                        &temp8Ptr, &tlen, TM_X509_EXTENSION_ARRAY);
                break;
            default:
                break;
        }

        if ( i == 0)
        {
            if ( len >0)
            {
                tlen = tm_asn1_sequence_enc((ttVoidPtr)0, &temp8Ptr, len);
            }
            else
            {
                *lengthPtr = 0;
                goto x509GeneralEncExit;
            }
            *lengthPtr = len + tlen;
            if (( derPtrPtr ==(ttVoidPtr)0)||(*derPtrPtr == (ttVoidPtr)0))
            {
                break;
            }
            temp8Ptr = *derPtrPtr;
            (void)tm_asn1_sequence_enc ((ttVoidPtr)0,&temp8Ptr, len);
        }
    }

    if ((derPtrPtr == (ttVoidPtr)0)|| (*derPtrPtr == (ttVoidPtr)0))
    {
        goto x509GeneralEncExit;
    }

    if ( *lengthPtr != (int)(temp8Ptr - *derPtrPtr) )
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfX509GenericEncode", "length error!\n");
#endif /* TM_ERROR_CHECKING */
        *lengthPtr = -1;
        goto x509GeneralEncExit;
    }

    *derPtrPtr  = temp8Ptr;

x509GeneralEncExit:
    return *lengthPtr;
}

#else /* !TM_USE_PKI */
/* To allow link for builds when TM_USE_PKI is not defined */
int tlX509Dummy = 0;

#endif /* TM_USE_PKI */


