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
 * Description: SSL .h file
 *
 * Filename: trssl.h
 * Author: Jin Zhang
 * Date Created: 01/01/2004
 * $Source: include/trssl.h $
 *
 * Modification History
 * $Revision: 6.0.2.16 $
 * $Date: 2015/07/27 10:28:25JST $
 * $Author: pcarney $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _TRSSL_H_
#define _TRSSL_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Include
 */
#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>
#include <trcrylib.h>
#include <trpki.h>

/*
 * Contex which tfSslIncomingRecord is called from
 */
/* TCP incoming path (Treck recv task) */
#define TM_SSL_TCP_INCOMING                 0x01
/* recv/tfZeroCopyRecv */
#define TM_SSL_RECV_API                     0x02
/* tfGetWaitingBytes/send/tfZeroCopySend/tfSslUserProcessHandshake */
#define TM_SSL_OTHER_API                    0x04

#define TM_SSL_CLIENT_START                 0
#define TM_SSL_REPLY_CERTINCLUDE_LENGTH     2048
/* if no certificate involved*/
#define TM_SSL_REPLY_LENGTH                 256

#define TM_SSL_VERSION_MAJOR        (tt8Bit)0x03
#define TM_SSL_VERSION_MINOR0       (tt8Bit)0x0
#define TM_SSL_VERSION_MINOR1       (tt8Bit)0x01
#define TM_SSL_VERSION_MINOR2       (tt8Bit)0x02
#define TM_SSL_VERSION_MINOR3       (tt8Bit)0x03

/*server hello depends on session id length. If session id = 32*/
#define TM_SSL_SERVERHELLO_DEFAULT_SIZE        74

/*ServerHelloDone length, not including ssl record header */
#define TM_SSL_SERVERHELLODONE_DEFAULT_SIZE     4
/* including ssl record header*/
#define TM_SSL_CHANGECIPHERSPEC_DEFAULT_SIZE    6

/* 5 certificates in the cert chain is the maximum */
#define TM_SSL_CERT_CHAIN_MAX                  8
/* according to SSL document, certificate length is always denoted in 3 octets */
#define TM_SSL_CERT_LENFIELD_LEN               3

#define TM_SSL_PRE_MASTER_SIZE                 48
#define TM_SSL_MASTER_SECRET_SIZE              48

#define TM_SSL_ALERT_MESSAGE_SIZE          2
/* SSL v3.0, the finished message maximum length is :record header (5) 
 * not included 
 * handshake(4) + md5_hash(16) + sha_hash(20) + 
 * sha_hash(20)+ maximum padding (16 for AES block cipher)
 */
#define TM_SSL_V30_FINISH_SIZE                 76

/* TLS v1.0, the finished message maximum length is : record header (5)
 * not include
 * handshake(4) + verified data(12) + sha_hash(20)+ maximum padding 
 * (16 for AES block cipher)
 */
#define TM_SSL_V31_FINISH_SIZE                 52
#define TM_SSL_V31_FINISH_VERIFY_SIZE          12

/* TLS v1.2, the finished message maximum length is : record header (5)
 * not include
 * handshake(4) + verified data(12) + sha_hash(32)+ maximum padding 
 * (16 for AES block cipher)
 */
#define TM_SSL_V33_FINISH_SIZE                 64

#define TM_SSL30_PAD1                         (tt8Bit)0x36
#define TM_SSL30_PAD2                         (tt8Bit)0x5c
#define TM_SSL30_PAD_MD5_ROUNDS               48
#define TM_SSL30_PAD_SHA1_ROUNDS              40

#ifdef TM_USE_SSL_VERSION_30
#define TM_SSL30_CLIENT_FINISH_LABEL         "CLNT"
#define TM_SSL30_SERVER_FINISH_LABEL         "SRVR"
#endif /* TM_USE_SSL_VERSION_30 */

#if (defined(TM_USE_SSL_VERSION_31) || defined(TM_USE_SSL_VERSION_32)\
  || defined(TM_USE_SSL_VERSION_33))
#define TM_SSL31_CLIENT_FINISH_LABEL         "client finished"
#define TM_SSL31_SERVER_FINISH_LABEL         "server finished"
#endif /* TM_USE_SSL_VERSION_31 || TM_USE_SSL_VERSION_32
       || TM_USE_SSL_VERSION_33 */

/*
 * TLS cipher suite
 */
/*cert request, certificate type */
#define TM_CERTTYPE_RSA_SIGN                   (tt8Bit)1
#define TM_CERTTYPE_DSS_SIGN                   (tt8Bit)2
#define TM_CERTTYPE_RSA_DH                     (tt8Bit)3
#define TM_CERTTYPE_DSS_DH                     (tt8Bit)4

#ifdef TM_USE_SSL_VERSION_33
/* Support Signature Algorithms */
/* HashAlgorithm */
#define TM_SSL_SUPPORT_HASH_MD5                (tt8Bit)1
#define TM_SSL_SUPPORT_HASH_SHA1               (tt8Bit)2
#define TM_SSL_SUPPORT_HASH_SHA256             (tt8Bit)4
/* SignatureAlgorithm */
#define TM_SSL_SUPPORT_SIG_RSA                 (tt8Bit)1
#define TM_SSL_SUPPORT_SIG_DSA                 (tt8Bit)2
#endif /* TM_USE_SSL_VERSION_33 */

