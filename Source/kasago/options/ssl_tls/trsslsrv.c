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
 * Description: SSL and TLS Server implementation
 *
 * Filename: trsslsrv.c
 * Author: Jin Zhang
 * Date Created:   12/07/2003
 * $Source: source/trsslsrv.c $
 *
 * Modification History
 * $Revision: 6.0.2.32 $
 * $Date: 2016/01/26 17:25:43JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * Include
 */
#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_SSL_SERVER

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>
#include <trssl.h>

/* Server Hello, assumes 2048 bytes, not just server hello, but also includes
 * the second round server reply, such as certificate, server hello done
 */


/* SSL Server function pointer indexes */
#define TM_SSL_SFN_PREREPLY          0
#define TM_SSL_SFN_SERVER_HELLO      1
#define TM_SSL_SFN_CERTIFICATE       2
#define TM_SSL_SFN_SERVER_KE         3
#define TM_SSL_SFN_CERT_REQUEST      4
#define TM_SSL_SFN_HELLO_DONE        5
#define TM_SSL_SFN_CHANGE_CIPHER     6
#define TM_SSL_SFN_FINISHED          7
#define TM_SSL_SFN_SENDQUEUE         8
#define TM_SSL_SFN_NOOP              255
/*
 * SSL Server States
 */
/* #define TM_SSL_ST_INITIAL          0 */
#define TM_SSL_ST_RECV_CH1            1 /* Received the regular Client Hello*/
#define TM_SSL_ST_RECV_CERT           3 /* Received the client certificate  */
/* #define TM_SSL_ST_RECV_CKE         4  Received the client KE
 * #define TM_SSL_ST_RECV_CH2         2  Received the resuming Clint Hello
 * #define TM_SSL_ST_RECV_CERTVERI    5
 * #define TM_SSL_ST_RECV_CCS1        6  Received the regular changecipher
 * #define TM_SSL_ST_RECV_CCS2        7  Received the resuming changcipher
 * #define TM_SSL_ST_OPENED           8
 * defined in trssl.h
 */
#define TM_SSL_ST_INVALID          0xff

/*
 * SSL Server Events
 */
#define TM_SSL_SE_INCH1               0
#define TM_SSL_SE_INCH2               1
#define TM_SSL_SE_INCERT              2
#define TM_SSL_SE_INCKE               3
#define TM_SSL_SE_INVERIFY            4
#define TM_SSL_SE_INCCS               5
#define TM_SSL_SE_INFINISH            6
#define TM_SSL_SE_NOEVENT          (tt8Bit)255

static int tfSslServerStateMachine(ttSslConnectStatePtr        connectStatusPtr,
                            ttVoidPtr                   argUnion,
                            tt8Bit                      event);
static int tfSslServerPreReply(ttSslConnectStatePtr    connStatePtr,
                               ttVoidPtr               paramPtr);
static int tfSslSendServerHello(ttSslConnectStatePtr    connStatePtr,
                                ttVoidPtr               paramPtr);
static int tfSslSendCertRequest(ttSslConnectStatePtr    connStatePtr,
                                ttVoidPtr               paramPtr);
static int tfSslSendHelloDone(ttSslConnectStatePtr    connStatePtr,
                                  ttVoidPtr               paramPtr);
static int tfSslSendServerCCS(ttSslConnectStatePtr    connStatePtr,
                              ttVoidPtr               paramPtr);
static int tfSslSendServerKE(ttSslConnectStatePtr    connStatePtr,
                                  ttVoidPtr          paramPtr);
static int tfSslServerSessAddClientAuthCa(ttSslSessionPtr sslSessionPtr,
                                          char   TM_FAR * caIdPtr);
/*
 * Verify that we have allocated the crypto engines, and
 * allocate MD5, and Sha1 structures.
 */
static int tfSslParseClientHelloAlloc(ttSslConnectStatePtr  connStatePtr);

/* re-allocate a buffer discarding old content */
static ttVoidPtr tfSslReAlloc(ttVoidPtr bufferPtr, int bufferLen);

/* The following array helps us to get the final version when we process
 * the client hello message.
 * client_minor  session_version  search_index(c_m << 2|s_v)   results
 *                                          0(n/a)                null
 *    0               1                     1                     1
 *    0               2                     2                     null
 *    0               1|2 = 3               3                     1
 *                                          4(n/a)                null
 *    1               1                     5                     1
 *    1               2                     6                     2
 *    1               1|2 = 3               7                     2
 */
static const tt8Bit TM_CONST_QLF tlVersion[] = {0,
                                         TM_SSL_VERSION_30,
                                         0,
                                         TM_SSL_VERSION_30,
                                         0,
                                         TM_SSL_VERSION_30,
                                         TM_SSL_VERSION_31,
                                         TM_SSL_VERSION_31};


/* function in this table means what server should do, generally,
 * it means need send something.
 */
static const ttSslStateFunctPtr  TM_CONST_QLF tlSslServerFuncTable[] =
{
    (ttSslStateFunctPtr)tfSslServerPreReply,    /*TM_SSL_SFN_PREREPLY     */
    (ttSslStateFunctPtr)tfSslSendServerHello,   /*TM_SSL_SFN_SERVER_HELLO */
    (ttSslStateFunctPtr)tfSslSendCertificate,   /*TM_SSL_SFN_CERTIFICATE  */
    (ttSslStateFunctPtr)tfSslSendServerKE,      /*TM_SSL_SFN_SERVER_KE    */
    (ttSslStateFunctPtr)tfSslSendCertRequest,   /*TM_SSL_SFN_CERT_REQUEST */
    (ttSslStateFunctPtr)tfSslSendHelloDone,     /*TM_SSL_SFN_HELLO_DONE   */
    (ttSslStateFunctPtr)tfSslSendServerCCS,     /*TM_SSL_SFN_CHANGE_CIPHER*/
    (ttSslStateFunctPtr)tfSslBuildSendFinished, /*TM_SSL_SFN_FINISHED     */
    (ttSslStateFunctPtr)tfSslSendQueuedData     /*TM_SSL_SFN_SENDQUEUE    */
};

/*
 * Action Procedure Index Defines
 * These are tied to the Function Table
 * If this is changed, the function table
 * MUST be changed as well and visa-versa.
 */
#define TM_SSL_SM_REGULAR_HELLO       0
#define TM_SSL_SM_RESUMING_HELLO      6
#define TM_SSL_SM_FINISH              9
#define TM_SSL_SM_RESUMEFINISH        12
#define TM_SSL_SM_LAST_INDEX          13
#define TM_SSL_SM_TRANS_NOOP          TM_SSL_SM_LAST_INDEX

static const ttSslStateProcedureEntry  TM_CONST_QLF tlSslServerProcEntry[] =
{
    {TM_SSL_SM_REGULAR_HELLO,    TM_SSL_SFN_PREREPLY    }, /* 00 */
    {TM_SSL_SM_REGULAR_HELLO,    TM_SSL_SFN_SERVER_HELLO}, /* 00 */
    {TM_SSL_SM_REGULAR_HELLO,    TM_SSL_SFN_CERTIFICATE }, /* 00 */
    {TM_SSL_SM_REGULAR_HELLO,    TM_SSL_SFN_SERVER_KE   }, /* 00 */
    {TM_SSL_SM_REGULAR_HELLO,    TM_SSL_SFN_CERT_REQUEST}, /* 00 */
    {TM_SSL_SM_REGULAR_HELLO,    TM_SSL_SFN_HELLO_DONE  }, /* 00 */
    {TM_SSL_SM_RESUMING_HELLO,   TM_SSL_SFN_SERVER_HELLO}, /* 06 */
    {TM_SSL_SM_RESUMING_HELLO,   TM_SSL_SFN_CHANGE_CIPHER},/* 06 */
    {TM_SSL_SM_RESUMING_HELLO,   TM_SSL_SFN_FINISHED    }, /* 06 */
    {TM_SSL_SM_FINISH,           TM_SSL_SFN_CHANGE_CIPHER},/* 09 */
    {TM_SSL_SM_FINISH,           TM_SSL_SFN_FINISHED    }, /* 09 */
    {TM_SSL_SM_FINISH,           TM_SSL_SFN_SENDQUEUE   }, /* 09 */
    {TM_SSL_SM_RESUMEFINISH,     TM_SSL_SFN_SENDQUEUE   }, /* 12 */
    {TM_SSL_SM_TRANS_NOOP ,      255                    }  /* 13 */
};



static const ttSslTransitionMatrixEntry TM_CONST_QLF tlSslServerMatrix[9][7] =
{
    {/* State == TM_SSL_ST_INITIAL */
    /*  Event                     Next State       Action Procedure Index */
    {/* TM_SSL_SE_INCH1    */ TM_SSL_ST_RECV_CH1,  TM_SSL_SM_REGULAR_HELLO },
    {/* TM_SSL_SE_INCH2    */ TM_SSL_ST_RECV_CH2,  TM_SSL_SM_RESUMING_HELLO},
    {/* TM_SSL_SE_INCERT   */ TM_SSL_ST_INITIAL,   TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCKE    */ TM_SSL_ST_INITIAL,   TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INVERIFY */ TM_SSL_ST_INITIAL,   TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCCS    */ TM_SSL_ST_INITIAL,   TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INFINISH */ TM_SSL_ST_INITIAL,   TM_SSL_SM_TRANS_NOOP    }
    },

    {/* State == TM_SSL_ST_RECV_CH1 */
    /*  Event                     Next State       Action Procedure Index */
    {/* TM_SSL_SE_INCH1    */ TM_SSL_ST_RECV_CH1,  TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCH2    */ TM_SSL_ST_RECV_CH1,  TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCERT   */ TM_SSL_ST_RECV_CERT, TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCKE    */ TM_SSL_ST_RECV_CKE,  TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INVERIFY */ TM_SSL_ST_RECV_CH1,  TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCCS    */ TM_SSL_ST_RECV_CH1,  TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INFINISH */ TM_SSL_ST_RECV_CH1,  TM_SSL_SM_TRANS_NOOP    }
    },

    {/* State == TM_SSL_ST_RECV_CH2 */
    /*  Event                     Next State       Action Procedure Index */
    {/* TM_SSL_SE_INCH1    */ TM_SSL_ST_RECV_CH2,  TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCH2    */ TM_SSL_ST_RECV_CH2,  TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCERT   */ TM_SSL_ST_RECV_CH2,  TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCKE    */ TM_SSL_ST_RECV_CH2,  TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INVERIFY */ TM_SSL_ST_RECV_CH2,  TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCCS    */ TM_SSL_ST_RECV_CCS2, TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INFINISH */ TM_SSL_ST_RECV_CH2,  TM_SSL_SM_TRANS_NOOP    }
    },

    {/* State == TM_SSL_ST_RECV_CERT */
    /*  Event                     Next State       Action Procedure Index */
    {/* TM_SSL_SE_INCH1    */ TM_SSL_ST_RECV_CERT, TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCH2    */ TM_SSL_ST_RECV_CERT, TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCERT   */ TM_SSL_ST_RECV_CERT, TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCKE    */ TM_SSL_ST_RECV_CKE , TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INVERIFY */ TM_SSL_ST_RECV_CERT, TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCCS    */ TM_SSL_ST_RECV_CERT, TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INFINISH */ TM_SSL_ST_RECV_CERT, TM_SSL_SM_TRANS_NOOP    }
    },

    {/* State == TM_SSL_ST_RECV_CKE */
    /*  Event                     Next State       Action Procedure Index */
    {/* TM_SSL_SE_INCH1    */ TM_SSL_ST_RECV_CKE,     TM_SSL_SM_TRANS_NOOP },
    {/* TM_SSL_SE_INCH2    */ TM_SSL_ST_RECV_CKE,     TM_SSL_SM_TRANS_NOOP },
    {/* TM_SSL_SE_INCERT   */ TM_SSL_ST_RECV_CKE,     TM_SSL_SM_TRANS_NOOP },
    {/* TM_SSL_SE_INCKE    */ TM_SSL_ST_RECV_CKE ,    TM_SSL_SM_TRANS_NOOP },
    {/* TM_SSL_SE_INVERIFY */ TM_SSL_ST_RECV_CERTVERI,TM_SSL_SM_TRANS_NOOP },
    {/* TM_SSL_SE_INCCS    */ TM_SSL_ST_RECV_CCS1,    TM_SSL_SM_TRANS_NOOP },
    {/* TM_SSL_SE_INFINISH */ TM_SSL_ST_RECV_CKE,     TM_SSL_SM_TRANS_NOOP }
    },

    {/* State == TM_SSL_ST_RECV_CERTVERI */
    /*  Event                     Next State       Action Procedure Index */
    {/* TM_SSL_SE_INCH1    */ TM_SSL_ST_RECV_CERTVERI,TM_SSL_SM_TRANS_NOOP },
    {/* TM_SSL_SE_INCH2    */ TM_SSL_ST_RECV_CERTVERI,TM_SSL_SM_TRANS_NOOP },
    {/* TM_SSL_SE_INCERT   */ TM_SSL_ST_RECV_CERTVERI,TM_SSL_SM_TRANS_NOOP },
    {/* TM_SSL_SE_INCKE    */ TM_SSL_ST_RECV_CERTVERI,TM_SSL_SM_TRANS_NOOP },
    {/* TM_SSL_SE_INVERIFY */ TM_SSL_ST_RECV_CERTVERI,TM_SSL_SM_TRANS_NOOP },
    {/* TM_SSL_SE_INCCS    */ TM_SSL_ST_RECV_CCS1,    TM_SSL_SM_TRANS_NOOP },
    {/* TM_SSL_SE_INFINISH */ TM_SSL_ST_RECV_CERTVERI,TM_SSL_SM_TRANS_NOOP }
    },

    {/* State == TM_SSL_ST_RECV_CCS1 */
    /*  Event                     Next State       Action Procedure Index */
    {/* TM_SSL_SE_INCH1    */ TM_SSL_ST_RECV_CCS1, TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCH2    */ TM_SSL_ST_RECV_CCS1, TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCERT   */ TM_SSL_ST_RECV_CCS1, TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCKE    */ TM_SSL_ST_RECV_CCS1, TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INVERIFY */ TM_SSL_ST_RECV_CCS1, TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCCS    */ TM_SSL_ST_RECV_CCS1, TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INFINISH */ TM_SSL_ST_OPENED,    TM_SSL_SM_FINISH        }
    },

    {/* State == TM_SSL_ST_RECV_CCS2 */
    /*  Event                     Next State       Action Procedure Index */
    {/* TM_SSL_SE_INCH1    */ TM_SSL_ST_RECV_CCS2, TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCH2    */ TM_SSL_ST_RECV_CCS2, TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCERT   */ TM_SSL_ST_RECV_CCS2, TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCKE    */ TM_SSL_ST_RECV_CCS2, TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INVERIFY */ TM_SSL_ST_RECV_CCS2, TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCCS    */ TM_SSL_ST_RECV_CCS2, TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INFINISH */ TM_SSL_ST_OPENED,    TM_SSL_SM_RESUMEFINISH  }
    },

    {/* State == TM_SSL_ST_OPENED */
    /*  Event                     Next State       Action Procedure Index */
    {/* TM_SSL_SE_INCH1    */ TM_SSL_ST_RECV_CH1,  TM_SSL_SM_REGULAR_HELLO },
    {/* TM_SSL_SE_INCH2    */ TM_SSL_ST_RECV_CH2,  TM_SSL_SM_RESUMING_HELLO},
    {/* TM_SSL_SE_INCERT   */ TM_SSL_ST_OPENED,    TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCKE    */ TM_SSL_ST_OPENED,    TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INVERIFY */ TM_SSL_ST_OPENED,    TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INCCS    */ TM_SSL_ST_OPENED,    TM_SSL_SM_TRANS_NOOP    },
    {/* TM_SSL_SE_INFINISH */ TM_SSL_ST_OPENED,    TM_SSL_SM_TRANS_NOOP    }
    }
};



/* DESCRIPTION: This function will be called before we construct Server
 * Hello. Basically, it determines how to reply server's second record
 * based on the cipher suite. For example, it determines if we need to
 * send certificate request, or server key exchange. And how many bytes
 * for the whole record. We need the whole record size because certificate
 * will be big and we have to send out partially. Without a good record
 * length for the whole record, client will drop our second reply message.
 * PARAMETERS: connStatePtr            the connection state
 */
