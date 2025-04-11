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
 * Description: Public Signature header file
 *
 * Filename: trpki.h
 * Author: jshang
 * Date Created: 01/20/2002
 * $Source: include/trpki.h $
 *
 * Modification History
 * $Revision: 6.0.2.7 $
 * $Date: 2016/04/13 18:15:53JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/* protected before publish alpha-version */

#ifndef _TRPKI_H_
#define _TRPKI_H_


#ifdef __cplusplus
extern "C" {
#endif

#ifndef TM_PKI_TIME
#define TM_PKI_TIME                         "090824200000Z"
#endif /* !TM_PKI_TIME */


/* only have top-level(root) CA, no chain/intermediate CA, 
 * if enable TM_PKI_CA_NOCHAIN, assume all certificates share same root CA 
 */

/*#define TM_PKI_CA_NOCHAIN */

/* Microsoft OID */
#define TM_X509_SUPPORT_MICROSOFT_OID

/* ASN1 definition */
/* ASN.1 */

#define TM_ASN1_UNIVERSAL_CLASS             (tt8Bit)0x00
#define TM_ASN1_APPLICATION_CLASS           (tt8Bit)0x40
#define TM_ASN1_CONTEXT_SPECIFIC_CLASS      (tt8Bit)0x80
#define TM_ASN1_PRIVATE_CLASS               (tt8Bit)0xc0
#define TM_ASN1_CONSTRUCTED_TYPE            (tt8Bit)0x20
#define TM_ASN1_PRIMITIVE_TYPE              (tt8Bit)0x00


/* negative flag */
#define TM_ASN1_TAG_NEG                     0x100    
    
#define TM_ASN1_TAG_UNDEF                   -1
#define TM_ASN1_TAG_EOC                     0
#define TM_ASN1_TAG_BOOLEAN                 1    
#define TM_ASN1_TAG_INTEGER                 2
#define TM_ASN1_TAG_NEG_INTEGER             (2 | TM_ASN1_TAG_NEG)
#define TM_ASN1_TAG_BITSTRING               3    
#define TM_ASN1_TAG_OCTETSTRING             4
#define TM_ASN1_TAG_NULL                    5
#define TM_ASN1_TAG_OBJECT                  6
#define TM_ASN1_TAG_OBJECT_DESCRIPTOR       7
#define TM_ASN1_TAG_EXTERNAL                8
#define TM_ASN1_TAG_REAL                    9
#define TM_ASN1_TAG_ENUMERATED              10
#define TM_ASN1_TAG_NEG_ENUMERATED          (10 | TM_ASN1_TAG_NEG)
#define TM_ASN1_TAG_UTF8STRING              12
#define TM_ASN1_TAG_SEQUENCE                (tt8Bit)16
#define TM_ASN1_TAG_SET                     (tt8Bit)17
#define TM_ASN1_TAG_NUMERICSTRING           18    
#define TM_ASN1_TAG_PRINTABLESTRING         19
#define TM_ASN1_TAG_T61STRING               20
#define TM_ASN1_TAG_TELETEXSTRING           20    
#define TM_ASN1_TAG_VIDEOTEXSTRING          21    
#define TM_ASN1_TAG_IA5STRING               22
#define TM_ASN1_TAG_UTCTIME                 23
#define TM_ASN1_TAG_GENERALIZEDTIME         24    
#define TM_ASN1_TAG_GRAPHICSTRING           25    
#define TM_ASN1_TAG_ISO64STRING             26    

#define TM_ASN1_TAG_VISIBLESTRING           26    
#define TM_ASN1_TAG_GENERALSTRING           27    
#define TM_ASN1_TAG_UNIVERSALSTRING         28    
#define TM_ASN1_TAG_BMPSTRING               30
    
typedef struct tsAsn1ObjectSt
{
    ttCharPtr objShortName;
    ttCharPtr objLongName;
    ttCharPtr objIdStr;
    ttCharPtr objDerStr;
} ttAsn1Object;
typedef ttAsn1Object           TM_FAR * ttAsn1ObjectPtr;
typedef ttAsn1ObjectPtr        TM_FAR * ttAsn1ObjectPtrPtr;

typedef struct tsAsn1IntegerSt           ttAsn1GenericType;
typedef ttAsn1GenericType      TM_FAR *  ttAsn1GenericTypePtr;
typedef ttAsn1GenericTypePtr   TM_FAR *  ttAsn1GenericTypePtrPtr;

typedef int                             ttAsn1Boolean;
typedef int                             ttAsn1Null;

#define ttAsn1String                    ttAsn1GenericType
#define ttAsn1Enumerated                ttAsn1GenericType
#define ttAsn1BitString                 ttAsn1GenericType
#define ttAsn1OctetString               ttAsn1GenericType
#define ttAsn1PrintableString           ttAsn1GenericType
#define ttAsn1T16String                 ttAsn1GenericType
#define ttAsn1IA5String                 ttAsn1GenericType
#define ttAsn1GeneralString             ttAsn1GenericType
#define ttAsn1UniversalString           ttAsn1GenericType
#define ttAsn1BmpString                 ttAsn1GenericType
#define ttAsn1UTCTime                   ttAsn1GenericType
#define ttAsn1Time                      ttAsn1GenericType
#define ttAsn1GeneralizedTime           ttAsn1GenericType
#define ttAsn1VisibleString             ttAsn1GenericType
#define ttAsn1UTF8String                ttAsn1GenericType

#define ttAsn1StringPtr                 ttAsn1GenericTypePtr
#define ttAsn1EnumeratedPtr             ttAsn1GenericTypePtr
#define ttAsn1BitStringPtr              ttAsn1GenericTypePtr
#define ttAsn1OctetStringPtr            ttAsn1GenericTypePtr
#define ttAsn1PrintableStringPtr        ttAsn1GenericTypePtr
#define ttAsn1T16StringPtr              ttAsn1GenericTypePtr
#define ttAsn1IA5StringPtr              ttAsn1GenericTypePtr
#define ttAsn1GeneralStringPtr          ttAsn1GenericTypePtr
#define ttAsn1UniversalStringPtr        ttAsn1GenericTypePtr
#define ttAsn1BmpStringPtr              ttAsn1GenericTypePtr
#define ttAsn1UTCTimePtr                ttAsn1GenericTypePtr
#define ttAsn1TimePtr                   ttAsn1GenericTypePtr
#define ttAsn1GeneralizedTimePtr        ttAsn1GenericTypePtr
#define ttAsn1VisibleStringPtr          ttAsn1GenericTypePtr
#define ttAsn1UTF8StringPtr             ttAsn1GenericTypePtr

#define ttAsn1StringPtrPtr              ttAsn1GenericTypePtrPtr
#define ttAsn1EnumeratedPtrPtr          ttAsn1GenericTypePtrPtr
#define ttAsn1BitStringPtrPtr           ttAsn1GenericTypePtrPtr
#define ttAsn1OctetStringPtrPtr         ttAsn1GenericTypePtrPtr
#define ttAsn1PrintableStringPtrPtr     ttAsn1GenericTypePtrPtr
#define ttAsn1T16StringPtrPtr           ttAsn1GenericTypePtrPtr
#define ttAsn1IA5StringPtrPtr           ttAsn1GenericTypePtrPtr
#define ttAsn1GeneralStringPtrPtr       ttAsn1GenericTypePtrPtr
#define ttAsn1UniversalStringPtrPtr     ttAsn1GenericTypePtrPtr
#define ttAsn1BmpStringPtrPtr           ttAsn1GenericTypePtrPtr
#define ttAsn1UTCTimePtrPtr             ttAsn1GenericTypePtrPtr
#define ttAsn1TimePtrPtr                ttAsn1GenericTypePtrPtr
#define ttAsn1GeneralizedTimePtrPtr     ttAsn1GenericTypePtrPtr
#define ttAsn1VisibleStringPtrPtr       ttAsn1GenericTypePtrPtr
#define ttAsn1UTF8StringPtrPtr          ttAsn1GenericTypePtrPtr



/* a:dump, b: outPtrPtr,c:tag */
#define tm_asn1_taghead_enc(a,b,c)  \
                        tfAsn1TagHeadEncode(b,\
                                TM_ASN1_UNIVERSAL_CLASS,\
                                TM_ASN1_PRIMITIVE_TYPE,\
                                c)
#define tm_asn1_taghead_gen_enc( a,b,c,d,e) \
                        tfAsn1TagHeadEncode(b,c,d,e)

#define tm_asn1_taghead_dec(a,b)  \
                        tfAsn1TagHeadDecode(a,b, \
                                TM_VOID_NULL_PTR,\
                                TM_VOID_NULL_PTR)
#define tm_asn1_taghead_gen_dec( a,b,c,d) \
                        tfAsn1TagHeadDecode(a,b,c,d)

#define tm_asn1_taglength_enc(a,b,c)    tfAsn1TagLengthEncode(a,b,c)
#define tm_asn1_taglength_dec(a,b)      tfAsn1TagLengthDecode(a,b)


#define tm_asn1_genlength_enc(a,b,c)    tfAsn1GenLengthEncode(a,b,c)
#define tm_asn1_genlength_dec(a, b)     tfAsn1GenLengthDecode(a,b)



#define tm_asn1_integer_enc(a,b,c)      tfAsn1IntegerEncode(a,b)
#define tm_asn1_integer_dec(a,b)        tfAsn1IntegerDecode(a,b)

#define tm_asn1_object_enc(a,b,c)       tfAsn1ObjectEncode(a,b)
#define tm_asn1_object_dec(a,b)         tfAsn1ObjectDecode(a,b)

#define tm_asn1_octetstring_enc(a,b,c)  tfAsn1OctetStringEncode(a,b)
#define tm_asn1_octetstring_dec(a,b)    tfAsn1OctetStringDecode(a,b)

#define tm_asn1_type_enc(a,b,c) \
            tm_asn1_octetstring_enc((ttAsn1StringPtr)a,b,c)
#define tm_asn1_type_dec(a,b) \
            tm_asn1_octetstring_dec((ttAsn1StringPtrPtr)a,b)

#define tm_asn1_string_enc(a,b,c)       tm_asn1_octetstring_enc(a,b,c)
#define tm_asn1_string_dec(a,b)         tm_asn1_octetstring_dec(a,b)

#define tm_asn1_time_enc(a,b,c) \
            tm_asn1_octetstring_enc((ttAsn1StringPtr)a,b,c)
#define tm_asn1_time_dec(a,b) \
            tm_asn1_octetstring_dec((ttAsn1StringPtrPtr)a,b)

#define tm_asn1_bitstring_enc(a,b,c)    tfAsn1BitStringEncode(a,b)
#define tm_asn1_bitstring_dec(a,b)      tfAsn1BitStringDecode(a,b)

#define tm_asn1_null_enc(a)             tfAsn1NullEncode(a)
#define tm_asn1_null_dec(a)             tfAsn1NullDecode(a)

#define tm_asn1_boolean_enc(a,b)        tfAsn1BooleanEncode(a,b)
#define tm_asn1_boolean_dec(a,b)        tfAsn1BooleanDecode(a,b)

#define tm_asn1_context_enc(a,b,c) \
                    tfAsn1ContextSpecificEncode(a,b,c)
#define tm_asn1_context_dec(a,b) \
                    tfAsn1ContextSpecificDecode(a,b)

#define tm_asn1_set_enc(a,b,c) \
                    tfAsn1SetEncode(a,b,c,TM_ASN1_CONSTRUCTED_TYPE);
#define tm_asn1_set_dec(a,b)   \
                    tfAsn1SetDecode(a,b, TM_ASN1_CONSTRUCTED_TYPE)

#define tm_asn1_sequence_enc(a,b,c) \
                    tfAsn1SequenceEncode(a,b,c,TM_ASN1_CONSTRUCTED_TYPE)
#define tm_asn1_sequence_dec(a,b)   \
                    tfAsn1SequenceDecode(a,b, TM_ASN1_CONSTRUCTED_TYPE)

#define tm_asn1_enc(source, dest,type)  tfAsn1Enc(source,dest,type)


/* X509 definition and IKE-PKI definition */

#define TM_PKI_MAX_STACK                20 

/* search object method */
#define TM_PKI_OBJECT_SEARCH_SN         0
#define TM_PKI_OBJECT_SEARCH_LN         1
#define TM_PKI_OBJECT_SEARCH_OBJ        2
#define TM_PKI_OBJECT_SEARCH_DER        3


/* define signature and authentication algorithm's object 
 * The following number corresponds to its sequence in tlPkiObject
 * see macro tm_x509_objid
 */
#define TM_PKI_OBJ_RSA                  3
#define TM_PKI_OBJ_RSAMD2               4
#define TM_PKI_OBJ_RSAMD5               5
#define TM_PKI_OBJ_RSASHA1              6
#define TM_PKI_OBJ_RSASHA256            7
#define TM_PKI_OBJ_RSASHA384            8
#define TM_PKI_OBJ_RSASHA512            9
#define TM_PKI_OBJ_RSARIPEMD            10
#define TM_PKI_OBJ_DSA                  11
#define TM_PKI_OBJ_DSASHA1              12
#define TM_PKI_OBJ_MD2                  13 
#define TM_PKI_OBJ_MD5                  14
#define TM_PKI_OBJ_SHA1                 15
#define TM_PKI_OBJ_SHA256               16
#define TM_PKI_OBJ_SHA384               17
#define TM_PKI_OBJ_SHA512               18

/* define signature and verify type */
#define TM_X509_SIGN_REQ                0
#define TM_X509_SIGN_CERT               1
#define TM_X509_SIGN_CRL                2

#define TM_X509_SIGNATURE               0
#define TM_X509_VERIFY                  1

/* certificate search key */
#define TM_PKI_CERTSEARCH_ID            0
#define TM_PKI_CERTSEARCH_ISSUER        1
#define TM_PKI_CERTSEARCH_SUBJECT       2
#define TM_PKI_CERTSEARCH_SERIALNUM     3


/* X509 DER encoding and decoding definitions*/
#define TM_X509_CERT                        0
#define TM_X509_CERTREQ                     1
#define TM_X509_CERTLIST                    2
#define TM_X509_TBSCERT                     3
#define TM_X509_CERTREQINFO                 4
#define TM_X509_TBSCERTLIST                 5
#define TM_X509_ALGORID                     6
#define TM_X509_NAME                        7
#define TM_X509_SUBPKINFO                   8
#define TM_X509_VALIDITY                    9
#define TM_X509_EXTENSION                   10
#define TM_X509_ATTRTYPEVALUE               11
#define TM_X509_REVOKEDCERT                 12
#define TM_X509_ATTRIBUTE                   13

#define TM_X509_EXTENSION_ARRAY             14
#define TM_X509_REVOKEDCERT_ARRAY           15

/* X509 CRL version */
#define TM_X509_CRL_VERSION_1               1
#define TM_X509_CRL_VERSION_2               2

/* DER encoding and decoding for X509, we should have:
 *  Cert - TBSCert - algorId,Name, Validity, subPkInfo,extension
 *                           Name-attrTypeValue
 *  CertReq - CertReqInfo - reqInfoSubPkInfoPtr 
 *  CertList - TBSCertList - RevokedCert 
 *  
 *************************************************************/
/* a: X509 structure 
 * b: string pointer 
 * c: length of string 
 * d: type
 *************************************************************/

#define tm_x509_generic_enc(a,b,c,d)    tfX509GenericEncode(a,b,c,d)
#define tm_x509_generic_dec(a,b,c,d)    tfX509GenericDecode(a,b,c,d)

#define tm_x509_rsa_enc(a,b)            tfX509RsaPubKeyEncode(a,b)
#define tm_x509_rsa_dec(a,b)            tfX509RsaPubKeyDecode(a,b)

#define tm_x509_dsa_enc(a,b)            tfX509DsaPubKeyEncode(a,b)
#define tm_x509_dsa_dec(a,b)            tfX509DsaPubKeyDecode(a,b)


/* for CRL */

#define tm_pki_global(a)    (((ttPkiGlobalPtr)(tm_context(tvPkiPtr)))->a)
#define tm_pki_is_dsa(a) \
            ((a == TM_PKI_OBJ_DSA )||( a == TM_PKI_OBJ_DSASHA1))?1:0
#define tm_pki_is_rsa(a) \
            ((a == TM_PKI_OBJ_RSA )||( a == TM_PKI_OBJ_RSASHA1) ||\
             (a == TM_PKI_OBJ_RSASHA256 )||( a == TM_PKI_OBJ_RSASHA384) ||\
             (a == TM_PKI_OBJ_RSASHA512 )||(a == TM_PKI_OBJ_RSAMD5)||\
             (a==TM_PKI_OBJ_RSAMD2))?1:0


typedef struct tsPkiPublicKey
{
    ttVoidPtr   pkKeyPtr;
    int         pkSigLen;
    int         pkType;
} ttPkiPublicKey;
typedef ttPkiPublicKey TM_FAR * ttPkiPublicKeyPtr;



/* RFC 3280 Page 97 */
typedef struct tsX509AlgorIden
{
    ttAsn1ObjectPtr         algorObjPtr; 
    ttAsn1StringPtr         algorParPtr; 
} ttX509AlgorIden;
typedef ttX509AlgorIden TM_FAR * ttX509AlgorIdenPtr;


/* RFC 3280 Page 14 */
typedef struct tsX509Validity
{
    ttAsn1TimePtr           notBeforePtr;
    ttAsn1TimePtr           notAfterPtr;
} ttX509Validity;
typedef ttX509Validity TM_FAR * ttX509ValidityPtr;


typedef struct tsX509SubPubKeyInfo
{
    ttX509AlgorIdenPtr      subPkAlgorPtr;
    ttAsn1BitStringPtr      subPkValuePtr; 
} ttX509SubPubKeyInfo;
typedef ttX509SubPubKeyInfo TM_FAR * ttX509SubPubKeyInfoPtr;

/* RFC 3280 Page 17 */
typedef struct tsX509AttrTypeValue
{
    ttAsn1ObjectPtr         atvObjPtr;
    ttAsn1StringPtr         atvValuePtr;
   
} ttX509AttrTypeValue;
typedef ttX509AttrTypeValue TM_FAR * ttX509AttrTypeValuePtr;

typedef struct tsX509NameSt
{
    ttX509AttrTypeValuePtr  nameRelativeDNPtr[TM_PKI_MAX_STACK];
    int                     nameNum;  
} ttX509Name;                         
typedef ttX509Name          TM_FAR * ttX509NamePtr;

/* RFC 3280 Page 15 */
typedef struct tsX509Extension
{
    ttAsn1ObjectPtr         extnIDPtr;
/* short */
    ttAsn1Boolean           extnCritical;
    ttAsn1OctetStringPtr    extnValuePtr;

} ttX509Extension;
typedef ttX509Extension     TM_FAR * ttX509ExtensionPtr;
typedef ttX509ExtensionPtr  TM_FAR * ttX509ExtensionPtrPtr;


/* RFC 3280 Page 14 */
typedef struct tsX509TBSCert
{
/* [0] default of v1 */
    ttAsn1Integer           tbsVersion;        
    ttAsn1Integer           tbsSerialNumber;
    ttX509AlgorIdenPtr      tbsSignaturePtr;
    ttX509NamePtr           tbsIssuerPtr;
    ttX509ValidityPtr       tbsValidityPtr;
    ttX509NamePtr           tbsSubjectPtr;
    ttX509SubPubKeyInfoPtr  tbsSubPkInfoPtr;
/* [1] version must be v2 or v3 */
    ttAsn1BitStringPtr      tbsIssuerUIDPtr;
/* [2] version must be v2 or v3 */
    ttAsn1BitStringPtr      tbsSubjectUIDPtr;
/* [3] version must be v3 */
    ttX509ExtensionPtr      tbsExtensionsPtr[TM_PKI_MAX_STACK];
    
} ttX509TBSCert;
typedef ttX509TBSCert       TM_FAR * ttX509TBSCertPtr;

typedef struct tsX509Cert
{
    ttX509TBSCertPtr        certTbsPtr;
    ttX509AlgorIdenPtr      certAlgorPtr;
    ttAsn1BitStringPtr      certSigPtr;
} ttX509Cert;
typedef ttX509Cert          TM_FAR * ttX509CertPtr;


/* PKCS #9 */
typedef struct tsX509Attribute
{
    ttAsn1ObjectPtr         attObjPtr;
   
    ttAsn1StringPtr         attValuesPtr[TM_PKI_MAX_STACK];
} ttX509Attribute;
typedef ttX509Attribute     TM_FAR * ttX509AttributePtr;

typedef struct tsX509CertReqInfo
{
    ttAsn1Integer           reqInfoVersion;
    ttX509NamePtr           reqInfoSubjectPtr;
    ttX509SubPubKeyInfoPtr  reqInfoSubPkInfoPtr;
    ttX509AttributePtr      reqInfoAttPtr[TM_PKI_MAX_STACK];
} ttX509CertReqInfo ;
typedef ttX509CertReqInfo   TM_FAR * ttX509CertReqInfoPtr;

typedef struct tsX509CertReq
{
    ttX509CertReqInfoPtr    certReqInfoPtr;
    ttX509AlgorIdenPtr      certReqAlgorPtr;
    ttAsn1BitStringPtr      certReqSigPtr;
} ttX509CertReq;
typedef ttX509CertReq       TM_FAR * ttX509CertReqPtr;


/* RFC 3280 CRL, page 48 */

typedef struct tsX509RevokedCert
{
    ttAsn1IntegerPtr        revUserCertPtr;
    ttAsn1TimePtr           revRevocationDatePtr;
/* optional, must be v2 if present */
    ttX509ExtensionPtr      revCrlEntryExtPtr[TM_PKI_MAX_STACK];
} ttX509RevokedCert;
typedef ttX509RevokedCert   TM_FAR * ttX509RevokedCertPtr;

/* TBS Cert List */
typedef struct tsX509TBSCertList
{
    ttAsn1Integer           tclVersion;
    ttX509AlgorIdenPtr      tclSignaturePtr;
    ttX509NamePtr           tclIssuerPtr;
    ttAsn1TimePtr           tclThisUpdatePtr;
    ttAsn1TimePtr           tclNextUpdatePtr;
/* optional */
    ttX509RevokedCertPtr    tclRevokedPtr[TM_PKI_MAX_STACK];
/* must be v2 if present */
    ttX509ExtensionPtr      tclCrlExtensionsPtr[TM_PKI_MAX_STACK];
} ttX509TBSCertList;
typedef ttX509TBSCertList   TM_FAR * ttX509TBSCertListPtr;

typedef struct tsX509CertList
{
    ttX509TBSCertListPtr    crlTbsCertListPtr;
    ttX509AlgorIdenPtr      crlAlgorPtr;
    ttAsn1BitStringPtr      crlSigPtr;
} ttX509CertList;
typedef ttX509CertList      TM_FAR * ttX509CertListPtr;

/* PKCS #10 */
typedef struct tsX509GenSigned
{
    ttVoidPtr               genToBeSignedPtr;
    ttX509AlgorIdenPtr      genAlgorPtr;
    ttAsn1BitStringPtr      genSigPtr;
} ttX509GenSigned;
typedef ttX509GenSigned     TM_FAR * ttX509GenSignedPtr;


/* 
    certContentPtr -- certificate content, or CRL content
    certCAPtr      -- issuer CA that issues the certificate
                   by list, we can trace to Root if certCAPtr is null
    certKeyPtr     -- public key or all key for current certificate 
    certIdPtr      -- Id
    certDNPtr      -- only for CA, DN of issuer
    certPkPtr      -- only for Own CA, publick key ptr
    certLength     -- length of certContent
    certIdLength   -- Id length
 */

typedef struct tsPkiCertListSt
{
    struct tsPkiCertListSt TM_FAR * 
                            certLinkNextPtr;
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
/* for ssl, we need to keep a seperate link of certificates
 * for each session, so that 
 * 
 */
    struct tsPkiCertListSt TM_FAR *
                            certSslNextPtr;
#ifdef TM_USE_SSL_VERSION_33
    tt8Bit                  certDigestIndex;
#endif /* TM_USE_SSL_VERSION_33 */
#endif /* SSL */
#ifdef TM_USE_IKE
    struct tsPkiCertListSt TM_FAR *
                            certIsaNextPtr;
#ifdef TM_USE_IKEV2
    /* for [CERTREQ] */
    tt8Bit                  certSubPubkeyInfoHash[TM_SHA1_HASH_SIZE];
#endif /* TM_USE_IKEV2 */
#endif /* TM_USE_IKE */
/* Certificate format is DER */
    ttVoidPtr               certContentPtr;
    ttVoidPtr               certCAPtr;

    ttPkiPublicKeyPtr       certKeyPtr;
    ttVoidPtr               certIdPtr;
    ttVoidPtr               certDNPtr;
    ttVoidPtr               certSerNumPtr;   
    ttVoidPtr               certValidBeforePtr;
    ttVoidPtr               certValidAfterPtr;
    ttVoidPtr               certSubPtr;

    tt16Bit                 certDNLength;
    tt16Bit                 certSubLength;
    tt16Bit                 certLength;
    tt16Bit                 certIdLength;

    tt8Bit                  certSerNumLength;
    tt8Bit                  certRevoked;
    tt8Bit                  certCheckOut;
    tt8Bit                  certPadding;
       
} ttPkiCertList;
typedef ttPkiCertList TM_FAR *    ttPkiCertListPtr;
typedef ttPkiCertListPtr TM_FAR * ttPkiCertListPtrPtr;

/* 
    pkiCertListHdrPtr -- header pointer for all other entity cert list
    pkiCRLListHdrPtr  -- header pointer for all CRL
    pkiOwnCertListHdrPtr -- self entity certificate list ( all multi cert)
    pkiOwnKeyPtr      -- temperally store key pair for self cert, we need
                         add one ownkey pair and then one local certificate.
                         And the local certificate will points to this 
                         ownKeyPtr. After local cert is added, you can zero
                         this pointer (because it is temperal)

 */
#define TM_PKI_CERT_OTHER_INDX      0
#define TM_PKI_CERT_CRL_INDX        1
#define TM_PKI_CERT_SELF_INDX       2
#define TM_PKI_CERT_LIST_MAX_INDX   (TM_PKI_CERT_SELF_INDX + 1)
#define pkiCertListHdrPtr    pkiCertListHdrPtrArr[TM_PKI_CERT_OTHER_INDX]
#define pkiCRLListHdrPtr     pkiCertListHdrPtrArr[TM_PKI_CERT_CRL_INDX]
#define pkiOwnCertListHdrPtr pkiCertListHdrPtrArr[TM_PKI_CERT_SELF_INDX]
typedef struct tsPkiGlobalSt
{
    ttPkiCertListPtr        pkiCertListHdrPtrArr[TM_PKI_CERT_LIST_MAX_INDX];
    ttPkiPublicKeyPtr       pkiOwnKeyPtr;
    ttAsn1ObjectPtr         pkiObjectPtr;
/* Maximum allowed key size. Default 0, no limit */
    tt16Bit                 pkiMaxKeySize;
/* PKI will check certificate is alive. This is only valid
 * when TM_PKI_CERT_CHECK_ALIVE is defined */
    tt8Bit                  pkiCertCheckAlive;
/* PKI will NOT verrify certificate. This is only valid
 * when TM_PKI_CERT_NOT_VERIFY is defined */
    tt8Bit                  pkiCertNotVerify;
} ttPkiGlobal;
typedef ttPkiGlobal TM_FAR * ttPkiGlobalPtr;

TM_PROTO_EXTERN int tfAsn1Enc( ttVoidPtr source, ttVoidPtr dest, int type);

TM_PROTO_EXTERN int tfAsn1IntegerSet(ttAsn1IntegerPtr a,  ttS32Bit v);
TM_PROTO_EXTERN ttS32Bit tfAsn1IntegerGet(ttAsn1IntegerPtr a);


TM_PROTO_EXTERN int tfAsn1StringSet(ttAsn1StringPtr strPtr, 
                                        ttVoidPtr dataPtr, int len);



TM_PROTO_EXTERN void tfAsn1StringInit(  ttAsn1StringPtr     strPtr);

TM_PROTO_EXTERN int tfAsn1ObjectEncode (ttAsn1ObjectPtr     objPtr, 
                                        tt8BitPtrPtr        outPtrPtr);
TM_PROTO_EXTERN int tfAsn1ObjectDecode (ttAsn1ObjectPtrPtr  objPtrPtr, 
                                        tt8BitPtrPtr        inPtrPtr);

TM_PROTO_EXTERN int tfAsn1OctetStringEncode ( ttAsn1OctetStringPtr octPtr,
                                             tt8BitPtrPtr outPtrPtr);
TM_PROTO_EXTERN int tfAsn1OctetStringDecode ( 
                                    ttAsn1OctetStringPtrPtr octPtrPtr,
                                    tt8BitPtrPtr inPtrPtr);

TM_PROTO_EXTERN int tfAsn1BitStringEncode ( ttAsn1BitStringPtr bitPtr,
                                           tt8BitPtrPtr outPtrPtr);
TM_PROTO_EXTERN int tfAsn1BitStringDecode ( 
                                    ttAsn1BitStringPtrPtr bitPtrPtr,
                                    tt8BitPtrPtr inPtrPtr);

TM_PROTO_EXTERN int tfAsn1IntegerEncode ( ttAsn1IntegerPtr intPtr,
                                         tt8BitPtrPtr outPtrPtr);
TM_PROTO_EXTERN int tfAsn1IntegerDecode ( ttAsn1IntegerPtrPtr intPtrPtr,
                                          tt8BitPtrPtr           inPtrPtr );

TM_PROTO_EXTERN int tfAsn1NullEncode ( tt8BitPtrPtr outPtrPtr);
TM_PROTO_EXTERN int tfAsn1NullDecode ( tt8BitPtrPtr inPtrPtr);

TM_PROTO_EXTERN int tfAsn1BooleanEncode ( int value, tt8BitPtrPtr outPtrPtr);
TM_PROTO_EXTERN int tfAsn1BooleanDecode ( ttIntPtr valuePtr, 
                                          tt8BitPtrPtr inPtrPtr);


TM_PROTO_EXTERN int tfAsn1TagHeadEncode (   tt8BitPtrPtr    outPtrPtr, 
                                            tt8Bit          classValue, 
                                            tt8Bit          typeValue,
                                            int             tag);

TM_PROTO_EXTERN int tfAsn1TagHeadDecode (   ttIntPtr        tagPtr,   
                                            tt8BitPtrPtr    inPtrPtr, 
                                            tt8BitPtr       classPtr, 
                                            tt8BitPtr       typePtr );





TM_PROTO_EXTERN int  tfAsn1SetEncode (  ttVoidPtr       dump, 
                                        tt8BitPtrPtr    outPtrPtr,
                                        int             length,
                                        tt8Bit          class );

TM_PROTO_EXTERN int  tfAsn1SetDecode (  ttVoidPtr       dump, 
                                        tt8BitPtrPtr    inPtrPtr,
                                        tt8Bit          class );

TM_PROTO_EXTERN int  tfAsn1SequenceEncode (  ttVoidPtr  dump, 
                                        tt8BitPtrPtr    outPtrPtr,
                                        int         length,
                                        tt8Bit      class );

TM_PROTO_EXTERN int  tfAsn1SequenceDecode (  ttVoidPtr   dump, 
                                        tt8BitPtrPtr     inPtrPtr,
                                        tt8Bit           class );

TM_PROTO_EXTERN int  tfAsn1GenLengthEncode (  ttVoidPtr  dump, 
                                        tt8BitPtrPtr     outPtrPtr,
                                        int              length );

TM_PROTO_EXTERN int  tfAsn1GenLengthDecode (ttIntPtr        lenPtr, 
                                            tt8BitPtrPtr    inPtrPtr);

TM_PROTO_EXTERN int tfAsn1TagLengthEncode ( ttVoidPtr dump, 
                                           tt8BitPtrPtr outPtrPtr,int length);
TM_PROTO_EXTERN int tfAsn1TagLengthDecode (ttIntPtr tagPtr, 
                                           tt8BitPtrPtr inPtrPtr);



TM_PROTO_EXTERN int tfAsn1ContextSpecificEncode (  tt8Bit tag,
                                               tt8BitPtrPtr outPtrPtr, 
                                               int length); 
TM_PROTO_EXTERN int tfAsn1ContextSpecificDecode (ttIntPtr    lenPtr,
                                                 tt8BitPtrPtr inPtrPtr) ;



TM_PROTO_EXTERN ttAsn1StringPtr tfAsn1StringNew(int type);
TM_PROTO_EXTERN int tfAsn1StringFree( ttAsn1StringPtr strPtr);

TM_PROTO_EXTERN ttAsn1TimePtr tfAsn1TimeNew(ttS32Bit offset);



TM_PROTO_EXTERN int tfX509GenericEncode (   ttVoidPtr       vPtr, 
                                            tt8BitPtrPtr    derPtrPtr, 
                                            ttIntPtr        length,
                                            int type);

TM_PROTO_EXTERN int tfX509GenericDecode (   ttVoidPtr       vPtr, 
                                            tt8BitPtrPtr    derPtrPtr, 
                                            ttIntPtr        length,
                                            int             type);


#ifdef TM_PUBKEY_USE_RSA
#define tm_pki_rsa_size(keyPtr)   \
                (((ttGeneralRsaPtr)keyPtr)->rsaN->asn1Length)  
TM_PROTO_EXTERN ttVoidPtr tfRsaKeyAsn1ToBn( ttGeneralRsaPtr rsaPtr);
TM_PROTO_EXTERN int tfX509RsaPubKeyEncode (     ttVoidPtr       vPtr, 
                                            tt8BitPtrPtr    derPtrPtr); 
TM_PROTO_EXTERN int tfX509RsaPubKeyDecode (     ttVoidPtrPtr       vPtr, 
                                            tt8BitPtrPtr    derPtrPtr); 
void tfPkiGeneralRsaFree(ttGeneralRsaPtr rsaPtr);
#endif /* TM_PUBKEY_USE_RSA */

#ifdef TM_PUBKEY_USE_DSA
#define tm_pki_dsa_size(keyPtr)   \
                (20+2*((ttGeneralDsaPtr)keyPtr)->dsaQ->asn1Length)
TM_PROTO_EXTERN ttVoidPtr tfDsaKeyAsn1ToBn( ttGeneralDsaPtr dsaPtr);




TM_PROTO_EXTERN int tfX509DsaPubKeyEncode (     
                                ttVoidPtr       vPtr, 
                                tt8BitPtrPtr    derPtrPtr) ;
TM_PROTO_EXTERN int tfX509DsaPubKeyDecode (     
                                ttVoidPtrPtr    vPtrPtr, 
                                tt8BitPtrPtr    derPtrPtr) ;
#endif /* TM_PUBKEY_USE_DSA */


/* Object operation API */

TM_PROTO_EXTERN ttVoidPtr               tfPkiObjSearch(    
                                            ttVoidPtr   key, 
                                            int         method, 
                                            int         len);

TM_PROTO_EXTERN ttVoidPtr               tfPkiAsn1ObjIdSearch(
                                            ttVoidPtr   keyPtr,
                                            int         method,
                                            int         len);

TM_PROTO_EXTERN ttAsn1ObjectPtr         tfPkiObjDuplicate(
                                            ttAsn1ObjectPtr objPtr  );
TM_PROTO_EXTERN int                     tfPkiObjIdSearch(ttCharPtr s);
TM_PROTO_EXTERN int                     tfX509ObjectInit(void);
TM_PROTO_EXTERN void                    tfX509ObjectFree(void);
TM_PROTO_EXTERN int                     tfX509GetObjId(tt8BitPtr x);
/* Name and PubKeyInfo API */

TM_PROTO_EXTERN int                     tfX509NameAddById ( 
                                            ttX509NamePtr name, 
                                            ttCharPtr     value, 
                                            int           nid ); 

TM_PROTO_EXTERN int                     tfX509PubKeyInfoSet (   
                                            ttX509SubPubKeyInfoPtr  subPk, 
                                            ttVoidPtr               pkey);
TM_PROTO_EXTERN ttPkiPublicKeyPtr       tfX509PubKeyInfoGet ( 
                                            ttX509SubPubKeyInfoPtr   subPk);


/* certificate api */

tt8BitPtr  tfX509PemToDer ( tt8BitPtr pemPtr, tt16BitPtr lenPtr);
tt8BitPtr  tfX509DerToPem ( tt8BitPtr derPtr, 
                                   int length, tt16BitPtr lenPtr);

TM_PROTO_EXTERN ttX509CertPtr           tfX509CertNew(void);
TM_PROTO_EXTERN int tfX509CertFree(ttX509CertPtr certPtr);

TM_PROTO_EXTERN ttX509CertListPtr   tfX509CRLNew(void);
TM_PROTO_EXTERN int tfX509CRLFree( ttX509CertListPtr ptr);

#if (defined(TM_PUBKEY_USE_RSA) || defined(TM_PUBKEY_USE_DSA))
TM_PROTO_EXTERN int tfX509CertVerify( ttVoidPtr reqin, 
                                      ttVoidPtr certPtr, 
                                      ttVoidPtr keyin);
TM_PROTO_EXTERN int tfX509GenericFunc (int          length, 
                                       ttVoidPtr    source, 
                                       ttIntPtr     sigLen,
                                       tt8BitPtrPtr sig, 
                                       ttVoidPtr    key,
                                       int          method,
                                       int          type );
#endif /* (defined(TM_PUBKEY_USE_RSA) || defined(TM_PUBKEY_USE_DSA)) */

TM_PROTO_EXTERN int tfX509CertAlive(ttPkiCertListPtr certPtr);

TM_PROTO_EXTERN ttPkiCertListPtr tfPkiCertificateSearch(
                                        int              type,
                                        ttConstVoidPtr   keyPtr,
                                        int              keyLength,
                                        int              certListIndex,
                                        tt8Bit           needLock);

TM_PROTO_EXTERN ttPkiPublicKeyPtr tfPkiCertificateReadKey( 
                                        ttVoidPtr           idPtr,
                                        int                 idLength,
                                        int                 certListIndex); 




TM_PROTO_EXTERN int tfPkiCertificateCheckAdd(
                            ttPkiCertListPtrPtr  retCertPtrPtr,
                            ttCharPtr            commonName,
                            ttUserConstVoidPtr   fileNamePtr, 
                            int                  typeFormat,
                            int                  typeHeader,
                            ttUserConstVoidPtr   idPtr,
                            int                  idLength,
                            ttUserConstVoidPtr   caIdPtr);

TM_PROTO_EXTERN tt8BitPtr   tfPkiGetStringInFile (
                                ttConstVoidPtr  directory,
                                int             typeFormat,
                                tt16BitPtr      lenPtr);

TM_PROTO_EXTERN int         tfPkiPutStringToFile (      
                                ttConstVoidPtr   directory, 
                                int         typeFormat,
                                tt8BitPtr   bufferPtr,
                                tt16Bit     length,
                                ttCharPtr   titlePtr);
TM_PROTO_EXTERN int tfPkiOwnKeyPairAddTo(ttConstVoidPtr fileNamePtr,
                                         int            typeFormat,
                                         int            keyType,
                                         ttVoidPtrPtr   toAddrPtr,
                                         int            checkExport);
TM_PROTO_EXTERN void tfPkiCertCleanup(ttPkiCertListPtr certPtr);
#ifdef TM_USE_SSL_VERSION_33
TM_EXTERN const ttAsn1Object tlPkiConstObject[];
#endif /* TM_USE_SSL_VERSION_33 */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _TRPKI_H_ */