/* Key exchange method */
#define TM_SSL_KEXCH_NULL           (tt8Bit)0
#define TM_SSL_KEXCH_RSA            (tt8Bit)1
#define TM_SSL_KEXCH_RSAEXPORT      (tt8Bit)2
#define TM_SSL_KEXCH_DH             (tt8Bit)3
#define TM_SSL_KEXCH_DHE            (tt8Bit)4
#define TM_SSL_KEXCH_RSAEXPORT1024  (tt8Bit)5
#define TM_SSL_KEXCH_DHEEXPORT1024  (tt8Bit)6

/* Auth method */

#define TM_SSL_EXPORTABLE           (tt8Bit)1

#define TM_SSL_COMPMETH_NULL           (tt8Bit)0x0

/* for application data, tfSslSend is responsible to build SSL
 * header, trailer, mac, padding
 */
/* For non-application data, the caller to tfSslSend is 
 * responsible for constructing the SSL header stuff. */
/* user data calls this macro. If it is SSL handshake messages,
 * we don't use this flag
 */
#define TM_SSLSF_APPLICATION                   0x01
/* user sets OOB */
#define TM_SSLSF_MSG_OOB                       0x02
/* User is using zero copy buffer */
#define TM_SSLSF_PKT_USER_OWNS                 0x04


/* Flag bits for tcpsSslFlags */
#define TM_SSLF_SEND_ENABLED                   0x0001
#define TM_SSLF_RECV_ENABLED                   0x0002
#define TM_SSLF_CLIENT                         0x0004
#define TM_SSLF_SERVER                         0x0008
#define TM_SSLF_ESTABLISHED                    0x0010
/* indicate that we have more user data to receive */
#define TM_SSLF_MORE_RECORDS                   0x0020
/* indicate we already set the session to a given socket */
#define TM_SSLF_SESSION_SET                    0x0040
#define TM_SSLF_HANDSHK_FAILURE                0x0080
/* 
 * Indicates that we have received a handshake message that need to be
 * processed in the context of the user application
 */
#define TM_SSLF_HANDSHK_PROCESS                0x0100
/*
 * Do not process handshake messages in the TCP incoming path.
 */
#define TM_SSLF_USER_HANDSHK_PROCESS           0x0200
/* Send SSL data with no delay */
#define TM_SSLF_TCP_NO_DELAY                   0x0400
/* We have received a CLOSE Notify */
#define TM_SSLF_RECV_CLOSE_NOTIFY              0x0800

/* client server mask  + user handshake process option */
#define TM_SSLF_CLTSRV_MASK  \
        (TM_SSLF_CLIENT | TM_SSLF_SERVER | TM_SSLF_USER_HANDSHK_PROCESS)

/* The TCP connection has been established */
#define TM_SSL_RUNFLAG_HELLOSENT               0x01
/* last record has not finished yet, we need keep receiving
 * before we process
 */
#define TM_SSL_RUNFLAG_RESUMESESSION           0x02

/* server needs to send cert request in second record */
#define TM_SSL_RUNFLAG_SERVER_SENDCR           0x04

/* server needs to send ephemeral dh key in second record */
#define TM_SSL_RUNFLAG_SERVER_SENDEDH          0x08

/* server needs to send ephemeral RSA key in second record */
#define TM_SSL_RUNFLAG_SERVER_SENDERSA         0x10

/* client needs to send cert in third record*/
#define TM_SSL_RUNFLAG_CLIENT_SENDCERT         0x20

/* client needs to send cert verify in third record */
#define TM_SSL_RUNFLAG_CLIENT_SENDCV           0x40

/* Change cipher received. Following SSL record from the peer 
 * should be encrypted 
 */
#define TM_SSL_RUNFLAG_CCS_RECVD               0x80

/* Rehandshake. All rehandshake messages has to be protected.
 * Before CCS, using the old key material. After CCS, using
 * the new key material
 */
#define TM_SSL_RUNFLAG_REHANDSHAKE             0x100

/* If we allocate exportable key buffer, we need free it
 */
#define TM_SSL_RUNFLAG_EXPORT_KEY              0x200

/* 
 * flag to indicate that tfTcpIncomingPacket=>tfSslIncomingPacket has
 * not returned yet.
 */
#define TM_SSL_RUNFLAG_TCP_INCOMING            0x400

/* To indicate an imcomplete SSL close, SSL must not be resumed */
#define TM_SSL_RUNFLAG_IMCOMP_CLOSE            0x800

/*
 * Client only: to indicate that TCP needs to be notified of
 * SSL connection establishment.
 */
#define TM_SSL_RUNFLAG_TCP_CONNECTED           0x1000

/* An empty fragment (or 1-byte fragment) has been sent */
#define TM_SSL_RUNFLAG_FRAG_SENT               0x2000

/* client needs to send no cert in third record */
#define TM_SSL_RUNFLAG_CLIENT_SEND_NOCERT      0x4000

#define TM_SSL_CLIENT                          0
#define TM_SSL_SERVER                          1

/* Client and Server state, other states are defined in either trsslsrv.c
 * or trsslclt.c
 */
#define TM_SSL_CT_RECV_SH2                     3
#define TM_SSL_CT_SENT_FINISH                  7
#define TM_SSL_CT_RECV_CCS1                    8
#define TM_SSL_CT_RECV_CCS2                    9
#define TM_SSL_CT_OPENED                       10