static int tfSslServerPreReply(ttSslConnectStatePtr    connStatePtr,
                               ttVoidPtr               paramPtr)
{
#ifdef TM_SSL_USE_MUTUAL_AUTH
    ttPkiCertListPtr          prevCertListPtr;
#endif /* TM_SSL_USE_MUTUAL_AUTH */
    ttPkiCertListPtr          certListPtr;
    ttSslSessionPtr           sslSessionPtr;
    ttSslCipherSuiteConstPtr  cipherPtr;
#ifdef TM_SSL_USE_MUTUAL_AUTH
    ttSslClientAuthCalistPtr  calistPtr;
#endif /* TM_SSL_USE_MUTUAL_AUTH */
#ifdef TM_PUBKEY_USE_RSA
    ttGeneralRsaPtr           eRsaPtr;
#endif /* TM_PUBKEY_USE_RSA */
    int                       errorCode;
    int                       bufferNeeds;
    int                       certBytes;
    tt16Bit                   keyLength;
#ifdef TM_SSL_USE_MUTUAL_AUTH
    tt16Bit                   certReqLen;
#endif /* TM_SSL_USE_MUTUAL_AUTH */
    tt16Bit                   pairLength;
    tt8Bit                    digSignLength;

#ifdef TM_SSL_USE_MUTUAL_AUTH
    prevCertListPtr = (ttPkiCertListPtr)0;
#endif /* TM_SSL_USE_MUTUAL_AUTH */
    errorCode = TM_ENOERROR;
    bufferNeeds = 0;
/* must have a SSL record header*/
    TM_UNREF_IN_ARG(paramPtr);
    keyLength = 0;
    certBytes = 0;
#ifdef TM_SSL_USE_MUTUAL_AUTH
    certReqLen = 0;
#endif /* TM_SSL_USE_MUTUAL_AUTH */
    pairLength    = (tt16Bit)0;
    digSignLength = (tt8Bit)0;

    sslSessionPtr = connStatePtr->scsSessionPtr;

/*1. Server Hello length */
    connStatePtr->scsSessionIdLength = TM_SSL_SESSION_ID_SIZE;
    bufferNeeds = TM_SSL_SERVERHELLO_DEFAULT_SIZE;

    if(connStatePtr->scsSocIndex == TM_SOC_NO_INDEX)
    {
/* connStatePtr->scsSocIndex could be NULL by now if we set SSL on listening
 * socket, and when tfSslServerCreateState was called, the socIndex was
 * still TM_SOC_NO_INDEX
 */
        connStatePtr->scsSocIndex = connStatePtr->scsTcpVectPtr
                       ->tcpsSocketEntry.socIndex;
    }

/*2. possible certificate length */
    certListPtr = sslSessionPtr->ssCertListPtr;
    connStatePtr->scsTotalCertBytes = 0;
    while (certListPtr)
    {
#ifdef TM_SSL_USE_MUTUAL_AUTH
        prevCertListPtr = certListPtr;
#endif /* TM_SSL_USE_MUTUAL_AUTH */
        connStatePtr->scsTotalCertBytes = (tt16Bit)(connStatePtr->
                    scsTotalCertBytes + certListPtr->certLength + 3);
                    /*The '3' is the length field*/
        certListPtr = certListPtr->certSslNextPtr;
    }
    connStatePtr->scsTotalCertBytes = (tt16Bit)(
                       connStatePtr->scsTotalCertBytes + 3);
    bufferNeeds = (tt16Bit)(bufferNeeds + TM_SSL_HANDSHAKE_HEADER_SIZE +
                   connStatePtr->scsTotalCertBytes);

#ifdef TM_SSL_USE_MUTUAL_AUTH
/*3. Possible certificate request, at this moment, prevCertListPtr should
 *   be the direct neighbor certificate to CA
 */
    if ((sslSessionPtr->ssOption & TM_SSL_OPT_CLIENTAUTH) ||
/* in rehandshake case, even the session doesn't require clientauth, but
 * we want this connection to be mutual authed
 */
        (connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_SERVER_SENDCR))
    {
/* we need to send certificate request */
        connStatePtr->scsRunFlags |= TM_SSL_RUNFLAG_SERVER_SENDCR;
/* now we are going to add the ca lists */
        if (!sslSessionPtr->ssClientAuthCasPtr)
        {
            certListPtr = tfPkiCertificateSearch(
                              TM_PKI_CERTSEARCH_ISSUER,
                              prevCertListPtr->certDNPtr,
                              prevCertListPtr->certDNLength,
                              TM_PKI_CERT_OTHER_INDX, /* certificate list */
                              TM_8BIT_YES); /* need lock */
            if (!certListPtr)
            {
                errorCode = TM_EINVAL;
                goto sslServerPreReplyExit;
            }
            tfSslServerSessAddClientAuthCa(sslSessionPtr,
                                           certListPtr->certIdPtr);
        }
        calistPtr = sslSessionPtr->ssClientAuthCasPtr;
        while (calistPtr)
        {
/* the '2' is the field length used by opaque <> */
            certReqLen = (tt16Bit)(certReqLen +
                          calistPtr->scacCertPtr->certDNLength + 2);
            calistPtr = calistPtr->scacNextPtr;
        }
        certReqLen = (tt16Bit)(certReqLen + 2);

        certReqLen = (tt16Bit)(TM_SSL_HANDSHAKE_HEADER_SIZE +
/* we accepsts RSA_SIGN and DSA_SIGN only */
                       3 +
                       certReqLen);


        bufferNeeds = (tt16Bit)(bufferNeeds + certReqLen);
    }
#endif /* TM_SSL_USE_MUTUAL_AUTH */

/*4. possible server key exchange length, we need certListPtr points to the
 *   end entity
 */
    certListPtr = sslSessionPtr->ssCertListPtr;
    if (!certListPtr)
    {
        errorCode = TM_EINVAL;
        goto sslServerPreReplyExit;
    }
    else
    {
/* if we have export control we need check the pubkey infor in our
 * certificate to see if we need seperate KE message. If we don't have
 * export restrict, we don't need send server KE at all. We send Server KE,
 * only because we are using RSA_export, however the key is more than 512
 * bits
 */
        cipherPtr = connStatePtr->scsCipherPtr;
#ifdef TM_USE_SSL_VERSION_33
        if (connStatePtr->scsVersion == TM_SSL_VERSION_33)
        {
            if ((sslSessionPtr->ssOption & TM_SSL_OPT_CLIENTAUTH) ||
                (connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_SERVER_SENDCR))
            {
/* Hash and Signature pair number */
                pairLength    = TM_SSL_SUPPORT_PAIR_MAX +
                                2; /* length field of pair length*/
            }
/* Algorithm field is 2 byte  */
            digSignLength = (tt8Bit)2;
        }
        bufferNeeds = (tt16Bit)(bufferNeeds +
                        pairLength + /* hash and signature pair number */
                        digSignLength/* algorithm field length */);
#endif /* TM_USE_SSL_VERSION_33 */

#ifdef TM_PUBKEY_USE_RSA
        if (sslSessionPtr->ssSignAlgorithm == SADB_PUBKEY_RSA)
        {
            certBytes = ((ttGeneralRsaPtr)certListPtr->certKeyPtr
                       ->pkKeyPtr)->rsaN->asn1Length +
/* many implementations expect the signature to be an opaque structure */
                        2;
        }
#ifdef TM_PUBKEY_USE_DSA
        else
#endif /* TM_PUBKEY_USE_DSA */
#endif /* TM_PUBKEY_USE_RSA */
#ifdef TM_PUBKEY_USE_DSA
        if (sslSessionPtr->ssSignAlgorithm == SADB_PUBKEY_DSA)
        {
/* for DSA signature, we can only estimates, because we are going to ASN.1
 * encapsulate the r and s, and the actual length depends on the first bit
 * is set (negative asn.1 integar) or not, each r and s could be 160 bit
 * (20 octets) long, plus one byte (zer0), + asn1 object type and length
 */
            certBytes = 48 +
/* many implementations expect the signature to be an opaque structure */
                        2;
        }
#endif /* TM_PUBKEY_USE_DSA */

#ifdef TM_SSL_USE_EPHEMERAL_DH
        if (cipherPtr && ((cipherPtr->csKeyExchange == TM_SSL_KEXCH_DHE)
            ||(cipherPtr->csKeyExchange == TM_SSL_KEXCH_DHEEXPORT1024)))
        {
/* we want to use ephemeral DH key exchange. We definitely need Server KE */
            connStatePtr->scsRunFlags |= TM_SSL_RUNFLAG_SERVER_SENDEDH;
            keyLength = (tt16Bit)(TM_SSL_HANDSHAKE_HEADER_SIZE +
                        2 + /* length field of P */
                        2 + /* length of g (generator) */
                        1 + /* we use generator 0x02 or 0x05, one octect */
                        2 + /* length of public value*/ +
                        2 + /* length field of exp length*/
/* Signature of (MD5 hashing and/or SHA1 hashing)*/
                        certBytes);
#if (TM_SSL_DHE_PRIME_GROUP == TM_DHGROUP_1)
            keyLength = (tt16Bit)(keyLength + 2 * 96);
#elif (TM_SSL_DHE_PRIME_GROUP == TM_DHGROUP_2)
            keyLength = (tt16Bit)(keyLength + 2 * 128);
#elif (TM_SSL_DHE_PRIME_GROUP == TM_DHGROUP_5)
            keyLength = (tt16Bit)(keyLength + 2 * 192);
#elif (TM_SSL_DHE_PRIME_GROUP == TM_DHGROUP_EXPORT)
            keyLength = (tt16Bit)(keyLength + 2 * 64); /* 512 bits DH group */
#else
#error "Invalid definition of TM_SSL_DHE_PRIME_GROUP"
#endif /* TM_SSL_DHE_PRIME_GROUP == TM_DHGROUP_xxx */

        }
#ifdef TM_PUBKEY_USE_RSA
        else
#endif /* TM_PUBKEY_USE_RSA */
#endif /* TM_SSL_USE_EPHEMERAL_DH*/
#ifdef TM_PUBKEY_USE_RSA
        if (cipherPtr && ((cipherPtr->csKeyExchange == TM_SSL_KEXCH_RSAEXPORT)
                  ||(cipherPtr->csKeyExchange == TM_SSL_KEXCH_RSAEXPORT1024)))
        {
            int     allowedLength;

            if(cipherPtr->csKeyExchange == TM_SSL_KEXCH_RSAEXPORT)
            {
                allowedLength = 512;
            }
            else
            {
                allowedLength = 1024;
            }

            if (((certBytes - 2) << 3 ) > allowedLength)
            {
/* we don't have the ability to generate RSA, user is responsbile to add
 * ephemeral RSA keys if the key length in certificate is larger than 512 bits
 */
                if (sslSessionPtr->ssEphemRsaPtr)
                {
                    eRsaPtr = (ttGeneralRsaPtr)sslSessionPtr->
                             ssEphemRsaPtr->pkKeyPtr;
                }
                else
                {
                    eRsaPtr = (ttGeneralRsaPtr)0;
                }

                if (eRsaPtr)
                {
                    connStatePtr->scsRunFlags |= TM_SSL_RUNFLAG_SERVER_SENDERSA;
                    keyLength = (tt16Bit)(TM_SSL_HANDSHAKE_HEADER_SIZE +
                              2 + /* length field of mod value*/
                              eRsaPtr->rsaN->asn1Length +
                              /* mod value length, use 512 bits*/ +
                              2 + /* length field of exp length*/
                              eRsaPtr->rsaE->asn1Length + /* exp value */
                              certBytes); /* Signature of (MD5 hashing +
                                               SHA1 hashing)*/
                }
                else
                {
                    errorCode = TM_EINVAL;
                    goto sslServerPreReplyExit;
                }
            }
        }
#endif /* TM_PUBKEY_USE_RSA */

    }

/*5. KE + Server hello done */
    bufferNeeds = (tt16Bit)(bufferNeeds + keyLength +
                        TM_SSL_SERVERHELLODONE_DEFAULT_SIZE);

/*5. if HelloRequest involved, we need extra space for hashing and padding */
    if(connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_REHANDSHAKE)
    {
        bufferNeeds = (tt16Bit)(bufferNeeds +
                         TM_SSL_RECORD_OVERHEAD_MAX_SIZE);
    }

/*6. The record size doesn't include ssl record header (5) bytes, if DSA
 *   signature is used and key exchange is sent, this record size is not
 *   necessory the actual record length.
 *
 *   For Server, we can just save the bufferNeeds to scsNextWriteRecordSize
 *   Because the server will reply Handshake Record type only. For Client,
 *   we are going to send back handshake + changecipher + handshake, we need
 *   maintain scsNextWriteRecordSize and scsXmitBufferSize.
 */
    connStatePtr->scsNextWriteRecordSize = (tt16Bit)bufferNeeds;

sslServerPreReplyExit:
    return errorCode;
}

#ifdef TM_SSL_USE_MUTUAL_AUTH
/*
 * Description
 * This function tfSslServerAddClientAuthCa is valid only if both
 *TM_USE_SSL_SERVER and TM_SSL_USE_MUTUAL_AUTH are defined, and must be
 * called against an SSL server session. It adds possible CA's certificate
 * the server supports when it request client's certificate. If this
 * function is not called, no new CA certificate will be added, and when
 * mutual authentication applies, the client must send back its certificate
 * signed by the same CA as the server's ceritificate. If  this function is
 * called, (it may be called multiple times to add multiple client CAs),
 * client can send back certificate signed by any CA which the server
 * supports. In the certificate request message, the server will let the
 * client know what CA(s) it can support
 *
 * Parameters
 * sessionId                the session Id used
 * caIdPtr                  The certificate name of CA to be added
 *
 *
 * Returns
 * Value        Meaning
 * TM_ENOERROR success
 * TM_EINVAL    Invalid session numbers or invalid certificate name
 *            (we couldn't find that certificate according to the name).
 */

int tfSslServerAddClientAuthCa(int                 sessionId,
                               char TM_FAR *       caIdPtr)
{
    ttSslSessionPtr            sslSessionPtr;
    int                        errorCode;


    sslSessionPtr = (ttSslSessionPtr)tfSslGetSessionUsingIndex(sessionId);
    if (!sslSessionPtr)
    {
/* this session does not support client auth, why bother ? */
        errorCode = TM_EINVAL;
    }
    else
    {
        errorCode = tfSslServerSessAddClientAuthCa(sslSessionPtr, caIdPtr);
        tfSslSessionReturn(sslSessionPtr);
    }
    return errorCode;
}

static int tfSslServerSessAddClientAuthCa(ttSslSessionPtr sslSessionPtr,
                                          char   TM_FAR * caIdPtr)
{
    ttSslClientAuthCalistPtr   newCaPtr;
    ttPkiCertListPtr           certListPtr;
    int                        errorCode;

    errorCode = TM_EINVAL;
    if(sslSessionPtr->ssOption & TM_SSL_OPT_CLIENTAUTH)
    {
        tm_call_lock_wait(&(tm_context(tvPkiLockEntry)));
        certListPtr = tfPkiCertificateSearch(
                          TM_PKI_CERTSEARCH_ID,
                          caIdPtr,
                          (int) tm_strlen(caIdPtr),
                          TM_PKI_CERT_OTHER_INDX, /* certificate list */
                          TM_8BIT_NO); /* already locked */
        if (certListPtr)
        {
            newCaPtr = tm_get_raw_buffer(sizeof(ttSslClientAuthCalist));
            newCaPtr->scacCertPtr = certListPtr;
            certListPtr->certCheckOut++;
            newCaPtr->scacNextPtr = sslSessionPtr->ssClientAuthCasPtr;
            sslSessionPtr->ssClientAuthCasPtr = newCaPtr;
            errorCode = TM_ENOERROR;
        }
        tm_call_unlock(&(tm_context(tvPkiLockEntry)));
    }
    return errorCode;
}

/* Free the client auth CA list entry */
static void tfSslFreeClientAuthCaList(ttSslClientAuthCalistPtr newCaPtr)
{
    ttSslClientAuthCalistPtr   nextCaPtr;

    while (newCaPtr)
    {
        nextCaPtr = newCaPtr->scacNextPtr;
        newCaPtr->scacNextPtr = 0;
        newCaPtr->scacCertPtr->certCheckOut --;
        tm_free_raw_buffer(newCaPtr);
        newCaPtr = nextCaPtr;
    }
}
/*
 * Description
 * This function tfSslServerRemoveClientAuthCa is valid only if both
 * TM_USE_SSL_SERVER and TM_SSL_USE_MUTUAL_AUTH are defined, and must be
 * called against an SSL server session. It removes possible CA's
 * certificate the server supports when it request client's certificate.
 * It may be called multiple times to remove multiple client CAs.
 *
 * Parameters
 * sessionId                the session Id used
 * caIdPtr                  The certificate name of CA to be added
 *
 *
 * Returns
 * Value    Meaning
 * TM_ENOERROR  success
 * TM_EINVAL        Invalid session numbers or invalid certificate name
 *            (we couldn't find that certificate according to the name).
 */

int tfSslServerRemoveClientAuthCa(int                 sessionId,
                                  char TM_FAR *       caIdPtr)
{
    ttSslSessionPtr            sslSessionPtr;
    ttSslClientAuthCalistPtr   newCaPtr;
    ttSslClientAuthCalistPtr   prevCaPtr;
    int                        errorCode;
    int                        isEqual;

    errorCode = TM_ENOERROR;
    sslSessionPtr = (ttSslSessionPtr)tfSslGetSessionUsingIndex(sessionId);
    if (!sslSessionPtr)
    {
/* this session does not support client auth, why bother ? */
        errorCode = TM_EINVAL;
        goto sslRemoveClientCaExit;
    }

    if(sslSessionPtr->ssOption & TM_SSL_OPT_CLIENTAUTH)
    {
        prevCaPtr = (ttSslClientAuthCalistPtr)0;
        newCaPtr = sslSessionPtr->ssClientAuthCasPtr;
        while (newCaPtr)
        {
            isEqual = tm_memcmp(newCaPtr->scacCertPtr->certIdPtr, caIdPtr,
                tm_strlen(caIdPtr));
            if (isEqual == 0)
            {
                break;
            }
            prevCaPtr = newCaPtr;
            newCaPtr = newCaPtr->scacNextPtr;
        }

        if (!newCaPtr)
        {
/* we didn't find this CA, or this CA has not been added to the client
 * auth CA yet.
 */
            errorCode = TM_EINVAL;
        }
        else
        {
            if (prevCaPtr)
            {
                prevCaPtr->scacNextPtr = newCaPtr->scacNextPtr;
            }
            else
            {
                sslSessionPtr->ssClientAuthCasPtr = newCaPtr->scacNextPtr;
            }
            newCaPtr->scacNextPtr = 0;
            tfSslFreeClientAuthCaList(newCaPtr);
        }
    }
    tfSslSessionReturn(sslSessionPtr);
sslRemoveClientCaExit:
    return errorCode;
}
#endif /* TM_SSL_USE_MUTUAL_AUTH */

/*
 * state function, build server hello (will send when we build server
 * hello done
 */
