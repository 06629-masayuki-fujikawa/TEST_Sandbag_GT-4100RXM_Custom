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
 * Description: SSL and TLS implementation
 *
 * Filename: trssl.c
 * Author: Jin Zhang
 * Date Created:   10/27/2003
 * $Source: source/trssl.c $
 *
 * Modification History
 * $Revision: 6.0.2.43 $
 * $Date: 2016/05/13 11:13:25JST $
 * $Author: pcarney $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * Include
 */
#include <trsocket.h>
#include <trmacro.h>

#if (defined(TM_USE_SSL_SERVER) || defined (TM_USE_SSL_CLIENT))

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>
#include <trpki.h>
#include <trcrylib.h>
#include <trssl.h>
#include <tropnssl.h>

static void tfSslCleanupPeerCerts(ttSslConnectStatePtr connStatePtr);
static void tfSslSessionClose(ttSslSessionPtr         sslSessionPtr,
                              int                     tcpsSslFlags);
static int tfSslCacheConnectInfo(ttSslConnectStatePtr connStatePtr);
static int tfSslRecordPacketTransform(
                                     ttSslConnectStatePtr     connStatePtr,
                                     ttPacketPtr              packetPtr,
                                     ttPktLen                 offset,
                                     ttPacketPtr              newPacketPtr,
                                     tt16Bit                  dataLength,
                                     tt8Bit                   direction);
static int tfSslMakeRecordContiguous(ttSslConnectStatePtr     connStatePtr,
                                     ttPacketPtrPtr           packetPtrPtr,
                                     tt16Bit                  recordSize);
static int tfSslRecordDecodeMacVerify(ttSslConnectStatePtr    connStatePtr,
                                      ttSslRecordHeaderPtr    sslHeaderPtr,
                                      ttPacketPtr             packetPtr,
                                      ttPktLen                offset,
                                      ttPktLen                dataLength,
                                      tt16BitPtr              origDataLenPtr);
static int tfSslHasMoreRecord(ttSslConnectStatePtr   connStatePtr,
                              ttPacketPtr            nextPacketPtr,
                              tt8BitPtr              hasMoreRecordPtr);
static void tfSslCutQueuedPacketList(ttPacketPtr         origPacketPtr,
                                     tt16Bit             recordSize,
                                     ttPacketPtrPtr      firstPtrPtr,
                                     ttPacketPtrPtr      secondPtrPtr);
static ttPacketPtr tfSslBuildRecordFromUserData(
                                  ttTcpVectPtr           tcpVectPtr,
                                  ttSslConnectStatePtr   connStatePtr);

static int tfSslSessionCloseConnection(ttNodePtr       nodePtr,
                                       int             tcpSslFlags);

/* This local variable determines which cipher suites(s) we are going to
 * offer. The user function tfSslSetSessionProposals may change the value and
 * offers different cipher suites
 */
static const tt16Bit TM_CONST_QLF tlSslProposalSuites[TM_SSL_PROPOSAL_NUMBER] =
{
#ifndef TM_EXPORT_RESTRICT
    TM_TLS_RSA_RC4_128_SHA,
    TM_TLS_RSA_RC4_128_MD5,
    TM_TLS_RSA_AES_128_CBC_SHA,
    TM_TLS_DHE_DSS_AES_128_CBC_SHA,
    TM_TLS_RSA_3DES_EDE_CBC_SHA,
    TM_TLS_DHE_DSS_3DES_EDE_CBC_SHA
#else /* TM_EXPORT_RESTRICT */
    TM_TLS_RSA_EPT1K_DES_CBC_SHA,
    TM_TLS_RSA_EPT1K_RC4_56_SHA,
    TM_TLS_RSA_EXPORT_RC4_40_MD5,
    TM_TLS_RSA_EXPORT_RC2_40_MD5,
    TM_TLS_DHE_DSS_EPT_DES40_CBC_SHA,
    TM_TLS_RSA_EXPORT_DES40_CBC_SHA,
#endif /* TM_EXPORT_RESTRICT */
};

#ifdef TM_USE_SSL_VERSION_30
/* AES256 + SHA1 needs 2*32 + 2*20 + 2*16 bytes key, needs MD5 to run
 * 9 iterations
 */
static const char TM_CONST_QLF tlSsl30Label[] =
"ABBCCCDDDDEEEEEFFFFFFGGGGGGGHHHHHHHHIIIIIIIII";

/*
 * DESCRIPTION: function tfSsl30GetLabelIndex is called to get the label
 * index provided the interation number. The relationship is as the following
 * get the label starting index using the iteration.
 *   Iteration             Label          index in tlSsl30Label
 *      1:                  "A"              0
 *      2                   "BB"             1
 *      3                   "CCC"            3
 *      4                   "DDDD"           6
 *      5                   "EEEEE"          10
 *      6                   "FFFFFF"         15
 * Iteration must begins with 1. The number of literals in each
 * iteration is equal to the iteration number. For example,
 * iteration 3, label "CCC" has 3 letter 'C'
 */
static int tfSsl30GetLabelIndex(int      iteration)
{
    int       i;
    int       sum;

    sum = 0;
    for (i = 1; i < iteration ; i++)
    {
        sum += i;
    }
    return sum;
}
#endif /* TM_USE_SSL_VERSION_30 */

#if (defined(TM_USE_SSL_VERSION_31) || defined(TM_USE_SSL_VERSION_33))

#define TM_TLS_MASTER_SECRET_LABEL       "master secret"
/* length of the above label */
#define TM_TLS_MASTER_SECRET_LABELLEN    13

#define TM_TLS_KEY_EXPAN_LABEL           "key expansion"
#define TM_TLS_KEY_EXPAN_LABELLEN        13

#define TM_TLS_CLIENT_WKEY_LABEL         "client write key"
#define TM_TLS_CLIENT_WKEY_LABELLEN      16

#define TM_TLS_SERVER_WKEY_LABEL         "server write key"
#define TM_TLS_SERVER_WKEY_LABELLEN      16

#define TM_TLS_IVBLOCK_LABEL             "IV block"
#define TM_TLS_IVBLOCK_LABELLEN          8
#endif /* TM_USE_SSL_VERSION_31 || TM_USE_SSL_VERSION_33 */


/* All the cipher suites we support */
static const ttSslCipherSuite TM_CONST_QLF tlSslCiphers[] =
{
/*TM_TLS_NULL_WITH_NULL_NULL     */
    {0x0000,             SADB_PUBKEY_NULL,   TM_SSL_KEXCH_NULL,
     SADB_EALG_NULL,     SADB_AALG_NULL,     0,     TM_SSL_EXPORTABLE},
/*TM_TLS_RSA_NULL_MD5            */
    {0x0001,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_RSA,
     SADB_EALG_NULL,     SADB_AALG_MD5HMAC,  0,     TM_SSL_EXPORTABLE},
/*TM_TLS_RSA_NULL_SHA            */
    {0x0002,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_RSA,
     SADB_EALG_NULL,     SADB_AALG_SHA1HMAC, 0,     TM_SSL_EXPORTABLE},
/*TM_TLS_RSA_EXPORT_RC4_40_MD5   */
    {0x0003,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_RSAEXPORT,
     SADB_EALG_ARCFOUR,  SADB_AALG_MD5HMAC,  5,     16},
/*TM_TLS_RSA_RC4_128_MD5         */
    {0x0004,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_RSA,
     SADB_EALG_ARCFOUR,  SADB_AALG_MD5HMAC,  16,    0},
/*TM_TLS_RSA_RC4_128_SHA         */
    {0x0005,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_RSA,
     SADB_EALG_ARCFOUR,  SADB_AALG_SHA1HMAC, 16,    0},
/*TM_TLS_RSA_EXPORT_RC2_40_MD5  */
    {0x0006,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_RSAEXPORT,
     SADB_EALG_RC2CBC,   SADB_AALG_MD5HMAC,   5,    16},
/*TM_TLS_RSA_IDEA_SHA            we don't support */
    {0x0007,             SADB_PUBKEY_NULL,   TM_SSL_KEXCH_NULL,
     SADB_EALG_NULL,     SADB_AALG_NULL,     0,     0},
/*TM_TLS_RSA_EXPORT_DES40_CBC_SHA*/
    {0x0008,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_RSAEXPORT,
     SADB_EALG_DESCBC,   SADB_AALG_SHA1HMAC, 5,     8},
/*TM_TLS_RSA_DES_CBC_SHA         */
    {0x0009,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_RSA,
     SADB_EALG_DESCBC,   SADB_AALG_SHA1HMAC, 8,     0},
/*TM_TLS_RSA_3DES_EDE_CBC_SHA    */
    {0x000a,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_RSA,
     SADB_EALG_3DESCBC,  SADB_AALG_SHA1HMAC, 24,    0},
/*TM_TLS_DH_DSS_EPT_DES40_CBC_SHA*/
    {0x000b,             SADB_PUBKEY_DSA,    TM_SSL_KEXCH_DH,
     SADB_EALG_DESCBC,   SADB_AALG_SHA1HMAC, 5,     8},
/*TM_TLS_DH_DSS_DES_CBC_SHA      */
    {0x000c,             SADB_PUBKEY_DSA,    TM_SSL_KEXCH_DH,
     SADB_EALG_DESCBC,   SADB_AALG_SHA1HMAC, 8,     0},
/*TM_TLS_DH_DSS_3DES_EDE_CBC_SHA */
    {0x000d,             SADB_PUBKEY_DSA,    TM_SSL_KEXCH_DH,
     SADB_EALG_3DESCBC,  SADB_AALG_SHA1HMAC, 24,    0},
/*TM_TLS_DH_RSA_EPT_DES40_CBC_SHA*/
    {0x000e,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_DH,
     SADB_EALG_DESCBC,   SADB_AALG_SHA1HMAC, 5,     8},
/*TM_TLS_DH_RSA_DES_CBC_SHA      */
    {0x000f,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_DH,
     SADB_EALG_DESCBC,   SADB_AALG_SHA1HMAC, 8,     0},
/*TM_TLS_DH_RSA_3DES_EDE_CBC_SHA */
    {0x0010,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_DH,
     SADB_EALG_3DESCBC,  SADB_AALG_SHA1HMAC, 24,    0},
/*TM_TLS_DHE_DSS_EPT_DES40_CBC_SHA*/
    {0x0011,             SADB_PUBKEY_DSA,    TM_SSL_KEXCH_DHE,
     SADB_EALG_DESCBC,   SADB_AALG_SHA1HMAC, 5,     8},
/*TM_TLS_DHE_DSS_DES_CBC_SHA     */
    {0x0012,             SADB_PUBKEY_DSA,    TM_SSL_KEXCH_DHE,
     SADB_EALG_DESCBC,   SADB_AALG_SHA1HMAC, 8,     0},
/*TM_TLS_DHE_DSS_3DES_EDE_CBC_SHA*/
    {0x0013,             SADB_PUBKEY_DSA,    TM_SSL_KEXCH_DHE,
     SADB_EALG_3DESCBC,  SADB_AALG_SHA1HMAC, 24,    0},
/*TM_TLS_DHE_RSA_EPT_DES40_CBC_SHA*/
    {0x0014,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_DHE,
     SADB_EALG_DESCBC,   SADB_AALG_SHA1HMAC, 5,     8},
/*TM_TLS_DHE_RSA_DES_CBC_SHA     */
    {0x0015,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_DHE,
     SADB_EALG_DESCBC,   SADB_AALG_SHA1HMAC, 8,     0},
/*TM_TLS_DHE_RSA_3DES_EDE_CBC_SHA*/
    {0x0016,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_DHE,
     SADB_EALG_3DESCBC,  SADB_AALG_SHA1HMAC, 24,    0},
/*  CipherSuite TLS_RSA_WITH_AES_128_CBC_SHA      = { 0x00, 0x2F };*/
    {0x002f,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_RSA,
     SADB_EALG_AESCBC,   SADB_AALG_SHA1HMAC, 16,    0},
/*   CipherSuite TLS_DH_DSS_WITH_AES_128_CBC_SHA   = { 0x00, 0x30 };*/
    {0x0030,             SADB_PUBKEY_DSA,    TM_SSL_KEXCH_DH,
     SADB_EALG_AESCBC,   SADB_AALG_SHA1HMAC, 16,    0},
/*   CipherSuite TLS_DH_RSA_WITH_AES_128_CBC_SHA   = { 0x00, 0x31 };*/
    {0x0031,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_DH,
     SADB_EALG_AESCBC,   SADB_AALG_SHA1HMAC, 16,    0},
/*   CipherSuite TLS_DHE_DSS_WITH_AES_128_CBC_SHA  = { 0x00, 0x32 };*/
    {0x0032,             SADB_PUBKEY_DSA,    TM_SSL_KEXCH_DHE,
     SADB_EALG_AESCBC,   SADB_AALG_SHA1HMAC, 16,    0},
/*   CipherSuite TLS_DHE_RSA_WITH_AES_128_CBC_SHA  = { 0x00, 0x33 };*/
    {0x0033,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_DHE,
     SADB_EALG_AESCBC,   SADB_AALG_SHA1HMAC, 16,    0},
/*   CipherSuite TLS_RSA_WITH_AES_256_CBC_SHA      = { 0x00, 0x35 };*/
    {0x0035,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_RSA,
     SADB_EALG_AESCBC,   SADB_AALG_SHA1HMAC, 32,    0},
/*   CipherSuite TLS_DH_DSS_WITH_AES_256_CBC_SHA   = { 0x00, 0x36 };*/
    {0x0036,             SADB_PUBKEY_DSA,    TM_SSL_KEXCH_DH,
     SADB_EALG_AESCBC,   SADB_AALG_SHA1HMAC, 32,    0},
/*   CipherSuite TLS_DH_RSA_WITH_AES_256_CBC_SHA   = { 0x00, 0x37 };*/
    {0x0037,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_DH,
     SADB_EALG_AESCBC,   SADB_AALG_SHA1HMAC, 32,    0},
/*   CipherSuite TLS_DHE_DSS_WITH_AES_256_CBC_SHA  = { 0x00, 0x38 };*/
    {0x0038,             SADB_PUBKEY_DSA,    TM_SSL_KEXCH_DHE,
     SADB_EALG_AESCBC,   SADB_AALG_SHA1HMAC, 32,    0},
/*   CipherSuite TLS_DHE_RSA_WITH_AES_256_CBC_SHA  = { 0x00, 0x39 };*/
    {0x0039,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_DHE,
     SADB_EALG_AESCBC,   SADB_AALG_SHA1HMAC, 32,    0},
/*   CipherSuite TM_TLS_RSA_NULL_SHA256            = { 0x00,0x3B };*/
    {0x003B,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_RSA,
     SADB_EALG_NULL,     SADB_AALG_SHA256HMAC, 0,   TM_SSL_EXPORTABLE},
/*   CipherSuite TLS_RSA_WITH_AES_128_CBC_SHA256  = { 0x00, 0x3C };*/
    {0x003C,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_RSA,
     SADB_EALG_AESCBC,   SADB_AALG_SHA256HMAC, 16,  0},
/*   CipherSuite TLS_RSA_WITH_AES_256_CBC_SHA256  = { 0x00, 0x3D };*/
    {0x003D,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_RSA,
     SADB_EALG_AESCBC,   SADB_AALG_SHA256HMAC, 32,  0},
/*   CipherSuite TLS_DHE_DSS_WITH_AES_128_CBC_SHA256     = { 0x00,0x40 };*/
    {0x0040,             SADB_PUBKEY_DSA,    TM_SSL_KEXCH_DHE,
     SADB_EALG_AESCBC,   SADB_AALG_SHA256HMAC, 16,  0},
/*   CipherSuite TLS_RSA_EXPORT1024_WITH_DES_CBC_SHA     = { 0x00,0x62 };*/
    {0x0062,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_RSAEXPORT1024,
     SADB_EALG_DESCBC,   SADB_AALG_SHA1HMAC, 8,     8},
/*   CipherSuite TLS_DHE_DSS_EXPORT1024_WITH_DES_CBC_SHA = { 0x00,0x63 };*/
    {0x0063,             SADB_PUBKEY_DSA,    TM_SSL_KEXCH_DHEEXPORT1024,
     SADB_EALG_DESCBC,   SADB_AALG_SHA1HMAC, 8,     8},
/*   CipherSuite TLS_RSA_EXPORT1024_WITH_RC4_56_SHA      = { 0x00,0x64 };*/
    {0x0064,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_RSAEXPORT1024,
     SADB_EALG_ARCFOUR,  SADB_AALG_SHA1HMAC, 7,     16},
/*   CipherSuite TLS_DHE_DSS_EXPORT1024_WITH_RC4_56_SHA  = { 0x00,0x65 };*/
    {0x0065,             SADB_PUBKEY_DSA,    TM_SSL_KEXCH_DHEEXPORT1024,
     SADB_EALG_ARCFOUR,  SADB_AALG_SHA1HMAC, 7,     16},
/*   CipherSuite TLS_DHE_DSS_WITH_RC4_128_SHA            = { 0x00,0x66 };*/
    {0x0066,             SADB_PUBKEY_DSA,    TM_SSL_KEXCH_DHE,
     SADB_EALG_ARCFOUR,  SADB_AALG_SHA1HMAC, 16,    0},
/*   CipherSuite TLS_DHE_RSA_WITH_AES_128_CBC_SHA256     = { 0x00,0x67 };*/
    {0x0067,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_DHE,
     SADB_EALG_AESCBC,   SADB_AALG_SHA256HMAC, 16,  0},
/*   CipherSuite TLS_DHE_DSS_WITH_AES_256_CBC_SHA256     = { 0x00,0x6A };*/
    {0x006A,             SADB_PUBKEY_DSA,    TM_SSL_KEXCH_DHE,
     SADB_EALG_AESCBC,   SADB_AALG_SHA256HMAC, 32,  0},
/*   CipherSuite TLS_DHE_RSA_WITH_AES_256_CBC_SHA256     = { 0x00,0x6B };*/
    {0x006B,             SADB_PUBKEY_RSA,    TM_SSL_KEXCH_DHE,
     SADB_EALG_AESCBC,   SADB_AALG_SHA256HMAC, 32,  0}
};

#ifdef TM_USE_SSL_VERSION_33
/* Signature of TLS1.2 support DER encoding */
/* MD5 */
/* (0x)30 20 30 0c 06 08 2a 86 48 86 f7 0d 02 05 05 00 04 10 || H */
#define TM_MD5_DIGESTINFO_LEN       18
static const tt8Bit tlMd5DigestInfo[] = {
    0x30, 0x20, 0x30, 0x0c, 0x06, 0x08, 0x2a, 0x86,
    0x48, 0x86, 0xf7, 0x0d, 0x02, 0x05, 0x05, 0x00,
    0x04, 0x10
};
static const tt8Bit tlMd5DigestInfoNull[] = {
    0x30, 0x1E, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86,
    0x48, 0x86, 0xf7, 0x0d, 0x02, 0x05, 0x04, 0x10
};
/* SHA-1 */
/* (0x)30 21 30 09 06 05 2b 0e 03 02 1a 05 00 04 14 || H */
#define TM_SHA1_DIGESTINFO_LEN      15
static const tt8Bit tlSha1DigestInfo[] = {
    0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2B, 0x0E,
    0x03, 0x02, 0x1A, 0x05, 0x00, 0x04, 0x14
};
static const tt8Bit tlSha1DigestInfoNull[] = {
    0x30, 0x1F, 0x30, 0x07, 0x06, 0x05, 0x2B, 0x0E,
    0x03, 0x02, 0x1A, 0x04, 0x14
};
/* SHA-256 */
/* (0x)30 31 30 0d 06 09 60 86 48 01 65 03 04 02 01 05 00 04 20 || H */
#define TM_SHA256_DIGESTINFO_LEN    19
static const tt8Bit tlSha256DigestInfo[] = {
    0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86,
    0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05,
    0x00, 0x04, 0x20
};
static const tt8Bit tlSha256DigestInfoNull[] = {
    0x30, 0x2F, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86,
    0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x04,
    0x20
};
#endif /* TM_USE_SSL_VERSION_33 */

#ifdef TM_USE_SSL_VERSION_33

/* MD5:  (0x)30 20 30 0c 06 08 2a 86 48 86 f7 0d 02 05 05 00 04 10 || H. */
#define TM_SSL_MD5WITHRSA_ENCODED_LEN (18 + TM_MD5_HASH_SIZE)

static const tt8Bit tlAsn1Md5AlgoId[] = {
    0x30, 0x0c, 0x06, 0x08, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x02, 0x05, 0x05, 0x00
};

/* SHA1: (0x)30 21 30 09 06 05 2b 0e 03 02 1a 05 00 04 14 || H */
#define TM_SSL_SHA1WITHRSA_ENCODED_LEN (15 + TM_SHA1_HASH_SIZE)

static const tt8Bit tlAsn1Sha1AlgoId[] = {
    0x30, 0x09, 0x06, 0x05, 0x2B, 0x0E, 0x03, 0x02, 0x1A, 0x05, 0x00
};

/* SHA2: (0x)30 31 30 0d 06 09 60 86 48 01 65 03 04 02 01 05 00 04 20 || H. */
#define TM_SSL_SHA256WITHRSA_ENCODED_LEN (19 + TM_SHA256_HASH_SIZE)

static const tt8Bit tlAsn1Sha256AlgoId[] = {
    0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00
};

#define TM_SSL_ASN1_MD5_ALGO_ID_LEN  14
#define TM_SSL_ASN1_SHA1_ALGO_ID_LEN 11
#define TM_SSL_ASN1_SHA2_ALGO_ID_LEN 15

#endif /* TM_USE_SSL_VERSION_33 */

/*
 * DESCIPTION
 * tfSslCipherSuiteLookup is called to get the cipher suite structure
 * ttSslCipherSuite pointer given the cipher suite number.
 *PARAMETER:
 * cipherNumber         The 16Bit cipher suite number
 *RETURN:
 * The const pointer to the cipher suite structure. If NULL pointer
 * is returned, we didn't find any thing matches the ciphernumber
 */
ttSslCipherSuiteConstPtr tfSslCipherSuiteLookup(tt16Bit   cipherNumber)
{
    ttSslCipherSuiteConstPtr   cipherPtr;
    unsigned int               i;

    cipherPtr = (ttSslCipherSuitePtr)0;
    if (cipherNumber <= 16)
    {
        cipherPtr = &tlSslCiphers[cipherNumber];
    }
    else
    {
        for (i = 17; i < sizeof(tlSslCiphers)/sizeof(ttSslCipherSuite); i++)
        {
            cipherPtr = &tlSslCiphers[i];
            if (cipherPtr->csNumber == cipherNumber)
            {
                break;
            }
            else
            {
                cipherPtr = (ttSslCipherSuiteConstPtr)0;
            }
        }
    }
    return cipherPtr;

}

/*DESCRIPTION:
 * tfSslSendCertificate will be called by the SSL client or SSL server
 * state machine when SSL is in the handshake mode. It constructs the
 * certificate message.
 * Parameter:
 * connStatePtr      The state pointer of SSL
 * paramPtr         The parameter of state machine, (not used here)
 * return:
 * TM_ENOERROR      No error
 * TM_ENOBUFS       No buffer available
 */
int tfSslSendCertificate(ttSslConnectStatePtr    connStatePtr,
                            ttVoidPtr            paramPtr)
{
    ttSslHandShakeHeaderPtr  handshakeHeaderPtr;
    ttPkiCertListPtr         certListPtr;
    tt8BitPtr                dataPtr;
    int                      errorCode;
#ifdef TM_USE_SSL_CLIENT
    ttSslRecordHeaderPtr     sslHeaderPtr;
    ttPacketPtr              packetPtr;
#endif /* TM_USE_SSL_CLIENT*/
    errorCode = TM_ENOERROR;
    TM_UNREF_IN_ARG(paramPtr);
    dataPtr = (tt8BitPtr)0;

#ifdef TM_USE_SSL_CLIENT
    if (connStatePtr->scsClientOrServer == TM_SSL_CLIENT)
    {
/* client is calling this*/
        packetPtr = tfGetSharedBuffer(0, connStatePtr->scsXmitBufferSize, 0);
        if (!packetPtr)
        {
            errorCode = TM_ENOBUFS;
            goto sendCertExit;
        }
        dataPtr = packetPtr->pktLinkDataPtr;
        connStatePtr->scsXmitPacketPtr = packetPtr;
        connStatePtr->scsXmitDataPtr = dataPtr;
        connStatePtr->scsXmitDataLength = 0;

        if (    (connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_CLIENT_SENDCERT)
#ifdef TM_SSL_CLIENT_RESPONSE_NO_CERT
             || (connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_CLIENT_SEND_NOCERT)
#endif /* TM_SSL_CLIENT_RESPONSE_NO_CERT */
           )
        {
/* Certificate seg is the first part of this record, we need construct
 * the SSL record header */
            sslHeaderPtr = (ttSslRecordHeaderPtr)dataPtr;
            sslHeaderPtr->srhType = TM_SSL_TYPE_HANDSHAKE;
            sslHeaderPtr->srhVersionMajor = TM_SSL_VERSION_MAJOR;
#ifdef TM_USE_SSL_VERSION_33
            if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
            {
                sslHeaderPtr->srhVersionMinor = TM_SSL_VERSION_MINOR3;
            }
            else
#endif /* TM_USE_SSL_VERSION_33 */
            {
                sslHeaderPtr->srhVersionMinor = (tt8Bit)
                          (connStatePtr->scsVersion >> 1);
            }
/* if we need certificate, our record size is unable to determined yet,
 * for example, the DSA signature length is undetermined. We set the record
 * size when we finish certificate verify
 */
            dataPtr += TM_SSL_RECORD_HEADER_SIZE;
            connStatePtr->scsXmitDataLength = (tt16Bit)(
                       connStatePtr->scsXmitDataLength +
                       TM_SSL_RECORD_HEADER_SIZE);
        }
        else
        {
            goto sendCertExit;
        }
    }
#endif /* TM_USE_SSL_CLIENT */

#ifdef TM_USE_SSL_SERVER
    if (connStatePtr->scsClientOrServer == TM_SSL_SERVER)
    {
        dataPtr = connStatePtr->scsXmitDataPtr;
    }
#endif /* TM_USE_SSL_SERVER */

/* now construct the certificate segment, dataPtr already points to the
 * right position
 */
    handshakeHeaderPtr = (ttSslHandShakeHeaderPtr)dataPtr;
    handshakeHeaderPtr->hsHandShakeType = TM_SSL_CERTIFICATE;
    handshakeHeaderPtr->hsLengthPad = 0;
    handshakeHeaderPtr->hsLengthHigh =
           (tt8Bit)(connStatePtr->scsTotalCertBytes >> 8);
    handshakeHeaderPtr->hsLengthLow =
           (tt8Bit)(connStatePtr->scsTotalCertBytes & 0xff);

    dataPtr += TM_SSL_HANDSHAKE_HEADER_SIZE;
/* write the total cert size again */
    *dataPtr ++ = 0;
    *dataPtr ++ = (tt8Bit)((connStatePtr->
                            scsTotalCertBytes - 3) >> 8);
    *dataPtr ++ = (tt8Bit)((connStatePtr->scsTotalCertBytes - 3) & 0xff);

#ifdef TM_USE_SSL_VERSION_33
    if ((connStatePtr->scsVersion == TM_SSL_VERSION_33)
      &&(connStatePtr->scsClientOrServer == TM_SSL_CLIENT))
    {
        certListPtr = connStatePtr->scsSessionPtr->ssCertSuppPtr;
    }
    else
#endif /* TM_USE_SSL_VERSION_33 */
    {
/* the first cert */
        certListPtr = connStatePtr->scsSessionPtr->ssCertListPtr;
    }
    while (certListPtr )
    {
        *dataPtr ++ = 0;
        *dataPtr ++ = (tt8Bit)(certListPtr->certLength >> 8);
        *dataPtr ++ = (tt8Bit)(certListPtr->certLength & 0xff);
        tm_bcopy(certListPtr->certContentPtr,
                 dataPtr,
                 certListPtr->certLength);
        dataPtr += certListPtr->certLength;
        certListPtr = certListPtr->certSslNextPtr;
    }

    connStatePtr->scsXmitDataPtr = (tt8BitPtr)dataPtr;
    connStatePtr->scsXmitDataLength = (tt16Bit)(connStatePtr->scsXmitDataLength
                       + TM_SSL_HANDSHAKE_HEADER_SIZE
                       + connStatePtr->scsTotalCertBytes);

/* hash cert(Header + Data) */
#ifdef TM_USE_SSL_VERSION_33
    if (connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
        tfSha256Update(connStatePtr->scsSha256CtxPtr,
                       (tt8BitPtr)handshakeHeaderPtr,
                       (tt16Bit)(TM_SSL_HANDSHAKE_HEADER_SIZE
                       + connStatePtr->scsTotalCertBytes),
                       0);
    }
#endif /* TM_USE_SSL_VERSION_33 */
    tfSslUpdateMd5Sha1Ctx(connStatePtr,
                          (tt8BitPtr)handshakeHeaderPtr,
                          (tt16Bit)(TM_SSL_HANDSHAKE_HEADER_SIZE
                          + connStatePtr->scsTotalCertBytes));

#ifdef TM_USE_SSL_CLIENT
sendCertExit:
#endif /*  TM_USE_SSL_CLIENT */

    return errorCode;

}

/*DESCRIPTION:
 * tfSslGetSessionUsingIndex is called to get the session pointer
 * given the session index.
 * Parameter:
 * sessionId    The session id used
 * return:
 * NULL pointer     There is no session pointer given that number
 * non-NULL pointer successful return
 */

ttVoidPtr  tfSslGetSessionUsingIndex(int        sessionId)
{
    ttSslSessionPtr      sslSessionPtr;

    sslSessionPtr = (ttSslSessionPtr)0;
    tm_call_lock_wait(&tm_context(tvSslTlsLockEntry));
    if (tm_context(tvSslTlsPtr) &&
        sessionId >= 0 &&
        (tt16Bit)sessionId < ((ttSslGlobalEntryPtr)tm_context(tvSslTlsPtr))
        ->sgMaxSession)
    {
        sslSessionPtr = (((ttSslGlobalEntryPtr)
                          tm_context(tvSslTlsPtr))
                         ->sgSessionArray[sessionId]);
        if(!sslSessionPtr)
        {
            goto getSessionExit;
        }
        tm_call_lock_wait(&sslSessionPtr->ssLockEntry);
        sslSessionPtr->ssOwnerCount++;
    }
getSessionExit:
    tm_call_unlock(&tm_context(tvSslTlsLockEntry));
    return (ttVoidPtr)sslSessionPtr;
}

void  tfSslSessionReturn(ttSslSessionPtr sslSessionPtr)
{
    tt16Bit              ownerCount;

    sslSessionPtr->ssOwnerCount--;
    ownerCount = sslSessionPtr->ssOwnerCount;
    tm_call_unlock(&sslSessionPtr->ssLockEntry);
    if (ownerCount == TM_16BIT_ZERO)
    {
        tm_free_raw_buffer(sslSessionPtr);
    }
    return;
}

/* DESCRIPTION:
 * tfSslParseCertificate is called by state machine in order to parse an
 * incoming certificate message.
 * PARAMETER
 * connStatePtr          SSL state pointer
 * dataPtr              The beginning of certificate message header
 * msgSize              The length of certificate message
 * RETURN:
 * TM_ENOERROR          successful return
 * TM_ENOBUFS           No buffer available
 * TM_EINVAL            Invalid message
 */
int tfSslParseCertificate(ttSslConnectStatePtr    connStatePtr,
                          tt8BitPtr               dataPtr,
                          tt16Bit                 msgSize)
{
/* we just use any NULL idPtr to denote the cert chain we may receive.
 * later, we are going to delete them all if SSL connection is
 * established.
 */
    ttSslCertCallbackFuncPtr  certCBPtr;
    ttCertificateEntryPtr     certEntryPtr;
    tt16Bit                   thisCertLen[TM_SSL_CERT_CHAIN_MAX];
    tt8BitPtr                 certContentPtr[TM_SSL_CERT_CHAIN_MAX];
    ttPkiCertListPtr          certListPtr[TM_SSL_CERT_CHAIN_MAX];
    char TM_FAR *             serverName;
    int                       i;
    int                       lastCertIndex;
    int                       errorCode;
    tt16Bit                   totalCertLen;
    tt8Bit                    criticalErrorFound;
    tt8Bit                    nonCriticalErrorFound;
/* we can maximully accept TM_SSL_CERT_CHAIN_MAX(8) level certificates,
 * which is very rare to reach
 */
    tm_trace_func_entry(tfSslParseCertificate);
    tm_debug_log1("tfSslParseCertificate sd=%u",
                  connStatePtr->scsTcpVectPtr->tcpsSocketEntry.socIndex);
#ifdef TM_LOG_SSL_DEBUG
    tfEnhancedLogWrite(
            TM_LOG_MODULE_SSL,
            TM_LOG_LEVEL_DEBUG,
            "tfSslParseCertificate: sd=%u",
            connStatePtr->scsTcpVectPtr->tcpsSocketEntry.socIndex);
#endif /* TM_LOG_SSL_DEBUG */

    i = 0;
    criticalErrorFound = TM_8BIT_NO;
    nonCriticalErrorFound = TM_8BIT_NO;
    TM_UNREF_IN_ARG(msgSize);
    serverName = (char TM_FAR *)0;
    certEntryPtr = (ttCertificateEntryPtr)0;
    tm_bzero(certContentPtr, sizeof(certContentPtr));
    tm_bzero(certListPtr, sizeof(certListPtr));
    totalCertLen = (tt16Bit)(((*(dataPtr + 1)) << 8) | (*(dataPtr + 2)));
    dataPtr += TM_SSL_CERT_LENFIELD_LEN;/* skip the total cert length field*/
    tm_bzero(&certContentPtr[0],TM_SSL_CERT_CHAIN_MAX * sizeof(tt8BitPtr));
    lastCertIndex = (int)0;

/* Make sure we've got a valid length */
    if (!totalCertLen)
    {
        errorCode = TM_EINVAL;
        goto parseCertExit;
    }

/* the certificate comes in sequence EndUser-> 1st upper level RA ->
 * 2nd upper level RA -------> root CA */
    while (totalCertLen)
    {
        thisCertLen[i] = (tt16Bit)(((*(dataPtr + 1)) << 8) |
                                    (*(dataPtr + 2)));
        certContentPtr[i] = tm_get_raw_buffer(thisCertLen[i]);
        if (!certContentPtr[i])
        {
            lastCertIndex = i;
            errorCode = TM_ENOBUFS;
            goto parseCertExit;
        }
        dataPtr += TM_SSL_CERT_LENFIELD_LEN;
        tm_bcopy(dataPtr, certContentPtr[i], thisCertLen[i]);
        dataPtr += thisCertLen[i];
        totalCertLen = (tt16Bit)(totalCertLen -
                                 thisCertLen[i] -
                                 TM_SSL_CERT_LENFIELD_LEN);
        lastCertIndex = i; /* allocated up to index i */
        i++;
        if (i >= TM_SSL_CERT_CHAIN_MAX)
        {
/* we don't support that many certificates, free previous ones */
            lastCertIndex = i - 1;
            errorCode = TM_EOPNOTSUPP;
            goto parseCertExit;
        }
    }

    certEntryPtr = (ttCertificateEntryPtr)
                  tm_get_raw_buffer((ttPktLen)i * sizeof(ttCertificateEntry));
    if(certEntryPtr == (ttCertificateEntryPtr)0)
    {
        errorCode = TM_ENOBUFS;
        goto parseCertExit;
    }

    for (i = lastCertIndex; i >= 0; i--)
/* we add the certificates from the reverse way, so that when we add each
 * certificate, their RA(CA)'s certificate has already been added before
 */
    {
#ifdef TM_USE_SSL_CLIENT
        if (i == 0)
        {
/* The cert chain has been added successfully, we need check if the end
 * entity's certificate has the common name we are connecting to
 */
            serverName = connStatePtr->scsServerCNPtr;
        }
#endif /* TM_USE_SSL_CLIENT */
        errorCode = tfPkiCertificateCheckAdd(
                         &certListPtr[i],
                         serverName,
                         certContentPtr[i],
                         TM_PKI_CERTIFICATE_STRING|TM_PKI_CERTIFICATE_DER,
                         TM_PKI_CERT_NONLOCAL,
                         (ttUserVoidPtr)0,/* this cert's ID*/
                         0, /* this cert's id length */
                         (ttUserVoidPtr)0);/*ca's ID */

        if(errorCode & TM_CERTFLAG_BADCERT)
        {
/* to indicate that a critical error happens */
            criticalErrorFound = TM_8BIT_YES;
        }
        else if(errorCode != TM_ENOERROR)
        {
/* to indicate that a non-critical error happens. Non-critical error means an
 * error that can be ignored by user's callback function
 */
            nonCriticalErrorFound = TM_8BIT_YES;
        }
        if (certListPtr[i] != (ttPkiCertListPtr)0)
        {
/*
 * If TM_CERTFLAG_BADCERT is set tfPkiCertificateCheckAdd()
 * set certListPtr[i] to NULL !!
 */
            certListPtr[i]->certCheckOut++;
        }
        certEntryPtr[i].certDataPtr = (const char TM_FAR *)(certContentPtr[i]);
        certEntryPtr[i].certLength = thisCertLen[i];
        certEntryPtr[i].certFlags = errorCode;
    }

/* we finish processing all certificates, if user registers certificate
 * call back function, we let user decide what to do next. (for
 * critical failures, no matter what user decides, we are going to quit
 */
    certCBPtr = connStatePtr->scsSessionPtr->ssCertCBFuncPtr;
    if(certCBPtr)
    {
        errorCode = (*certCBPtr)(connStatePtr->scsSessionPtr->ssNumber,
                                 (int)connStatePtr->scsSocIndex,
                                 certEntryPtr,
                                 lastCertIndex + 1);
    }
    else
    {
/* if no call back founction is registered, any non-critical error
 * will abort the SSL negotiation
 */
        errorCode = (int)nonCriticalErrorFound;
    }
/* link these certificates to connStatePtr.
 * (notice that we already checked them out
 */
    i = 0;
    if (certListPtr[0] != (ttPkiCertListPtr)0)
    {
        tfSslCleanupPeerCerts(connStatePtr);
        connStatePtr->scsPeerCertListPtr = certListPtr[0];
    }
    else
    {
        for (i = 0; i <= lastCertIndex; i ++)
        {
            if (certListPtr[i] != (ttPkiCertListPtr)0)
            {
                tfSslCleanupPeerCerts(connStatePtr);
                connStatePtr->scsPeerCertListPtr = certListPtr[i];
                break;
            }
        }
    }

    if (certListPtr[i] != (ttPkiCertListPtr)0)
    {
#ifdef TM_PUBKEY_USE_RSA
        if (connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_SERVER_SENDERSA)
        {
/* we suspect that the server should send us ERSA key exchange.
 * we need to verify the certificate public key length, if it is
 * longer than 512 bits, our suspection is true. Otherwise, server
 * does not have to send us ephemeral rsa keys.
 */
            if (((ttGeneralRsaPtr)certListPtr[i]->certKeyPtr
                           ->pkKeyPtr)->rsaN->asn1Length <= (512 >> 3))
            {
                connStatePtr->scsRunFlags &=
                                ~(unsigned)TM_SSL_RUNFLAG_SERVER_SENDERSA;
            }
        }
#endif /* TM_PUBKEY_USE_RSA */

        for (; i < lastCertIndex; i ++)
        {
            certListPtr[i]->certSslNextPtr = certListPtr[i+1];
        }
    }
    if((criticalErrorFound == TM_8BIT_YES) || /* critical error found */
        (errorCode != TM_ENOERROR)) /* user suggested to reject */
    {
        errorCode = TM_EOPNOTSUPP;
/* send fatal bad_certificate message */
        (void)tfSslBuildSendAlert(connStatePtr,
                                  TM_SSL_ALTLEV_FATAL,
                                  TM_SSL_ALT_BADCERT);

    }
    else
    {
        errorCode = TM_ENOERROR;
    }

parseCertExit:
/*    if (errorCode != TM_ENOERROR) *//* BUG ID3287:unmerged */
    {
        for (i = lastCertIndex; i >= 0; i--)
        {
            if (certContentPtr[i] != 0)
            {
                tm_free_raw_buffer(certContentPtr[i]);
            }
        }
    }
    if(certEntryPtr != (ttCertificateEntryPtr)0)
    {
        tm_free_raw_buffer(certEntryPtr);
    }

    tm_trace_func_exit_rc(tfSslParseCertificate, errorCode);
    return errorCode;
}


/* tfSslRegisterCertificateCB
 * Description
 * This function is provided so that users have access to the client/server
 * certificates during SSL session negotiation. If no callback function is
 * registered, the stack itself determines how to treat a certificate, for
 * example, to deny any certificate without known CA, or to deny any
 * certificate without valid lifetime. If callback function is registered,
 * all certificates will be passed to user in a ttCertificateEntry array,
 * and user determines how to handle the certificate. Note that, if the
 * certificate is determined to be a bad one (bad_certificate), user's
 * direction will be ignored by the stack.
 *
 * Parameters
 *
 * sessionId       the session Id which the callback function will bind to
 * certCBFuncPtr   callback function pointer, of type ttSslCertCallbackFuncPtr
 */

int tfSslRegisterCertificateCB(
                          int                            sessionId,
                          ttSslCertCallbackFuncPtr       certCBFuncPtr)
{
    ttSslSessionPtr     sslSessionPtr;
    int                 errorCode;


    errorCode = TM_ENOERROR;
    sslSessionPtr = (ttSslSessionPtr)tfSslGetSessionUsingIndex(sessionId);
    if (!sslSessionPtr)
    {
        errorCode = TM_EINVAL;
        goto sslRegisterCertCBExit;
    }

    sslSessionPtr->ssCertCBFuncPtr = certCBFuncPtr;
    tfSslSessionReturn(sslSessionPtr);
sslRegisterCertCBExit:
    return errorCode;
}





/*
 * DESCIPTION
 * tfSslUpdateRecordAfterMac is called after we finish the MAC computation,
 * before the MAC computation, the SSL record header has the length value
 * equal to original record data length. After MAC computation, the SSL
 * record should be properly padded, and the length field should contain
 * the MAC and PADDING length. Call this function for this purpose.
 *PARAMETER:
 * connStatePtr         sslState
 * sslHeaderPtr         The SSL record header
 * macPaddingLen        The length of the padding used in the incoming
 *                      packet (for decryption) or 0 (for encryption)
 * hashOutPtr           The pointer pointing to the hashing result
 *RETURN:
 * none
 */
static void tfSslUpdateRecordAfterMac(ttSslConnectStatePtr  connStatePtr,
                                      ttSslRecordHeaderPtr  sslHeaderPtr,
                                      tt8Bit                macPaddingLen,
                                      tt8BitPtr             hashOutPtr)
{
    int                     i;
    tt16Bit                 sslDataLength;
    tt8Bit                  blockSize;
    tt8Bit                  hashSize;
    tt8Bit                  paddingLen;

    blockSize = connStatePtr->scsBlockSize;
    hashSize = connStatePtr->scsHashOutputSize;
    sslDataLength = (tt16Bit)((sslHeaderPtr->srhLengthHigh << 8) |
                               sslHeaderPtr->srhLengthLow);
#ifdef TM_USE_SSL_VERSION_33
    if((connStatePtr->scsVersion == TM_SSL_VERSION_33)
     &&((connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_ARCFOUR)
      &&(connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_NULL)))
    {
        sslDataLength = sslDataLength + (tt16Bit)
            (connStatePtr->scsCryptoAlgPtr->ealgBlockSize);
    }
#endif /* TM_USE_SSL_VERSION_33 */
    sslDataLength = (tt16Bit)(sslDataLength + hashSize);
    if (blockSize > 1) /* not a stream cipher */
    {
        if (macPaddingLen != 0)
        {
            paddingLen = macPaddingLen;
        }
        else
        {
            paddingLen = (tt8Bit)((blockSize -
                             ((sslDataLength + 1) % blockSize))
                                % blockSize);
        }

        for (i = 0; i < paddingLen + 1; i ++)
        {
            hashOutPtr[hashSize + i] = paddingLen;
        }
/*1 is padding length field*/
        sslDataLength = (tt16Bit)(sslDataLength + 1 + paddingLen);
    }

    sslHeaderPtr->srhLengthHigh = (tt8Bit)(sslDataLength >> 8);
    sslHeaderPtr->srhLengthLow = (tt8Bit)(sslDataLength & 0xff);

    return;
}



#if (defined(TM_USE_SSL_VERSION_31) || defined(TM_USE_SSL_VERSION_33))

/* The following procedure is used to get MAC out of a record
 * using TLS version:
 *     HMAC_hash(MAC_write_secret, seq_num +
 *              TLSCompressed.type + TLSCompressed.version +
 *              TLSCompressed.length + TLSCompressed.fragment));
 *
 * The md5hmac_hash (16 bytes) and sha1hmac_hash(20bytes) will be write to
 * memory location provided by macOutPtr. Padding will be taken care of
 * before return. And We are responsible to update the  sslHeaderPtr to
 * have the length value = originalLength + hashing + paddingLength + 1.
 * sslRecordDataPtr must points to the record data. (NOT THE record
 * header.)
 * sslDataLength: the value should be exactly the same as that in
 *                sslHeaderPtr at this moment.
 * direction: Can be either TM_SSL_ENCODE or TM_SSL_DECODE
 * macOutPtr: Where to write the mac output. (After the MAC, we will
 *            do proper padding and then update the SSLHEADER
 */

int tfTlsGetRecordMac(ttSslConnectStatePtr         connStatePtr,
                      ttSslRecordHeaderPtr         sslHeaderPtr,
/* the head packet should be connStatePtr->scsWritePktPtr*/
                      ttPacketPtr                  headPktPtr,
                      ttPktLen                     offset,
                      ttPktLen                     dataLength,
                      tt8Bit                       direction,
                      tt8BitPtr                    macOutPtr)
{

    ttGenericKeyPtr      gkeyPtr;
    tt8BitPtr            extraDataPtr;
    int                  errorCode;

    errorCode = TM_ENOBUFS;
/*1. firstly, build the extra dataPtr */
    extraDataPtr = tm_get_raw_buffer(TM_SSL_RECORD_HEADER_SIZE +
                                     2* sizeof(tt32Bit));
    if (!extraDataPtr)
    {
        errorCode = TM_ENOBUFS;
        goto tlsGetRecordMacExit;
    }

    if (direction == TM_SSL_ENCODE)
    {
        gkeyPtr = &connStatePtr->scsEncodeMacKey;
        *((tt32BitPtr)extraDataPtr) =
            htonl(connStatePtr->scsOutSeqNumberHigh);
        *((tt32BitPtr)extraDataPtr + 1) =
            htonl(connStatePtr->scsOutSeqNumber);
    }
    else
    {
        gkeyPtr = &connStatePtr->scsDecodeMacKey;
        *((tt32BitPtr)extraDataPtr) =
            htonl(connStatePtr->scsInSeqNumberHigh);
        *((tt32BitPtr)extraDataPtr + 1) =
            htonl(connStatePtr->scsInSeqNumber);
    }

    tm_bcopy(sslHeaderPtr, extraDataPtr + 2* sizeof(tt32Bit),
              TM_SSL_RECORD_HEADER_SIZE);

    errorCode = tfAhMac(headPktPtr,
                         (int)offset,
                         extraDataPtr,
                         TM_SSL_RECORD_HEADER_SIZE + 2 * sizeof(tt32Bit),
                         dataLength,
                         macOutPtr,
                         gkeyPtr,
                         connStatePtr->scsHashAlgPtr,
                         TM_8BIT_NO,
                         TM_8BIT_YES);
    if (errorCode != TM_ENOERROR)
    {
        goto tlsGetRecordMacExit;
    }

tlsGetRecordMacExit:
    if (extraDataPtr)
    {
        tm_free_raw_buffer(extraDataPtr);
    }
    return errorCode;
}

/*
 * tfTlsPhash is used for TLS version only. The formula is:
 * P_hash(secret, seed) = HMAC_hash(secret, A(1) + seed) +
 *                        HMAC_hash(secret, A(2) + seed) +
 *                        HMAC_hash(secret, A(3) + seed) + ...
 *
 *   Where + indicates concatenation.
 *
 *   A() is defined as:
 *       A(0) = seed
 *       A(i) = HMAC_hash(secret, A(i-1))
 *
 * We use seed1 and seed2 because sometimes we may use this
 * same formular for two concatted seeds
 * PARAMETERS:
 * connStatePtr                      SSL state
 * algorithm                        md5 or sha1 or sha256
 * secretPtr, secretLength          parameter for secret
 * seed1Ptr, seed1Length
 * seed2Ptr seed2Length             parameter for seeds
 * outputPtr                        pointer to output
 * rounds                           How many rounds to compute
 */

 static int tfTlsPhash(ttSslConnectStatePtr connStatePtr,
                       tt8Bit               algorithm,
                       tt8BitPtr            secretPtr,
                       int                  secretLength,
                       tt8BitPtr            seed1Ptr,
                       int                  seed1Length,
                       tt8BitPtr            seed2Ptr, /* could be NULL */
                       int                  seed2Length,
                       tt8BitPtr            outputPtr,
                       int                  rounds)
 {
     ttAhAlgorithmPtr          aalgPtr;
     tt8BitPtr                 aiOutPtr;
     tt8BitPtr                 dataPtr;
/* if secretLength is longer than 64 bytes, hash it first */
     tt8BitPtr                 newSecretPtr;
     tt8BitPtr                 keyPadPtr;
     ttVoidPtr                 contextPtr;
     ttCryptoChainData         chainAi;
     ttCryptoChainData         chainSeed1;
     ttCryptoChainData         chainSeed2;
     ttCryptoRequest           request;
     ttCryptoRequestHmacHash   hhashRequest;
     int                       hashOutSize;
     int                       errorCode;
     int                       i;


     aiOutPtr = (tt8BitPtr)0;
     hashOutSize = 0;
     errorCode = TM_ENOERROR;
     newSecretPtr = (tt8BitPtr)0;
/* the hifn board requires that any HMAC hashing inputs 64 byte key*/
     keyPadPtr = (tt8BitPtr)0;
     tm_bzero(&request, sizeof(request));
     aalgPtr = tfAhAlgorithmLookup(algorithm);
     if (!aalgPtr)
     {
         errorCode = TM_EINVAL;
         goto tlsPhashExit;
     }

     if (algorithm == SADB_AALG_MD5HMAC)
     {
         hashOutSize = TM_MD5_HASH_SIZE;
     }
     else if (algorithm == SADB_AALG_SHA1HMAC)
     {
         hashOutSize = TM_SHA1_HASH_SIZE;
     }
     else if (algorithm == SADB_AALG_SHA256HMAC)
     {
         hashOutSize = TM_SHA256_HASH_SIZE;
     }
     aiOutPtr = tm_get_raw_buffer((ttPktLen)hashOutSize);
     if (!aiOutPtr)
     {
         errorCode = TM_ENOBUFS;
         goto tlsPhashExit;
     }

     if (secretLength > TM_HMAC_BLOCK_SIZE)
     {
         newSecretPtr = tm_get_raw_buffer((ttPktLen)hashOutSize);
         if (!newSecretPtr)
         {
             errorCode = TM_ENOBUFS;
             goto tlsPhashExit;
         }
/* use this new secretPtr*/
         contextPtr = tm_get_raw_buffer((ttPktLen)
                                        (aalgPtr->aalgContextLenFuncPtr()));
         if (!contextPtr)
         {
             errorCode = TM_ENOBUFS;
             goto tlsPhashExit;
         }
         aalgPtr->aalgInitFuncPtr(contextPtr);
         aalgPtr->aalgUpdateFuncPtr(contextPtr, secretPtr, (ttPktLen)secretLength,0);
         aalgPtr->aalgFinalFuncPtr(newSecretPtr, contextPtr);
         tm_free_raw_buffer(contextPtr);
         secretPtr = newSecretPtr;
         secretLength = hashOutSize;
     }
     keyPadPtr =  tm_get_raw_buffer(TM_HMAC_BLOCK_SIZE);
     if (!keyPadPtr)
     {
         errorCode = TM_ENOBUFS;
         goto tlsPhashExit;
     }
     tm_bzero(keyPadPtr, TM_HMAC_BLOCK_SIZE);
     tm_bcopy(secretPtr, keyPadPtr, secretLength);
/* since seed1 is always followed by seed 2. We make them append */
     chainSeed1.ccdDataLen = (ttPktLen)seed1Length;
     chainSeed1.ccdDataPtr = seed1Ptr;
     if (seed2Ptr && seed2Length)
     {
         chainSeed1.ccdNextPtr = &chainSeed2;
         chainSeed2.ccdNextPtr = 0;
         chainSeed2.ccdDataLen = (ttPktLen)seed2Length;
         chainSeed2.ccdDataPtr = seed2Ptr;
     }
     else
     {
         chainSeed1.ccdNextPtr = 0;
     }

/* we know the chainAi */
     chainAi.ccdDataLen = (ttPktLen)hashOutSize;
     chainAi.ccdDataPtr = aiOutPtr;
     request.crType = TM_CEREQUEST_CRYPTO_HMACHASH;
     request.crParamUnion.crHmacHashParamPtr = &hhashRequest;
     hhashRequest.crhhAlgPtr = aalgPtr;
     hhashRequest.crhhHashAlg = algorithm;
     hhashRequest.crhhKeyLen = TM_HMAC_BLOCK_SIZE;
     hhashRequest.crhhKeyPtr = keyPadPtr;

     for (i = 0; i < rounds; i ++)
     {
/* compute A(i) */
         hhashRequest.crhhOutPtr = aiOutPtr;
         if (i == 0)
         {
             hhashRequest.crhhInDataPtr = &chainSeed1;
             hhashRequest.crhhDataLength = (ttPktLen)(seed1Length + seed2Length);
         }
         else
         {
             chainAi.ccdNextPtr = 0;
             hhashRequest.crhhInDataPtr = &chainAi;
             hhashRequest.crhhDataLength = (ttPktLen)hashOutSize;

         }
         errorCode = connStatePtr->scsCryptoEnginePtr->ceSessionProcessFuncPtr
                      (&request);
         if (errorCode != TM_ENOERROR)
         {
             goto tlsPhashExit;
         }
/* compute Master_Secret(i) */
         dataPtr = outputPtr + (hashOutSize * i);
         chainAi.ccdNextPtr = &chainSeed1;
         hhashRequest.crhhInDataPtr = &chainAi;
         hhashRequest.crhhOutPtr = dataPtr;
         hhashRequest.crhhDataLength = (ttPktLen)
                                (hashOutSize + seed1Length + seed2Length);
         errorCode = connStatePtr->scsCryptoEnginePtr->ceSessionProcessFuncPtr
                      (&request);
         if (errorCode != TM_ENOERROR)
         {
             goto tlsPhashExit;
         }
     }

tlsPhashExit:
     if (aiOutPtr)
     {
         tm_free_raw_buffer(aiOutPtr);
     }
     if (newSecretPtr)
     {
         tm_free_raw_buffer(newSecretPtr);
     }
     if (keyPadPtr)
     {
         tm_free_raw_buffer(keyPadPtr);
     }
     return errorCode;
 }

#ifdef TM_USE_SSL_VERSION_31
/* For TLS 1.0 used only, in order to compute PRF():
 *       L_S = length in bytes of secret;
 *       L_S1 = L_S2 = ceil(L_S / 2);
 *
 *   The secret is partitioned into two halves (with the possibility of
 *   one shared byte) as described above, S1 taking the first L_S1 bytes
 *   and S2 the last L_S2 bytes.
 *
 *   The PRF is then defined as the result of mixing the two pseudorandom
 *   streams by exclusive-or'ing them together.
 *
 *       PRF(secret, label, seed) = P_MD5(S1, label + seed) XOR
 *                                  P_SHA-1(S2, label + seed);
 * PARAMETER:
 * connStatePtr                      SSL state
 * secretPtr, secretLength          parameter for secret
 * labelPtr, labelLength            The label for computation, will be seed1
 *                                  when we call tfTlsPhash
 * seed2Ptr seed2Length             The seed for computation, will be seed2
 *                                  when we call tfTlsPhash
 * outputPtr                        pointer to output
 * outputLength                     How many bytes output needed
 */


int tfTlsPrf(ttSslConnectStatePtr  connStatePtr,
             tt8BitPtr             secretPtr,
             int                   secretLength,
             char TM_FAR *         labelPtr,
             int                   labelLength,
             tt8BitPtr             seedPtr,
             int                   seedLength,
             tt8BitPtr             outputPtr,
             int                   outputLength)
{
    tt8BitPtr         md5OutPtr;
    tt8BitPtr         sha1OutPtr;
    tt8BitPtr         s1Ptr;
    tt8BitPtr         s2Ptr;
    int               lengthS1; /* lengthS2 = lengthS1*/
    int               i;
    int               errorCode;
    tt8Bit            md5Rounds;
    tt8Bit            sha1Rounds;



    errorCode = TM_ENOERROR;
    md5OutPtr = (tt8BitPtr)0;
    sha1OutPtr = (tt8BitPtr)0;
/*1. caculate the md5Rounds and sha1 rounds */
    md5Rounds = (tt8Bit)(outputLength / TM_MD5_HASH_SIZE);
    if (outputLength % TM_MD5_HASH_SIZE)
    {
        md5Rounds = (tt8Bit)(md5Rounds + 1);
    }

    sha1Rounds = (tt8Bit)(outputLength / TM_SHA1_HASH_SIZE);
    if (outputLength % TM_SHA1_HASH_SIZE)
    {
        sha1Rounds = (tt8Bit)(sha1Rounds + 1);
    }

/*2. Get the hashing output buffer */
    md5OutPtr = tm_get_raw_buffer(md5Rounds * TM_MD5_HASH_SIZE);
    sha1OutPtr = tm_get_raw_buffer(sha1Rounds * TM_SHA1_HASH_SIZE);
    if (!md5OutPtr || !sha1OutPtr)
    {
        errorCode = TM_ENOBUFS;
        goto tlsPrfExit;
    }

/*3 Divide the secret to be s1 + s2, and set lengthS1, ls2 */
    s1Ptr = secretPtr;

#if (   defined(TM_USE_DIFFIEHELLMAN_PADDING) \
     && !(defined(TM_SSL_DH_HASH_WITH_PADDING)))
/*
 * If Diffie-Hellman padding is being used, we must detect the leading zeros
 * that were appended and strip them off before doing the hash.
 * If the secretLength == TM_SSL_MASTER_SECRET_SIZE, we should not strip the
 * leading zeros from the master secret.
 */
    if (secretLength != TM_SSL_MASTER_SECRET_SIZE)
    {
        while ((*s1Ptr == TM_8BIT_ZERO) && (secretLength > 0))
        {
            s1Ptr++;
            secretLength--;
        }
    }
#endif /* TM_USE_DIFFIEHELLMAN_PADDING && !TM_SSL_DH_HASH_WITH_PADDING */

    lengthS1 = secretLength >> 1;
    s2Ptr = s1Ptr + lengthS1;
    if (secretLength & 0x01)
    {
        lengthS1 ++;
    }

/*4. Call p_hash (secret, seed) */
    errorCode = tfTlsPhash(connStatePtr,
                           SADB_AALG_MD5HMAC,
                           s1Ptr,
                           lengthS1,
                           (tt8BitPtr)labelPtr,
                           labelLength,
                           seedPtr,
                           seedLength,
                           md5OutPtr,
                           md5Rounds);
    if (errorCode != TM_ENOERROR)
    {
        goto tlsPrfExit;
    }

    errorCode = tfTlsPhash(connStatePtr,
                           SADB_AALG_SHA1HMAC,
                           s2Ptr,
                           lengthS1,
                           (tt8BitPtr)labelPtr,
                           labelLength,
                           seedPtr,
                           seedLength,
                           sha1OutPtr,
                           sha1Rounds);
    if (errorCode != TM_ENOERROR)
    {
        goto tlsPrfExit;
    }

/*5. Last step, do the XOR  */
    for (i = 0; i < outputLength; i ++)
    {
        outputPtr[i] = (tt8Bit)(md5OutPtr[i] ^ sha1OutPtr[i]);
    }

tlsPrfExit:
    if (md5OutPtr)
    {
        tm_free_raw_buffer(md5OutPtr);
    }
    if (sha1OutPtr)
    {
        tm_free_raw_buffer(sha1OutPtr);
    }
    return errorCode;
}
#endif /* TM_USE_SSL_VERSION_31 */

#ifdef TM_USE_SSL_VERSION_33
/* For TLS 1.2 used only, in order to compute PRF():
 *   Unlike previous TLS versions 1.0, this PRF algorithm use only
 *   SHA-256 hash function. (This function can't use without TLS 1.2
 *   negotiation.)
 *
 * The PRF is created by applying P_hash to the secret as:
 *    PRF(secret, label, seed) = P_<hash>(secret, label + seed)
 *
 * And the difference of specification is outputLength. verify_data_length
 * change to depends on the cipher suite in current version. But any cipher
 * suite which does not explicitly specify verify_data_length has a
 * verify_data_length equal to 12 byte.
 *
 * PARAMETER:
 * connStatePtr                     SSL state
 * secretPtr, secretLength          parameter for secret
 * labelPtr, labelLength            The label for computation, will be seed1
 *                                  when we call tfTlsPhash
 * seed2Ptr seed2Length             The seed for computation, will be seed2
 *                                  when we call tfTlsPhash
 * outputPtr                        pointer to output
 * outputLength                     How many bytes output needed
 */


int tfTlsPrf2(ttSslConnectStatePtr  connStatePtr,
              tt8BitPtr             secretPtr,
              int                   secretLength,
              char TM_FAR *         labelPtr,
              int                   labelLength,
              tt8BitPtr             seedPtr,
              int                   seedLength,
              tt8BitPtr             outputPtr,
              int                   outputLength)
{
    int               errorCode;
    tt8BitPtr         sha256OutPtr;
    tt8Bit            sha256Rounds;


    errorCode = TM_ENOERROR;

    sha256OutPtr = (tt8BitPtr)0;
/*1. caculate the sha256 rounds */
    sha256Rounds = (tt8Bit)(outputLength / TM_SHA256_HASH_SIZE);
    if (outputLength % TM_SHA256_HASH_SIZE)
    {
        sha256Rounds = (tt8Bit)(sha256Rounds + 1);
    }

/*2. Get the hashing output buffer */
    sha256OutPtr = tm_get_raw_buffer(sha256Rounds * TM_SHA256_HASH_SIZE);
    if (!sha256OutPtr)
    {
        errorCode = TM_ENOBUFS;
        goto tlsPrfExit;
    }

/*3. Call p_hash (secret, seed) */
    errorCode = tfTlsPhash(connStatePtr,
                           SADB_AALG_SHA256HMAC,
                           secretPtr,
                           secretLength,
                           (tt8BitPtr)labelPtr,
                           labelLength,
                           seedPtr,
                           seedLength,
                           sha256OutPtr,
                           sha256Rounds);
    if (errorCode != TM_ENOERROR)
    {
        goto tlsPrfExit;
    }

/*4. Last step, copy hash result */
    tm_memcpy(outputPtr, sha256OutPtr, outputLength);

tlsPrfExit:
    if (sha256OutPtr)
    {
        tm_free_raw_buffer(sha256OutPtr);
    }
    return errorCode;
}
#endif /* TM_USE_SSL_VERSION_33 */
#endif /* TM_USE_SSL_VERSION_31 || TM_USE_SSL_VERSION_33 */


#ifdef TM_USE_SSL_VERSION_30

/* This function is called to get the master secret segments for
 * SSL 3.0 only. It is going to be called multiple, each time
 * you get an output of 16 bytes. You call this function again
 * and again until you get enough key material. This function
 * is called by tfSslGenerateMasterSecret to get the master
 * secret and is called by tfSslGenerateCryptoMacKeys to generate
 * final key material.
 * The formula when you calculate master secret is :
 *      master_secret =
 *       MD5(pre_master_secret + SHA('A' + pre_master_secret +
 *           ClientHello.random + ServerHello.random)) +
 *       MD5(pre_master_secret + SHA('BB' + pre_master_secret +
 *           ClientHello.random + ServerHello.random)) +
 *       MD5(pre_master_secret + SHA('CCC' + pre_master_secret +
 *           ClientHello.random + ServerHello.random));
 * The formula when you calculate the key material is:
 *      key_block =
 *       MD5(master_secret + SHA(`A' + master_secret +
 *                               ServerHello.random +
 *                               ClientHello.random)) +
 *       MD5(master_secret + SHA(`BB' + master_secret +
 *                               ServerHello.random +
 *                               ClientHello.random)) +
 *       MD5(master_secret + SHA(`CCC' + master_secret +
 *                               ServerHello.random +
 *                               ClientHello.random)) + [...];
 * The master secret includes three segments, each segment is
 * a md5 hashing output, we need call this function three times,
 * (each time with different label, and different output index)
 * For SSL 3.0, the index happens to be the length of labelPtr,
 * otherwise, we should use tm_strlen(labelPtr)-1 to get the
 * labelLength.
 * PARAMETER:
 * secretPtr, secretLen           The secret used. In the first formula,
 *                                 it is pre_master_secret, in the second
 *                                formula, it is master_secret
 * iteration                      The iteration you call this function,
 *                                 starts with 1
 * random1Ptr, random2Ptr         Pointer to the random, each is 32 bytes
 * outHashPtr                     Pointer to the output, each iteration,
 *                                  16 bytes output is generated
 * RETURN:
 * none
 */
static void tfSsl30GetKeyMaterial(tt8BitPtr          secretPtr,
                                  tt16Bit            secretLen,
                                  int                iteration,
                                  tt8BitPtr          random1Ptr,
                                  tt8BitPtr          random2Ptr,
                                  tt8BitPtr          outHashPtr)
{
    ttMd5CtxPtr            md5Ptr;
    ttSha1CtxPtr           sha1Ptr;
    tt8BitPtr              sha1OutPtr;

    md5Ptr = tm_get_raw_buffer(sizeof(ttMd5Ctx));
    sha1Ptr = tm_get_raw_buffer(sizeof(ttSha1Ctx));
    sha1OutPtr = tm_get_raw_buffer(TM_SHA1_HASH_SIZE);
    if (!md5Ptr || !sha1Ptr || !sha1OutPtr)
    {
        goto ssl30GetKeyMaterialExit;
    }

    tfSha1Init(sha1Ptr);
    tfSha1Update(sha1Ptr,(tt8BitPtr)(tlSsl30Label +
                 tfSsl30GetLabelIndex(iteration)),
                 (ttPktLen)iteration, 0);
    tfSha1Update(sha1Ptr, secretPtr, secretLen, 0);
    tfSha1Update(sha1Ptr, random1Ptr, TM_SSL_RANDOM_SIZE, 0);
    tfSha1Update(sha1Ptr, random2Ptr, TM_SSL_RANDOM_SIZE, 0);
    tfSha1Final(sha1OutPtr, sha1Ptr);

    tfMd5Init(md5Ptr);
    tfMd5Update(md5Ptr, secretPtr, secretLen, 0);
    tfMd5Update(md5Ptr, sha1OutPtr, TM_SHA1_HASH_SIZE, 0);
    tfMd5Final(outHashPtr, md5Ptr);

ssl30GetKeyMaterialExit:
    if (md5Ptr)
    {
        tm_free_raw_buffer((ttRawBufferPtr)md5Ptr);
    }
    if (sha1Ptr)
    {
        tm_free_raw_buffer((ttRawBufferPtr)sha1Ptr);
    }
    if (sha1OutPtr)
    {
        tm_free_raw_buffer((ttRawBufferPtr)sha1OutPtr);
    }

    return;
}


/* The following procedure is used to get MD5 and/or SHA1 hash output
 *     md5_hash       MD5(master_secret + pad2 +
 *                       MD5(handshake_messages + Sender +
 *                           master_secret + pad1));
 *    sha_hash        SHA(master_secret + pad2 +
 *                        SHA(handshake_messages + Sender +
 *                            master_secret + pad1));

 * The label is the Sender. It could be NULL (for example, when we
 * calculate the client cert verify message.
 * The md5_hash (16 bytes) and sh1_hash(20bytes) will be write to
 * memory location provided by digestOutPtr. If there is no md5_hash,
 * the sha1_hash will still be written to memory "digestOutPtr + 16".
 * Before making this call, md5Ptr and sha1Ptr should have information
 * about the md5(handshake_messages)
 */


int tfSsl30GetHandShakeHash(ttSslConnectStatePtr   connStatePtr,
                            ttMd5CtxPtr            md5Ptr,
                            ttSha1CtxPtr           sha1Ptr,
                            char TM_FAR *          labelPtr,
                            int                    labelLength,
                            tt8BitPtr              digestOutPtr)
{
    tt8BitPtr     pad1Ptr;
    tt8BitPtr     pad2Ptr;
    int           errorCode;
    int           i;

    errorCode = TM_ENOERROR;
    pad1Ptr = tm_get_raw_buffer(TM_SSL30_PAD_MD5_ROUNDS);
    pad2Ptr = tm_get_raw_buffer(TM_SSL30_PAD_MD5_ROUNDS);
    if (!pad1Ptr || !pad2Ptr)
    {
        errorCode = TM_ENOBUFS;
        goto ssl30GetHandShakeHashExit;
    }

/* we must have digestOutPtr for us to write our output,
 * For either RSA signature or DSA signature, we must have sha1Ptr
 * for DSA signature, the md5Ptr could be null.
 */
    if (!digestOutPtr || !sha1Ptr)
    {
        errorCode = TM_EINVAL;
        goto ssl30GetHandShakeHashExit;
    }

    for (i = 0; i < TM_SSL30_PAD_MD5_ROUNDS; i ++)
    {
        pad1Ptr[i] = TM_SSL30_PAD1;
        pad2Ptr[i] = TM_SSL30_PAD2;
    }

    if (md5Ptr)
    {
        if (labelPtr)
        {
            tfMd5Update(md5Ptr, (tt8BitPtr)labelPtr, (ttPktLen)labelLength, 0);
        }
        tfMd5Update(md5Ptr, connStatePtr->scsMasterSecret,
            TM_SSL_MASTER_SECRET_SIZE, 0);
        tfMd5Update(md5Ptr, pad1Ptr, TM_SSL30_PAD_MD5_ROUNDS, 0);
        tfMd5Final(digestOutPtr, md5Ptr);

        tfMd5Init(md5Ptr);
        tfMd5Update(md5Ptr, connStatePtr->scsMasterSecret,
            TM_SSL_MASTER_SECRET_SIZE, 0);
        tfMd5Update(md5Ptr, pad2Ptr, TM_SSL30_PAD_MD5_ROUNDS, 0);
        tfMd5Update(md5Ptr, digestOutPtr, TM_MD5_HASH_SIZE, 0);
        tfMd5Final(digestOutPtr, md5Ptr);
    }

    if (sha1Ptr)
    {
        if (labelPtr)
        {
            tfSha1Update(sha1Ptr, (tt8BitPtr)labelPtr, (ttPktLen)labelLength, 0);
        }
        tfSha1Update(sha1Ptr, connStatePtr->scsMasterSecret,
            TM_SSL_MASTER_SECRET_SIZE, 0);
        tfSha1Update(sha1Ptr, pad1Ptr, TM_SSL30_PAD_SHA1_ROUNDS, 0);
        tfSha1Final(digestOutPtr + TM_MD5_HASH_SIZE, sha1Ptr);

        tfSha1Init(sha1Ptr);
        tfSha1Update(sha1Ptr, connStatePtr->scsMasterSecret,
            TM_SSL_MASTER_SECRET_SIZE, 0);
        tfSha1Update(sha1Ptr, pad2Ptr, TM_SSL30_PAD_SHA1_ROUNDS, 0);
        tfSha1Update(sha1Ptr, digestOutPtr + TM_MD5_HASH_SIZE,
            TM_SHA1_HASH_SIZE, 0);
        tfSha1Final(digestOutPtr + TM_MD5_HASH_SIZE, sha1Ptr);
    }

ssl30GetHandShakeHashExit:
    if (pad1Ptr)
    {
        tm_free_raw_buffer(pad1Ptr);
    }
    if (pad2Ptr)
    {
        tm_free_raw_buffer(pad2Ptr);
    }
    return errorCode;
}




/* The following procedure is used to get MAC out of a record
 *     hash(MAC_write_secret + pad_2 +
 *         hash(MAC_write_secret + pad_1 + seq_num +
 *              SSLCompressed.type + SSLCompressed.length +
 *              SSLCompressed.fragment));
 *
 * The md5hmac_hash (16 bytes) and sha1hmac_hash(20bytes) will be write to
 * memory location provided by macOutPtr. Padding will be taken care of
 * before return. And We are responsible to update the  sslHeaderPtr to
 * have the length value = originalLength + hashing + paddingLength + 1.
 * sslRecordDataPtr must points to the record data. (NOT THE record
 * header.)
 * sslDataLength: the value should be exactly the same as that in
 *                sslHeaderPtr at this moment.
 * direction: Can be either TM_SSL_ENCODE or TM_SSL_DECODE
 * macOutPtr: Where to write the mac output. (After the MAC, we will
 *            do proper padding and then update the SSLHEADER
 */

/* if *macOutPtrPtr is NULL, we need put the mac at the end of
 * our packet list, and return. If *macOutPtrPtr is not NULL,
 * we just use it.
 */
int tfSsl30GetRecordMac(ttSslConnectStatePtr     connStatePtr,
                        ttSslRecordHeaderPtr     sslHeaderPtr,
                        ttPacketPtr              headPktPtr,
                        ttPktLen                 offset,
                        ttPktLen                 dataLength,
                        tt8Bit                   direction,
                        tt8BitPtr                macOutPtr)
{

    ttAhAlgorithmPtr       algPtr;
    tt8BitPtr              secretPtr;
    ttVoidPtr              contextPtr;
    tt8BitPtr              pad1Ptr;
    tt8BitPtr              pad2Ptr;
    tt32Bit                sequenceHigh;
    tt32Bit                sequenceLow;
    ttPktLen               thisRoundLen;
    int                    secretLength;
    int                    hashSize;
    int                    errorCode;
    int                    i;
    tt8Bit                 rounds;
#ifdef TM_SSL_DEBUG
    ttCharPtr              printBufPtr;
    ttPacketPtr            origPacketPtr;
    int                    debugOffset;
    tt32Bit                origOffset;

    origPacketPtr = headPktPtr;
    debugOffset = 0;
    origOffset = offset;
#endif /* TM_SSL_DEBUG */

    errorCode = TM_ENOERROR;
    rounds = 0;
    contextPtr = (ttVoidPtr)0;
    pad1Ptr = (tt8BitPtr)0;
    pad2Ptr = (tt8BitPtr)0;
    hashSize = 0;

/*1. Get the algorithmPtr, so that we can get the proper rounds, allocate
 *   the proper context
 */
    algPtr = connStatePtr->scsHashAlgPtr;

    if (algPtr->aalgName == SADB_AALG_MD5HMAC)
    {
        rounds = TM_SSL30_PAD_MD5_ROUNDS;
    }
    else if (algPtr->aalgName == SADB_AALG_SHA1HMAC)
    {
        rounds = TM_SSL30_PAD_SHA1_ROUNDS;
    }
    hashSize = connStatePtr->scsHashOutputSize;

    contextPtr = tm_get_raw_buffer((ttPktLen)(algPtr->aalgContextLenFuncPtr()));
    if (!contextPtr)
    {
        errorCode = TM_ENOBUFS;
        goto ssl30GetRecordMacExit;
    }

/*2. Allocate pad1 and pad2 */
    pad1Ptr = tm_get_raw_buffer(TM_SSL30_PAD_MD5_ROUNDS);
    pad2Ptr = tm_get_raw_buffer(TM_SSL30_PAD_MD5_ROUNDS);
    if (!pad1Ptr || !pad2Ptr)
    {
        errorCode = TM_ENOBUFS;
        goto ssl30GetRecordMacExit;
    }
    for (i = 0; i < TM_SSL30_PAD_MD5_ROUNDS; i ++)
    {
        pad1Ptr[i] = TM_SSL30_PAD1;
        pad2Ptr[i] = TM_SSL30_PAD2;
    }

/*3 Get the proper secret */
#if (defined(TM_SSL_DEBUG) || defined(TM_LOG_SSL_DEBUG))
    if (direction == TM_SSL_ENCODE)
    {
#ifdef TM_SSL_DEBUG
        tm_debug_log0("\n--- MAC ENCODE ---\n");
#endif /* TM_SSL_DEBUG */
#ifdef TM_LOG_SSL_DEBUG
        tfEnhancedLogWrite(
            TM_LOG_MODULE_SSL,
            TM_LOG_LEVEL_DEBUG,
            "tfSsl30GetRecordMac: MAC Encode");
#endif /* TM_LOG_SSL_DEBUG */
    }
    else
    {
#ifdef TM_SSL_DEBUG
        tm_debug_log0("\n--- MAC DECODE ---\n");
#endif /* TM_SSL_DEBUG */
#ifdef TM_LOG_SSL_DEBUG
        tfEnhancedLogWrite(
            TM_LOG_MODULE_SSL,
            TM_LOG_LEVEL_DEBUG,
            "tfSsl30GetRecordMac: MAC Decode");
#endif /* TM_LOG_SSL_DEBUG */
    }
#endif /* TM_SSL_DEBUG || TM_LOG_SSL_DEBUG */

    if (direction == TM_SSL_ENCODE)
    {
        secretPtr = connStatePtr->scsEncodeMacKey.keyDataPtr;
        secretLength = (int)(connStatePtr->scsEncodeMacKey.keyBits >> 3);
        sequenceHigh = htonl(connStatePtr->scsOutSeqNumberHigh);
        sequenceLow = htonl(connStatePtr->scsOutSeqNumber);
    }
    else
    {
        secretPtr = connStatePtr->scsDecodeMacKey.keyDataPtr;
        secretLength = (int)(connStatePtr->scsDecodeMacKey.keyBits >> 3);
        sequenceHigh = htonl(connStatePtr->scsInSeqNumberHigh);
        sequenceLow = htonl(connStatePtr->scsInSeqNumber);
    }

/*4. Calculate the inner hash */
    algPtr->aalgInitFuncPtr(contextPtr);
    algPtr->aalgUpdateFuncPtr(contextPtr, secretPtr, (ttPktLen)secretLength,0);
    algPtr->aalgUpdateFuncPtr(contextPtr, pad1Ptr, rounds, 0);
    algPtr->aalgUpdateFuncPtr(contextPtr, (tt8BitPtr)&sequenceHigh,
                              sizeof(tt32Bit), 0);
    algPtr->aalgUpdateFuncPtr(contextPtr, (tt8BitPtr)&sequenceLow,
                              sizeof(tt32Bit), 0);
    algPtr->aalgUpdateFuncPtr(contextPtr, &sslHeaderPtr->srhType,1,0);
    algPtr->aalgUpdateFuncPtr(contextPtr, &sslHeaderPtr->srhLengthHigh,2,0);
    while (headPktPtr)
    {
        thisRoundLen = ((headPktPtr->pktLinkDataLength - offset) >=
                dataLength) ? dataLength :
                (headPktPtr->pktLinkDataLength - offset);
        algPtr->aalgUpdateFuncPtr(contextPtr,
                                  headPktPtr->pktLinkDataPtr + offset,
                                  thisRoundLen,
                                  0);
        offset = 0;
        dataLength -= (ttPktLen)thisRoundLen;
        headPktPtr = headPktPtr ->pktLinkNextPtr;
    }

    algPtr->aalgFinalFuncPtr(macOutPtr, contextPtr);
#ifdef TM_SSL_DEBUG
    tm_debug_log0("\nMac inner data = ");
    printBufPtr = (ttCharPtr)0;
    tfCryptoDebugPrintf(&printBufPtr,
                        secretPtr,
                        secretLength,
                        &debugOffset);
    tfCryptoDebugPrintf(&printBufPtr,
                        pad1Ptr,
                        rounds,
                        &debugOffset);
    tfCryptoDebugPrintf(&printBufPtr,
                        (tt8BitPtr)&sequenceHigh,
                        4,
                        &debugOffset);
    tfCryptoDebugPrintf(&printBufPtr,
                        (tt8BitPtr)&sequenceLow,
                        4,
                        &debugOffset);
    tfCryptoDebugPrintf(&printBufPtr,
                        &sslHeaderPtr->srhType,
                        1,
                        &debugOffset);
    tfCryptoDebugPrintf(&printBufPtr,
                        &sslHeaderPtr->srhLengthHigh,
                        2,
                        &debugOffset);
    while (origPacketPtr)
    {
        tfCryptoDebugPrintf(&printBufPtr,
                            origPacketPtr->pktLinkDataPtr + origOffset,
                            origPacketPtr->pktLinkDataLength - origOffset,
                            &debugOffset);
        origOffset = 0;
        origPacketPtr = origPacketPtr->pktLinkNextPtr;
    }
    tfCryptoDebugFlush(&printBufPtr);
#endif /* TM_SSL_DEBUG */

/*5. Calculate the outer hash */
    algPtr->aalgInitFuncPtr(contextPtr);
    algPtr->aalgUpdateFuncPtr(contextPtr, secretPtr, (ttPktLen)secretLength,0);
    algPtr->aalgUpdateFuncPtr(contextPtr, pad2Ptr, rounds, 0);
    algPtr->aalgUpdateFuncPtr(contextPtr, macOutPtr, (ttPktLen)hashSize,0);
    algPtr->aalgFinalFuncPtr(macOutPtr,  contextPtr);
#ifdef TM_SSL_DEBUG
    debugOffset = 0;
    tm_debug_log0("\nMac outer data = ");
    tfCryptoDebugPrintf(&printBufPtr,
                        secretPtr,
                        secretLength,
                        &debugOffset);
    tfCryptoDebugPrintf(&printBufPtr,
                        pad2Ptr,
                        rounds,
                        &debugOffset);
    tfCryptoDebugPrintf(&printBufPtr,
                        macOutPtr,
                        hashSize,
                        &debugOffset);
    tfCryptoDebugFlush(&printBufPtr);
    tm_debug_log0("\nMac OUTPUT = ");
    tfCryptoDebugPrintf((ttCharPtrPtr)0,
                        macOutPtr,
                        hashSize,
                        (ttIntPtr)0);
#endif /* TM_SSL_DEBUG */

ssl30GetRecordMacExit:
    if (pad1Ptr)
    {
        tm_free_raw_buffer(pad1Ptr);
    }
    if (pad2Ptr)
    {
        tm_free_raw_buffer(pad2Ptr);
    }
    if (contextPtr)
    {
        tm_free_raw_buffer(contextPtr);
    }
    return errorCode;
}


#endif /* TM_USE_SSL_VERSION_30 */



/* This function get the master keys */
int tfSslGenerateMasterSecret(ttSslConnectStatePtr    connStatePtr)
{

    int       errorCode;
#ifdef TM_USE_SSL_VERSION_30
    int       iter;
#endif /* TM_USE_SSL_VERSION_30 */

    errorCode = TM_ENOERROR;
/* no matter what, we calculate master secret here if Version
 * 3.0 is used. For version 3.1, master secret is only used when
 * we are trying to send finished message, so we calculate there.
 */
#ifdef TM_USE_SSL_VERSION_30
    if (connStatePtr->scsVersion == TM_SSL_VERSION_30)
    {
        for (iter = 1; iter < 4; iter ++)
        {
/*     master_secret =*/
/*       MD5(pre_master_secret + SHA('A' + pre_master_secret +*/
/*           ClientHello.random + ServerHello.random)) +*/
/*       MD5(pre_master_secret + SHA('BB' + pre_master_secret +*/
/*           ClientHello.random + ServerHello.random)) +*/
/*       MD5(pre_master_secret + SHA('CCC' + pre_master_secret +*/
/*           ClientHello.random + ServerHello.random));*/

            tfSsl30GetKeyMaterial(connStatePtr->scsPreMasterPtr,
                connStatePtr->scsPreMasterLen,
                iter,
                connStatePtr->scsClientRandom,
                connStatePtr->scsServerRandom,
                connStatePtr->scsMasterSecret
                + (iter - 1) * TM_MD5_HASH_SIZE);
        }
    }
#endif /* TM_USE_SSL_VERSION_30 */

#ifdef TM_USE_SSL_VERSION_31
    if (connStatePtr->scsVersion == TM_SSL_VERSION_31)
    {
/* 3.2 Compute master secret key */
        errorCode = tfTlsPrf(
                 connStatePtr,
                 connStatePtr->scsPreMasterPtr,
                 connStatePtr->scsPreMasterLen,
                 TM_TLS_MASTER_SECRET_LABEL,
                 TM_TLS_MASTER_SECRET_LABELLEN,
                 connStatePtr->scsClientRandom,
                 TM_SSL_RANDOM_SIZE << 1,
                 connStatePtr->scsMasterSecret,
                 TM_SSL_MASTER_SECRET_SIZE);
        if (errorCode != TM_ENOERROR)
        {
            goto sslGenerateMasterExit;
        }
    }
#endif /* TM_USE_SSL_VERSION_31 */

#ifdef TM_USE_SSL_VERSION_33
    if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
/* 3.2 Compute master secret key */
        errorCode = tfTlsPrf2(
                 connStatePtr,
                 connStatePtr->scsPreMasterPtr,
                 connStatePtr->scsPreMasterLen,
                 TM_TLS_MASTER_SECRET_LABEL,
                 TM_TLS_MASTER_SECRET_LABELLEN,
                 connStatePtr->scsClientRandom,
                 TM_SSL_RANDOM_SIZE << 1,
                 connStatePtr->scsMasterSecret,
                 TM_SSL_MASTER_SECRET_SIZE);
        if (errorCode != TM_ENOERROR)
        {
            goto sslGenerateMasterExit;
        }
    }
#endif /* TM_USE_SSL_VERSION_33 */

#ifdef TM_SSL_DEBUG
    tm_debug_log0("Master Secret = \n");
    tfCryptoDebugPrintf((ttCharPtrPtr)0,
                        connStatePtr->scsMasterSecret,
                        TM_SSL_MASTER_SECRET_SIZE,
                        (ttIntPtr)0);
#endif /* TM_SSL_DEBUG */

/* SSL requires The Pre master secret should be deleted from memory once
 * the master secret has been computed
 */
    if ((connStatePtr->scsCipherPtr->csKeyExchange !=
        TM_SSL_KEXCH_DHE) &&
        (connStatePtr->scsCipherPtr->csKeyExchange !=
        TM_SSL_KEXCH_DHEEXPORT1024))
    {
        if (connStatePtr->scsPreMasterPtr)
        {
            tm_free_raw_buffer(connStatePtr->scsPreMasterPtr);
            connStatePtr->scsPreMasterPtr = (tt8BitPtr)0;
        }
        connStatePtr->scsPreMasterLen = 0;
    }
#ifdef TM_SSL_USE_EPHEMERAL_DH
    else
    {
/* we should free the whole DH information structure */
        tm_free_raw_buffer(connStatePtr->scsDHInfoPtr->dhGxiPtr);
        tm_free_raw_buffer(connStatePtr->scsDHInfoPtr->dhGxrPtr);
        tm_free_raw_buffer(connStatePtr->scsDHInfoPtr->dhGxyPtr);
        tm_free_raw_buffer(connStatePtr->scsDHInfoPtr->dhPrimePtr);
        tm_free_raw_buffer(connStatePtr->scsDHInfoPtr->dhSecretPtr);
        tm_free_raw_buffer(connStatePtr->scsDHInfoPtr);
        connStatePtr->scsDHInfoPtr = (ttDiffieHellmanPtr)0;
        connStatePtr->scsPreMasterPtr = (tt8BitPtr)0;
        connStatePtr->scsPreMasterLen = 0;
    }
#endif /*TM_SSL_USE_EPHEMERAL_DH */

#if (defined(TM_USE_SSL_VERSION_31) || defined(TM_USE_SSL_VERSION_33))
sslGenerateMasterExit:
#endif /* TM_USE_SSL_VERSION_31 || TM_USE_SSL_VERSION_33 */
    return errorCode;
}


/* search the whole certificate list according to the certicate name. If we
 * can not find the whole certificate chain according to the certified name,
 * we return error.
 */

int tfSslGetCertList(ttSslSessionPtr    sslSessionPtr)
{
    char TM_FAR *             nameId;
    ttPkiCertListPtr          nextCertPtr;
    ttPkiCertListPtr          prevCertPtr;
    int                       pkiType;
    int                       isEqual;

    prevCertPtr = (ttPkiCertListPtr)0;
    nameId = sslSessionPtr->ssMyIdPtr;

    tm_call_lock_wait(&(tm_context(tvPkiLockEntry)));
/* firstly, search from our own certificate (TM_CERT_LOCAL) */
    nextCertPtr = tfPkiCertificateSearch(TM_PKI_CERTSEARCH_ID,
                           nameId,
                           (int) tm_strlen(nameId),
                           TM_PKI_CERT_SELF_INDX, /* own certificate list */
                           TM_8BIT_NO); /* already locked */
    if (nextCertPtr)
    {
        pkiType = nextCertPtr->certKeyPtr->pkType;
        if (pkiType == TM_PKI_OBJ_DSA  ||
                pkiType == TM_PKI_OBJ_DSASHA1)
        {
            sslSessionPtr->ssSignAlgorithm = SADB_PUBKEY_DSA;
        }
        else if (pkiType == TM_PKI_OBJ_RSA    ||
                pkiType == TM_PKI_OBJ_RSAMD5 ||
                pkiType == TM_PKI_OBJ_RSASHA1||
                pkiType == TM_PKI_OBJ_RSASHA256||
                pkiType == TM_PKI_OBJ_RSARIPEMD)
        {
            sslSessionPtr->ssSignAlgorithm = SADB_PUBKEY_RSA;
        }
    }

    while (nextCertPtr)
    {
        isEqual = tm_memcmp(nextCertPtr->certDNPtr,
                      nextCertPtr->certSubPtr,
                      nextCertPtr->certDNLength);
        if (!isEqual)
        {
/* we got the Root CA, break */
            if (sslSessionPtr->ssCertListPtr == (ttPkiCertListPtr)0)
            {
                sslSessionPtr->ssCertListPtr = nextCertPtr;
                nextCertPtr->certCheckOut++;
            }
            break;
        }

        if (prevCertPtr)
        {
            prevCertPtr->certSslNextPtr = nextCertPtr;
        }
        else
        {
            sslSessionPtr->ssCertListPtr = nextCertPtr;
        }

        nextCertPtr->certCheckOut++;
        prevCertPtr = nextCertPtr;
/* search by subject. (using my issurer information as subject)*/
        nextCertPtr = tfPkiCertificateSearch(TM_PKI_CERTSEARCH_SUBJECT,
                            prevCertPtr->certDNPtr,
                            prevCertPtr->certDNLength,
                            TM_PKI_CERT_OTHER_INDX, /* other certificate list */
                            TM_8BIT_NO); /* already locked */

    }
    tm_call_unlock(&(tm_context(tvPkiLockEntry)));
/* even if we don't find the CA, don't return error, later we
 * pass this decision to user to see if they accept this certificate or
 * not
 */
    return TM_ENOERROR;
}

/*
 * Description tfSslConnectUserClose
 * User makes this function call to close a SSL connection. Inside this
 * function, we will flush the SSL send buffer and send a close notify
 * to the SSL peer.  User may call tfClose to close the whole socket
 * instead of just closing the SSL layer. (Note that the peer may not
 * accept any data after SSL layer is closed.)
 *
 * Parameters
 * socketDescriptor         socket number

 * Returns
 * Value    Meaning
 * TM_ENOERROR          Success call
 * -1                   Error happens, user has to call tfGetSocketError to
 *                      get the related socket error
 */
int tfSslConnectUserClose(int  socketDescriptor)
{
    ttSocketEntryPtr     socketEntryPtr;
    ttSslConnectStatePtr connStatePtr;
    ttTcpVectPtr         tcpVectPtr;
    tt16Bit              cbFlags;
    int                  errorCode;
#ifdef TM_USE_BSD_DOMAIN
    int                  af;
#endif /* TM_USE_BSD_DOMAIN */

    errorCode = TM_ENOERROR;
/* Map from socket descriptor to locked socket entry pointer */
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_INET;
#endif /* TM_USE_BSD_DOMAIN */
    socketEntryPtr = tfSocketCheckValidLock(  socketDescriptor
#ifdef TM_USE_BSD_DOMAIN
                                            , &af
#endif /* TM_USE_BSD_DOMAIN */
                                           );
    if ( socketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR )
    {
        errorCode = TM_EBADF;
        goto sslConnectUserCloseExit;
    }
/* make sure it is a TCP socket */
    if (socketEntryPtr->socProtocolNumber != (tt8Bit)IP_PROTOTCP)
    {
        errorCode = TM_EPROTONOSUPPORT;
        goto sslConnectUserCloseExit;
    }

    tcpVectPtr = (ttTcpVectPtr)((ttVoidPtr)socketEntryPtr);

    errorCode = socketEntryPtr->socSendError;
    if (errorCode != TM_ENOERROR)
    {
        socketEntryPtr->socSendError = 0;
        goto sslConnectUserCloseExit;
    }
    if (tm_16bit_one_bit_set( socketEntryPtr->socFlags,
                              TM_SOCF_NO_MORE_SEND_DATA) )
    {
/* Tcp connection already closed by the user */
        errorCode = TM_ESHUTDOWN;
        goto sslConnectUserCloseExit;
    }
    if (!tm_16bit_all_bits_set(tcpVectPtr->tcpsSslFlags,
                       (TM_SSLF_ESTABLISHED | TM_SSLF_SEND_ENABLED)))
    {
/* either SSL is not established or already closed */
        errorCode = TM_EINVAL;
        goto sslConnectUserCloseExit;
    }

    connStatePtr = tcpVectPtr->tcpsSslConnStatePtr;
    errorCode = tfSslClose(connStatePtr, 1); /* user */
    cbFlags = connStatePtr->scsSocCBFlags;
/*
 * After tfSocketNotify(), or after tfTcpSendPacket(), connStatePtr might
 * no longer be a valid pointer, because the TCP vector will be unlocked
 * and relocked. So reset scsSocCBFlags before the call.
 */
    connStatePtr->scsSocCBFlags = TM_16BIT_ZERO;
    if (errorCode == TM_ENOERROR)
    {
        if (    (tcpVectPtr->tcpsState == TM_TCPS_ESTABLISHED)
             || (tcpVectPtr->tcpsState == TM_TCPS_CLOSE_WAIT) )
        {
/* TCP vector/socket entry unlocked/relocked in this call */
            errorCode = tfTcpSendPacket(tcpVectPtr);
        }
    }
    if (cbFlags != TM_16BIT_ZERO)
    {
/* TCP vector/socket entry unlocked/relocked in this call */
        tfSocketNotify(&(tcpVectPtr->tcpsSocketEntry),
                       TM_SELECT_READ | TM_SELECT_WRITE,
                       cbFlags,
                       0);
    }
sslConnectUserCloseExit:
    return tfSocketReturn(socketEntryPtr, socketDescriptor,
                                 errorCode, errorCode);
}

/*
 * Description
 * SSL will queue user data if the send path is not triggered up. There is a
 * send size threshold in order to trigger sending. See macro
 * TM_SSL_SEND_DATA_MIN_SIZE for details. If user wants to manually flush the
 * SSL send buffer, user may call this function, all data if any will be sent
 * out right away. (Instead of making this function call, user may set NODELAY
 * option to send the data right away too.)
 *
 * Parameters
 * socketDescriptor         socket number
 *
 * Returns
 * Value    Meaning
 * -1       No data was flushed, user has to call tfGetSocketError to
 *          get the related socket error
 * Non-negative  The number of data bytes flushed by this call
 */
int tfSslUserSendFlush(int          socketDescriptor)
{
    ttSocketEntryPtr socketEntryPtr;
    ttTcpVectPtr     tcpVectPtr;
    int              errorCode;
    int              retCode;
#ifdef TM_USE_BSD_DOMAIN
    int              af;
#endif /* TM_USE_BSD_DOMAIN */
    ttPktLen         copyBytes;
    ttPacketPtr      newPacketPtr;

    errorCode = TM_ENOERROR;
    copyBytes = 0;
/* Map from socket descriptor to locked socket entry pointer */
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_INET;
#endif /* TM_USE_BSD_DOMAIN */
    socketEntryPtr = tfSocketCheckValidLock(  socketDescriptor
#ifdef TM_USE_BSD_DOMAIN
                                            , &af
#endif /* TM_USE_BSD_DOMAIN */
                                           );
    if ( socketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR )
    {
        errorCode = TM_EBADF;
        goto sendExit;
    }
/* make sure it is a TCP socket */
    if (socketEntryPtr->socProtocolNumber != (tt8Bit)IP_PROTOTCP)
    {
        errorCode = TM_EPROTONOSUPPORT;
        goto sendExit;
    }

    tcpVectPtr = (ttTcpVectPtr)((ttVoidPtr)socketEntryPtr);

    errorCode = socketEntryPtr->socSendError;
    if (errorCode != TM_ENOERROR)
    {
        socketEntryPtr->socSendError = 0;
        goto sendExit;
    }

    if (!tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                             TM_SSLF_ESTABLISHED))
    {
        errorCode = TM_EINVAL;
        goto sendExit;
    }

    if (!tcpVectPtr->tcpsSslConnStatePtr)
    {
/* we don't have any SSL connection state, exit*/
        goto sendExit;
    }

    if (tcpVectPtr->tcpsSslSendQBytes > 0)
    {

        newPacketPtr =  tfSslBuildRecordFromUserData(tcpVectPtr,
                                    tcpVectPtr->tcpsSslConnStatePtr);
        if (newPacketPtr)
        {
            copyBytes = newPacketPtr->pktChainDataLength;
            errorCode = tfSslSend(&tcpVectPtr->tcpsSocketEntry,
                      tcpVectPtr->tcpsSslConnStatePtr,
                      &newPacketPtr,
                      &copyBytes,
                      0); /* don't treat it as application data*/
            if (errorCode == TM_ENOERROR)
            {
                if (   (tcpVectPtr->tcpsState == TM_TCPS_ESTABLISHED)
                    || (tcpVectPtr->tcpsState == TM_TCPS_CLOSE_WAIT) )
                {
/*
 * TCP vector/socket entry unlocked/relocked in this call. connStatePtr
 * might not be valid after this call.
 */
                    errorCode = tfTcpSendPacket(tcpVectPtr);
                }
            }
        }
    }

sendExit:
    retCode = tfSocketReturn(socketEntryPtr, socketDescriptor,
                                 errorCode, (int)copyBytes);
    return retCode;
}

/*
 * Called by user to process the SSL handshake in the user context
 */
int tfSslUserProcessHandshake(int socketDescriptor)
{
    ttSocketEntryPtr        socketEntryPtr;
    ttTcpVectPtr            tcpVectPtr;
    ttSslConnectStatePtr    connStatePtr;
    int                     errorCode;
#ifdef TM_USE_BSD_DOMAIN
    int                     af;
#endif /* TM_USE_BSD_DOMAIN */

    errorCode = TM_ENOERROR;
/* Map from socket descriptor to locked socket entry pointer */
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_INET;
#endif /* TM_USE_BSD_DOMAIN */
    socketEntryPtr = tfSocketCheckValidLock(  socketDescriptor
#ifdef TM_USE_BSD_DOMAIN
                                            , &af
#endif /* TM_USE_BSD_DOMAIN */
                                           );
    if ( socketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR )
    {
        errorCode = TM_EBADF;
        goto processHandshkExit;
    }
/* make sure it is a TCP socket */
    if (socketEntryPtr->socProtocolNumber != (tt8Bit)IP_PROTOTCP)
    {
        errorCode = TM_EPROTONOSUPPORT;
        goto processHandshkExit;
    }

    tcpVectPtr = (ttTcpVectPtr)((ttVoidPtr)socketEntryPtr);
    connStatePtr = tcpVectPtr->tcpsSslConnStatePtr;

    if (!connStatePtr)
    {
/* we don't have any SSL connection state, exit*/
        errorCode = TM_ENOENT;
        goto processHandshkExit;
    }

    if (tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                             TM_SSLF_HANDSHK_PROCESS))
    {
        errorCode = tfSslIncomingRecord(connStatePtr,
                                        TM_SSL_OTHER_API);
    }
    else
    {
        errorCode = TM_EWOULDBLOCK;
    }
processHandshkExit:
    return tfSocketReturn(socketEntryPtr, socketDescriptor, errorCode, 0);
}

/*This function is called to send alert message to the SSL peer.
 * PARAMETER
 * alertLevel               The alert level of alert message
 * alertType                The alert type of alert message
 * RETURN:
 * TM_ENOERROR              no error
 * TM_ENOBUFS               no buffer
 */
int tfSslBuildSendAlert(ttSslConnectStatePtr   connStatePtr,
                        tt8Bit                 alertLevel,
                        tt8Bit                 alertType)
{
    ttPacketPtr               newPacketPtr;
    ttSslRecordHeaderPtr      sslHeaderPtr;
    tt8BitPtr                 dataPtr;
    int                       errorCode;
    ttPktLen                  totalLength;
    tt16Bit                   alertLen;
    tt16Bit                   cbFlags;
    tt8Bit                    hashSize;
    tt8Bit                    blockSize;
    tt8Bit                    ivSize;


    errorCode = TM_ENOERROR;
    hashSize = connStatePtr->scsHashOutputSize;
    blockSize = connStatePtr->scsBlockSize;
    ivSize = (tt8Bit)0;

#ifdef TM_USE_SSL_VERSION_33
    if((connStatePtr->scsVersion == TM_SSL_VERSION_33)
      &&(
#ifdef TM_USE_SSL_CLIENT
        ((connStatePtr->scsClientOrServer == TM_SSL_CLIENT) &&
        ((connStatePtr->scsState >= TM_SSL_CT_SENT_FINISH)
        ||(connStatePtr->scsState == TM_SSL_CT_RECV_SH2)))
#ifdef TM_USE_SSL_SERVER
        ||
#endif /* TM_USE_SSL_SERVER */
#endif /* TM_USE_SSL_CLIENT */
#ifdef TM_USE_SSL_SERVER
        ((connStatePtr->scsClientOrServer == TM_SSL_SERVER) &&
         ((connStatePtr->scsState >= TM_SSL_ST_OPENED)
        ||(connStatePtr->scsState == TM_SSL_ST_RECV_CH2)))
#endif /* TM_USE_SSL_SERVER */
        ))
    {
        if((connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_ARCFOUR)
         &&(connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_NULL))
        {
            ivSize = connStatePtr->scsCryptoAlgPtr->ealgBlockSize;
        }
    }
#endif /* TM_USE_SSL_VERSION_33 */

    newPacketPtr = tfGetSharedBuffer(0,
                                     TM_SSL_ALERT_MESSAGE_SIZE +
                                     TM_SSL_RECORD_HEADER_SIZE +
                                     blockSize +
                                     hashSize +
                                     ivSize,
                                     0);
    if (!newPacketPtr)
    {
        errorCode = TM_ENOBUFS;
        goto buildSendAlertExit;
    }
/* don't reset pktLinkDataLength. Keep pktLinkDataLength the be the maximum
 * possible length so that when we do cbcencryption, we can always include
 * the mac data.
 */
    dataPtr = newPacketPtr->pktLinkDataPtr;
/* 1. Construct part of the SSL record header */
    sslHeaderPtr = (ttSslRecordHeaderPtr)dataPtr;
    sslHeaderPtr->srhType = TM_SSL_TYPE_ALERT;
    sslHeaderPtr->srhVersionMajor = TM_SSL_VERSION_MAJOR;
    if (    (connStatePtr->scsVersion != TM_SSL_VERSION_33)
         && (connStatePtr->scsVersion != TM_SSL_VERSION_31)
         && (connStatePtr->scsVersion != TM_SSL_VERSION_30) )
    {
        if (connStatePtr->scsSsVersion & TM_SSL_VERSION_33)
        {
            connStatePtr->scsVersion = TM_SSL_VERSION_33;
        }
        else if (connStatePtr->scsSsVersion & TM_SSL_VERSION_31)
        {
            connStatePtr->scsVersion = TM_SSL_VERSION_31;
        }
        else
        {
            connStatePtr->scsVersion = TM_SSL_VERSION_30;
        }
    }
#ifdef TM_USE_SSL_VERSION_33
    if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
        sslHeaderPtr->srhVersionMinor = TM_SSL_VERSION_MINOR3;
    }
    else
#endif /* TM_USE_SSL_VERSION_33 */
    {
        sslHeaderPtr->srhVersionMinor = (tt8Bit)
                  (connStatePtr->scsVersion >> 1);
    }
    dataPtr += TM_SSL_RECORD_HEADER_SIZE;

    if(ivSize)
    {
/* Insert IV field in case of CBC. */
        tfGetRandomBytes(connStatePtr->scsEncodeIvPtr, ivSize);
        tm_bcopy(connStatePtr->scsEncodeIvPtr,
                 dataPtr,
                 ivSize);
        dataPtr += ivSize;
#ifdef TM_SSL_DEBUG
/* print the key blocks */
        tm_debug_log0("\nRandom IV = \n");
        tfCryptoDebugPrintf((ttCharPtrPtr)0,
                            connStatePtr->scsEncodeIvPtr,
                            ivSize ,
                            (ttIntPtr)0);
#endif /* TM_SSL_DEBUG */
    }

/* 2 dataPtr here points to the alert message*/
    *dataPtr ++ = alertLevel;
    *dataPtr ++ = alertType;

    sslHeaderPtr->srhLengthHigh = (tt8Bit)(TM_SSL_ALERT_MESSAGE_SIZE >> 8);
    sslHeaderPtr->srhLengthLow = (tt8Bit)(TM_SSL_ALERT_MESSAGE_SIZE & 0xff);

    if(  ((connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_REHANDSHAKE)
       && (connStatePtr->scsCryptoEnginePtr != (ttCryptoEnginePtr)0))
        ||
#ifdef TM_USE_SSL_CLIENT
        ((connStatePtr->scsClientOrServer == TM_SSL_CLIENT) &&
        ((connStatePtr->scsState >= TM_SSL_CT_SENT_FINISH)
        ||(connStatePtr->scsState == TM_SSL_CT_RECV_SH2)))
#ifdef TM_USE_SSL_SERVER
        ||
#endif /* TM_USE_SSL_SERVER */
#endif /* TM_USE_SSL_CLIENT */
#ifdef TM_USE_SSL_SERVER
        ((connStatePtr->scsClientOrServer == TM_SSL_SERVER) &&
         ((connStatePtr->scsState >= TM_SSL_ST_OPENED)
        ||(connStatePtr->scsState == TM_SSL_ST_RECV_CH2)))
#endif /* TM_USE_SSL_CLIENT */
        )

    {

        alertLen = TM_SSL_ALERT_MESSAGE_SIZE;

/* 6. Do SSL encapsulation of this record.
 * Note that: 1) The handshake header is fixed, You can not modify it.
 *            2) Do hashing across the whole SSL record
 *            3) Padding if necessory
 *            4) Set the new SSL record length
 *            5) Encryption the SSL record
 */
        errorCode = tfSslHandshakeMsgTransform(connStatePtr,
                                               TM_CRYPTOREQUEST_ENCODE,
                                               sslHeaderPtr,
                                               newPacketPtr,
                                               dataPtr,
                                               &alertLen,
                                               TM_SSL_RECORD_HEADER_SIZE +
                                               ivSize);

        if (errorCode != TM_ENOERROR)
        {
            tfFreePacket(newPacketPtr, TM_SOCKET_LOCKED);
            goto buildSendAlertExit;
        }
    }
    else
    {
        alertLen = TM_SSL_ALERT_MESSAGE_SIZE;
    }
    alertLen += TM_SSL_RECORD_HEADER_SIZE;
    newPacketPtr->pktLinkDataLength = alertLen;
    newPacketPtr->pktChainDataLength = alertLen;

    totalLength = alertLen;
    errorCode = tfSslSend(&connStatePtr->scsTcpVectPtr->tcpsSocketEntry,
                          connStatePtr,
                          &newPacketPtr,
                          &totalLength,
                          0);

    cbFlags = (tt16Bit)0;
    if(alertLevel == TM_SSL_ALTLEV_FATAL)
    {
        cbFlags = (tt16Bit)TM_CB_SSL_HANDSHK_FAILURE;
/* SSL handshake failure, for security reason, you can not send if this bit
 * is set
 */
        connStatePtr->scsTcpVectPtr->tcpsSslFlags |= TM_SSLF_HANDSHK_FAILURE;
    }
    if(alertType == TM_SSL_ALT_CLOSE_NOTIFY)
    {
        cbFlags |= (tt16Bit)TM_CB_SSL_SEND_CLOSE;
    }
    else if(alertType == TM_SSL_ALT_BADCERT)
    {
        cbFlags |= (tt16Bit)TM_CB_SSL_BADCERTIFICATE;
    }

    if(cbFlags != 0)
    {
/*
 * Notify user that ssl error has happened. Done at the end of
 * tfSslIncomingRecord(), or tfSslConnectUserClose() (user app path)
 * or tfSslIncomingPacket() (TCP incoming path).
 */
        connStatePtr->scsSocCBFlags |= cbFlags;
    }
buildSendAlertExit:
    return  errorCode;
}

/* This function is called by TCP state machine or by tfSslConnectUserClose
 * Upon calling, we need do
 * 1) Flush queued bytes if any
 * 2) set flag that we don't send any more data
 * 3) send close_notify alert message
 * RETURN:
 * TM_ENOERROR          No error
 */
int tfSslClose(ttSslConnectStatePtr connStatePtr, int user)
{
    ttPacketPtr      newPacketPtr;
    ttTcpVectPtr     tcpVectPtr;
    int              errorCode;
    ttPktLen         totalBytes;

    errorCode = TM_ENOERROR;
    tcpVectPtr = connStatePtr->scsTcpVectPtr;

/* 1) Flush queued bytes if any */
    if ( (tcpVectPtr->tcpsSslSendQBytes > 0) &&
        (tcpVectPtr->tcpsSslFlags & TM_SSLF_ESTABLISHED))
    {
        newPacketPtr =  tfSslBuildRecordFromUserData(tcpVectPtr,
                                    tcpVectPtr->tcpsSslConnStatePtr);
        if (newPacketPtr)
        {
            totalBytes = newPacketPtr->pktChainDataLength;
            errorCode = tfSslSend(&tcpVectPtr->tcpsSocketEntry,
                      tcpVectPtr->tcpsSslConnStatePtr,
                      &newPacketPtr,
                      &totalBytes,
                      0); /* don't treat it as application data*/
            if ( (errorCode != TM_ENOERROR) && user)
            {
/* return TM_EWOULDBLOCK to the user. */
                goto sslCloseExit;
            }
/* Don't call tfTcpSendPacket. Done by the caller. */
        }
    }

/* 2) set flag that we don't send any more data */
    tm_16bit_clr_bit(tcpVectPtr->tcpsSslFlags, TM_SSLF_SEND_ENABLED);

/* 3) send close_notify alert message */
    errorCode = tfSslBuildSendAlert(connStatePtr,
                                    TM_SSL_ALTLEV_WARNING,
                                    TM_SSL_ALT_CLOSE_NOTIFY);
/* we should free any peer certificate we obtained from the handshake */
    tfSslCleanupPeerCerts(connStatePtr);

/* Don't Remove this connection from the session's active connect list
 * because we remove it here, if user tries to call tfSslSessionUserClose
 * later, user's active connection has no way to know that the session
 * is already gone
 */
sslCloseExit:
    return errorCode;
}

/* free any peer certificates stored on the TLS client connection */
static void tfSslCleanupPeerCerts(ttSslConnectStatePtr connStatePtr)
{
    ttPkiCertListPtr certListPtr;
    ttPkiCertListPtr sslNextCertListPtr;
    ttPkiCertListPtr pkiNextCertListPtr;
    ttPkiCertListPtr pkiNextNextCertListPtr;

    certListPtr = connStatePtr->scsPeerCertListPtr;
    connStatePtr->scsPeerCertListPtr = (ttPkiCertListPtr)0;

    if(certListPtr)
    {
        tm_call_lock_wait(&(tm_context(tvPkiLockEntry)));
        while(certListPtr)
        {
            certListPtr->certCheckOut--;
            sslNextCertListPtr = certListPtr->certSslNextPtr;
            if (certListPtr->certCheckOut == 0)
            {
                tfPkiCertCleanup(certListPtr);
                pkiNextCertListPtr =
                    ((ttPkiGlobalPtr)tm_context(tvPkiPtr))->pkiCertListHdrPtr;
/* remove this certificate from the PKI certificate list */
                if (certListPtr == pkiNextCertListPtr)
                {
/* remove it from the head of the list, if it is at the head */
                    ((ttPkiGlobalPtr)tm_context(tvPkiPtr))->pkiCertListHdrPtr
                        = certListPtr->certLinkNextPtr;
                }
                else
                {
                    tm_assert(tfSslCleanupPeerCerts,
                              (pkiNextCertListPtr != (ttPkiCertListPtr)0));
/* search for it after the head of the list, and unlink it from the list */
                    pkiNextNextCertListPtr =
                        pkiNextCertListPtr->certLinkNextPtr;
                    while(pkiNextNextCertListPtr)
                    {
                        if (certListPtr == pkiNextNextCertListPtr)
                        {
/* we found a match - unlink it from the list */
                            pkiNextCertListPtr->certLinkNextPtr =
                                certListPtr->certLinkNextPtr;
                            break;
                        }

/* advance to the next certificate in the PKI certificate list */
                        pkiNextCertListPtr =
                            pkiNextNextCertListPtr;
                        pkiNextNextCertListPtr =
                            pkiNextCertListPtr->certLinkNextPtr;
                    }
                    tm_assert(tfSslCleanupPeerCerts,
                              (pkiNextNextCertListPtr != (ttPkiCertListPtr)0));
                }
                tm_free_raw_buffer(certListPtr);
            }
            certListPtr = sslNextCertListPtr;
        }
        tm_call_unlock( &(tm_context(tvPkiLockEntry)));
    }

    return;
}

/*DESCRIPTION:
 * This function is called to store the sslSessionPtr into global variable.
 * we look the ssl global variable to find an empty slot to store this
 * pointer and return the index number for the slot.
 * RETURN:
 * The index of session pointer. If -1, error happens, the slots are full
 */
int tfSslPutSessionIntoGlobal(ttSslSessionPtr sslSessionPtr)
{
    ttSslGlobalEntryPtr   sslGlobalPtr;
    int                   currentIndex;

    tm_call_lock_wait(&tm_context(tvSslTlsLockEntry));
    sslGlobalPtr = (ttSslGlobalEntryPtr)tm_context(tvSslTlsPtr);
    currentIndex = sslGlobalPtr->sgCurrentSession;
/* find an empty slot to store sslSessionPtr. currentIndex is our next try,
 * however, it may have been occupied before, if so, we try next
 */
    while (sslGlobalPtr->sgSessionArray[currentIndex] != (ttSslSessionPtr)0)
    {
        currentIndex = (currentIndex + 1) % sslGlobalPtr->sgMaxSession;
        if ((tt16Bit)currentIndex == sslGlobalPtr->sgCurrentSession)
        {
/* all slots are full */
            currentIndex = -1;
            break;
        }
    }
    if (currentIndex != -1)
    {
        sslGlobalPtr->sgSessionArray[currentIndex] = sslSessionPtr;
        sslSessionPtr->ssOwnerCount = 1; /* in the array */
        sslSessionPtr->ssNumber = (tt16Bit)currentIndex;
        sslGlobalPtr->sgCurrentSession = (tt16Bit)((currentIndex + 1)
                       % sslGlobalPtr->sgMaxSession);
    }
    tm_call_unlock(&tm_context(tvSslTlsLockEntry));

    return currentIndex;
}


/* If a connection is going to set SSL on and off and on and off, we need
 * to free some stuff of previous SSL connection before we create our new SSL
 * connection
 */
void tfSslPrefreeConnState(ttSslConnectStatePtr connStatePtr)
{
    ttSslSessionPtr sslSessionPtr;

/* in case this connStatePtr is reused later, we need to reset something*/
    connStatePtr->scsRunFlags = 0;
    sslSessionPtr = connStatePtr->scsSessionPtr;
    if (sslSessionPtr != (ttSslSessionPtr)0)
    {
        tm_call_lock_wait(&sslSessionPtr->ssLockEntry);
        if (connStatePtr->scsConnectNode.nodeNextPtr != (ttNodePtr)0)
        {
/* If not already removed */
            tfListRemove(&sslSessionPtr->ssActiveConnectList,
                        (&connStatePtr->scsConnectNode));
        }
        connStatePtr->scsSessionPtr = (ttSslSessionPtr)0;
/*
 * Decrease ownership count by 1, because we no longer have the back pointer,
 * and unlock.
 */
        tfSslSessionReturn(sslSessionPtr);
    }
#ifdef TM_USE_SSL_CLIENT
    if (connStatePtr->scsServerCNPtr)
    {
        tm_free_raw_buffer(connStatePtr->scsServerCNPtr);
        connStatePtr->scsServerCNPtr = (char TM_FAR *)0;
    }
#endif /* TM_USE_SSL_CLIENT */
    if (connStatePtr->scsMd5CtxPtr)
    {
        tm_free_raw_buffer(connStatePtr->scsMd5CtxPtr);
        connStatePtr->scsMd5CtxPtr = (ttMd5CtxPtr)0;
    }

    if (connStatePtr->scsSha1CtxPtr)
    {
        tm_free_raw_buffer(connStatePtr->scsSha1CtxPtr);
        connStatePtr->scsSha1CtxPtr = (ttSha1CtxPtr)0;
    }
#ifdef TM_USE_SSL_VERSION_33
    if (connStatePtr->scsSha256CtxPtr)
    {
        tm_free_raw_buffer(connStatePtr->scsSha256CtxPtr);
        connStatePtr->scsSha256CtxPtr = (ttSha256CtxPtr)0;
    }
#endif /* TM_USE_SSL_VERSION_33 */

    if (connStatePtr->scsExportKeyPtr)
    {
        tm_free_raw_buffer(connStatePtr->scsExportKeyPtr);
        connStatePtr->scsExportKeyPtr = (tt8BitPtr)0;
    }
    if (connStatePtr->scsExportKeyBackupPtr)
    {
        tm_free_raw_buffer(connStatePtr->scsExportKeyBackupPtr);
        connStatePtr->scsExportKeyBackupPtr = (tt8BitPtr)0;
    }
    if (connStatePtr->scsKeyBlockPtr)
    {
        tfEspClearSchedule(connStatePtr->scsDecodeCryptoKey.keySchedulePtr);
        connStatePtr->scsDecodeCryptoKey.keySchedulePtr = (ttVoidPtr)0;
        tfEspClearSchedule(connStatePtr->scsEncodeCryptoKey.keySchedulePtr);
        connStatePtr->scsEncodeCryptoKey.keySchedulePtr = (ttVoidPtr)0;
        tfAhClearContext(connStatePtr->scsEncodeMacKey.keyContextPtr);
        connStatePtr->scsEncodeMacKey.keyContextPtr = (ttVoidPtr)0;
        tfAhClearContext(connStatePtr->scsDecodeMacKey.keyContextPtr);
        connStatePtr->scsDecodeMacKey.keyContextPtr = (ttVoidPtr)0;
        tm_free_raw_buffer(connStatePtr->scsKeyBlockPtr);
        connStatePtr->scsKeyBlockPtr = (tt8BitPtr)0;
    }
    if (connStatePtr->scsKeyBlockBackupPtr)
    {
        tm_free_raw_buffer(connStatePtr->scsKeyBlockBackupPtr);
        connStatePtr->scsKeyBlockBackupPtr = (tt8BitPtr)0;
    }
    return;
}

/*
 * Server to find if we have a cache given session id. This is used when
 * client tries to resume a connection. If we do find a cache, we return
 * it in cachePtrPtr. If no cache found, we deny the resume request,
 * instead, we offer a new session id. The client should go ahead as
 * if no resuming request has been offered
 */
int tfSslFreeCache( ttSslSessionPtr               sslSessionPtr,
                    tt8BitPtr                     sessionIdPtr,
                    tt8Bit                        sessionIdLen)
{
    ttSslCacheElementPtr   cachePtr;
    unsigned int           i;
    int                    errorCode;
    int                    isEqual;

    errorCode = TM_EINVAL;
    cachePtr = (ttSslCacheElementPtr)0;

    if ( (sslSessionPtr == (ttSslSessionPtr)0) ||
         (sslSessionPtr->ssTotalCache == 0)    ||
         (sessionIdLen == 0)
       )
    {
        goto sslFreeCacheExit;
    }
/* Lock the session */
    tm_call_lock_wait(&sslSessionPtr->ssLockEntry);
    for (i=0; i < sslSessionPtr->ssTotalCache; i++)
    {
        cachePtr = (ttSslCacheElementPtr)(sslSessionPtr->ssConnectCache[i]);
        if (cachePtr && (sessionIdLen == cachePtr->sceSessionIdLength))
        {
            isEqual = tm_memcmp(cachePtr->sceSessionId,
                                sessionIdPtr,
                                sessionIdLen);
            if(!isEqual)
            {
                errorCode = TM_ENOERROR;
                if (!(tm_16bit_one_bit_set(sslSessionPtr->ssOption,
                                           TM_SSL_OPT_CLIENT_CACHEID)))
                {
                    tm_free_raw_buffer((ttRawBufferPtr)cachePtr);
                    sslSessionPtr->ssConnectCache[i] = (ttSslCacheElementPtr)0;
                }
                else
                {
                    tm_16bit_clr_bit( sslSessionPtr->ssOption,
                                      TM_SSL_OPT_CLIENT_CACHEID);
                }
                break;
            }
        }
    }
/* Unlock session */
    tm_call_unlock(&sslSessionPtr->ssLockEntry);
sslFreeCacheExit:
    return errorCode;
}


/*
 * This function is called to free all raw buffers allocated in a SSL
 * state. It is called by:
 * 1.  TCP tfTcpCompleteClose. Note that, if socket is not closed,
 *     the SSL buffer won't be freed (tcpSslFlags = 0)
 * or
 * 2.  tfSslSessionCloseConnection() itself called by either:
 *     . tfSslSessionUserClose() (tcpSslFlags != 0)
 *     or
 *     . tfSslUninitialize() (tcpSslFlags == 0)
 *
 * . If tcpSslFlags is 0, we reset pointers between TCP vector and connStatePtr
 *   we reset the tcpsSslFlags, and free the connStatePtr.
 *
 * . else (tcpSslFlags is not 0) (called from tfSslSessionUserClose()), then
 *   we keep the CLIENT or SERVER flag on tcpsSslFlags, do not reset the
 *   pointers, and do not free the connStatePtr. It could be re-used later on
 *   with a new session.
 *
 * Called with non null connStatePtr, and non null tcpVectPtr.
 */
void tfSslClearConnState(ttSslConnectStatePtr    connStatePtr,
                         ttTcpVectPtr            tcpVectPtr,
                         int                     tcpSslFlags)
{
    ttSslSessionPtr         sslSessionPtr;
#ifdef TM_PUBKEY_USE_RSA
    ttGeneralRsaPtr         rsaPtr;
#endif /* TM_PUBKEY_USE_RSA */

    tm_trace_func_entry(tfSslClearConnState);

    sslSessionPtr = connStatePtr->scsSessionPtr;
/*
 * We should free any peer certificate we obtained from the handshake (in
 * case tfSslClose() did not finish()
 */
    tfSslCleanupPeerCerts(connStatePtr);
#ifdef TM_SSL_USE_EPHEMERAL_DH
/* when using ephemeral d-h exchange, we need to finish
 * one Diffie-Hellman calculation.
 */
    if (connStatePtr->scsDHInfoPtr)
    {
        if (connStatePtr->scsDHInfoPtr->dhGxiPtr)
        {
            tm_free_raw_buffer(connStatePtr->scsDHInfoPtr->dhGxiPtr);
        }
        if (connStatePtr->scsDHInfoPtr->dhGxrPtr)
        {
            tm_free_raw_buffer(connStatePtr->scsDHInfoPtr->dhGxrPtr);
        }
        if (connStatePtr->scsDHInfoPtr->dhGxyPtr)
        {
            tm_free_raw_buffer(connStatePtr->scsDHInfoPtr->dhGxyPtr);
        }
        if (connStatePtr->scsDHInfoPtr->dhPrimePtr)
        {
            tm_free_raw_buffer(connStatePtr->scsDHInfoPtr->dhPrimePtr);
        }
        if (connStatePtr->scsDHInfoPtr->dhSecretPtr)
        {
            tm_free_raw_buffer(connStatePtr->scsDHInfoPtr->dhSecretPtr);
        }

        tm_free_raw_buffer(connStatePtr->scsDHInfoPtr);
        connStatePtr->scsDHInfoPtr = (ttDiffieHellmanPtr)0;
    }
#endif /* TM_SSL_USE_EPHEMERAL_DH */

#ifdef TM_PUBKEY_USE_RSA
    rsaPtr = connStatePtr->scsEphemRsaPtr;
    if (rsaPtr)
    {
        if (rsaPtr->rsaN)
        {
            if (rsaPtr->rsaN->asn1Data)
            {
                tm_free_raw_buffer(rsaPtr->rsaN->asn1Data);
            }
            tm_free_raw_buffer(rsaPtr->rsaN);
        }
        if (rsaPtr->rsaE)
        {
            if (rsaPtr->rsaE->asn1Data)
            {
                tm_free_raw_buffer(rsaPtr->rsaE->asn1Data);
            }
            tm_free_raw_buffer(rsaPtr->rsaE);
        }
        tm_free_raw_buffer(rsaPtr);
        connStatePtr->scsEphemRsaPtr = (ttGeneralRsaPtr)0;
    }
#endif /* TM_PUBKEY_USE_RSA */

    if (connStatePtr->scsCipherPtr != (ttSslCipherSuiteConstPtr)0)
    {
        if ((connStatePtr->scsCipherPtr->csKeyExchange !=
            TM_SSL_KEXCH_DHE) && 
            (connStatePtr->scsCipherPtr->csKeyExchange !=
            TM_SSL_KEXCH_DHEEXPORT1024))
        {
            if (connStatePtr->scsPreMasterPtr)
            {
                tm_free_raw_buffer(connStatePtr->scsPreMasterPtr);
                connStatePtr->scsPreMasterPtr = (tt8BitPtr)0;
            }
            connStatePtr->scsPreMasterLen = 0;
        }
    }

    if (connStatePtr->scsXmitPacketPtr != (ttPacketPtr)0)
    {
        tfFreePacket(connStatePtr->scsXmitPacketPtr, TM_SOCKET_LOCKED);
        connStatePtr->scsXmitPacketPtr = (ttPacketPtr)0;
    }

#ifdef TM_USE_SSL_CLIENT
    if (connStatePtr->scsServerCNPtr)
    {
        tm_free_raw_buffer(connStatePtr->scsServerCNPtr);
        connStatePtr->scsServerCNPtr = (char TM_FAR *) 0;
    }
#endif /* TM_USE_SSL_CLIENT */

    if (connStatePtr->scsKeyBlockPtr)
    {
        tfEspClearSchedule(connStatePtr->scsDecodeCryptoKey.keySchedulePtr);
        connStatePtr->scsDecodeCryptoKey.keySchedulePtr = (ttVoidPtr)0;
        tfEspClearSchedule(connStatePtr->scsEncodeCryptoKey.keySchedulePtr);
        connStatePtr->scsEncodeCryptoKey.keySchedulePtr = (ttVoidPtr)0;
        tfAhClearContext(connStatePtr->scsEncodeMacKey.keyContextPtr);
        connStatePtr->scsEncodeMacKey.keyContextPtr = (ttVoidPtr)0;
        tfAhClearContext(connStatePtr->scsDecodeMacKey.keyContextPtr);
        connStatePtr->scsDecodeMacKey.keyContextPtr = (ttVoidPtr)0;

        tm_free_raw_buffer(connStatePtr->scsKeyBlockPtr);
        connStatePtr->scsKeyBlockPtr = (tt8BitPtr)0;
        connStatePtr->scsEncodeMacKey.keyDataPtr = (tt8BitPtr)0;
        connStatePtr->scsDecodeMacKey.keyDataPtr = (tt8BitPtr)0;
        connStatePtr->scsEncodeCryptoKey.keyDataPtr = (tt8BitPtr)0;
        connStatePtr->scsDecodeCryptoKey.keyDataPtr = (tt8BitPtr)0;
        connStatePtr->scsEncodeIvPtr = (tt8BitPtr)0;
        connStatePtr->scsDecodeIvPtr = (tt8BitPtr)0;

    }

    if (connStatePtr->scsKeyBlockBackupPtr)
    {
        tm_free_raw_buffer(connStatePtr->scsKeyBlockBackupPtr);
        connStatePtr->scsKeyBlockBackupPtr = (tt8BitPtr)0;
    }

    if (connStatePtr->scsExportKeyPtr)
    {
/* decode key is pointing to the same buffer block */
        tm_free_raw_buffer(connStatePtr->scsExportKeyPtr);
        connStatePtr->scsExportKeyPtr = (tt8BitPtr)0;
    }

    if (connStatePtr->scsExportKeyBackupPtr)
    {
/* decode key is pointing to the same buffer block */
        tm_free_raw_buffer(connStatePtr->scsExportKeyBackupPtr);
        connStatePtr->scsExportKeyBackupPtr = (tt8BitPtr)0;
    }
#ifdef TM_USE_SSL_VERSION_33
    if (connStatePtr->scsSha256CtxPtr)
    {
        tm_free_raw_buffer(connStatePtr->scsSha256CtxPtr);
        connStatePtr->scsSha256CtxPtr = (ttSha256CtxPtr)0;
    }
#endif /* TM_USE_SSL_VERSION_33 */
    if (connStatePtr->scsSha1CtxPtr)
    {
        tm_free_raw_buffer(connStatePtr->scsSha1CtxPtr);
        connStatePtr->scsSha1CtxPtr = (ttSha1CtxPtr)0;
    }
    if (connStatePtr->scsMd5CtxPtr)
    {
        tm_free_raw_buffer(connStatePtr->scsMd5CtxPtr);
        connStatePtr->scsMd5CtxPtr = (ttMd5CtxPtr)0;
    }
    if (connStatePtr->scsReadPktPtr)
    {
/* we still have something in recv buffer */
        tfFreePacket(connStatePtr->scsReadPktPtr, TM_SOCKET_LOCKED);
        connStatePtr->scsReadPktPtr = (ttPacketPtr)0;
    }

    if (connStatePtr->scsWritePktPtr)
    {
/* we still have something in send buffer */
        tfFreePacket(connStatePtr->scsWritePktPtr, TM_SOCKET_LOCKED);
        connStatePtr->scsWritePktPtr = (ttPacketPtr)0;
    }
/* don't free connStatePtr, this function is just clear connStatePtr,
 * We need to keep the Node information
 */
/* if session is not resumable, we should remove it from the cache
 * if already cached
 */
    if (connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_IMCOMP_CLOSE)
    {
        (void)tfSslFreeCache(sslSessionPtr,
                             connStatePtr->scsSessionId,
                             connStatePtr->scsSessionIdLength);
    }
/* clear any session information */
/* Lock the session */
    if (sslSessionPtr != (ttSslSessionPtr)0)
    {
        tm_call_lock_wait(&sslSessionPtr->ssLockEntry);
        connStatePtr->scsSessionPtr = (ttSslSessionPtr)0;
/* Decrease ownership count by 1, because we no longer have the back pointer */
        tfSslSessionReturn(sslSessionPtr);
    }
    if (tcpSslFlags == 0)
    {
/* because this connection state is also stored in its corresponding
 * SSL session. You also have to let the session know that this connection
 * is already closed by calling tfListRemove (done before this call either
 * in tfSslSessionCloseConnection() or tfTcpCompleteClose().)
 */
/* Tcp Vector and SSL connect state vector part company */
        tcpVectPtr->tcpsSslConnStatePtr = (ttVoidPtr)0;
        tcpVectPtr->tcpsSslFlags = 0; /* clear all SSL flags */
/* Has to be last: Cannot access connStatePtr after this */
        tm_free_raw_buffer(connStatePtr);
    }
    else
    {
/*
 * We need to partially clear the tcpsSslFlags, i.e. don't reset client|server
 * | User handshake process option flags, and we need to keep the connStatePtr
 * around.
 * Scenario: When user calls tfSslSessionUserClose, and sets the
 * socket to a new session, we don't need to create an SSL connection
 * state again, and we don't need to set SSL client/server again either,
 * because this information has not been changed even if we have modified
 * the session ID.
 */
        tcpVectPtr->tcpsSslFlags &= TM_SSLF_CLTSRV_MASK;
    }
    tm_trace_func_exit(tfSslClearConnState);
    return;
}

void tfSslConnectNodeRemove(ttSslConnectStatePtr   connStatePtr)
{
    ttSslSessionPtr sslSessionPtr;

    sslSessionPtr = connStatePtr->scsSessionPtr;
    if (sslSessionPtr != (ttSslSessionPtr)0)
    {
/* Lock the session */
        tm_call_lock_wait(&sslSessionPtr->ssLockEntry);
        if (connStatePtr->scsConnectNode.nodeNextPtr != (ttNodePtr)0)
        {
/* If not already removed */
            tfListRemove(&sslSessionPtr->ssActiveConnectList,
                        (&connStatePtr->scsConnectNode));
        }
/* Unlock session */
        tm_call_unlock(&sslSessionPtr->ssLockEntry);
    }
}

/*
 * used by client and server, to cache the connection state information
 * of each SSL connection. Later, if we are in the same session, we are
 * able to resume the connections from this cache. This function is called
 * by SSL state machine function tfSslParseFinished
 * Called with session lock on.
 * PARAMETER
 * connStatePtr              the SSL state
 * RETURN:
 * TM_ENOERROR              No error
 * TM_ENOBUFS               No buffer available
 */
static int tfSslCacheConnectInfo(ttSslConnectStatePtr connStatePtr)
{
    ttSslSessionPtr          sslSessionPtr;
    ttSslCacheElementPtr     cachePtr;
    int                      errorCode;
    unsigned int             cacheIndex;

    errorCode = TM_ENOERROR;
    sslSessionPtr = connStatePtr->scsSessionPtr;
    cachePtr = tm_get_raw_buffer(sizeof(ttSslCacheElement));
    if (!cachePtr)
    {
        errorCode = TM_ENOBUFS;
        goto cacheConnectInfoExit;
    }
    tm_bzero(cachePtr, sizeof(ttSslCacheElement));
    cachePtr->sceSessionIdLength = connStatePtr->scsSessionIdLength;
    tm_bcopy(connStatePtr->scsSessionId,
                 cachePtr->sceSessionId,
                 connStatePtr->scsSessionIdLength);
    tm_bcopy(connStatePtr->scsMasterSecret,
             cachePtr->sceMasterSecret,
             TM_SSL_MASTER_SECRET_SIZE);
    cachePtr->sceCipherSuite = connStatePtr->scsCipherPtr->csNumber;
    cachePtr->sceCompressMethod = connStatePtr->scsCompressMethod;
    cachePtr->sceVersion = connStatePtr->scsVersion;
/* cache the peer's address information, if we are SSL client */
#ifdef TM_USE_SSL_CLIENT
    tm_bcopy(&connStatePtr->scsTcpVectPtr->tcpsSocketEntry.socPeerIpAddress,
             &cachePtr->scePeerAddr,
             sizeof(ttIpAddress));
    cachePtr->scePeerPort = connStatePtr->scsTcpVectPtr->tcpsSocketEntry.
             socPeerLayer4Port;
#endif /* TM_USE_SSL_CLIENT */
/* we are going to store this to the cache array. If that slot is
 * previously occupied, we need free that one, because it is too old
 */
    cacheIndex = sslSessionPtr->ssCacheIndex;

    if (sslSessionPtr->ssConnectCache[cacheIndex] != (ttSslCacheElementPtr)0)
    {
        tm_free_raw_buffer((ttRawBufferPtr)(sslSessionPtr->
                     ssConnectCache[cacheIndex]))
    }

    sslSessionPtr->ssConnectCache[cacheIndex] = cachePtr;
    cacheIndex++;
    if (cacheIndex >= sslSessionPtr->ssTotalCache)
    {
        cacheIndex = 0;
    }
    sslSessionPtr->ssCacheIndex = cacheIndex;

cacheConnectInfoExit:
    return errorCode;
}

/*
 * DESCRIPTION:
 * tfSslRecordEncodeMac is called to caculate the mac results of a given
 * SSL record data. We will first calculate the hashing value, then padding
 * and update the SSL header fields. (tfSslRecordPacketTransform is called
 * later to do encryption)
 * PARAMETERS:
 * conStateptr              SSL state
 * sslHeaderPtr             Pointer to the SSL header field
 * headPktPtr               Pointer to the head of packet link
 * offset                   How many bytes of data is the SSL record data,
 *                          starting from pktLinkDataPtr, we will compute the
 *                          hashing from here
 * dataLength               The ssl record data length
 * macOutPtr                Where to write the mac results
 * RETURN:
 * TM_ENOERROR              No error
 * Other value              Error happens, the caller will handle it
 */

static int tfSslRecordEncodeMac(ttSslConnectStatePtr     connStatePtr,
                                ttSslRecordHeaderPtr     sslHeaderPtr,
                                ttPacketPtr              headPktPtr,
                                ttPktLen                 offset,
                                ttPktLen                 dataLength,
                                tt8BitPtr                macOutPtr)
{
    int          errorCode;
#ifdef TM_SSL_DEBUG
    ttPacketPtr  origHeadPktPtr;
    ttCharPtr    printBufPtr;
    int          origLength;
    int          printLen;
    int          debugOffset;
#endif

    errorCode = TM_ENOERROR;

#ifdef TM_SSL_DEBUG
    tm_debug_log0("Encode mac key = ");
    tfCryptoDebugPrintf((ttCharPtrPtr)0,
                        connStatePtr->scsEncodeMacKey.keyDataPtr,
                        (connStatePtr->scsEncodeMacKey.keyBits >> 3),
                        (ttIntPtr)0);
    tm_debug_log1("Mac data offset = %d, data = \n", offset);
    debugOffset = 0;
    origHeadPktPtr = headPktPtr;
    origLength = dataLength + offset;
    printBufPtr = (ttCharPtr)0;
    while (origHeadPktPtr && origLength > 0)
    {
        printLen = origLength > (int)origHeadPktPtr->pktLinkDataLength ?
                origHeadPktPtr->pktLinkDataLength : origLength;
        tfCryptoDebugPrintf(&printBufPtr,
                           origHeadPktPtr->pktLinkDataPtr,
                           printLen,
                           &debugOffset);
        origLength -= printLen;
        origHeadPktPtr = origHeadPktPtr->pktLinkNextPtr;
    }
    tfCryptoDebugFlush(&printBufPtr);
#endif /* TM_SSL_DEBUG */


/*2.5.3 Building the MAC and Padding on this linked list */
#ifdef TM_USE_SSL_VERSION_30
    if (connStatePtr->scsVersion == TM_SSL_VERSION_30)
    {
        errorCode = tfSsl30GetRecordMac(
                                   connStatePtr,
                                   sslHeaderPtr,
                                   headPktPtr,
                                   offset,
                                   dataLength,
                                   TM_SSL_ENCODE,
                                   macOutPtr);
     }
#endif /* TM_USE_SSL_VERSION_30 */
#if (defined(TM_USE_SSL_VERSION_31) || defined(TM_USE_SSL_VERSION_33))
    if( (connStatePtr->scsVersion == TM_SSL_VERSION_31)
     ||(connStatePtr->scsVersion == TM_SSL_VERSION_33))
    {
        errorCode = tfTlsGetRecordMac(
                                   connStatePtr,
                                   sslHeaderPtr,
                                   headPktPtr,
                                   offset,
                                   dataLength,
                                   TM_SSL_ENCODE,
                                   macOutPtr);
    }
#endif /* TM_USE_SSL_VERSION_31 || TM_USE_SSL_VERSION_33 */

#if (defined(TM_USE_SSL_VERSION_30) || defined(TM_USE_SSL_VERSION_31)\
  || defined(TM_USE_SSL_VERSION_33))
    if (errorCode != TM_ENOERROR)
    {
        goto sslRecordEncodeMacExit;
    }
#else
#ifndef TM_SSL_DEBUG
    TM_UNREF_IN_ARG(headPktPtr);
    TM_UNREF_IN_ARG(offset);
    TM_UNREF_IN_ARG(dataLength);
#endif /* TM_SSL_DEBUG */
#endif /* TM_USE_SSL_VERSION_30 || TM_USE_SSL_VERSION_31)
       || TM_USE_SSL_VERSION_33 */

    tfSslUpdateRecordAfterMac(connStatePtr, sslHeaderPtr, 0, macOutPtr);

#ifdef TM_SSL_DEBUG
    tm_debug_log0("\nMac+Padding results = \n");
    tfCryptoDebugPrintf((ttCharPtrPtr)0,
                        macOutPtr,
                        ((sslHeaderPtr->srhLengthHigh << 8) |
                         (sslHeaderPtr->srhLengthLow)) - dataLength,
                         (ttIntPtr)0);
#endif /* TM_SSL_DEBUG */

#if (defined(TM_USE_SSL_VERSION_30) || defined(TM_USE_SSL_VERSION_31)\
  || defined(TM_USE_SSL_VERSION_33))
sslRecordEncodeMacExit:
#endif /* TM_USE_SSL_VERSION_30 || TM_USE_SSL_VERSION_31
       || TM_USE_SSL_VERSION_33 */
    return errorCode;
}

/* This function is called to encapsulate user data (linked packet list
 * begins at connStatePtr->scsWritePktPtr) into one or more SSL
 * record(s). Note that, in each SSL record, the user data is at most
 * tcpsSslSendMaxSize bytes. If we have more than tcpsSslSendMaxSize bytes
 * of user application data to encapsulate, we will make more than one
 * SSL record packets. All these SSL record packets are linked together
 * through pktLinkNextPtr, the header of this linked list will be returned
 *
 * RETURN:
 * packetPtr           links the SSL record(s) we just generated
 *                      If it is NULL, no SSL record is generated
 */

static ttPacketPtr tfSslBuildRecordFromUserData(
                          ttTcpVectPtr                   tcpVectPtr,
                          ttSslConnectStatePtr           connStatePtr)
{

    ttPacketPtr               newPacketPtr;   /* each new record will
                                               have one packet */
    ttPacketPtr               headNewPacketPtr;/* return this one */
    ttPacketPtr               prevNewPacketPtr;/* link to previous one */
    ttPacketPtr               headPacketPtr;   /* original packet list head*/
    ttPacketPtr               lastPacketPtr;   /* original packet tail */
    ttPacketPtr               newLastPacketPtr;/* we may need to allocate a
                                            new tail to the original list */
    tt8BitPtr                 macOutPtr;
    ttSslRecordHeaderPtr      sslHeaderPtr;
    ttPktLen                  leftOverBytes;
    int                       errorCode;
    tt16Bit                   bufferLen;
    tt16Bit                   thisRecordSize;
    tt8Bit                    hashSize;
    tt8Bit                    blockSize;
    tt8Bit                    ivSize;
    tt8Bit                    curRecordCount;

    errorCode = TM_ENOERROR;
    hashSize = connStatePtr->scsHashOutputSize;
    blockSize = connStatePtr->scsBlockSize;
    ivSize = (tt8Bit)0;
    newPacketPtr = (ttPacketPtr)0;
    headPacketPtr = (ttPacketPtr)0;
    headNewPacketPtr = (ttPacketPtr)0;
    prevNewPacketPtr = (ttPacketPtr)0;
    curRecordCount = 0;

    if (   (connStatePtr->scsCryptoAlgPtr->ealgName == SADB_EALG_NULL)
#ifdef TM_USE_ARCFOUR
        || (connStatePtr->scsCryptoAlgPtr->ealgName == SADB_EALG_ARCFOUR)
#endif /* TM_USE_ARCFOUR */
        )
    {
        connStatePtr->scsRunFlags |= TM_SSL_RUNFLAG_FRAG_SENT;
    }

#ifdef TM_SSL_TEST_ZERO_SEND
    while (connStatePtr->scsWritePktPtr != (ttPacketPtr)0)
#else /* TM_SSL_TEST_ZERO_SEND */
    while ( tcpVectPtr->tcpsSslSendQBytes > 0)
#endif /* TM_SSL_TEST_ZERO_SEND */
    {
        headPacketPtr = connStatePtr->scsWritePktPtr;
/*
 * Implement countermeasure for CBC IV attack (except for NULL and RC4)
 * Attack #2 in http://www.openssl.org/~bodo/tls-cbc.txt
 * Note that the empty fragment has been known to cause problems
 * with certain implementations (notably MSIE) so we allow the
 * user to use a one-byte version instead. This means that the
 * adversary may still be able to decode that one byte.
 */
        if (   (curRecordCount == 0)
            && ((connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_FRAG_SENT) == 0) )
        {
            if (tm_16bit_one_bit_set(connStatePtr->scsSessionPtr->ssOption,
                                     TM_SSL_OPT_NO_SEND_EMPTY_FRAG))
            {
                thisRecordSize = 1;
            }
            else
            {
                thisRecordSize = 0;
            }
            curRecordCount++;
            connStatePtr->scsRunFlags |= TM_SSL_RUNFLAG_FRAG_SENT;
        }
        else if (tcpVectPtr->tcpsSslSendQBytes
                > (tt32Bit)tcpVectPtr->tcpsSslSendMaxSize)
        {
/* this means we can't encapsulate all send queue bytes into a single
 * one ssl record, we need to form more ssl records later.
 * Set the record size of the current record to be the maxsize.
 */
            if (curRecordCount == 1)
            {
/*
 * Keep the alignment as best we can by sending these sizes:
 *   1-byte
 *   (max - 1) bytes
 *   max bytes
 *   max bytes
 *   ...
 */
                thisRecordSize = tcpVectPtr->tcpsSslSendMaxSize - 1;
                curRecordCount++;
            }
            else
            {
                thisRecordSize = tcpVectPtr->tcpsSslSendMaxSize;
            }
        }
        else
        {
            thisRecordSize = (tt16Bit)tcpVectPtr->tcpsSslSendQBytes;
        }

        tcpVectPtr->tcpsSslSendQBytes -= thisRecordSize;

/* Before the following call, headPacketPtr holds the remaining queued user
 * data, we are going to split it (because our tcpsSslSendMaxSize limitation).
 * We are going to split the remaining queued user data from one linked list
 * to two linked list. The first linked list will have the same head as the
 * original linked list, but has a total chain datalength to be thisRecordSize.
 * A second linked list will begins right there and hold all the remaining
 * user data. At the boarder, a duplicate packet may be necessory to avoid
 * copy.
 * After the spliting, the first linked list will be stored in headPacketPtr,
 * and the second linked list will be stored in connStatePtr->scsWritePktPtr.
 * And later in this while loop, the first linked list will be used to build
 * an SSL record packet pointed by newPacketPtr.
 * Loop until we encapsulate all user data, link all these ssl record packet(s),
 * and return the head headNewPacketPtr
 */
        tfSslCutQueuedPacketList(headPacketPtr,
                                 thisRecordSize,
                                &headPacketPtr,
                                &connStatePtr->scsWritePktPtr);
        if (!connStatePtr->scsWritePktPtr)
        {
            connStatePtr->scsWritePktTailPtr = (ttPacketPtr)0;
        }

#ifdef TM_USE_SSL_VERSION_33
        if((connStatePtr->scsVersion == TM_SSL_VERSION_33)
         &&((connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_ARCFOUR)
          &&(connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_NULL)))
        {
            ivSize = connStatePtr->scsCryptoAlgPtr->ealgBlockSize;
        }
#endif /* TM_USE_SSL_VERSION_33 */

        newPacketPtr = tfGetSharedBuffer(0,
                          thisRecordSize +
                          TM_SSL_RECORD_HEADER_SIZE +
                          blockSize +
                          hashSize +
                          ivSize,
                          0);
        if (!newPacketPtr)
        {
            goto sslBuildRecordUserDataExit;
        }
        sslHeaderPtr = (ttSslRecordHeaderPtr)newPacketPtr->pktLinkDataPtr;
        sslHeaderPtr->srhType = TM_SSL_TYPE_APPLICATION;
        sslHeaderPtr->srhVersionMajor = TM_SSL_VERSION_MAJOR;
#ifdef TM_USE_SSL_VERSION_33
        if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
        {
            sslHeaderPtr->srhVersionMinor = TM_SSL_VERSION_MINOR3;
        }
        else
#endif /* TM_USE_SSL_VERSION_33 */
        {
            sslHeaderPtr->srhVersionMinor = (tt8Bit)
                      (connStatePtr->scsVersion >> 1);
        }
        sslHeaderPtr->srhLengthHigh = (tt8Bit)
            (thisRecordSize >> 8);
        sslHeaderPtr->srhLengthLow = (tt8Bit)
            (thisRecordSize & 0xff);
        newPacketPtr->pktLinkDataLength = TM_SSL_RECORD_HEADER_SIZE;

        if(ivSize)
        {
/* Insert IV field in case of CBC. */
            tfGetRandomBytes(connStatePtr->scsEncodeIvPtr, ivSize);
            tm_bcopy(connStatePtr->scsEncodeIvPtr,
                     (tt8BitPtr)sslHeaderPtr
                     + TM_SSL_RECORD_HEADER_SIZE,
                     ivSize);
            newPacketPtr->pktLinkDataLength += ivSize;
#ifdef TM_SSL_DEBUG
/* print the key blocks */
            tm_debug_log0("\nRandom IV = \n");
            tfCryptoDebugPrintf((ttCharPtrPtr)0,
                                connStatePtr->scsEncodeIvPtr,
                                ivSize ,
                                (ttIntPtr)0);
#endif /* TM_SSL_DEBUG */
        }
/*2.5.2 Test if we have enough tail space to hold the original mac and
 * padding. And then we encrypted from the original packet lists to this new
 * packet pointer
 */
        lastPacketPtr = headPacketPtr;
        while (lastPacketPtr->pktLinkNextPtr)
        {
            lastPacketPtr = lastPacketPtr->pktLinkNextPtr;
        }
        if (lastPacketPtr->pktSharedDataPtr->dataOwnerCount == 1)
        {
            leftOverBytes = (ttPktLen)((ttPktLen)
                tm_byte_count(lastPacketPtr->pktSharedDataPtr->dataBufLastPtr
                - lastPacketPtr->pktLinkDataPtr )
                - lastPacketPtr->pktLinkDataLength);
        }
        else
        {
            leftOverBytes = 0;
        }

        if (leftOverBytes < (ttPktLen)(blockSize + hashSize))
        {
            newLastPacketPtr = tfGetSharedBuffer(0,
                (blockSize + hashSize),
                0);
            if (!newLastPacketPtr)
            {
                goto sslBuildRecordUserDataExit;
            }
            newLastPacketPtr->pktLinkDataLength = 0;
            lastPacketPtr->pktLinkNextPtr = newLastPacketPtr;
            lastPacketPtr = newLastPacketPtr;
        }
/* we are going to put AUTH (MAC data) in this packet */
        lastPacketPtr->pktFlags2 |=  TM_PF2_AUTH_ICVDATA_AT_END;
        macOutPtr = lastPacketPtr->pktLinkDataPtr +
            lastPacketPtr->pktLinkDataLength;
/*2.5.3 Building the MAC and Padding on this linked list */
        errorCode = tfSslRecordEncodeMac
            (connStatePtr,
            sslHeaderPtr,
            headPacketPtr,
            0,
            thisRecordSize,
            macOutPtr);
        if (errorCode != TM_ENOERROR)
        {
            goto sslBuildRecordUserDataExit;
        }

        bufferLen =(tt16Bit)((sslHeaderPtr->srhLengthHigh << 8) |
            (sslHeaderPtr->srhLengthLow));
        lastPacketPtr->pktLinkDataLength +=
            (bufferLen - thisRecordSize);
/*2.5.4 Encrypt the linked list */
        errorCode = tfSslRecordPacketTransform(
            connStatePtr,
            headPacketPtr,
            0,
            newPacketPtr,
            (tt16Bit)bufferLen,
            TM_SSL_ENCODE);
        headPacketPtr = (ttPacketPtr)0;
        if (errorCode != TM_ENOERROR)
        {
            goto sslBuildRecordUserDataExit;
        }

        if (headNewPacketPtr == (ttPacketPtr)0)
        {
            headNewPacketPtr = newPacketPtr;
            headNewPacketPtr->pktChainDataLength =
                     headNewPacketPtr->pktLinkDataLength;
        }
        else
        {
            headNewPacketPtr->pktChainDataLength +=
                newPacketPtr->pktLinkDataLength;
            prevNewPacketPtr->pktLinkNextPtr = newPacketPtr;
        }

        prevNewPacketPtr = newPacketPtr;
    }
    tcpVectPtr->tcpsFlags2 |= TM_TCPF2_SEND_DATA;

sslBuildRecordUserDataExit:
    if (headNewPacketPtr == (ttPacketPtr)0 &&
           newPacketPtr != (ttPacketPtr)0)
    {
        tfFreePacket(newPacketPtr, TM_SOCKET_LOCKED);
    }
    if (errorCode != TM_ENOERROR && headNewPacketPtr)
    {
        tfFreePacket(headNewPacketPtr, TM_SOCKET_LOCKED);
        headNewPacketPtr = (ttPacketPtr)0;
    }
    if (headPacketPtr != (ttPacketPtr)0)
    {
        tfFreePacket(headPacketPtr, TM_SOCKET_LOCKED);
    }
    return headNewPacketPtr;
}

/* flags should indicate that the data is application data or SSL
 * protocol data.
 * 1) If the caller is not application data, the caller is responsible
 * to encapsulate the SSL record before calling us. Inside this function,
 * if it is not application data call, we will append this packet
 * to our socket send queue immediately.
 * 2) If the caller indicates this is application data, we need check
 * if we need queue the application data in SSL layer (until we trigger
 * ssl send).  Anyway, if user data sets MSG_OOB or socket sets NO_DELAY,
 * we will not queue in SSL layer, instead, we will encapsulate the data
 * into an SSL record and append it to socket send queue immediately.
 * 3) If the caller indicates application data, and neither MSG_OOB nor NO_DELAY
 * is set, and we don't reach the SSL send threshold yet, we need queue
 * the application data into SSL layer. Test the previous buffer size
 * to see if we can append it there first.
 *PARAMETER:
 * socketEntryPtr                   Socket entry pointer
 * connStatePtr                      SSL state
 * packetPtrPtr                     pass in the packet pointer, if later
 *                                  we build a new packet pointer, return
 *                                  it here
 *bufferLenPtr                      Pass in the old length, return the new
 *                                  length
 * flags:
 * #define TM_SSLSF_APPLICATION                   0x01
 * #define TM_SSLSF_MSG_OOB                       0x02
 * #define TM_SSLSF_PKT_USER_OWNS                 0x04
 * RETURN:
 * TM_ENOERROR
 * TM_EINPROGRESS
 * or TM_EINVAL
 */
int tfSslSend(ttSocketEntryPtr        socketEntryPtr,
              ttSslConnectStatePtr    connStatePtr,
              ttPacketPtrPtr          packetPtrPtr,
              ttPktLen TM_FAR *       bufferLenPtr,
              int                     flags)
{
/* for non-application data, we have already constructed the SSL
 * header, and if encryption is required, we have already encrypted
 * the packet. All we need to do here, is to append this packet to
 * tcpVectPtr and set READY_TO_SEND
 */
    ttTcpVectPtr          tcpVectPtr;
    ttPacketPtr           newPacketPtr;
    ttPacketPtr           lastPacketPtr;
    ttPacketPtr           packetPtr;
    ttPktLen              bufferLen;
    ttPktLen              tempLen;
    int                   errorCode;
    tt8Bit                needQueue;


    needQueue = TM_8BIT_NO;
    errorCode = TM_ENOERROR;
    newPacketPtr = (ttPacketPtr)0;
    packetPtr = *packetPtrPtr;
    bufferLen = *bufferLenPtr;
    tcpVectPtr = (ttTcpVectPtr)socketEntryPtr;

    if ((flags & TM_SSLSF_APPLICATION) == 0)
    {
/* NOTE: SSL record should be constructed before this call if
 * TM_SSL_SENDFLAG_APPLICATION is not set. If user is calling this
 * after constructing the queued bytes (say about 20000 bytes), we
 * must end up in linked packet pointer. Each of them is a full SSL
 * record.
 */

/*1. we reach here because we call tfSslSend from the receiving path, for
 * example, we received an SSL handshake message and needs reply. Or
 * after we establish SSL connection (receive the last SSL finish message
 * and we check our queued buffer and found that we need send user's
 * queued data.
 */
        socketEntryPtr->socSendQueueBytes += bufferLen;
/* Queue the data */
        if (socketEntryPtr->socSendQueueNextPtr == TM_PACKET_NULL_PTR)
        {
/* This is the only packet on the send queue */
            socketEntryPtr->socSendQueueNextPtr = packetPtr;
        }
        else
        {
/* Put it at the end of the list */
            socketEntryPtr->socSendQueueLastPtr->pktLinkNextPtr =
                (ttVoidPtr)(packetPtr);
        }

        if (packetPtr->pktLinkNextPtr != (ttPacketPtr)0)
        {
            lastPacketPtr = packetPtr->pktLinkNextPtr;
            while (lastPacketPtr->pktLinkNextPtr != (ttPacketPtr)0)
            {
                lastPacketPtr = lastPacketPtr->pktLinkNextPtr;
            }
            socketEntryPtr->socSendQueueLastPtr = lastPacketPtr;
        }
        else
        {
            socketEntryPtr->socSendQueueLastPtr = packetPtr;
        }
/* Set TM_TCPF2_ACK to force immediate TCP send */
        tcpVectPtr->tcpsFlags2 |= (TM_TCPF2_SEND_DATA | TM_TCPF2_ACK);
/*
 * Also set TM_SSLF_TCP_NO_DELAY to force immediate TCP send (in case data
 * fits in more than one segment)
 */
        tcpVectPtr->tcpsSslFlags |= TM_SSLF_TCP_NO_DELAY;
    }
    else
    {
/*2 NOTE: user may call using scattered zerocopy buffer. We eventually need
 * to support that option. So, here, we can't assume that packetPtr has no
 * extra links
 */

/* SSL encapsulation comes here, don't call tfTcpSendPacket, we expect the
 * caller to be tfSocketSend exclusively, so tfSocketSend is responsible to
 * call tfTcpSendPacket if we returns TM_ENOERROR.
 */
        if ((tcpVectPtr->tcpsSslFlags & TM_SSLF_ESTABLISHED) == 0)
        {
/*2.1. If SSL connection is not established, we queue everything up to our
 * queue limit
 */
            needQueue = TM_8BIT_YES;
        }
        else
        {
/*2.2 If we don't set OOB and we don't set NOWAIT and our SSL send threshold
 *   has not been reached, we queue.
 */
            if (tm_16bit_bits_not_set(packetPtr->pktFlags, MSG_OOB) &&
                 tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags, TCP_NODELAY) &&
                 (tcpVectPtr->tcpsSslSendQBytes + (tt32Bit)bufferLen) <
                 (tt32Bit)tcpVectPtr->tcpsSslSendMinSize)
            {
                needQueue = TM_8BIT_YES;
            }
        }
/*2.3. If we do need queue, firstly, we check if we can append. For
 * scattered buffer, we may only be able to append part of the scattered
 * buffer blocks, return the nextpacketPtr. When we append, we are
 * responsible to update tcpsSslSendQBytes inside tfSslSend
 */
        if (needQueue == TM_8BIT_YES)
        {
/* When tfSocketSend gets this error code (TM_EWOULDBLOCK), tfSocketSend
 * knows that the packet is queued in SSL buffer for later transmission.
 * tfSocketSend will just reset this errorCode to TM_ENOERROR, and returns
 * it to user.
 * In case errorCode TM_ENOERROR is returned to tfSocketSend (i.e.
 * needQueue == TM_8BIT_NO), tfSocketSend will update the socSendQueueBytes,
 * socSendQueueNextPtr and socSendQueueLastPtr
 */
            errorCode = TM_EWOULDBLOCK;
/* test if we need append,  (if user uses zero copy send, we don't append*/
            if ((tcpVectPtr->tcpsSslSendQBytes != 0 )
                             && ((flags & TM_SSLSF_PKT_USER_OWNS) == 0)
                             && (bufferLen <= (ttPktLen)
                             ((ttSocketEntryPtr)tcpVectPtr)->
                             socSendAppendThreshold))
            {
/* you try to append everything to previous buffer. You can either append
 * everything or nothing.
 */
                tempLen = tfSendAppend( socketEntryPtr,
                                 (const char *)packetPtr->pktLinkDataPtr,
                                 bufferLen,
#ifdef TM_USE_QOS
                                 packetPtr->pktIpTos,
#endif /* TM_USE_QOS */
                                 TM_SSLF_SEND_ENABLED);
                if ( tempLen != (ttPktLen)0)
                {
/* we are done ! we already append everything. Just return to the caller that
 * errorCode = TM_EWOULDBLOCK */
                    tfFreePacket(packetPtr, TM_SOCKET_LOCKED);
                    bufferLen = 0;
                    goto sslSendExit;
                }
            }
        }

/*2.4. If we don't need queue, or we need queue but we can't append
 * everything to previous packet, we need link the packetPtr to our
 * SSL send queue, and update the first and last queue packetPtr
 */
/* we link this packetPtr to the end of our queue. even if we know we
 * are not queueing the packet, we still link them to our queue list, so
 * that we can more conveniently process it.
 */
        if (!connStatePtr->scsWritePktPtr)
        {
/* if we don't have any user application packet yet, this one (packetPtr)
 * will be put first, otherwise, put packetPtr to scsWritePktTailPtr. From
 * scsWritePktPtr to scsWritePktTailPtr, a linked list is formed for user
 * application data queuing
 */
            connStatePtr->scsWritePktPtr = packetPtr;
        }
        else
        {
            connStatePtr->scsWritePktTailPtr->pktLinkNextPtr = packetPtr;
        }
        lastPacketPtr = packetPtr->pktLinkNextPtr;
        if (lastPacketPtr)
        {
            while (lastPacketPtr->pktLinkNextPtr)
            {
                lastPacketPtr = lastPacketPtr->pktLinkNextPtr;
            }
            connStatePtr->scsWritePktTailPtr = lastPacketPtr;
        }
        else
        {
            connStatePtr->scsWritePktTailPtr = packetPtr;
        }
        tcpVectPtr->tcpsSslSendQBytes += bufferLen;

/*2.5. If we don't need queue, it means that we need build a new ssl record
 * with all data we have. This new ssl record will be saved in newPacketPtr,
 * and bufferLen returns the new bufferLen
 */
        if (needQueue == TM_8BIT_NO)
        {
/*2.5.1 create a new packetPtr to hold the resulting ssl record , put in SSL
 *header, this newPacketPtr could be a chained packet pointer. The reason is
 *we may end up in situation like that our send queue is too big than one
 *ssl record allows*/
            newPacketPtr = tfSslBuildRecordFromUserData
                              (tcpVectPtr,connStatePtr);
            if (newPacketPtr != (ttPacketPtr)0)
            {
/*
 * 3. We have newPacketPtr, which should be a ready-to-send SSL record data,
 * and more importantly, it should have no link extras. A single packet
 * pointer and a single SSL record. We need insert this ready-to-send
 * SSL record to socket send queue. Set TM_TCPF2_SEND_DATA flag so that
 * TCP knows to send it
 */
                bufferLen = newPacketPtr->pktChainDataLength;
                tcpVectPtr->tcpsFlags2 |= TM_TCPF2_SEND_DATA;
            }
            else
            {
                errorCode = TM_ENOBUFS;
            }
        }
    }
sslSendExit:
    *packetPtrPtr = newPacketPtr;
    *bufferLenPtr = bufferLen;
    return errorCode;
}

/*
 * Description
 * tfUseSsl must be called after tfStartTreck(), and before any other
 * SSL function calls. This function allocates memory for SSL global variable.

 * Parameters
 * maximumSession   Maximum sessions user wants to support, must greater or
 * equal to zero. If user passes in zero, default value TM_SSL_NUMBEROF_
 * SESSION_DEFAULT (32) is used.  In Treck SSL, each session maintains its
 * own connect information for the convenience of resumption. You can resume
 * a previous connection only if the current connect attempt and the previous
 * one are in the same SSL session, and your previous connection cache hasn't
 * been overwritten yet. For Treck SSL server, all connections spawned from a
 * single listening socket belongs to one session, you should assign the
 * session number to the listening socket. All accepted sockets will
 * inheritate that session number. For Treck SSL client, user determines which
 * session one connection belongs to. For example, user may decide that any
 * connection to the same SSL-enabled web server uses a single session number,
 * so that when user is trying to connect to the same server again, (s)he may
 * resume the previous connection.
 *
 * Returns
 * Value    Meaning
 * TM_ENOERROR  Success
 * TM_EINVAL    Invalid value is passed in
 * TM_EALREADY  tfUseSsl has been called before
 * TM_ENOBUFS   No more buffers
 */
int tfUseSsl(int       maximumSession)
{
    int         errorCode;

    errorCode = TM_ENOERROR;

    if (maximumSession < 0)
    {
        errorCode = TM_EINVAL;
        goto useSslExit;
    }
    else if (maximumSession == 0
#ifndef TM_INT_LE16
        || ((tt32Bit)maximumSession > TM_UL(0xffff))
#endif /* TM_INT_LE16 */
        )
    {
        maximumSession = TM_SSL_NUMBEROF_SESSION_DEFAULT;
    }

    tm_call_lock_wait(&tm_context(tvSslTlsLockEntry));
    if (tm_context(tvSslTlsPtr))
    {
        errorCode = TM_EALREADY;
    }
    else
    {
        tm_context(tvSslTlsPtr) = tm_get_raw_buffer(sizeof(ttSslGlobalEntry));
        if (!tm_context(tvSslTlsPtr))
        {
            errorCode = TM_ENOBUFS;
        }
        else
        {
            tm_bzero(tm_context(tvSslTlsPtr), sizeof(ttSslGlobalEntry));
            ((ttSslGlobalEntryPtr)tm_context(tvSslTlsPtr))->sgSessionArray
                = (ttSslSessionPtr TM_FAR *) tm_get_raw_buffer(
                    sizeof(ttSslSessionPtr) * (ttPktLen)maximumSession);
            if (((ttSslGlobalEntryPtr)tm_context(tvSslTlsPtr))->
                sgSessionArray == (ttSslSessionPtr TM_FAR *) 0)
            {
                tm_free_raw_buffer(tm_context(tvSslTlsPtr));
                tm_context(tvSslTlsPtr) = (ttVoidPtr)0;
                errorCode = TM_ENOBUFS;
            }
            else
            {
                tm_bzero(((ttSslGlobalEntryPtr)tm_context(tvSslTlsPtr))->
                            sgSessionArray,
                            sizeof(ttSslSessionPtr) * (ttPktLen)maximumSession);
                ((ttSslGlobalEntryPtr)tm_context(tvSslTlsPtr))->
                             sgMaxSession = (tt16Bit)maximumSession;
            }
        }
    }
    tm_call_unlock(&tm_context(tvSslTlsLockEntry));
useSslExit:
    return errorCode;
}


/* Description
 * tfSslUninitialize frees the SSL global variable. If there is any SSL
 * session open, that session will be closed. If there is any connection
 * open for that session, the connection will be closed first.
 */

void tfSslUninitialize(void)
{
    ttSslGlobalEntryPtr     sslGlobalPtr;
    ttSslSessionPtr         sslSessionPtr;
    int                     i;

    tm_call_lock_wait(&tm_context(tvSslTlsLockEntry));
    sslGlobalPtr = tm_context(tvSslTlsPtr);
    tm_context(tvSslTlsPtr) = (ttVoidPtr)0; /* no more user access */
    tm_call_unlock(&tm_context(tvSslTlsLockEntry));

    if (sslGlobalPtr)
    {
        for (i = 0; (tt16Bit)i < sslGlobalPtr->sgMaxSession; i ++)
        {
            sslSessionPtr = sslGlobalPtr->sgSessionArray[i];
/* If you still has any session uncleared, you need to close
 * them
 */
            if (sslSessionPtr)
            {
                tm_call_lock_wait(&sslSessionPtr->ssLockEntry);
/* we need to reset the connection to disable SSL */
                tfSslSessionClose(sslSessionPtr, 0);
/* Out of the array */
                tfSslSessionReturn(sslSessionPtr);
            }
        }
        tm_free_raw_buffer(sslGlobalPtr->sgSessionArray);
        tm_free_raw_buffer(sslGlobalPtr);
    }
}


/*
 * Description:
 * This function is used to set proposals to a certain session.  For a
 * client session, the proposals are used to send in Client Hello message.
 * For a server session, the proposals are used to match Client Hello
 * message in order to determine the final cipher suite to use. The size
 * can't exceed TM_SSL_PROPOSAL_NUMBER (6), which is changeable in compiling
 * time. If user doesn't call this function to set session proposals, the
 * default value is going to use. For exportable version
 * (TM_EXPORT_RESTRICT is defined in trsystem.h), the default cipher
 * suites are:
    TM_TLS_RSA_EPT1K_DES_CBC_SHA
    TM_TLS_RSA_EPT1K_RC4_56_SHA
    TM_TLS_RSA_EXPORT_RC4_40_MD5,
    TM_TLS_RSA_EXPORT_RC2_40_MD5,
    TM_TLS_DHE_DSS_EPT_DES40_CBC_SHA,
    TM_TLS_RSA_EXPORT_DES40_CBC_SHA,
 * For non-exportable version (shippable to US market only), the
 * following default cipher suites are using:
    TM_TLS_RSA_RC4_128_SHA,
    TM_TLS_RSA_RC4_128_MD5,
    TM_TLS_RSA_AES_128_CBC_SHA,
    TM_TLS_DHE_DSS_AES_128_CBC_SHA,
    TM_TLS_RSA_3DES_EDE_CBC_SHA,
    TM_TLS_DHE_DSS_3DES_EDE_CBC_SHA

 * Parameters
 * propArray    Array of integer values. Must be valid numbers of cipher
 *              suites defined by SSL/TLS document. We support the
 *              following value:
 *                TM_TLS_RSA_NULL_MD5:
 *                TM_TLS_RSA_NULL_SHA:
 *                TM_TLS_RSA_NULL_SHA256:
 *                TM_TLS_RSA_EXPORT_RC4_40_MD5:
 *                TM_TLS_RSA_EXPORT_DES40_CBC_SHA:
 *                TM_TLS_DHE_DSS_EPT_DES40_CBC_SHA:
 *                TM_TLS_DHE_RSA_EPT_DES40_CBC_SHA:
 *                TM_TLS_RSA_EPT1K_DES_CBC_SHA
 *                TM_TLS_DHE_DSS_EPT1K_DES_CBC_SHA
 *                TM_TLS_RSA_EPT1K_RC4_56_SHA
 *                TM_TLS_DHE_DSS_EPT1K_RC4_56_SHA
 *                TM_TLS_RSA_RC4_128_MD5:
 *                TM_TLS_RSA_RC4_128_SHA:
 *                TM_TLS_RSA_DES_CBC_SHA:
 *                TM_TLS_RSA_3DES_EDE_CBC_SHA:
 *                TM_TLS_RSA_AES_128_CBC_SHA:
 *                TM_TLS_RSA_AES_128_CBC_SHA256:
 *                TM_TLS_RSA_AES_256_CBC_SHA:
 *                TM_TLS_RSA_AES_256_CBC_SHA256:
 *                TM_TLS_DHE_DSS_DES_CBC_SHA:
 *                TM_TLS_DHE_DSS_3DES_EDE_CBC_SHA:
 *                TM_TLS_DHE_RSA_DES_CBC_SHA:
 *                TM_TLS_DHE_RSA_3DES_EDE_CBC_SHA:
 *                TM_TLS_DHE_DSS_AES_128_CBC_SHA:
 *                TM_TLS_DHE_RSA_AES_128_CBC_SHA:
 *                TM_TLS_DHE_DSS_AES_256_CBC_SHA:
 *                TM_TLS_DHE_RSA_AES_256_CBC_SHA:
 *                TM_TLS_DHE_RSA_AES_128_CBC_SHA256:
 *                TM_TLS_DHE_RSA_AES_256_CBC_SHA256:
 *                TM_TLS_DHE_DSS_AES_256_CBC_SHA256:
 *                TM_TLS_DHE_RSA_AES_256_CBC_SHA256:
 * Size       Number of entries in the array, up to
 *            TM_SSL_PROPOSAL_NUMBER(6). If more than TM_SSL_PROPOSAL_NUMBER
 *            entries are input, we will use the first TM_SSL_PROPOSAL_NUMBER
 *            entries.
 * Returns
 * Value    Meaning
 * TM_ENOERROR  success
 * TM_EINVAL    Invalid cipher suite used.
 */
int tfSslSetSessionProposals(int                    sessionId,
                             int TM_FAR  *          propArray,
                             int                    propSize)

{
    ttSslSessionPtr  sslSessionPtr;
    int              errorCode;
#if (!defined(TM_SSL_REJECT_EXPORTABLE_CIPHER) || !defined(TM_EXPORT_RESTRICT))
#define TM_SSL_TEMP_CASE_EXISTS
    int              i;
    tt16Bit          prop;
#endif /* !TM_SSL_REJECT_EXPORTABLE_CIPHER || !TM_EXPORT_RESTRICT */

/* verify that each proposal is a supported one */
    errorCode = TM_ENOERROR;
    sslSessionPtr = (ttSslSessionPtr)tfSslGetSessionUsingIndex(sessionId);
    if ((sslSessionPtr == (ttSslSessionPtr)0 ) || (propSize <= 0))
    {
        errorCode = TM_EINVAL;
        goto SSL_SET_SESSION_PROPOSALS_UNLOCK;
    }

    if (propSize > TM_SSL_PROPOSAL_NUMBER)
    {
        propSize = TM_SSL_PROPOSAL_NUMBER;
    }

    tm_bzero(sslSessionPtr->ssProposals,
             TM_SSL_PROPOSAL_NUMBER * sizeof(tt16Bit));

#ifndef TM_SSL_TEMP_CASE_EXISTS
    TM_UNREF_IN_ARG(propArray);

    if (propSize > 0)
    {
            /* Proposals provided but NO algorithms are supported! */
            errorCode = TM_EINVAL;
    }
#else /* TM_SSL_TEMP_CASE_EXISTS */
    for (i = 0; i < propSize; i++)
    {
        prop = (tt16Bit)(propArray[i]);
        switch(prop)
        {
#ifndef TM_SSL_REJECT_EXPORTABLE_CIPHER
        case TM_TLS_RSA_NULL_MD5:
        case TM_TLS_RSA_NULL_SHA:
        case TM_TLS_RSA_NULL_SHA256:
        case TM_TLS_RSA_EXPORT_RC4_40_MD5:
        case TM_TLS_RSA_EXPORT_RC2_40_MD5:
        case TM_TLS_RSA_EXPORT_DES40_CBC_SHA:
        case TM_TLS_DHE_DSS_EPT_DES40_CBC_SHA:
        case TM_TLS_DHE_RSA_EPT_DES40_CBC_SHA:
        case TM_TLS_RSA_EPT1K_DES_CBC_SHA:
        case TM_TLS_DHE_DSS_EPT1K_DES_CBC_SHA:
        case TM_TLS_RSA_EPT1K_RC4_56_SHA:
        case TM_TLS_DHE_DSS_EPT1K_RC4_56_SHA:
            break;
#endif /* TM_SSL_REJECT_EXPORTABLE_CIPHER */
#ifndef TM_EXPORT_RESTRICT
        case TM_TLS_RSA_RC4_128_MD5:
        case TM_TLS_RSA_RC4_128_SHA:
        case TM_TLS_RSA_DES_CBC_SHA:
        case TM_TLS_RSA_3DES_EDE_CBC_SHA:
        case TM_TLS_RSA_AES_128_CBC_SHA:
        case TM_TLS_RSA_AES_256_CBC_SHA:
#ifdef TM_USE_SSL_VERSION_33
        case TM_TLS_RSA_AES_128_CBC_SHA256:
        case TM_TLS_RSA_AES_256_CBC_SHA256:
#endif /* TM_USE_SSL_VERSION_33 */
#ifdef TM_SSL_USE_EPHEMERAL_DH
        case TM_TLS_DHE_DSS_RC4_128_SHA:
        case TM_TLS_DHE_DSS_DES_CBC_SHA:
        case TM_TLS_DHE_DSS_3DES_EDE_CBC_SHA:
        case TM_TLS_DHE_RSA_DES_CBC_SHA:
        case TM_TLS_DHE_RSA_3DES_EDE_CBC_SHA:
        case TM_TLS_DHE_DSS_AES_128_CBC_SHA:
        case TM_TLS_DHE_RSA_AES_128_CBC_SHA:
        case TM_TLS_DHE_DSS_AES_256_CBC_SHA:
        case TM_TLS_DHE_RSA_AES_256_CBC_SHA:
#ifdef TM_USE_SSL_VERSION_33
        case TM_TLS_DHE_DSS_AES_128_CBC_SHA256:
        case TM_TLS_DHE_RSA_AES_128_CBC_SHA256:
        case TM_TLS_DHE_DSS_AES_256_CBC_SHA256:
        case TM_TLS_DHE_RSA_AES_256_CBC_SHA256:
#endif /* TM_USE_SSL_VERSION_33 */
#endif /* TM_SSL_USE_EPHEMERAL_DH */
            break;
#endif /* TM_EXPORT_RESTRICT */
        default:
/* we don't support these algorithms */
            errorCode = TM_EINVAL;
            goto SSL_SET_SESSION_PROPOSALS_UNLOCK;
        }
#ifdef TM_USE_SSL_VERSION_33
        if (!((sslSessionPtr->ssVersion) & TM_SSL_VERSION_33))
        {
            switch(prop)
            {
#ifndef TM_EXPORT_RESTRICT
            case TM_TLS_RSA_AES_128_CBC_SHA256:
            case TM_TLS_RSA_AES_256_CBC_SHA256:
#ifdef TM_SSL_USE_EPHEMERAL_DH
            case TM_TLS_DHE_DSS_AES_128_CBC_SHA256:
            case TM_TLS_DHE_RSA_AES_128_CBC_SHA256:
            case TM_TLS_DHE_DSS_AES_256_CBC_SHA256:
            case TM_TLS_DHE_RSA_AES_256_CBC_SHA256:
#endif /* TM_SSL_USE_EPHEMERAL_DH */
/* we don't support these algorithms */
                errorCode = TM_EINVAL;
                goto SSL_SET_SESSION_PROPOSALS_UNLOCK;
#endif /* TM_EXPORT_RESTRICT */
            default:
                break;
            }
        }
#endif /* TM_USE_SSL_VERSION_33 */
        sslSessionPtr->ssProposals[i] = prop;
    }
#endif /* TM_SSL_TEMP_CASE_EXISTS */

SSL_SET_SESSION_PROPOSALS_UNLOCK:
    tfSslSessionReturn(sslSessionPtr);

    return errorCode;
}


/* The list walk function to be called by tfSslSessionClose, which
 * is going to close all SSL connections in that session. Clear all
 * SSL-related feature, such as clear tcpSslFlags,
 * clear tcpSslConnStatePtr
 */
static int tfSslSessionCloseConnection(ttNodePtr       nodePtr,
                                       int             tcpSslFlags)
{
    ttSslSessionPtr               sslSessionPtr;
    ttSslConnectStatePtr          connStatePtr;
    ttSocketEntryPtr              socketEntryPtr;
    ttTcpVectPtr                  tcpVectPtr;
    int                           errorCode;
#ifdef TM_USE_BSD_DOMAIN
    int                           af;
#endif /* TM_USE_BSD_DOMAIN */
    int                           socketDescriptor;

    errorCode = TM_ENOERROR;

    connStatePtr = (ttSslConnectStatePtr)nodePtr;
    sslSessionPtr = connStatePtr->scsSessionPtr;
    socketDescriptor = (int)connStatePtr->scsSocIndex;
    tm_call_unlock(&sslSessionPtr->ssLockEntry);

/*1 Map from socket descriptor to locked socket entry pointer */
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_INET;
#endif /* TM_USE_BSD_DOMAIN */
    socketEntryPtr = tfSocketCheckValidLock(  socketDescriptor
#ifdef TM_USE_BSD_DOMAIN
                                            , &af
#endif /* TM_USE_BSD_DOMAIN */
                                           );
    if ( socketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR )
    {
        errorCode = TM_EBADF;
        goto sslSessionCloseConnectionExit;
    }
/* make sure it is a TCP socket */
    if (socketEntryPtr->socProtocolNumber != (tt8Bit)IP_PROTOTCP)
    {
        errorCode = TM_EPROTONOSUPPORT;
        goto sslSessionCloseConnectionExit;
    }
    tcpVectPtr = (ttTcpVectPtr)socketEntryPtr;

    if (tcpVectPtr->tcpsSslConnStatePtr == connStatePtr)
/* If not already freed between unlocking the session, and locking the socket */
    {
/*2. Free the connStatePtr and clear tcpVectPtr for any SSL information,
 * also cleared is the scsSessionPtr field. After this call, the
 * connStatePtr should not have direct relationship to an SSL session
 */
        connStatePtr->scsRunFlags |= TM_SSL_RUNFLAG_IMCOMP_CLOSE;
        tfSslClearConnState(connStatePtr, tcpVectPtr, tcpSslFlags);
/* notify user that ssl connection is closed for both send and receive
 * direction
 */
        tfSocketNotify(socketEntryPtr,
                       (TM_SELECT_READ | TM_SELECT_WRITE),
                       (TM_CB_SSL_SEND_CLOSE | TM_CB_SSL_RECV_CLOSE),
                       0);
    }

/*3. Return the lock */
sslSessionCloseConnectionExit:
    errorCode = tfSocketReturn(socketEntryPtr, socketDescriptor,
                               errorCode, errorCode);
/* Relock the session */
    tm_call_lock_wait(&sslSessionPtr->ssLockEntry);
    return errorCode;
}

/* The caller of this function is responsible to clear the sgSessionArray
 * in tvSslTlsPtr
 * tfSslSessionClose will check the cached connections, clear each of them
 * for any SSL related features. Don't send Close Alert (User should call
 * tfSslConnectUserClose in order to send out CloseAlert
 * The caller should have locked the session
 * PARAMETER: resetSocOpt, two functions are going to call tfSslSessionClose.
 * One is tfSslSessionUserClose, in this case, we just reset the running
 * ssl flags (keep the server|client on), another call is from
 * tfSslUninitialize, in which case, all tcpsSslFlags needs to be cleared
 */
static void tfSslSessionClose(ttSslSessionPtr    sslSessionPtr,
                              int                tcpSslFlags)
{
#ifdef TM_PUBKEY_USE_RSA
    ttPkiPublicKeyPtr            eRsaPtr;
#endif /* TM_PUBKEY_USE_RSA */
    ttPkiCertListPtr             certPtr;
#ifdef TM_USE_SSL_SERVER
#ifdef TM_SSL_USE_MUTUAL_AUTH
    ttSslClientAuthCalistPtr     calistPtr;
#endif /*  TM_SSL_USE_MUTUAL_AUTH */
#endif /* TM_USE_SSL_SERVER*/
    ttListPtr                    activeListPtr;
    ttNodePtr                    nodePtr;
    unsigned int                 i;

/* if this session has any active connection, we need terminate them and
 * clear each connection state by calling tfSslClearConnState, if we don't
 * free the connection state yet, we need to clear the sslSessionPtr
 */
    activeListPtr = &sslSessionPtr->ssActiveConnectList;
    nodePtr = activeListPtr->listHeadNode.nodeNextPtr;
    while (nodePtr != &(activeListPtr->listHeadNode))
    {
/*
 * Note: remove before calling tfSslSessionCloseConnection because we unlock
 * the session inside that call. This implies that all other calls
 * to tfListRemove() will have to check that the node is still in the
 * queue before removing it.
 * Removing this entry before calling tfSslSessionCloseConnection() (which
 * unlocks the session, to lock the TCP vector) will ensure that another call
 * to tfSslUninitialize(), or tfSslSessionClose() won't try and remove the
 * same entry twice.
 */
        tfListRemove(activeListPtr, nodePtr);
/* Note: tfSslSesionCloseConnection() will free entry if tcpSslFlags is 0. */
        tfSslSessionCloseConnection(nodePtr, tcpSslFlags);
/* Previous connection removed */
        nodePtr = activeListPtr->listHeadNode.nodeNextPtr;
    }
#ifdef TM_PUBKEY_USE_RSA
    eRsaPtr = sslSessionPtr->ssEphemRsaPtr;
    if (eRsaPtr)
    {
        tfPkiGeneralRsaFree((ttGeneralRsaPtr)
            (eRsaPtr->pkKeyPtr));
        tm_free_raw_buffer(eRsaPtr);
        sslSessionPtr->ssEphemRsaPtr = (ttPkiPublicKeyPtr)0;
    }
#endif /* TM_PUBKEY_USE_RSA */
    if (sslSessionPtr->ssMyIdPtr)
    {
        tm_free_raw_buffer(sslSessionPtr->ssMyIdPtr);
        sslSessionPtr->ssMyIdPtr = (ttCharPtr)0;
    }

/* check in the certificate list we used */
    tm_call_lock_wait(&(tm_context(tvPkiLockEntry)));
    certPtr = sslSessionPtr->ssCertListPtr;
    while (certPtr)
    {
        certPtr->certCheckOut--;
        certPtr = certPtr->certSslNextPtr;
    }
    sslSessionPtr->ssCertListPtr = (ttPkiCertListPtr)0;
    tm_call_unlock( &(tm_context(tvPkiLockEntry)));
/* if this session has any cache, we need free them */
    for (i = 0; i < sslSessionPtr->ssTotalCache; i ++)
    {
/* free each cached element */
        if (sslSessionPtr->ssConnectCache[i])
        {
            tm_free_raw_buffer((ttRawBufferPtr)
                (sslSessionPtr->ssConnectCache[i]));
            sslSessionPtr->ssConnectCache[i] = (ttSslCacheElementPtr)0;
        }
    }
    if (sslSessionPtr->ssTotalCache)
    {
        tm_free_raw_buffer((ttRawBufferPtr)sslSessionPtr->ssConnectCache);
        sslSessionPtr->ssTotalCache = 0;
    }

#ifdef TM_USE_SSL_SERVER
#ifdef TM_SSL_USE_MUTUAL_AUTH
    calistPtr = sslSessionPtr->ssClientAuthCasPtr;
    while (calistPtr)
    {
        sslSessionPtr->ssClientAuthCasPtr = calistPtr->scacNextPtr;
        tm_free_raw_buffer(calistPtr);
        calistPtr = sslSessionPtr->ssClientAuthCasPtr;
    }
    sslSessionPtr->ssClientAuthCasPtr = (ttSslClientAuthCalistPtr)0;
#endif /* TM_SSL_USE_MUTUAL_AUTH */
#endif /*TM_USE_SSL_SERVER */
/* let the caller unlock session and free the buffer */
    return;
}
/*
 * Description tfSslSessionUserClose
 * User calls this function to close a session. There are maybe some SSL
 * connections open inside this session. User should call
 * tfSslConnectUserClose or tfClose first in order to close all active
 * connections in this session. If user fails to do so themselves, this
 * function call will close all active SSL connections in this session
 * before free the session memory, flushing each SSL send buffer if
 * necessory. All cached connection information in this session will be
 * freed too.
 * Parameters
 * sessionId            The session ID used
 *
 * Returns
 * Value    Meaning
 * TM_ENOERROR          Success call
 * TM_EINVAL            Session not found
 */
int tfSslSessionUserClose(int          sessionId)
{
    ttSslSessionPtr              sslSessionPtr;
    int                          errorCode;

    errorCode = TM_ENOERROR;
    sslSessionPtr = (ttSslSessionPtr)tfSslGetSessionUsingIndex(sessionId);
    if (sslSessionPtr)
    {
/* This session is gone, clear the corresponding slot in sgSessionArray
 * to indicate that
 */
        ((ttSslGlobalEntryPtr)tm_context(tvSslTlsPtr))
                ->sgSessionArray[sessionId] = (ttSslSessionPtr) 0;
        sslSessionPtr->ssOwnerCount--; /* out of the array */
        tfSslSessionClose(sslSessionPtr, TM_SSLF_CLTSRV_MASK);
        tfSslSessionReturn(sslSessionPtr);
    }
    else
    {
        errorCode = TM_EINVAL;
    }
    return errorCode;
}

/* this function pre-process the received packet list to determine if we have
 * a full SSL record or not
 * Firstly, check how many ssl header bytes we already have. Copy until we
 * get a full record header. Free any packet if we are done.
 * Then link the following packets to the SSL buffer
 * Three conditions need to be taken care:
 * case 1: The very first SSL record header is full, and we already got the
 *         whole SSL record to be decrypted. For some reason, user hasn't
 *         trigger the decrypt process yet. There is maybe more SSL record
 *         attached. For this case, we just append the packetPtr the the end
 *         of buffer. Return TM_ENOERROR for sure.
 * case 2: The very first SSL record header is full, but we haven't got the
 *         whole SSL record yet. (we may already have some scattered data
 *         queued, but all together still short of a whole record) For this
 *         case, we check if we can make a new ssl record. If yes, we return
 *         TM_ENOERROR, otherwise, return TM_EWOULDBLOCK. Eitherway, we need
 *         append the packetPtr if any
 * case 3: The very first SSL record header is not full yet. Firstly, we need
 *         to see if we can convert to case 2.
 */
static int tfSslHasMoreRecord(ttSslConnectStatePtr   connStatePtr,
                              ttPacketPtr            nextPacketPtr,
                              tt8BitPtr              hasMoreRecordPtr)
{
    ttSslRecordHeaderPtr    sslHeaderPtr;
    ttPacketPtr             currtPacketPtr;
    tt8BitPtr               dataPtr;
    ttPktLen                dataLength;
    int                     errorCode;
    tt8Bit                  needFree;
    tt8Bit                  needCopyLen;
    tt8Bit                  getFullRecord;
    tt8Bit                  needAppendPacket;
    tt8Bit                  noNewPacket;
    tt8Bit                  recordHeaderSize;


    getFullRecord = TM_8BIT_NO;
    needFree = TM_8BIT_NO;
    noNewPacket = TM_8BIT_NO;
    needAppendPacket = TM_8BIT_YES;
    errorCode = TM_ENOERROR;

    if (!nextPacketPtr)
    {
        needAppendPacket = TM_8BIT_NO;
        noNewPacket = TM_8BIT_YES;
        nextPacketPtr = connStatePtr->scsReadPktPtr;
    }

    while (nextPacketPtr)
    {
        needFree = TM_8BIT_NO; /* if it needs free, we don't append !*/
        dataPtr = nextPacketPtr->pktLinkDataPtr;
        dataLength = nextPacketPtr->pktLinkDataLength;
/*1. we don't have RecordSize yet (recordSize hasn't been set yet)*/
        if (!connStatePtr->scsNextReadRecordSize)
        {
#ifdef TM_USE_SSL_SERVER
#ifdef TM_SSL_ACCEPT_20_CLIENTHELLO
/* We don't support SSL 2.0, we only support SSL 2.0 client hello */
            if (   (connStatePtr->scsClientOrServer == TM_SSL_SERVER)
                && (   (    (connStatePtr->scsHeaderBufferCount == 0)
                         && (*dataPtr & 0x80) != 0) )
                    || (    (connStatePtr->scsHeaderBufferCount == 1)
                         && (connStatePtr->scsHeaderBuffer[0] & 0x80) != 0)
               )
/* SSL 2.0 header: first 2 bytes gives the length of the record. */
            {
                recordHeaderSize = 2;
            }
            else
#endif /*TM_SSL_ACCEPT_20_CLIENTHELLO*/
#endif /* TM_USE_SSL_SERVER */
            {
                recordHeaderSize = TM_SSL_RECORD_HEADER_SIZE;
            }
            if (connStatePtr->scsHeaderBufferCount < recordHeaderSize)
            {
                needCopyLen = (tt8Bit)(recordHeaderSize -
                              connStatePtr->scsHeaderBufferCount);
                if ((ttPktLen)needCopyLen >= dataLength)
                {
                    needCopyLen = (tt8Bit)dataLength;
/* This packet is done, need free it */
                    needFree = TM_8BIT_YES;
                    dataLength = 0;
                }
                else
                {
                    tm_pkt_ulp_hdr(nextPacketPtr, needCopyLen);
                    dataLength = nextPacketPtr->pktLinkDataLength;
                }
                tm_bcopy(dataPtr,
                         &connStatePtr->scsHeaderBuffer
                         [connStatePtr->scsHeaderBufferCount],
                         needCopyLen);
                connStatePtr->scsHeaderBufferCount =
                            (tt8Bit)(connStatePtr->scsHeaderBufferCount
                             +  needCopyLen);
                if (connStatePtr->scsHeaderBufferCount == recordHeaderSize)
                {
#ifdef TM_USE_SSL_SERVER
#ifdef TM_SSL_ACCEPT_20_CLIENTHELLO
                    if (recordHeaderSize == 2)
                    {
                        connStatePtr->scsNextReadRecordSize = (tt16Bit)
                        ( (tt16Bit)((tt16Bit)(connStatePtr->scsHeaderBuffer[0]
                                              & 0x7f) << 8)
                         |(tt16Bit)((connStatePtr->scsHeaderBuffer[1])) );
                    }
                    else
#endif /*TM_SSL_ACCEPT_20_CLIENTHELLO*/
#endif /* TM_USE_SSL_SERVER */
                    {
                        sslHeaderPtr = (ttSslRecordHeaderPtr)
                                         connStatePtr->scsHeaderBuffer;
                        connStatePtr->scsNextReadRecordSize =
                            (tt16Bit)((sslHeaderPtr->srhLengthHigh << 8)
                                      | sslHeaderPtr->srhLengthLow);
                    }
                    connStatePtr->scsNextReadRecordSizeToGo =
                        connStatePtr->scsNextReadRecordSize;
                    if (connStatePtr->scsNextReadRecordSize >
                                            TM_SSL_BUFFER_SIZE_MAX)
                    {
                        getFullRecord = TM_8BIT_NO;
                        connStatePtr->scsNextReadRecordSize = 0;
                        connStatePtr->scsNextReadRecordSizeToGo = 0;
                        connStatePtr->scsHeaderBufferCount = 0;
                        needFree = TM_8BIT_YES;
                        errorCode = TM_EMSGSIZE;
                        (void)tfSslBuildSendAlert(connStatePtr,
                                                  TM_SSL_ALTLEV_FATAL,
                                                  TM_SSL_ALT_RECOVERFLOW);
                    }
                }
            }
        }
/* we could set the scsNextReadRecordSize just in the above*/
        if ((connStatePtr->scsNextReadRecordSize != 0) &&
            (connStatePtr->scsNextReadRecordSizeToGo == 0))
        {
            getFullRecord = TM_8BIT_YES;
/* even after we set getFullRecord to YES, we still need to go through each
 * nextPacketPtr and link them together
 */
        }
        else if(connStatePtr->scsNextReadRecordSize != 0)
        {
            if ((ttPktLen)connStatePtr->scsNextReadRecordSizeToGo > dataLength)
            {
                connStatePtr->scsNextReadRecordSizeToGo = (tt16Bit)
                      (connStatePtr->scsNextReadRecordSizeToGo - dataLength);
            }
            else
            {
                connStatePtr->scsNextReadRecordSizeToGo = 0;
                getFullRecord = TM_8BIT_YES;
            }
        }


        currtPacketPtr = nextPacketPtr;
        nextPacketPtr = nextPacketPtr->pktLinkNextPtr;

        if (needFree == TM_8BIT_YES)
        {
            if (currtPacketPtr == connStatePtr->scsReadPktPtr)
            {
                connStatePtr->scsReadPktPtr = nextPacketPtr;
            }
            if (currtPacketPtr == connStatePtr->scsReadPktTailPtr)
            {
                connStatePtr->scsReadPktTailPtr = (ttPacketPtr)0;
            }
            currtPacketPtr->pktLinkNextPtr = (ttPacketPtr)0;
            tfFreePacket(currtPacketPtr, TM_SOCKET_LOCKED);
            if (errorCode != TM_ENOERROR)
            {
                break;
            }
        }
        else if (needAppendPacket == TM_8BIT_YES)
        {
/* if needFree == TM_8BIT_YES, we don't append even if needAppendPacket
 * == YES
 */
            if (!connStatePtr->scsReadPktTailPtr)
            {
                connStatePtr->scsReadPktPtr = currtPacketPtr;
            }
            else
            {
                connStatePtr->scsReadPktTailPtr->pktLinkNextPtr =
                          currtPacketPtr;
            }
            connStatePtr->scsReadPktTailPtr = currtPacketPtr;
            connStatePtr->scsTcpVectPtr->tcpsSslRecvQBytes +=
                     currtPacketPtr->pktLinkDataLength;

        }

        if ((getFullRecord == TM_8BIT_YES) && (needAppendPacket == TM_8BIT_NO))
        {
            break;
        }
    }
    if (connStatePtr->scsHeaderBuffer[0] == TM_SSL_TYPE_APPLICATION)
    {
/* set the tcpVectPtr bit to indicate if we have records or not */
        if (getFullRecord == TM_8BIT_YES)
        {
            connStatePtr->scsTcpVectPtr->tcpsSslFlags |=
                       TM_SSLF_MORE_RECORDS;
        }
        else
        {
            tm_16bit_clr_bit( connStatePtr->scsTcpVectPtr->tcpsSslFlags,
                              TM_SSLF_MORE_RECORDS);
        }
    }

/* If we use the already queued packet to search for next SSL record, we need
 * to deduct whatever stored in the SSL header bufer
 */
    if (noNewPacket == TM_8BIT_YES)
    {
        connStatePtr->scsTcpVectPtr->tcpsSslRecvQBytes -=
            connStatePtr->scsHeaderBufferCount;


    }

    if (errorCode == TM_ENOERROR)
    {
        *hasMoreRecordPtr = getFullRecord;
    }
    return errorCode;
}

/*
 * Description
 * tfSslNewSession opens a new SSL session.
 *
 * Parameters
 * certIdentity The SSL peer's own certificate name. For Treck crypto
 * library, whenever you add one certificate, you assign a name to that
 * certificate. certIdentity is the name of your own certificate.
 *
 * maxCaches    The maximum number of caches user wants to store for this
 * session. Must be greater or equal to zero. If zero is passed in, there
 * is no cache at all. That means there is no connection resumption at all
 * either. Note that each cache entry will consume about 100 bytes memory

 * Version  The version number user wants to support for this session. Valid
 * value can be TM_SSL_VERSION_30, TM_SSL_VERSION_31, or (TM_SSL_VERSION_30 |
 * TM_SSL_VERSION_31). Notice that TM_USE_SSL_VERSION_30 must be defined if
 * user inputs TM_SSL_VERSION_30, and TM_USE_SSL_VERSION_31 must be defined if
 * user inputs TM_SSL_VERSION_31
 *
 * option   The options we support currently is TM_SSL_OPT_CLIENTAUTH. If
 * a session is created with this option, and if that session is bound to a
 * SSL server , then it requires client authentication. Enter zero to
 * disable any option. Another option we support is TM_SSL_OPT_SERVERPROP
 *
 * Returns
 * Value    Meaning
 * -TM_EPERM                tvSslTlsPtr is NULL, need to call tfUseSsl first
 * -TM_EINVAL               Input parameter is invalid
 * -TM_ENOBUFS              No buffers available
 * Non-negative number      The session ID for server session just created
 */

int tfSslNewSession(  ttUserConstVoidPtr           certIdentity,
                      unsigned int                 maxCaches,
                      int                          version,
                      unsigned int                 option)
{
    ttSslSessionPtr        sslSessionPtr;
    int                    sessionId;
    int                    errorCode;
    int                    supportVersion;

    errorCode = TM_ENOERROR;
    sslSessionPtr = (ttSslSessionPtr)0;
/* assume new session error */
    sessionId = -1;
    if (!tm_context(tvSslTlsPtr))
    {
/* need to call tfUseSsl first */
        errorCode = TM_EPERM;
        goto sslNewSessionExit;
    }

#ifndef TM_INT_LE16
    if ((tt32Bit)option > TM_UL(0xffff))
    {
/* we have only 16 bit to store the option */
        errorCode = TM_EINVAL;
        goto sslNewSessionExit;
    }
#endif /* TM_INT_LE16 */

/* 1. Check the parameters */
    supportVersion = 0;
#ifdef TM_USE_SSL_VERSION_30
    supportVersion |= TM_SSL_VERSION_30;
#endif /* TM_USE_SSL_VERSION_30 */

#ifdef TM_USE_SSL_VERSION_31
    supportVersion |= TM_SSL_VERSION_31;
#endif /* TM_USE_SSL_VERSION_31 */

#ifdef TM_USE_SSL_VERSION_33
    supportVersion |= TM_SSL_VERSION_33;
#endif /* TM_USE_SSL_VERSION_33 */

    if ((version == 0) || ((version & supportVersion) != version))
    {
        errorCode = TM_EINVAL;
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfSslNewSession", "Version not support");
#endif /* TM_ERROR_CHECKING */
        goto sslNewSessionExit;
    }


/*2. Allocate buffer for connStatePtr */
    sslSessionPtr = (ttSslSessionPtr)tm_get_raw_buffer
                        (sizeof(ttSslSession));
    if (!sslSessionPtr)
    {
        errorCode = TM_ENOBUFS;
        goto sslNewSessionExit;
    }

    tm_bzero(sslSessionPtr, sizeof(ttSslSession));
    sslSessionPtr->ssVersion = (tt8Bit)version;
#ifndef TM_SSL_USE_MUTUAL_AUTH
    if (option & TM_SSL_OPT_CLIENTAUTH)
    {
        errorCode = TM_EINVAL;
        goto sslNewSessionExit;
    }
#endif /*  TM_SSL_USE_MUTUAL_AUTH */
#if (defined(TM_USE_SSL_CLIENT) && defined(TM_SSL_CLIENT_RESPONSE_NO_CERT))
    option |= TM_SSL_OPT_CLIENT_NOCERT;
#endif /* (defined(TM_USE_SSL_CLIENT) && defined(TM_SSL_CLIENT_RESPONSE_NO_CERT)) */
    sslSessionPtr->ssOption = (tt16Bit)(option);
/* allocate memory for session caches if we need cache*/
    if (maxCaches != 0)
    {
        sslSessionPtr->ssConnectCache =
                    tm_get_raw_buffer(maxCaches * sizeof(tt32Bit));
        if (!sslSessionPtr->ssConnectCache)
        {
            errorCode = TM_ENOBUFS;
            goto sslNewSessionExit;
        }
        else
        {
            tm_bzero(sslSessionPtr->ssConnectCache,
                maxCaches * sizeof(tt32Bit));
            sslSessionPtr->ssTotalCache = maxCaches;
        }
    }
    tfListInit(&sslSessionPtr->ssActiveConnectList);
/* copy the common name to this session */
    if (certIdentity)
    {
        sslSessionPtr->ssMyIdPtr = tm_get_raw_buffer
            ((ttPktLen) (tm_strlen(certIdentity) + 1));
        if (!sslSessionPtr->ssMyIdPtr)
        {
            errorCode = TM_ENOBUFS;
            goto sslNewSessionExit;
        }
        else
        {
            tm_strcpy(sslSessionPtr->ssMyIdPtr, certIdentity);
        }

/* if we can not find the whole certificate list according to this
 * common name, we quit
 */
        errorCode = tfSslGetCertList(sslSessionPtr);
        if (errorCode != TM_ENOERROR)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfSslNewSession", "Certificate not found");
#endif /* TM_ERROR_CHECKING */
            goto sslNewSessionExit;
        }
    }
/* copy the default proposal to this session */
    tm_bcopy(tlSslProposalSuites, sslSessionPtr->ssProposals,
             TM_SSL_PROPOSAL_NUMBER * sizeof(tt16Bit));

#ifdef TM_USE_SSL_VERSION_33
    if (sslSessionPtr->ssVersion & TM_SSL_VERSION_33)
    {
        sslSessionPtr->ssProposals[0] = TM_TLS_RSA_AES_128_CBC_SHA256;
        sslSessionPtr->ssProposals[1] = TM_TLS_DHE_RSA_AES_128_CBC_SHA256;
    }
#endif /* TM_USE_SSL_VERSION_33 */

/* put this sslSessionPtr into our global variable tvSslTlsPTr*/
    sessionId = tfSslPutSessionIntoGlobal(sslSessionPtr);
    if (sessionId == -1)
    {
/* No slot available */
        errorCode = TM_ENOENT;
    }

sslNewSessionExit:
    if (errorCode != TM_ENOERROR)
    {
        sessionId = - errorCode;
        if (sslSessionPtr != (ttSslSessionPtr)0)
        {
            tfSslSessionClose(sslSessionPtr, 0);
            tm_free_raw_buffer((ttRawBufferPtr)sslSessionPtr);
        }
    }
    return sessionId;
}

/*
 * Description
 * User may call this function tfSslSetSessionOptions to set session
 * option in running time. The only option supported right now is
 * TM_SSL_OPTNAM_CLIENTAUTH, which is used to enable mutual authentication
 *
 * Parameters
 * sessionId            The session interested
 * optionName           OptionName to change, valid value:
 *
 * TM_SSL_OPTNAM_CLIENTAUTH
 *  This option enables client authentication
 *
 * TM_SSL_OPTNAM_SERVERPROP
 * If user wants the SSL server to have more control on which cipher
 * suite to use, user may call tfSslSetSessionProposal on the server
 * SSL session. Upon receiving the ClientHello message, SSL server
 * will match the client proposals against the proposals in server
 * session. Only these matched proposals could be selected. Set this
 * option on server session to have this feature. Otherwise, when server
 * receives ClientHello message, the client proposals will be used to
 * match all possible cipher suites the SSL server supports.
 *
 * optionValue  Any non-zero value will enable mutual authentication, zero
 * value will disable mutual authentication.
 *
 * Returns
 * Value    Meaning
 * TM_ENOERROR  success
 * TM_EINVAL    Invalid option name, or invalid session Id.
 */
int tfSslSetSessionOptions(int                     sessionId,
                           int                     optionName,
                           int                     optionValue)
{
    ttSslSessionPtr        sslSessionPtr;
    int                    errorCode;

    errorCode = TM_EINVAL;

    sslSessionPtr = (ttSslSessionPtr)tfSslGetSessionUsingIndex(sessionId);
    if (sslSessionPtr)
    {
        errorCode = TM_ENOERROR;
        switch (optionName)
        {
        case TM_SSL_OPTNAM_CLIENTAUTH:
            if (optionValue)
            {
                sslSessionPtr->ssOption |= TM_SSL_OPT_CLIENTAUTH;
            }
            else
            {
                tm_16bit_clr_bit( sslSessionPtr->ssOption,
                                  TM_SSL_OPT_CLIENTAUTH);
            }
            break;
        case TM_SSL_OPTNAM_SERVERPROP:
            if (optionValue)
            {
                sslSessionPtr->ssOption |= TM_SSL_OPT_SERVERPROP;
            }
            else
            {
                tm_16bit_clr_bit(sslSessionPtr->ssOption, TM_SSL_OPT_SERVERPROP);
            }
            break;
        case TM_SSL_OPTNAM_NO_SEND_EMPTY_FRAG:
            if (optionValue)
            {
                sslSessionPtr->ssOption |= TM_SSL_OPT_NO_SEND_EMPTY_FRAG;
            }
            else
            {
                tm_16bit_clr_bit(sslSessionPtr->ssOption, TM_SSL_OPT_NO_SEND_EMPTY_FRAG);
            }
            break;
#ifdef TM_SSL_CLIENT_RESPONSE_NO_CERT
        case TM_SSL_OPTNAM_CLIENT_NOCERT:
            if (optionValue)
            {
                sslSessionPtr->ssOption |= TM_SSL_OPT_CLIENT_NOCERT;
            }
            else
            {
                tm_16bit_clr_bit( sslSessionPtr->ssOption,
                                  TM_SSL_OPT_CLIENT_NOCERT);
            }
            break;
#endif /* TM_SSL_CLIENT_RESPONSE_NO_CERT */
        case TM_SSL_OPTNAM_CLIENT_CACHEID:
            if (optionValue)
            {
                sslSessionPtr->ssOption |= TM_SSL_OPT_CLIENT_CACHEID;
            }
            else
            {
                tm_16bit_clr_bit( sslSessionPtr->ssOption,
                                  TM_SSL_OPT_CLIENT_CACHEID);
            }
            break;
        default:
            break;
        }
        tfSslSessionReturn(sslSessionPtr);
    }
    return errorCode;
}

static int tfSslIncomingAlert(ttSslConnectStatePtr    connStatePtr,
                              ttPacketPtr             packetPtr)
{
    ttTcpVectPtr    tcpVectPtr;
    tt8BitPtr       dataPtr;
    int             errorCode;
    tt16Bit         recordSize;
    tt16Bit         cbFlags;
    tt8Bit          alertLevel;
    tt8Bit          alertDescription;
    tt8Bit          ivSize;

    errorCode = TM_ENOERROR;
    recordSize = connStatePtr->scsNextReadRecordSize;
    dataPtr = packetPtr->pktLinkDataPtr;
    ivSize = (tt8Bit)0;
/* We already decrypted this non-application record */

#ifdef TM_USE_SSL_VERSION_33
    if((connStatePtr->scsVersion == TM_SSL_VERSION_33)
      &&(
#ifdef TM_USE_SSL_CLIENT
        ((connStatePtr->scsClientOrServer == TM_SSL_CLIENT) &&
        (connStatePtr->scsState >= TM_SSL_CT_SENT_FINISH))
#ifdef TM_USE_SSL_SERVER
        ||
#endif /* TM_USE_SSL_SERVER */
#endif /* TM_USE_SSL_CLIENT */
#ifdef TM_USE_SSL_SERVER
        ((connStatePtr->scsClientOrServer == TM_SSL_SERVER) &&
        (connStatePtr->scsState >= TM_SSL_ST_OPENED))
#endif /* TM_USE_SSL_SERVER */
        ))
    {
        if((connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_ARCFOUR)
         &&(connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_NULL))
        {
            ivSize = connStatePtr->scsCryptoAlgPtr->ealgBlockSize;
        }
    }
#endif /* TM_USE_SSL_VERSION_33 */
    if (recordSize != 2)
    {
        errorCode = TM_EINVAL;
        goto incomingAlertExit;
    }
    else
    {
        alertLevel = *(dataPtr + ivSize);
        alertDescription = *(dataPtr + ivSize + 1);
    }

    if (alertDescription == TM_SSL_ALT_CLOSE_NOTIFY ||
              alertLevel == TM_SSL_ALTLEV_FATAL)
    {
        tcpVectPtr = connStatePtr->scsTcpVectPtr;
        tm_16bit_clr_bit( tcpVectPtr->tcpsSslFlags, TM_SSLF_MORE_RECORDS);
        tcpVectPtr->tcpsSslFlags |= TM_SSLF_RECV_CLOSE_NOTIFY;
/* notify user that ssl recv path is closed */
        cbFlags = TM_CB_SSL_RECV_CLOSE;
        if(alertLevel == TM_SSL_ALTLEV_FATAL)
        {
            cbFlags |= TM_CB_SSL_HANDSHK_FAILURE;
/* SSL handshake failure, for security reason, you can not send if this bit
 * is set
 */
            tcpVectPtr->tcpsSslFlags |= TM_SSLF_HANDSHK_FAILURE;
        }
        if (alertDescription == TM_SSL_ALT_BADCERT)
        {
            cbFlags |= TM_CB_SSL_BADCERTIFICATE;
        }
/*
 * Notify user that ssl error has happened. Done at the end of
 * tfSslIncomingRecord(), or tfSslConnectUserClose() (user app path)
 * or tfSslIncomingPacket() (TCP incoming path).
 */
        connStatePtr->scsSocCBFlags |= cbFlags;

#ifdef TM_SSL_IMMEDIATE_CLOSE_NOTIFY
/* if we already send close notify, we don't need to close again */
        if (tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                                 TM_SSLF_SEND_ENABLED))
        {
            errorCode = tfSslClose(connStatePtr, 0);
        }
#endif /* TM_SSL_IMMEDIATE_CLOSE_NOTIFY */
    }

incomingAlertExit:
    return errorCode;
}

/* this module will cut the packet queue into two parts,
 * the first one is saved in firstPacketPtr (will have chainDataLength
 * equal to thisRecordSize), the second will saved in secondPacketPtr,
 * tfDuplicatePacket is used if cut should be applied to one shared
 * data.
 */
static void tfSslCutQueuedPacketList(ttPacketPtr         origPacketPtr,
                                     tt16Bit             thisRecordSize,
                                     ttPacketPtrPtr      firstPtrPtr,
                                     ttPacketPtrPtr      secondPtrPtr)
{
    ttPacketPtr           nextPacketPtr;
    ttPacketPtr           nextnextPacketPtr;
    ttPacketPtr           prevPacketPtr;
    ttPacketPtr           firstPktPtr;
    ttPacketPtr           secondPktPtr;
    ttPacketPtr           duplicatePacketPtr;
    ttPktLen              dataLength;
    ttPktLen              needLength;

    dataLength = 0;
    needLength = 0;
    nextPacketPtr = origPacketPtr;
    firstPktPtr = origPacketPtr;
    secondPktPtr = (ttPacketPtr)0;
    prevPacketPtr = (ttPacketPtr)0;

    while (nextPacketPtr)
    {
        needLength = thisRecordSize - dataLength;
        dataLength += nextPacketPtr->pktLinkDataLength;
        if (dataLength == (ttPktLen)thisRecordSize)
        {
/* cut the link list here, we have a clear boundary, so don't need duplicate */
            prevPacketPtr = nextPacketPtr;
            nextPacketPtr = nextPacketPtr->pktLinkNextPtr;
            prevPacketPtr->pktLinkNextPtr = (ttPacketPtr)0;
            secondPktPtr = nextPacketPtr;
            break;
        }
        else if (dataLength > (ttPktLen)thisRecordSize)
        {
/* we need duplicate this nextPacketPtr, and cut the link here, note that, we
 * need to duplicate nextPacketPtr only, not the pktLinkNextPtr ...
 */
            nextnextPacketPtr = nextPacketPtr->pktLinkNextPtr;
            nextPacketPtr->pktLinkNextPtr = (ttPacketPtr)0;
/* if we don't set nextnextPacketPtr, then all packetPtr linked later will
 * be duplicated. That is not what we want
 */
            duplicatePacketPtr = tfDuplicatePacket(nextPacketPtr);
/* set pointer back*/
            nextPacketPtr->pktLinkNextPtr = nextnextPacketPtr;
            if (duplicatePacketPtr)
            {
                if (prevPacketPtr)
                {
                    prevPacketPtr->pktLinkNextPtr = duplicatePacketPtr;
                }
                else
                {
                    firstPktPtr = duplicatePacketPtr;
                }
                duplicatePacketPtr->pktLinkDataLength = needLength;
                tm_pkt_ulp_hdr(nextPacketPtr, needLength);
                secondPktPtr = nextPacketPtr;
                break;
            }
        }
        prevPacketPtr = nextPacketPtr;
        nextPacketPtr = nextPacketPtr->pktLinkNextPtr;
    }

    firstPktPtr->pktChainDataLength = thisRecordSize;
    *firstPtrPtr = firstPktPtr;
    *secondPtrPtr = secondPktPtr;
    return;
}


/* This function handles incoming application data. We
 * 1) cut the queued packet list into half (see tfSslCutQueuedPacketList)
 * The first part should have a whole SSL record in it.
 * 2) Decrypt the SSL record into new packet pointer
 * 3) verfiy the padding and hashing
 * 4) If ok, we link the new packet pointer to socket receive queue,
 * update the socRecvQueueBytes and corresponding queue packet pointer
 */
int tfSslIncomingApplicationData(ttSslConnectStatePtr  connStatePtr)
{
    ttSocketEntryPtr          socketEntryPtr;
    ttPacketPtr               newPacketPtr;
    ttPacketPtr               origPacketPtr;
    tt16Bit                   origDataLen;
    int                       errorCode;
    tt8Bit                    ivLength;
    tt16Bit                   thisRecordSize;


    errorCode = TM_ENOERROR;
    newPacketPtr = (ttPacketPtr)0;
    ivLength = (tt8Bit)0;
/* 1. We need get a new packet pointer to hold the decrypted SSL data.
 */
    origPacketPtr = connStatePtr->scsReadPktPtr;
    thisRecordSize = connStatePtr->scsNextReadRecordSize;
    newPacketPtr = tfGetSharedBuffer(0, thisRecordSize +
                                     TM_SSL_RECORD_HEADER_SIZE,
                                     0);
    if (newPacketPtr == (ttPacketPtr)0)
    {
        errorCode = TM_ENOBUFS;
        goto incomingApplicationExit;
    }
    tm_bcopy(connStatePtr->scsHeaderBuffer,
               newPacketPtr->pktLinkDataPtr,
               TM_SSL_RECORD_HEADER_SIZE);
    newPacketPtr->pktLinkDataLength = TM_SSL_RECORD_HEADER_SIZE;
/* Copy the incoming device handle */
    newPacketPtr->pktDeviceEntryPtr = origPacketPtr->pktDeviceEntryPtr;
#ifdef TM_USE_SSL_VERSION_33
    if((connStatePtr->scsVersion == TM_SSL_VERSION_33)
     &&((connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_ARCFOUR)
      &&(connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_NULL)))
    {
/* Insert IV field in case of CBC. */
        ivLength = connStatePtr->scsCryptoAlgPtr->ealgBlockSize;
    }
#endif /* TM_USE_SSL_VERSION_33 */

/*2. Go through the linked list to find where we should disconnect
 * and maybe duplicate buffer.
 */
    tfSslCutQueuedPacketList(origPacketPtr,
                             thisRecordSize,
                             &origPacketPtr,
                             &connStatePtr->scsReadPktPtr);
    if (!connStatePtr->scsReadPktPtr)
    {
        connStatePtr->scsReadPktTailPtr = (ttPacketPtr)0;
    }

/*3. Ready to do decrypt */
    errorCode = tfSslRecordPacketTransform(connStatePtr,
        origPacketPtr,
        0,
        newPacketPtr,
        thisRecordSize,
        TM_SSL_DECODE);
    if (errorCode != TM_ENOERROR)
    {
        (void)tfSslBuildSendAlert(connStatePtr,
                                  TM_SSL_ALTLEV_FATAL,
                                  TM_SSL_ALT_BADRECMAC);
        goto incomingApplicationExit;
    }

/* calculate IV length */
    newPacketPtr->pktUserStruct.pktuChainDataLength -= ivLength;

    errorCode = tfSslRecordDecodeMacVerify(
                                       connStatePtr,
                                       (ttSslRecordHeaderPtr)newPacketPtr
                                       ->pktLinkDataPtr,
                                       newPacketPtr,
                                       TM_SSL_RECORD_HEADER_SIZE,
                                       thisRecordSize -
                                       ivLength,
                                       &origDataLen);
    if (errorCode != TM_ENOERROR)
    {
        (void)tfSslBuildSendAlert(connStatePtr,
                                  TM_SSL_ALTLEV_FATAL,
                                  TM_SSL_ALT_BADRECMAC);
        goto incomingApplicationExit;
    }
    if (origDataLen == 0)
/* Empty record */
    {
        errorCode = TM_EWOULDBLOCK; /* indicate empty record */
    }
    else
    {
/* change newPacketPtr data pointer position */
        newPacketPtr->pktLinkDataPtr += TM_SSL_RECORD_HEADER_SIZE;
        newPacketPtr->pktLinkDataLength = origDataLen;

/* put data to user's recv queue */
        socketEntryPtr = (ttSocketEntryPtr)connStatePtr->scsTcpVectPtr;
        if (socketEntryPtr->socReceiveQueueNextPtr == TM_PACKET_NULL_PTR)
        {
            socketEntryPtr->socReceiveQueueNextPtr = newPacketPtr;
        }
        else
        {
            socketEntryPtr->socReceiveQueueLastPtr->pktLinkNextPtr = newPacketPtr;
        }
        socketEntryPtr->socReceiveQueueLastPtr = newPacketPtr;
        socketEntryPtr->socRecvQueueBytes += origDataLen;
        newPacketPtr = (ttPacketPtr)0;
    }

incomingApplicationExit:
    if (newPacketPtr != (ttPacketPtr)0)
    {
        tfFreePacket(newPacketPtr, TM_SOCKET_LOCKED);
    }
    return errorCode;
}

/* This function tries to copy one whole SSL record to contiguous buffer
 * This function is called only when we have non-contiguous SSL record.
 * Application Data SSL record doesn't need to make this call. We only copy
 * handshake records or alert records to contiguous buffer
 */
static int tfSslMakeRecordContiguous(ttSslConnectStatePtr    connStatePtr,
                              ttPacketPtrPtr          packetPtrPtr,
                              tt16Bit                 recordSize)
{
    ttPacketPtr         newPacketPtr;
    ttPacketPtr         prevPacketPtr;
    ttPacketPtr         nextPacketPtr;
    tt8BitPtr           bufferPtr;
    tt16Bit             copied;
    int                 errorCode;

    prevPacketPtr = (ttPacketPtr)0;
    copied = (tt16Bit)0;
    errorCode = TM_ENOERROR;

    newPacketPtr = tfGetSharedBuffer(0, recordSize, 0);
    if (!newPacketPtr)
    {
        errorCode = TM_ENOBUFS;
    }
    else
    {
        bufferPtr = newPacketPtr->pktLinkDataPtr;
/* need to cut the queue packet list and copy to this new packetPtr
 *, after we are done, don't forget the free.
 */
        nextPacketPtr = (ttPacketPtr)connStatePtr->scsReadPktPtr;
/* nextPacketPtr is the head of received queue */
        while (nextPacketPtr && (copied < recordSize))
        {
            if (nextPacketPtr->pktLinkDataLength > (ttPktLen)
                       (recordSize - copied))
            {
                tm_bcopy(nextPacketPtr->pktLinkDataPtr,
                         bufferPtr + copied,
                         recordSize - copied);
                tm_pkt_ulp_hdr(nextPacketPtr, (recordSize - copied));
                break;
            }
            tm_bcopy(nextPacketPtr->pktLinkDataPtr,
                bufferPtr + copied,
                nextPacketPtr->pktLinkDataLength);
            copied = (tt16Bit)(copied + nextPacketPtr->pktLinkDataLength);
            prevPacketPtr = nextPacketPtr;
            nextPacketPtr = nextPacketPtr->pktLinkNextPtr;
        }

        newPacketPtr->pktLinkDataLength = recordSize;

        if (prevPacketPtr)
        {
/* cut the previous link, we don't need them any more */
            prevPacketPtr->pktLinkNextPtr = (ttPacketPtr)0;
            tfFreePacket(connStatePtr->scsReadPktPtr, TM_SOCKET_LOCKED);
        }
/*      else
 *      we don't need worry about the else case, connStatePtr->scsReadPktPtr
 * should still points to nextPacketPtr, we don't need free anything.
 */
        connStatePtr->scsReadPktPtr = (ttUserMessage)nextPacketPtr;
        if (!nextPacketPtr)
        {
            connStatePtr->scsReadPktTailPtr = (ttUserMessage)0;
        }
    }

    *packetPtrPtr = newPacketPtr;
    return errorCode;
}

/* This function is responsible to handle handshake records or alert
 * record to make it contiguous, if decryption is required, we decrypt
 * it and verify the results. Before this call, we know that we already
 * have at one full SSL record. So, the first packet link data length
 * could have three possibility
 * 1) pktLinkDataLength = thisRecordSize, no new packet is allocated,
 * the new packet is the old packet, clear cut boundary
 * 2) pktLinkDataLength > thisRecordSize, tfDuplicatePacket is called
 * to make a cut on the same shared data
 * 3) pktLinkDataLength < thisRecordSize, it means we need copy multiple
 * scattered packets into one bigger packet
 * RETURN:
 * The new packet is returned in newPacketPtrPtr
 * function returns errorCode, TM_ENOERROR means no error
 */

static int tfSslIncomingPreProcessing(ttSslConnectStatePtr connStatePtr,
                                      ttPacketPtrPtr       newPacketPtrPtr)
{
    ttSslRecordHeaderPtr      sslHeaderPtr;
    ttPacketPtr               newPacketPtr;
    ttPacketPtr               packetPtr;
    ttPacketPtr               nextPacketPtr;
    int                       errorCode;
    tt16Bit                   recordSize;

    errorCode = TM_ENOERROR;
    packetPtr = connStatePtr->scsReadPktPtr;
    sslHeaderPtr = (ttSslRecordHeaderPtr)(connStatePtr->scsHeaderBuffer);
/* this version is the version of this record */
    if ((sslHeaderPtr->srhVersionMinor > TM_SSL_VERSION_MINOR3)
        ||(!((1 << sslHeaderPtr->srhVersionMinor) &
             connStatePtr->scsSessionPtr->ssVersion)))
    {
/* respond the highest version supported by the server */

        if (connStatePtr->scsClientOrServer == TM_SSL_CLIENT)
        {
incomingPreProcessError:
            connStatePtr->scsVersion = (tt8Bit)
                      (1 << sslHeaderPtr->srhVersionMinor);
            (void)tfSslBuildSendAlert(connStatePtr,
                                      TM_SSL_ALTLEV_FATAL,
                                      TM_SSL_ALT_PROTOVERSION);
            errorCode = TM_EOPNOTSUPP;
            goto incomingPreProcessExit;
        }
        else
        {
/* accepted version03.xx in record layer only initial.
   (i.e., before it is known which version of the protocol will be employed)
   from RFC 5246 Appendix E.  Backward Compatibility E.1. */
            if(connStatePtr->scsState == TM_SSL_ST_INITIAL)
            {
                if ((connStatePtr->scsSessionPtr->ssVersion) & 
                                                TM_SSL_VERSION_33)
                {
                    connStatePtr->scsVersion = TM_SSL_VERSION_33;
                }
                else if ((connStatePtr->scsSessionPtr->ssVersion) & 
                                                TM_SSL_VERSION_31)
                {
                    connStatePtr->scsVersion = TM_SSL_VERSION_31;
                }
                else
                {
                    connStatePtr->scsVersion = TM_SSL_VERSION_30;
                }
                
                if((1 << sslHeaderPtr->srhVersionMinor) < 
                                            connStatePtr->scsVersion)
                {
                    connStatePtr->scsVersion = 
                                (1 << sslHeaderPtr->srhVersionMinor);
                }
            }
            else
            {
                goto incomingPreProcessError;
            }
        }
    }
    else
    {
/* respond the older version available in the server */
        connStatePtr->scsVersion = (1 << sslHeaderPtr->srhVersionMinor)
                              & connStatePtr->scsSessionPtr->ssVersion;
    }
    recordSize = connStatePtr->scsNextReadRecordSize;

/* we definitely don't want to process scattered record in negotiation
 * stage. (what if each individual message header is scattered? that is a
 * pain...)So, if a message is not contiguous, we copy it to make it
 * contiguous. For certificates, we definitely need to copy it contiguous
 * in order for processing. For application data, we don't have to do this
 * especially for stream cipher
 */
    if (packetPtr->pktLinkDataLength == (ttPktLen)recordSize)
    {
/* if equal to record size, we need to remove this packet from the
 * receive chain and update scsReadPktPtr
 */
        connStatePtr->scsReadPktPtr = packetPtr->pktLinkNextPtr;
        if (!connStatePtr->scsReadPktPtr)
        {
            connStatePtr->scsReadPktTailPtr = (ttPacketPtr)0;
        }
        packetPtr->pktLinkNextPtr = 0;
        newPacketPtr = packetPtr;
    }
    else if (packetPtr->pktLinkDataLength < (ttPktLen)recordSize)
    {
/* need to free the original packet list upon return */
        errorCode = tfSslMakeRecordContiguous(connStatePtr,
                                              &newPacketPtr,
                                              recordSize);
    }
    else/* (packetPtr->pktLinkDataLength > recordSize) */
    {
/* need duplicate a packetPtr  needFreePacket = NO;
 */
        nextPacketPtr = packetPtr->pktLinkNextPtr;
        packetPtr->pktLinkNextPtr = (ttPacketPtr)0;
        newPacketPtr = tfDuplicatePacket(packetPtr);
        packetPtr->pktLinkNextPtr = nextPacketPtr; /* set nextpacket back*/
        if(newPacketPtr)
        {
            newPacketPtr->pktLinkDataLength = recordSize;
/* update the old packet pointer */
            tm_pkt_ulp_hdr(packetPtr, recordSize);
            packetPtr->pktLinkNextPtr = nextPacketPtr;
        }
        else
        {
            errorCode = TM_ENOBUFS;
            goto incomingPreProcessExit;
        }
    }

/* if decode is necessory, we decode this data buffer */
    if (connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_CCS_RECVD)
    {
        errorCode = tfSslHandshakeMsgTransform(connStatePtr,
                                               TM_CRYPTOREQUEST_DECODE,
                                               sslHeaderPtr,
                                               newPacketPtr,
/* for decoding, the mac will be write to a tempory address, set to NULL  */
                                               (tt8BitPtr)0,
                                               &recordSize,
                                               0);
        if (errorCode != TM_ENOERROR)
        {
            tfFreePacket(newPacketPtr, TM_SOCKET_LOCKED);
            (void)tfSslBuildSendAlert(connStatePtr,
                                      TM_SSL_ALTLEV_FATAL,
                                      TM_SSL_ALT_BADRECMAC);
            goto incomingPreProcessExit;
        }
/* the record size is going to be used later. We need to strip off the
 * MAC's and paddings
 */
        connStatePtr->scsTcpVectPtr->tcpsSslRecvQBytes -=
               (connStatePtr->scsNextReadRecordSize - recordSize);

        connStatePtr->scsNextReadRecordSize = recordSize;
    }

    *newPacketPtrPtr = newPacketPtr;

incomingPreProcessExit:
    return errorCode;
}


/* This function will be called by tfSslRead or tfSslExecute.
 * Before we call this function from tfSslRead, we already verified that we
 * have at least one whole SSL record for us to process.
 * If we did get a full SSL record, we need do the following
 * 1) If we don't need decrypt, process the record right away. otherwise
 * 2) decrypt it, and write the result to ssReadPktPtr
 * 3) verify the integr.
 * 4) If integrity fails, we need to clear the ssReadPktPtr
 * 5) get the next SSL record size and size to go
 */
/*DESCRIPTION:  This function is called because we got at least one full
 *              SSL handshaking/change cipher record*/
int tfSslIncomingHandShake(ttSslConnectStatePtr connStatePtr,
                           ttPacketPtr          packetPtr)
{
    ttSslHandShakeHeaderPtr   handShakeHeaderPtr;
    tt8BitPtr                 dataPtr;
    int                       errorCode;
    tt16Bit                   segRecordSize;
    tt16Bit                   recordSize;

    errorCode = TM_ENOERROR;
    recordSize = connStatePtr->scsNextReadRecordSize;
    dataPtr = packetPtr->pktLinkDataPtr;

#ifdef TM_USE_SSL_VERSION_33
    if((connStatePtr->scsVersion == TM_SSL_VERSION_33)
     &&(
#ifdef TM_USE_SSL_SERVER
       (((connStatePtr->scsState == TM_SSL_ST_RECV_CCS1)
       ||(connStatePtr->scsState == TM_SSL_ST_RECV_CCS2))
      &&(connStatePtr->scsClientOrServer == TM_SSL_SERVER))
#ifdef TM_USE_SSL_CLIENT
      ||
#endif /* TM_USE_SSL_CLIENT */
#endif /* TM_USE_SSL_SERVER */
#ifdef TM_USE_SSL_CLIENT
       (((connStatePtr->scsState == TM_SSL_CT_RECV_CCS1)
       ||(connStatePtr->scsState == TM_SSL_CT_RECV_CCS2)
       ||(connStatePtr->scsState == TM_SSL_CT_OPENED))
      &&(connStatePtr->scsClientOrServer == TM_SSL_CLIENT))
#endif /* TM_USE_SSL_CLIENT */
      ))
    {
/* Current process is TLS1.2 FINISHED*/
        if((connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_ARCFOUR)
          &&(connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_NULL))
        {
            dataPtr += connStatePtr->scsCryptoAlgPtr->ealgBlockSize;
        }
    }
#endif /* TM_USE_SSL_VERSION_33 */
    while (recordSize)
    {

        handShakeHeaderPtr = (ttSslHandShakeHeaderPtr)dataPtr;
#ifdef TM_USE_SSL_SERVER
#ifdef TM_SSL_ACCEPT_20_CLIENTHELLO
        if (    (connStatePtr->scsClientOrServer == TM_SSL_SERVER)
             && (connStatePtr->scsHeaderBuffer[0] & 0x80) )
        {
            if (handShakeHeaderPtr->hsHandShakeType != TM_SSL_CLIENT_HELLO)
            {
                errorCode = TM_EINVAL;
                goto sslIncomingHandshakeError; /* Incorrect header */
            }
            segRecordSize = connStatePtr->scsNextReadRecordSize;
            recordSize += TM_SSL_HANDSHAKE_HEADER_SIZE;
            goto PARSECLIENTHELLO;
        }
#endif /*TM_SSL_ACCEPT_20_CLIENTHELLO*/
#endif /* TM_USE_SSL_SERVER */

        if (   (recordSize < TM_SSL_HANDSHAKE_HEADER_SIZE)
            || (handShakeHeaderPtr->hsLengthPad != 0))
        {
            errorCode = TM_EINVAL;
            goto sslIncomingHandshakeError;
        }
/* each individual hand shake message length */
        segRecordSize = (tt16Bit)(handShakeHeaderPtr->hsLengthLow
                    + (handShakeHeaderPtr->hsLengthHigh << 8));
        if (segRecordSize > (tt16Bit)(recordSize -
                            TM_SSL_HANDSHAKE_HEADER_SIZE))
        {
            errorCode = TM_EINVAL;
            goto sslIncomingHandshakeError;/* too long */
        }

        dataPtr += TM_SSL_HANDSHAKE_HEADER_SIZE;

#ifdef TM_USE_SSL_SERVER
#ifdef TM_SSL_ACCEPT_20_CLIENTHELLO
PARSECLIENTHELLO:
#endif /*TM_SSL_ACCEPT_20_CLIENTHELLO*/
/* dataPtr points to the real content after the handshake header */
        if (connStatePtr->scsClientOrServer == TM_SSL_SERVER)
        {
            errorCode = tfSslServerProcessHandShake(
                               handShakeHeaderPtr->hsHandShakeType,
                               connStatePtr,
                               dataPtr,
                               segRecordSize);
        }
#endif /* TM_USE_SSL_SERVER */
#ifdef TM_USE_SSL_CLIENT
        if (connStatePtr->scsClientOrServer == TM_SSL_CLIENT)
        {
            errorCode = tfSslClientProcessHandShake(
                               handShakeHeaderPtr->hsHandShakeType,
                               connStatePtr,
                               dataPtr,
                               segRecordSize);
            if (errorCode == TM_SSL_ALT_NO_CERTIFICATE)
            {
/* If server asked us for certificate but we don't have any,
 * send warning no_certificate message
 */
#ifdef TM_SSL_CLIENT_RESPONSE_NO_CERT
                if ((connStatePtr->scsRunFlags
                          & TM_SSL_RUNFLAG_CLIENT_SEND_NOCERT) == 0)
#endif /* TM_SSL_CLIENT_RESPONSE_NO_CERT */
                {
                    (void)tfSslBuildSendAlert(connStatePtr,
                        TM_SSL_ALTLEV_WARNING,
                        TM_SSL_ALT_NO_CERTIFICATE);
                }
#if (defined(TM_SSL_CLIENT_ALLOW_NO_CERT) || defined(TM_SSL_CLIENT_RESPONSE_NO_CERT))
/* If the client allows no certificate operation even in client-authentication
 * situation, we should ignore this error and continue SSL negotiation. To
 * close the connection or not will be up to the server to decide. In any case
 * the client should send an alert-warning message to server.
 */
                errorCode = TM_ENOERROR;
#endif /* (defined(TM_SSL_CLIENT_ALLOW_NO_CERT) || defined(TM_SSL_CLIENT_RESPONSE_NO_CERT)) */
            }
        }
#endif /* TM_USE_SSL_CLIENT */
        if (errorCode != TM_ENOERROR)
        {
sslIncomingHandshakeError:
/* Handshake failure, we need to clear ourself. Note that the peer may send
 * us close_notify. If we don't clear, we can not see that packet */
            (void)tfSslBuildSendAlert(connStatePtr,
                                      TM_SSL_ALTLEV_FATAL,
                                      TM_SSL_ALT_HANDSHAKEFAIL);
            goto sslIncomingHandshakeExit;
        }
/* we already test that recordSize must be bigger than segRecordSize
 * + TM_SSL_HANDSHAKE_HEADER_SIZE
 */
        recordSize = (tt16Bit)(recordSize - segRecordSize -
                             TM_SSL_HANDSHAKE_HEADER_SIZE);
        dataPtr += segRecordSize;
    }

sslIncomingHandshakeExit:
    return errorCode;
}

/* we know for sure that we got at least full SSL record in the receiving
 * buffer. this function is called so that we can get the ssl record. Inside
 * this call, we check if we have more than one SSL record, if yes, we loop
 * until we processed all full SSL record. If a SSL receiving buffer is used
 * up, we free that buffer. (It should be a zero copy buffer).
 */

int tfSslIncomingRecord(ttSslConnectStatePtr  connStatePtr, tt8Bit context)
{
    ttTcpVectPtr              tcpVectPtr;
    ttPacketPtr               nextPacketPtr;
    ttPacketPtr               newPacketPtr;
    int                       errorCode;
    tt16Bit                   selectFlags;
    tt16Bit                   cbFlags;
    tt8Bit                    recordType;
    tt8Bit                    hasMoreRecord;

    errorCode = TM_ENOERROR;

    nextPacketPtr = connStatePtr->scsReadPktPtr;
    tcpVectPtr = connStatePtr->scsTcpVectPtr;

    while (nextPacketPtr)
    {
        recordType = connStatePtr->scsHeaderBuffer[0];
/*
 * for handshake messages, if we got scattered buffer, we copy to this
 * new packetPtr. And need free it eventually
 */
        newPacketPtr = (ttPacketPtr)0;

        if (recordType != TM_SSL_TYPE_APPLICATION)
        {
/* If it is not application data, i.e., handshake messages, we are going to
 * copy non-contiguous records into one big buffer. (otherwise, the server
 * hello, server certificate, finished can be scattered at any length. We
 * don't want to make the handshake processing too hard.) However, for
 * application data, we expect the data to be scattered. We never copy them
 * into one big buffer.
 * Also, for non-application data, we are going to decode it as data buffer,
 * the decrypted result is written back to the same data buffer. However,
 * for application data, we are going to decode it as linked packetPtr or
 * linked zero copy buffer, the decrypted result is either written back to
 * user provided buffer or to SSL processed buffer list.
 */
            errorCode = tfSslIncomingPreProcessing(connStatePtr,
                                                   &newPacketPtr);
            if (errorCode != TM_ENOERROR)
            {
                goto sslIncomingRecordExit;
            }
        }
#ifdef TM_USE_SSL_SERVER
#ifdef TM_SSL_ACCEPT_20_CLIENTHELLO
        if (    (connStatePtr->scsClientOrServer == TM_SSL_SERVER)
             && (recordType & 0x80) )
        {
/* If the record type > 0x80, we know it is client hello in version 2.0 */
            recordType = TM_SSL_TYPE_HANDSHAKE;
        }
#endif  /* TM_SSL_ACCEPT_20_CLIENTHELLO */
#endif /* TM_USE_SSL_SERVER */

        switch (recordType)
        {
        case TM_SSL_TYPE_CHANGECIPHERSPEC:
/* after processing each individual record, you are responsible to set
 * connStatePtr->scsReadPktPtr and cut the link of readMsgPtr
 */
#ifdef TM_USE_SSL_CLIENT
            if (connStatePtr->scsClientOrServer == TM_SSL_CLIENT)
            {
                errorCode = tfSslParseServerCCS(connStatePtr, newPacketPtr);
                if (errorCode == TM_EOPNOTSUPP)
                {
                    (void)tfSslBuildSendAlert(connStatePtr,
                                              TM_SSL_ALTLEV_FATAL,
                                              TM_SSL_ALT_UNEXPECTEDMSG);
                }
            }
#endif /*TM_USE_SSL_CLIENT*/
#ifdef TM_USE_SSL_SERVER
            if (connStatePtr->scsClientOrServer == TM_SSL_SERVER)
            {
                if (((connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_SERVER_SENDCR)
                         || (    (connStatePtr->scsState != TM_SSL_ST_RECV_CH2)
                              && (connStatePtr->scsState != TM_SSL_ST_RECV_CKE)
                            )
                    )
                    && (connStatePtr->scsState != TM_SSL_ST_RECV_CERTVERI))
                {
/* if server asked certificate, but client send back CCS without sending Cert
 * verification first, it will be in invalid state and needs termination
 */
                    (void)tfSslBuildSendAlert(connStatePtr,
                                              TM_SSL_ALTLEV_FATAL,
                                              TM_SSL_ALT_HANDSHAKEFAIL);
                    errorCode = TM_EINVAL;
                }
                else
                {
                    errorCode = tfSslParseClientCCS(connStatePtr, newPacketPtr);
                }
            }
#endif /*TM_USE_SSL_SERVER*/

/* we should take out the needFreePacket testing part of all incoming
            procedure !!! */

            break;
        case TM_SSL_TYPE_HANDSHAKE:
/* after processing each individual record, you are responsible to set
 * connStatePtr->scsReadPktPtr and cut the link of readMsgPtr
 */
            errorCode = tfSslIncomingHandShake(connStatePtr, newPacketPtr);

            break;
        case TM_SSL_TYPE_ALERT:
            errorCode = tfSslIncomingAlert(connStatePtr, newPacketPtr);
            break;
        case TM_SSL_TYPE_APPLICATION:
/* for application data, if our SSL connection is not ready, discard it */
            if (
#ifdef TM_USE_SSL_CLIENT
                    (   (connStatePtr->scsClientOrServer == TM_SSL_CLIENT)
                     && (connStatePtr->scsState == TM_SSL_CT_OPENED) )
#endif /* TM_USE_SSL_CLIENT */
#if (defined(TM_USE_SSL_CLIENT) && defined(TM_USE_SSL_SERVER))
                 ||
#endif /* TM_USE_SSL_CLIENT && TM_USE_SSL_SERVER */
#ifdef TM_USE_SSL_SERVER
                    (   (connStatePtr->scsClientOrServer == TM_SSL_SERVER)
                     && (connStatePtr->scsState == TM_SSL_ST_OPENED) )
#endif /* TM_USE_SSL_SERVER */
                )
            {
                errorCode = tfSslIncomingApplicationData(connStatePtr);
                if (errorCode == TM_ENOERROR)
                {
                    if (context != TM_SSL_RECV_API)
                    {
/* In TCP incoming path, or user SSL task */
/* Netscape will send application data with handshake data together */
                        connStatePtr->scsSocCBFlags |= TM_CB_RECV;
                    }
                }
                if (errorCode == TM_EWOULDBLOCK)
/* Empty record */
                {
                    errorCode = TM_ENOERROR;
                }
            }
            else
            {
                errorCode = TM_EINVAL;
                (void)tfSslBuildSendAlert(connStatePtr,
                                          TM_SSL_ALTLEV_FATAL,
                                          TM_SSL_ALT_UNEXPECTEDMSG);
            }
            break;
        default:
/* we don't know when to stop free, so free all of them */
            errorCode = TM_EPROTONOSUPPORT;
            (void)tfSslBuildSendAlert(connStatePtr,
                                      TM_SSL_ALTLEV_FATAL,
                                      TM_SSL_ALT_UNEXPECTEDMSG);
            break;
        }

/* we have cut the link of recv SSL buffer queue, need decrease the buffer
 * size*/
        tcpVectPtr->tcpsSslRecvQBytes -=
               connStatePtr->scsNextReadRecordSize;


/* you should not free anytime you processed a full record. Sometime,
 * a full message may have multiple records
 */

        if (newPacketPtr)
        {
            tfFreePacket(newPacketPtr, TM_SOCKET_LOCKED);
        }

        if (errorCode != TM_ENOERROR)
        {
            goto sslIncomingRecordExit;
        }
        nextPacketPtr = connStatePtr->scsReadPktPtr;

        connStatePtr->scsNextReadRecordSize = 0;
        connStatePtr->scsNextReadRecordSizeToGo = 0;
        connStatePtr->scsHeaderBufferCount = 0;
/* you may got more than one records, testing if we have any more */
        errorCode = tfSslHasMoreRecord(connStatePtr,
                                       (ttPacketPtr)0,
                                       &hasMoreRecord);
        if (errorCode == TM_ENOERROR)
        {
            if (hasMoreRecord == TM_8BIT_NO)
            {
                break;
            }
        }
        else
        {
            errorCode = TM_EINVAL;
            goto sslIncomingRecordExit;
        }
    }

sslIncomingRecordExit:
    tm_16bit_clr_bit(tcpVectPtr->tcpsSslFlags, TM_SSLF_HANDSHK_PROCESS);
    if (errorCode != TM_ENOERROR)
    {
/* we won't abort the TCP connection (consider FTP over SSL), just close SSL
 * layer. We let user know this, so that they can do what they want.
 */
        if (errorCode == TM_ENOBUFS)
        {
            (void)tfSslBuildSendAlert(connStatePtr,
                                      TM_SSL_ALTLEV_FATAL,
                                      TM_SSL_ALT_INTERNALERR);
        }
        if (tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                                 TM_SSLF_HANDSHK_FAILURE))
        {
            connStatePtr->scsNextReadRecordSize = 0;
            connStatePtr->scsNextReadRecordSizeToGo = 0;
            connStatePtr->scsHeaderBufferCount = 0;
            tcpVectPtr->tcpsSslRecvQBytes = 0;
            tm_16bit_clr_bit(tcpVectPtr->tcpsSslFlags, TM_SSLF_MORE_RECORDS);
            if(connStatePtr->scsReadPktPtr)
            {
                tfFreePacket(connStatePtr->scsReadPktPtr, TM_SOCKET_LOCKED);
                connStatePtr->scsReadPktPtr = (ttPacketPtr)0;
                connStatePtr->scsReadPktTailPtr = (ttPacketPtr)0;
            }
        }
        tfSslClose(connStatePtr, 0);
    }
    cbFlags = connStatePtr->scsSocCBFlags;
    connStatePtr->scsSocCBFlags = TM_16BIT_ZERO;
#ifdef TM_USE_SSL_CLIENT
    if (connStatePtr->scsClientOrServer == TM_SSL_CLIENT)
    {
        if (connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_TCP_CONNECTED)
/* we need notify the TCP state machine for any pending state transition */
        {
            connStatePtr->scsRunFlags &= ~(unsigned)TM_SSL_RUNFLAG_TCP_CONNECTED;
/* connStatePtr might not be valid after this call */
            tfTcpSslConnected(tcpVectPtr);
            connStatePtr = tcpVectPtr->tcpsSslConnStatePtr;
            if (    (connStatePtr != (ttSslConnectStatePtr)0)
                 && (connStatePtr->scsSocCBFlags != 0) )
            {
                cbFlags |= connStatePtr->scsSocCBFlags;
                connStatePtr->scsSocCBFlags = TM_16BIT_ZERO;
            }
        }
    }
#endif /* TM_USE_SSL_CLIENT */
    if (cbFlags != TM_16BIT_ZERO)
    {
        if (cbFlags == TM_CB_RECV)
        {
            selectFlags = TM_SELECT_READ;
        }
        else
        {
            if (cbFlags == TM_CB_SSL_ESTABLISHED)
            {
                selectFlags = TM_SELECT_WRITE;
            }
            else
            {
                selectFlags = TM_SELECT_READ | TM_SELECT_WRITE;
            }
        }
        if (context != TM_SSL_TCP_INCOMING)
        {
/*
 * This function is not called from the incoming TCP path.
 * Notify the user directly.
 * Make sure conSatePtr is not referenced after the notification.
 */
            tfSocketNotify(&(tcpVectPtr->tcpsSocketEntry),
                           selectFlags,
                           cbFlags,
                           0);
        }
        else
        {
/*
 * User notification required.
 * Since This function is called from the incoming TCP path,
 * Let TCP notify the user, so that we do not have to do it here, and
 * therefore do not have to unlock/relock. That way our connStatePtr stays
 * valid, and our TCP state machine is not messed up..
 */
            tcpVectPtr->tcpsSelectFlags |= selectFlags;
            tcpVectPtr->tcpsSocketCBFlags |= cbFlags;
        }
    }
    if (context != TM_SSL_TCP_INCOMING)
    {
/* If we built some SSL message while processing, make sure to send it. */
        if (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags2, TM_TCPF2_SEND_DATA))
        {
            if (    (tcpVectPtr->tcpsState == TM_TCPS_ESTABLISHED)
                 || (tcpVectPtr->tcpsState == TM_TCPS_CLOSE_WAIT) )
            {
/* TCP vector/socket entry unlocked/relocked in this call */
                errorCode = tfTcpSendPacket(tcpVectPtr);
            }
        }
    }
    return errorCode;
}

/* When client build CV, or server verify CV, both will call this
 * module. This module is only used when mutual authentication is
 * used
 */
#ifdef TM_SSL_USE_MUTUAL_AUTH

int tfSslBuildCertVerify(ttSslConnectStatePtr     connStatePtr,
                         tt8BitPtr                dataPtr,
                         tt32BitPtr               certVeriLenPtr)
{
    ttMd5CtxPtr               md5Ptr;
    ttSha1CtxPtr              sha1Ptr;
#ifdef TM_USE_SSL_VERSION_33
    ttSha256CtxPtr            sha256Ptr;
#endif /* TM_USE_SSL_VERSION_33 */
    ttPkiCertListPtr          certUsedPtr;
    ttCryptoRequest           request;
#ifdef TM_PUBKEY_USE_RSA
    ttCryptoRequestRsarequest rsareq;
#endif /* TM_PUBKEY_USE_RSA */
#ifdef TM_PUBKEY_USE_DSA
    ttCryptoRequestDsarequest dsareq;
#endif /* TM_PUBKEY_USE_DSA */
    tt8BitPtr                 digestOutPtr;
    int                       errorCode;
    int                       certVeriLen;
    int                       pkiType;
    tt8Bit                    signAlgorithm;
    tt8Bit                    signature;
#ifdef TM_USE_SSL_VERSION_33
    tt8Bit                    hashAlg;
    tt8Bit                    signAlg;
    tt8Bit                    hashAlgorithm;
    tt8Bit                    suppAlg;
    int                       digestInfoLen;
    int                       digestOutLen;
#endif /* TM_USE_SSL_VERSION_33 */

    errorCode = 0;
    digestOutPtr = (tt8BitPtr)0;
    md5Ptr = (ttMd5CtxPtr)0;
    sha1Ptr = (ttSha1CtxPtr)0;
#ifdef TM_USE_SSL_VERSION_33
    sha256Ptr = (ttSha256CtxPtr)0;
    hashAlgorithm = (tt8Bit)0;
    digestInfoLen = (int)0;
    digestOutLen = (int)0;
#endif /* TM_USE_SSL_VERSION_33 */
    tm_bzero(&request, sizeof(request));
    certVeriLen = 0;
    signAlgorithm = 0;
    signature = TM_8BIT_NO;
#ifdef TM_PUBKEY_USE_RSA
    tm_bzero(&rsareq, sizeof(ttCryptoRequestRsarequest));
#endif /* TM_PUBKEY_USE_RSA */
#ifdef TM_PUBKEY_USE_DSA
    tm_bzero(&dsareq, sizeof(ttCryptoRequestDsarequest));
#endif /* TM_PUBKEY_USE_DSA */

    if (connStatePtr->scsClientOrServer == TM_SSL_CLIENT)
    {
#ifdef TM_USE_SSL_VERSION_33
        if (connStatePtr->scsVersion == TM_SSL_VERSION_33)
        {
            certUsedPtr = connStatePtr->scsSessionPtr->ssCertSuppPtr;
        }
        else
#endif /* TM_USE_SSL_VERSION_33 */
        {
            certUsedPtr = connStatePtr->scsSessionPtr->ssCertListPtr;
        }
        signature = TM_8BIT_YES;
    }
    else
    {
        certUsedPtr = connStatePtr->scsPeerCertListPtr;
        certVeriLen = (int)*certVeriLenPtr;
    }
#ifdef TM_USE_SSL_VERSION_33
    if ((connStatePtr->scsVersion == TM_SSL_VERSION_33)
     && (connStatePtr->scsClientOrServer == TM_SSL_SERVER))
    {
        hashAlg = *dataPtr;
        signAlg = *(dataPtr + 1);
/* HashAlgorithm */
        switch(hashAlg)
        {
            case TM_SSL_SUPPORT_HASH_MD5:
                hashAlgorithm = SADB_AALG_MD5HMAC;
                break;
            case TM_SSL_SUPPORT_HASH_SHA1:
                hashAlgorithm = SADB_AALG_SHA1HMAC;
                break;
            case TM_SSL_SUPPORT_HASH_SHA256:
                hashAlgorithm = SADB_AALG_SHA256HMAC;
                break;
            default :
                errorCode = TM_EOPNOTSUPP;
                goto sslBuildCertVeirfyExit;
                break;
        }
/* SignatureAlgorithm*/
        switch(signAlg)
        {
            case TM_SSL_SUPPORT_SIG_RSA:
                signAlgorithm = SADB_PUBKEY_RSA;
                break;
            case TM_SSL_SUPPORT_SIG_DSA:
                signAlgorithm = SADB_PUBKEY_DSA;
                break;
            default :
                errorCode = TM_EOPNOTSUPP;
                goto sslBuildCertVeirfyExit;
                break;
        }
        dataPtr = dataPtr + 2;
    }
    else if ((connStatePtr->scsVersion == TM_SSL_VERSION_33)
          && (connStatePtr->scsClientOrServer == TM_SSL_CLIENT))
    {
        suppAlg = certUsedPtr->certDigestIndex;
        switch (suppAlg)
        {
            case TM_PKI_OBJ_RSAMD5 :
                hashAlg = TM_SSL_SUPPORT_HASH_MD5;
                signAlg = TM_SSL_SUPPORT_SIG_RSA;
                hashAlgorithm = SADB_AALG_MD5HMAC;
                signAlgorithm = SADB_PUBKEY_RSA;
                break;
            case TM_PKI_OBJ_RSASHA1 :
                hashAlg = TM_SSL_SUPPORT_HASH_SHA1;
                signAlg = TM_SSL_SUPPORT_SIG_RSA;
                hashAlgorithm = SADB_AALG_SHA1HMAC;
                signAlgorithm = SADB_PUBKEY_RSA;
                break;
            case TM_PKI_OBJ_RSASHA256 :
                hashAlg = TM_SSL_SUPPORT_HASH_SHA256;
                signAlg = TM_SSL_SUPPORT_SIG_RSA;
                hashAlgorithm = SADB_AALG_SHA256HMAC;
                signAlgorithm = SADB_PUBKEY_RSA;
                break;
            case TM_PKI_OBJ_DSASHA1 :
                hashAlg = TM_SSL_SUPPORT_HASH_SHA1;
                signAlg = TM_SSL_SUPPORT_SIG_DSA;
                hashAlgorithm = SADB_AALG_SHA1HMAC;
                signAlgorithm = SADB_PUBKEY_DSA;
                break;
            case TM_PKI_OBJ_RSA :
            case TM_PKI_OBJ_RSAMD2 :
            case TM_PKI_OBJ_RSASHA384 :
            case TM_PKI_OBJ_RSASHA512 :
            case TM_PKI_OBJ_RSARIPEMD :
            case TM_PKI_OBJ_DSA :
            default :
                hashAlg = (tt8Bit)255;
                signAlg = (tt8Bit)255;
                break;
        }
        *dataPtr = hashAlg;
        *(dataPtr + 1) = signAlg;
        dataPtr = dataPtr + 2;
    }
    else
#endif /* TM_USE_SSL_VERSION_33 */
    {
        pkiType = certUsedPtr->certKeyPtr->pkType;
        if (pkiType == TM_PKI_OBJ_DSA  ||
            pkiType == TM_PKI_OBJ_DSASHA1)
        {
            signAlgorithm = SADB_PUBKEY_DSA;
        }
        else if (pkiType == TM_PKI_OBJ_RSA    ||
                 pkiType == TM_PKI_OBJ_RSAMD5 ||
                 pkiType == TM_PKI_OBJ_RSASHA1||
                 pkiType == TM_PKI_OBJ_RSASHA256||
                 pkiType == TM_PKI_OBJ_RSARIPEMD)
        {
            signAlgorithm  = SADB_PUBKEY_RSA;
        }
    }
#ifdef TM_USE_SSL_VERSION_33
    if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
        switch(hashAlgorithm){
            case SADB_AALG_MD5HMAC :
                digestOutLen = TM_MD5_HASH_SIZE;
#ifdef TM_PUBKEY_USE_RSA
                if (signAlgorithm == SADB_PUBKEY_RSA)
                {
                    digestInfoLen = TM_MD5_DIGESTINFO_LEN;
                }
#endif /* TM_PUBKEY_USE_RSA */
                md5Ptr = tm_get_raw_buffer(sizeof(ttMd5Ctx));
                if (!md5Ptr)
                {
                    errorCode = TM_ENOBUFS;
                    goto sslBuildCertVeirfyExit;
                }
                tm_bcopy(connStatePtr->scsMd5CtxPtr,
                         md5Ptr,
                         sizeof(ttMd5Ctx));
                break;
            case SADB_AALG_SHA1HMAC :
                digestOutLen = TM_SHA1_HASH_SIZE;
#ifdef TM_PUBKEY_USE_RSA
                if (signAlgorithm == SADB_PUBKEY_RSA)
                {
                    digestInfoLen = TM_SHA1_DIGESTINFO_LEN;
                }
#endif /* TM_PUBKEY_USE_RSA */
                sha1Ptr = tm_get_raw_buffer(sizeof(ttSha1Ctx));
                if (!sha1Ptr)
                {
                    errorCode = TM_ENOBUFS;
                    goto sslBuildCertVeirfyExit;
                }
                tm_bcopy(connStatePtr->scsSha1CtxPtr,
                        sha1Ptr,
                        sizeof(ttSha1Ctx));
                break;
            case SADB_AALG_SHA256HMAC :
                digestOutLen = TM_SHA256_HASH_SIZE;
#ifdef TM_PUBKEY_USE_RSA
                if (signAlgorithm == SADB_PUBKEY_RSA)
                {
                    digestInfoLen = TM_SHA256_DIGESTINFO_LEN;
                }
#endif /* TM_PUBKEY_USE_RSA */
                sha256Ptr = tm_get_raw_buffer(sizeof(ttSha256Ctx));
                if (!sha256Ptr)
                {
                    errorCode = TM_ENOBUFS;
                    goto sslBuildCertVeirfyExit;
                }
                tm_bcopy(connStatePtr->scsSha256CtxPtr,
                        sha256Ptr,
                        sizeof(ttSha256Ctx));
                break;
            default :
                break;
        }
        digestOutPtr = tm_get_raw_buffer((ttPktLen)
                                         (digestOutLen + digestInfoLen));
        if (!digestOutPtr)
        {
            errorCode = TM_ENOBUFS;
            goto sslBuildCertVeirfyExit;
        }
    }
    else
#endif /* TM_USE_SSL_VERSION_33 */
    {
        digestOutPtr = tm_get_raw_buffer(TM_MD5_HASH_SIZE +
            TM_SHA1_HASH_SIZE);
        if (!digestOutPtr)
        {
            errorCode = TM_ENOBUFS;
            goto sslBuildCertVeirfyExit;
        }
        sha1Ptr = tm_get_raw_buffer(sizeof(ttSha1Ctx));
        if (!sha1Ptr)
        {
            errorCode = TM_ENOBUFS;
            goto sslBuildCertVeirfyExit;
        }
        tm_bcopy(connStatePtr->scsSha1CtxPtr,
            sha1Ptr,
            sizeof(ttSha1Ctx));
        if (signAlgorithm == SADB_PUBKEY_RSA)
        {
            md5Ptr = tm_get_raw_buffer(sizeof(ttMd5Ctx));
            if (!md5Ptr)
            {
                errorCode = TM_ENOBUFS;
                goto sslBuildCertVeirfyExit;
            }
            tm_bcopy(connStatePtr->scsMd5CtxPtr,
                md5Ptr,
                sizeof(ttMd5Ctx));
        }
    }

/* scsVersion can be any of TM_SSL_VERSION_33 or
 * TM_SSL_VERSION_31 or TM_SSL_VERSION_30.
 */

#ifdef TM_USE_SSL_VERSION_33
    if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
        switch(hashAlgorithm)
        {
            case SADB_AALG_MD5HMAC :
#ifdef TM_PUBKEY_USE_RSA
                if (signAlgorithm == SADB_PUBKEY_RSA)
                {
                    tm_memcpy(digestOutPtr,
                              tlMd5DigestInfo,
                              digestInfoLen);
                }
#endif /* TM_PUBKEY_USE_RSA */
                tfMd5Final(digestOutPtr + digestInfoLen,
                           md5Ptr);
                break;
            case SADB_AALG_SHA1HMAC :
#ifdef TM_PUBKEY_USE_RSA
                if (signAlgorithm == SADB_PUBKEY_RSA)
                {
                    tm_memcpy(digestOutPtr,
                              tlSha1DigestInfo,
                              digestInfoLen);
                }
#endif /* TM_PUBKEY_USE_RSA */
                tfSha1Final(digestOutPtr + digestInfoLen,
                            sha1Ptr);
                break;
            case SADB_AALG_SHA256HMAC :
#ifdef TM_PUBKEY_USE_RSA
                if (signAlgorithm == SADB_PUBKEY_RSA)
                {
                    tm_memcpy(digestOutPtr,
                              tlSha256DigestInfo,
                              digestInfoLen);
                }
#endif /* TM_PUBKEY_USE_RSA */
                tfSha256Final(digestOutPtr + digestInfoLen,
                              sha256Ptr);
                break;
            default :
                break;
        }
    }
#ifdef TM_USE_SSL_VERSION_31
    else
#endif /* TM_USE_SSL_VERSION_31 */
#endif /* TM_USE_SSL_VERSION_33 */
#ifdef TM_USE_SSL_VERSION_31
    if (connStatePtr->scsVersion == TM_SSL_VERSION_31)
    {
        if (signAlgorithm == SADB_PUBKEY_RSA)
        {
            tfMd5Final(digestOutPtr, md5Ptr);
        }
        tfSha1Final(digestOutPtr + TM_MD5_HASH_SIZE,
            sha1Ptr);
    }
#endif /* TM_USE_SSL_VERSION_31 */
#ifdef TM_USE_SSL_VERSION_30
    if (connStatePtr->scsVersion == TM_SSL_VERSION_30)
    {
        errorCode = tfSsl30GetHandShakeHash(connStatePtr,
            md5Ptr,
            sha1Ptr,
            (char TM_FAR *)0,
            0,
            digestOutPtr);

    }
#endif /*  TM_USE_SSL_VERSION_30 */

/* We already get the signature part (digestOutPtr), use the corresponding
 * signature algorithm to sign it.
 */
#ifdef TM_PUBKEY_USE_RSA
    if (signAlgorithm == SADB_PUBKEY_RSA)
    {
        if (signature == TM_8BIT_YES)
        {
            request.crType = TM_CEREQUEST_PUBKEY_RSASIGN;
        }
        else
        {
            request.crType = TM_CEREQUEST_PUBKEY_RSAVERIFY;
        }
        request.crParamUnion.crRsaParamPtr   = &rsareq;
        rsareq.crrRsainfoPtr    = (ttGeneralRsaPtr)
            (certUsedPtr->certKeyPtr->pkKeyPtr);
#ifdef TM_USE_SSL_VERSION_33
        if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
        {
            rsareq.crrRsaDataLength = digestInfoLen
                                      + digestOutLen;

            if (connStatePtr->scsClientOrServer == TM_SSL_SERVER)
            {
                certVeriLen -= 2;
            }
        }
        else
#endif /*  TM_USE_SSL_VERSION_33 */
        {
            rsareq.crrRsaDataLength = TM_MD5_HASH_SIZE + TM_SHA1_HASH_SIZE;
        }
        rsareq.crrRsaDataPtr    = digestOutPtr;
        rsareq.crrRsaTransPtr   = dataPtr + 2;
        rsareq.crrRsaTransLenPtr = &certVeriLen;

    }
#endif /* TM_PUBKEY_USE_RSA */

#ifdef TM_PUBKEY_USE_DSA
    if (signAlgorithm == SADB_PUBKEY_DSA)
    {
        if (signature == TM_8BIT_YES)
        {
            request.crType = TM_CEREQUEST_PUBKEY_DSASIGN;
        }
        else
        {
            request.crType = TM_CEREQUEST_PUBKEY_DSAVERIFY;
        }
        request.crParamUnion.crDsaParamPtr   = &dsareq;
        dsareq.crdDsainfoPtr    = (ttGeneralDsaPtr)
            (certUsedPtr->certKeyPtr->pkKeyPtr);
#ifdef TM_USE_SSL_VERSION_33
        if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
        {
            dsareq.crdDsaDataLength =  digestOutLen;
            dsareq.crdDsaDataPtr    = digestOutPtr;
        }
        else
#endif /*  TM_USE_SSL_VERSION_33 */
        {
            dsareq.crdDsaDataLength = TM_SHA1_HASH_SIZE;
            dsareq.crdDsaDataPtr    = digestOutPtr + TM_MD5_HASH_SIZE;
        }
        dsareq.crdDsaTransPtr   = dataPtr + 2;
        dsareq.crdDsaTransLenPtr = &certVeriLen;
    }
#endif /* TM_PUBKEY_USE_DSA */

    errorCode = connStatePtr->scsPubkeyEnginePtr->
        ceSessionProcessFuncPtr(&request);

#ifdef TM_USE_SSL_VERSION_33
/* accept without DigestInfo.AlgorithmIdentifier.parameters field */
    if ((connStatePtr->scsVersion == TM_SSL_VERSION_33)
        &&(signAlgorithm == SADB_PUBKEY_RSA)
        &&(errorCode != TM_ENOERROR))
    {
        switch(hashAlgorithm)
        {
            case SADB_AALG_MD5HMAC :
                tm_memcpy(digestOutPtr,
                          tlMd5DigestInfoNull,
                          digestInfoLen - 2);
                tfMd5Final(digestOutPtr + digestInfoLen - 2,
                           md5Ptr);
                break;
            case SADB_AALG_SHA1HMAC :
                tm_memcpy(digestOutPtr,
                          tlSha1DigestInfoNull,
                          digestInfoLen - 2);
                tfSha1Final(digestOutPtr + digestInfoLen - 2,
                            sha1Ptr);
                break;
            case SADB_AALG_SHA256HMAC :
                tm_memcpy(digestOutPtr,
                          tlSha256DigestInfoNull,
                          digestInfoLen - 2);
                tfSha256Final(digestOutPtr + digestInfoLen - 2,
                              sha256Ptr);
                break;
            default :
                break;
        }
#ifdef TM_PUBKEY_USE_RSA
        if (signAlgorithm == SADB_PUBKEY_RSA)
        {
            rsareq.crrRsaDataPtr    = digestOutPtr;
            rsareq.crrRsaDataLength -= 2;
        }
#ifdef TM_PUBKEY_USE_DSA
        else
#endif /* TM_PUBKEY_USE_DSA */
#endif /* TM_PUBKEY_USE_RSA */
#ifdef TM_PUBKEY_USE_DSA
        if (signAlgorithm == SADB_PUBKEY_DSA)
        {
            dsareq.crdDsaDataPtr    = digestOutPtr;
            dsareq.crdDsaDataLength -= 2;
        }
#endif /* TM_PUBKEY_USE_DSA */
        errorCode = connStatePtr->scsPubkeyEnginePtr->
            ceSessionProcessFuncPtr(&request);
    }
#endif /* TM_USE_SSL_VERSION_33 */
    if (errorCode != TM_ENOERROR)
    {
        goto sslBuildCertVeirfyExit;
    }
/* we always treat the signature like opaque struture */
    if ( signature == TM_8BIT_YES)
    {
        *dataPtr = (tt8Bit)(certVeriLen >> 8);
        *(dataPtr + 1) = (tt8Bit)(certVeriLen & 0xff);
        certVeriLen += 2;
#ifdef TM_USE_SSL_VERSION_33
        if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
        {
/* Algorithm field length */
            certVeriLen += 2;
        }
#endif /* TM_USE_SSL_VERSION_33 */
    }

sslBuildCertVeirfyExit:
    if (digestOutPtr)
    {
        tm_free_raw_buffer(digestOutPtr);
    }
    if (md5Ptr)
    {
        tm_free_raw_buffer((ttRawBufferPtr)md5Ptr);
    }
    if (sha1Ptr)
    {
        tm_free_raw_buffer((ttRawBufferPtr)sha1Ptr);
    }
#ifdef TM_USE_SSL_VERSION_33
    if (sha256Ptr)
    {
        tm_free_raw_buffer((ttRawBufferPtr)sha256Ptr);
    }
#endif /* TM_USE_SSL_VERSION_33 */
    *certVeriLenPtr = (tt32Bit)certVeriLen;
    return errorCode;
}
#endif /*TM_SSL_USE_MUTUAL_AUTH */

/* The common routine to do SSL encoding and decoding */
int tfSslHandshakeMsgTransform(ttSslConnectStatePtr         connStatePtr,
                               int                          direction,
                               ttSslRecordHeaderPtr         sslHeaderPtr,
                               ttPacketPtr                  packetPtr,
                               tt8BitPtr                    macOutPtr,
                               tt16BitPtr                   recordLenPtr,
                               tt16Bit                      headOffset)
{
    ttCryptoRequest           request;
    ttCryptoRequestSsl        sslRequest;

    tm_bzero(&request, sizeof(request));
    request.crType = TM_CEREQUEST_CRYPTO_SSL;
    request.crFlags = (ttUser8Bit)direction;
    request.crParamUnion.crSslParamPtr = &sslRequest;
    sslRequest.crsConnectStatePtr = connStatePtr;
    sslRequest.crsRecordLenPtr = recordLenPtr;
    sslRequest.crsCryptHeadLength = headOffset;
    sslRequest.crsPacketPtr = (ttUserPacketPtr)packetPtr;
    sslRequest.crsMacOutPtr = macOutPtr;
    sslRequest.crsSslHeaderPtr = sslHeaderPtr;
    return connStatePtr->scsCryptoEnginePtr->
                  ceSessionProcessFuncPtr(&request);

}

/*
 * Build and send FINISHED handshake message
 */
int tfSslBuildSendFinished(ttSslConnectStatePtr   connStatePtr,
                           ttVoidPtr              paramPtr)
{
    ttSslRecordHeaderPtr      sslHeaderPtr;
    ttSslHandShakeHeaderPtr   handshakeHeaderPtr;
    ttMd5CtxPtr               md5Ptr;
    ttSha1CtxPtr              sha1Ptr;
    ttSha256CtxPtr            sha256Ptr;
    tt8BitPtr                 digestOutPtr;
    tt8BitPtr                 dataPtr;
#if (defined(TM_USE_SSL_VERSION_30) || defined(TM_USE_SSL_VERSION_31)\
  || defined(TM_USE_SSL_VERSION_33))
    char TM_FAR *             finishLabelPtr;
    int                       finishLabelLen;
    tt8Bit                    clientOrServer;
#endif /* TM_USE_SSL_VERSION_30 || TM_USE_SSL_VERSION_31
       || TM_USE_SSL_VERSION_33 */
    int                       dataLength;
    int                       errorCode;
    tt16Bit                   finishLength;
    tt8Bit                    ivLength;


    errorCode = TM_ENOERROR;
    TM_UNREF_IN_ARG(paramPtr);
    finishLength = 0;
    ivLength = (tt8Bit)0;
    md5Ptr = (ttMd5CtxPtr)0;
    sha1Ptr = (ttSha1CtxPtr)0;
    sha256Ptr = (ttSha256CtxPtr)0;
    dataPtr = connStatePtr->scsXmitDataPtr;
#if (defined(TM_USE_SSL_VERSION_30) || defined(TM_USE_SSL_VERSION_31)\
  || defined(TM_USE_SSL_VERSION_33))
    clientOrServer = connStatePtr->scsClientOrServer;
#endif /* TM_USE_SSL_VERSION_30 || TM_USE_SSL_VERSION_31
       || TM_USE_SSL_VERSION_33 */
/* 1. Construct part of the SSL record header */
    sslHeaderPtr = (ttSslRecordHeaderPtr)dataPtr;
    sslHeaderPtr->srhType = TM_SSL_TYPE_HANDSHAKE;
    sslHeaderPtr->srhVersionMajor = TM_SSL_VERSION_MAJOR;
#ifdef TM_USE_SSL_VERSION_33
    if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
        sslHeaderPtr->srhVersionMinor = TM_SSL_VERSION_MINOR3;
    }
    else
#endif /* TM_USE_SSL_VERSION_33 */
    {
        sslHeaderPtr->srhVersionMinor = (tt8Bit)
                  (connStatePtr->scsVersion >> 1);
    }
    dataPtr += TM_SSL_RECORD_HEADER_SIZE;
/* 1.2 insert IV field in case of CBC. */
#ifdef TM_USE_SSL_VERSION_33
    if((connStatePtr->scsVersion == TM_SSL_VERSION_33)
     &&((connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_ARCFOUR)
      &&(connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_NULL)))
    {
        ivLength = connStatePtr->scsCryptoAlgPtr->ealgBlockSize;
        tfGetRandomBytes(connStatePtr->scsEncodeIvPtr, ivLength);
        tm_bcopy(connStatePtr->scsEncodeIvPtr, dataPtr, ivLength);
        dataPtr += ivLength;
#ifdef TM_SSL_DEBUG
/* print the key blocks */
        tm_debug_log0("\nRandom IV = \n");
        tfCryptoDebugPrintf((ttCharPtrPtr)0,
                            connStatePtr->scsEncodeIvPtr,
                            ivLength ,
                            (ttIntPtr)0);
#endif /* TM_SSL_DEBUG */
    }
#endif /* TM_USE_SSL_VERSION_33 */
/* 2 dataPtr here points to the handshake message*/
    handshakeHeaderPtr = (ttSslHandShakeHeaderPtr)dataPtr;
    handshakeHeaderPtr->hsHandShakeType = TM_SSL_FINISHED;
    handshakeHeaderPtr->hsLengthPad = 0;

    dataPtr += TM_SSL_HANDSHAKE_HEADER_SIZE;

#ifdef TM_USE_SSL_VERSION_33
/* 3 compute the verify data [12] or SSL finished message [36]*/
    if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
/* 3.1 Compute SHA256(handshake_messages)*/
        digestOutPtr = tm_get_raw_buffer(TM_SHA256_HASH_SIZE);
        sha256Ptr = tm_get_raw_buffer(sizeof(ttSha256Ctx));
        if ((!digestOutPtr) || (!sha256Ptr))
        {
            errorCode = TM_ENOBUFS;
            goto sslBuildSendFinishedExit;
        }
        tm_bcopy(connStatePtr->scsSha256CtxPtr, sha256Ptr,sizeof(ttSha256Ctx));
    }
    else
#endif /* TM_USE_SSL_VERSION_33 */
    {
/* 3.1 Compute MD5(handshake_messages) and SHA1(handshake_messages)*/
        digestOutPtr = tm_get_raw_buffer(TM_MD5_HASH_SIZE +
            TM_SHA1_HASH_SIZE);
        md5Ptr = tm_get_raw_buffer(sizeof(ttMd5Ctx));
        sha1Ptr = tm_get_raw_buffer(sizeof(ttSha1Ctx));
        if ((!digestOutPtr) || (!md5Ptr) || (!sha1Ptr))
        {
            errorCode = TM_ENOBUFS;
            goto sslBuildSendFinishedExit;
        }
        tm_bcopy(connStatePtr->scsMd5CtxPtr, md5Ptr,sizeof(ttMd5Ctx));
        tm_bcopy(connStatePtr->scsSha1CtxPtr, sha1Ptr,sizeof(ttSha1Ctx));
    }

#ifdef TM_USE_SSL_VERSION_33
    if (connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
/* 3.2 Compute SHA2(handshake_messages)*/
        finishLength = TM_SSL_V31_FINISH_VERIFY_SIZE;
        tfSha256Final(digestOutPtr, sha256Ptr);
/* 3.3 compute verify data */
        if (clientOrServer == TM_SSL_CLIENT)
        {
            finishLabelPtr = TM_SSL31_CLIENT_FINISH_LABEL;
        }
        else
        {
            finishLabelPtr = TM_SSL31_SERVER_FINISH_LABEL;
        }
        finishLabelLen = (int) tm_strlen(finishLabelPtr);
        errorCode = tfTlsPrf2(connStatePtr,
                              connStatePtr->scsMasterSecret,
                              TM_SSL_MASTER_SECRET_SIZE,
                              finishLabelPtr,
                              finishLabelLen,
                              digestOutPtr,
                              TM_SHA256_HASH_SIZE,
                              dataPtr,
                              TM_SSL_V31_FINISH_VERIFY_SIZE);
        if (errorCode != TM_ENOERROR)
        {
            goto sslBuildSendFinishedExit;
        }
    }
#endif /* TM_USE_SSL_VERSION_33 */

#ifdef TM_USE_SSL_VERSION_31
    if (connStatePtr->scsVersion == TM_SSL_VERSION_31)
    {
/* 3.2 Compute MD5(handshake_messages) and SHA1(handshake_messages)*/
        finishLength = TM_SSL_V31_FINISH_VERIFY_SIZE;
        tfMd5Final(digestOutPtr, md5Ptr);
        tfSha1Final(digestOutPtr + TM_MD5_HASH_SIZE,sha1Ptr);
/* 3.3 compute verify data */
        if (clientOrServer == TM_SSL_CLIENT)
        {
            finishLabelPtr = TM_SSL31_CLIENT_FINISH_LABEL;
        }
        else
        {
            finishLabelPtr = TM_SSL31_SERVER_FINISH_LABEL;
        }
        finishLabelLen = (int) tm_strlen(finishLabelPtr);
        errorCode = tfTlsPrf(connStatePtr,
                             connStatePtr->scsMasterSecret,
                             TM_SSL_MASTER_SECRET_SIZE,
                             finishLabelPtr,
                             finishLabelLen,
                             digestOutPtr,
                             TM_MD5_HASH_SIZE+TM_SHA1_HASH_SIZE,
                             dataPtr,
                             TM_SSL_V31_FINISH_VERIFY_SIZE);
        if (errorCode != TM_ENOERROR)
        {
            goto sslBuildSendFinishedExit;
        }
    }
#endif /* TM_USE_SSL_VERSION_31 */

#ifdef TM_USE_SSL_VERSION_30
    if (connStatePtr->scsVersion == TM_SSL_VERSION_30)
    {
        if (clientOrServer == TM_SSL_CLIENT)
        {
            finishLabelPtr = TM_SSL30_CLIENT_FINISH_LABEL;
        }
        else
        {
            finishLabelPtr = TM_SSL30_SERVER_FINISH_LABEL;
        }
        finishLabelLen = (int) tm_strlen(finishLabelPtr);
        errorCode = tfSsl30GetHandShakeHash(connStatePtr,
                                            md5Ptr,
                                            sha1Ptr,
                                            finishLabelPtr,
                                            finishLabelLen,
                                            dataPtr);
        if (errorCode != TM_ENOERROR)
        {
            goto sslBuildSendFinishedExit;
        }
        finishLength = TM_MD5_HASH_SIZE + TM_SHA1_HASH_SIZE;
    }
#endif /* TM_USE_SSL_VERSION_30 */
/* 4. Set the handshake message header */
    handshakeHeaderPtr->hsLengthHigh = (tt8Bit)(finishLength >> 8);
    handshakeHeaderPtr->hsLengthLow = (tt8Bit)(finishLength & 0xff);

/* 5. Update the Hash algorithm context */
    finishLength = (tt16Bit)(finishLength + TM_SSL_HANDSHAKE_HEADER_SIZE);
/* we need put this length into SSL header because we are going to use this
 * field to calculate the MAC
 */
    sslHeaderPtr->srhLengthHigh = (tt8Bit)(finishLength >> 8);
    sslHeaderPtr->srhLengthLow = (tt8Bit)(finishLength & 0xff);

#ifdef TM_USE_SSL_VERSION_33
    if (connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
        tfSha256Update(connStatePtr->scsSha256CtxPtr,
                       (tt8BitPtr)handshakeHeaderPtr,
                       finishLength,
                       0);
    }
#endif /* TM_USE_SSL_VERSION_33 */
    tfSslUpdateMd5Sha1Ctx(connStatePtr,
                          (tt8BitPtr)handshakeHeaderPtr,
                          finishLength);

/* 6. Do SSL encapsulation of this record.
 * Note that: 1) The handshake header is fixed, You can not modify it.
 *            2) Do hashing across the whole SSL record
 *            3) Padding if necessory
 *            4) Set the new SSL record length
 *            5) Encryption the SSL record
 */

    errorCode = tfSslHandshakeMsgTransform(connStatePtr,
                                           TM_CRYPTOREQUEST_ENCODE,
                                           sslHeaderPtr,
                                           connStatePtr->scsXmitPacketPtr,
                                           (tt8BitPtr)handshakeHeaderPtr
                                           + finishLength,
                                           &finishLength,
                                           (tt16Bit)((tt8BitPtr)
                                           (handshakeHeaderPtr) -
                                           (connStatePtr->scsXmitPacketPtr
                                           ->pktLinkDataPtr)));
    if (errorCode != TM_ENOERROR)
    {
        goto sslBuildSendFinishedExit;
    }
    connStatePtr->scsXmitDataLength = (tt16Bit)
           (connStatePtr->scsXmitDataLength + TM_SSL_RECORD_HEADER_SIZE
            + finishLength);

/* 7. Send out the ssl record */
    dataLength = connStatePtr->scsXmitDataLength;
    connStatePtr->scsXmitPacketPtr->pktLinkDataLength = (ttPktLen)dataLength;
    connStatePtr->scsXmitPacketPtr->pktChainDataLength = (ttPktLen)dataLength;
    errorCode = tfSslSend(&connStatePtr->scsTcpVectPtr->tcpsSocketEntry,
                  connStatePtr,
                  &connStatePtr->scsXmitPacketPtr,
                  (ttPktLen TM_FAR *)&dataLength,
                  0);


/* 10. Update the following fields and */
    connStatePtr->scsXmitPacketPtr = 0;
    connStatePtr->scsNextWriteRecordSize = 0;
    connStatePtr->scsXmitDataPtr = 0;
    connStatePtr->scsXmitDataLength = 0;

/* we send finished to the peer. We should be able to receive */

sslBuildSendFinishedExit:
    if (digestOutPtr)
    {
        tm_free_raw_buffer(digestOutPtr);
    }
    if (md5Ptr)
    {
        tm_free_raw_buffer((ttRawBufferPtr)md5Ptr);
    }
    if (sha1Ptr)
    {
        tm_free_raw_buffer((ttRawBufferPtr)sha1Ptr);
    }
    if (sha256Ptr)
    {
        tm_free_raw_buffer((ttRawBufferPtr)sha256Ptr);
    }

    return errorCode;
}

/* Description: We receive a linked list packetPtr. We need to decide whether
 * we have a full SSL record or not.
 * If YES, we have a full SSL record, and the ssl type is APPLICATION data,
 * we need notify the user that there is some data for them to read. If the ssl
 * type is handshake, or alert, we go ahead and call tfSslIncomingRecord
 * ourselves, unless the user has set the option to process the handshake from
 * a user thread. 
 */
int tfSslIncomingPacket(ttTcpVectPtr     tcpVectPtr,
                        ttPacketPtr      packetPtr)
{
    ttSslConnectStatePtr     connStatePtr;
    int                      errorCode;
    tt16Bit                  cbFlags;
    tt8Bit                   hasMoreRecord;
    tt8Bit                   recordType;

    connStatePtr = tcpVectPtr->tcpsSslConnStatePtr;
    if (tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                             TM_SSLF_HANDSHK_FAILURE))
    {
        tfFreePacket(packetPtr, TM_SOCKET_LOCKED);
        connStatePtr->scsNextReadRecordSize = 0;
        connStatePtr->scsNextReadRecordSizeToGo = 0;
        connStatePtr->scsHeaderBufferCount = 0;
        tcpVectPtr->tcpsSslRecvQBytes = 0;
        tm_16bit_clr_bit(tcpVectPtr->tcpsSslFlags, TM_SSLF_MORE_RECORDS);
        if(connStatePtr->scsReadPktPtr)
        {
            tfFreePacket(connStatePtr->scsReadPktPtr, TM_SOCKET_LOCKED);
            connStatePtr->scsReadPktPtr = (ttPacketPtr)0;
            connStatePtr->scsReadPktTailPtr = (ttPacketPtr)0;
        }
        errorCode = TM_EWOULDBLOCK;
        goto sslIncomingPacketExit;
    }
    errorCode = tfSslHasMoreRecord(connStatePtr, packetPtr, &hasMoreRecord);
    if (errorCode == TM_ENOERROR)
    {
        errorCode = TM_EWOULDBLOCK;
        if (hasMoreRecord)
        {
            recordType = connStatePtr->scsHeaderBuffer[0];
            if (recordType != TM_SSL_TYPE_APPLICATION)
            {
                if (tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                                         TM_SSLF_USER_HANDSHK_PROCESS) )
                {
/* Tell the user to process the handshake message */
                    tcpVectPtr->tcpsSslFlags |= TM_SSLF_HANDSHK_PROCESS;
                    tcpVectPtr->tcpsSelectFlags |= TM_SELECT_EXCEPTION;
                    tcpVectPtr->tcpsSocketCBFlags |= (tt16Bit)TM_CB_SSL_HANDSHK_PROCESS;
/*
 * Don't return TM_ENOERROR, otherwise, select_read will be improperly
 * triggered by tfTcpReassemblePacket even if it is an SSL handshake message
 */
                }
                else
                {
                    errorCode = tfSslIncomingRecord(connStatePtr,
                                                    TM_SSL_TCP_INCOMING);
                    if (errorCode == TM_ENOERROR)
                    {
/*
 * Don't return TM_ENOERROR, otherwise, select_read will be improperly
 * triggered by tfTcpReassemblePacket, even if it is an SSL handshake message
 */
                        errorCode = TM_EWOULDBLOCK;
                    }
                }
            }
            else
            {
/*
 * else set errorCode to TM_ENOERROR so that TCP will set TM_SELECT_READ
 * and TM_CB_RECV
 */
                errorCode = TM_ENOERROR;
            }
        }
    }
    else
    {
        tfSslClose(connStatePtr, 0);
        cbFlags = connStatePtr->scsSocCBFlags;
        connStatePtr->scsSocCBFlags = TM_16BIT_ZERO;
        tcpVectPtr->tcpsSelectFlags |= (TM_SELECT_READ | TM_SELECT_WRITE);
        tcpVectPtr->tcpsSocketCBFlags |= cbFlags;
    }

sslIncomingPacketExit:
    return errorCode;
}


/* called by state machine. If we have any data in our queue before we finish
 * our ssl handshake negotiation, when we are done, we need send these queued
 * data immediately if send threshold condition is satisfied
 */
int tfSslSendQueuedData(ttSslConnectStatePtr    connStatePtr,
                        ttVoidPtr               paramPtr)
{
    ttPacketPtr       newPacketPtr;
    ttTcpVectPtr      tcpVectPtr;
    ttPktLen          sendBytes;
/* Test if we have any queued user data, if yes, we need build a SSL record
 * and send it out.
 */
    TM_UNREF_IN_ARG(paramPtr);
    tcpVectPtr = connStatePtr->scsTcpVectPtr;

    if (tcpVectPtr->tcpsSslSendQBytes > 0)
    {
        if (tm_16bit_one_bit_set(connStatePtr->scsWritePktPtr->pktFlags,
            MSG_OOB)
            || tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags, TCP_NODELAY)
            || (tcpVectPtr->tcpsSslSendQBytes >=
            (tt32Bit)tcpVectPtr->tcpsSslSendMinSize))
        {
            newPacketPtr = tfSslBuildRecordFromUserData
                          (tcpVectPtr, connStatePtr);
            if (newPacketPtr)
            {
                sendBytes = newPacketPtr->pktChainDataLength;
                tfSslSend(&tcpVectPtr->tcpsSocketEntry,
                          connStatePtr,
                          &newPacketPtr,
                          &sendBytes,
                          0); /* don't treat it as application data*/
            }
        }
    }
    return TM_ENOERROR;
}

/* parse FINISHED handshake message, after this call, if the finished
 * message is valid, we need cache the connection information, so
 * that later, we are able to resume this conenction
 */
int tfSslParseFinished(ttSslConnectStatePtr   connStatePtr,
                       tt8BitPtr              dataPtr,
                       tt16Bit                msgSize)
{
    ttSslSessionPtr           sslSessionPtr;
    ttTcpVectPtr              tcpVectPtr;
    ttMd5CtxPtr               md5Ptr;
    ttSha1CtxPtr              sha1Ptr;
#ifdef TM_USE_SSL_VERSION_33
    ttSha256CtxPtr            sha256Ptr;
#endif /* TM_USE_SSL_VERSION_33 */
    int                       isEqual;
#if (defined(TM_USE_SSL_VERSION_30) || defined(TM_USE_SSL_VERSION_31)\
  || defined(TM_USE_SSL_VERSION_33))
    char TM_FAR *             finishLabelPtr;
    int                       finishLabelLen;
    tt8Bit                    clientOrServer;
#endif /* TM_USE_SSL_VERSION_30 || TM_USE_SSL_VERSION_31
       || TM_USE_SSL_VERSION_33 */
    tt8BitPtr                 digestOutPtr;
    tt8BitPtr                 verifyDataPtr;
    int                       errorCode;
    tt16Bit                   finishLength;

    errorCode = TM_ENOERROR;
    digestOutPtr = (tt8BitPtr)0;
    md5Ptr = (ttMd5CtxPtr)0;
    sha1Ptr = (ttSha1CtxPtr)0;
#ifdef TM_USE_SSL_VERSION_33
    sha256Ptr = (ttSha256CtxPtr)0;
#endif /* TM_USE_SSL_VERSION_33 */
    verifyDataPtr = (tt8BitPtr)0;
    finishLength = 0;
    tcpVectPtr = connStatePtr->scsTcpVectPtr;
    sslSessionPtr = connStatePtr->scsSessionPtr;
/* for parsing, we should assume the peer's role */
#if (defined(TM_USE_SSL_VERSION_30) || defined(TM_USE_SSL_VERSION_31) \
    || defined(TM_USE_SSL_VERSION_33))
    clientOrServer = (tt8Bit)(!connStatePtr->scsClientOrServer);
#endif /* (defined(TM_USE_SSL_VERSION_30) || defined(TM_USE_SSL_VERSION_31))
    || defined(TM_USE_SSL_VERSION_33) */
/* 3 compute the verify data [verify_data_length] or SSL finished message [36]*/

/* 3.1 Compute MD5(handshake_messages) and SHA1(handshake_messages)*/
    if (connStatePtr->scsVersion == TM_SSL_VERSION_30)
    {
        verifyDataPtr = tm_get_raw_buffer(TM_MD5_HASH_SIZE +
                        TM_SHA1_HASH_SIZE);
    }
    else
    {
        verifyDataPtr = tm_get_raw_buffer(TM_SSL_V31_FINISH_VERIFY_SIZE);
    }

#ifdef TM_USE_SSL_VERSION_33
    if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
        digestOutPtr = tm_get_raw_buffer(TM_SHA256_HASH_SIZE);
        sha256Ptr = tm_get_raw_buffer(sizeof(ttSha256Ctx));
        if ((!digestOutPtr) || (!sha256Ptr) || (!verifyDataPtr))
        {
            errorCode = TM_ENOBUFS;
            goto sslParseFinishedExit;
        }
        tm_bcopy(connStatePtr->scsSha256CtxPtr, sha256Ptr,sizeof(ttSha256Ctx));
    }
    else
#endif /* TM_USE_SSL_VERSION_33 */
    {
        digestOutPtr = tm_get_raw_buffer(TM_MD5_HASH_SIZE +
                                         TM_SHA1_HASH_SIZE);
        md5Ptr = tm_get_raw_buffer(sizeof(ttMd5Ctx));
        sha1Ptr = tm_get_raw_buffer(sizeof(ttSha1Ctx));
        if ((!digestOutPtr) || (!md5Ptr) || (!sha1Ptr) || (!verifyDataPtr))
        {
            errorCode = TM_ENOBUFS;
            goto sslParseFinishedExit;
        }
        tm_bcopy(connStatePtr->scsMd5CtxPtr, md5Ptr,sizeof(ttMd5Ctx));
        tm_bcopy(connStatePtr->scsSha1CtxPtr, sha1Ptr,sizeof(ttSha1Ctx));
    }

#ifdef TM_USE_SSL_VERSION_33
    if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
/* 3.2 Compute SHA256(handshake_messages)*/
        finishLength = TM_SSL_V31_FINISH_VERIFY_SIZE;
        tfSha256Final(digestOutPtr, sha256Ptr);
/* 3.3 compute verify data */
        if (clientOrServer == TM_SSL_CLIENT)
        {
            finishLabelPtr = TM_SSL31_CLIENT_FINISH_LABEL;
        }
        else
        {
            finishLabelPtr = TM_SSL31_SERVER_FINISH_LABEL;
        }
        finishLabelLen = (int) tm_strlen(finishLabelPtr);
        errorCode = tfTlsPrf2(connStatePtr,
                              connStatePtr->scsMasterSecret,
                              TM_SSL_MASTER_SECRET_SIZE,
                              finishLabelPtr,
                              finishLabelLen,
                              digestOutPtr,
                              TM_SHA256_HASH_SIZE,
                              verifyDataPtr,
                              TM_SSL_V31_FINISH_VERIFY_SIZE);
        if (errorCode != TM_ENOERROR)
        {
            goto sslParseFinishedExit;
        }
    }
#endif /* TM_USE_SSL_VERSION_33 */

#ifdef TM_USE_SSL_VERSION_31
    if (connStatePtr->scsVersion == TM_SSL_VERSION_31)
    {
/* 3.2 Compute MD5(handshake_messages) and SHA1(handshake_messages)*/
        finishLength = TM_SSL_V31_FINISH_VERIFY_SIZE;
        tfMd5Final(digestOutPtr, md5Ptr);
        tfSha1Final(digestOutPtr + TM_MD5_HASH_SIZE,sha1Ptr);
/* 3.3 compute verify data */
        if (clientOrServer == TM_SSL_CLIENT)
        {
            finishLabelPtr = TM_SSL31_CLIENT_FINISH_LABEL;
        }
        else
        {
            finishLabelPtr = TM_SSL31_SERVER_FINISH_LABEL;
        }
        finishLabelLen = (int) tm_strlen(finishLabelPtr);
        errorCode = tfTlsPrf(connStatePtr,
                             connStatePtr->scsMasterSecret,
                             TM_SSL_MASTER_SECRET_SIZE,
                             finishLabelPtr,
                             finishLabelLen,
                             digestOutPtr,
                             TM_MD5_HASH_SIZE+TM_SHA1_HASH_SIZE,
                             verifyDataPtr,
                             TM_SSL_V31_FINISH_VERIFY_SIZE);
        if (errorCode != TM_ENOERROR)
        {
            goto sslParseFinishedExit;
        }
    }
#endif /* TM_USE_SSL_VERSION_31 */

#ifdef TM_USE_SSL_VERSION_30
    if (connStatePtr->scsVersion == TM_SSL_VERSION_30)
    {
        if (clientOrServer == TM_SSL_CLIENT)
        {
            finishLabelPtr = TM_SSL30_CLIENT_FINISH_LABEL;
        }
        else
        {
            finishLabelPtr = TM_SSL30_SERVER_FINISH_LABEL;
        }
        finishLabelLen = (int) tm_strlen(finishLabelPtr);
        errorCode = tfSsl30GetHandShakeHash(connStatePtr,
                                            md5Ptr,
                                            sha1Ptr,
                                            finishLabelPtr,
                                            finishLabelLen,
                                            verifyDataPtr);
        if (errorCode != TM_ENOERROR)
        {
            goto sslParseFinishedExit;
        }
        finishLength = TM_MD5_HASH_SIZE + TM_SHA1_HASH_SIZE;
    }
#endif /* TM_USE_SSL_VERSION_30 */

    isEqual = tm_memcmp(verifyDataPtr,dataPtr,finishLength);
    if ((msgSize == finishLength) && (isEqual == 0))
    {
        tcpVectPtr->tcpsSslFlags |= TM_SSLF_ESTABLISHED;

/*
 * If we enabled caching, and the remote gave us a session ID, and
 * if this handshake is not already a resumed one, we need to cache this
 * connection
 */
/* Lock session */
        tm_call_lock_wait(&sslSessionPtr->ssLockEntry);
        if ((sslSessionPtr->ssTotalCache > 0) &&
            (connStatePtr->scsSessionIdLength != 0) &&
            ((connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_RESUMESESSION) == 0))
        {
            tfSslCacheConnectInfo(connStatePtr);
        }

/* add this connection to session's activeconnect list, only if this is not
 * a rehandshake. For rehandshake, this connection is already in session's
 * activeconnect list
 */
        if(!(connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_REHANDSHAKE))
        {
            tfListAddToTail(&sslSessionPtr->ssActiveConnectList,
                            (ttNodePtr)connStatePtr);
        }
/* Unlock session */
        tm_call_unlock(&sslSessionPtr->ssLockEntry);
/* notify user that ssl established */
        connStatePtr->scsSocCBFlags |= TM_CB_SSL_ESTABLISHED;
#ifdef TM_USE_SSL_CLIENT
/*
 * We need notify the TCP state machine for any pending state transition.
 * Postpone doing so until we don't need to use connStatePtr.
 */
        connStatePtr->scsRunFlags |= TM_SSL_RUNFLAG_TCP_CONNECTED;
#endif /* TM_USE_SSL_CLIENT */
    }
    else
    {
        errorCode = TM_EINVAL;
    }

sslParseFinishedExit:
    if (md5Ptr)
    {
        tm_free_raw_buffer((ttRawBufferPtr)md5Ptr);
    }
    if (sha1Ptr)
    {
        tm_free_raw_buffer((ttRawBufferPtr)sha1Ptr);
    }
#ifdef TM_USE_SSL_VERSION_33
    if (sha256Ptr)
    {
        tm_free_raw_buffer((ttRawBufferPtr)sha256Ptr);
    }
#endif /* TM_USE_SSL_VERSION_33 */
    if (digestOutPtr)
    {
        tm_free_raw_buffer(digestOutPtr);
    }
    if (verifyDataPtr)
    {
        tm_free_raw_buffer(verifyDataPtr);
    }

    return errorCode;
}

/* This function is called to update the running md5 , and sha1 context.
 * When SSL negotiation finishes, it need to verify this hash value
 * to make sure both sides send and receive the same thing.
 * Parameter:
 * handshakeHeaderPtr           The pointer to the handshake message
 * msgLength                    msgLength is the whole handshake message
 *                              length, including the hand shake message header
 * RETURN:
 * none
 */
void tfSslUpdateMd5Sha1Ctx(ttSslConnectStatePtr   connStatePtr,
                           tt8BitPtr              handshakeHeaderPtr,
                           tt16Bit                msgLength)
{

    tfMd5Update(connStatePtr->scsMd5CtxPtr,
        handshakeHeaderPtr,
        msgLength,
        0);
    tfSha1Update(connStatePtr->scsSha1CtxPtr,
        handshakeHeaderPtr,
        msgLength,
        0);
    return;
}

/* This function is called to encrypt/decrypt linked packetPtr into one
 * single packet pointer newPacketPtr.  We always put the transformed results
 * to this new packet. We don't want to decrypt to user buffer directly,
 * because even if we want to decrypt to user buffer, we still need to check
 * 1) firstly if our queue has enough data for user, 2) update user's buffer
 * size, because we may have already provided something in our queue. 3)
 * decrypt something to user's remain buffer. 4) put the remain stuff to
 * queue again. We surely want to have a single solution, decrypt everything
 * to user socket queue. User buffer always gets refilled from user socket
 * queue. If user don't like any copy, user can always use tfZeroCopyRecv.
 *
 * The transformed packetPtr will be put to user's recv queue if
 * it is decoding direction. Otherwise, if it is encode direction,
 * it will be linked into socket send queue.
 *
 * IMPORTANT NOTE: After the transforming, the old packet linked list
 *                 will be freed!
 * PARAMETER:
 * packetPtr:    Points to the head of queued packet. It should have
 * already been disconnected from the queue. The last item, if it should
 * still leave in the queue, a duplicate packet should be used. So that
 * we can set the previous one pktLinkNextPtr to NULL, while the original
 * one be the queue head
 * newPacketPtr:  points to where we are going to write the transformed
 * results
 * DataLength: The number of bytes to encrypt or decrypt
 * Direction:   TM_SSL_ENCODE or TM_SSL_DECODE
 */

static int tfSslRecordPacketTransform(
                             ttSslConnectStatePtr     connStatePtr,
/* will be freed upon return*/
                             ttPacketPtr              packetPtr,
                             ttPktLen                 offset,
                             ttPacketPtr              newPacketPtr,
                             tt16Bit                  dataLength,
                             tt8Bit                   direction)
{
    ttGenericKeyPtr      gkeyPtr;
    ttPacketPtr          origHeadPacketPtr;
    tt8BitPtr            writeAddrPtr;
    ttPktLen             thisRoundSize;
    ttPktLen             origDataLength;
    int                  errorCode;
    tt8Bit               blockSize;
    tt8Bit               additionalData;
#ifdef TM_USE_SSL_VERSION_33
    ttPacketPtr          tmpPacketPtr;
    ttPktLen             restLen;
    ttPktLen             copyLen;
    ttPktLen             copiedLen;
#endif /* TM_USE_SSL_VERSION_33 */

    errorCode = TM_ENOERROR;
/* SSL has additional data to encrypt */
    additionalData = 1;
/* 1. Variable initialization and Sanity check */
    if (connStatePtr->scsCryptoAlgPtr != (ttEspAlgorithmPtr)0)
    {
        blockSize = connStatePtr->scsCryptoAlgPtr->ealgBlockSize;
    }
    else
    {
        blockSize = TM_8BIT_ZERO;
    }
    origHeadPacketPtr = packetPtr;
    origDataLength = dataLength;
    if (!packetPtr
        || (connStatePtr->scsCryptoAlgPtr == (ttEspAlgorithmPtr)0)
        || ((blockSize != TM_8BIT_ZERO) && ((dataLength % blockSize) != 0)))
    {
        errorCode = TM_EPERM;
        goto sslRecordPacketTransformExit;
    }
/* 2. Get the right generic key */
    if (direction == TM_SSL_ENCODE)
    {
        gkeyPtr = &connStatePtr->scsEncodeCryptoKey;
    }
    else
    {
        gkeyPtr = &connStatePtr->scsDecodeCryptoKey;
    }
/* 3. Where to write the results */
/* The newPacketPtr could have SSL record header built-in */
    if (newPacketPtr)
    {
        writeAddrPtr = newPacketPtr->pktLinkDataPtr +
                   newPacketPtr->pktLinkDataLength;
    }
    else
    {
        writeAddrPtr = packetPtr->pktLinkDataPtr + offset;
    }
/* 4. Transform the original packet lists*/
#ifdef TM_USE_ARCFOUR
    if (connStatePtr->scsCryptoAlgPtr->ealgName == SADB_EALG_ARCFOUR)
    {
/* 4.1 stream cipher is used, we don't care the direction, as long as we are
 * using the right key, the operation is exactly the same
 */
        while ((dataLength > 0) && packetPtr != (ttPacketPtr)0)
        {
            thisRoundSize = ((ttPktLen)dataLength >
                             (packetPtr->pktLinkDataLength - offset))?
                            (packetPtr->pktLinkDataLength - offset):
                            (ttPktLen)dataLength;
            tfArc4Encrypt(gkeyPtr->keySchedulePtr,
                          writeAddrPtr,
                          packetPtr->pktLinkDataPtr + offset,
                          thisRoundSize);
            offset = 0;
            writeAddrPtr += thisRoundSize;
            dataLength = (tt16Bit)(dataLength - thisRoundSize);
            packetPtr = packetPtr->pktLinkNextPtr;
        }
        if (newPacketPtr)
        {
            newPacketPtr->pktLinkDataLength += origDataLength;
        }
    }
    else
#endif /* TM_USE_ARCFOUR */
    if (connStatePtr->scsCryptoAlgPtr->ealgName == SADB_EALG_NULL)
    {
/* Note that, EALG_NULL is never used in SSL practically */
        while ((dataLength > 0) && packetPtr != (ttPacketPtr)0)
        {
            thisRoundSize = ((ttPktLen)dataLength >
                             (packetPtr->pktLinkDataLength - offset))?
                            (packetPtr->pktLinkDataLength - offset):
                            (ttPktLen)dataLength;
            tm_bcopy(packetPtr->pktLinkDataPtr + offset,
                     writeAddrPtr,
                     thisRoundSize);
            offset = 0;
            writeAddrPtr += thisRoundSize;
            dataLength = (tt16Bit)(dataLength - thisRoundSize);
            packetPtr = packetPtr->pktLinkNextPtr;
        }
        if (newPacketPtr)
        {
            newPacketPtr->pktLinkDataLength += origDataLength;
        }
    }
    else
    {
/* 4.2 block cipher CBC mode, we need know we are decrypting or encrypting */
        if (direction == TM_SSL_ENCODE)
        {
#ifdef TM_USE_SSL_VERSION_33
            if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
            {
                dataLength = dataLength -
                    (tt16Bit)(connStatePtr->scsCryptoAlgPtr->ealgBlockSize);
            }
#endif /* TM_USE_SSL_VERSION_33 */

            errorCode = tfEspPacketCbcEncrypt(connStatePtr->scsEncodeIvPtr,
                                  packetPtr,
                                  offset,
                                  newPacketPtr,
                                  dataLength,
                                  gkeyPtr,
                                  connStatePtr->scsCryptoAlgPtr,
                                  &additionalData);
        }
        else if (direction == TM_SSL_DECODE)
        {
#ifdef TM_USE_SSL_VERSION_33
            if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
            {
/* Read IV data */
                restLen = blockSize;
                copyLen = 0;
                copiedLen = 0;
                do {
                    if (packetPtr->pktLinkDataLength > restLen)
                    {
                        copyLen = restLen;
                        tm_bcopy(packetPtr->pktLinkDataPtr,
                                 connStatePtr->scsDecodeIvPtr + copiedLen,
                                 copyLen);
                        offset += copyLen;
                    }
                    else
                    {
                        copyLen = packetPtr->pktLinkDataLength;
                        tm_bcopy(packetPtr->pktLinkDataPtr,
                                 connStatePtr->scsDecodeIvPtr + copiedLen,
                                 copyLen);
                        tmpPacketPtr = packetPtr;
                        packetPtr = packetPtr->pktLinkNextPtr;
                        if (packetPtr == TM_PACKET_NULL_PTR)
                        {
                            errorCode = TM_EPERM;
                            goto sslRecordPacketTransformExit;
                        }
                        offset = 0;
                    }
                    restLen -= copyLen;
                    copiedLen += copyLen;
                } while (restLen > 0);
                dataLength = dataLength -
                    (tt16Bit)(connStatePtr->scsCryptoAlgPtr->ealgBlockSize);
            }
#endif /* TM_USE_SSL_VERSION_33 */

            errorCode = tfEspPacketCbcDecrypt(connStatePtr->scsDecodeIvPtr,
                                  packetPtr,
                                  offset,
                                  newPacketPtr,
                                  dataLength,
                                  gkeyPtr,
                                  connStatePtr->scsCryptoAlgPtr,
                                  &additionalData);
        }
    }

    if (direction == TM_SSL_ENCODE && errorCode == TM_ENOERROR)
    {
/* increase the sequence number*/
        connStatePtr->scsOutSeqNumber ++;
        if (connStatePtr->scsOutSeqNumber == 0)
        {
            connStatePtr->scsOutSeqNumberHigh ++;
        }
    }

sslRecordPacketTransformExit:
    if (newPacketPtr && newPacketPtr != origHeadPacketPtr)
    {
/* if we want to put everything into a new packet, just free the old one */
        tfFreePacket(origHeadPacketPtr, TM_SOCKET_LOCKED);
    }
    return errorCode;
}


/* Generate all crypto keys, Mac secrets, and possible IVs if
 * block cipher is used. And also, calculate the schedule or contexts
 * Calculate crypto keys, mac keys, ivs, for server, we calculate keys when
 * we processed the Client CCS if this is a regular handshake. For resumpted
 * handshake, server should calculate keys after we successfully parse the
 * client hello and construct the server hello. i.e., in tfSslParseClientCCS
 * and tfSslSendServerHello respectively. The bottom line is that, you should
 * have master secret and client random and server random ready.
 *
 * For client, we calculate keys after we construct CCS for regular
 * handshake. For resumption handshake, client should calculate keys when
 * we parse server hello, and if server verifies our resumption request.
 */
int tfSslGenerateCryptoMacKeys(ttSslConnectStatePtr connStatePtr)
{
#if (defined(TM_USE_SSL_VERSION_31) || defined(TM_USE_SSL_VERSION_33))
    tt8BitPtr            swappedRandomPtr;
#endif /* TM_USE_SSL_VERSION_31 || TM_USE_SSL_VERSION_33 */
#ifdef TM_USE_SSL_VERSION_30
    ttMd5Ctx             md5Ctx;
    tt8Bit               exportKey[20];
    int                  iteration;
    int                  i;
#endif /* TM_USE_SSL_VERSION_30 */
    int                  errorCode;
    int                  keyBlockSizeNeed;
#if (defined(TM_USE_SSL_VERSION_30) || defined(TM_USE_SSL_VERSION_31)\
  || defined(TM_USE_SSL_VERSION_33))
    int                  keyExpandedSize;
#endif /* TM_USE_SSL_VERSION_30 || TM_USE_SSL_VERSION_31
       || TM_USE_SSL_VERSION_33 */
    tt8Bit               cryptoKeySize;
/* blockSize = ivSize */
    tt8Bit               blockSize;
/* hashSize = hashKeySize*/
    tt8Bit               hashKeySize;
    tt8Bit               exportKeyLen;

#if (defined(TM_USE_SSL_VERSION_31) || defined(TM_USE_SSL_VERSION_33))
    swappedRandomPtr = (tt8BitPtr)0;
#endif /* TM_USE_SSL_VERSION_31 || TM_USE_SSL_VERSION_33 */
    errorCode = TM_ENOERROR;
    exportKeyLen = connStatePtr->scsCipherPtr->csExpandedLength;
    blockSize = connStatePtr->scsCryptoAlgPtr->ealgBlockSize;
    if (blockSize == 1)
    {
/* for stream cipher we don't need iv */
        blockSize = 0;
    }
    hashKeySize = connStatePtr->scsHashOutputSize;
    cryptoKeySize = connStatePtr->scsCipherPtr->csKeyLength;
    keyBlockSizeNeed = (hashKeySize << 1) +
                       (cryptoKeySize << 1) +
                       (blockSize << 1);
/* we keep all key materials in this block buffer (even for exportable
 * cipher suite.) For exportable cipher suite, we don't use the iv
 * length when we generate the key block
 */
#ifdef TM_USE_SSL_VERSION_30
/* For SSL 3.0, the keyBlockSizeNeed has to be multiples of MD5 hashout,
 * otherwise, we need to count how many bytes left each iteration.
 * Be careful don't overwrite the buffer allocated.
 */
    if (connStatePtr->scsVersion == TM_SSL_VERSION_30)
    {
        keyBlockSizeNeed += (TM_MD5_HASH_SIZE - keyBlockSizeNeed %
                             TM_MD5_HASH_SIZE) % TM_MD5_HASH_SIZE;
    }
#endif /*TM_USE_SSL_VERSION_30*/

/* save another instance of the keys (for rehandshake usage) */
    connStatePtr->scsKeyBlockBackupPtr = connStatePtr->scsKeyBlockPtr;
    connStatePtr->scsKeyBlockPtr = tm_get_raw_buffer((ttPktLen)
                                                     keyBlockSizeNeed);
    if (!connStatePtr->scsKeyBlockPtr)
    {
        errorCode = TM_ENOBUFS;
        goto sslGenerateCryptoMacKeyExit;
    }
    tm_bzero(connStatePtr->scsKeyBlockPtr, keyBlockSizeNeed);
    if (exportKeyLen > 0)
    {
/* we compute keyExpandedSize, i.e., how many bytes we need expand
 * from master secret. For non-exportable cipher suite, that is just
 * keyBlockSizeNeed we computed before. However, for exportable
 * ciphers, we don't need to expand IV (blockSize * 2) from master
 * secret, so our keyExpandedSize should be smaller. Do remember that,
 * we have already allocated the buffer for IV though.
 * For exportable encrypt/decrypt key, we need another expand from
 * the 40bits keys to 128bits keys for example. That is another
 * story. We will allocated another buffer to store that key pair, i.e.
 * scsExportKeyPtr.
 * For SSL 3.0, when we minus the two IV from keyBlockSizeNeed, we
 * need to make sure the results are still multiples of TM_MD5_HASH_SIZE,
 * since blockSize is either 0 or 8 or 16. We automatically satisfy
 * this requirement
 */
#if (defined(TM_USE_SSL_VERSION_30) || defined(TM_USE_SSL_VERSION_31)\
  || defined(TM_USE_SSL_VERSION_33))
        keyExpandedSize = keyBlockSizeNeed - (blockSize << 1);
#endif /* TM_USE_SSL_VERSION_30 || TM_USE_SSL_VERSION_31
       || TM_USE_SSL_VERSION_33 */

        connStatePtr->scsExportKeyBackupPtr = connStatePtr->scsExportKeyPtr;
        connStatePtr->scsExportKeyPtr = tm_get_raw_buffer
                    (exportKeyLen << 1);
        if (!connStatePtr->scsExportKeyPtr)
        {
            errorCode = TM_ENOBUFS;
            goto sslGenerateCryptoMacKeyExit;
        }
        connStatePtr->scsRunFlags |= TM_SSL_RUNFLAG_EXPORT_KEY;
    }
#if (defined(TM_USE_SSL_VERSION_30) || defined(TM_USE_SSL_VERSION_31)\
  || defined(TM_USE_SSL_VERSION_33))
    else
    {
        keyExpandedSize = keyBlockSizeNeed;
    }
#endif /* TM_USE_SSL_VERSION_30 || TM_USE_SSL_VERSION_31
       || TM_USE_SSL_VERSION_33 */

#ifdef TM_USE_SSL_VERSION_31
    if (connStatePtr->scsVersion == TM_SSL_VERSION_31)
    {
        swappedRandomPtr = tm_get_raw_buffer
                    (TM_SSL_RANDOM_SIZE << 1);
        if (!swappedRandomPtr)
        {
            errorCode = TM_ENOBUFS;
            goto sslGenerateCryptoMacKeyExit;
        }
        tm_bcopy(connStatePtr->scsServerRandom,
                 swappedRandomPtr,
                 TM_SSL_RANDOM_SIZE);
        tm_bcopy(connStatePtr->scsClientRandom,
                 swappedRandomPtr + TM_SSL_RANDOM_SIZE,
                 TM_SSL_RANDOM_SIZE);

        errorCode = tfTlsPrf(connStatePtr,
                             connStatePtr->scsMasterSecret,
                             TM_SSL_MASTER_SECRET_SIZE,
                             TM_TLS_KEY_EXPAN_LABEL,
                             TM_TLS_KEY_EXPAN_LABELLEN,
                             swappedRandomPtr,
                             TM_SSL_RANDOM_SIZE << 1,
                             connStatePtr->scsKeyBlockPtr,
                             keyExpandedSize);
        if (errorCode != TM_ENOERROR)
        {
            goto sslGenerateCryptoMacKeyExit;
        }
        if (exportKeyLen > 0)
        {
/* final client write key*/
            errorCode = tfTlsPrf(connStatePtr,
                connStatePtr->scsKeyBlockPtr + (hashKeySize << 1),
                cryptoKeySize,
                TM_TLS_CLIENT_WKEY_LABEL,
                TM_TLS_CLIENT_WKEY_LABELLEN,
                connStatePtr->scsClientRandom,
                TM_SSL_RANDOM_SIZE << 1,
                connStatePtr->scsExportKeyPtr,
                exportKeyLen);
            if (errorCode != TM_ENOERROR)
            {
                goto sslGenerateCryptoMacKeyExit;
            }
/* final server write key*/
            errorCode = tfTlsPrf(connStatePtr,
                connStatePtr->scsKeyBlockPtr + (hashKeySize << 1)
                + cryptoKeySize,
                cryptoKeySize,
                TM_TLS_SERVER_WKEY_LABEL,
                TM_TLS_SERVER_WKEY_LABELLEN,
                connStatePtr->scsClientRandom,
                TM_SSL_RANDOM_SIZE << 1,
                connStatePtr->scsExportKeyPtr + exportKeyLen,
                exportKeyLen);
            if (errorCode != TM_ENOERROR)
            {
                goto sslGenerateCryptoMacKeyExit;
            }
/* final IV */
            if (blockSize)
            {
                errorCode = tfTlsPrf(connStatePtr,
                    (tt8BitPtr)"",
                    0,
                    TM_TLS_IVBLOCK_LABEL,
                    TM_TLS_IVBLOCK_LABELLEN,
                    connStatePtr->scsClientRandom,
                    TM_SSL_RANDOM_SIZE << 1,
                    connStatePtr->scsKeyBlockPtr + (hashKeySize << 1)
                    + (cryptoKeySize << 1),
                    blockSize << 1);
                if (errorCode != TM_ENOERROR)
                {
                    goto sslGenerateCryptoMacKeyExit;
                }
            }
        }
    }
#endif /* TM_USE_SSL_VERSION_31 */

#ifdef TM_USE_SSL_VERSION_33
#ifdef TM_USE_SSL_VERSION_31
    else
#endif /* TM_USE_SSL_VERSION_31 */
    if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
        swappedRandomPtr = tm_get_raw_buffer
                    (TM_SSL_RANDOM_SIZE << 1);
        if (!swappedRandomPtr)
        {
            errorCode = TM_ENOBUFS;
            goto sslGenerateCryptoMacKeyExit;
        }
        tm_bcopy(connStatePtr->scsServerRandom,
                 swappedRandomPtr,
                 TM_SSL_RANDOM_SIZE);
        tm_bcopy(connStatePtr->scsClientRandom,
                 swappedRandomPtr + TM_SSL_RANDOM_SIZE,
                 TM_SSL_RANDOM_SIZE);

        errorCode = tfTlsPrf2(connStatePtr,
                             connStatePtr->scsMasterSecret,
                             TM_SSL_MASTER_SECRET_SIZE,
                             TM_TLS_KEY_EXPAN_LABEL,
                             TM_TLS_KEY_EXPAN_LABELLEN,
                             swappedRandomPtr,
                             TM_SSL_RANDOM_SIZE << 1,
                             connStatePtr->scsKeyBlockPtr,
                             keyExpandedSize);
        if (errorCode != TM_ENOERROR)
        {
            goto sslGenerateCryptoMacKeyExit;
        }
        if (exportKeyLen > 0)
        {
/* final client write key*/
            errorCode = tfTlsPrf2(connStatePtr,
                connStatePtr->scsKeyBlockPtr + (hashKeySize << 1),
                cryptoKeySize,
                TM_TLS_CLIENT_WKEY_LABEL,
                TM_TLS_CLIENT_WKEY_LABELLEN,
                connStatePtr->scsClientRandom,
                TM_SSL_RANDOM_SIZE << 1,
                connStatePtr->scsExportKeyPtr,
                exportKeyLen);
            if (errorCode != TM_ENOERROR)
            {
                goto sslGenerateCryptoMacKeyExit;
            }
/* final server write key*/
            errorCode = tfTlsPrf2(connStatePtr,
                connStatePtr->scsKeyBlockPtr + (hashKeySize << 1)
                + cryptoKeySize,
                cryptoKeySize,
                TM_TLS_SERVER_WKEY_LABEL,
                TM_TLS_SERVER_WKEY_LABELLEN,
                connStatePtr->scsClientRandom,
                TM_SSL_RANDOM_SIZE << 1,
                connStatePtr->scsExportKeyPtr + exportKeyLen,
                exportKeyLen);
            if (errorCode != TM_ENOERROR)
            {
                goto sslGenerateCryptoMacKeyExit;
            }
/* final IV */
            if (blockSize)
            {
                errorCode = tfTlsPrf2(connStatePtr,
                    (tt8BitPtr)"",
                    0,
                    TM_TLS_IVBLOCK_LABEL,
                    TM_TLS_IVBLOCK_LABELLEN,
                    connStatePtr->scsClientRandom,
                    TM_SSL_RANDOM_SIZE << 1,
                    connStatePtr->scsKeyBlockPtr + (hashKeySize << 1)
                    + (cryptoKeySize << 1),
                    blockSize << 1);
                if (errorCode != TM_ENOERROR)
                {
                    goto sslGenerateCryptoMacKeyExit;
                }
            }
        }
    }
#endif /* TM_USE_SSL_VERSION_33 */

#ifdef TM_USE_SSL_VERSION_30
    if (connStatePtr->scsVersion == TM_SSL_VERSION_30)
    {
        iteration = keyExpandedSize / TM_MD5_HASH_SIZE;
/* We already make sure that keyExpandedSize is multiple of TM_MD5_HASH_SIZE*/

        for (i = 0; i < iteration; i ++)
        {
/*     key_block =
 *      MD5(master_secret + SHA(`A' + master_secret +
 *                              ServerHello.random +
 *                              ClientHello.random)) +
 *       MD5(master_secret + SHA(`BB' + master_secret +
 *                               ServerHello.random +
 *                               ClientHello.random)) +
 *       MD5(master_secret + SHA(`CCC' + master_secret +
 *                               ServerHello.random +
 *                               ClientHello.random)) + [...];
 */
            tfSsl30GetKeyMaterial(connStatePtr->scsMasterSecret,
                                  TM_SSL_MASTER_SECRET_SIZE,
                                  i + 1,
                                  connStatePtr->scsServerRandom,
                                  connStatePtr->scsClientRandom,
                                  connStatePtr->scsKeyBlockPtr +
                                  i * TM_MD5_HASH_SIZE);
        }

        if (exportKeyLen > 0)
        {
/* final client write key*/
/*    final_client_write_key = MD5(client_write_key +
 *                                 ClientHello.random +
 *                                 ServerHello.random);
 */
            tfMd5Init(&md5Ctx);
            tfMd5Update(&md5Ctx,
                        connStatePtr->scsKeyBlockPtr + 2 * hashKeySize,
                        cryptoKeySize,
                        0);
            tfMd5Update(&md5Ctx,
                        connStatePtr->scsClientRandom,
                        2 * TM_SSL_RANDOM_SIZE,
                        0);
            tfMd5Final(exportKey,
                       &md5Ctx);
/* Notice that, the exportKeyLen can be maximum 16 bytes, (could be 8 bytes),
 * one MD5 hash should be enough
 */
            tm_bcopy(exportKey,
                     connStatePtr->scsExportKeyPtr,
                     exportKeyLen);

/* final server write key*/
/*     final_server_write_key = MD5(server_write_key +
 *                                 ServerHello.random +
 *                                 ClientHello.random);
 */
            tfMd5Init(&md5Ctx);
            tfMd5Update(&md5Ctx,
                        connStatePtr->scsKeyBlockPtr + 2 * hashKeySize
                         + cryptoKeySize,
                        cryptoKeySize,
                        0);
            tfMd5Update(&md5Ctx,
                        connStatePtr->scsServerRandom,
                        TM_SSL_RANDOM_SIZE,
                        0);
            tfMd5Update(&md5Ctx,
                        connStatePtr->scsClientRandom,
                        TM_SSL_RANDOM_SIZE,
                        0);
            tfMd5Final(exportKey, &md5Ctx);
            tm_bcopy(exportKey,
                     connStatePtr->scsExportKeyPtr + exportKeyLen,
                     exportKeyLen);

/* final IV */
            if (blockSize)
            {
/* client write IV*/
/*    client_write_iv = MD5(ClientHello.random +
 *                                 ServerHello.random);
 */
                tfMd5Init(&md5Ctx);
                tfMd5Update(&md5Ctx,
                    connStatePtr->scsClientRandom,
                    2 * TM_SSL_RANDOM_SIZE,
                    0);
                tfMd5Final(exportKey, &md5Ctx);
                tm_bcopy(exportKey,
                    connStatePtr->scsKeyBlockPtr
                    + 2 * hashKeySize + 2* cryptoKeySize,
                    blockSize);
/* server write iv */
/*     server_write_iv = MD5(ServerHello.random +
 *                                 ClientHello.random);
 */
                tfMd5Init(&md5Ctx);
                tfMd5Update(&md5Ctx,
                    connStatePtr->scsServerRandom,
                    TM_SSL_RANDOM_SIZE,
                    0);
                tfMd5Update(&md5Ctx,
                    connStatePtr->scsClientRandom,
                    TM_SSL_RANDOM_SIZE,
                    0);
                tfMd5Final(exportKey, &md5Ctx);
                tm_bcopy(exportKey,
                    connStatePtr->scsKeyBlockPtr
                    + 2 * hashKeySize + 2* cryptoKeySize + blockSize,
                    blockSize);
            }/* exportable iv */
        } /* exportable */
    }/*ssl v30 */
#endif /* TM_USE_SSL_VERSION_30 */

#ifdef TM_SSL_DEBUG
/* print the key blocks */
    tm_debug_log0("\nExpanded Key Block = \n");
    tfCryptoDebugPrintf((ttCharPtrPtr)0,
                        connStatePtr->scsKeyBlockPtr,
                        keyBlockSizeNeed ,
                        (ttIntPtr)0);
#endif /* TM_SSL_DEBUG */

sslGenerateCryptoMacKeyExit:
#if (defined(TM_USE_SSL_VERSION_31) || defined(TM_USE_SSL_VERSION_33))
    if (swappedRandomPtr)
    {
        tm_free_raw_buffer(swappedRandomPtr);
    }
#endif /* TM_USE_SSL_VERSION_31 || TM_USE_SSL_VERSION_33 */
    return errorCode;
}

int tfSslClientWriteKeySchedule(ttSslConnectStatePtr  connStatePtr,
                                ttGenericKeyPtr       cwMacKeyPtr,
                                ttGenericKeyPtr       cwCryptoKeyPtr,
                                tt8BitPtrPtr          cwIvPtrPtr)
{
    int                     errorCode;
    tt8Bit                  hashKeySize;
    tt8Bit                  cryptoKeySize;
    tt8Bit                  exportKeyLen;

    errorCode = TM_ENOERROR;
    hashKeySize = connStatePtr->scsHashOutputSize;
    cryptoKeySize = connStatePtr->scsCipherPtr->csKeyLength;
    exportKeyLen = connStatePtr->scsCipherPtr->csExpandedLength;

/* update all my raw keys and schedules, i.e.change all keys and
 * key schedules for the client write keys
 */
/* client_write_mac_secret */
     cwMacKeyPtr->keyDataPtr = connStatePtr->scsKeyBlockPtr;
     cwMacKeyPtr->keyBits = (hashKeySize << 3);

#ifdef TM_USE_SW_CRYPTOENGINE
     tfAhClearContext(cwMacKeyPtr->keyContextPtr);
     cwMacKeyPtr->keyContextPtr = (tt8BitPtr)0;
     errorCode = tfAhContext(connStatePtr->scsHashAlgPtr,
                             cwMacKeyPtr);
     if (errorCode != TM_ENOERROR)
     {
         goto cwKeyScheduleExit;
     }
#endif /* TM_USE_SW_CRYPTOENGINE */
/* client_write_key */
     if(exportKeyLen)
     {
         cwCryptoKeyPtr->keyDataPtr = connStatePtr->scsExportKeyPtr;
         cwCryptoKeyPtr->keyBits = (exportKeyLen << 3);
     }
     else
     {
         cwCryptoKeyPtr->keyDataPtr = cwMacKeyPtr->keyDataPtr +
                     (hashKeySize << 1);
         cwCryptoKeyPtr->keyBits = (cryptoKeySize << 3);
     }
#ifdef TM_USE_SW_CRYPTOENGINE
     tfEspClearSchedule(cwCryptoKeyPtr->keySchedulePtr);
     cwCryptoKeyPtr->keySchedulePtr = (tt8BitPtr)0;
     errorCode = tfEspSchedule(connStatePtr->scsCryptoAlgPtr,
         cwCryptoKeyPtr);
     if (errorCode != TM_ENOERROR)
     {
         goto cwKeyScheduleExit;
     }
#endif /* TM_USE_SW_CRYPTOENGINE */
/* client write iv */
     *cwIvPtrPtr = connStatePtr->scsKeyBlockPtr
                    + 2 * hashKeySize + 2* cryptoKeySize;


#ifdef TM_USE_SW_CRYPTOENGINE
cwKeyScheduleExit:
#endif /* TM_USE_SW_CRYPTOENGINE */
     return errorCode;
}

int tfSslServerWriteKeySchedule(ttSslConnectStatePtr  connStatePtr,
                                ttGenericKeyPtr       swMacKeyPtr,
                                ttGenericKeyPtr       swCryptoKeyPtr,
                                tt8BitPtrPtr          swIvPtrPtr)
{
    int                     errorCode;
    tt8Bit                  blockSize;
    tt8Bit                  hashKeySize;
    tt8Bit                  cryptoKeySize;
    tt8Bit                  exportKeyLen;

    blockSize = connStatePtr->scsBlockSize;
    hashKeySize = connStatePtr->scsHashOutputSize;
    cryptoKeySize = connStatePtr->scsCipherPtr->csKeyLength;
    exportKeyLen = connStatePtr->scsCipherPtr->csExpandedLength;

/* update all my raw keys and schedules, i.e.change all keys and
 * key schedules for the server write keys
 */
/* server_write_mac_secret */
     swMacKeyPtr->keyDataPtr = connStatePtr->scsKeyBlockPtr + hashKeySize;
     swMacKeyPtr->keyBits = (hashKeySize << 3);
     tfAhClearContext(swMacKeyPtr->keyContextPtr);
     swMacKeyPtr->keyContextPtr = (tt8BitPtr)0;
     errorCode = tfAhContext(connStatePtr->scsHashAlgPtr,
                             swMacKeyPtr);
     if (errorCode != TM_ENOERROR)
     {
         goto swKeyScheduleExit;
     }
/* server_write_key */
     if(exportKeyLen)
     {
         swCryptoKeyPtr->keyDataPtr = connStatePtr->scsExportKeyPtr
                   + exportKeyLen;
         swCryptoKeyPtr->keyBits = (exportKeyLen << 3);
     }
     else
     {
         swCryptoKeyPtr->keyDataPtr = swMacKeyPtr->keyDataPtr + hashKeySize
                   + cryptoKeySize;
         swCryptoKeyPtr->keyBits = (cryptoKeySize << 3);
     }
     tfEspClearSchedule(swCryptoKeyPtr->keySchedulePtr);
     swCryptoKeyPtr->keySchedulePtr = (tt8BitPtr)0;
     errorCode = tfEspSchedule(connStatePtr->scsCryptoAlgPtr,
         swCryptoKeyPtr);
     if (errorCode != TM_ENOERROR)
     {
         goto swKeyScheduleExit;
     }
/* server write iv */
     *swIvPtrPtr = connStatePtr->scsKeyBlockPtr
                    + 2 * hashKeySize + 2* cryptoKeySize + blockSize;

swKeyScheduleExit:
     return errorCode;
}

/*
 * Decode direction, after decryption, this function is called to
 * verify the mac results
 */

static int tfSslRecordDecodeMacVerify( ttSslConnectStatePtr    connStatePtr,
                                       ttSslRecordHeaderPtr    sslHeaderPtr,
                                       ttPacketPtr             packetPtr,
                                       ttPktLen                offset,
                                       ttPktLen                dataLength,
                                       tt16BitPtr              origDataLenPtr)
{
    tt8BitPtr            macOutPtr;
    tt8BitPtr            dataPtr;
    int                  errorCode;
    int                  isEqual;
    tt16Bit              origDataLength;
    tt8Bit               macPaddingLen;


    errorCode = TM_ENOERROR;
    packetPtr->pktFlags2 |= TM_PF2_AUTH_ICVDATA_AT_END;
    dataPtr = packetPtr->pktLinkDataPtr + offset;
/* Allocate for worst case */
    macOutPtr = tm_get_raw_buffer((ttPktLen)(connStatePtr->scsHashOutputSize +
                                             1 +
                                             255));
    if (!macOutPtr)
    {
        errorCode = TM_ENOBUFS;
        goto sslDecodeMacVerifyExit;
    }

    macPaddingLen = (connStatePtr->scsHashOutputSize);
    if (connStatePtr->scsBlockSize > 1)
    {
        macPaddingLen = (tt8Bit)(macPaddingLen + 1 /* padding length field */
             + *(dataPtr + dataLength - 1));
    }

    if(dataLength < (ttPktLen)macPaddingLen)
    {
/* most likely, decryption error happens before*/
        errorCode = TM_EINVAL;
        goto sslDecodeMacVerifyExit;
    }

    origDataLength = (tt16Bit)(dataLength - macPaddingLen);
    *origDataLenPtr = origDataLength;
/* We must have set the ssl header to its original value (without mac &
 * padding), because the ssl header is included in the MAC computation.
 */
    sslHeaderPtr->srhLengthHigh = (tt8Bit)(origDataLength >> 8);
    sslHeaderPtr->srhLengthLow = (tt8Bit)(origDataLength & 0xff);

#ifdef TM_SSL_DEBUG
    tm_debug_log0("Decode mac key = ");
    tfCryptoDebugPrintf((ttCharPtrPtr)0,
                        connStatePtr->scsDecodeMacKey.keyDataPtr,
                        (connStatePtr->scsDecodeMacKey.keyBits >> 3),
                        (ttIntPtr)0);
    tm_debug_log0("Decode mac data = ");
#ifdef TM_USE_SSL_VERSION_33
    if (connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
        tfCryptoDebugPrintf((ttCharPtrPtr)0,
                            packetPtr->pktLinkDataPtr + offset
                            + connStatePtr->scsCryptoAlgPtr->ealgBlockSize,
                            origDataLength,
                            (ttIntPtr)0);
    }
    else
#endif /* TM_USE_SSL_VERSION_33 */
    {
        tfCryptoDebugPrintf((ttCharPtrPtr)0,
                            packetPtr->pktLinkDataPtr + offset,
                            origDataLength,
                            (ttIntPtr)0);
    }
#endif /*TM_SSL_DEBUG*/
#ifdef TM_USE_SSL_VERSION_30
    if (connStatePtr->scsVersion == TM_SSL_VERSION_30)
    {
        errorCode = tfSsl30GetRecordMac(
                              connStatePtr,
                              sslHeaderPtr,
                              packetPtr,
                              offset,
                              origDataLength,
                              TM_SSL_DECODE,
                              macOutPtr);
    }
#endif /* TM_USE_SSL_VERSION_30 */
#if (defined(TM_USE_SSL_VERSION_31) || defined(TM_USE_SSL_VERSION_33))
    if ((connStatePtr->scsVersion == TM_SSL_VERSION_31)
     || (connStatePtr->scsVersion == TM_SSL_VERSION_33))
    {
        errorCode = tfTlsGetRecordMac(
                              connStatePtr,
                              sslHeaderPtr,
                              packetPtr,
                              offset,
                              origDataLength,
                              TM_SSL_DECODE,
                              macOutPtr);
    }
#endif /* TM_USE_SSL_VERSION_31 || TM_USE_SSL_VERSION_33 */

#if (defined(TM_USE_SSL_VERSION_30) || defined(TM_USE_SSL_VERSION_31)\
  || defined(TM_USE_SSL_VERSION_33))
    if (errorCode != TM_ENOERROR)
    {
#ifdef TM_SSL_DEBUG
        tm_debug_log0("Decode mac failed. \n");
#endif /*TM_SSL_DEBUG*/
#ifdef TM_LOG_SSL_DEBUG
        tfEnhancedLogWrite(
            TM_LOG_MODULE_SSL,
            TM_LOG_LEVEL_DEBUG,
            "tfSslRecordDecodeMacVerify: Decoding MAC failed");
#endif /* TM_LOG_SSL_DEBUG */
        goto sslDecodeMacVerifyExit;
    }
#endif /* TM_USE_SSL_VERSION_30 || TM_USE_SSL_VERSION_31 ||
          TM_USE_SSL_VERSION_33 */

/* BUG ID 1389: Since SSL 3.0 doesn't care what padding content is,
 * we should loose our verification rules a little bit:
 * For SSL 3.0: No padding content check should be performed
 * For TLS: RFC says padding content must be the same as
 * padding length field. Thus, we need to check padding for TLS
 */
#if (defined(TM_USE_SSL_VERSION_31) || defined(TM_USE_SSL_VERSION_33))
    if( (connStatePtr->scsVersion == TM_SSL_VERSION_31)
      ||(connStatePtr->scsVersion == TM_SSL_VERSION_33))
    {
        tfSslUpdateRecordAfterMac(connStatePtr,
                                  sslHeaderPtr,
                                  *(dataPtr + dataLength - 1),
                                  macOutPtr);
    }
#endif /* TM_USE_SSL_VERSION_31 || TM_USE_SSL_VERSION_33 */

#ifdef TM_USE_SSL_VERSION_30
    if (connStatePtr->scsVersion == TM_SSL_VERSION_30)
    {
/* For SSL 3.0, we don't need to verify the padding, just check the hash
 * output
 */
        macPaddingLen = connStatePtr->scsHashOutputSize;
    }
#endif /* TM_USE_SSL_VERSION_30 */

#ifdef TM_SSL_DEBUG
    tm_debug_log0("Decode mac Result = (ignore the bytes after padding) \n ");
    tfCryptoDebugPrintf((ttCharPtrPtr)0,
                        macOutPtr,
                        connStatePtr->scsHashOutputSize +
                        connStatePtr->scsBlockSize,
                        (ttIntPtr)0);
#endif /*TM_SSL_DEBUG*/

/* compare what we got */
    isEqual = tm_memcmp(dataPtr + origDataLength
                        , macOutPtr, macPaddingLen);
    if (isEqual != 0)
    {
#ifdef TM_SSL_DEBUG
        tm_debug_log0("Decode mac verification failed. \n");
#endif /*TM_SSL_DEBUG*/
#ifdef TM_LOG_SSL_DEBUG
        tfEnhancedLogWrite(
            TM_LOG_MODULE_SSL,
            TM_LOG_LEVEL_DEBUG,
            "tfSslRecordDecodeMacVerify: Decoding MAC verification failed");
#endif /* TM_LOG_SSL_DEBUG */
        errorCode = TM_EINVAL;
        goto sslDecodeMacVerifyExit;
    }
    else
    {
#ifdef TM_SSL_DEBUG
        tm_debug_log0("Decode mac verification successful. \n");
#endif /*TM_SSL_DEBUG*/
#ifdef TM_LOG_SSL_DEBUG
        tfEnhancedLogWrite(
            TM_LOG_MODULE_SSL,
            TM_LOG_LEVEL_DEBUG,
            "tfSslRecordDecodeMacVerify: Decoding MAC verification successful");
#endif /* TM_LOG_SSL_DEBUG */
        connStatePtr->scsInSeqNumber ++;
        if (connStatePtr->scsInSeqNumber == 0)
        {
            connStatePtr->scsInSeqNumberHigh ++;
        }
    }
sslDecodeMacVerifyExit:
    if (macOutPtr)
    {
        tm_free_raw_buffer(macOutPtr);
    }
    return errorCode;
}


/* DESCRIPTION: This function is called to process the SSL record
 * mac calculation and encryption/decryption.
 * crqPtr->crFlags & TM_CRYPTOREQUEST_DECODE means decode direction
 * and if (crqPtr->crFlags &TM_CRYPTOREQUEST_DECODE), it means a
 * encode operation.
 * For ENCODE, we need calculate the mac, do the padding and
 * encryption. For DECODE, we need do decryption, verify the padding
 * and mac.
 */
int tfSslSoftwareRequestProcess(ttCryptoRequestPtr    crqPtr)
{
    ttSslRecordHeaderPtr           sslHeaderPtr;
    ttSslConnectStatePtr           connStatePtr;
    ttCryptoRequestSslPtr          sslRequestPtr;
    int                            errorCode;
    tt16Bit                        cryptoLength;
    tt8Bit                         ivLength;

    errorCode = TM_ENOERROR;
    sslRequestPtr = crqPtr->crParamUnion.crSslParamPtr;
    connStatePtr = (ttSslConnectStatePtr)sslRequestPtr->crsConnectStatePtr;
    sslHeaderPtr = (ttSslRecordHeaderPtr)sslRequestPtr->crsSslHeaderPtr;
    cryptoLength = *sslRequestPtr->crsRecordLenPtr;
    ivLength = (tt8Bit)0;

#ifdef TM_USE_SSL_VERSION_33
    if((connStatePtr->scsVersion == TM_SSL_VERSION_33)
     &&((connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_ARCFOUR)
      &&(connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_NULL)))
    {
        ivLength = connStatePtr->scsCryptoAlgPtr->ealgBlockSize;
    }
#endif /* TM_USE_SSL_VERSION_33 */
    if (crqPtr->crFlags & TM_CRYPTOREQUEST_ENCODE)
    {
/* FOR ENCODING: sslRequestPtr->crsRecordLenPtr should return how many bytes
 * the encrypted SSL record is, including mac and padding
 */
        errorCode = tfSslRecordEncodeMac
                            (connStatePtr,
                            sslHeaderPtr,
                            (ttPacketPtr)sslRequestPtr->crsPacketPtr,
                            sslRequestPtr->crsCryptHeadLength,
                            cryptoLength,
                            sslRequestPtr->crsMacOutPtr);
        if (errorCode != TM_ENOERROR)
        {
            goto sslSoftwareRequestProcessExit;
        }


/*5.2, we now need to do the encryption */
        cryptoLength = (tt16Bit)((sslHeaderPtr->srhLengthHigh << 8)|
            (sslHeaderPtr->srhLengthLow));
        *sslRequestPtr->crsRecordLenPtr = cryptoLength;

        errorCode = tfSslRecordPacketTransform(
                              connStatePtr,
                              (ttPacketPtr)sslRequestPtr->crsPacketPtr,
                              sslRequestPtr->crsCryptHeadLength,
                              (ttPacketPtr)0,
                              cryptoLength,
                              TM_SSL_ENCODE);
        if (errorCode != TM_ENOERROR)
        {
            goto sslSoftwareRequestProcessExit;
        }
    }
    else /* DECODE */
    {
/* FOR DECODING: sslRequestPtr->crsRecordLenPtr should return how many bytes
 * the original compressed SSL record is, stripping off any mac and padding
 */
/*        sslRequestPtr->crsMacOutPtr is not used for DECODING */

        errorCode = tfSslRecordPacketTransform(connStatePtr,
                              (ttPacketPtr)sslRequestPtr->crsPacketPtr,
                              sslRequestPtr->crsCryptHeadLength,
                              (ttPacketPtr)0,
                              cryptoLength,
                              TM_SSL_DECODE);
        if (errorCode != TM_ENOERROR)
        {
            goto sslSoftwareRequestProcessExit;
        }

        errorCode = tfSslRecordDecodeMacVerify(
                              connStatePtr,
                              sslHeaderPtr,
                              (ttPacketPtr)sslRequestPtr->crsPacketPtr,
                              sslRequestPtr->crsCryptHeadLength +
                              ivLength,
                              cryptoLength -
                              ivLength,
                              sslRequestPtr->crsRecordLenPtr);
        if (errorCode != TM_ENOERROR)
        {
            goto sslSoftwareRequestProcessExit;
        }
    }

sslSoftwareRequestProcessExit:
    return errorCode;
}

#ifdef TM_USE_SSL_VERSION_33
/*
 * refer RFC 3447 EMSA-PKCS1-v1_5 decoding method
 */
int tfSslRsaPkcsVerify(ttPkiPublicKeyPtr publicKeyPtr,
                       tt8BitPtr         dataPtr,
                       tt8BitPtr         hashPtr,
                       tt16Bit           dataLen)
{
    ttVoidPtr                 keyPtr;
    tt8BitPtr                 decryptedDataPtr;
    tt8BitPtr                 tPtr;
    int                       decryptedDataLen;
    int                       errorCode;
    tt16Bit                   tLen;
    tt16Bit                   retLen;
    tt8Bit                    cmpAsnId;

    errorCode = TM_ENOERROR;

    keyPtr = tfRsaKeyAsn1ToBn((ttGeneralRsaPtr)publicKeyPtr->pkKeyPtr);

    decryptedDataPtr = tm_get_raw_buffer(dataLen + 100);
    if (decryptedDataPtr == (tt8BitPtr)0)
    {
        errorCode = TM_ENOBUFS;
        goto rsaPkcsDecodeExit;
    }

    errorCode = tfRSAPublicDecrypt((int)dataLen,
                                   dataPtr,
                                   decryptedDataPtr,
                                   keyPtr,
                                   &decryptedDataLen);

    tfRsaFree(keyPtr);

    if (errorCode != TM_ENOERROR)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfIkev2RsaPkcsDecode",
                        "RSA Decrypt Error!!");
#endif /* TM_ERROR_CHECKING */
        goto rsaPkcsDecodeExit;
    }

    /*
     * take out SHA1 hash H from DER encoded T
     * T should be (0x)30 21 30 09 06 05 2b 0e 03 02 1a 05 00 04 14 || H
     */
    tPtr = decryptedDataPtr;
    tLen = (tt16Bit)decryptedDataLen;
    if ( (tLen != TM_SSL_SHA1WITHRSA_ENCODED_LEN) &&
         (tLen != TM_SSL_SHA256WITHRSA_ENCODED_LEN) &&
         (tLen != TM_SSL_MD5WITHRSA_ENCODED_LEN) )
    {
        errorCode = TM_EINVAL;
        goto rsaPkcsDecodeExit;
    }
    /*
     * decode DER :
     *
     *  DigestInfo ::= SEQUENCE {
     *      digestAlgorithm DigestAlgorithm,
     *      digest OCTET STRING
     *  }
     *  DigestAlgorithm ::=
     *      AlgorithmIdentifier { {PKCS1-v1-5DigestAlgorithms} }
     *  PKCS1-v1-5DigestAlgorithms    ALGORITHM-IDENTIFIER ::= {
     *      { OID id-md2 PARAMETERS NULL    }|
     *      { OID id-md5 PARAMETERS NULL    }|
     *      { OID id-sha1 PARAMETERS NULL   }|
     *      { OID id-sha256 PARAMETERS NULL }|
     *      { OID id-sha384 PARAMETERS NULL }|
     *      { OID id-sha512 PARAMETERS NULL }
     *  }
     *
     * id-sha1                   : 1.3.14.3.2.26
     * PARAMETERS                : omitted
     */
    if(tLen == TM_SSL_SHA1WITHRSA_ENCODED_LEN)
    {
        retLen = (tt16Bit)tm_asn1_sequence_dec((ttVoidPtr)0, &tPtr);
        if (tLen != retLen)
        {
            errorCode = TM_EINVAL;
            goto rsaPkcsDecodeExit;
        }
        cmpAsnId = (tt8Bit)tm_memcmp(tPtr,
                                     &tlAsn1Sha1AlgoId[0],
                                     TM_SSL_ASN1_SHA1_ALGO_ID_LEN);
        if (cmpAsnId != TM_8BIT_ZERO)
        {
            errorCode = TM_EINVAL;
            goto rsaPkcsDecodeExit;
        }
        tPtr += TM_SSL_ASN1_SHA1_ALGO_ID_LEN;
        if ((*tPtr != 0x04)
            || (*(tPtr + 1) != 0x14))
        {
            errorCode = TM_EINVAL;
            goto rsaPkcsDecodeExit;
        }
        tPtr += 2;
        /* copy H to user specified buffer */
        if(tm_memcmp(hashPtr, tPtr, TM_SHA1_HASH_SIZE) != 0)
        {
            errorCode = TM_EINVAL;
            goto rsaPkcsDecodeExit;
        }      
    }
    else if(tLen == TM_SSL_SHA256WITHRSA_ENCODED_LEN)
    {
        retLen = (tt16Bit)tm_asn1_sequence_dec((ttVoidPtr)0, &tPtr);
        if (tLen != retLen)
        {
            errorCode = TM_EINVAL;
            goto rsaPkcsDecodeExit;
        }
        cmpAsnId = (tt8Bit)tm_memcmp(tPtr,
                                     &tlAsn1Sha256AlgoId[0],
                                     TM_SSL_ASN1_SHA2_ALGO_ID_LEN);
        if (cmpAsnId != TM_8BIT_ZERO)
        {
            errorCode = TM_EINVAL;
            goto rsaPkcsDecodeExit;
        }
        tPtr += TM_SSL_ASN1_SHA2_ALGO_ID_LEN;
        if ((*tPtr != 0x04)
            || (*(tPtr + 1) != 0x20))
        {
            errorCode = TM_EINVAL;
            goto rsaPkcsDecodeExit;
        }
        tPtr += 2;
        /* copy H to user specified buffer */
        if(tm_memcmp(hashPtr, tPtr, TM_SHA256_HASH_SIZE) != 0)
        {
            errorCode = TM_EINVAL;
            goto rsaPkcsDecodeExit;
        }
    }

rsaPkcsDecodeExit:
    if (decryptedDataPtr != (tt8BitPtr)0)
    {
        tm_free_raw_buffer(decryptedDataPtr);
    }
    return errorCode;
}

/*
 * refer RFC 3447 EMSA-PKCS1-v1_5 encoding method
 */
int tfSslRsaPkcs1Encode(ttPkiPublicKeyPtr     privateKeyPtr,
                        tt8BitPtr             hashPtr,
                        tt8BitPtr             dataPtr,
                        ttIntPtr              dataLenPtr,
                        tt8Bit                hashAlg)
{
    ttAsn1OctetString         asn1OctString;
    ttVoidPtr                 keyPtr;
    tt8BitPtr                 tPtr;
    tt8Bit                    t[TM_SSL_SHA256WITHRSA_ENCODED_LEN];/* Max Length = SHA256 */
    int                       errorCode;

    errorCode = TM_ENOERROR;

    /*
     * encode DER format of T:
     *
     *  DigestInfo ::= SEQUENCE {
     *      digestAlgorithm DigestAlgorithm,
     *      digest OCTET STRING
     *  }
     *  DigestAlgorithm ::=
     *      AlgorithmIdentifier { {PKCS1-v1-5DigestAlgorithms} }
     *  PKCS1-v1-5DigestAlgorithms    ALGORITHM-IDENTIFIER ::= {
     *      { OID id-md2 PARAMETERS NULL    }|
     *      { OID id-md5 PARAMETERS NULL    }|
     *      { OID id-sha1 PARAMETERS NULL   }|
     *      { OID id-sha256 PARAMETERS NULL }|
     *      { OID id-sha384 PARAMETERS NULL }|
     *      { OID id-sha512 PARAMETERS NULL }
     *  }
     *
     * id-sha1                   : 1.3.14.3.2.26
     * PARAMETERS                : omitted
     */
    tPtr = &t[0];
    if(hashAlg == TM_SSL_SUPPORT_HASH_SHA256)
    {
        (void)tm_asn1_sequence_enc((ttVoidPtr)0,
                                   &tPtr,
                                   TM_SSL_SHA256WITHRSA_ENCODED_LEN - 2);
        tm_memcpy(tPtr, tlAsn1Sha256AlgoId, TM_SSL_ASN1_SHA2_ALGO_ID_LEN);
        tPtr += TM_SSL_ASN1_SHA2_ALGO_ID_LEN;
        /* encode digest */
        asn1OctString.asn1Length = TM_SHA256_HASH_SIZE;
        asn1OctString.asn1Type   = TM_ASN1_TAG_OCTETSTRING;
        asn1OctString.asn1Data   = hashPtr;
        (void)tm_asn1_octetstring_enc(&asn1OctString, &tPtr, 0);
        /* SHA2: (0x)30 31 30 0d 06 09 60 86 48 01 65 03 04 02 01 05 00 04 20 || H. */

        keyPtr = tfRsaKeyAsn1ToBn((ttGeneralRsaPtr)privateKeyPtr->pkKeyPtr);

        errorCode = tfRSAPrivateEncrypt((int)TM_SSL_SHA256WITHRSA_ENCODED_LEN,
                                        &t[0],
                                        dataPtr,
                                        keyPtr,
                                        dataLenPtr);

    }
    else if(hashAlg == TM_SSL_SUPPORT_HASH_SHA1)
    {
        (void)tm_asn1_sequence_enc((ttVoidPtr)0,
                                   &tPtr,
                                   TM_SSL_SHA1WITHRSA_ENCODED_LEN - 2);
        tm_memcpy(tPtr, tlAsn1Sha1AlgoId, TM_SSL_ASN1_SHA1_ALGO_ID_LEN);
        tPtr += TM_SSL_ASN1_SHA1_ALGO_ID_LEN;
        /* encode digest */
        asn1OctString.asn1Length = TM_SHA1_HASH_SIZE;
        asn1OctString.asn1Type   = TM_ASN1_TAG_OCTETSTRING;
        asn1OctString.asn1Data   = hashPtr;
        (void)tm_asn1_octetstring_enc(&asn1OctString, &tPtr, 0);
        /* T must be (0x)30 21 30 09 06 05 2b 0e 03 02 1a 05 00 04 14 || H */

        keyPtr = tfRsaKeyAsn1ToBn((ttGeneralRsaPtr)privateKeyPtr->pkKeyPtr);

        errorCode = tfRSAPrivateEncrypt((int)TM_SSL_SHA1WITHRSA_ENCODED_LEN,
                                        &t[0],
                                        dataPtr,
                                        keyPtr,
                                        dataLenPtr);
    }


    tfRsaFree(keyPtr);

    if (errorCode != TM_ENOERROR)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfRsaPkcs1Encode",
                        "RSA Encoding Error!!");
#endif /* TM_ERROR_CHECKING */
        goto rsaPkcsEncodeExit;
    }

rsaPkcsEncodeExit:

    return errorCode;
}

#endif /* TM_USE_SSL_VERSION_33 */


#else /* ! (defined(TM_USE_SSL_SERVER) || defined (TM_USE_SSL_CLIENT)) */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_SSL_SERVER,
 * TM_USE_SSL_CLIENT not defined */
int tlSslTlsDummy = 0;
#endif /* (defined(TM_USE_SSL_SERVER) || defined (TM_USE_SSL_CLIENT)) */

/***************** End Of File *****************/