#define TM_SSL_ST_INITIAL                      0
#define TM_SSL_ST_RECV_CH2                     2
#define TM_SSL_ST_RECV_CKE                     4
#define TM_SSL_ST_RECV_CERTVERI                5
#define TM_SSL_ST_RECV_CCS1                    6
#define TM_SSL_ST_RECV_CCS2                    7
#define TM_SSL_ST_OPENED                       8

/* 4 bytes time + 28 bytes random data*/
#define TM_SSL_RANDOM_SIZE             32

/* 32 bytes session ID */
#define TM_SSL_SESSION_ID_SIZE         32
#define TM_SSL_SESSION_ID_SIZE_MAX     32


#define TM_SSL_NUMBEROF_SESSION_DEFAULT      32

/* RFC 2246. The length should not exceed 2^14 + 2048 */
#define TM_SSL_BUFFER_SIZE_MAX              18432

/* This header means TYPE + Version(2) + Length(2)*/
#define TM_SSL_RECORD_HEADER_SIZE              5
#define TM_SSL_CHANGE_CIPHER_SIZE              1

#define TM_SSL_ENCODE                       (tt8Bit)0x01
#define TM_SSL_DECODE                       (tt8Bit)0x02
/*
 * Record Content Type 
 */
#define TM_SSL_TYPE_CHANGECIPHERSPEC        (tt8Bit)20
#define TM_SSL_TYPE_ALERT                   (tt8Bit)21
#define TM_SSL_TYPE_HANDSHAKE               (tt8Bit)22
#define TM_SSL_TYPE_APPLICATION             (tt8Bit)23
 
/*
 * HandShake Protocol Type
 */
#define TM_SSL_HELLO_REQUEST                 (tt8Bit)0
#define TM_SSL_CLIENT_HELLO                  (tt8Bit)1
#define TM_SSL_SERVER_HELLO                  (tt8Bit)2
#define TM_SSL_CERTIFICATE                   (tt8Bit)11
#define TM_SSL_SERVER_KEY_EXCHANGE           (tt8Bit)12
#define TM_SSL_CERTIFICATE_REQUEST           (tt8Bit)13
#define TM_SSL_SERVER_HELLO_DONE             (tt8Bit)14
#define TM_SSL_CERTIFICATE_VERIFY            (tt8Bit)15
#define TM_SSL_CLIENT_KEY_EXCHANGE           (tt8Bit)16
#define TM_SSL_FINISHED                      (tt8Bit)20
#define TM_SSL_HANDSHAKE_INVALID             (tt8Bit)255


/* Change Cipher */
#define TM_SSL_CHANGECIPHER_VALID            (tt8Bit)1
#define TM_SSL_CHANGECIPHER_INVALID          (tt8Bit)255

#ifdef TM_USE_SSL_VERSION_33
/* The Number of pairs support Sign and Hash algorithms */
#define TM_SSL_SUPPORT_PAIR_MAX              (tt16Bit)8

/* When use signature_algorithms(Client Hello Extension),
 * number of pair algorithms data contains 
 * signature_algorithms header.
 */
#define TM_SSL_EXTENSION_SIG_LEN
#endif /* TM_USE_SSL_VERSION_33 */

/*
 * Alert Protocol Type
 */
/* Alert warning level */
#define TM_SSL_ALTLEV_WARNING                 (tt8Bit)1
#define TM_SSL_ALTLEV_FATAL                   (tt8Bit)2
/* Alert Description */
#define TM_SSL_ALT_CLOSE_NOTIFY               (tt8Bit)0
#define TM_SSL_ALT_UNEXPECTEDMSG              (tt8Bit)10
#define TM_SSL_ALT_BADRECMAC                  (tt8Bit)20
#define TM_SSL_ALT_DECRYPTFAIL                (tt8Bit)21
#define TM_SSL_ALT_RECOVERFLOW                (tt8Bit)22
#define TM_SSL_ALT_DECOMPFAIL                 (tt8Bit)30
#define TM_SSL_ALT_HANDSHAKEFAIL              (tt8Bit)40
#define TM_SSL_ALT_NO_CERTIFICATE             (tt8Bit)41
#define TM_SSL_ALT_BADCERT                    (tt8Bit)42
#define TM_SSL_ALT_UNSUPPORTEDCERT            (tt8Bit)43
#define TM_SSL_ALT_CERTREVOKED                (tt8Bit)44
#define TM_SSL_ALT_CERTEXPIRED                (tt8Bit)45
#define TM_SSL_ALT_CERTUNKNOWN                (tt8Bit)46
#define TM_SSL_ALT_ILLEGALPARAM               (tt8Bit)47
#define TM_SSL_ALT_UNKNOWNCA                  (tt8Bit)48
#define TM_SSL_ALT_ACCESSDENIED               (tt8Bit)49
#define TM_SSL_ALT_DECODEERROR                (tt8Bit)50
#define TM_SSL_ALT_DECRYPTERROR               (tt8Bit)51
#define TM_SSL_ALT_EXPORTRESTRICT             (tt8Bit)60
#define TM_SSL_ALT_PROTOVERSION               (tt8Bit)70
#define TM_SSL_ALT_INSUFFSECURITY             (tt8Bit)71
#define TM_SSL_ALT_INTERNALERR                (tt8Bit)80
#define TM_SSL_ALT_USERCANCELED               (tt8Bit)90
#define TM_SSL_ALT_NORENEGOTIATION            (tt8Bit)100
#define TM_SSL_ALT_UNSUPPORTEDEXT             (tt8Bit)110
#define TM_SSL_ALT_INVALID                    (tt8Bit)255