static int tfSslSendServerHello(ttSslConnectStatePtr    connStatePtr,
                            ttVoidPtr               paramPtr)
{
    ttSslRecordHeaderPtr     sslHeaderPtr;
    ttPacketPtr              packetPtr;
    ttSslHandShakeHeaderPtr  handshakeHeaderPtr;
    ttServerHelloPtr         serverHelloPtr;
    tt8BitPtr                dataPtr;
    int                      dataLength;
    int                      bufferLength;
    int                      errorCode;
    tt16Bit                  tempLength;
    tt8Bit                   resume;
    tt8Bit                   ivSize;

    errorCode = TM_ENOERROR;
    TM_UNREF_IN_ARG(paramPtr);
    resume = TM_8BIT_NO;
    ivSize = (tt8Bit)0;

    if (connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_RESUMESESSION)
    {
        bufferLength = TM_SSL_SERVERHELLO_DEFAULT_SIZE +
                       TM_SSL_CHANGECIPHERSPEC_DEFAULT_SIZE;

        if (connStatePtr->scsVersion == TM_SSL_VERSION_30)
        {
            bufferLength += TM_SSL_RECORD_HEADER_SIZE +
                            TM_SSL_V30_FINISH_SIZE;
        }
        else if (connStatePtr->scsVersion == TM_SSL_VERSION_31)
        {
            bufferLength += TM_SSL_RECORD_HEADER_SIZE +
                            TM_SSL_V31_FINISH_SIZE;
        }
#ifdef TM_USE_SSL_VERSION_33
        else if (connStatePtr->scsVersion == TM_SSL_VERSION_33)
        {
            if((connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_ARCFOUR)
              &&(connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_NULL))
            {
                ivSize = connStatePtr->scsCryptoAlgPtr->ealgBlockSize;
            }
            bufferLength += TM_SSL_RECORD_HEADER_SIZE + ivSize +
                            TM_SSL_V33_FINISH_SIZE;
        }
#endif /* TM_USE_SSL_VERSION_33 */
        else
        {
            errorCode = TM_EINVAL;
            goto sslSendServerHelloExit;
        }
        resume = TM_8BIT_YES;
    }
    else
    {
        bufferLength = connStatePtr->scsNextWriteRecordSize;
    }
/* bufferLength is just record data size, not included the header(5) bytes*/
    packetPtr = tfGetSharedBuffer(0, (ttPktLen)bufferLength +
                TM_SSL_RECORD_HEADER_SIZE,
                0);
    if (! packetPtr)
    {
        errorCode = TM_ENOBUFS;
        goto sslSendServerHelloExit;
    }
    dataPtr = packetPtr->pktLinkDataPtr;

    dataLength = 0;
    connStatePtr->scsXmitPacketPtr = packetPtr;
/* We don't need this field anymore. It is not actuate.
 * The actual record size (Hello, KE, cert, done...) will be stored in
 * scsXmitDataLength
 */
    connStatePtr->scsNextWriteRecordSize = 0;
/* construct a server hello message */
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
    if (resume == TM_8BIT_YES)
    {
/* if it is a resumed session, the record header length include
 * server hello only, recalculate bufferLength, count the server hello
 only */
        bufferLength = TM_SSL_SERVERHELLO_DEFAULT_SIZE +
                       connStatePtr->scsSessionIdLength -
                       TM_SSL_SESSION_ID_SIZE_MAX;
/* for resumption case, we know the record size now, set it*/
        tempLength = (tt16Bit)bufferLength;
        sslHeaderPtr->srhLengthHigh = (tt8Bit)(tempLength >> 8);
        sslHeaderPtr->srhLengthLow = (tt8Bit)(tempLength & 0xff);
    }

    dataPtr += TM_SSL_RECORD_HEADER_SIZE;
    handshakeHeaderPtr = (ttSslHandShakeHeaderPtr) dataPtr;
    serverHelloPtr = (ttServerHelloPtr)((tt8BitPtr)handshakeHeaderPtr +
                   sizeof(ttSslHandShakeHeader));
    handshakeHeaderPtr->hsHandShakeType = TM_SSL_SERVER_HELLO;
    handshakeHeaderPtr->hsLengthPad = 0;

    serverHelloPtr->shVersionMajor = TM_SSL_VERSION_MAJOR;
/* TBD, the minor version should be the highest in this session */
#ifdef TM_USE_SSL_VERSION_33
    if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
        serverHelloPtr->shVersionMinor = TM_SSL_VERSION_MINOR3;
    }
    else
#endif /* TM_USE_SSL_VERSION_33 */
    {
        serverHelloPtr->shVersionMinor = (tt8Bit)
                  (connStatePtr->scsVersion >> 1);
    }
/* we don't support GMT time format*/
    tfGetRandomBytes(serverHelloPtr->shRandom,
                     tm_packed_byte_count(TM_SSL_RANDOM_SIZE));
/* We don't have real time, in case some browser may reject our random
 * value if our first four bytes is not a time value, we hack in
 * some data here
 */
    {
        serverHelloPtr->shRandom[0] = connStatePtr->scsClientRandom[0];
        serverHelloPtr->shRandom[1] = connStatePtr->scsClientRandom[1];
        serverHelloPtr->shRandom[2] = connStatePtr->scsClientRandom[2];
        serverHelloPtr->shRandom[3] = (tt8Bit)
                                      (connStatePtr->scsClientRandom[3] + 1);
    }

    tm_bcopy(serverHelloPtr->shRandom,
             connStatePtr->scsServerRandom,
             TM_SSL_RANDOM_SIZE);

    dataPtr += TM_SSL_HELLO_SESSIONID_OFFSET;
    serverHelloPtr->shSessionIdLength = connStatePtr->scsSessionIdLength;
    tm_bcopy(connStatePtr->scsSessionId,
             dataPtr,
             connStatePtr->scsSessionIdLength);

    dataLength = TM_SSL_HELLO_SESSIONID_OFFSET +
                 connStatePtr->scsSessionIdLength;
    dataPtr += connStatePtr->scsSessionIdLength;
    *(dataPtr++) = (tt8Bit)(connStatePtr->scsCipherPtr->csNumber >> 8);
    *(dataPtr++) = (tt8Bit)(connStatePtr->scsCipherPtr->csNumber & 0xff);
    *(dataPtr++) = connStatePtr->scsCompressMethod;

    dataLength = dataLength + 3; /*2cipher suite + 1 compress*/

/* The ServerHello length doesn't count TM_SSL_HANDSHAKE_HEADER_SIZE*/
    connStatePtr->scsNextWriteRecordSize = (tt16Bit)dataLength;
    tempLength = (tt16Bit)(dataLength - TM_SSL_HANDSHAKE_HEADER_SIZE);
    handshakeHeaderPtr->hsLengthHigh = (tt8Bit)(tempLength >> 8);
    handshakeHeaderPtr->hsLengthLow = (tt8Bit)(tempLength & 0xff);

    connStatePtr->scsXmitDataPtr = (tt8BitPtr)dataPtr;
    connStatePtr->scsXmitDataLength = (tt16Bit)
                    (dataLength + TM_SSL_RECORD_HEADER_SIZE);

/* don't send it right away, we are suppose to build other message into
 * this record.
 */
#ifdef TM_USE_SSL_VERSION_33
    if (connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
        tfSha256Update(connStatePtr->scsSha256CtxPtr,
                       (tt8BitPtr)handshakeHeaderPtr,
                       (tt16Bit)dataLength,
                       0);
    }
#endif /* TM_USE_SSL_VERSION_33 */
    tfSslUpdateMd5Sha1Ctx(connStatePtr,
                          (tt8BitPtr)handshakeHeaderPtr,
                          (tt16Bit)dataLength);
    if (resume == TM_8BIT_YES)
    {
        errorCode = tfSslGenerateCryptoMacKeys(connStatePtr);
    }

    if(connStatePtr->scsSocIndex == TM_SOC_NO_INDEX)
    {
/* connStatePtr->scsSocIndex could be NULL by now if we set SSL on listening
 * socket, and when tfSslServerCreateState was called, the socIndex was
 * still TM_SOC_NO_INDEX
 */
        connStatePtr->scsSocIndex = connStatePtr->scsTcpVectPtr
                       ->tcpsSocketEntry.socIndex;
    }


sslSendServerHelloExit:
    return errorCode;
}


/*
 * state function, build server KE (will send when we build server
 * hello done
 */
static int tfSslSendServerKE(ttSslConnectStatePtr    connStatePtr,
                                  ttVoidPtr          paramPtr)
{
    ttSslHandShakeHeaderPtr   handshakeHeaderPtr;
    ttCryptoRequest           request;
#ifdef TM_PUBKEY_USE_RSA
    ttCryptoRequestRsarequest rsareq;
#endif /* TM_PUBKEY_USE_RSA */
#ifdef TM_PUBKEY_USE_DSA
    ttCryptoRequestDsarequest dsareq;
#endif /* TM_PUBKEY_USE_DSA */
    ttMd5CtxPtr               md5CtxPtr;
    ttSha1CtxPtr              sha1CtxPtr;
#ifdef TM_USE_SSL_VERSION_33
    ttSha256CtxPtr            sha256CtxPtr;
#endif /* TM_USE_SSL_VERSION_33 */
    tt8BitPtr                 keBeginPtr;
    tt8BitPtr                 dataPtr;
    tt8BitPtr                 digestOutPtr;
    int                       errorCode;
    int                       sigLength;
    int                       keLength;
#if (defined(TM_PUBKEY_USE_RSA) || defined(TM_SSL_USE_EPHEMERAL_DH))
    int                       fieldLength;
#endif /* TM_PUBKEY_USE_RSA || TM_SSL_USE_EPHEMERAL_DH */
    tt16Bit                   tempLength;
    tt8Bit                    signAlgorithm;
#ifdef TM_PUBKEY_USE_RSA
    ttGeneralRsaPtr           eRsaPtr;
#endif /* TM_PUBKEY_USE_RSA */
#ifdef TM_SSL_USE_EPHEMERAL_DH
    char TM_FAR *             primePtr;
    ttCryptoRequestDhrequest  dhrequest;
    ttCryptoEnginePtr         cePtr;
    tt8Bit                    dhAlgorithm;
    tt8Bit                    dhGroupLen;
#endif /*TM_SSL_USE_EPHEMERAL_DH */

    errorCode = TM_ENOERROR;
    TM_UNREF_IN_ARG(paramPtr);
    keLength = 0;
    md5CtxPtr = (ttMd5CtxPtr)0;
    sha1CtxPtr = (ttSha1CtxPtr)0;
#ifdef TM_USE_SSL_VERSION_33
    sha256CtxPtr = (ttSha256CtxPtr)0;
#endif /* TM_USE_SSL_VERSION_33 */
    digestOutPtr = (tt8BitPtr)0;
    sigLength = 0;

    keBeginPtr = connStatePtr->scsXmitDataPtr +
               TM_SSL_HANDSHAKE_HEADER_SIZE;

    if ((connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_SERVER_SENDERSA) ||
       (connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_SERVER_SENDEDH))
    {
#ifdef TM_USE_SSL_VERSION_33
        if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
        {
            if(connStatePtr->scsSessionPtr->ssHashAlgorithm == TM_SSL_SUPPORT_HASH_SHA256)
            {
                sha256CtxPtr = tm_get_raw_buffer(sizeof(ttSha256Ctx));
                digestOutPtr = tm_get_raw_buffer(TM_SHA256_HASH_SIZE); 
                if ((!sha256CtxPtr)|| (!digestOutPtr))
                {
                    errorCode = TM_ENOBUFS;
                    goto sslSendServerKeExit;
                }
                tfSha256Init(sha256CtxPtr);

            }
            else if(connStatePtr->scsSessionPtr->ssHashAlgorithm == TM_SSL_SUPPORT_HASH_SHA1)
            {
                sha1CtxPtr = tm_get_raw_buffer(sizeof(ttSha1Ctx));
                digestOutPtr = tm_get_raw_buffer(TM_SHA1_HASH_SIZE);
                if ((!sha1CtxPtr)|| (!digestOutPtr))
                {
                    errorCode = TM_ENOBUFS;
                    goto sslSendServerKeExit;
                }
                tfSha1Init(sha1CtxPtr);
            }
        }
        else
#endif /* TM_USE_SSL_VERSION_33 */
        {
            md5CtxPtr = tm_get_raw_buffer(sizeof(ttMd5Ctx));
            sha1CtxPtr = tm_get_raw_buffer(sizeof(ttSha1Ctx));
            digestOutPtr = tm_get_raw_buffer(TM_MD5_HASH_SIZE +
                        TM_SHA1_HASH_SIZE );
            if ((!md5CtxPtr) || (!sha1CtxPtr)|| (!digestOutPtr))
            {
                errorCode = TM_ENOBUFS;
                goto sslSendServerKeExit;
            }
            tfMd5Init(md5CtxPtr);
            tfSha1Init(sha1CtxPtr);
        }
        dataPtr = keBeginPtr;
        tm_bzero(&request, sizeof(request));
        signAlgorithm = connStatePtr->scsCipherPtr
                ->csSignAlgorithm;
    }
    else
    {
/* no KE needed */
        goto sslSendServerKeExit;
    }

#ifdef TM_PUBKEY_USE_RSA

    if (connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_SERVER_SENDERSA)
    {
/* needs to send RSA modular parameters */
        if (!connStatePtr->scsSessionPtr->ssEphemRsaPtr ||
               signAlgorithm != SADB_PUBKEY_RSA)
        {
            errorCode = TM_EINVAL;
            goto sslSendServerKeExit;
        }

        eRsaPtr = (ttGeneralRsaPtr)connStatePtr->scsSessionPtr->
                             ssEphemRsaPtr->pkKeyPtr;
        fieldLength = eRsaPtr->rsaN->asn1Length;
        keLength = keLength + fieldLength + 2;
        tempLength = (tt16Bit)fieldLength;
        *dataPtr = (tt8Bit)(tempLength >> 8);
        *(dataPtr + 1) = (tt8Bit)(tempLength & 0xff);
        dataPtr += 2;
        tm_bcopy(eRsaPtr->rsaN->asn1Data,
                 dataPtr,
                 fieldLength);
        dataPtr += fieldLength;
        fieldLength = eRsaPtr->rsaE->asn1Length;
        keLength = keLength + fieldLength + 2;
        tempLength = (tt16Bit)fieldLength;
        *dataPtr = (tt8Bit)(tempLength >> 8);
        *(dataPtr + 1) = (tt8Bit)(tempLength & 0xff);
        dataPtr += 2;
        tm_bcopy(eRsaPtr->rsaE->asn1Data,
                 dataPtr,
                 fieldLength);
        dataPtr += fieldLength;
    }
#endif /* TM_PUBKEY_USE_RSA */

#ifdef TM_SSL_USE_EPHEMERAL_DH
    if (connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_SERVER_SENDEDH)
    {
/* needs to send DiffieHellman parameters */
        connStatePtr->scsDHInfoPtr =
                tm_get_raw_buffer(sizeof(ttDiffieHellman));
        if (!connStatePtr->scsDHInfoPtr)
        {
            errorCode = TM_ENOBUFS;
            goto sslSendServerKeExit;
        }
        tm_bzero(connStatePtr->scsDHInfoPtr,
                sizeof(ttDiffieHellman));
        connStatePtr->scsDHInfoPtr->dhGroup = (ttUser8Bit)TM_SSL_DHE_PRIME_GROUP;
        connStatePtr->scsDHInfoPtr->dhSecretLen =
                                       TM_SSL_DHSECRET_DEFAULT_SIZE;
#if (TM_SSL_DHE_PRIME_GROUP == TM_DHGROUP_1)
        dhGroupLen = 96;
        dhAlgorithm = SADB_PUBKEY_DIFFIEHELLMAN1;
        primePtr = TM_OAKLEY_PRIME_MODP768;
#elif (TM_SSL_DHE_PRIME_GROUP == TM_DHGROUP_2)
        /*768bits*/
        dhGroupLen = 128;
        dhAlgorithm = SADB_PUBKEY_DIFFIEHELLMAN2;
        primePtr = TM_OAKLEY_PRIME_MODP1024;
#elif (TM_SSL_DHE_PRIME_GROUP == TM_DHGROUP_5)
        /*1024bits*/
        dhGroupLen = 192;
        dhAlgorithm = SADB_PUBKEY_DIFFIEHELLMAN5;
        primePtr = TM_OAKLEY_PRIME_MODP1536;
#elif (TM_SSL_DHE_PRIME_GROUP == TM_DHGROUP_EXPORT)
        /* 512 bits */
        /* use Diffie-Hellman group 1 to find the engine */
        dhGroupLen = 64;
        dhAlgorithm = SADB_PUBKEY_DIFFIEHELLMAN1;
        primePtr = TM_OAKLEY_PRIME_MODP512;
#else
#error "Invalid definition of TM_SSL_DHE_PRIME_GROUP"
#endif /* TM_SSL_DHE_PRIME_GROUP == TM_DHGROUP_xxx */


        connStatePtr->scsDHInfoPtr->dhGenerator = TM_SSL_DHE_GENERATOR;
        connStatePtr->scsDHInfoPtr->dhGroupLen = dhGroupLen;
        connStatePtr->scsDHInfoPtr->dhPrimePtr =
            tm_get_raw_buffer(dhGroupLen);
        if (!connStatePtr->scsDHInfoPtr->dhPrimePtr)
        {
            errorCode = TM_ENOBUFS;
            goto sslSendServerKeExit;
        }

        errorCode = tfHexByteStr2HexByteArr(primePtr,
                                            dhGroupLen,
                                            (char TM_FAR *)connStatePtr
                                            ->scsDHInfoPtr->dhPrimePtr);


        dhrequest.crdDhinfoPtr = connStatePtr->scsDHInfoPtr;
/* We are the initiator */
        dhrequest.crdIsInitialtor = TM_ISAKMP_INITIATOR;
        request.crType = TM_CEREQUEST_PUBKEY_DHKEYPAIR;
        request.crParamUnion.crDhgroupParamPtr = &dhrequest;
        if (connStatePtr->scsPubkeyEnginePtr->ceSupportFlags
             & (TM_UL(1) << dhAlgorithm))
        {
/* The signature crypto engine can also do diffie-hellman */
            cePtr = connStatePtr->scsPubkeyEnginePtr;
        }
        else
        {
            cePtr = tfCryptoEngineGet(dhAlgorithm);
        }
        if (cePtr)
        {
            cePtr->ceSessionProcessFuncPtr((ttVoidPtr)&request);
        }
        else
        {
            errorCode = TM_EINVAL;
            goto sslSendServerKeExit;
        }
/* we already get the diffie-hellman secret and public */
/*Prime */
        fieldLength = ((int)dhGroupLen) & 0xFF;
        keLength = keLength + fieldLength + 2;
        tempLength = (tt16Bit)fieldLength;
        *dataPtr = (tt8Bit)(tempLength >> 8);
        *(dataPtr + 1) = (tt8Bit)(tempLength & 0xff);
        dataPtr += 2;
        tm_bcopy(connStatePtr->scsDHInfoPtr->dhPrimePtr,
                 dataPtr,
                 fieldLength);
        dataPtr += fieldLength;
/*Generator */
        fieldLength = 1; /* we use one single octect generator*/
        keLength = keLength + fieldLength + 2;
        tempLength = (tt16Bit)fieldLength;
        *dataPtr = (tt8Bit)(tempLength >> 8);
        *(dataPtr + 1) = (tt8Bit)(tempLength & 0xff);
        dataPtr += 2;
        *dataPtr = TM_SSL_DHE_GENERATOR;
        dataPtr += fieldLength;
/* my public value  */
        fieldLength = ((int)connStatePtr->scsDHInfoPtr->dhGxiLen) & 0xFFFF;
        keLength = keLength + fieldLength + 2;
        tempLength = (tt16Bit)fieldLength;
        *dataPtr = (tt8Bit)(tempLength >> 8);
        *(dataPtr + 1) = (tt8Bit)(tempLength & 0xff);
        dataPtr += 2;
        tm_bcopy(connStatePtr->scsDHInfoPtr->dhGxiPtr,
                 dataPtr,
                 fieldLength);
        dataPtr += fieldLength;
    }
#endif /*TM_SSL_USE_EPHEMERAL_DH */

#ifdef TM_USE_SSL_VERSION_33
    if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
        if(connStatePtr->scsSessionPtr->ssHashAlgorithm == TM_SSL_SUPPORT_HASH_SHA256)
        {
            tfSha256Update(sha256CtxPtr, connStatePtr->scsClientRandom,
                TM_SSL_RANDOM_SIZE, 0);
            tfSha256Update(sha256CtxPtr, connStatePtr->scsServerRandom,
                TM_SSL_RANDOM_SIZE, 0);
            tfSha256Update(sha256CtxPtr, keBeginPtr, (ttPktLen)keLength, 0);
            tfSha256Final(digestOutPtr, sha256CtxPtr);
        }
        else if(connStatePtr->scsSessionPtr->ssHashAlgorithm == TM_SSL_SUPPORT_HASH_SHA1)
        {
/* sign the sha1 hash */
/* SHA1(ClientHello.random + ServerHello.random + ServerParams);*/
            tfSha1Update(sha1CtxPtr, connStatePtr->scsClientRandom,
                TM_SSL_RANDOM_SIZE, 0);
            tfSha1Update(sha1CtxPtr, connStatePtr->scsServerRandom,
                TM_SSL_RANDOM_SIZE, 0);
            tfSha1Update(sha1CtxPtr, keBeginPtr, (ttPktLen)keLength, 0);
            tfSha1Final(digestOutPtr, sha1CtxPtr);
        }
    }
    else