/* SESSION ID position from the very begining of handshake type
 * HS type(1) + Length(3) + version(2) + Random(32) + SessionLength(1)
 */
#define TM_SSL_HELLO_SESSIONID_OFFSET           39

/* maximum overhead for each SSL record: SHA1 + AES padding */
#define TM_SSL_RECORD_OVERHEAD_MAX_SIZE         36 

#define TM_SSL_HANDSHAKE_HEADER_SIZE            4
             
typedef struct tsSslHandShakeHeader
{
    tt8Bit      hsHandShakeType;
    tt8Bit      hsLengthPad;
    tt8Bit      hsLengthHigh;
    tt8Bit      hsLengthLow;
}
ttSslHandShakeHeader;
typedef ttSslHandShakeHeader TM_FAR * ttSslHandShakeHeaderPtr;

typedef struct tsClientHello
{
    tt8Bit      chVersionMajor;
    tt8Bit      chVersionMinor;
    tt8Bit      chRandom[TM_SSL_RANDOM_SIZE];
    tt8Bit      chSessionIdLength;
    tt8Bit      chSessionIdBegin;
}
ttClientHello;
typedef ttClientHello TM_FAR * ttClientHelloPtr;

#ifdef TM_SSL_ACCEPT_20_CLIENTHELLO
typedef struct tsClientHello20
{
    tt8Bit      ch20MsgType;
    tt8Bit      ch20VersionMajor;
    tt8Bit      ch20VersionMinor;
    tt8Bit      ch20CipherLenHigh;
    tt8Bit      ch20CipherLenLow;
    tt8Bit      ch20SessionLenHigh;
    tt8Bit      ch20SessionLenLow;
    tt8Bit      ch20ChallengeHigh;
    tt8Bit      ch20ChallengeLow;
}
ttClientHello20;
typedef ttClientHello20 TM_FAR * ttClientHello20Ptr;

#endif /*TM_SSL_ACCEPT_20_CLIENTHELLO*/

typedef struct tsServerHello
{
    tt8Bit      shVersionMajor;
    tt8Bit      shVersionMinor;
    tt8Bit      shRandom[TM_SSL_RANDOM_SIZE];
    tt8Bit      shSessionIdLength;
    tt8Bit      shSessionIdBegin;
}
ttServerHello;
typedef ttServerHello TM_FAR * ttServerHelloPtr;

/* 
 * Transition matrix Entry for PPP event/state processing 
 */
#ifdef TM_DSP
typedef struct tsSslTransitionMatrixEntry
{
    unsigned int    smtxTransition     :8;
    unsigned int    smtxProcedureIndex  :8;
} ttSslTransitionMatrixEntry;
#else /* ! TM_DSP */
typedef struct tsSslTransitionMatrixEntry
{
    tt8Bit          smtxTransition;
    tt8Bit          smtxProcedureIndex;
} ttSslTransitionMatrixEntry;
#endif /* TM_DSP */

typedef ttSslTransitionMatrixEntry TM_CONST_QLF * 
            ttSslTransitionMatrixEntryPtr;

/* 
 * SSL state function entry 
 */
#ifdef TM_DSP
typedef struct tsSslStateProcedureEntry
{
    unsigned int    sprocIndex        :8;
    unsigned int    sstaFunctIndex   :8;
} ttSslStateProcedureEntry;
#else /* !TM_DSP */
typedef struct tsSslStateProcedureEntry
{
    tt8Bit          sprocIndex;
    tt8Bit          sstaFunctIndex;
} ttSslStateProcedureEntry;
#endif /* TM_DSP */

typedef ttSslStateProcedureEntry TM_CONST_QLF * ttSslStateProcedureEntryPtr;

/* Ssl state function pointer */
typedef int (*ttSslStateFunctPtr)(ttVoidPtr    sslContextPtr,
                                  ttPacketPtr  packetPtr);

/* We will cache the following fields for each connection,
 * We won't cache ttSslConnectState, because it is pretty big
 * for example, the random numbers... 
 */
typedef struct tsSslCacheElement
{
/* peer address, session id, connectionState, and next element
 */
    struct tsSslCacheElment TM_FAR * sceNextPtr;
#ifdef TM_USE_SSL_CLIENT
    ttIpAddress                      scePeerAddr;
    ttIpPort                         scePeerPort;
#endif /* TM_USE_SSL_CLIENT */
/* The 0 and 1 bit indicates the version.*/
    tt16Bit                          sceElementStatus;
    tt16Bit                          sceCipherSuite;
    tt8Bit                           sceVersion;
    tt8Bit                           sceCompressMethod;
    tt8Bit                           sceSessionIdLength;
    tt8Bit                           scePadding;
    tt8Bit                           sceSessionId[TM_SSL_SESSION_ID_SIZE_MAX];
    tt8Bit                           sceMasterSecret[TM_SSL_MASTER_SECRET_SIZE];
}
ttSslCacheElement;
typedef ttSslCacheElement TM_FAR * ttSslCacheElementPtr;