#endif /* TM_USE_SSL_VERSION_33 */
    {
/* sign the md5 hash and/or sha1 hash */
/* MD5(ClientHello.random + ServerHello.random + ServerParams);*/
/* SHA(ClientHello.random + ServerHello.random + ServerParams); */
        if (signAlgorithm == SADB_PUBKEY_RSA)
        {
            tfMd5Update(md5CtxPtr, connStatePtr->scsClientRandom,
                TM_SSL_RANDOM_SIZE, 0);
            tfMd5Update(md5CtxPtr, connStatePtr->scsServerRandom,
                TM_SSL_RANDOM_SIZE, 0);
            tfMd5Update(md5CtxPtr, keBeginPtr, (ttPktLen)keLength, 0);
            tfMd5Final(digestOutPtr, md5CtxPtr);
        }

        tfSha1Update(sha1CtxPtr,connStatePtr->scsClientRandom,
            TM_SSL_RANDOM_SIZE, 0);
        tfSha1Update(sha1CtxPtr,connStatePtr->scsServerRandom,
            TM_SSL_RANDOM_SIZE, 0);
        tfSha1Update(sha1CtxPtr, keBeginPtr, (ttPktLen)keLength, 0);
        tfSha1Final(digestOutPtr + TM_MD5_HASH_SIZE, sha1CtxPtr);
    }
/* signature */
#ifdef TM_PUBKEY_USE_RSA
    if (signAlgorithm == SADB_PUBKEY_RSA)
    {
#ifdef TM_USE_SSL_VERSION_33
        if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
        {
            tfSslRsaPkcs1Encode(connStatePtr->scsSessionPtr->ssCertListPtr->certKeyPtr,
                                digestOutPtr,
                                dataPtr + 4,
                                &sigLength,
                                connStatePtr->scsSessionPtr->ssHashAlgorithm);
        }
        else
#endif /* TM_USE_SSL_VERSION_33 */
        {
            request.crType          = TM_CEREQUEST_PUBKEY_RSASIGN;
            request.crParamUnion.crRsaParamPtr   = &rsareq;
            rsareq.crrRsainfoPtr    = (ttGeneralRsaPtr)
                          (connStatePtr->scsSessionPtr->
                           ssCertListPtr->certKeyPtr->pkKeyPtr);
            rsareq.crrRsaDataLength = TM_MD5_HASH_SIZE + TM_SHA1_HASH_SIZE;
            rsareq.crrRsaTransPtr   = dataPtr + 2;
            rsareq.crrRsaDataPtr    = digestOutPtr;
            rsareq.crrRsaTransLenPtr = &sigLength;
            errorCode = connStatePtr->scsPubkeyEnginePtr->
                     ceSessionProcessFuncPtr(&request);
            if (errorCode != TM_ENOERROR)
            {
                goto sslSendServerKeExit;
            }
        }
    }
#endif /* TM_PUBKEY_USE_RSA */

#ifdef TM_PUBKEY_USE_DSA
    if (signAlgorithm == SADB_PUBKEY_DSA)
    {
        request.crType          = TM_CEREQUEST_PUBKEY_DSASIGN;
        request.crParamUnion.crDsaParamPtr   = &dsareq;
        dsareq.crdDsainfoPtr    = (ttGeneralDsaPtr)
                      (connStatePtr->scsSessionPtr->
                       ssCertListPtr->certKeyPtr->pkKeyPtr);
        dsareq.crdDsaDataLength = TM_SHA1_HASH_SIZE;
#ifdef TM_USE_SSL_VERSION_33
        if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
        {
            dsareq.crdDsaDataPtr    = digestOutPtr;
            dsareq.crdDsaTransPtr   = dataPtr + 4;
        }
        else
#endif /* TM_USE_SSL_VERSION_33 */
        {
            dsareq.crdDsaDataPtr    = digestOutPtr + TM_MD5_HASH_SIZE;
            dsareq.crdDsaTransPtr   = dataPtr + 2;
        }
        dsareq.crdDsaTransLenPtr = &sigLength;
        errorCode = connStatePtr->scsPubkeyEnginePtr->
                 ceSessionProcessFuncPtr(&request);
        if (errorCode != TM_ENOERROR)
        {
            goto sslSendServerKeExit;
        }
    }
#endif /* TM_PUBKEY_USE_DSA */

#ifdef TM_SSL30_DSSSIGN_NETSCAPE_FORMAT
/* Netscape ssl 30 implementation doesn't use Der encode of DSS signature,
 * instead, it uses naked r and s, hack code here
 */
    if (connStatePtr->scsVersion == TM_SSL_VERSION_30)
    {
        sigLength = 40;
        if (*(dataPtr + 5) == 0x14)
        {
            tm_bcopy(dataPtr + 6, dataPtr + 2, 20);
            if ((*(dataPtr + 27) == 0x14))
            {
                tm_bcopy(dataPtr + 28, dataPtr + 22, 20);
            }
            else
            {
                tm_bcopy(dataPtr + 29, dataPtr + 22, 20);
            }
        }
        else if (*(dataPtr + 5) == 0x15)
        {
            tm_bcopy(dataPtr + 7, dataPtr + 2, 20);
            if ((*(dataPtr + 28) == 0x14))
            {
                tm_bcopy(dataPtr + 29, dataPtr + 22, 20);
            }
            else
            {
                tm_bcopy(dataPtr + 30, dataPtr + 22, 20);
            }
        }
    }
#endif /* TM_SSL30_DSSSIGN_NETSCAPE_FORMAT */

#ifdef TM_USE_SSL_VERSION_33
    if (connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
/* HashAlgorithm*/
        *dataPtr++ = connStatePtr->scsSessionPtr->ssHashAlgorithm;
/* SignatureAlgorithm*/
        switch(signAlgorithm)
        {
            case SADB_PUBKEY_RSA:
                *dataPtr++ = TM_SSL_SUPPORT_SIG_RSA;
                break;
            case SADB_PUBKEY_DSA:
                *dataPtr++ = TM_SSL_SUPPORT_SIG_DSA;
                break;
            default :
                break;
        }
        keLength += (int)2;
    }
#endif /* TM_USE_SSL_VERSION_33 */

    tempLength = (tt16Bit)sigLength;
    *dataPtr = (tt8Bit)(tempLength >> 8);
    *(dataPtr + 1) = (tt8Bit)(tempLength & 0xff);
    keLength = keLength + sigLength + 2;/* signature length */

    handshakeHeaderPtr = (ttSslHandShakeHeaderPtr)
        (connStatePtr->scsXmitDataPtr);
    handshakeHeaderPtr->hsHandShakeType = TM_SSL_SERVER_KEY_EXCHANGE;
    handshakeHeaderPtr->hsLengthPad = 0;
    tempLength = (tt16Bit)keLength;
    handshakeHeaderPtr->hsLengthHigh = (tt8Bit)(tempLength >> 8);
    handshakeHeaderPtr->hsLengthLow = (tt8Bit)(tempLength & 0xff);

/* include Handshake header into keLength */
    keLength = keLength + TM_SSL_HANDSHAKE_HEADER_SIZE;
    connStatePtr->scsXmitDataPtr += keLength;
    connStatePtr->scsXmitDataLength = (tt16Bit)(connStatePtr->scsXmitDataLength
               + keLength);
#ifdef TM_USE_SSL_VERSION_33
    if (connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
        tfSha256Update(connStatePtr->scsSha256CtxPtr,
                       (tt8BitPtr)handshakeHeaderPtr,
                       (tt16Bit)keLength,
                       0);
    }
#endif /* TM_USE_SSL_VERSION_33 */
    tfSslUpdateMd5Sha1Ctx(connStatePtr,
                          (tt8BitPtr)handshakeHeaderPtr,
                          (tt16Bit)keLength);

sslSendServerKeExit:
    if (md5CtxPtr)
    {
        tm_free_raw_buffer((ttRawBufferPtr)md5CtxPtr);
    }
    if (sha1CtxPtr)
    {
        tm_free_raw_buffer((ttRawBufferPtr)sha1CtxPtr);
    }
#ifdef TM_USE_SSL_VERSION_33
    if (sha256CtxPtr)
    {
        tm_free_raw_buffer((ttRawBufferPtr)sha256CtxPtr);
    }
#endif /* TM_USE_SSL_VERSION_33 */
    if (digestOutPtr)
    {
        tm_free_raw_buffer(digestOutPtr);
    }
    return errorCode;
}

/*
 * state function, build and send server hello done
 */

static int tfSslSendHelloDone(ttSslConnectStatePtr    connStatePtr,
                                  ttVoidPtr               paramPtr)
{
    ttSslRecordHeaderPtr     sslHeaderPtr;
    ttSslHandShakeHeaderPtr  handshakeHeaderPtr;
    tt32Bit                  dataLength;
    tt16Bit                  temp16;
    int                      errorCode;

    errorCode = TM_ENOERROR;
    TM_UNREF_IN_ARG(paramPtr);
    handshakeHeaderPtr = (ttSslHandShakeHeaderPtr)
                   (connStatePtr->scsXmitDataPtr);
    handshakeHeaderPtr->hsHandShakeType = TM_SSL_SERVER_HELLO_DONE;
    handshakeHeaderPtr->hsLengthPad = 0;
    handshakeHeaderPtr->hsLengthHigh = 0;
    handshakeHeaderPtr->hsLengthLow = 0;

    connStatePtr->scsXmitDataLength = (tt16Bit)(connStatePtr->scsXmitDataLength
               + TM_SSL_HANDSHAKE_HEADER_SIZE);
    sslHeaderPtr = (ttSslRecordHeaderPtr)
                   (connStatePtr->scsXmitPacketPtr->pktLinkDataPtr);
    temp16 = (tt16Bit)
            (connStatePtr->scsXmitDataLength - TM_SSL_RECORD_HEADER_SIZE);
    sslHeaderPtr->srhLengthLow = (tt8Bit)(temp16 & 0xff);

    sslHeaderPtr->srhLengthHigh = (tt8Bit)(temp16 >> 8);
#ifdef TM_USE_SSL_VERSION_33
    if (connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
        tfSha256Update(connStatePtr->scsSha256CtxPtr,
                       (tt8BitPtr)handshakeHeaderPtr,
                       TM_SSL_HANDSHAKE_HEADER_SIZE,
                       0);
    }
#endif /* TM_USE_SSL_VERSION_33 */
    tfSslUpdateMd5Sha1Ctx(connStatePtr,
                          (tt8BitPtr)handshakeHeaderPtr,
                          TM_SSL_HANDSHAKE_HEADER_SIZE);
    dataLength = connStatePtr->scsXmitDataLength;
/* send out the packet */
    if(connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_REHANDSHAKE)
    {
/* if HelloRequest has been sent, we need encrypt this packet */
        temp16 = (tt16Bit)(dataLength - TM_SSL_RECORD_HEADER_SIZE);
        errorCode = tfSslHandshakeMsgTransform(connStatePtr,
                                               TM_CRYPTOREQUEST_ENCODE,
                                               sslHeaderPtr,
                                               connStatePtr->scsXmitPacketPtr,
                                               (tt8BitPtr)
                                               (handshakeHeaderPtr + 1),
                                               &temp16,
                                               TM_SSL_RECORD_HEADER_SIZE);
        dataLength = temp16 + TM_SSL_RECORD_HEADER_SIZE;
    }

    if(errorCode == TM_ENOERROR)
    {
        connStatePtr->scsXmitPacketPtr->pktLinkDataLength =
                   (ttPktLen)dataLength;
        connStatePtr->scsXmitPacketPtr->pktChainDataLength =
                   (ttPktLen)dataLength;
        errorCode = tfSslSend(&connStatePtr->scsTcpVectPtr->tcpsSocketEntry,
                      connStatePtr,
                      &connStatePtr->scsXmitPacketPtr,
                      (ttPktLen TM_FAR *)&dataLength,
                      0);
        connStatePtr->scsXmitDataPtr = (tt8BitPtr)0;
        connStatePtr->scsXmitPacketPtr = (ttPacketPtr)0;
        connStatePtr->scsXmitDataLength = 0;
    }

    return errorCode;
}
/*
 * state function, build cert request (will send when we build server
 * hello done
 */

static int tfSslSendCertRequest(ttSslConnectStatePtr    connStatePtr,
                                ttVoidPtr               paramPtr)
{

#ifdef TM_SSL_USE_MUTUAL_AUTH
    tt8BitPtr                dataPtr;
    tt8BitPtr                caListLengthPtr;
    ttSslClientAuthCalistPtr caPtr;
    ttSslHandShakeHeaderPtr  handshakeHeaderPtr;
    int                      errorCode;
    tt16Bit                  caListLength;
    tt8Bit                   crtTypeLength;
    tt16Bit                  thisCaLength;
    tt8BitPtr                dataLengthPtr;
#ifdef TM_USE_SSL_VERSION_33
    tt16Bit                  suppPairLength;
#endif /* TM_USE_SSL_VERSION_33 */

    TM_UNREF_IN_ARG(paramPtr);
    errorCode = TM_ENOERROR;
#ifdef TM_USE_SSL_VERSION_33
    suppPairLength = 0;
#endif /* TM_USE_SSL_VERSION_33 */

    if (connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_SERVER_SENDCR)
    {
        dataPtr = connStatePtr->scsXmitDataPtr;
        handshakeHeaderPtr = (ttSslHandShakeHeaderPtr)(dataPtr);
        handshakeHeaderPtr->hsHandShakeType = TM_SSL_CERTIFICATE_REQUEST;
        handshakeHeaderPtr->hsLengthPad = 0;
        dataPtr += TM_SSL_HANDSHAKE_HEADER_SIZE;
/* we support certificate type of RSA_SIGN and DSS_SIGN */
        dataLengthPtr = dataPtr++;
#ifdef TM_PUBKEY_USE_RSA
        *dataPtr ++ = TM_CERTTYPE_RSA_SIGN;
#endif /* TM_PUBKEY_USE_RSA */
#ifdef TM_PUBKEY_USE_DSA
        *dataPtr ++ = TM_CERTTYPE_DSS_SIGN;
#endif /* TM_PUBKEY_USE_DSA */
        crtTypeLength = (tt8Bit)(dataPtr - dataLengthPtr - 1);

#ifdef TM_USE_SSL_VERSION_33
        *dataLengthPtr = crtTypeLength;
        if (connStatePtr->scsVersion == TM_SSL_VERSION_33)
        {
/* Hash and Signature Algorithms pair */
            dataLengthPtr = dataPtr;
            dataPtr += 2;
#ifdef TM_PUBKEY_USE_RSA
            *dataPtr ++ = TM_SSL_SUPPORT_HASH_SHA256;
            *dataPtr ++ = TM_SSL_SUPPORT_SIG_RSA;
            *dataPtr ++ = TM_SSL_SUPPORT_HASH_SHA1;
            *dataPtr ++ = TM_SSL_SUPPORT_SIG_RSA;
#endif /* TM_PUBKEY_USE_RSA */
#ifdef TM_PUBKEY_USE_DSA
            *dataPtr ++ = TM_SSL_SUPPORT_HASH_SHA1;
            *dataPtr ++ = TM_SSL_SUPPORT_SIG_DSA;
#endif /* TM_PUBKEY_USE_DSA */
#ifdef TM_PUBKEY_USE_RSA
            *dataPtr ++ = TM_SSL_SUPPORT_HASH_MD5;
            *dataPtr ++ = TM_SSL_SUPPORT_SIG_RSA;
#endif /* TM_PUBKEY_USE_RSA */
            /* Need to add pair */
            suppPairLength = (tt16Bit)(dataPtr - dataLengthPtr - 2);
            *dataLengthPtr ++ =(tt8Bit)(suppPairLength >> 8);
            *dataLengthPtr =(tt8Bit)(suppPairLength & 0xff);
        }
#endif /* TM_USE_SSL_VERSION_33 */

/* here is the CA's distinguished name we expect ! */
        caListLength = 0;
        caListLengthPtr = dataPtr;
        dataPtr += 2;
        caPtr = connStatePtr->scsSessionPtr->ssClientAuthCasPtr;
        while (caPtr)
        {
            thisCaLength = caPtr->scacCertPtr->certDNLength;
            *dataPtr ++ =(tt8Bit)(thisCaLength >> 8);
            *dataPtr ++ =(tt8Bit)(thisCaLength & 0xff);
            tm_bcopy(caPtr->scacCertPtr->certDNPtr,
                      dataPtr,
                      thisCaLength);
            caListLength = (tt16Bit)(caListLength + thisCaLength + 2);
            dataPtr += thisCaLength;
            caPtr = caPtr->scacNextPtr;
        }
        *caListLengthPtr ++ =(tt8Bit)(caListLength >> 8);
        *caListLengthPtr ++ =(tt8Bit)(caListLength & 0xff);
        caListLength = (tt16Bit)(caListLength + crtTypeLength + 3);
#ifdef TM_USE_SSL_VERSION_33
        if (connStatePtr->scsVersion == TM_SSL_VERSION_33)
        {
            caListLength = caListLength +
                           suppPairLength +
                           2;
        }
#endif /* TM_USE_SSL_VERSION_33 */
        handshakeHeaderPtr->hsLengthHigh = (tt8Bit)(caListLength >> 8);
        handshakeHeaderPtr->hsLengthLow = (tt8Bit)(caListLength & 0xff);


        connStatePtr->scsXmitDataPtr = dataPtr;
        caListLength = (tt16Bit)(TM_SSL_HANDSHAKE_HEADER_SIZE + caListLength);
        connStatePtr->scsXmitDataLength =  (tt16Bit)
                          (connStatePtr->scsXmitDataLength  + caListLength);
#ifdef TM_USE_SSL_VERSION_33
        if (connStatePtr->scsVersion == TM_SSL_VERSION_33)
        {
            tfSha256Update(connStatePtr->scsSha256CtxPtr,
                           (tt8BitPtr)handshakeHeaderPtr,
                           caListLength,
                           0);
        }
#endif /* TM_USE_SSL_VERSION_33 */
        tfSslUpdateMd5Sha1Ctx(connStatePtr,
                              (tt8BitPtr)handshakeHeaderPtr,
                              caListLength);
    }
    return errorCode;
#else /* TM_SSL_USE_MUTUAL_AUTH */
    TM_UNREF_IN_ARG(connStatePtr);
    TM_UNREF_IN_ARG(paramPtr);
    return TM_ENOERROR;

#endif /*  TM_SSL_USE_MUTUAL_AUTH */
}

/*
 * state function, parse client change cipher message
 */
int tfSslParseClientCCS(ttSslConnectStatePtr   connStatePtr,
                        ttPacketPtr            packetPtr)
{
    tt8BitPtr             dataPtr;
    int                   errorCode;
    tt8Bit                event;


    errorCode = TM_ENOERROR;
    event = TM_SSL_SE_NOEVENT;
    dataPtr = packetPtr->pktLinkDataPtr;

    if (*dataPtr == TM_SSL_CHANGECIPHER_VALID &&
         connStatePtr->scsNextReadRecordSize == 1)
    {
        event = TM_SSL_SE_INCCS;
        connStatePtr->scsRunFlags |= TM_SSL_RUNFLAG_CCS_RECVD;
    }

    if ((connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_RESUMESESSION) == 0)
    {
        errorCode = tfSslGenerateCryptoMacKeys(connStatePtr);
        if(errorCode != TM_ENOERROR)
        {
            goto parseClientCCSExit;
        }
    }

    errorCode = tfSslClientWriteKeySchedule(connStatePtr,
            &connStatePtr->scsDecodeMacKey,
            &connStatePtr->scsDecodeCryptoKey,
            &connStatePtr->scsDecodeIvPtr);

    if(errorCode != TM_ENOERROR)
    {
        goto parseClientCCSExit;
    }
    connStatePtr->scsInSeqNumber = 0;
    connStatePtr->scsInSeqNumberHigh = 0;

    if (event != TM_SSL_SE_NOEVENT)
    {
        errorCode = tfSslServerStateMachine (connStatePtr,
            0,
            event);
    }

parseClientCCSExit:
    return errorCode;
}


/*
 * state function, build server change cipher message (will send when we build
 * finished message)
 */

static int tfSslSendServerCCS(ttSslConnectStatePtr    connStatePtr,
                              ttVoidPtr               paramPtr)
{

    tt8BitPtr                dataPtr;
    ttSslRecordHeaderPtr     sslHeaderPtr;
    ttPacketPtr              packetPtr;
    int                      errorCode;
    tt16Bit                  recordSize;


    TM_UNREF_IN_ARG(paramPtr);
    errorCode = TM_ENOERROR;

    if (!connStatePtr->scsXmitPacketPtr)
    {
/* we need allocate buffer, if the buffer is not already allocated. (For
 * resumption case, when we reach here, the buffer has already been allocated
 * in server hello construction.
 */
/*We need figure out how much buffer we need to hold the CCS + FINISH
 */
#ifdef TM_USE_SSL_VERSION_30
        if (connStatePtr->scsVersion == TM_SSL_VERSION_30)
        {
/* SSL v3.0, the finished message maximum length is :
 * record header (5) + handshake(4) + md5_hash(16) + sha_hash(20) +
 * sha_hash(20)+ maximum padding (16 for AES block cipher)
 */
            connStatePtr->scsXmitBufferSize = (tt16Bit)(
                            TM_SSL_RECORD_HEADER_SIZE
                          + TM_SSL_CHANGE_CIPHER_SIZE
                          + TM_SSL_RECORD_HEADER_SIZE
                          + TM_SSL_V30_FINISH_SIZE);
        }
#if (defined(TM_USE_SSL_VERSION_31) || defined(TM_USE_SSL_VERSION_33))
        else
#endif /* TM_USE_SSL_VERSION_31 || TM_USE_SSL_VERSION_33 */
#endif /* TM_USE_SSL_VERSION_30 */

#ifdef TM_USE_SSL_VERSION_31
        if (connStatePtr->scsVersion == TM_SSL_VERSION_31)
        {
/* TLS v1.0, the finished message maximum length is :
 * record header (5) + handshake(4) + verified data(12)
 * sha_hash(20)+ maximum padding (16 for AES block cipher)
 */
            connStatePtr->scsXmitBufferSize = (tt16Bit)(
                            TM_SSL_RECORD_HEADER_SIZE
                          + TM_SSL_CHANGE_CIPHER_SIZE
                          + TM_SSL_RECORD_HEADER_SIZE
                          + TM_SSL_V31_FINISH_SIZE);
        }
#ifdef TM_USE_SSL_VERSION_33
        else
#endif /* TM_USE_SSL_VERSION_33 */
#endif /* TM_USE_SSL_VERSION_31 */
#ifdef TM_USE_SSL_VERSION_33
        if (connStatePtr->scsVersion == TM_SSL_VERSION_33)
        {
/* TLS v1.2, the finished message maximum length is :
 * record header (5) + handshake(4) + verified data(12)
 * sha_hash(32)+ maximum padding (16 for AES block cipher)
 */
            connStatePtr->scsXmitBufferSize = (tt16Bit)(
                            TM_SSL_RECORD_HEADER_SIZE
                          + TM_SSL_CHANGE_CIPHER_SIZE
                          + TM_SSL_RECORD_HEADER_SIZE
                          + TM_SSL_V33_FINISH_SIZE);
        }
#endif /* TM_USE_SSL_VERSION_33 */
/* if we are running inside an SSL channel, CCS has to be
 * protected too.
 */
        if(connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_REHANDSHAKE)
        {
            connStatePtr->scsXmitBufferSize = (tt16Bit)
                 ( connStatePtr->scsXmitBufferSize +
                   TM_SSL_RECORD_OVERHEAD_MAX_SIZE);
        }
#ifdef TM_USE_SSL_VERSION_33
        if((connStatePtr->scsVersion == TM_SSL_VERSION_33)
         &&((connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_ARCFOUR)
          &&(connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_NULL)))
        {
            connStatePtr->scsXmitBufferSize = connStatePtr->scsXmitBufferSize +
                (tt16Bit)connStatePtr->scsCryptoAlgPtr->ealgBlockSize;
        }
#endif /* TM_USE_SSL_VERSION_33 */
        packetPtr = tfGetSharedBuffer(0, connStatePtr->scsXmitBufferSize,0);
        if (!(packetPtr))
        {
            errorCode = TM_ENOBUFS;
            goto sslSendServerCcsExit;
        }
        dataPtr = packetPtr->pktLinkDataPtr;
        connStatePtr->scsXmitPacketPtr = packetPtr;
    }
    else
    {
        dataPtr = connStatePtr->scsXmitDataPtr;
    }


    sslHeaderPtr = (ttSslRecordHeaderPtr)dataPtr;
    sslHeaderPtr->srhType = TM_SSL_TYPE_CHANGECIPHERSPEC;
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
/* for change cipher spec message, the length is 1 */
    sslHeaderPtr->srhLengthHigh = 0;
    sslHeaderPtr->srhLengthLow = (tt8Bit)TM_SSL_CHANGE_CIPHER_SIZE;

    dataPtr += TM_SSL_RECORD_HEADER_SIZE;
    *dataPtr ++ = TM_SSL_CHANGECIPHER_VALID;

    recordSize = (tt16Bit)(TM_SSL_CHANGE_CIPHER_SIZE);
/* if HelloRequest is involved, we should encrypt this message
 * using the old keys
 */
    if(connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_REHANDSHAKE)
    {
/* need encryption if HelloRequest is exchanged */
        errorCode = tfSslHandshakeMsgTransform(connStatePtr,
                                               TM_CRYPTOREQUEST_ENCODE,
                                               sslHeaderPtr,
                                               connStatePtr->scsXmitPacketPtr,
                                               dataPtr,
                                               &recordSize,
                                               TM_SSL_RECORD_HEADER_SIZE);
        if(errorCode != TM_ENOERROR)
        {
            goto sslSendServerCcsExit;
        }
        dataPtr += ( recordSize - TM_SSL_CHANGE_CIPHER_SIZE);
    }
    connStatePtr->scsXmitDataLength = (tt16Bit)(connStatePtr->scsXmitDataLength
                       + recordSize
                       + TM_SSL_RECORD_HEADER_SIZE);
    connStatePtr->scsXmitDataPtr = dataPtr;

/* Time to build Server Write keys */
    errorCode = tfSslServerWriteKeySchedule(connStatePtr,
                    &connStatePtr->scsEncodeMacKey,
                    &connStatePtr->scsEncodeCryptoKey,
                    &connStatePtr->scsEncodeIvPtr);
    connStatePtr->scsOutSeqNumber = 0;
    connStatePtr->scsOutSeqNumberHigh = 0;
/* we should be able to free the old key blocks if any */
    if(connStatePtr->scsKeyBlockBackupPtr)
    {
        tm_free_raw_buffer(connStatePtr->scsKeyBlockBackupPtr);
        connStatePtr->scsKeyBlockBackupPtr = (tt8BitPtr)0;
    }
    if(connStatePtr->scsExportKeyBackupPtr)
    {
        tm_free_raw_buffer(connStatePtr->scsExportKeyBackupPtr);
        connStatePtr->scsExportKeyBackupPtr = (tt8BitPtr)0;
    }

sslSendServerCcsExit:
    return errorCode;
}

/*
 * state function, parse client key exchange message
 */
static int tfSslParseClientKeyExchange(ttSslConnectStatePtr   connStatePtr,
                                       tt8BitPtr              dataPtr,
                                       tt16Bit                msgSize)
{
    ttCryptoRequest           request;
#ifdef TM_PUBKEY_USE_RSA
    ttCryptoRequestRsarequest rsareq;
#endif /* TM_PUBKEY_USE_RSA */
#ifdef TM_SSL_USE_EPHEMERAL_DH
    ttCryptoRequestDhrequest  dhrequest;
    ttDiffieHellmanPtr        dhInfoPtr;
#endif /*TM_SSL_USE_EPHEMERAL_DH */
    tt8BitPtr                 tempPremasterPtr;
    int                       errorCode;
#if (defined(TM_PUBKEY_USE_RSA) || defined(TM_SSL_USE_EPHEMERAL_DH))
    int                       fieldLength;
#endif /* TM_PUBKEY_USE_RSA || TM_SSL_USE_EPHEMERAL_DH */
#ifdef TM_PUBKEY_USE_RSA
    int                       signLength;
#endif /* TM_PUBKEY_USE_RSA */
#ifdef TM_SSL_USE_EPHEMERAL_DH
    tt16Bit                   tempLength;
#endif /*TM_SSL_USE_EPHEMERAL_DH */
#ifdef TM_PUBKEY_USE_RSA
    tt8Bit                    sslVersion;
#endif /* TM_PUBKEY_USE_RSA */

#if (!defined(TM_SSL_USE_EPHEMERAL_DH) && !defined(TM_PUBKEY_USE_RSA))
    TM_UNREF_IN_ARG(dataPtr)
    TM_UNREF_IN_ARG(msgSize)
#endif /* !TM_SSL_USE_EPHEMERAL_DH && !TM_PUBKEY_USE_RSA */

    errorCode = TM_ENOERROR;
    tempPremasterPtr = (tt8BitPtr)0;
    tm_bzero(&request, sizeof(request));

#ifdef TM_SSL_USE_EPHEMERAL_DH
    if ((connStatePtr->scsCipherPtr->csKeyExchange == TM_SSL_KEXCH_DHE)
      ||(connStatePtr->scsCipherPtr->csKeyExchange ==
          TM_SSL_KEXCH_DHEEXPORT1024))
    {
/* Client should send us back Client public value. We should generate
 * the premaster key using the Diffie-hellman exchange method.
 */
        dhInfoPtr = connStatePtr->scsDHInfoPtr;
/* parsing the client's public value */
        tempLength = (tt16Bit)((*dataPtr << 8) + *(dataPtr + 1)) ;
        fieldLength = ((int)tempLength) & 0xFFFF;
/* check this public value should be our prime length or less (if
 * pre-zero padding is not used
 */

        if ( ((ttUser16Bit)fieldLength > dhInfoPtr->dhGroupLen) ||
             ((tt16Bit)fieldLength > (msgSize - 2)) )
        {
            errorCode = TM_EINVAL;
            goto sslParseClientKeExit;
        }
        dataPtr += 2;
        dhInfoPtr->dhGxrPtr = tm_get_raw_buffer((ttPktLen)fieldLength);
        if (!dhInfoPtr->dhGxrPtr)
        {
            errorCode = TM_ENOBUFS;
            goto sslParseClientKeExit;
        }
        tm_bcopy(dataPtr, dhInfoPtr->dhGxrPtr,fieldLength);
        dhInfoPtr->dhGxrLen = (tt16Bit)fieldLength;
/* Generate the shared secret */
        dhrequest.crdDhinfoPtr = dhInfoPtr;
        dhrequest.crdIsInitialtor = TM_ISAKMP_INITIATOR;
        request.crParamUnion.crDhgroupParamPtr = &dhrequest;
        request.crType = TM_CEREQUEST_PUBKEY_DHSHAREDKEY;
        errorCode = connStatePtr->scsPubkeyEnginePtr->ceSessionProcessFuncPtr
                   ((ttVoidPtr)&request);
        if (errorCode != TM_ENOERROR)
        {
            goto sslParseClientKeExit;
        }
        connStatePtr->scsPreMasterPtr = dhrequest.crdDhinfoPtr->dhGxyPtr;
        connStatePtr->scsPreMasterLen = dhrequest.crdDhinfoPtr->dhGxyLen;
    }
#endif /*TM_SSL_USE_EPHEMERAL_DH */

#ifdef TM_PUBKEY_USE_RSA

    if ((connStatePtr->scsCipherPtr->csKeyExchange == TM_SSL_KEXCH_RSA)
        || (connStatePtr->scsCipherPtr->csKeyExchange ==
            TM_SSL_KEXCH_RSAEXPORT)
        || (connStatePtr->scsCipherPtr->csKeyExchange ==
            TM_SSL_KEXCH_RSAEXPORT1024))

    {
/* Client should send us random premaster key and encrypted using
 * the ephemeral RSA key if ephemeral RSA was sent. Otherwise, use
 * the normal RSA.
 */
        if (connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_SERVER_SENDERSA)
        {
            rsareq.crrRsainfoPtr = connStatePtr->scsSessionPtr->
                       ssEphemRsaPtr->pkKeyPtr;
        }
        else
        {
            rsareq.crrRsainfoPtr    = (ttGeneralRsaPtr)connStatePtr->
                 scsSessionPtr->ssCertListPtr->certKeyPtr->pkKeyPtr;
        }
        signLength = rsareq.crrRsainfoPtr->rsaN->asn1Length;
/* Note that for IE6.0.2800, when SSL 3.0 is used, the client ke handshake
 * message looks like "10 00 00 80" for 128 bytes RSA encryption. However
 * for TLS 1.0, the same IE will transmit client ke like "10 00 00 82 00 80
 * ..." We need to handle both case.
 */
        if (msgSize != (tt16Bit)signLength )
        {
            if (msgSize == (tt16Bit)(signLength + 2))
            {
                dataPtr += 2;
            }
            else
            {
/* all data followed should be the RSA encryped data.*/
                errorCode = TM_EINVAL;
                goto sslParseClientKeExit;
            }
        }
/* we need to put the decrypted results here. And after decryption,
 * if the length is ok, we copy to premaster.
 */
        tempPremasterPtr = tm_get_raw_buffer((ttPktLen)signLength);
        if (!tempPremasterPtr)
        {
            errorCode = TM_ENOBUFS;
            goto sslParseClientKeExit;
        }
        request.crType = TM_CEREQUEST_PUBKEY_RSADECRYPT;
        request.crParamUnion.crRsaParamPtr   = &rsareq;
        rsareq.crrRsaDataLength = signLength;
        rsareq.crrRsaDataPtr    = dataPtr;
        rsareq.crrRsaTransPtr   = tempPremasterPtr;
        rsareq.crrRsaTransLenPtr = &fieldLength;
        errorCode = connStatePtr->scsPubkeyEnginePtr->
                 ceSessionProcessFuncPtr(&request);
        sslVersion = *(tempPremasterPtr + 1);
        if ( (errorCode != TM_ENOERROR) ||
             (fieldLength != TM_SSL_PRE_MASTER_SIZE) ||
             (*tempPremasterPtr != TM_SSL_VERSION_MAJOR) ||
             (sslVersion > TM_SSL_VERSION_MINOR3) )
        {
            errorCode = TM_EINVAL;
            goto sslParseClientKeExit;
        }
        if ((connStatePtr->scsClientVersion >= TM_SSL_VERSION_32)
           && ((1 << sslVersion) != connStatePtr->scsClientVersion))
        {
            (void)tfSslBuildSendAlert(connStatePtr,
                                      TM_SSL_ALTLEV_FATAL,
                                      TM_SSL_ALT_BADRECMAC);
            errorCode = TM_EOPNOTSUPP;
            goto sslParseClientKeExit;

        }

        connStatePtr->scsPreMasterLen = (tt16Bit)fieldLength;
        connStatePtr->scsPreMasterPtr = tm_get_raw_buffer(
                                                    (ttPktLen)fieldLength);
        if (!connStatePtr->scsPreMasterPtr)
        {
            errorCode = TM_ENOBUFS;
            goto sslParseClientKeExit;
        }
        tm_bcopy(tempPremasterPtr,
                 connStatePtr->scsPreMasterPtr,
                 fieldLength);
    }
#endif /* TM_PUBKEY_USE_RSA */

#ifdef TM_SSL_DEBUG
    tm_debug_log0("Server premaster key = ");
    tfCryptoDebugPrintf((ttCharPtrPtr)0,
                        connStatePtr->scsPreMasterPtr,
                        connStatePtr->scsPreMasterLen,
                        (ttIntPtr)0);
#endif /* TM_SSL_DEBUG */

/* Now we are able to generate master key */
    errorCode = tfSslGenerateMasterSecret(connStatePtr);
    if (errorCode != TM_ENOERROR)
    {
        goto sslParseClientKeExit;
    }

sslParseClientKeExit:
    if (tempPremasterPtr)
    {
        tm_free_raw_buffer(tempPremasterPtr);
    }
    return errorCode;
}


#ifdef TM_SSL_USE_MUTUAL_AUTH
/*
 * state function, parse certificate verify message
 */
static int tfSslParseCertificateVerify(ttSslConnectStatePtr   connStatePtr,
                                       tt8BitPtr              dataPtr,
                                       tt16Bit                msgSize)
{

    tt32Bit              certVeriLen;
    int                  errorCode;

    certVeriLen = msgSize - 2;
    errorCode = tfSslBuildCertVerify(connStatePtr,
                                     dataPtr,
                                     &certVeriLen);
    return errorCode;
}
#endif /* TM_SSL_USE_MUTUAL_AUTH */