/*
 * SSL entry structure (pointed to by pointer in socket entry)
 */
typedef struct tsSslCipherSuite
{
    tt16Bit                     csNumber;
    tt8Bit                      csSignAlgorithm;
    tt8Bit                      csKeyExchange;
    tt8Bit                      csEncryptAlg;
    tt8Bit                      csHashAlg;
    tt8Bit                      csKeyLength; /* in octets */
/* if expanded length is non-zero, which means it is an 
 * expandible cipher. We need to expand the key size
 * to this specified length
 */
    tt8Bit                      csExpandedLength;
}
ttSslCipherSuite;
typedef ttSslCipherSuite TM_FAR * ttSslCipherSuitePtr;
typedef ttSslCipherSuite const TM_FAR * ttSslCipherSuiteConstPtr;

typedef struct tsSslConnectState
{
/* double link list to link all authentication structures */
    ttNode                       scsConnectNode;
    struct tsSslSession TM_FAR * scsSessionPtr;
/* back pointer to scsTcpVectPtr */
    ttTcpVectPtr                 scsTcpVectPtr;
/* Peer Certificate list per connection, not per session */
    ttPkiCertListPtr             scsPeerCertListPtr;
/* you must keep the following random number contiguous, the master_secret 
 * needs them */
    tt8Bit                       scsClientRandom[TM_SSL_RANDOM_SIZE];
    tt8Bit                       scsServerRandom[TM_SSL_RANDOM_SIZE];
/* master key */
    tt8Bit                       scsMasterSecret[TM_SSL_MASTER_SECRET_SIZE];
    tt8Bit                       scsSessionId[TM_SSL_SESSION_ID_SIZE_MAX];
/* if EDH is used, scsPreMasterPtr points to the DH shared secret */
    tt8BitPtr                    scsPreMasterPtr;
/* The following key blocks are needed */
/* we maintain one key block buffer */
    tt8BitPtr                    scsKeyBlockPtr;
/* backup key block buffer */
    tt8BitPtr                    scsKeyBlockBackupPtr;
/* if 40 bit key need to expand is used, we need to save to this buffer*/
    tt8BitPtr                    scsExportKeyPtr;
    tt8BitPtr                    scsExportKeyBackupPtr;
    ttGenericKey                 scsEncodeMacKey;
    ttGenericKey                 scsDecodeMacKey;
    ttGenericKey                 scsEncodeCryptoKey;
    ttGenericKey                 scsDecodeCryptoKey;
    tt8BitPtr                    scsEncodeIvPtr;
    tt8BitPtr                    scsDecodeIvPtr;

/* points to the SSL to_be_read buffer. All data here is processed SSL 
 * record, i.e application data, in ttPacketPtr */
    ttPacketPtr                  scsReadPktPtr;
    ttPacketPtr                  scsReadPktTailPtr;
/* points to the SSL write buffer, we queue user's application data here
 * until we reach the threshhold to write out. 
 */
    ttPacketPtr                  scsWritePktPtr;
    ttPacketPtr                  scsWritePktTailPtr;
/* for handshake exchange, we allocate scsXmitPacketPtr for each round
 * when we are going to send back anything 
 */
    ttPacketPtr                  scsXmitPacketPtr;
    tt8BitPtr                    scsXmitDataPtr;

/* crypto engine is used to encrypt/decrypt data, hopefully, hashing
 * should use this crypto engine 
 */
    ttCryptoEnginePtr            scsCryptoEnginePtr;
/* public key engine is used to sign/verify, hopefully, diffie-hellman
 * crypto engine should use this one
 */
    ttCryptoEnginePtr            scsPubkeyEnginePtr;
/* to save md5 and sha1, or sha256 state whenever a handshake message is sent */
    ttMd5CtxPtr                  scsMd5CtxPtr;
    ttSha1CtxPtr                 scsSha1CtxPtr;
#ifdef TM_USE_SSL_VERSION_33
    ttSha256CtxPtr               scsSha256CtxPtr;
#endif /* TM_USE_SSL_VERSION_33 */
/* algorithm pointer for our hash algorithm */
    ttAhAlgorithmPtr             scsHashAlgPtr;
/* algorithm pointer for our crypto algorithm */
    ttEspAlgorithmPtr            scsCryptoAlgPtr;
/* This cipher suite is what we are negotiating */
    ttSslCipherSuiteConstPtr     scsCipherPtr;

#ifdef TM_SSL_USE_EPHEMERAL_DH
/* when using ephemeral d-h exchange, we need to finish
 * one Diffie-Hellman calculation.
 */
    ttDiffieHellmanPtr           scsDHInfoPtr;
#endif /* TM_SSL_USE_EPHEMERAL_DH */
#ifdef TM_USE_SSL_CLIENT
/* If the server is under export restrict and using ephemeral
 * RSA, we need to have place to store that eRSA info.
 * For SSL server, we store it into the session structure.
 */
#ifdef TM_PUBKEY_USE_RSA
    ttGeneralRsaPtr              scsEphemRsaPtr;
#endif /* TM_PUBKEY_USE_RSA */
    char TM_FAR *                scsServerCNPtr;
#endif /* TM_USE_SSL_CLIENT */

/* each connection state needs maintain a 64 bit sequence number for each
 * direction.
 */
    tt32Bit                      scsOutSeqNumber;
    tt32Bit                      scsOutSeqNumberHigh;
    tt32Bit                      scsInSeqNumber;
    tt32Bit                      scsInSeqNumberHigh;
/* set state related flags here */
    tt32Bit                      scsRunFlags;
    unsigned int                 scsSocIndex;
    tt16Bit                      scsXmitBufferSize;
    tt16Bit                      scsXmitDataLength;
    tt16Bit                      scsPreMasterLen;
/* current to be read SSL record size, this size doesnt include record 
 * header
 */
    tt16Bit                      scsNextReadRecordSize;
/* next to be written record size, doesn't include the record header */
    tt16Bit                      scsNextWriteRecordSize;
/* how many bytes we need to finish this SSL record.*/
    tt16Bit                      scsNextReadRecordSizeToGo;
    tt16Bit                      scsTotalCertBytes;
/* 
 * Temporarily store CB flags for user notification, so that in the incoming
 * path, we can just let TCP do the notification for us, and prevent loss
 * of our TCP vector/socket lock, that would make us have to recheck on the
 * connection state pointer still being there on the TCP vector.
 */
    tt16Bit                      scsSocCBFlags;
/* State tracking the command/response exchange */
    tt8Bit                       scsState;
    tt8Bit                       scsSessionIdLength;
/* how many bytes for one block, i.e. our maximum padding length */
    tt8Bit                       scsBlockSize;
/* how many bytes for our hashing algorithm output */
    tt8Bit                       scsHashOutputSize;
    tt8Bit                       scsHeaderBufferCount;
/* in case you get half of the SSL record header, copy to scsHeaderBuffer
 * so that we can read the length field later. scsHeaderBufferCount
 * stores how many bytes we right now have
 */
    tt8Bit                       scsHeaderBuffer[TM_SSL_RECORD_HEADER_SIZE];
    tt8Bit                       scsClientOrServer;
    tt8Bit                       scsVersion;
/* Copy from session version */
    tt8Bit                       scsSsVersion;
    tt8Bit                       scsCompressMethod;
#ifdef TM_USE_SSL_SERVER
    tt8Bit                       scsClientVersion;
#endif /* TM_USE_SSL_SERVER */
} ttSslConnectState;
typedef ttSslConnectState TM_FAR * ttSslConnectStatePtr;