/*
 * Server to find if we have a cache given session id. This is used when
 * client tries to resume a connection. If we do find a cache, we return
 * it in cachePtrPtr. If no cache found, we deny the resume request,
 * instead, we offer a new session id. The client should go ahead as
 * if no resuming request has been offered
 */
static int tfSslServerFindCache(ttSslSessionPtr               sslSessionPtr,
                                ttSslCacheElementPtr TM_FAR * cachePtrPtr,
                                tt8BitPtr                     sessionIdPtr,
                                tt8Bit                        sessionIdLen)
{
    ttSslCacheElementPtr   cachePtr;
    int                    errorCode;
    int                    i;
    int                    cmp;

    errorCode = TM_EINVAL;
    cachePtr = (ttSslCacheElementPtr)0;

    for (i=0; (tt16Bit)i < sslSessionPtr->ssTotalCache; i++)
    {
        cachePtr = (ttSslCacheElementPtr)(sslSessionPtr->ssConnectCache[i]);
        if (cachePtr && (sessionIdLen == cachePtr->sceSessionIdLength))
        {
            cmp = tm_memcmp(cachePtr->sceSessionId, sessionIdPtr, sessionIdLen);
            if (!cmp)
            {
                errorCode = TM_ENOERROR;
                break;
            }
        }
    }

    if(errorCode == TM_ENOERROR)
    {
        *cachePtrPtr = cachePtr;
    }
    else
    {
        *cachePtrPtr = (ttSslCacheElementPtr)0;
    }
    return errorCode;
}

/*
 * When server parses the client hello, it needs to determine which
 * cipher suite to use. Each cipher suite will be called against this
 * function, if the return value is TM_8BIT_YES, we support that cipher
 * suite, otherwise, we don't support that cipher suite, check next
 * one please.
 */
static tt8Bit tfSslCheckOneCipherSuite(ttSslConnectStatePtr    connStatePtr,
                                       tt16Bit                 cipherSuite)
{
    ttSslCipherSuiteConstPtr  cipherPtr;
    tt8Bit                    supported;

    supported = TM_8BIT_NO;

    switch(cipherSuite)
    {
/* we dont support DH key exchange method */
#if (defined(TM_EXPORT_RESTRICT) || \
(!defined(TM_SSL_REJECT_EXPORTABLE_CIPHER)))
/* */
    case TM_TLS_RSA_NULL_MD5: /* RSA part, go through */
    case TM_TLS_RSA_NULL_SHA:
    case TM_TLS_RSA_NULL_SHA256:
    case TM_TLS_RSA_EXPORT_RC4_40_MD5:
    case TM_TLS_RSA_EXPORT_RC2_40_MD5:
    case TM_TLS_RSA_EXPORT_DES40_CBC_SHA:
    case TM_TLS_RSA_EPT1K_DES_CBC_SHA:
    case TM_TLS_RSA_EPT1K_RC4_56_SHA:
#endif /* TM_EXPORT_RESTRICT || !TM_SSL_REJECT_EXPORTABLE_CIPHER */
#ifdef TM_SSL_USE_EPHEMERAL_DH
    case TM_TLS_DHE_RSA_EPT_DES40_CBC_SHA:
#endif /* TM_SSL_USE_EPHEMERAL_DH */
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
    case TM_TLS_DHE_RSA_DES_CBC_SHA:
    case TM_TLS_DHE_RSA_3DES_EDE_CBC_SHA:
    case TM_TLS_DHE_RSA_AES_128_CBC_SHA:
    case TM_TLS_DHE_RSA_AES_256_CBC_SHA:
#ifdef TM_USE_SSL_VERSION_33
    case TM_TLS_DHE_RSA_AES_128_CBC_SHA256:
    case TM_TLS_DHE_RSA_AES_256_CBC_SHA256:
#endif /* TM_USE_SSL_VERSION_33 */
#endif /* TM_SSL_USE_EPHEMERAL_DH */
#endif /* TM_EXPORT_RESTRICT */
        if (connStatePtr->scsSessionPtr->ssSignAlgorithm == SADB_PUBKEY_RSA)
        {
            if (connStatePtr->scsVersion != TM_SSL_VERSION_33)
            {
                switch(cipherSuite)
                {
#if (defined(TM_EXPORT_RESTRICT) || \
(!defined(TM_SSL_REJECT_EXPORTABLE_CIPHER)))
                case TM_TLS_RSA_NULL_SHA256:
#endif /* TM_EXPORT_RESTRICT || !TM_SSL_REJECT_EXPORTABLE_CIPHER */
#ifndef TM_EXPORT_RESTRICT
#ifdef TM_USE_SSL_VERSION_33
                case TM_TLS_RSA_AES_128_CBC_SHA256:
                case TM_TLS_RSA_AES_256_CBC_SHA256:
#endif /* TM_USE_SSL_VERSION_33 */
#ifdef TM_SSL_USE_EPHEMERAL_DH
#ifdef TM_USE_SSL_VERSION_33
                case TM_TLS_DHE_RSA_AES_128_CBC_SHA256:
                case TM_TLS_DHE_RSA_AES_256_CBC_SHA256:
#endif /* TM_USE_SSL_VERSION_33 */
#endif /* TM_SSL_USE_EPHEMERAL_DH */
                    break;
#endif /* TM_EXPORT_RESTRICT */
                default:
                    supported = TM_8BIT_YES;
                    break;
                }
            }
            else
            {
                switch(cipherSuite)
                {
#ifdef TM_USE_SSL_VERSION_33
#ifndef TM_SSL_REJECT_EXPORTABLE_CIPHER
                case TM_TLS_RSA_EXPORT_RC4_40_MD5:
                case TM_TLS_RSA_EXPORT_RC2_40_MD5:
                case TM_TLS_RSA_EXPORT_DES40_CBC_SHA:
                case TM_TLS_RSA_EPT1K_DES_CBC_SHA:
                case TM_TLS_RSA_EPT1K_RC4_56_SHA:
#endif /* TM_SSL_REJECT_EXPORTABLE_CIPHER */
#ifdef TM_SSL_USE_EPHEMERAL_DH
                case TM_TLS_DHE_RSA_EPT_DES40_CBC_SHA:
#endif /* TM_SSL_USE_EPHEMERAL_DH */
#ifndef TM_EXPORT_RESTRICT
                case TM_TLS_RSA_DES_CBC_SHA:
#ifdef TM_SSL_USE_EPHEMERAL_DH
                case TM_TLS_DHE_RSA_DES_CBC_SHA:
#endif /* TM_SSL_USE_EPHEMERAL_DH */
#endif /* TM_EXPORT_RESTRICT */
#endif /* TM_USE_SSL_VERSION_33 */
                    break;
                default:
                    supported = TM_8BIT_YES;
                    break;
                }
            }
        }
        break;

#ifdef TM_SSL_USE_EPHEMERAL_DH
#if (defined(TM_EXPORT_RESTRICT) || \
(!defined(TM_SSL_REJECT_EXPORTABLE_CIPHER)))
    case TM_TLS_DHE_DSS_EPT_DES40_CBC_SHA:/* DSS part, go through */
    case TM_TLS_DHE_DSS_EPT1K_DES_CBC_SHA:
    case TM_TLS_DHE_DSS_EPT1K_RC4_56_SHA:
#endif /*TM_EXPORT_RESTRICT|| !defined(TM_SSL_REJECT_EXPORTABLE_CIPHER*/
#ifndef TM_EXPORT_RESTRICT
    case TM_TLS_DHE_DSS_RC4_128_SHA:
    case TM_TLS_DHE_DSS_DES_CBC_SHA:
    case TM_TLS_DHE_DSS_3DES_EDE_CBC_SHA:
    case TM_TLS_DHE_DSS_AES_128_CBC_SHA:
    case TM_TLS_DHE_DSS_AES_256_CBC_SHA:
#ifdef TM_USE_SSL_VERSION_33
    case TM_TLS_DHE_DSS_AES_128_CBC_SHA256:
    case TM_TLS_DHE_DSS_AES_256_CBC_SHA256:
#endif /* TM_USE_SSL_VERSION_33 */
#endif /* TM_EXPORT_RESTRICT */
        if (connStatePtr->scsSessionPtr->ssSignAlgorithm == SADB_PUBKEY_DSA)
        {
            if (connStatePtr->scsVersion != TM_SSL_VERSION_33)
            {
                switch(cipherSuite)
                {
#ifndef TM_EXPORT_RESTRICT
#ifdef TM_USE_SSL_VERSION_33
                case TM_TLS_DHE_DSS_AES_128_CBC_SHA256:
                case TM_TLS_DHE_DSS_AES_256_CBC_SHA256:
#endif /* TM_USE_SSL_VERSION_33 */
                    break;
#endif /* TM_EXPORT_RESTRICT */
                default:
                    supported = TM_8BIT_YES;
                    break;
                }
            }
            else
            {
                switch(cipherSuite)
                {
#ifdef TM_USE_SSL_VERSION_33
#ifndef TM_SSL_REJECT_EXPORTABLE_CIPHER
                case TM_TLS_DHE_DSS_EPT_DES40_CBC_SHA:
                case TM_TLS_DHE_DSS_EPT1K_DES_CBC_SHA:
                case TM_TLS_DHE_DSS_EPT1K_RC4_56_SHA:
#endif /* TM_SSL_REJECT_EXPORTABLE_CIPHER */
                case TM_TLS_DHE_DSS_DES_CBC_SHA:
#endif /* TM_USE_SSL_VERSION_33 */
                    break;
                default:
                    supported = TM_8BIT_YES;
                    break;
                }
            }
        }
        break;
#endif /*  TM_SSL_USE_EPHEMERAL_DH */
    default:
        break;
    }

    if (supported == TM_8BIT_YES)
    {
        connStatePtr->scsCipherPtr = tfSslCipherSuiteLookup
            (cipherSuite);
        cipherPtr = connStatePtr->scsCipherPtr;
        connStatePtr->scsCryptoAlgPtr = tfEspAlgorithmLookup
            (cipherPtr->csEncryptAlg);
/*
 * We have the cipher suite in our table, that doesn't guarantee
 * we support this cipher suite. For example RC2 is in our cipher
 * suite table, but we support it only if TM_USE_RC2 is defined
 */
        if (connStatePtr->scsCryptoAlgPtr == (ttEspAlgorithmPtr)0)
        {
            supported = TM_8BIT_NO;
        }
        else
        {
            connStatePtr->scsBlockSize = connStatePtr->scsCryptoAlgPtr
                ->ealgBlockSize;
            if (connStatePtr->scsBlockSize == 1)
            {
                connStatePtr->scsBlockSize = 0;
            }
            connStatePtr->scsHashAlgPtr = tfAhAlgorithmLookup
                        (cipherPtr->csHashAlg);
            connStatePtr->scsHashOutputSize = (tt8Bit)(connStatePtr->
                        scsHashAlgPtr->aalgDigestOutBits >> 3);
            connStatePtr->scsPubkeyEnginePtr = tfCryptoEngineGet
                        (cipherPtr->csSignAlgorithm);
            connStatePtr->scsCryptoEnginePtr = tfCryptoEngineGet
                        (cipherPtr->csEncryptAlg);
            if ((!connStatePtr->scsPubkeyEnginePtr) ||
                (!connStatePtr->scsCryptoEnginePtr))
            {
                supported = TM_8BIT_NO;
            }
        }
    }

    return supported;
}

/*
 * Description:
 * tfSslServerUserHelloRequest is used to trigger rehandshaking.
 * SSL server sends HelloRequest to SSL client. Upon receiving HelloRequest,
 * the SSL client should start SSL renegotiation. A common scenerio is
 * that: SSL client/server has negotiated a normal connection. Later, when SSL
 * server determines that mutual auth is necessory, then SSL server
 * requests the client to start a new connection negotiation with
 * mutual authentication. The client doesn't know mutual auth will be
 * required for the renegotiation, so the client may request a
 * resumption, in this case, the SSL server should ignore the resumption
 * attempt.
 *
 * PARAMETERS:
 * socketDescriptor         socket number
 * options                  options
 *
 * RETURNS:
 *  TM_ENOBUFS       No more buffer available
 *  TM_EPERM         Function call not permitted, check if
 *                   TM_TCP_SSLSESSION and TM_TCP_SSL_SERVER
 *                   are both set on this socket. And previous SSL
 *                   connection should be established.
 *  TM_EPROTONOTSUPPORT The socket is not TCP socket
 *  TM_EBADF         Socket pointer is not found
 *  TM_EOPNOTSUPP    Option not supported.
 */

int tfSslServerUserHelloRequest(int           socketDescriptor,
                                int           option)
{
    ttSocketEntryPtr          socketEntryPtr;
    ttSslConnectStatePtr      connStatePtr;
    ttTcpVectPtr              tcpVectPtr;
    ttSslRecordHeaderPtr      sslHeaderPtr;
    ttPacketPtr               packetPtr;
    tt8BitPtr                 dataPtr;
    ttSslHandShakeHeaderPtr   handshakeHeaderPtr;
    int                       errorCode;
#ifdef TM_USE_BSD_DOMAIN
    int                       af;
#endif /* TM_USE_BSD_DOMAIN */
    ttPktLen                  recordSize;
    tt16Bit                   helloRequestLen;

    errorCode = TM_ENOERROR;

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
        goto sslServerUserHQExit;
    }
/* make sure it is a TCP socket */
    if (socketEntryPtr->socProtocolNumber != (tt8Bit)IP_PROTOTCP)
    {
        errorCode = TM_EPROTONOSUPPORT;
        goto sslServerUserHQExit;
    }

    tcpVectPtr = (ttTcpVectPtr)((ttVoidPtr)socketEntryPtr);
    connStatePtr = (ttSslConnectStatePtr)tcpVectPtr->tcpsSslConnStatePtr;

    if(option & TM_SSL_OPT_CLIENTAUTH)
    {
#ifndef TM_SSL_USE_MUTUAL_AUTH
        errorCode = TM_EOPNOTSUPP;
        goto sslServerUserHQExit;
#else /* TM_SSL_USE_MUTUAL_AUTH */
/* we need to send certificate request */
        connStatePtr->scsRunFlags |= TM_SSL_RUNFLAG_SERVER_SENDCR;
#endif /* TM_SSL_USE_MUTUAL_AUTH */
    }

    if (tm_16bit_all_bits_set(tcpVectPtr->tcpsSslFlags,
                              (TM_SSLF_SERVER |
                               TM_SSLF_SESSION_SET |
                               TM_SSLF_ESTABLISHED)))
    {
/* construct HELLOREQUEST message */
        helloRequestLen = (tt16Bit)(TM_SSL_RECORD_HEADER_SIZE +
                     TM_SSL_HANDSHAKE_HEADER_SIZE +
/* hashing and encryption */
                     TM_SSL_RECORD_OVERHEAD_MAX_SIZE);
        packetPtr = tfGetSharedBuffer(0, (ttPktLen)helloRequestLen, 0);
        if (!packetPtr)
        {
            errorCode = TM_ENOBUFS;
            goto sslServerUserHQExit;
        }
        dataPtr = packetPtr->pktLinkDataPtr;
        connStatePtr->scsXmitPacketPtr = packetPtr;
/* construct a server hello message */
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
        sslHeaderPtr->srhLengthHigh = (tt8Bit)(TM_SSL_HANDSHAKE_HEADER_SIZE
                                               >> 8);
        sslHeaderPtr->srhLengthLow = (tt8Bit)(TM_SSL_HANDSHAKE_HEADER_SIZE
                                            & 0xff);

        handshakeHeaderPtr = (ttSslHandShakeHeaderPtr)
            (dataPtr + TM_SSL_RECORD_HEADER_SIZE);
        handshakeHeaderPtr->hsHandShakeType = TM_SSL_HELLO_REQUEST;
        handshakeHeaderPtr->hsLengthPad = 0;
        handshakeHeaderPtr->hsLengthHigh = 0;
        handshakeHeaderPtr->hsLengthLow = 0;
        dataPtr = (tt8BitPtr)(handshakeHeaderPtr + 1);

        helloRequestLen = TM_SSL_HANDSHAKE_HEADER_SIZE;

        errorCode = tfSslHandshakeMsgTransform(connStatePtr,
                                               TM_CRYPTOREQUEST_ENCODE,
                                               sslHeaderPtr,
                                               packetPtr,
                                               dataPtr,
                                               &helloRequestLen,
                                               TM_SSL_RECORD_HEADER_SIZE);
        if (errorCode != TM_ENOERROR)
        {
            goto sslServerUserHQExit;
        }
        recordSize = (ttPktLen)(helloRequestLen + TM_SSL_RECORD_HEADER_SIZE);
        packetPtr->pktLinkDataLength = recordSize;
        packetPtr->pktChainDataLength = recordSize;
/* clear ESTABLISHED flag to prohibit sending */
        connStatePtr->scsRunFlags &= ~(unsigned)TM_SSLF_ESTABLISHED;
        connStatePtr->scsRunFlags |= TM_SSL_RUNFLAG_REHANDSHAKE;
/* free the cache because we have to rehandshake */
        (void)tfSslFreeCache(connStatePtr->scsSessionPtr,
                             connStatePtr->scsSessionId,
                             connStatePtr->scsSessionIdLength);
        (void)tfSslSend(socketEntryPtr,
                  connStatePtr,
                  &packetPtr,
                  &recordSize,
                  0);
        connStatePtr->scsXmitPacketPtr = (ttPacketPtr)0;

        if (    (tcpVectPtr->tcpsState == TM_TCPS_ESTABLISHED)
             || (tcpVectPtr->tcpsState == TM_TCPS_CLOSE_WAIT) )
        {
            tcpVectPtr->tcpsFlags |= TCP_NODELAY;
            errorCode = tfTcpSendPacket(tcpVectPtr);
            if (errorCode != TM_ENOERROR)
            {
                tm_16bit_clr_bit(tcpVectPtr->tcpsSslFlags,
                       (   TM_SSLF_SEND_ENABLED | TM_SSLF_RECV_ENABLED
                          | TM_SSLF_MORE_RECORDS ));
            }
        }
    }
    else
    {
        errorCode = TM_EPERM;
    }
sslServerUserHQExit:
    return tfSocketReturn(socketEntryPtr,
                          socketDescriptor,
                          errorCode,
                          TM_ENOERROR);
}



/*
 * Description
 * tfSslServerUserCreate is used to create SSL server states. It must be
 * called if user sets the socket option to use SSL server after the TCP
 *connection has been established. If user sets socket option (to enable
 * SSL) before the TCP handshake begins, user don't need to make this call.
 * This function is only valid when TM_USE_SSL_SERVER is defined.
 *
 * Parameters
 * socketDescriptor         socket number
 *
 *
 * Returns
 * Value    Meaning
 * TM_ENOERROR      Success
 * -1               Error happens, user needs to call tfGetSocketError
 *                  to get the related socket error code
 *                  TM_ENOBUFS       No more buffer available
 *                  TM_EPERM         Function call not permitted, check if
 *                                   TM_TCP_SSLSESSION and TM_TCP_SSL_SERVER
 *                                   are both set on this socket
 *                  TM_EPROTONOTSUPPORT The socket is not TCP socket
 *                  TM_EBADF         Socket pointer is not found
 */

int tfSslServerUserCreate(int            socketDescriptor)
{
    int                       errorCode;
#ifdef TM_USE_BSD_DOMAIN
    int                       af;
#endif /* TM_USE_BSD_DOMAIN */
    ttSocketEntryPtr          socketEntryPtr;
    ttTcpVectPtr              tcpVectPtr;

    errorCode = TM_ENOERROR;

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
        goto sslServerUserCreateExit;
    }
/* make sure it is a TCP socket */
    if (socketEntryPtr->socProtocolNumber != (tt8Bit)IP_PROTOTCP)
    {
        errorCode = TM_EPROTONOSUPPORT;
        goto sslServerUserCreateExit;
    }

    tcpVectPtr = (ttTcpVectPtr)((ttVoidPtr)socketEntryPtr);
/* if SSL client is defined and this socket is SSL_enabled, we
 * trigger ssl state machine
 */
    if (tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                            TM_SSLF_SERVER))
    {
        errorCode = tfSslServerCreateState(tcpVectPtr);
    }
    else
    {
        errorCode = TM_EPERM;
    }
sslServerUserCreateExit:
    return tfSocketReturn(socketEntryPtr,
                          socketDescriptor,
                          errorCode,
                          TM_ENOERROR);
}


static tt8Bit tfSslCheckCipherSuite(ttSslConnectStatePtr    connStatePtr,
                                    ttSslCacheElementPtr    cachePtr,
                                    tt8BitPtr               cCipherPtr,
                                    tt16Bit                 clientCipherLen,
                                    tt8Bit                  perCipherLen)
{
    tt8BitPtr   dataPtr;
    int         i;
    int         len;
    tt16Bit     cCipherSuite;
    tt16Bit     sCipherSuite;
    tt8Bit      supported;

    supported = TM_8BIT_NO;

    if ((connStatePtr->scsSessionPtr->ssOption & TM_SSL_OPT_SERVERPROP)
         && (cachePtr == (ttSslCacheElementPtr)0))
    {
/* in this case, we need to use server's proposals to select a matched cipher
 * suite from client hello message, and in server's preference order
 */
        for (i = 0; i < TM_SSL_PROPOSAL_NUMBER; i ++)
        {
            sCipherSuite = connStatePtr->scsSessionPtr->ssProposals[i];
            if(sCipherSuite)
            {
                dataPtr = cCipherPtr;
                for(len = clientCipherLen; len > 0; len = len - perCipherLen)
                {
                    cCipherSuite = (tt16Bit)(((*(dataPtr + perCipherLen - 2)) << 8) |
                              (*(dataPtr + perCipherLen - 1)));
                    if(sCipherSuite == cCipherSuite)
                    {
                        supported = tfSslCheckOneCipherSuite(connStatePtr,
                            sCipherSuite);
                        if(supported == TM_8BIT_YES)
                        {
                            break;
                        }
                    }
                    dataPtr += perCipherLen;
                }
                if(supported == TM_8BIT_YES)
                {
                    break;
                }
            }
        }
    }
    else
    {
/* in this case, we need to use client's proposals to select a supported cipher
 * suite, and in client's preference order
 */
        len = clientCipherLen;
        while (len)
        {
            cCipherSuite = (tt16Bit)(((*(cCipherPtr + perCipherLen - 2)) << 8) |
                                     (*(cCipherPtr + perCipherLen - 1)));

            if (cachePtr == (ttSslCacheElementPtr)0 ||
                (cachePtr && cachePtr->sceCipherSuite == cCipherSuite))
            {
                supported = tfSslCheckOneCipherSuite(connStatePtr,
                    cCipherSuite);
            }
            if (supported == TM_8BIT_YES)
            {
                break;
            }
            cCipherPtr += perCipherLen;
            len =(tt16Bit) (len - perCipherLen);
        }
    }
    return supported;
}

#ifdef TM_SSL_ACCEPT_20_CLIENTHELLO
/*
 * state function, parse SSL 2.0 client hello message. We don't support
 * SSL 2.0 handshake, we only knows SSL 2.0 client hello, we can
 * understand what the client offers in the ssl 2.0 client hello and
 * reply according to it. The SSL 2.0 client hello must indicate that
 * the client either supports SSL 3.0 or TLS 1.0, otherwise,
 * we deny that SSL 2.0 client hello
 */
static int tfSslParseClientHello20(ttSslConnectStatePtr  connStatePtr,
                                   tt8BitPtr             dataPtr,
                                   tt16Bit               msgSize)
{
    ttClientHello20Ptr        cHello20Ptr;
    tt16Bit                   suiteLength;
    tt16Bit                   clientSessionLen;
    tt16Bit                   randomLen;
    int                       errorCode;
    tt8Bit                    temp8Bit;

    TM_UNREF_IN_ARG(msgSize);
    errorCode = TM_ENOERROR;
    cHello20Ptr = (ttClientHello20Ptr)dataPtr;

    suiteLength = (tt16Bit)((cHello20Ptr->ch20CipherLenHigh << 8) |
                  (cHello20Ptr->ch20CipherLenLow));
    clientSessionLen = (tt16Bit)((cHello20Ptr->ch20SessionLenHigh << 8) |
                        cHello20Ptr->ch20SessionLenLow);
    randomLen = (tt16Bit)((cHello20Ptr->ch20ChallengeHigh << 8) |
                 cHello20Ptr->ch20ChallengeLow);

    if ((connStatePtr->scsState != TM_SSL_ST_INITIAL) &&
        (connStatePtr->scsState != TM_SSL_ST_OPENED))
    {
/* we are in the middle of the negotiation, don't keep sending us
 * client hellos
 */
        errorCode = TM_EINVAL;
        goto sslParseClientHello20Exit;
    }

    if (cHello20Ptr->ch20VersionMajor != TM_SSL_VERSION_MAJOR ||
       cHello20Ptr->ch20VersionMinor > TM_SSL_VERSION_MINOR1)
    {
        errorCode = TM_EINVAL;
        goto sslParseClientHello20Exit;
    }

/* client verion support, can be either 0 or 1 */
    connStatePtr->scsVersion =
            tlVersion[(   (cHello20Ptr->ch20VersionMinor << 2)
                        | connStatePtr->scsSsVersion )];
    if (!connStatePtr->scsVersion)
    {
/* no agreement on version */
        errorCode = TM_EINVAL;
        goto sslParseClientHello20Exit;
    }

    dataPtr += sizeof(ttClientHello20);

    temp8Bit = tfSslCheckCipherSuite(connStatePtr,
                             (ttSslCacheElementPtr)0,
                             dataPtr,
                             suiteLength,
                             3);  /* SSL2.0 per cipher suite takes 3 bytes*/
    if (temp8Bit != TM_8BIT_YES)
    {
        errorCode = TM_EINVAL;/* no valid cipher suite found*/
        goto sslParseClientHello20Exit;
    }

/* we won't do any resumption with ssl 20 */
    tfGetRandomBytes(connStatePtr->scsSessionId,
                     tm_packed_byte_count(TM_SSL_SESSION_ID_SIZE));
    dataPtr += suiteLength + clientSessionLen;
/* random */
    if (randomLen > TM_SSL_RANDOM_SIZE)
    {
        tm_bcopy(dataPtr + randomLen - TM_SSL_RANDOM_SIZE,
                 connStatePtr->scsClientRandom,
                 TM_SSL_RANDOM_SIZE);
    }
    else
    {
        tm_bcopy(dataPtr,
                 connStatePtr->scsClientRandom +
                 TM_SSL_RANDOM_SIZE - randomLen,
                 randomLen);

    }

    connStatePtr->scsCompressMethod = TM_SSL_COMPMETH_NULL;

    if (errorCode == TM_ENOERROR)
    {
        errorCode = tfSslParseClientHelloAlloc(connStatePtr);
    }
sslParseClientHello20Exit:
    return errorCode;
}
#endif /*TM_SSL_ACCEPT_20_CLIENTHELLO*/

/*
 * state function, parse SSL 3.0 or TLS 1.0 client hello, the resumePtr
 * will return if it is a resume request or not.
 */
static int tfSslParseClientHello(ttSslConnectStatePtr  connStatePtr,
                                 tt8BitPtr             dataPtr,
                                 tt8BitPtr             resumePtr,
                                 tt16Bit               msgSize)
{
    ttClientHelloPtr          clientHelloPtr;
    ttSslCacheElementPtr      cachePtr;
    int                       i;
    int                       compMethCount;
    int                       errorCode;
#ifdef TM_USE_SSL_VERSION_33
    int                       j;
#endif /* TM_USE_SSL_VERSION_33 */
    tt16Bit                   suiteLength;
#ifdef TM_USE_SSL_VERSION_33
    tt16Bit                   extTotalLength;
    tt16Bit                   extLength;
    tt16Bit                   extType;
    tt16Bit                   extSigLength;
    tt8Bit                    hashAlg;
#endif /* TM_USE_SSL_VERSION_33 */
    tt8Bit                    clientSessionLen;
    tt8Bit                    temp8Bit;

    cachePtr = (ttSslCacheElementPtr)0;
    errorCode = TM_ENOERROR;
    clientHelloPtr = (ttClientHelloPtr)dataPtr;

    if ((connStatePtr->scsState != TM_SSL_ST_INITIAL) &&
        (connStatePtr->scsState != TM_SSL_ST_OPENED))
    {
/* we are in the middle of the negotiation, don't keep sending us
 * client hellos
 */
        errorCode = TM_EINVAL;
        goto sslParseClientHelloExit;
    }

    if (clientHelloPtr->chVersionMajor != TM_SSL_VERSION_MAJOR )
    {
        errorCode = TM_EINVAL;
        goto sslParseClientHelloExit;
    }
    /* length check(version + sessionID + suiteLength) */
    if( msgSize <= (4 + TM_SSL_SESSION_ID_SIZE_MAX))
    {
        errorCode = TM_EINVAL;
        goto sslParseClientHelloExit;/* message length invalid */
    }
    msgSize -= (4 + TM_SSL_SESSION_ID_SIZE_MAX);
/*
 *  If a TLS server receives a ClientHello containing a version number
 *  greater than the highest version supported by the server, it MUST
 *  reply according to the highest version supported by the server [RFC 5246]
 */
    if ((clientHelloPtr->chVersionMinor > TM_SSL_VERSION_MINOR3)
     ||(!((1 << clientHelloPtr->chVersionMinor)
              & connStatePtr->scsSessionPtr->ssVersion)))
    {
/* respond the highest version supported by the server */
        if (((connStatePtr->scsSessionPtr->ssVersion) & TM_SSL_VERSION_33)
            &&((1 << clientHelloPtr->chVersionMinor) >= TM_SSL_VERSION_33))
        {
            connStatePtr->scsVersion = TM_SSL_VERSION_33;
        }
        else if (((connStatePtr->scsSessionPtr->ssVersion) & TM_SSL_VERSION_31)
            &&((1 << clientHelloPtr->chVersionMinor) >= TM_SSL_VERSION_31))
        {
            connStatePtr->scsVersion = TM_SSL_VERSION_31;
        }
        else if (((connStatePtr->scsSessionPtr->ssVersion) & TM_SSL_VERSION_30)
            &&((1 << clientHelloPtr->chVersionMinor) >= TM_SSL_VERSION_30))
        {
            connStatePtr->scsVersion = TM_SSL_VERSION_30;
        }
    }
    else
    {
/* respond the older version available in the server */
        connStatePtr->scsVersion = (1 << clientHelloPtr->chVersionMinor)
                              & connStatePtr->scsSessionPtr->ssVersion;
    }
    if (!connStatePtr->scsVersion)
    {
/* no agreement on version */
        errorCode = TM_EINVAL;
        goto sslParseClientHelloExit;
    }
/* copy ClientHello.client_version */
    connStatePtr->scsClientVersion =
                        (tt8Bit)(1 << clientHelloPtr->chVersionMinor);

    tm_bcopy(clientHelloPtr->chRandom,
             connStatePtr->scsClientRandom,
             TM_SSL_RANDOM_SIZE);

    clientSessionLen = clientHelloPtr->chSessionIdLength;

    /* length check(sessionIdLen(1byte) + sessionId size) */
    if( msgSize <= (clientSessionLen + 1) )
    {
        errorCode = TM_EINVAL;
        goto sslParseClientHelloExit;/* message length invalid */
    }
    msgSize -= (clientSessionLen + 1);

    dataPtr += (sizeof(ttClientHello) - 1 +
                 clientSessionLen);

    if (clientSessionLen)
    {
        if (clientSessionLen > TM_SSL_SESSION_ID_SIZE_MAX)
        {
            errorCode = TM_EINVAL;
            goto sslParseClientHelloExit;
        }
/* client is trying to resume a session, we find if we still have this
 * session information
 */
/* if this is a rehandshake (for example, a response to SSL server's
 * HELLOREQUEST message, then we don't honor any session resumption.
 * Everything must be done from scratch
 */
        if(!(connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_REHANDSHAKE))
        {
            (void)tfSslServerFindCache(connStatePtr->scsSessionPtr,
                             &cachePtr,
                             &clientHelloPtr->chSessionIdBegin,
                             clientSessionLen);
        }

        if (cachePtr)
        {
            *resumePtr = TM_8BIT_YES;
/* copy this session id to our state */
            tm_bcopy(&clientHelloPtr->chSessionIdBegin,
                    connStatePtr->scsSessionId,
                    clientSessionLen);
            connStatePtr->scsSessionIdLength =
                    clientSessionLen;
            goto FINISH_SESSIONID;
        }
    }

    *resumePtr = TM_8BIT_NO;
/* if ClientHello is due to HelloRequest message, we need to clear flag
 * TM_SSL_RUNFLAG_RESUMESESSION if it was set
 */
    connStatePtr->scsRunFlags &= ~(unsigned)TM_SSL_RUNFLAG_RESUMESESSION;

/* generate random session id */
    tfGetRandomBytes(connStatePtr->scsSessionId,
                     tm_packed_byte_count(TM_SSL_SESSION_ID_SIZE));
    connStatePtr->scsSessionIdLength = TM_SSL_SESSION_ID_SIZE;

FINISH_SESSIONID:
/* dataPtr points to suite */
    suiteLength = (tt16Bit)(((*dataPtr) << 8) | (*(dataPtr+1)));
    /* length check(cipher suite + complessionLen(1byte) ) */
    if(msgSize <= (suiteLength + 1))
    {
        errorCode = TM_EINVAL;/* message length invalid */
        goto sslParseClientHelloExit;
    }
    msgSize -= (suiteLength + 1);
/* the suite part is maybe not aligned to 16bit border, because you may
 * have an odd session ID length
 */
    dataPtr += 2; /* 2 bytes for the cipher suite length */
    temp8Bit = tfSslCheckCipherSuite(connStatePtr,
                             cachePtr,
                             dataPtr,
                             suiteLength,
                             2);  /* SSL3.0 TLS1.0 per cipher suite
                                   * takes 2 bytes
                                   */
    if (temp8Bit != TM_8BIT_YES)
    {
        errorCode = TM_EINVAL;/* no valid cipher suite found*/
        goto sslParseClientHelloExit;
    }

    dataPtr += suiteLength;
    connStatePtr->scsCompressMethod = 0xFF;
    compMethCount = *dataPtr;

#ifdef TM_USE_SSL_VERSION_33
    if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
        /* length check(complession method) */
        if(msgSize <= compMethCount)
        {
            errorCode = TM_EINVAL;/* message length invalid */
            goto sslParseClientHelloExit;
        }
        msgSize -= (tt16Bit)compMethCount;
    }
#endif /* TM_USE_SSL_VERSION_33 */

    for (i = 0; i < compMethCount; i++)
    {
        dataPtr++;
        if (*dataPtr == TM_SSL_COMPMETH_NULL)
        {
            connStatePtr->scsCompressMethod = TM_SSL_COMPMETH_NULL;
            break;
        }
    }
    if (connStatePtr->scsCompressMethod == 0xFF)
    {
        errorCode = TM_EINVAL;
    }
#ifdef TM_USE_SSL_VERSION_33
    if((connStatePtr->scsVersion == TM_SSL_VERSION_33) && (msgSize > 0))
    {
        /* Check Extention */
        dataPtr++;
        extTotalLength = (tt16Bit)(((*dataPtr) << 8) | (*(dataPtr+1)));
        dataPtr += 2;
        /* length check(extentionLen field(2byte) + extention data len) */
        if(msgSize < (extTotalLength + 2))
        {
            errorCode = TM_EINVAL;/* message length invalid */
            goto sslParseClientHelloExit;
        }    

        while(extTotalLength != 0)
        {
            /* Get extention type and length */
            extType = (tt16Bit)(((*dataPtr) << 8) | (*(dataPtr+1)));
            dataPtr += 2;
            extLength = (tt16Bit)(((*dataPtr) << 8) | (*(dataPtr+1)));
            dataPtr += 2;
            /* Check extentionLength */
            if(extTotalLength < extLength)
            {
                errorCode = TM_EINVAL;
                break;
            }
            extTotalLength -= (extLength + 4);

            /* Check signature algorithm extention */
            if(extType == (tt16Bit)0x000d)
            {
                extSigLength = (tt16Bit)(((*dataPtr) << 8) | (*(dataPtr+1)));
                if((extLength-2) != extSigLength)
                {
                    errorCode = TM_EINVAL;
                    break;
                }
                dataPtr += 2;
                /* Check HASH Algorithm (SHA256,SHA1,MD5) */
                for(j = 0; j < extSigLength; j += 2)
                {
                    if((*dataPtr == TM_SSL_SUPPORT_HASH_SHA1) ||
                       (*dataPtr == TM_SSL_SUPPORT_HASH_SHA256))
                    {
                        hashAlg = *dataPtr;
                        dataPtr++;
                        if(*dataPtr == TM_SSL_SUPPORT_SIG_RSA)
                        {
                            if(connStatePtr->scsSessionPtr->ssSignAlgorithm ==
                               SADB_PUBKEY_RSA)
                            {
                                connStatePtr->scsSessionPtr->ssHashAlgorithm =
                                                                    hashAlg;
                                dataPtr++;
                                j+=2;
                                break;
                            }
                        }
                        else if(*dataPtr == TM_SSL_SUPPORT_SIG_DSA)
                        {
                            if(connStatePtr->scsSessionPtr->ssSignAlgorithm ==
                               SADB_PUBKEY_DSA)
                            {
                                connStatePtr->scsSessionPtr->ssHashAlgorithm =
                                                                    hashAlg;
                                dataPtr++;
                                j+=2;
                                break;
                            }
                        }
                        dataPtr++;
                    }
                    else
                    {
                        dataPtr += 2;
                    }
                }
                /* next extention */
                if(connStatePtr->scsSessionPtr->ssHashAlgorithm != 0)
                {
                    dataPtr += (extSigLength - j);
                }
            }
            /* Other extention is through */
            else
            {
                dataPtr += extLength;
            }
        }
    }
    /* Matching hash algorithm not found. Use default algorithm(SHA1) */
    if(connStatePtr->scsSessionPtr->ssHashAlgorithm == 0)
    {
        connStatePtr->scsSessionPtr->ssHashAlgorithm = 
                                TM_SSL_SUPPORT_HASH_SHA1;
    }