#ifdef TM_USE_SSL_SERVER
#ifdef TM_SSL_USE_MUTUAL_AUTH
typedef struct tsSslClientAuthCalist
{
    ttPkiCertListPtr                      scacCertPtr;
    struct tsSslClientAuthCalist TM_FAR * scacNextPtr; 
}ttSslClientAuthCalist;
typedef ttSslClientAuthCalist TM_FAR * ttSslClientAuthCalistPtr;
#endif /* TM_SSL_USE_MUTUAL_AUTH */
#endif /* TM_USE_SSL_SERVER */

typedef struct tsSslSession
{  
#ifdef TM_LOCK_NEEDED
    ttLockEntry                  ssLockEntry;
#endif /* TM_LOCK_NEEDED */
#if (defined(TM_USE_SSL_SERVER) && defined(TM_SSL_USE_MUTUAL_AUTH ))
    ttSslClientAuthCalistPtr     ssClientAuthCasPtr;
#endif /* TM_USE_SSL_SERVER && TM_SSL_USE_MUTUAL_AUTH*/
#ifdef TM_PUBKEY_USE_RSA
/* for exportable cipher, we can't use RSA longer than 512 bits, use it to 
 * sign only. In this case, we need to have an ephemeral RSA to exchange.
 */
    ttPkiPublicKeyPtr            ssEphemRsaPtr;
#endif /* TM_PUBKEY_USE_RSA */

/* cache each successful connection using this session. For each connection,
 * we need construct a structure ttSslCacheElement in order to save the 
 * important information of a connection, such information includes but not
 * limits to sessionId, sessionIdLength, cipherSuite, compressMethod,
 * and most importantly, the master key. The purpose for us to cache each 
 * connection is to avoid calculation master key. We could simply cache each
 * ttSslConnectState, however, it is too big comparing to structre 
 * ttSslCacheElement. If the cache array overflows, we overwrite the previous
 * one, remember to free the previous one
 */
    ttSslCacheElementPtr TM_FAR *ssConnectCache;

/* Our certificate */
    ttPkiCertListPtr             ssCertListPtr;
#ifdef TM_USE_SSL_VERSION_33
    ttPkiCertListPtr             ssCertSuppPtr;
#endif /* TM_USE_SSL_VERSION_33 */
    ttSslCertCallbackFuncPtr     ssCertCBFuncPtr;
/* we use an ID to refer a specified Certificate */
    char TM_FAR *                ssMyIdPtr;
/* We save each active connect's socket index in this circular double linked
 * list. So that when user frees a session, we can disconnect all active
 * connections using this session. And if user disconnects a single 
 * connection, we just simply kick it out of the double linked-list.
 */

    ttList                       ssActiveConnectList;
    unsigned int                 ssCacheIndex;
    unsigned int                 ssTotalCache;
    tt16Bit                      ssProposals[TM_SSL_PROPOSAL_NUMBER];
    tt16Bit                      ssOption;
    tt16Bit                      ssNumber;
    tt16Bit                      ssOwnerCount; /* version owner count */
    tt8Bit                       ssVersion;
    tt8Bit                       ssSignAlgorithm;/*auth method, using rsa or dss*/
#ifdef TM_USE_SSL_VERSION_33
    tt8Bit                       ssHashAlgorithm;
#endif /* TM_USE_SSL_VERSION_33 */
/* my session number */
} ttSslSession;
typedef ttSslSession TM_FAR * ttSslSessionPtr;