#endif /* TM_USE_SSL_VERSION_33 */
    if (cachePtr)
    {
        connStatePtr->scsRunFlags |= TM_SSL_RUNFLAG_RESUMESESSION;
        tm_bcopy(cachePtr->sceMasterSecret,
                       connStatePtr->scsMasterSecret,
                       TM_SSL_MASTER_SECRET_SIZE);
    }

    if (errorCode == TM_ENOERROR)
    {
        errorCode = tfSslParseClientHelloAlloc(connStatePtr);
    }
sslParseClientHelloExit:
    return errorCode;
}

/*
 * Verify that we have allocated the crypto engines, and
 * allocate MD5, Sha1, and Sha256 structures.
 */
static int tfSslParseClientHelloAlloc (ttSslConnectStatePtr  connStatePtr)
{
    int errorCode;

/* get the crypto engine for public stuff */
    if (   (connStatePtr->scsPubkeyEnginePtr == (ttCryptoEnginePtr)0)
        || (connStatePtr->scsCryptoEnginePtr == (ttCryptoEnginePtr)0) )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        errorCode = TM_ENOBUFS;
        connStatePtr->scsMd5CtxPtr = (ttMd5CtxPtr)
                tfSslReAlloc(connStatePtr->scsMd5CtxPtr, sizeof(ttMd5Ctx));
        if (connStatePtr->scsMd5CtxPtr != (ttMd5CtxPtr)0)
        {
            connStatePtr->scsSha1CtxPtr = (ttSha1CtxPtr) 
              tfSslReAlloc(connStatePtr->scsSha1CtxPtr, sizeof(ttSha1Ctx));
            if (connStatePtr->scsSha1CtxPtr != (ttSha1CtxPtr)0)
            {
#ifdef TM_USE_SSL_VERSION_33
                connStatePtr->scsSha256CtxPtr = (ttSha256CtxPtr) 
                    tfSslReAlloc(connStatePtr->scsSha256CtxPtr,
                                 sizeof(ttSha256Ctx));
                if (connStatePtr->scsSha256CtxPtr == (ttSha256CtxPtr)0)
                {
                    tm_free_raw_buffer(connStatePtr->scsMd5CtxPtr);
                    connStatePtr->scsMd5CtxPtr = (ttMd5CtxPtr)0;
                    tm_free_raw_buffer(connStatePtr->scsSha1CtxPtr);
                    connStatePtr->scsSha1CtxPtr = (ttSha1CtxPtr)0;
                }
                else
#endif /* TM_USE_SSL_VERSION_33 */
                {
                    errorCode = TM_ENOERROR;
                    tfMd5Init(connStatePtr->scsMd5CtxPtr);
                    tfSha1Init(connStatePtr->scsSha1CtxPtr);
#ifdef TM_USE_SSL_VERSION_33
                    tfSha256Init(connStatePtr->scsSha256CtxPtr);
#endif /* TM_USE_SSL_VERSION_33 */
                }
            }
            else
            {
                tm_free_raw_buffer(connStatePtr->scsMd5CtxPtr);
                connStatePtr->scsMd5CtxPtr = (ttMd5CtxPtr)0;
            }
        }
    }
    return errorCode;
}

/* Re-allocate buffer pointed to by bufferPtr. Do not save old content. */
static ttVoidPtr tfSslReAlloc (ttVoidPtr bufferPtr, int bufferLen)
{
    if (bufferPtr != (ttVoidPtr)0)
    {
        tm_free_raw_buffer(bufferPtr);
    }
    bufferPtr = (ttVoidPtr)tm_get_raw_buffer((ttPktLen)bufferLen);
    return bufferPtr;
}

 /*
 * State machine main function. Advance to next state and calls the
 * appropriate SSL function(s) based on state vector current State,
 * and event. Returns error value.
 *
 * Parameters:
 * connStatePtr   SSL state vector
 * paramPtr      parameter to be passed to SSL state function
 * event         Event to trigger state machine (to be used as index in
 *               tl??pTransitionMatrix[][]
 *
 * Return value
 * errorCode
 */
static int tfSslServerStateMachine (ttSslConnectStatePtr connStatePtr,
                                    ttVoidPtr            paramPtr,
                                    tt8Bit               event)
{
    ttSslTransitionMatrixEntryPtr matrixEntryPtr;
    ttSslStateProcedureEntryPtr   sslProcEntryPtr;
    int                           errCode;
    tt8Bit                        curState;
    tt8Bit                        procIndex;

/* Save current state */
    curState = connStatePtr->scsState;
/* point to transition matrix for the current state, event pair */
    matrixEntryPtr = (ttSslTransitionMatrixEntryPtr)
                                    &tlSslServerMatrix[curState][event];
/* Store next state for the event */
    connStatePtr->scsState = matrixEntryPtr->smtxTransition;
/* Index into state function table */
    procIndex = matrixEntryPtr->smtxProcedureIndex;
/* Check whether there is any function to call */
    if (procIndex < TM_SSL_SM_LAST_INDEX)
    {
/* State function table entry pointer */
        sslProcEntryPtr = (ttSslStateProcedureEntryPtr)
                   &tlSslServerProcEntry[procIndex];
/* Call all functions corresponding to index */
        do
        {
            errCode = tlSslServerFuncTable[sslProcEntryPtr->sstaFunctIndex]
                       (connStatePtr, paramPtr);
/* If state function returned an error */
            if (errCode != TM_ENOERROR)
            {
/* restore previous state, and return */
                connStatePtr->scsState = curState;
                break;
            }
/* Point to next entry in state function table */
            sslProcEntryPtr++;
        } while (sslProcEntryPtr->sprocIndex == procIndex);
    }
    else
    {
/* No function to call, just a state transition */
        errCode = 0;
    }

    return errCode;
}

#ifdef TM_PUBKEY_USE_RSA
/*
 * Description tfSslAddEphemeralRsaKeys
 * When an exportable cipher suite is used and if the server's certificate
 * is RSA and its key length is more than 512 bits, we can not use that RSA
 * key pair to exchange keys, instead, we have to use ephemeral RSA keys.
 * To generate RSA key pair in running time will cost lots of CPU time, we
 * don't support that, user must add ephemeral RSA keys by themselves using
 * this function call. This function is valid only if both TM_USE_SSL_SERVER
 * and TM_PUBKEY_USE_RSA is defined. If you previously added one ephemeral RSA
 * key, and later you want to update with a new ephemeral RSA key, you must
 * call tfSslRemoveEphemeralRsaKeys first.
 *
 * Parameters
 * sessionId    The session Id used to  add ephemeral RSA keys
 *
 * fileNamePtr  Pointer to a certificate file name with directory information
 * or PEM/DER memory string of the certificate.

 * fileFormat   Specify the type and format in fileNamePtr. The format is
 * following OpenSSL format with following macros.
 *             TM_PKI_CERTIFICATE_PEM   0x01
 *             TM_PKI_CERTIFICATE_DER   0x02
 *             TM_PKI_CERTIFICATE_STRING    0x04
 *
 * Returns
 * Value    Meaning
 * TM_ENOERROR  success
 * TM_EINVAL    Invalid session id or file format
 * TM_ENOBUFS       No more buffers
 */

int tfSslAddEphemeralRsaKeys(int                 sessionId,
                             ttUserVoidPtr       fileNamePtr,
                             int                 typeFormat)
{
    ttSslSessionPtr       sslSessionPtr;
    int                   errorCode;


    errorCode = TM_ENOERROR;
    if ( !tm_context(tvSslTlsPtr))
    {
        errorCode = TM_EPERM;
        goto sslAddEphemeralRsaExit;
    }

    if(!fileNamePtr)
    {
        errorCode = TM_EINVAL;
        goto sslAddEphemeralRsaExit;
    }

    sslSessionPtr = (ttSslSessionPtr)tfSslGetSessionUsingIndex(sessionId);
    if (!sslSessionPtr)
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        errorCode = tfPkiOwnKeyPairAddTo (fileNamePtr,
                                 typeFormat,
                                 TM_PKI_RSA_KEY,
                                 (ttVoidPtrPtr)
                                 (&sslSessionPtr->ssEphemRsaPtr),
                                 1);
        tfSslSessionReturn(sslSessionPtr);
    }

sslAddEphemeralRsaExit:
    return errorCode;

}

/* Description
 * tfSslRemoveEphemeralRsaKeys is called to remove a previously added ephemeral
 * RSA keys. This function is valid only if both TM_USE_SSL_SERVER and
 * TM_PUBKEY_USE_RSA is defined.
 *
 * Parameters
 * sessionId    The session id used to add ephemeral RSA keys
 * Returns
 * Value    Meaning
 * TM_ENOERROR      success
 * TM_EINVAL        Invalid session Id
 */

int tfSslRemoveEphemeralRsaKeys(int           sessionId)
{
    ttSslSessionPtr       sslSessionPtr;
    ttPkiPublicKeyPtr     ephRsaPtr;
    int                   errorCode;

    errorCode = TM_ENOERROR;
    sslSessionPtr = (ttSslSessionPtr)tfSslGetSessionUsingIndex(sessionId);

    if (sslSessionPtr)
    {
        ephRsaPtr = sslSessionPtr->ssEphemRsaPtr;
        if (ephRsaPtr)
        {
            sslSessionPtr->ssEphemRsaPtr = (ttPkiPublicKeyPtr)0;
            tfPkiGeneralRsaFree(ephRsaPtr->pkKeyPtr);
            tm_free_raw_buffer(ephRsaPtr);
        }
        tfSslSessionReturn(sslSessionPtr);
    }
    else
    {
        errorCode = TM_EINVAL;
    }
    return errorCode;
}
#endif /* TM_PUBKEY_USE_RSA */


/* To allocate SSL state buffer if it is not allocated, and assign the
 * state pointer value to the corresponding field in tcpVectPtr
 * Initialize the state vector
 */
int tfSslServerCreateState(ttTcpVectPtr   tcpVectPtr)
{
    ttSslSessionPtr           sslSessionPtr;
    ttSslConnectStatePtr      connStatePtr;
    int                       errorCode;
    tt8Bit                    connAllocatedHere;

    errorCode = TM_ENOERROR;
    connAllocatedHere = TM_8BIT_NO;
    connStatePtr = tcpVectPtr->tcpsSslConnStatePtr;
/* make sure that we have set the SSL session*/
    if (tm_16bit_bits_not_set(tcpVectPtr->tcpsSslFlags, TM_SSLF_SESSION_SET))
    {
        errorCode = TM_EPERM;
        goto sslServerCreateStateExit;
    }
/* Note that the state may be allocated when you set TM_TCP_SSL_SERVER
 * option, it is a all zero state vector, you still need this function
 * to initialize it
 */
    if (!connStatePtr)
    {
        tcpVectPtr->tcpsSslConnStatePtr =
               tm_get_raw_buffer(sizeof(ttSslConnectState));
        connStatePtr = tcpVectPtr->tcpsSslConnStatePtr;
        if (!connStatePtr)
        {
            errorCode = TM_ENOBUFS;
            goto sslServerCreateStateExit;
        }
        connAllocatedHere = TM_8BIT_YES;
        tm_bzero(connStatePtr, sizeof(ttSslConnectState));
    }
    else
    {
/* clear stuff of previous connection if any */
        tfSslPrefreeConnState(connStatePtr);
    }
    sslSessionPtr = (ttSslSessionPtr)
                    tfSslGetSessionUsingIndex(tcpVectPtr->tcpsSslSession);
    connStatePtr->scsSessionPtr = sslSessionPtr;
/* For server, if no session found, or session doesn't have
 * any certificate, we return error, we can't run SSL server
 * on this configuration
 */
    if (!sslSessionPtr)
    {
        errorCode = TM_EINVAL;
        goto sslServerCreateStateExit;
    }
    connStatePtr->scsSocIndex = tcpVectPtr->tcpsSocketEntry.socIndex;
    if (connStatePtr->scsSessionPtr->ssCertListPtr)
    {
        connStatePtr->scsTcpVectPtr = tcpVectPtr;
/* As a server, we should able to receive and send SSL packet */
        tcpVectPtr->tcpsSslFlags |= TM_SSLF_RECV_ENABLED | TM_SSLF_SEND_ENABLED;
        connStatePtr->scsState = TM_SSL_ST_INITIAL;
        connStatePtr->scsClientOrServer = TM_SSL_SERVER;
/* Copy version from session. What we support. */
        connStatePtr->scsSsVersion = connStatePtr->scsSessionPtr->ssVersion;
/* To be changed later during negotiation. */
        connStatePtr->scsVersion = connStatePtr->scsSsVersion;
/* ownership for back pointer from connStatePtr */
        sslSessionPtr->ssOwnerCount++;
    }
    else
    {
/* An SSL server must have certificates */
        errorCode = TM_EINVAL;
        connStatePtr->scsSessionPtr = (ttSslSessionPtr)0;
    }
/* Session will be freed if ssOwnerCount has not been increased */
    tfSslSessionReturn(sslSessionPtr);

sslServerCreateStateExit:
    if (errorCode != TM_ENOERROR)
    {
        if (connAllocatedHere != TM_8BIT_NO)
        {
            tm_free_raw_buffer(connStatePtr);
            tcpVectPtr->tcpsSslConnStatePtr = (ttSslConnectStatePtr)0;
        }
    }
    return errorCode;
}

/* The main function to process hand shake messages.
 * hsType:              handshake message type
 * connStatePtr          SSl state
 * dataPtr              The beginning of hand shake message,
 *                      should point right after the handshake
 *                      message header
 * msgSize              The handshake message size.
 * RETURN:
 * errorCode            TM_ENOERROR means success, otherwise
 *                      error condition happens.
 */
int tfSslServerProcessHandShake(tt8Bit                hsType,
                                ttSslConnectStatePtr  connStatePtr,
                                tt8BitPtr             dataPtr,
                                tt16Bit               msgSize)
{
    int                       errorCode;
    tt8Bit                    event;
    tt8Bit                    resume;

    event = TM_SSL_SE_NOEVENT;
    errorCode = TM_EINVAL;
    if (    (hsType == TM_SSL_CLIENT_HELLO)
         || (connStatePtr->scsState > TM_SSL_ST_INITIAL)
       )
    {
        switch(hsType)
        {
        case TM_SSL_CLIENT_HELLO:
#ifdef TM_SSL_ACCEPT_20_CLIENTHELLO
            if (connStatePtr->scsHeaderBuffer[0] & 0x80)
            {
                resume = TM_8BIT_NO;
                errorCode = tfSslParseClientHello20(connStatePtr,
                                                    dataPtr,
                                                    msgSize);
/* we need change it back when we calculate tfSslUpdateMd5Sha1Ctx */
                dataPtr += TM_SSL_HANDSHAKE_HEADER_SIZE;
                msgSize -= TM_SSL_HANDSHAKE_HEADER_SIZE;
            }
            else
#endif /*TM_SSL_ACCEPT_20_CLIENTHELLO*/
            {
                errorCode = tfSslParseClientHello(connStatePtr,dataPtr,
                             &resume, msgSize);
            }

            if (errorCode == TM_ENOERROR)
            {
                if (resume)
                {
                    event = TM_SSL_SE_INCH2;
                }
                else
                {
                    event = TM_SSL_SE_INCH1;
                }
            }
            break;
        case TM_SSL_CERTIFICATE:
            errorCode = tfSslParseCertificate(connStatePtr,dataPtr, msgSize);
            if (errorCode == TM_ENOERROR)
            {
                event = TM_SSL_SE_INCERT;
            }
            break;
#ifdef TM_SSL_USE_MUTUAL_AUTH
        case TM_SSL_CERTIFICATE_VERIFY:
            errorCode = tfSslParseCertificateVerify(connStatePtr,dataPtr,
                                                    msgSize);
            if (errorCode == TM_ENOERROR)
            {
                event = TM_SSL_SE_INVERIFY;
            }
            break;
#endif /* TM_SSL_USE_MUTUAL_AUTH */
        case TM_SSL_CLIENT_KEY_EXCHANGE:
            if (   (!(connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_SERVER_SENDCR))
                || (connStatePtr->scsState == TM_SSL_ST_RECV_CERT))
            {
/*
 * Server did not ask for a certificate, or client already replied with
 * a certificate.
 */
                errorCode = tfSslParseClientKeyExchange(connStatePtr,
                                                        dataPtr,
                                                        msgSize);
                if (errorCode == TM_ENOERROR)
                {
                    event = TM_SSL_SE_INCKE;
                }
            }
            break;
        case TM_SSL_FINISHED:
            errorCode = tfSslParseFinished(connStatePtr,dataPtr, msgSize);
            if (errorCode == TM_ENOERROR)
            {
                event = TM_SSL_SE_INFINISH;
            }
            break;
        default:
            break;
        }
        if (errorCode == TM_ENOERROR)
        {
#ifdef TM_USE_SSL_VERSION_33
            if (connStatePtr->scsVersion == TM_SSL_VERSION_33)
            {
                tfSha256Update(connStatePtr->scsSha256CtxPtr,
                               dataPtr - TM_SSL_HANDSHAKE_HEADER_SIZE,
                               (tt16Bit)(msgSize +
                                         TM_SSL_HANDSHAKE_HEADER_SIZE),
                               0);
            }
#endif /* TM_USE_SSL_VERSION_33 */
            tfSslUpdateMd5Sha1Ctx(connStatePtr,
                                  dataPtr - TM_SSL_HANDSHAKE_HEADER_SIZE,
                                  (tt16Bit)(msgSize +
                                            TM_SSL_HANDSHAKE_HEADER_SIZE));
            if (event != TM_SSL_SE_NOEVENT)
            {
                errorCode = tfSslServerStateMachine (connStatePtr,
                                                     0,
                                                     event);
            }
        }
    }
    return errorCode;
}

#else /* ! TM_USE_SSL_SERVER */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_SSL_SERVER not defined */
int tlSslServerDummy = 0;
#endif /* TM_USE_SSL_SERVER */

/***************** End Of File *****************/