typedef struct tsSsLRecordHeader
{
    tt8Bit         srhType;
    tt8Bit         srhVersionMajor;
    tt8Bit         srhVersionMinor;
    tt8Bit         srhLengthHigh;
    tt8Bit         srhLengthLow;
}
ttSslRecordHeader;
typedef ttSslRecordHeader TM_FAR * ttSslRecordHeaderPtr;

typedef struct tsSslGlobalEntry
{
    ttSslSessionPtr TM_FAR *     sgSessionArray;
    tt16Bit                      sgMaxSession;
    tt16Bit                      sgCurrentSession;
}ttSslGlobalEntry;
typedef ttSslGlobalEntry TM_FAR * ttSslGlobalEntryPtr;

#ifdef TM_USE_SSL_CLIENT
/*
 * Used by TM_USE_SSL_CLIENT only 
 */
void tfTcpSslConnected(ttTcpVectPtr tcpVectPtr);
int tfSslClientProcessHandShake(tt8Bit                hsType,
                                ttSslConnectStatePtr  connStatePtr,
                                tt8BitPtr             dataPtr,
                                tt16Bit               msgSize);
int tfSslParseServerCCS(ttSslConnectStatePtr   connStatePtr,
                        ttPacketPtr            packetPtr);
#endif /* TM_USE_SSL_CLIENT */

#ifdef TM_USE_SSL_SERVER
/*
 * Used by TM_USE_SSL_SERVER only 
 */
int tfSslServerCreateState(ttTcpVectPtr   tcpVectPtr);
int tfSslParseClientCCS(ttSslConnectStatePtr   connStatePtr,
                        ttPacketPtr            packetPtr);

int tfSslServerProcessHandShake(tt8Bit                hsType,
                                ttSslConnectStatePtr  connStatePtr,
                                tt8BitPtr             dataPtr,
                                tt16Bit               msgSize);
#endif /* TM_USE_SSL_SERVER */

#ifdef TM_USE_SSL_VERSION_30
int tfSsl30GetHandShakeHash(ttSslConnectStatePtr   connStatePtr,
                            ttMd5CtxPtr            md5Ptr,
                            ttSha1CtxPtr           sha1Ptr,
                            char TM_FAR *          labelPtr,
                            int                    labelLength,
                            tt8BitPtr              digestOutPtr);

int tfSsl30GetRecordMac(ttSslConnectStatePtr     connStatePtr,
                        ttSslRecordHeaderPtr     sslHeaderPtr,
                        ttPacketPtr              headPktPtr,
                        ttPktLen                 offset,
                        ttPktLen                 dataLength,
                        tt8Bit                   direction,
                        tt8BitPtr                macOutPtr);
#endif /* TM_USE_SSL_VERSION_30 */

#if (defined(TM_USE_SSL_VERSION_31) || defined(TM_USE_SSL_VERSION_32)\
  || defined(TM_USE_SSL_VERSION_33))
int tfTlsGetRecordMac(ttSslConnectStatePtr         connStatePtr,
                      ttSslRecordHeaderPtr         sslHeaderPtr,
/* the head packet should be connStatePtr->scsWritePktPtr*/
                      ttPacketPtr                  headPktPtr, 
                      ttPktLen                     offset,
                      ttPktLen                     dataLength,
                      tt8Bit                       direction,
                      tt8BitPtr                    macOutPtr);

/*
 * Used by both TM_USE_SSL_CLIENT and TM_USE_SSL_SERVER
 */
#if (defined(TM_USE_SSL_VERSION_31) || defined(TM_USE_SSL_VERSION_32))
int tfTlsPrf(ttSslConnectStatePtr  connStatePtr,
             tt8BitPtr             secretPtr,
             int                   secretLength,
             char TM_FAR *         labelPtr,
             int                   labelLength,
             tt8BitPtr             seedPtr,
             int                   seedLength,
             tt8BitPtr             outputPtr,
             int                   outputLength);
#endif /* TM_USE_SSL_VERSION_31 || TM_USE_SSL_VERSION_32 */
#ifdef TM_USE_SSL_VERSION_33
int tfTlsPrf2(ttSslConnectStatePtr  connStatePtr,
              tt8BitPtr             secretPtr,
              int                   secretLength,
              char TM_FAR *         labelPtr,
              int                   labelLength,
              tt8BitPtr             seedPtr,
              int                   seedLength,
              tt8BitPtr             outputPtr,
              int                   outputLength);
#endif /* TM_USE_SSL_VERSION_33 */
#endif /* TM_USE_SSL_VERSION_31 || TM_USE_SSL_VERSION_32)
       || TM_USE_SSL_VERSION_33 */

int tfSslSend(ttSocketEntryPtr        socketPtr,
              ttSslConnectStatePtr    connStatePtr,
              ttPacketPtrPtr          packetPtrPtr,
              ttPktLen TM_FAR *       bufferLengthPtr,
              int                     flags);

/* Get session pointer using Session ID (index), and lock it */
ttVoidPtr tfSslGetSessionUsingIndex(int             sessionNumber);
/* decrease owner count, unlock session */
void  tfSslSessionReturn(ttSslSessionPtr sslSessionPtr);
/* Remove connStatePtr from list of active nodes on the session */
void tfSslConnectNodeRemove(ttSslConnectStatePtr    connStatePtr);
/* Store session into global array */
int tfSslPutSessionIntoGlobal(ttSslSessionPtr sessionPtr);
int tfSslGenerateMasterSecret(ttSslConnectStatePtr  connStatePtr);
int tfSslRecordEncode(ttSslConnectStatePtr          connStatePtr,
                      ttSslRecordHeaderPtr          sslHeaderPtr,
                      ttPacketPtr                   sslPacketPtr,
                      ttPktLen                      dataOffset,
                      ttPktLen                      dataLength);
int tfSslIncomingPacket(ttTcpVectPtr                tcpVectPtr, 
                        ttPacketPtr                 packetPtr);

int tfSslSendCertificate(ttSslConnectStatePtr       connStatePtr,
                         ttVoidPtr                  packetPtr);
int tfSslParseCertificate(ttSslConnectStatePtr      connStatePtr,
                          tt8BitPtr                 dataPtr,
                          tt16Bit                   msgSize);
int  tfSslClose(ttSslConnectStatePtr                connStatePtr,
                int                                 caller);
void tfSslPrefreeConnState(ttSslConnectStatePtr     connStatePtr);
void tfSslClearConnState( ttSslConnectStatePtr      connStatePtr,
                          ttTcpVectPtr              tcpVectPtr,
                          int                       tcpSslFlags );
int tfSslGetCertList(ttSslSessionPtr    sessionPtr);

void tfSslUpdateMd5Sha1Ctx(ttSslConnectStatePtr     connStatePtr,
                           tt8BitPtr                handshakeHeaderPtr,
                           tt16Bit                  msgLength);
int tfSslHandshakeMsgTransform(ttSslConnectStatePtr connStatePtr,
                               int                  direction,
                               ttSslRecordHeaderPtr sslHeaderPtr,
                               ttPacketPtr          packetPtr,
                               tt8BitPtr            macOutPtr,
                               tt16BitPtr           recordLenPtr,
                               tt16Bit              headOffset);
int tfSslSoftwareRequestProcess(ttCryptoRequestPtr  crqPtr);
int tfSslGenerateCryptoMacKeys(ttSslConnectStatePtr connStatePtr);
#ifdef TM_SSL_USE_MUTUAL_AUTH
int tfSslBuildCertVerify(ttSslConnectStatePtr       connStatePtr, 
                         tt8BitPtr                  dataPtr,
                         tt32BitPtr                 certVeriLenPtr);
#endif /* TM_SSL_USE_MUTUAL_AUTH */
int tfSslBuildSendFinished(ttSslConnectStatePtr     connStatePtr,
                           ttVoidPtr                paramPtr);
int tfSslParseFinished(ttSslConnectStatePtr         connStatePtr,
                       tt8BitPtr                    dataPtr,
                       tt16Bit                      msgSize);
int tfSslSendQueuedData(ttSslConnectStatePtr        connStatePtr,
                        ttVoidPtr                   paramPtr);
int tfSslIncomingApplicationData(ttSslConnectStatePtr  connStatePtr);
int tfSslIncomingHandShake(ttSslConnectStatePtr connStatePtr,
                           ttPacketPtr          packetPtr);
int tfSslIncomingRecord(ttSslConnectStatePtr        connStatePtr,
                        tt8Bit                      context);
ttSslCipherSuiteConstPtr tfSslCipherSuiteLookup(tt16Bit   cipherNumber);
int tfSslFreeCache(ttSslSessionPtr                   sessionPtr,
                   tt8BitPtr                         sessionIdPtr,
                   tt8Bit                            sessionIdLen);
int tfSslClientWriteKeySchedule(ttSslConnectStatePtr connStatePtr,
                                ttGenericKeyPtr      swMacKeyPtr,
                                ttGenericKeyPtr      swCryptoKeyPtr,
                                tt8BitPtrPtr         swIvPtrPtr);
int tfSslServerWriteKeySchedule(ttSslConnectStatePtr connStatePtr,
                                ttGenericKeyPtr      swMacKeyPtr,
                                ttGenericKeyPtr      swCryptoKeyPtr,
                                tt8BitPtrPtr         swIvPtrPtr);

int tfSslBuildSendAlert(ttSslConnectStatePtr   connStatePtr,
                        tt8Bit                 alertLevel,
                        tt8Bit                 alertType);

#ifdef TM_USE_SSL_VERSION_33
int tfSslRsaPkcsVerify(ttPkiPublicKeyPtr       publicKeyPtr,
                       tt8BitPtr               dataPtr,
                       tt8BitPtr               hashPtr,
                       tt16Bit                 dataLen);

int tfSslRsaPkcs1Encode(ttPkiPublicKeyPtr      privateKeyPtr,
                        tt8BitPtr              hashPtr,
                        tt8BitPtr              dataPtr,
                        ttIntPtr               dataLenPtr,
                        tt8Bit                 hashAlg);
#endif /* TM_USE_SSL_VERSION_33 */

#ifdef __cplusplus
}
#endif

#endif /* _TRSSL_H_ */
