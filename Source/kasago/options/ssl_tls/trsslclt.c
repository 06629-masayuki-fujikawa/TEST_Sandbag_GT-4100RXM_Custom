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
 * Description: SSL and TLS Client implementation
 *
 * Filename: trsslclt.c
 * Author: Jin Zhang
 * Date Created:   12/07/2003
 * $Source: source/trsslclt.c $
 *
 * Modification History
 * $Revision: 6.0.2.31 $
 * $Date: 2015/12/11 10:52:26JST $
 * $Author: pcarney $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * Include
 */
#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_SSL_CLIENT

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>
#include <trssl.h>

#define TM_SSL_CLIENT_REPLY_SIZE        256

/* TM_SSL_CLIENT_HELLO_LENGTH must be equal or greater than:
 * TM_SSL_PROPOSAL_NUMBER * 2 + 2(suite length)
 * + 2(compress) + sessionid + 1 + 32(random) + 11 (header)
 * = 90 (when proposal number = 5)
 */
#define TM_SSL_CLIENT_HELLO_LENGTH        128
/* SSL Client function pointer indexes */
#define TM_SSL_CFN_CLIENT_HELLO      0
#define TM_SSL_CFN_PREREPLY          1
#define TM_SSL_CFN_CERTIFICATE       2
#define TM_SSL_CFN_CLIENT_KE         3
#define TM_SSL_CFN_CERTVERI          4
#define TM_SSL_CFN_CHANGE_CIPHER     5
#define TM_SSL_CFN_FINISHED          6
#define TM_SSL_CFN_SENDQUEUE         7
#define TM_SSL_CFN_NOOP              8
/*
 * SSL Client States
 */
#define TM_SSL_CT_INITIAL             0
#define TM_SSL_CT_SENT_CHELLO         1 /* Sent Client Hello                */
#define TM_SSL_CT_RECV_SH1            2 /* Received regular Server Hello    */
#define TM_SSL_CT_RECV_CERT           4 /* Received server certificate      */
#define TM_SSL_CT_RECV_CERTREQ        5 /* Received server cert request     */
#define TM_SSL_CT_RECV_SKE            6 /* Received server key exchange     */
/* #define TM_SSL_CT_RECV_SH2         3  Received resuming Server Hello
 * #define TM_SSL_CT_SENT_FINISH      7  Sent client Finished to server
 * #define TM_SSL_CT_RECV_CCS1        8  Received regular Change Cipher
 * #define TM_SSL_CT_RECV_CCS2        9  Receiving resuming change cipher
 * #define TM_SSL_CT_OPENED          10  Client Opened
 * defined in trssl.h
 */
#define TM_SSL_CT_INVALID          0xff

/*
 * SSL Client Events
 */
#define TM_SSL_CE_OUTCH              TM_SSL_CLIENT_START
#define TM_SSL_CE_INSH1              1
#define TM_SSL_CE_INSH2              2
#define TM_SSL_CE_INCERT             3
#define TM_SSL_CE_INCERTREQ          4
#define TM_SSL_CE_INSKE              5
#define TM_SSL_CE_INDONE             6
#define TM_SSL_CE_INCCS              7
#define TM_SSL_CE_INFINISH           8
#define TM_SSL_CE_INHELLREQ          9
#define TM_SSL_CE_NOEVENT          (tt8Bit)255

/*
 * Action Procedure Index Defines
 * These are tied to the Function Table
 * If this is changed, the function table
 * MUST be changed as well and visa-versa.
 */
#define TM_SSL_CM_SENDCCH            0
/*resume session. Finished from server */
#define TM_SSL_CM_RESUMFINISH        1
/* regular finished from server */
#define TM_SSL_CM_FINISH             4
#define TM_SSL_CM_HELLODONE          5
#define TM_SSL_CM_LAST_INDEX         11
#define TM_SSL_CM_TRANS_NOOP         TM_SSL_CM_LAST_INDEX

static ttSslCacheElementPtr tfSslClientFindCache
                         (ttSslConnectStatePtr          connStatePtr);
static int tfSslClientStateMachine(ttSslConnectStatePtr connectStatusPtr,
                                   ttVoidPtr            argUnion,
                                   tt8Bit               event);
static int tfSslSendClientHello(ttSslConnectStatePtr    connStatePtr,
                                 ttVoidPtr              cachedItemPtr);
static int tfSslClientPreReply(ttSslConnectStatePtr     connStatePtr,
                               ttVoidPtr                paramPtr);
static int tfSslSendClientKE(ttSslConnectStatePtr       connStatePtr,
                             ttVoidPtr                  paramPtr);
static int tfSslSendCertVerify(ttSslConnectStatePtr     connStatePtr,
                               ttVoidPtr                paramPtr);
static int tfSslSendClientCCS(ttSslConnectStatePtr      connStatePtr,
                              ttVoidPtr                 paramPtr);
/* function in this table means what client should do, generally,
 * it means need send something.
 */
static const ttSslStateFunctPtr  TM_CONST_QLF tlSslClientFuncTable[] =
{
    (ttSslStateFunctPtr)tfSslSendClientHello,   /* TM_SSL_CFN_CLIENT_HELLO */
    (ttSslStateFunctPtr)tfSslClientPreReply,    /* TM_SSL_CFN_PREREPLY     */
    (ttSslStateFunctPtr)tfSslSendCertificate,   /* TM_SSL_CFN_CERTIFICATE  */
    (ttSslStateFunctPtr)tfSslSendClientKE,      /* TM_SSL_CFN_CLIENT_KE    */
    (ttSslStateFunctPtr)tfSslSendCertVerify,    /* TM_SSL_CFN_CERTVERI     */
    (ttSslStateFunctPtr)tfSslSendClientCCS,     /* TM_SSL_CFN_CHANGE_CIPHER*/
    (ttSslStateFunctPtr)tfSslBuildSendFinished, /* TM_SSL_CFN_FINISHED     */
    (ttSslStateFunctPtr)tfSslSendQueuedData     /* TM_SSL_CFN_SENDQUEUE    */
};


static const ttSslStateProcedureEntry TM_CONST_QLF tlSslClientProcedureEntry[] =
{
    {TM_SSL_CM_SENDCCH,      TM_SSL_CFN_CLIENT_HELLO  }, /* 00 */
    {TM_SSL_CM_RESUMFINISH,  TM_SSL_CFN_CHANGE_CIPHER }, /* 01 */
    {TM_SSL_CM_RESUMFINISH,  TM_SSL_CFN_FINISHED      }, /* 01 */
    {TM_SSL_CM_RESUMFINISH,  TM_SSL_CFN_SENDQUEUE     }, /* 01 */
    {TM_SSL_CM_FINISH,       TM_SSL_CFN_SENDQUEUE     }, /* 04 */
    {TM_SSL_CM_HELLODONE,    TM_SSL_CFN_PREREPLY      }, /* 05 */
    {TM_SSL_CM_HELLODONE,    TM_SSL_CFN_CERTIFICATE   }, /* 05 */
    {TM_SSL_CM_HELLODONE,    TM_SSL_CFN_CLIENT_KE     }, /* 05 */
    {TM_SSL_CM_HELLODONE,    TM_SSL_CFN_CERTVERI      }, /* 05 */
    {TM_SSL_CM_HELLODONE,    TM_SSL_CFN_CHANGE_CIPHER }, /* 05 */
    {TM_SSL_CM_HELLODONE,    TM_SSL_CFN_FINISHED      }, /* 05 */
    {TM_SSL_CM_TRANS_NOOP ,  255                      }  /* 12 */
};



static const ttSslTransitionMatrixEntry TM_CONST_QLF tlSslClientMatrix[11][10] =
{
    {/* State == TM_SSL_CT_INITIAL */
    /*  Event                     Next State       Action Procedure Index */
    {/* TM_SSL_CE_OUTCH    */ TM_SSL_CT_SENT_CHELLO,  TM_SSL_CM_SENDCCH    },
    {/* TM_SSL_CE_INSH1    */ TM_SSL_CT_INITIAL,      TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSH2    */ TM_SSL_CT_INITIAL,      TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCERT   */ TM_SSL_CT_INITIAL,      TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCERTREQ*/ TM_SSL_CT_INITIAL,      TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSKE    */ TM_SSL_CT_INITIAL,      TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INDONE   */ TM_SSL_CT_INITIAL,      TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCCS    */ TM_SSL_CT_INITIAL,      TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INFINISH */ TM_SSL_CT_INITIAL,      TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INHELLREQ*/ TM_SSL_CT_SENT_CHELLO,  TM_SSL_CM_SENDCCH    }
    },

    {/* State == TM_SSL_CT_SENT_CHELLO */
    /*  Event                     Next State       Action Procedure Index */
    {/* TM_SSL_CE_OUTCH    */ TM_SSL_CT_SENT_CHELLO,  TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSH1    */ TM_SSL_CT_RECV_SH1,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSH2    */ TM_SSL_CT_RECV_SH2,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCERT   */ TM_SSL_CT_SENT_CHELLO,  TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCERTREQ*/ TM_SSL_CT_SENT_CHELLO,  TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSKE    */ TM_SSL_CT_SENT_CHELLO,  TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INDONE   */ TM_SSL_CT_SENT_CHELLO,  TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCCS    */ TM_SSL_CT_SENT_CHELLO,  TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INFINISH */ TM_SSL_CT_SENT_CHELLO,  TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INHELLREQ*/ TM_SSL_CT_SENT_CHELLO,  TM_SSL_CM_TRANS_NOOP }
    },

    {/* State == TM_SSL_CT_RECV_SH1 */
    /*  Event                     Next State       Action Procedure Index */
    {/* TM_SSL_CE_OUTCH    */ TM_SSL_CT_RECV_SH1,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSH1    */ TM_SSL_CT_RECV_SH1,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSH2    */ TM_SSL_CT_RECV_SH1,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCERT   */ TM_SSL_CT_RECV_CERT,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCERTREQ*/ TM_SSL_CT_RECV_SH1,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSKE    */ TM_SSL_CT_RECV_SH1,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INDONE   */ TM_SSL_CT_RECV_SH1,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCCS    */ TM_SSL_CT_RECV_SH1,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INFINISH */ TM_SSL_CT_RECV_SH1,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INHELLREQ*/ TM_SSL_CT_RECV_SH1,     TM_SSL_CM_TRANS_NOOP }
    },

    {/* State == TM_SSL_CT_RECV_SH2 */
    /*  Event                     Next State       Action Procedure Index */
    {/* TM_SSL_CE_OUTCH    */ TM_SSL_CT_RECV_SH2,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSH1    */ TM_SSL_CT_RECV_SH2,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSH2    */ TM_SSL_CT_RECV_SH2,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCERT   */ TM_SSL_CT_RECV_SH2,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCERTREQ*/ TM_SSL_CT_RECV_SH2,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSKE    */ TM_SSL_CT_RECV_SH2,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INDONE   */ TM_SSL_CT_RECV_SH2,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCCS    */ TM_SSL_CT_RECV_CCS2,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INFINISH */ TM_SSL_CT_RECV_SH2,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INHELLREQ*/ TM_SSL_CT_RECV_SH2,     TM_SSL_CM_TRANS_NOOP }
    },

    {/* State == TM_SSL_CT_RECV_CERT */
    /*  Event                     Next State       Action Procedure Index */
    {/* TM_SSL_CE_OUTCH    */ TM_SSL_CT_RECV_CERT,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSH1    */ TM_SSL_CT_RECV_CERT,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSH2    */ TM_SSL_CT_RECV_CERT,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCERT   */ TM_SSL_CT_RECV_CERT,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCERTREQ*/ TM_SSL_CT_RECV_CERTREQ, TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSKE    */ TM_SSL_CT_RECV_SKE,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INDONE   */ TM_SSL_CT_SENT_FINISH,  TM_SSL_CM_HELLODONE  },
    {/* TM_SSL_CE_INCCS    */ TM_SSL_CT_RECV_CERT,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INFINISH */ TM_SSL_CT_RECV_CERT,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INHELLREQ*/ TM_SSL_CT_RECV_CERT,    TM_SSL_CM_TRANS_NOOP }
    },

    {/* State == TM_SSL_CT_RECV_CERTREQ */
    /*  Event                     Next State       Action Procedure Index */
    {/* TM_SSL_CE_OUTCH    */ TM_SSL_CT_RECV_CERTREQ, TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSH1    */ TM_SSL_CT_RECV_CERTREQ, TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSH2    */ TM_SSL_CT_RECV_CERTREQ, TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCERT   */ TM_SSL_CT_RECV_CERTREQ, TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCERTREQ*/ TM_SSL_CT_RECV_CERTREQ, TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSKE    */ TM_SSL_CT_RECV_CERTREQ, TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INDONE   */ TM_SSL_CT_SENT_FINISH,  TM_SSL_CM_HELLODONE  },
    {/* TM_SSL_CE_INCCS    */ TM_SSL_CT_RECV_CERTREQ, TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INFINISH */ TM_SSL_CT_RECV_CERTREQ, TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INHELLREQ*/ TM_SSL_CT_RECV_CERTREQ, TM_SSL_CM_TRANS_NOOP }
    },

    {/* State == TM_SSL_CT_RECV_SKE  */
    /*  Event                     Next State       Action Procedure Index */
    {/* TM_SSL_CE_OUTCH    */ TM_SSL_CT_RECV_SKE,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSH1    */ TM_SSL_CT_RECV_SKE,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSH2    */ TM_SSL_CT_RECV_SKE,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCERT   */ TM_SSL_CT_RECV_SKE,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCERTREQ*/ TM_SSL_CT_RECV_CERTREQ, TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSKE    */ TM_SSL_CT_RECV_SKE,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INDONE   */ TM_SSL_CT_SENT_FINISH,  TM_SSL_CM_HELLODONE  },
    {/* TM_SSL_CE_INCCS    */ TM_SSL_CT_RECV_SKE,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INFINISH */ TM_SSL_CT_RECV_SKE,     TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INHELLREQ*/ TM_SSL_CT_RECV_SKE,     TM_SSL_CM_TRANS_NOOP }
    },

    {/* State == TM_SSL_CT_SENT_FINISH  */
    /*  Event                     Next State       Action Procedure Index */
    {/* TM_SSL_CE_OUTCH    */ TM_SSL_CT_SENT_FINISH,  TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSH1    */ TM_SSL_CT_SENT_FINISH,  TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSH2    */ TM_SSL_CT_SENT_FINISH,  TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCERT   */ TM_SSL_CT_SENT_FINISH,  TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCERTREQ*/ TM_SSL_CT_SENT_FINISH,  TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSKE    */ TM_SSL_CT_SENT_FINISH,  TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INDONE   */ TM_SSL_CT_SENT_FINISH,  TM_SSL_CM_HELLODONE  },
    {/* TM_SSL_CE_INCCS    */ TM_SSL_CT_RECV_CCS1,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INFINISH */ TM_SSL_CT_SENT_FINISH,  TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INHELLREQ*/ TM_SSL_CT_SENT_FINISH,  TM_SSL_CM_TRANS_NOOP }
    },

    {/* State == TM_SSL_CT_RECV_CCS1  */
    /*  Event                     Next State       Action Procedure Index */
    {/* TM_SSL_CE_OUTCH    */ TM_SSL_CT_RECV_CCS1,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSH1    */ TM_SSL_CT_RECV_CCS1,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSH2    */ TM_SSL_CT_RECV_CCS1,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCERT   */ TM_SSL_CT_RECV_CCS1,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCERTREQ*/ TM_SSL_CT_RECV_CCS1,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSKE    */ TM_SSL_CT_RECV_CCS1,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INDONE   */ TM_SSL_CT_RECV_CCS1,    TM_SSL_CM_HELLODONE  },
    {/* TM_SSL_CE_INCCS    */ TM_SSL_CT_RECV_CCS1,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INFINISH */ TM_SSL_CT_OPENED,       TM_SSL_CM_FINISH     },
    {/* TM_SSL_CE_INHELLREQ*/ TM_SSL_CT_RECV_CCS1,    TM_SSL_CM_TRANS_NOOP }
    },

    {/* State == TM_SSL_CT_RECV_CCS2  */
    /*  Event                     Next State       Action Procedure Index */
    {/* TM_SSL_CE_OUTCH    */ TM_SSL_CT_RECV_CCS2,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSH1    */ TM_SSL_CT_RECV_CCS2,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSH2    */ TM_SSL_CT_RECV_CCS2,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCERT   */ TM_SSL_CT_RECV_CCS2,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCERTREQ*/ TM_SSL_CT_RECV_CCS2,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSKE    */ TM_SSL_CT_RECV_CCS2,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INDONE   */ TM_SSL_CT_RECV_CCS2,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCCS    */ TM_SSL_CT_RECV_CCS2,    TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INFINISH */ TM_SSL_CT_OPENED,       TM_SSL_CM_RESUMFINISH},
    {/* TM_SSL_CE_INHELLREQ*/ TM_SSL_CT_RECV_CCS2,    TM_SSL_CM_TRANS_NOOP }
    },

    {/* State == TM_SSL_CT_OPENED  */
    /*  Event                     Next State       Action Procedure Index */
    {/* TM_SSL_CE_OUTCH    */ TM_SSL_CT_SENT_CHELLO,  TM_SSL_CM_SENDCCH    },
    {/* TM_SSL_CE_INSH1    */ TM_SSL_CT_OPENED,       TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSH2    */ TM_SSL_CT_OPENED,       TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCERT   */ TM_SSL_CT_OPENED,       TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCERTREQ*/ TM_SSL_CT_OPENED,       TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INSKE    */ TM_SSL_CT_OPENED,       TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INDONE   */ TM_SSL_CT_OPENED,       TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INCCS    */ TM_SSL_CT_OPENED,       TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INFINISH */ TM_SSL_CT_OPENED,       TM_SSL_CM_TRANS_NOOP },
    {/* TM_SSL_CE_INHELLREQ*/ TM_SSL_CT_SENT_CHELLO,  TM_SSL_CM_SENDCCH    }
    }
};

/* Our procedure for the state machine is that.
 * 1) For a given state and given event, look up the tlSslClientMatrix
 *    to find the next state, and the action procedure index we should
 *    take. Action Procedure Index looks like TM_SSL_CM_xxxx
 * 2) Using that action procedure index number and look up
 *    tlSslClientProcedureEntry, to get all the function indices
 *    (TM_SSL_CFN_*)
 *    which we should call one by one.
 * 3) Using function index TM_SSL_CFN_* to call function
 *    tlSslClientFuncTable[TM_SSL_CFN_*]
 */





/*
 * parses the server hello handshake message
 */
static int tfSslParseServerHello(ttSslConnectStatePtr    connStatePtr,
                                 tt8BitPtr               dataPtr,
                                 tt8BitPtr               resumePtr,
                                 tt16Bit                 msgSize)
{
    ttServerHelloPtr            serverHelloPtr;
    tt8BitPtr                   sessionIdPtr;
    unsigned                    sessionIdLength;
    int                         errorCode;
    int                         i;
    int                         compMethCount;
    int                         isEqual;
    tt16Bit                     cipherSuite;
    tt8Bit                      resume;
    tt8Bit                      validSessionId;

    tm_trace_func_entry(tfSslParseServerHello);
    tm_debug_log1("tfSslParseServerHello sd=%u",
          connStatePtr->scsTcpVectPtr->tcpsSocketEntry.socIndex);
#ifdef TM_LOG_SSL_DEBUG
    tfEnhancedLogWrite(
            TM_LOG_MODULE_SSL,
            TM_LOG_LEVEL_DEBUG,
            "tfSslParseServerHello: sd=%u",
            connStatePtr->scsTcpVectPtr->tcpsSocketEntry.socIndex);
#endif /* TM_LOG_SSL_DEBUG */

    errorCode = TM_ENOERROR;
    resume = TM_8BIT_NO;
    TM_UNREF_IN_ARG(msgSize);
    serverHelloPtr = (ttServerHelloPtr)dataPtr;
    if ((serverHelloPtr->shVersionMajor != TM_SSL_VERSION_MAJOR) ||
         (serverHelloPtr->shVersionMinor > TM_SSL_VERSION_MINOR3))
    {
        errorCode = TM_EINVAL;
        goto sslParseServerHelloExit;
    }
#ifdef TM_USE_SSL_VERSION_33
    if ((connStatePtr->scsVersion != TM_SSL_VERSION_33)
      &&(connStatePtr->scsSha256CtxPtr))
    {
/* Free unused calculating buffer */
        tm_free_raw_buffer(connStatePtr->scsSha256CtxPtr);
        connStatePtr->scsSha256CtxPtr = (ttSha256CtxPtr)0;
    }
#endif /* TM_USE_SSL_VERSION_33 */

/*  we don't care what server 's highest version
 *  serverHelloPtr->shVersionMinor, we already set it using server's
 *  handshake message version. See tfSslIncomingPreProcessing where
 *  we set the client's version
 */
    tm_bcopy(serverHelloPtr->shRandom,
             connStatePtr->scsServerRandom,
             TM_SSL_RANDOM_SIZE);

    sessionIdLength = (unsigned)serverHelloPtr->shSessionIdLength;
    sessionIdPtr = &serverHelloPtr->shSessionIdBegin;
    if (sessionIdLength != 0)
    {
/* Check that the session ID length is valid */
        validSessionId = TM_8BIT_NO; /* assume failure */
        if (sessionIdLength <= TM_SSL_SESSION_ID_SIZE_MAX)
        {
/* Check that the server ID is not zero */
            for (i = 0 ; (unsigned)i < sessionIdLength; i++)
            {
                if (sessionIdPtr[i] != (tt8Bit)0)
                {
                    validSessionId = TM_8BIT_YES;
                    break;
                }
            }
        }
        if (validSessionId == TM_8BIT_NO)
        {
/* Invalid session ID */
            errorCode = TM_EINVAL;
            goto sslParseServerHelloExit;
        }
    }

    if (connStatePtr->scsSessionIdLength)
    {
/* we tried resume, see what server replies*/
        isEqual = tm_memcmp(connStatePtr->scsSessionId,
                       sessionIdPtr,
                       sessionIdLength);
        if (( sessionIdLength == connStatePtr->scsSessionIdLength) &&
            ( isEqual == 0))
        {
            resume = TM_8BIT_YES;
        }
        else
        {
/* Server doesn't honor our resumption attempt. We should clear our
 * obsolete cache.
 */
            (void)tfSslFreeCache(connStatePtr->scsSessionPtr,
                           connStatePtr->scsSessionId,
                           connStatePtr->scsSessionIdLength);
        }
    }

    if (resume == TM_8BIT_NO)
    {
        connStatePtr->scsSessionIdLength = (tt8Bit)sessionIdLength;
        if (sessionIdLength != 0)
        {
            tm_bcopy(&serverHelloPtr->shSessionIdBegin,
                     connStatePtr->scsSessionId,
                     sessionIdLength);
        }
    }

    dataPtr += sessionIdLength + TM_SSL_RANDOM_SIZE + 3;/* v1+v2+sessionlen*/

    cipherSuite = (tt16Bit)
                     (((*dataPtr) << 8) | (*(dataPtr+1)));
/* verify if that cipher suite is what we offered */
    if (resume == TM_8BIT_YES)
    {
        if (cipherSuite != connStatePtr->scsCipherPtr->csNumber)
        {
            errorCode = TM_EINVAL;
        }
    }
    else
    {
        errorCode = TM_EINVAL; /* assume not match */
        for (i = 0; i < TM_SSL_PROPOSAL_NUMBER; i++)
        {
            if (connStatePtr->scsSessionPtr->ssProposals[i] == cipherSuite)
            {
                connStatePtr->scsCipherPtr = tfSslCipherSuiteLookup
                           (cipherSuite);
                errorCode = TM_ENOERROR;
                break;
            }
        }
    }

    if (errorCode != TM_ENOERROR)
    {
        goto sslParseServerHelloExit;
    }
    else
    {
/*
 * We have the cipher suite in our table, that doesn't guarantee
 * we support this cipher suite. For example RC2 is in our cipher
 * suite table, but we support it only if TM_USE_RC2 is defined
 */
        connStatePtr->scsCryptoAlgPtr = tfEspAlgorithmLookup
                   (connStatePtr->scsCipherPtr->csEncryptAlg);
        if (connStatePtr->scsCryptoAlgPtr == (ttEspAlgorithmPtr)0)
        {
            errorCode = TM_EINVAL;
            goto sslParseServerHelloExit;
        }
        connStatePtr->scsBlockSize = connStatePtr->scsCryptoAlgPtr
                   ->ealgBlockSize;
        if (connStatePtr->scsBlockSize == 1)
        {
            connStatePtr->scsBlockSize = 0;
        }
        connStatePtr->scsHashAlgPtr = tfAhAlgorithmLookup
                   (connStatePtr->scsCipherPtr->csHashAlg);
        connStatePtr->scsHashOutputSize = (tt8Bit)(connStatePtr->
            scsHashAlgPtr->aalgDigestOutBits >> 3);
    }

/* get the public key engine using the signature algorithm */
    connStatePtr->scsPubkeyEnginePtr = tfCryptoEngineGet
        (connStatePtr->scsCipherPtr->csSignAlgorithm);
    connStatePtr->scsCryptoEnginePtr = tfCryptoEngineGet
        (connStatePtr->scsCipherPtr->csEncryptAlg);
    if (!connStatePtr->scsPubkeyEnginePtr ||
        !connStatePtr->scsCryptoEnginePtr)
    {
        errorCode = TM_EINVAL;
        goto sslParseServerHelloExit;
    }

    connStatePtr->scsCompressMethod = 0xFF;
    compMethCount = *++dataPtr;
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
        goto sslParseServerHelloExit;
    }

    if ((connStatePtr->scsCipherPtr->csKeyExchange == TM_SSL_KEXCH_DHE)
         || (connStatePtr->scsCipherPtr->csKeyExchange ==
             TM_SSL_KEXCH_DHEEXPORT1024))
    {
/* in this case, we expect server to send us EDH in server KE */
        connStatePtr->scsRunFlags |= TM_SSL_RUNFLAG_SERVER_SENDEDH;
    }
    else if ((connStatePtr->scsCipherPtr->csKeyExchange ==
             TM_SSL_KEXCH_RSAEXPORT) ||
             (connStatePtr->scsCipherPtr->csKeyExchange ==
             TM_SSL_KEXCH_RSAEXPORT1024))
    {
/* in this case, we MAY expect server to to send us ERSA in server KE
 * we need reset this flag bit after we process the certificate to
 * see exactly how many bits the RSA uses.
 */
        connStatePtr->scsRunFlags |= TM_SSL_RUNFLAG_SERVER_SENDERSA;

    }

    if (resume == TM_8BIT_YES)
    {
        errorCode = tfSslGenerateCryptoMacKeys(connStatePtr);
        if(errorCode != TM_ENOERROR)
        {
            goto sslParseServerHelloExit;
        }
        connStatePtr->scsRunFlags |= TM_SSL_RUNFLAG_RESUMESESSION;
    }

sslParseServerHelloExit:
    if (resumePtr)
    {
        *resumePtr = resume;
    }

    tm_trace_func_exit_rc(tfSslParseServerHello, errorCode);
    return errorCode;
}

/*
 * parses the server Change Cipher handshake message
 */
int tfSslParseServerCCS(ttSslConnectStatePtr   connStatePtr,
                        ttPacketPtr            packetPtr)
{
    tt8BitPtr               dataPtr;
    int                     errorCode;
    tt8Bit                  origState;

    errorCode = TM_ENOERROR;

    dataPtr = packetPtr->pktLinkDataPtr;
    if (*dataPtr == TM_SSL_CHANGECIPHER_VALID &&
         connStatePtr->scsNextReadRecordSize == 1)
    {
        origState = connStatePtr->scsState;
        (void)tfSslClientStateMachine(connStatePtr,
                                      0,
                                      TM_SSL_CE_INCCS);
        if (connStatePtr->scsState != origState)
        {
            connStatePtr->scsRunFlags |= TM_SSL_RUNFLAG_CCS_RECVD;
            errorCode = tfSslServerWriteKeySchedule(connStatePtr,
                &connStatePtr->scsDecodeMacKey,
                &connStatePtr->scsDecodeCryptoKey,
                &connStatePtr->scsDecodeIvPtr);
            if (errorCode == TM_ENOERROR)
            {
                connStatePtr->scsInSeqNumber = 0;
                connStatePtr->scsInSeqNumberHigh = 0;
            }
            else
            {
                connStatePtr->scsState = origState;
            }
        }
        else
        {
            (void)tfSslBuildSendAlert(connStatePtr,
                                      TM_SSL_ALTLEV_FATAL,
                                      TM_SSL_ALT_UNEXPECTEDMSG);
            errorCode = TM_EINVAL;
        }
    }

    return errorCode;
}


/*
 * parses the server certificate request handshake message
 */
static int tfSslParseCertificateRequest(ttSslConnectStatePtr    connStatePtr,
                                        tt8BitPtr               dataPtr,
                                        tt16Bit                 msgSize)
{
    ttPkiCertListPtr    certListPtr;
    int                 i;
    int                 errorCode;
    tt16Bit             fieldLength;
    tt8Bit              signAlgorithm;
    tt8Bit              creqMatch;
    int                 pkiType;
    int                 cmp;
#ifdef TM_USE_SSL_VERSION_33
    ttPkiCertListPtr    suppCertListPtr;
    tt8Bit              hashAlg[64];
    tt8Bit              signAlg[64];
    tt8Bit              certHash;
    tt8Bit              certSign;
#endif /* TM_USE_SSL_VERSION_33 */

    errorCode = TM_ENOERROR;
    creqMatch = TM_8BIT_NO;
    signAlgorithm = 0;

/* 1. Parse the certificate type*/
    certListPtr = connStatePtr->scsSessionPtr->ssCertListPtr;
    if (!certListPtr)
    {
/* we don't have any certificate, however, server request our certificate,
 * we will return this error to server.
 */
#ifdef TM_SSL_CLIENT_RESPONSE_NO_CERT
        if (tm_16bit_one_bit_set(connStatePtr->scsSessionPtr->ssOption,
                                 TM_SSL_OPT_CLIENT_NOCERT))
        {
            connStatePtr->scsRunFlags |= TM_SSL_RUNFLAG_CLIENT_SEND_NOCERT;
        }
#endif /* TM_SSL_CLIENT_RESPONSE_NO_CERT */
        goto sslParseCertRequestExit;
    }
    else
    {
        pkiType = certListPtr->certKeyPtr->pkType;
        if (pkiType == TM_PKI_OBJ_DSA  ||
                pkiType == TM_PKI_OBJ_DSASHA1)
        {
            signAlgorithm = TM_CERTTYPE_DSS_SIGN;
        }
        else if (pkiType == TM_PKI_OBJ_RSA    ||
                pkiType == TM_PKI_OBJ_RSAMD5 ||
                pkiType == TM_PKI_OBJ_RSASHA1||
                pkiType == TM_PKI_OBJ_RSASHA256||
                pkiType == TM_PKI_OBJ_RSARIPEMD)
        {
            signAlgorithm = TM_CERTTYPE_RSA_SIGN;
        }
    }

    fieldLength = (tt16Bit) (*dataPtr);
    fieldLength++;
    for (i = 1; (tt16Bit)i < fieldLength; i ++)
    {
        if (*(dataPtr + i) == signAlgorithm)
        {
            creqMatch = TM_8BIT_YES;
            break;
        }
    }

    if (creqMatch == TM_8BIT_NO)
    {
        goto sslParseCertRequestExit;
    }
/*2 Parse the CA lists, we need to have certificate to at
 *   least one of these CAs
 */
    if (fieldLength > msgSize)
    {
        errorCode = TM_EINVAL;
        goto sslParseCertRequestExit;
    }
    msgSize = (tt16Bit) (msgSize - fieldLength);
    dataPtr += fieldLength;
    if (msgSize < 2)
    {
        errorCode = TM_EINVAL;
        goto sslParseCertRequestExit;
    }

/* Server support DigestAlgorithm */
#ifdef TM_USE_SSL_VERSION_33
    if (connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
        fieldLength = (tt16Bit)(((*dataPtr) << 8) + *(dataPtr + 1));
        if ((fieldLength == 0) || (fieldLength % 2))
        {
            errorCode = TM_EINVAL;
            goto sslParseCertRequestExit;
        }
        dataPtr += 2;
        msgSize -= 2;

/* Read SupportAlgorithm */
        for (i = 0; i < fieldLength / 2; i++)
        {
            hashAlg[i] = *(dataPtr + i * 2);
            signAlg[i] = *(dataPtr + i * 2 + 1);
        }
        suppCertListPtr = certListPtr;
/* Select Certificate that matches server request */
        while (suppCertListPtr)
        {
            creqMatch = TM_8BIT_NO;
            switch (suppCertListPtr->certDigestIndex)
            {
                case TM_PKI_OBJ_RSAMD5 :
                    certHash = TM_SSL_SUPPORT_HASH_MD5;
                    certSign = TM_SSL_SUPPORT_SIG_RSA;
                    break;
                case TM_PKI_OBJ_RSASHA1 :
                    certHash = TM_SSL_SUPPORT_HASH_SHA1;
                    certSign = TM_SSL_SUPPORT_SIG_RSA;
                    break;
                case TM_PKI_OBJ_RSASHA256 :
                    certHash = TM_SSL_SUPPORT_HASH_SHA256;
                    certSign = TM_SSL_SUPPORT_SIG_RSA;
                    break;
                case TM_PKI_OBJ_DSASHA1 :
                    certHash = TM_SSL_SUPPORT_HASH_SHA1;
                    certSign = TM_SSL_SUPPORT_SIG_DSA;
                    break;
                case TM_PKI_OBJ_RSA :
                case TM_PKI_OBJ_RSAMD2 :
                case TM_PKI_OBJ_RSASHA384 :
                case TM_PKI_OBJ_RSASHA512 :
                case TM_PKI_OBJ_RSARIPEMD :
                case TM_PKI_OBJ_DSA :
                default :
                    certHash = (tt8Bit)255;
                    certSign = (tt8Bit)255;
                    break;
            }
            for (i = 0; i < fieldLength / 2; i++)
            {
                if ((certHash == hashAlg[i])
                  &&(certSign == signAlg[i]))
                {
                    creqMatch = TM_8BIT_YES;
                    break;
                }
            }
            if(creqMatch == TM_8BIT_YES)
            {
                break;
            }
            else
            {
                suppCertListPtr = suppCertListPtr->certLinkNextPtr;
            }
        }
        if (creqMatch == TM_8BIT_YES)
        {
            connStatePtr->scsSessionPtr->ssCertSuppPtr = suppCertListPtr;
        }
        else
        {
            errorCode = TM_EINVAL;
            goto sslParseCertRequestExit;
        }
        dataPtr = dataPtr + fieldLength;
        msgSize = msgSize - fieldLength;
    }
#endif /* TM_USE_SSL_VERSION_33 */

    dataPtr += 2;
    msgSize -= 2;
/* if (msgSize == 0, it means that the server didn't list any preferred
 * root CA, in this case, we just send our certificate.
 */
    if (msgSize == 0)
    {
        creqMatch = TM_8BIT_YES;
    }
    else
    {
        creqMatch = TM_8BIT_NO;
        while (certListPtr->certSslNextPtr)
        {
            certListPtr = certListPtr->certSslNextPtr;
        }
/* certListPtr points to the direct neighbor to root CA */
        if (certListPtr->certCAPtr != (ttVoidPtr)0)
        {
            certListPtr = certListPtr->certCAPtr;
        }
        while (msgSize)
        {
            fieldLength = (tt16Bit)(((*dataPtr) << 8) + *(dataPtr + 1));

            if (fieldLength == certListPtr->certDNLength)
            {
                cmp = tm_memcmp(dataPtr + 2, certListPtr->certDNPtr,
                                fieldLength);
                if (cmp == 0)
                {
                    creqMatch = TM_8BIT_YES;
                    break;
                }
            }
            fieldLength = (tt16Bit)(fieldLength + 2);
            if (msgSize < fieldLength)
            {
                errorCode = TM_EINVAL;
                break;
            }
            msgSize = (tt16Bit)(msgSize - fieldLength);
            dataPtr += fieldLength;
        }
    }

sslParseCertRequestExit:
    if (errorCode == TM_ENOERROR)
    {
        if (creqMatch == TM_8BIT_NO)
        {
            errorCode = TM_SSL_ALT_NO_CERTIFICATE;
        }
        else
        {
            connStatePtr->scsRunFlags |= TM_SSL_RUNFLAG_CLIENT_SENDCERT;
        }
    }
    return errorCode;
}
#ifdef TM_PUBKEY_USE_RSA
/*
 * parses the server key, and write into asn.1 formst
 */
static int tfSslParseKeyIntoAsn1(tt8BitPtrPtr        dataPtrPtr,
                                 ttAsn1IntegerPtr    asnIntPtr)
{
    tt8BitPtr    dataPtr;
    int          errorCode;
    tt16Bit      fieldLength;

    errorCode = TM_ENOERROR;
    dataPtr = *dataPtrPtr;

    fieldLength = (tt16Bit)((*dataPtr << 8) + *(dataPtr + 1)) ;
    dataPtr += 2;
    asnIntPtr->asn1Type = TM_ASN1_TAG_INTEGER;
    asnIntPtr->asn1Length = fieldLength;
    asnIntPtr->asn1Data = tm_get_raw_buffer(fieldLength);
    if (!asnIntPtr->asn1Data)
    {
        errorCode = TM_ENOBUFS;
    }
    else
    {
        tm_bcopy(dataPtr, asnIntPtr->asn1Data, fieldLength);
        dataPtr += fieldLength;
    }

    *dataPtrPtr = dataPtr;
    return errorCode;
}
#endif /* TM_PUBKEY_USE_RSA */

/*
 * parses the server key exchange handshake message
 */
static int tfSslParseServerKeyExchange(ttSslConnectStatePtr    connStatePtr,
                                       tt8BitPtr               dataPtr,
                                       tt16Bit                 msgSize)
{
    ttCryptoRequest           request;
#ifdef TM_PUBKEY_USE_RSA
    ttGeneralRsaPtr           rsaPtr;
    ttCryptoRequestRsarequest rsareq;
#endif /* TM_PUBKEY_USE_RSA */
#ifdef TM_PUBKEY_USE_DSA
    ttCryptoRequestDsarequest dsareq;
#endif /* TM_PUBKEY_USE_DSA */

#ifdef TM_SSL_USE_EPHEMERAL_DH
    ttDiffieHellmanPtr        dhInfoPtr;
    tt16Bit                   fieldLength;
#endif /*TM_SSL_USE_EPHEMERAL_DH */
    ttMd5CtxPtr               md5CtxPtr;
    ttSha1CtxPtr              sha1CtxPtr;
    ttSha256CtxPtr            sha256CtxPtr;
    tt8BitPtr                 keBeginPtr;
    tt8BitPtr                 digestOutPtr;
    int                       errorCode;
#if (defined(TM_PUBKEY_USE_RSA) || defined(TM_PUBKEY_USE_DSA))
    int                       signLength;
    tt16Bit                   sigLen;
#endif /* TM_PUBKEY_USE_RSA || TM_PUBKEY_USE_DSA */
    tt16Bit                   keLength;
    tt8Bit                    signAlgorithm;
#ifdef TM_USE_SSL_VERSION_33
    tt8Bit                    hashAlgorithm;
    tt8Bit                    hashAlg;
    tt8Bit                    sigAlg;
#endif /* TM_USE_SSL_VERSION_33 */

    tm_trace_func_entry(tfSslParseServerKeyExchange);
    tm_debug_log1("tfSslParseServerKeyExchange sd=%u",
          connStatePtr->scsTcpVectPtr->tcpsSocketEntry.socIndex);
#ifdef TM_LOG_SSL_DEBUG
    tfEnhancedLogWrite(
            TM_LOG_MODULE_SSL,
            TM_LOG_LEVEL_DEBUG,
            "tfSslParseServerKeyExchange: sd=%u",
            connStatePtr->scsTcpVectPtr->tcpsSocketEntry.socIndex);
#endif /* TM_LOG_SSL_DEBUG */

    errorCode = TM_ENOERROR;

#if (defined(TM_PUBKEY_USE_RSA) || defined(TM_PUBKEY_USE_DSA))
    sigLen = 0;
#endif /* TM_PUBKEY_USE_RSA || TM_PUBKEY_USE_DSA */
#ifdef TM_PUBKEY_USE_RSA
    rsaPtr = (ttGeneralRsaPtr)0;
#endif /* TM_PUBKEY_USE_RSA */
    keBeginPtr = dataPtr;
    keLength = 0;
    signAlgorithm = 0;
    TM_UNREF_IN_ARG(msgSize);
    md5CtxPtr = (ttMd5CtxPtr)0;
    sha1CtxPtr = (ttSha1CtxPtr)0;
    sha256CtxPtr = (ttSha256CtxPtr)0;
    digestOutPtr = (tt8BitPtr)0;
#ifdef TM_USE_SSL_VERSION_33
    hashAlgorithm = 0;
#endif /* TM_USE_SSL_VERSION_33 */

#ifdef TM_PUBKEY_USE_RSA
    if (connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_SERVER_SENDERSA)
    {
        rsaPtr = tm_get_raw_buffer(sizeof(ttGeneralRsa));
        if (!rsaPtr)
        {
            errorCode = TM_ENOBUFS;
            goto sslParseServerKeExit;
        }
        connStatePtr->scsEphemRsaPtr = rsaPtr;
        tm_bzero(rsaPtr, sizeof(ttGeneralRsa));
        rsaPtr->rsaN = tm_get_raw_buffer(sizeof(ttAsn1Integer));
        rsaPtr->rsaE = tm_get_raw_buffer(sizeof(ttAsn1Integer));
        if ((!rsaPtr->rsaN) || (!rsaPtr->rsaE))
        {
            errorCode = TM_ENOBUFS;
            goto sslParseServerKeExit;
        }
/* parsing the modulus, i.e. rsaPtr->rsaN */
        errorCode = tfSslParseKeyIntoAsn1(&dataPtr,
                                          rsaPtr->rsaN);
        if (errorCode != TM_ENOERROR)
        {
            goto sslParseServerKeExit;
        }
        keLength = (tt16Bit)(keLength + 2 + rsaPtr->rsaN->asn1Length);
/* parsing the exponent, i.e. rsaPtr->rsaE */
        errorCode = tfSslParseKeyIntoAsn1(&dataPtr,
                                          rsaPtr->rsaE);
        if (errorCode != TM_ENOERROR)
        {
            goto sslParseServerKeExit;
        }
        keLength = (tt16Bit)(keLength + 2 + rsaPtr->rsaE->asn1Length);
    }
#endif /* TM_PUBKEY_USE_RSA */

#ifdef TM_SSL_USE_EPHEMERAL_DH
    if (connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_SERVER_SENDEDH)
    {
        connStatePtr->scsDHInfoPtr = tm_get_raw_buffer
                    (sizeof(ttDiffieHellman));
        if (!connStatePtr->scsDHInfoPtr)
        {
            errorCode = TM_ENOBUFS;
            goto sslParseServerKeExit;
        }
        dhInfoPtr = connStatePtr->scsDHInfoPtr;
        tm_bzero(dhInfoPtr, sizeof(*dhInfoPtr));
/* parsing the prime */
        dhInfoPtr->dhGroupLen = (tt16Bit)((*(dataPtr) << 8) + *(dataPtr + 1));
        dataPtr += 2;
        dhInfoPtr->dhPrimePtr = tm_get_raw_buffer(dhInfoPtr->dhGroupLen);
        if (!dhInfoPtr->dhPrimePtr)
        {
            errorCode = TM_ENOBUFS;
            goto sslParseServerKeExit;
        }
        tm_bcopy(dataPtr, dhInfoPtr->dhPrimePtr,dhInfoPtr->dhGroupLen);
        dataPtr += dhInfoPtr->dhGroupLen;
        keLength = (tt16Bit)(keLength + 2 + dhInfoPtr->dhGroupLen);
/* now parse the generator. We only accept generator less or equal to
 * four octets. Common practice is only two BITS value, eg 2.
 */
        fieldLength = (tt16Bit)((*dataPtr << 8) + *(dataPtr + 1)) ;
        dataPtr += 2;
        if (fieldLength > 4)
        {
/* to connect Windows */
            if ((*(dataPtr + fieldLength - 1) <= (tt8Bit)4)
              &&(*(dataPtr + fieldLength - 1) != (tt8Bit)0))
            {
                dataPtr = dataPtr + fieldLength - 1;
                keLength = (tt16Bit)(keLength + fieldLength - 1);
                fieldLength = (tt8Bit)1;
            }
            else
            {
                errorCode = TM_EINVAL;
                goto sslParseServerKeExit;
            }
        }
        switch(fieldLength)
        {
        case 1:
            dhInfoPtr->dhGenerator = *dataPtr;
            break;
        case 2:
            dhInfoPtr->dhGenerator = (*dataPtr << 8) + *(dataPtr + 1);
            break;
        case 3:
            dhInfoPtr->dhGenerator = (*dataPtr << 16) +
                                     (*(dataPtr + 1) << 8) +
                                     *(dataPtr + 2);
            break;
        case 4:
            dhInfoPtr->dhGenerator = (*dataPtr << 24) +
                                     (*(dataPtr + 1) << 16) +
                                     (*(dataPtr + 2) << 8) +
                                     (*(dataPtr + 3));
            break;
        default:
            errorCode = TM_EINVAL;
            goto sslParseServerKeExit;
        }
        dataPtr += fieldLength;
        keLength = (tt16Bit)(keLength + 2 + fieldLength);
/* parsing the server's public key */
        fieldLength = (tt16Bit)((*dataPtr << 8) + *(dataPtr + 1)) ;
        dataPtr += 2;
        if (fieldLength > dhInfoPtr->dhGroupLen)
        {
/* the public value can't be bigger than prime length */
            errorCode = TM_EINVAL;
            goto sslParseServerKeExit;
        }
        dhInfoPtr->dhGxrPtr = tm_get_raw_buffer(fieldLength);
        if (!dhInfoPtr->dhGxrPtr)
        {
            errorCode = TM_ENOBUFS;
            goto sslParseServerKeExit;
        }
        dhInfoPtr->dhGxrLen = fieldLength;
        tm_bcopy(dataPtr, dhInfoPtr->dhGxrPtr,fieldLength);
        dataPtr += fieldLength;
        keLength = (tt16Bit)(keLength + 2 + fieldLength);
    }
#endif /*TM_SSL_USE_EPHEMERAL_DH */

/*2. Verify the signature, dataPtr now points to the signature part */

#ifdef TM_USE_SSL_VERSION_33
    if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
/* Hash */
        hashAlg = *dataPtr;
        switch(hashAlg)
        {
/* Support default only(SHA-1) */
            case TM_SSL_SUPPORT_HASH_SHA1 :
                hashAlgorithm = SADB_AALG_SHA1HMAC;
                break;
            case TM_SSL_SUPPORT_HASH_SHA256 :
                hashAlgorithm = SADB_AALG_SHA256HMAC;
                break;
            default :
                errorCode = TM_EOPNOTSUPP;
                break;
        }
/* Signature */
        sigAlg = *(dataPtr + 1);
        switch(sigAlg)
        {
            case TM_SSL_SUPPORT_SIG_RSA :
                signAlgorithm = SADB_PUBKEY_RSA;
                break;
            case TM_SSL_SUPPORT_SIG_DSA :
                signAlgorithm = SADB_PUBKEY_DSA;
                break;
            default :
                errorCode = TM_EOPNOTSUPP;
                break;
        }
        if(errorCode != TM_ENOERROR)
        {
            goto sslParseServerKeExit;
        }
        dataPtr = dataPtr + 2;
    }
    else
#endif /* TM_USE_SSL_VERSION_33 */
    {
        signAlgorithm = connStatePtr->scsCipherPtr
                    ->csSignAlgorithm;
    }
    tm_bzero(&request, sizeof(request));

    if (*((tt16BitPtr)dataPtr) == 0)
    {
/* we hack code here to handle some server. They may not build the opache length
 * field, instead, they send signature directly
 */
        dataPtr = dataPtr + 2;
    }
#if (defined(TM_PUBKEY_USE_RSA) || defined(TM_PUBKEY_USE_DSA))
    else
    {
        sigLen = *dataPtr++ << 8;
        sigLen = sigLen + (tt16Bit)*dataPtr++;
    }
#endif /* TM_PUBKEY_USE_RSA || TM_PUBKEY_USE_DSA */

#ifdef TM_USE_SSL_VERSION_33
    if (connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
        if (hashAlgorithm == SADB_AALG_SHA1HMAC)
        {
            sha1CtxPtr = tm_get_raw_buffer(sizeof(ttSha1Ctx));
            digestOutPtr = tm_get_raw_buffer(TM_SHA1_HASH_SIZE );
            if ((!sha1CtxPtr)|| (!digestOutPtr))
            {
                errorCode = TM_ENOBUFS;
                goto sslParseServerKeExit;
            }
            tfSha1Init(sha1CtxPtr);
        }
        else if (hashAlgorithm == SADB_AALG_SHA256HMAC)
        {
            sha256CtxPtr = tm_get_raw_buffer(sizeof(ttSha256Ctx));
            digestOutPtr = tm_get_raw_buffer(TM_SHA256_HASH_SIZE ); 
            if ((!sha256CtxPtr)|| (!digestOutPtr))
            {
                errorCode = TM_ENOBUFS;
                goto sslParseServerKeExit;
            }
            tfSha256Init(sha256CtxPtr);
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
            goto sslParseServerKeExit;
        }
        tfMd5Init(md5CtxPtr);
        tfSha1Init(sha1CtxPtr);
    }

#ifdef TM_USE_SSL_VERSION_33
    if (connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
        if(hashAlgorithm == SADB_AALG_SHA1HMAC)
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
        else if (hashAlgorithm == SADB_AALG_SHA256HMAC)
        {
            tfSha256Update(sha256CtxPtr, connStatePtr->scsClientRandom,
                TM_SSL_RANDOM_SIZE, 0);
            tfSha256Update(sha256CtxPtr, connStatePtr->scsServerRandom,
                TM_SSL_RANDOM_SIZE, 0);
            tfSha256Update(sha256CtxPtr, keBeginPtr, (ttPktLen)keLength, 0);
            tfSha256Final(digestOutPtr, sha256CtxPtr);
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
            tfMd5Update(md5CtxPtr,connStatePtr->scsClientRandom,
                        TM_SSL_RANDOM_SIZE, 0);
            tfMd5Update(md5CtxPtr,connStatePtr->scsServerRandom,
                        TM_SSL_RANDOM_SIZE, 0);
            tfMd5Update(md5CtxPtr,keBeginPtr,keLength,0);
            tfMd5Final(digestOutPtr,md5CtxPtr);
        }

        tfSha1Update(sha1CtxPtr,connStatePtr->scsClientRandom,
                     TM_SSL_RANDOM_SIZE, 0);
        tfSha1Update(sha1CtxPtr,connStatePtr->scsServerRandom,
                     TM_SSL_RANDOM_SIZE, 0);
        tfSha1Update(sha1CtxPtr,keBeginPtr,keLength,0);
        tfSha1Final(digestOutPtr + TM_MD5_HASH_SIZE,sha1CtxPtr);
    }

#ifdef TM_PUBKEY_USE_RSA
    if (signAlgorithm == SADB_PUBKEY_RSA)
    {
#ifdef TM_USE_SSL_VERSION_33
        if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
        {
            errorCode = tfSslRsaPkcsVerify(connStatePtr->scsPeerCertListPtr->certKeyPtr,
                                           dataPtr,
                                           digestOutPtr,
                                           sigLen);
        }
        else
#endif /* TM_USE_SSL_VERSION_33 */
        {
            request.crType          = TM_CEREQUEST_PUBKEY_RSAVERIFY;
            request.crParamUnion.crRsaParamPtr   = &rsareq;
            rsareq.crrRsainfoPtr    = (ttGeneralRsaPtr)
                connStatePtr->scsPeerCertListPtr->certKeyPtr->pkKeyPtr;
            signLength = (tt16Bit)rsareq.crrRsainfoPtr->rsaN->asn1Length;
            if ((sigLen != 0)
                && (sigLen != signLength))
            {
                errorCode = -1;
                goto sslParseServerKeExit;
            }
            rsareq.crrRsaDataLength = TM_MD5_HASH_SIZE + TM_SHA1_HASH_SIZE;
            rsareq.crrRsaDataPtr    = digestOutPtr;
            rsareq.crrRsaTransPtr   = dataPtr;
            rsareq.crrRsaTransLenPtr = &signLength;
            connStatePtr->scsPubkeyEnginePtr->ceSessionProcessFuncPtr(&request);
            errorCode = request.crResult;
        }
    }
#endif /* TM_PUBKEY_USE_RSA */

#ifdef TM_PUBKEY_USE_DSA
    if (signAlgorithm == SADB_PUBKEY_DSA)
    {
        signLength = *(dataPtr + 1) + 2;
/* the format is something like 30 2d 02 14, 0x2d+2 is the DSA signature
 * length
 */
        request.crType          = TM_CEREQUEST_PUBKEY_DSAVERIFY;
        request.crParamUnion.crDsaParamPtr   = &dsareq;
        dsareq.crdDsainfoPtr    = (ttGeneralDsaPtr)
            connStatePtr->scsPeerCertListPtr->certKeyPtr->pkKeyPtr;
        dsareq.crdDsaDataLength = TM_SHA1_HASH_SIZE;
#ifdef TM_USE_SSL_VERSION_33
        if(connStatePtr->scsVersion == TM_SSL_VERSION_33)
        {
            if (hashAlgorithm == SADB_AALG_SHA256HMAC)
            {
                dsareq.crdDsaDataLength = TM_SHA256_HASH_SIZE;
            }
            dsareq.crdDsaDataPtr    = digestOutPtr;
        }
        else
#endif /* TM_USE_SSL_VERSION_33 */
        {
            dsareq.crdDsaDataPtr    = digestOutPtr + TM_MD5_HASH_SIZE;
        }
        dsareq.crdDsaTransPtr   = dataPtr;
        dsareq.crdDsaTransLenPtr = &signLength;
        connStatePtr->scsPubkeyEnginePtr->ceSessionProcessFuncPtr(&request);
        errorCode = request.crResult;
    }
#endif /* TM_PUBKEY_USE_DSA */


sslParseServerKeExit:
    if (digestOutPtr)
    {
        tm_free_raw_buffer(digestOutPtr);
    }
    if (md5CtxPtr)
    {
        tm_free_raw_buffer((ttRawBufferPtr)md5CtxPtr);
    }
    if (sha1CtxPtr)
    {
        tm_free_raw_buffer((ttRawBufferPtr)sha1CtxPtr);
    }
    if (sha256CtxPtr)
    {
        tm_free_raw_buffer((ttRawBufferPtr)sha256CtxPtr);
    }

    tm_trace_func_exit_rc(tfSslParseServerKeyExchange, errorCode);
    return errorCode;
}


/* in this case, we will get the proposals from
 * tlSslProposalSuites, and set them here one by one */
static int tfSslGetProposals(ttSslConnectStatePtr   connStatePtr,
                             tt8BitPtr              dataPtr)
{
    tt16BitPtr   b16Ptr;
    tt16BitPtr   lengthPtr;
    int          length;
    int          i;
    tt16Bit      prop;

    length = 0;
/* the cipher suite length field is on 4-byte boundary */
    b16Ptr = (tt16BitPtr)dataPtr;
    lengthPtr = b16Ptr ++;

    for (i=0; i < TM_SSL_PROPOSAL_NUMBER; i ++)
    {
        prop = connStatePtr->scsSessionPtr->ssProposals[i];
        if (prop > 0)
        {
#ifdef TM_USE_SSL_VERSION_33
            if ((connStatePtr->scsSessionPtr->ssVersion)
               & TM_SSL_VERSION_33)
            {
                switch(prop)
                {
#ifndef TM_SSL_REJECT_EXPORTABLE_CIPHER
                    case TM_TLS_RSA_EXPORT_RC4_40_MD5:
                    case TM_TLS_RSA_EXPORT_RC2_40_MD5:
                    case TM_TLS_RSA_EXPORT_DES40_CBC_SHA:
                    case TM_TLS_DHE_DSS_EPT_DES40_CBC_SHA:
                    case TM_TLS_DHE_RSA_EPT_DES40_CBC_SHA:
                    case TM_TLS_RSA_EPT1K_DES_CBC_SHA:
                    case TM_TLS_DHE_DSS_EPT1K_DES_CBC_SHA:
                    case TM_TLS_RSA_EPT1K_RC4_56_SHA:
                    case TM_TLS_DHE_DSS_EPT1K_RC4_56_SHA:
#endif /* TM_SSL_REJECT_EXPORTABLE_CIPHER */
#ifndef TM_EXPORT_RESTRICT
                    case TM_TLS_RSA_DES_CBC_SHA:
#ifdef TM_SSL_USE_EPHEMERAL_DH
                    case TM_TLS_DHE_DSS_DES_CBC_SHA:
                    case TM_TLS_DHE_RSA_DES_CBC_SHA:
#endif /* TM_SSL_USE_EPHEMERAL_DH */
/* we don't support these algorithms */
#endif /* TM_EXPORT_RESTRICT */
                        break;
                    default:
                        length ++; /* number of effective proposals */
                        *b16Ptr++ = htons(prop);
                        break;
                }
            }
            else
#endif /* TM_USE_SSL_VERSION_33 */
            {
                length ++; /* number of effective proposals */
                *b16Ptr++ = htons(prop);
            }
        }
    }
    length = (int)sizeof(tt16Bit) * length;
    *lengthPtr = htons(length);
    length += 2; /* the total cipher suite length */
/* only compress method is NO_COMPRESS */
    dataPtr = (tt8BitPtr)b16Ptr;
    *dataPtr++ = 1; /* compress length */
    *dataPtr = TM_SSL_COMPMETH_NULL;

    length += 2; /* compress length + method */
    return length;
}


/* used by client only. For a server to find a cache element, it
 * uses session id as the key. For a client to find a cache, it
 * uses server's address as the key
 */

static ttSslCacheElementPtr tfSslClientFindCache
                         (ttSslConnectStatePtr          connStatePtr)
{
    ttSslSessionPtr       sslSessionPtr;
    ttIpAddressPtr        peerAddrPtr;
    ttSslCacheElementPtr  cachePtr;
    int                   i;
    int                   isEqual;

    cachePtr = 0;
    sslSessionPtr = connStatePtr->scsSessionPtr;
    peerAddrPtr = &(connStatePtr->scsTcpVectPtr->tcpsSocketEntry.
                  socPeerIpAddress);
    for (i=0; (tt16Bit)i < sslSessionPtr->ssTotalCache; i++)
    {
        cachePtr = (ttSslCacheElementPtr)(sslSessionPtr->ssConnectCache[i]);
        if (        cachePtr
                 &&  ((cachePtr->scePeerPort == connStatePtr->scsTcpVectPtr
                                  ->tcpsSocketEntry.socPeerLayer4Port)
                   || (tm_16bit_one_bit_set(sslSessionPtr->ssOption,
                                            TM_SSL_OPT_CLIENT_CACHEID)))
           )
        {
            isEqual = tm_memcmp(&cachePtr->scePeerAddr,
                                   peerAddrPtr,
                                   sizeof(ttIpAddress));
            if (!isEqual)
            {
                break;
            }
        }
    }
    return cachePtr;
}


/* Description
 * This function tfSslClientUserStart is valid only if TM_USE_SSL_CLIENT
 * is defined. It is called by user in order to start the SSL client.
 * Before this call, user should set proper socket options on socket
 * socketDescriptor. See section 4.16 set socket options for details.
 *
 *
 * Parameters
 * socketDescriptor         socket number you want to start SSL
 *                             negotiation
 * Returns
 * Value            Meaning
 * TM_ENOERROR      Success
 * -1               Error happens, user needs to call tfGetSocketError
 *                  to get the related socket error code
 *                  TM_ENOBUFS       No more buffer available
 *                  TM_EPERM         Function call not permitted, check if
 *                                   TM_TCP_SSLSESSION and TM_TCP_SSL_CLIENT
 *                                   are both set on this socket
 *                  TM_EPROTONOTSUPPORT The socket is not TCP socket
 *                  TM_EBADF         Socket pointer is not found
 */

int tfSslClientUserStart(int            socketDescriptor,
                         char TM_FAR *  serverCommonName)
{

    ttSocketEntryPtr          socketEntryPtr;
    ttSslCacheElementPtr      sslCacheItemPtr;
    ttSslConnectStatePtr      connStatePtr;
    ttTcpVectPtr              tcpVectPtr;
    ttSslSessionPtr           sslSessionPtr;
    int                       errorCode;
#ifdef TM_USE_BSD_DOMAIN
    int                       af;
#endif /* TM_USE_BSD_DOMAIN */

    errorCode = TM_ENOERROR;
    connStatePtr = (ttSslConnectStatePtr)0;

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
        goto sslClientUserStartExit;
    }
/* make sure it is a TCP socket */
    if (socketEntryPtr->socProtocolNumber != (tt8Bit)IP_PROTOTCP)
    {
        errorCode = TM_EPROTONOSUPPORT;
        goto sslClientUserStartExit;
    }
    tcpVectPtr = (ttTcpVectPtr)((ttVoidPtr)socketEntryPtr);

/* make sure that we have set the SSL session*/
    if (tm_16bit_bits_not_set(tcpVectPtr->tcpsSslFlags, TM_SSLF_SESSION_SET))
    {
        errorCode = TM_EPERM;
        goto sslClientUserStartExit;
    }
/* User should wait for TCP connection */
    if (   (tcpVectPtr->tcpsState != TM_TCPS_ESTABLISHED)
        && (tcpVectPtr->tcpsState != TM_TCPS_CLOSE_WAIT) )
    {
        errorCode = TM_ENOTCONN;
        goto sslClientUserStartExit;
    }
/* if SSL client is defined and this socket is SSL_enabled, we
 * trigger ssl statemachine
 */
    if (tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                            TM_SSLF_CLIENT))
    {
/* The connect state is allocated when you set the socket options either
 * TM_TCP_SSL_CLIENT or TM_TCP_SSL_SERVER
 */
        connStatePtr = (ttSslConnectStatePtr)tcpVectPtr->tcpsSslConnStatePtr;
/* clear stuff of previous connection */
        tfSslPrefreeConnState(connStatePtr);
        if (serverCommonName != (char TM_FAR *)0)
        {
            connStatePtr->scsServerCNPtr = tm_get_raw_buffer
                ((ttPktLen) (tm_strlen(serverCommonName) + 1));

            if (!connStatePtr->scsServerCNPtr)
            {
                errorCode = TM_ENOBUFS;
                goto sslClientUserStartExit;
            }
            tm_strcpy(connStatePtr->scsServerCNPtr, serverCommonName);
        }
/* Get a locked session using session ID preset by the user */
        sslSessionPtr = (ttSslSessionPtr)
                tfSslGetSessionUsingIndex(tcpVectPtr->tcpsSslSession);
        if (sslSessionPtr)
        {
            connStatePtr->scsSessionPtr = sslSessionPtr;
            connStatePtr->scsSocIndex = (unsigned int)socketDescriptor;
            connStatePtr->scsTcpVectPtr = tcpVectPtr;
            connStatePtr->scsState = TM_SSL_CT_INITIAL;
            connStatePtr->scsClientOrServer = TM_SSL_CLIENT;
/* Copy version from session. What we support. */
            connStatePtr->scsSsVersion = connStatePtr->scsSessionPtr->ssVersion;
/* To be changed later during negotiation. */
            connStatePtr->scsVersion = connStatePtr->scsSsVersion;
            connStatePtr->scsMd5CtxPtr = tm_get_raw_buffer(sizeof(ttMd5Ctx));
            connStatePtr->scsSha1CtxPtr = tm_get_raw_buffer(sizeof(ttSha1Ctx));
#ifdef TM_USE_SSL_VERSION_33
            connStatePtr->scsSha256CtxPtr = tm_get_raw_buffer(sizeof(ttSha256Ctx));
#endif /* TM_USE_SSL_VERSION_33 */

            if (connStatePtr->scsMd5CtxPtr && connStatePtr->scsSha1CtxPtr
#ifdef TM_USE_SSL_VERSION_33
             && connStatePtr->scsSha256CtxPtr
#endif /* TM_USE_SSL_VERSION_33 */
             )
            {
                sslCacheItemPtr = tfSslClientFindCache(connStatePtr);
                errorCode = tfSslClientStateMachine(connStatePtr,
                                                    sslCacheItemPtr,
                                                    TM_SSL_CLIENT_START);
/*
 * call unlock instead of tfSslSessionReturn() to keep the owner count
 * increased by 1 because the connection state has a back pointer to the
 * session.
 */
/* Unlock session */
                tm_call_unlock(&sslSessionPtr->ssLockEntry);
/* We should set SEND_ENABLED and RECV_ENABLED after we successfully
 * call tfTcpSendPacket. However, for automatic testing, we back and
 * forth and exchange messages but we never set these two enable
 * flags. A Hacking code is just set SEND and RECEIVE enable before
 * we send out anything.
 */
                tcpVectPtr->tcpsSslFlags |= TM_SSLF_SEND_ENABLED |
                                            TM_SSLF_RECV_ENABLED;
/* we move this line before tfTcpSendPacket, because in automatic testing,
 * there is no lock protection, the inside loop of tfTcpSendPacket maybe
 * try to free the connStatePtr.
 * Note: tfTcpSendPacket() always returns TM_ENOERROR.
 */
                errorCode = tfTcpSendPacket(tcpVectPtr);
            }
            else
            {
/* Unlock session */
                tm_call_unlock(&sslSessionPtr->ssLockEntry);
                errorCode = TM_ENOBUFS;
            }
        }
        else
        {
            errorCode = TM_ENOENT;
        }
        if (errorCode != TM_ENOERROR)
        {
/*
 * Will remove back pointer to session, decrease session owner count,
 * and free all buffers allocated in this routine.
 */
            tfSslPrefreeConnState(connStatePtr);
        }
    }
    else
    {
        errorCode = TM_EPERM;
    }
sslClientUserStartExit:
    return tfSocketReturn(socketEntryPtr,
                          socketDescriptor,
                          errorCode,
                          TM_ENOERROR);
}

/*
 * build the server hello handshake message
 */
static int tfSslSendClientHello(ttSslConnectStatePtr    connStatePtr,
                                ttVoidPtr               cachedItemPtr)
{
    ttSslCacheElementPtr     cachedElePtr;
    ttSslRecordHeaderPtr     sslHeaderPtr;
    ttPacketPtr              packetPtr;
    tt8BitPtr                sessionIdPtr;
    ttSslHandShakeHeaderPtr  handshakeHeaderPtr;
    ttClientHelloPtr         clientHelloPtr;
    tt8BitPtr                dataPtr;
    ttPktLen                 clientHelloLength;
    int                      errorCode;
    tt16Bit                  dataLength;
    tt8Bit                   sessionIdLength;
    tt8Bit                   versionMinor;
#ifdef TM_USE_SSL_VERSION_33
    tt8BitPtr                extensionLengthPtr;
    tt8BitPtr                extensionHeadPtr;
    tt16Bit                  extensionLength;
#endif /* TM_USE_SSL_VERSION_33 */

    errorCode = TM_ENOERROR;
    sessionIdLength = 0;
    packetPtr = (ttPacketPtr)0;
    cachedElePtr = (ttSslCacheElementPtr)cachedItemPtr;

    if (connStatePtr->scsSsVersion & TM_SSL_VERSION_33)
    {
        versionMinor = TM_SSL_VERSION_MINOR3;
    }
    else if (connStatePtr->scsSsVersion & TM_SSL_VERSION_31)
    {
        versionMinor = TM_SSL_VERSION_MINOR1;
    }
    else if (connStatePtr->scsSsVersion & TM_SSL_VERSION_30)
    {
        versionMinor = TM_SSL_VERSION_MINOR0;
    }
    else
    {
        errorCode = TM_EINVAL;
        goto sslSendClientHelloExit;
    }
    dataLength = TM_SSL_CLIENT_HELLO_LENGTH;
#ifdef TM_USE_SSL_VERSION_33
    if(connStatePtr->scsVersion & TM_SSL_VERSION_33)
    {
        dataLength = (tt16Bit)(dataLength +
                               TM_SSL_HANDSHAKE_HEADER_SIZE +
                               16  /* Extension Data Size */);
    }
#endif /* TM_USE_SSL_VERSION_33 */
    if(connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_REHANDSHAKE)
    {
        dataLength = (tt16Bit)(dataLength + TM_SSL_RECORD_OVERHEAD_MAX_SIZE);
    }

    packetPtr = tfGetSharedBuffer(0, dataLength, 0);
    if (packetPtr == (ttPacketPtr)0)
    {
        errorCode = TM_ENOBUFS;
        goto sslSendClientHelloExit;
    }
    dataPtr = packetPtr->pktLinkDataPtr;

    if (cachedElePtr != (ttSslCacheElementPtr)0)
    {
        sessionIdPtr = cachedElePtr->sceSessionId;
        sessionIdLength = cachedElePtr->sceSessionIdLength;
    }
    else
    {
/* if HelloRequest message is received, we need to clear flag
 * TM_SSL_RUNFLAG_RESUMESESSION if it was set
 */
        connStatePtr->scsRunFlags &= ~(unsigned)( TM_SSL_RUNFLAG_RESUMESESSION
/* clear ESTABLISHED flag to prohibit sending */
                                     | TM_SSLF_ESTABLISHED);
        sessionIdPtr = (tt8BitPtr)0;
    }

    dataLength = 0;

/* construct a client hello message */
    sslHeaderPtr = (ttSslRecordHeaderPtr)dataPtr;
    sslHeaderPtr->srhType = TM_SSL_TYPE_HANDSHAKE;
    sslHeaderPtr->srhVersionMajor = TM_SSL_VERSION_MAJOR;
    sslHeaderPtr->srhVersionMinor = versionMinor;

    dataPtr += TM_SSL_RECORD_HEADER_SIZE;
    handshakeHeaderPtr = (ttSslHandShakeHeaderPtr) dataPtr;
    clientHelloPtr = (ttClientHelloPtr)((tt8BitPtr)handshakeHeaderPtr +
                   sizeof(ttSslHandShakeHeader));
    handshakeHeaderPtr->hsHandShakeType = TM_SSL_CLIENT_HELLO;
    handshakeHeaderPtr->hsLengthPad = 0;
    clientHelloPtr->chVersionMajor = TM_SSL_VERSION_MAJOR;
/* The minor version should be the highest in this session */
    if(connStatePtr->scsVersion & TM_SSL_VERSION_33)
    {
        clientHelloPtr->chVersionMinor = TM_SSL_VERSION_MINOR3;
    }
    else if (connStatePtr->scsVersion & TM_SSL_VERSION_31)
    {
        clientHelloPtr->chVersionMinor = TM_SSL_VERSION_MINOR1;
    }
    else if (connStatePtr->scsVersion & TM_SSL_VERSION_30)
    {
        clientHelloPtr->chVersionMinor = TM_SSL_VERSION_MINOR0;
    }
    else
    {
        errorCode = TM_EINVAL;
        goto sslSendClientHelloExit;
    }
/* we don't support GMT time format*/
    tfGetRandomBytes(clientHelloPtr->chRandom,
                     tm_packed_byte_count(TM_SSL_RANDOM_SIZE));

    tm_bcopy(clientHelloPtr->chRandom,
             connStatePtr->scsClientRandom,
             TM_SSL_RANDOM_SIZE);

    dataPtr += TM_SSL_HELLO_SESSIONID_OFFSET;
    if (sessionIdPtr != (tt8BitPtr)0)
    {
/* Resume a session */
        connStatePtr->scsSessionIdLength = sessionIdLength;
        clientHelloPtr->chSessionIdLength = sessionIdLength;
/* we are not going to save the cachedElePtr, thus, we need to copy the
 * sceMasterSecret, and cipher suites. Later, if server denies our
 * request, we just overwrite these value
 */
        tm_bcopy(cachedElePtr->sceMasterSecret,
                   connStatePtr->scsMasterSecret,
                   TM_SSL_MASTER_SECRET_SIZE);
        connStatePtr->scsCipherPtr = tfSslCipherSuiteLookup
                   (cachedElePtr->sceCipherSuite);

        tm_bcopy(sessionIdPtr,
                 connStatePtr->scsSessionId,
                 sessionIdLength);
        tm_bcopy(sessionIdPtr, dataPtr,
                 sessionIdLength);
        dataPtr += sessionIdLength;
        *(dataPtr++) = 0;
        *(dataPtr++) = 2; /* only one cipher suite */
        *((tt16BitPtr)(dataPtr)) = htons
                      (cachedElePtr->sceCipherSuite);
        dataPtr += 2;
        *(dataPtr++) = 1; /* only one compress method */
        *dataPtr++ = TM_SSL_COMPMETH_NULL;
/* dataPtr now points to the end of ClientHello*/
        dataLength = (tt16Bit)(TM_SSL_HELLO_SESSIONID_OFFSET
                      + sessionIdLength + 6);
    }
    else
    {
        clientHelloPtr->chSessionIdLength = 0;
        dataLength =
/* Call get proposals in order to set the cipher suites and compress method */
                (tt16Bit)tfSslGetProposals(connStatePtr, (tt8BitPtr)dataPtr);
        dataPtr += dataLength; /* dataPtr points to the end of ClientHello*/
        dataLength = (tt16Bit)(dataLength + TM_SSL_HELLO_SESSIONID_OFFSET);

    }

    tm_assert(tfSslSendClientHello, dataLength <= TM_SSL_CLIENT_HELLO_LENGTH);

#ifdef TM_USE_SSL_VERSION_33
    if (connStatePtr->scsVersion & TM_SSL_VERSION_33)
    {
/* Extension :*/
        extensionHeadPtr = dataPtr;
        dataPtr += 2; /* Extension Length field */

/* signature_algorithms */
        *dataPtr++ = (tt8Bit)0x00;
        *dataPtr++ = (tt8Bit)0x0d;
        extensionLengthPtr = dataPtr;
        dataPtr += 2; /* signature_algorithms Length field */
#ifdef TM_SSL_EXTENSION_SIG_LEN
        dataPtr += 2; /* Data Length field */
#endif /* TM_SSL_EXTENSION_SIG_LEN */

#ifdef TM_PUBKEY_USE_RSA
        *dataPtr++ = TM_SSL_SUPPORT_HASH_SHA256;
        *dataPtr++ = TM_SSL_SUPPORT_SIG_RSA;
        *dataPtr++ = TM_SSL_SUPPORT_HASH_SHA1;
        *dataPtr++ = TM_SSL_SUPPORT_SIG_RSA;
#endif /* TM_PUBKEY_USE_RSA */
#ifdef TM_PUBKEY_USE_DSA
        *dataPtr++ = TM_SSL_SUPPORT_HASH_SHA1;
        *dataPtr++ = TM_SSL_SUPPORT_SIG_DSA;
#endif /* TM_PUBKEY_USE_DSA */
#ifdef TM_PUBKEY_USE_RSA
        *dataPtr++ = TM_SSL_SUPPORT_HASH_MD5;
        *dataPtr++ = TM_SSL_SUPPORT_SIG_RSA;
#endif /* TM_PUBKEY_USE_RSA */

        extensionLength = (tt16Bit)(dataPtr - extensionLengthPtr - 2);
        *extensionLengthPtr ++ = (tt8Bit)(extensionLength >> 8);
        *extensionLengthPtr ++ = (tt8Bit)(extensionLength & 0xff);
#ifdef TM_SSL_EXTENSION_SIG_LEN
        *extensionLengthPtr ++ = (tt8Bit)((extensionLength - 2) >> 8);
        *extensionLengthPtr =  (tt8Bit)((extensionLength - 2) & 0xff);
#endif /* TM_SSL_EXTENSION_SIG_LEN */

        extensionLength = (tt16Bit)(dataPtr - extensionHeadPtr - 2);
        *extensionHeadPtr ++ = (tt8Bit)(extensionLength >> 8);
        *extensionHeadPtr = (tt8Bit)(extensionLength & 0xff);
        dataLength = (tt16Bit)(dataLength + extensionLength + 2);
    }
#endif /* TM_USE_SSL_VERSION_33 */

/* The ClientHello length doesn't count TM_SSL_HANDSHAKE_HEADER_SIZE */
    handshakeHeaderPtr->hsLengthHigh =  (tt8Bit)
                 ((dataLength - TM_SSL_HANDSHAKE_HEADER_SIZE) >> 8);
    handshakeHeaderPtr->hsLengthLow =  (tt8Bit)
                 ((dataLength - TM_SSL_HANDSHAKE_HEADER_SIZE) & 0xff);
    tfMd5Init(connStatePtr->scsMd5CtxPtr);
    tfSha1Init(connStatePtr->scsSha1CtxPtr);
    tfSslUpdateMd5Sha1Ctx(connStatePtr,
                          (tt8BitPtr)handshakeHeaderPtr,
                          (tt16Bit)dataLength);
#ifdef TM_USE_SSL_VERSION_33
    if (connStatePtr->scsVersion & TM_SSL_VERSION_33)
    {
        tfSha256Init(connStatePtr->scsSha256CtxPtr);
        tfSha256Update(connStatePtr->scsSha256CtxPtr,
                       (tt8BitPtr)handshakeHeaderPtr,
                       (tt16Bit)dataLength,
                       0);
    }
#endif /* TM_USE_SSL_VERSION_33 */
    sslHeaderPtr->srhLengthHigh = (tt8Bit)(dataLength >> 8); ;
    sslHeaderPtr->srhLengthLow = (tt8Bit)(dataLength & 0xff); ;

/* this ClientHello could be renegotiation message, if so, we need encrypt */
    if(connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_REHANDSHAKE)
    {
        errorCode = tfSslHandshakeMsgTransform(connStatePtr,
                                               TM_CRYPTOREQUEST_ENCODE,
                                               sslHeaderPtr,
                                               packetPtr,
                                               dataPtr,
                                               &dataLength,
                                               TM_SSL_RECORD_HEADER_SIZE);
    }
    dataLength = (tt16Bit)(dataLength + TM_SSL_RECORD_HEADER_SIZE);
    packetPtr->pktLinkDataLength = dataLength;
    packetPtr->pktChainDataLength = dataLength;

    if(errorCode == TM_ENOERROR)
    {
        clientHelloLength = (ttPktLen)(dataLength);
        errorCode = tfSslSend(&connStatePtr->scsTcpVectPtr->tcpsSocketEntry,
                  connStatePtr,
                  &packetPtr,
                  (ttPktLen TM_FAR *)&clientHelloLength,
                  0);
        packetPtr = (ttPacketPtr)0;
    }

sslSendClientHelloExit:
    if (packetPtr != (ttPacketPtr)0)
    {
        tfFreePacket(packetPtr, TM_SOCKET_LOCKED);
    }
    return errorCode;
}

/*
 * build the client ke handshake message
 */
static int tfSslSendClientKE(ttSslConnectStatePtr    connStatePtr,
                             ttVoidPtr               paramPtr)
{
/* if the scsNextWriteRecordSize is still zero, it means that
 * we haven't calculate it yet (because we only calculate it in
 * tfSslClientPreReply if client needs to send certificate)
 * The scsBufferHandle and scsXmitDataPtr has been set in tf
 * tfSslSendCertificate
 */

    tt8BitPtr                 dataPtr;
    ttSslRecordHeaderPtr      sslHeaderPtr;
    ttSslHandShakeHeaderPtr   handshakeHeaderPtr;
    ttCryptoRequest           request;
#ifdef TM_PUBKEY_USE_RSA
    ttGeneralRsaPtr           rsaPtr;
    ttCryptoRequestRsarequest rsareq;
#endif /* TM_PUBKEY_USE_RSA */
#ifdef TM_SSL_USE_EPHEMERAL_DH
    ttCryptoRequestDhrequest  dhrequest;
#endif /*TM_SSL_USE_EPHEMERAL_DH */
    int                       keLength;
    tt8Bit                    keyExchangeMethod;
    int                       errorCode;


    errorCode = TM_ENOERROR;
    TM_UNREF_IN_ARG(paramPtr);

    dataPtr = connStatePtr->scsXmitDataPtr;
    tm_bzero(&request, sizeof(request));
/* keLength = ke (not include handshake header) */
    keLength =  0;
    if (    ((connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_CLIENT_SENDCERT) == 0)
#ifdef TM_SSL_CLIENT_RESPONSE_NO_CERT
         && ((connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_CLIENT_SEND_NOCERT) == 0)
#endif /* TM_SSL_CLIENT_RESPONSE_NO_CERT */
       )
    {
/* ClientKE is our first handshake message, we already have he nextWriteRecordSize
 * set in tfSslClientPreReply
 */
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
        connStatePtr->scsXmitDataLength = (tt16Bit)
               (TM_SSL_RECORD_HEADER_SIZE);

    }

/* dataPtr here points to the handshake message*/
    handshakeHeaderPtr = (ttSslHandShakeHeaderPtr)dataPtr;
    handshakeHeaderPtr->hsHandShakeType = TM_SSL_CLIENT_KEY_EXCHANGE;
    handshakeHeaderPtr->hsLengthPad = 0;
    dataPtr += TM_SSL_HANDSHAKE_HEADER_SIZE;

/* Put the Key Exchange Length field */
    keyExchangeMethod = connStatePtr->scsCipherPtr
                ->csKeyExchange;
/* if ephemeral RSA is used, we use the ephemeral RSA sign length,
 * otherwise, use the regular RSA sign length. If Ephermal DH is used,
 * the Key is the public part of Diffie-Hellman exchange.
 */
#ifdef TM_SSL_USE_EPHEMERAL_DH
    if (keyExchangeMethod == TM_SSL_KEXCH_DHE ||
        keyExchangeMethod == TM_SSL_KEXCH_DHEEXPORT1024 )
    {
        connStatePtr->scsDHInfoPtr->dhSecretLen = TM_SSL_DHSECRET_DEFAULT_SIZE;
        dhrequest.crdDhinfoPtr = connStatePtr->scsDHInfoPtr;


/* We are the initiator to generate the key pair */
        dhrequest.crdIsInitialtor = TM_ISAKMP_INITIATOR;
        request.crType = TM_CEREQUEST_PUBKEY_DHKEYPAIR;
        request.crParamUnion.crDhgroupParamPtr = &dhrequest;
        errorCode = connStatePtr->scsPubkeyEnginePtr->ceSessionProcessFuncPtr
                   ((ttVoidPtr)&request);
        if (errorCode != TM_ENOERROR)
        {
            goto sslSendClientKeExit;
        }

        keLength = (2 + /* the opaque length field */
            connStatePtr->scsDHInfoPtr->dhGxiLen);
/* we already get the diffie-hellman secret and public value, and we knows the
 * the peer's public value, so we are able to generate shared secret.
 */
        request.crType = TM_CEREQUEST_PUBKEY_DHSHAREDKEY;
        errorCode = connStatePtr->scsPubkeyEnginePtr->ceSessionProcessFuncPtr
                   ((ttVoidPtr)&request);
        if (errorCode != TM_ENOERROR)
        {
            goto sslSendClientKeExit;
        }

/* send my public value to the Server */
        *dataPtr = (tt8Bit)((keLength - 2) >> 8);
        *(dataPtr + 1) = (tt8Bit)((keLength - 2)& 0xff);
        dataPtr += 2;
        tm_bcopy(connStatePtr->scsDHInfoPtr->dhGxiPtr,
                 dataPtr,
                 keLength - 2);
        dataPtr += (keLength - 2) ;
        connStatePtr->scsPreMasterPtr = connStatePtr->scsDHInfoPtr
                ->dhGxyPtr;
        connStatePtr->scsPreMasterLen = connStatePtr->scsDHInfoPtr
            ->dhGxyLen;

    }
#endif /*TM_SSL_USE_EPHEMERAL_DH */

#ifdef TM_PUBKEY_USE_RSA

    if ( (keyExchangeMethod == TM_SSL_KEXCH_RSAEXPORT)
          || (keyExchangeMethod == TM_SSL_KEXCH_RSA)
          || (keyExchangeMethod == TM_SSL_KEXCH_RSAEXPORT1024))
    {
        if( (connStatePtr->scsVersion == TM_SSL_VERSION_31)
          ||(connStatePtr->scsVersion == TM_SSL_VERSION_33))
        {
/* For TLS, the public-encryption results should be opaque */
            dataPtr += 2;
        }
        connStatePtr->scsPreMasterPtr = tm_get_raw_buffer
                      (TM_SSL_PRE_MASTER_SIZE);
        if (!connStatePtr->scsPreMasterPtr)
        {
            errorCode = TM_ENOBUFS;
            goto sslSendClientKeExit;
        }
        *connStatePtr->scsPreMasterPtr = TM_SSL_VERSION_MAJOR;
/* the version in this session is our lastest version supported */

#ifdef TM_USE_SSL_VERSION_33
        if ((connStatePtr->scsSsVersion) & TM_SSL_VERSION_33)
        {
            *(connStatePtr->scsPreMasterPtr + 1) = TM_SSL_VERSION_MINOR3;
        }
        else
#endif /* TM_USE_SSL_VERSION_33 */
        if ((connStatePtr->scsSsVersion) & TM_SSL_VERSION_31)
        {
            *(connStatePtr->scsPreMasterPtr + 1) = TM_SSL_VERSION_MINOR1;
        }
        else if ((connStatePtr->scsSsVersion) & TM_SSL_VERSION_30)
        {
            *(connStatePtr->scsPreMasterPtr + 1) = TM_SSL_VERSION_MINOR0;
        }
        tfGetRandomBytes(connStatePtr->scsPreMasterPtr + sizeof(tt16Bit),
                         tm_packed_byte_count(TM_SSL_PRE_MASTER_SIZE - 2));
        connStatePtr->scsPreMasterLen = TM_SSL_PRE_MASTER_SIZE;
        if (connStatePtr->scsEphemRsaPtr)
        {
            rsareq.crrRsainfoPtr    = connStatePtr->scsEphemRsaPtr;
        }
        else
        {
            rsareq.crrRsainfoPtr    = (ttGeneralRsaPtr)
                connStatePtr->scsPeerCertListPtr->certKeyPtr->pkKeyPtr;
        }
        request.crType = TM_CEREQUEST_PUBKEY_RSAENCRYPT;
        request.crParamUnion.crRsaParamPtr   = &rsareq;
        rsareq.crrRsaDataLength = TM_SSL_PRE_MASTER_SIZE;
        rsareq.crrRsaDataPtr    = connStatePtr->scsPreMasterPtr;
        rsareq.crrRsaTransPtr   = dataPtr;
        rsareq.crrRsaTransLenPtr = &keLength;
        errorCode = connStatePtr->scsPubkeyEnginePtr->
               ceSessionProcessFuncPtr(&request);
        if (errorCode != TM_ENOERROR)
        {
            goto sslSendClientKeExit;
        }
#if (defined(TM_USE_SSL_VERSION_31) || defined(TM_USE_SSL_VERSION_33))
        if ((connStatePtr->scsVersion == TM_SSL_VERSION_31)
          ||(connStatePtr->scsVersion == TM_SSL_VERSION_33))
        {
/* For TLS, the public-encryption results should be opaque */
            dataPtr -= 2;
            *dataPtr ++ = (tt8Bit)(keLength >> 8);
            *dataPtr ++ = (tt8Bit)(keLength & 0xff);
            dataPtr += keLength;
            keLength += 2;
        }
        else
#endif /* TM_USE_SSL_VERSION_31 || TM_USE_SSL_VERSION_33 */
        {
            dataPtr += keLength;
        }
    }
#endif /* TM_PUBKEY_USE_RSA */

#ifdef TM_SSL_DEBUG
    tm_debug_log0("Client premaster key = ");
    tfCryptoDebugPrintf((ttCharPtrPtr)0,
                        connStatePtr->scsPreMasterPtr,
                        connStatePtr->scsPreMasterLen,
                        (ttIntPtr)0);
#endif /* TM_SSL_DEBUG */
    handshakeHeaderPtr->hsLengthHigh = (tt8Bit)(keLength >> 8);
    handshakeHeaderPtr->hsLengthLow = (tt8Bit)(keLength & 0xff);
    connStatePtr->scsXmitDataLength = (tt16Bit)
               (connStatePtr->scsXmitDataLength +
                TM_SSL_HANDSHAKE_HEADER_SIZE + keLength);
    connStatePtr->scsXmitDataPtr = dataPtr;

/* We are able to generate master secret now */
    errorCode = tfSslGenerateMasterSecret(connStatePtr);
    if (errorCode != TM_ENOERROR)
    {
        goto sslSendClientKeExit;
    }

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
#ifdef TM_USE_SSL_VERSION_33
    if (connStatePtr->scsVersion == TM_SSL_VERSION_33)
    {
        tfSha256Update(connStatePtr->scsSha256CtxPtr,
                       (tt8BitPtr)handshakeHeaderPtr,
                       (tt16Bit)(TM_SSL_HANDSHAKE_HEADER_SIZE
                       + keLength),
                       0);
    }
#endif /* TM_USE_SSL_VERSION_33 */
    tfSslUpdateMd5Sha1Ctx(connStatePtr,
                          (tt8BitPtr)handshakeHeaderPtr,
                          (tt16Bit)(TM_SSL_HANDSHAKE_HEADER_SIZE
                          + keLength));

sslSendClientKeExit:
    return errorCode;
}


/*
 * build the cert verify handshake message
 */
static int tfSslSendCertVerify(ttSslConnectStatePtr    connStatePtr,
                               ttVoidPtr               paramPtr)
{
#ifdef TM_SSL_USE_MUTUAL_AUTH
    ttSslHandShakeHeaderPtr   handshakeHeaderPtr;
    tt8BitPtr                 dataPtr;
    int                       certVeriLen;
#endif /* TM_SSL_USE_MUTUAL_AUTH */
    ttSslRecordHeaderPtr      sslHeaderPtr;
    tt16Bit                   recordSize;
    int                       errorCode;


    TM_UNREF_IN_ARG(paramPtr);
    errorCode = TM_ENOERROR;

#ifdef TM_SSL_USE_MUTUAL_AUTH
    dataPtr = connStatePtr->scsXmitDataPtr;
    certVeriLen = 0;
    if (connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_CLIENT_SENDCV)
    {

        handshakeHeaderPtr = (ttSslHandShakeHeaderPtr)dataPtr;
        handshakeHeaderPtr->hsHandShakeType = TM_SSL_CERTIFICATE_VERIFY;
        handshakeHeaderPtr->hsLengthPad = 0;
        dataPtr += TM_SSL_HANDSHAKE_HEADER_SIZE;

        errorCode = tfSslBuildCertVerify(connStatePtr, dataPtr,
               (tt32BitPtr)&certVeriLen);
        if (errorCode != TM_ENOERROR)
        {
            goto sslSendCertVerifyExit;
        }

        handshakeHeaderPtr->hsLengthHigh = (tt8Bit) (certVeriLen >> 8);
        handshakeHeaderPtr->hsLengthLow = (tt8Bit)(certVeriLen & 0xff);

        connStatePtr->scsXmitDataPtr = dataPtr + certVeriLen;
        connStatePtr->scsXmitDataLength = (tt16Bit)(
               connStatePtr->scsXmitDataLength + TM_SSL_HANDSHAKE_HEADER_SIZE
               + certVeriLen);

#ifdef TM_USE_SSL_VERSION_33
        if (connStatePtr->scsVersion == TM_SSL_VERSION_33)
        {
            tfSha256Update(connStatePtr->scsSha256CtxPtr,
                           (tt8BitPtr)handshakeHeaderPtr,
                           (tt16Bit)(TM_SSL_HANDSHAKE_HEADER_SIZE
                           + certVeriLen),
                           0);
        }
#endif /* TM_USE_SSL_VERSION_33 */
        tfSslUpdateMd5Sha1Ctx(connStatePtr,
                              (tt8BitPtr)handshakeHeaderPtr,
                              (tt16Bit)(TM_SSL_HANDSHAKE_HEADER_SIZE
                              + certVeriLen));
    }
#endif /* TM_SSL_USE_MUTUAL_AUTH */
/* because we don't know the exact DSA signature length before,
 * we set this record size until this moment
 */
    sslHeaderPtr = (ttSslRecordHeaderPtr)
                     (connStatePtr->scsXmitPacketPtr->pktLinkDataPtr);
    recordSize = (tt16Bit)(connStatePtr->scsXmitDataLength -
                       TM_SSL_RECORD_HEADER_SIZE);
    sslHeaderPtr->srhLengthHigh = (tt8Bit)
                     (recordSize >> 8);
    sslHeaderPtr->srhLengthLow = (tt8Bit)
                     (recordSize & 0xff);

    if(connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_REHANDSHAKE)
    {
/* need encryption if HelloRequest is exchanged */
        errorCode = tfSslHandshakeMsgTransform(connStatePtr,
                                               TM_CRYPTOREQUEST_ENCODE,
                                               sslHeaderPtr,
                                               connStatePtr->scsXmitPacketPtr,
                                               connStatePtr->scsXmitDataPtr,
                                               &recordSize,
                                               TM_SSL_RECORD_HEADER_SIZE);
        connStatePtr->scsXmitDataLength = (tt16Bit)(recordSize +
                          TM_SSL_RECORD_HEADER_SIZE);
        connStatePtr->scsXmitDataPtr = (tt8BitPtr)(sslHeaderPtr)
                                   + connStatePtr->scsXmitDataLength;
    }

#ifdef TM_SSL_USE_MUTUAL_AUTH
sslSendCertVerifyExit:
#endif /* TM_SSL_USE_MUTUAL_AUTH */
    return errorCode;
}

/*
 * build the client change cipher message
 */
static int tfSslSendClientCCS(ttSslConnectStatePtr    connStatePtr,
                              ttVoidPtr               paramPtr)
{

    tt8BitPtr                dataPtr;
    ttPacketPtr              packetPtr;
    ttSslRecordHeaderPtr     sslHeaderPtr;
    int                      errorCode;
    tt16Bit                  recordSize;
    tt8Bit                   ivLength;

    TM_UNREF_IN_ARG(paramPtr);
    errorCode = TM_ENOERROR;
    ivLength = (tt8Bit)0;

    if (!connStatePtr->scsXmitPacketPtr)
    {
/* we need allocate buffer here if we reply to a resumed session negotiation,
 */
#ifdef TM_USE_SSL_VERSION_33
        if((connStatePtr->scsVersion == TM_SSL_VERSION_33)
         &&((connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_ARCFOUR)
          &&(connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_NULL)))
        {
            ivLength = ivLength + connStatePtr->scsCryptoAlgPtr->ealgBlockSize;
        }
#endif /* TM_USE_SSL_VERSION_33 */
        packetPtr = tfGetSharedBuffer(0,
                         2*TM_SSL_RECORD_HEADER_SIZE +
                         TM_SSL_CHANGE_CIPHER_SIZE +
/* TBD, we don't know how much the finish size yet, use V30 length */
                         TM_SSL_V30_FINISH_SIZE +
                         ivLength,
                         0);
        if (!packetPtr)
        {

            errorCode = TM_ENOBUFS;
            goto sslSendClientCcsExit;
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
    recordSize = TM_SSL_CHANGE_CIPHER_SIZE;
    sslHeaderPtr->srhLengthHigh = 0;
    sslHeaderPtr->srhLengthLow = (tt8Bit)recordSize;

    dataPtr += TM_SSL_RECORD_HEADER_SIZE;
    *dataPtr ++ = TM_SSL_CHANGECIPHER_VALID;

    connStatePtr->scsXmitDataPtr = dataPtr;
    connStatePtr->scsXmitDataLength = (tt16Bit)
        (connStatePtr->scsXmitDataLength  +
         TM_SSL_RECORD_HEADER_SIZE +
         TM_SSL_CHANGE_CIPHER_SIZE);

    if(connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_REHANDSHAKE)
    {
/* HelloRequest is exchanged, this packet (CCS) has to be encrypted */
        errorCode = tfSslHandshakeMsgTransform(connStatePtr,
                                               TM_CRYPTOREQUEST_ENCODE,
                                               sslHeaderPtr,
                                               connStatePtr->scsXmitPacketPtr,
                                               dataPtr,
                                               &recordSize,
                                               (tt16Bit)(connStatePtr->
                                               scsXmitDataLength -
                                               recordSize));
/* now we need to see how many more bytes has been added, note
 * that, we already count the basic CCS message length
 */
        recordSize = (tt16Bit)(recordSize - TM_SSL_CHANGE_CIPHER_SIZE);
        connStatePtr->scsXmitDataPtr = dataPtr +
                        recordSize;
        connStatePtr->scsXmitDataLength = (tt16Bit)(
             connStatePtr->scsXmitDataLength + recordSize);

    }
/* Calculate crypto keys, mac keys, ivs, for server, we calculate keys when
 * we processed the Client CCS. For client, we calculate it after we
 * construct CCS.  For resumed handshake, we already built cryptomac keys when
 * we process server hello, so don't build it again.
 */
    if (!(connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_RESUMESESSION))
    {
        errorCode = tfSslGenerateCryptoMacKeys(connStatePtr);
    }
    if(errorCode == TM_ENOERROR)
    {
/* generate client write keys */
        errorCode = tfSslClientWriteKeySchedule(connStatePtr,
                           &connStatePtr->scsEncodeMacKey,
                           &connStatePtr->scsEncodeCryptoKey,
                           &connStatePtr->scsEncodeIvPtr);
        connStatePtr->scsOutSeqNumber = 0;
        connStatePtr->scsOutSeqNumberHigh = 0;
    }

sslSendClientCcsExit:
    return errorCode;
}

/* DESCRIPTION: This function will be called before we construct Server
 * Hello. Basically, it determines how to reply server's second record
 * based on the cipher suite. For example, it determines if we need to
 * send certificate request, or server key exchange. And how many bytes
 * for the whole record. We need the whole record size because certificate
 * will be big and we have to send out partially. Without a good record
 * length for the whole record, client will drop our second reply message.
 * PARAMETERS: connStatePtr            the connection state
 */
static int tfSslClientPreReply(ttSslConnectStatePtr    connStatePtr,
                               ttVoidPtr               paramPtr)
{
    ttPkiCertListPtr    myCertListPtr;
    ttPkiCertListPtr    peerCertListPtr;
    int                 errorCode;
    int                 keLength;
    int                 certVeriLen;
    tt8Bit              keyExchangeMethod;
    tt8Bit              digSignLength;

    TM_UNREF_IN_ARG(paramPtr);
    keLength = 0;
    certVeriLen = 0;
    errorCode = TM_ENOERROR;
    digSignLength = (tt8Bit)0;
/*  For client this time,  we are going to send back handshake +
 *  changecipher + handshake, we need maintain scsNextWriteRecordSize
 * and scsXmitBufferSize. Clear them first
 */

    connStatePtr->scsXmitBufferSize = TM_SSL_RECORD_HEADER_SIZE;
    connStatePtr->scsNextWriteRecordSize = 0;
    peerCertListPtr = connStatePtr->scsPeerCertListPtr;

/*1. Certificate message */
    if (    (connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_CLIENT_SENDCERT)
#ifdef TM_SSL_CLIENT_RESPONSE_NO_CERT
         || (connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_CLIENT_SEND_NOCERT)
#endif /* TM_SSL_CLIENT_RESPONSE_NO_CERT */
       )
    {
#ifdef TM_USE_SSL_VERSION_33
        if (connStatePtr->scsVersion == TM_SSL_VERSION_33)
        {
            myCertListPtr = connStatePtr->scsSessionPtr->ssCertSuppPtr;
        }
        else
#endif /* TM_USE_SSL_VERSION_33 */
        {
            myCertListPtr = connStatePtr->scsSessionPtr->ssCertListPtr;
        }
        connStatePtr->scsTotalCertBytes = 0;
        while (myCertListPtr)
        {
            connStatePtr->scsTotalCertBytes = (tt16Bit)(connStatePtr->
                      scsTotalCertBytes + myCertListPtr->certLength + 3);
/*The '3' is the length field*/
            myCertListPtr = myCertListPtr->certSslNextPtr;
        }
        connStatePtr->scsTotalCertBytes = (tt16Bit)(
                       connStatePtr->scsTotalCertBytes + 3);
        connStatePtr->scsXmitBufferSize = (tt16Bit)
                (connStatePtr->scsXmitBufferSize +
                 connStatePtr->scsTotalCertBytes);
        connStatePtr->scsNextWriteRecordSize = (tt16Bit)
                (connStatePtr->scsNextWriteRecordSize +
                 connStatePtr->scsTotalCertBytes);
    }

/*2. Client KE */
    keyExchangeMethod = connStatePtr->scsCipherPtr
                ->csKeyExchange;
/* if ephemeral RSA is used, we use the ephemeral RSA sign length,
 * otherwise, use the regular RSA sign length. If Ephermal DH is used,
 * the Key is the public part of Diffie-Hellman exchange.
 */
#ifdef TM_SSL_USE_EPHEMERAL_DH
    if (keyExchangeMethod == TM_SSL_KEXCH_DHE ||
        keyExchangeMethod == TM_SSL_KEXCH_DHEEXPORT1024)
    {
        keLength = TM_SSL_HANDSHAKE_HEADER_SIZE +
                   2 + /* the opaque length field */
                   connStatePtr->scsDHInfoPtr->dhGroupLen;
    }
#endif /*TM_SSL_USE_EPHEMERAL_DH */

#ifdef TM_PUBKEY_USE_RSA

    if ( (keyExchangeMethod == TM_SSL_KEXCH_RSAEXPORT)
          || (keyExchangeMethod == TM_SSL_KEXCH_RSAEXPORT1024)
          || (keyExchangeMethod == TM_SSL_KEXCH_RSA))
    {
        if (connStatePtr->scsEphemRsaPtr)
        {
            keLength = TM_SSL_HANDSHAKE_HEADER_SIZE +
                    connStatePtr->scsEphemRsaPtr->rsaN->asn1Length;

#if (defined(TM_USE_SSL_VERSION_31) || defined(TM_USE_SSL_VERSION_33))
            if( (connStatePtr->scsVersion == TM_SSL_VERSION_31)
              ||(connStatePtr->scsVersion == TM_SSL_VERSION_33))
            {
/* the extra 2 bytes is for TLS public encryption usage, it should be
 * encapsulated in opaque format
 */
                keLength += 2;
            }
#endif /* TM_USE_SSL_VERSION_31 || TM_USE_SSL_VERSION_33 */
        }
        else
        {
            keLength = TM_SSL_HANDSHAKE_HEADER_SIZE +
                    ((ttGeneralRsaPtr)peerCertListPtr->certKeyPtr->pkKeyPtr)->
                      rsaN->asn1Length;
#if (defined(TM_USE_SSL_VERSION_31) || defined(TM_USE_SSL_VERSION_33))
            if( (connStatePtr->scsVersion == TM_SSL_VERSION_31)
              ||(connStatePtr->scsVersion == TM_SSL_VERSION_33))
            {
/* the extra 2 bytes is for TLS public encryption usage, it should be
 * encapsulated in opaque format
 */
                keLength += 2;
            }
#endif /* TM_USE_SSL_VERSION_31 || TM_USE_SSL_VERSION_33 */
        }
    }
#endif /* TM_PUBKEY_USE_RSA */
    connStatePtr->scsXmitBufferSize = (tt16Bit)
                (connStatePtr->scsXmitBufferSize + keLength +
                 TM_SSL_RECORD_HEADER_SIZE);
    connStatePtr->scsNextWriteRecordSize = (tt16Bit)
                (connStatePtr->scsNextWriteRecordSize + keLength);
/*3. Client Certificate verify */
    myCertListPtr = connStatePtr->scsSessionPtr->ssCertListPtr;

#ifdef TM_SSL_USE_MUTUAL_AUTH
    if (connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_CLIENT_SENDCERT)
    {
/* if we send certificate, then we need send certificate verify
 * message too, because we only support DSA/RSA certificate, not
 * fixed Diffie-Hellman certificate. So, we need send cert verify
 * as long as we need send certificate
 */
        connStatePtr->scsRunFlags |= TM_SSL_RUNFLAG_CLIENT_SENDCV;
#ifdef TM_USE_SSL_VERSION_33
        if (connStatePtr->scsVersion == TM_SSL_VERSION_33)
        {
/* Algorithm field is 2byte */
            digSignLength = (tt8Bit)2;
        }
#endif /* TM_USE_SSL_VERSION_33 */
#ifdef TM_PUBKEY_USE_RSA
        if (connStatePtr->scsSessionPtr->ssSignAlgorithm == SADB_PUBKEY_RSA)
        {
            certVeriLen = ((ttGeneralRsaPtr)myCertListPtr->certKeyPtr
                       ->pkKeyPtr)->rsaN->asn1Length
/* many implementations expect the signature to be an opaque structure */
                       + 2
                       + digSignLength;

        }
#ifdef TM_PUBKEY_USE_DSA
        else
#endif /* TM_PUBKEY_USE_DSA */
#endif /* TM_PUBKEY_USE_RSA */
#ifdef TM_PUBKEY_USE_DSA

        if (connStatePtr->scsSessionPtr->ssSignAlgorithm ==
                     SADB_PUBKEY_DSA)
        {
/* for DSA signature, we can only estimates, because we are going to ASN.1
 * encapsulate the r and s, and the actual length depends on the first bit
 * is set (negative asn.1 integar) or not, each r and s could be 160 bit
 * (20 octets) long, plus one byte (zer0), + asn1 object type and length
 */
            certVeriLen = 48
/* many implementations expect the signature to be an opaque structure */
                       + 2
                       + digSignLength;
        }
#endif /* TM_PUBKEY_USE_RSA */
        certVeriLen = certVeriLen + TM_SSL_HANDSHAKE_HEADER_SIZE;
        connStatePtr->scsXmitBufferSize = (tt16Bit)
                (connStatePtr->scsXmitBufferSize + certVeriLen);
        connStatePtr->scsNextWriteRecordSize = (tt16Bit)
                (connStatePtr->scsNextWriteRecordSize + certVeriLen);

    }
#endif /* TM_SSL_USE_MUTUAL_AUTH */


/*4. We will try to send change cipher spec together with the handshake
 *   messages. New record header and Change Cipher Spec
 */
    connStatePtr->scsXmitBufferSize = (tt16Bit)
                (connStatePtr->scsXmitBufferSize + TM_SSL_RECORD_HEADER_SIZE
                 + TM_SSL_CHANGE_CIPHER_SIZE);
/*5. New record header and Client Finished message */
#ifdef TM_USE_SSL_VERSION_30
    if (connStatePtr->scsVersion == TM_SSL_VERSION_30)
    {
/* SSL v3.0, the finished message maximum length is :
 * record header (5) + handshake(4) + md5_hash(16) + sha_hash(20) +
 * sha_hash(20)+ maximum padding (16 for AES block cipher)
 */
        connStatePtr->scsXmitBufferSize = (tt16Bit)
                (connStatePtr->scsXmitBufferSize + TM_SSL_RECORD_HEADER_SIZE
                 + TM_SSL_V30_FINISH_SIZE);
    }
#if (defined(TM_USE_SSL_VERSION_31) || defined(TM_USE_SSL_VERSION_33))
    else
#endif /* TM_USE_SSL_VERSION_31 || TM_USE_SSL_VERSION_33 */
#endif /* TM_USE_SSL_VERSION_30 */

#ifdef TM_USE_SSL_VERSION_31
    if (connStatePtr->scsVersion == TM_SSL_VERSION_31)
    {
/* TLS v1.2, the finished message maximum length is :
 * record header (5) + handshake(4) + verified data(12)
 * sha_hash(20)+ maximum padding (16 for AES block cipher)
 */
        connStatePtr->scsXmitBufferSize = (tt16Bit)
                (connStatePtr->scsXmitBufferSize + TM_SSL_RECORD_HEADER_SIZE
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
        connStatePtr->scsXmitBufferSize = (tt16Bit)
                (connStatePtr->scsXmitBufferSize + TM_SSL_RECORD_HEADER_SIZE
                 + TM_SSL_V33_FINISH_SIZE);

    }
#endif /* TM_USE_SSL_VERSION_33 */

#ifdef TM_USE_SSL_VERSION_33
    if((connStatePtr->scsVersion == TM_SSL_VERSION_33)
     &&((connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_ARCFOUR)
      &&(connStatePtr->scsCryptoAlgPtr->ealgName != SADB_EALG_NULL)))
    {
        connStatePtr->scsXmitBufferSize = connStatePtr->scsXmitBufferSize +
            (tt16Bit)connStatePtr->scsCryptoAlgPtr->ealgBlockSize;
    }
#endif /* TM_USE_SSL_VERSION_33 */

/* if HelloRequest is involved, we need two more SSL tailers, because
 * Client KE and ClientCipherSpec messages have to be protected
 */
    if(connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_REHANDSHAKE)
    {
        connStatePtr->scsXmitBufferSize = (tt16Bit)
                (connStatePtr->scsXmitBufferSize +
                 (TM_SSL_RECORD_OVERHEAD_MAX_SIZE << 1));
    }

    return errorCode;
}



/* FUNCTION DESCRIPTION:
 * This function is called when user wants to make a connection to the
 * SSL server. The same ssl handle can be connected and disconnected
 * multiple times.  The sslClientHandle remains allocated unless
 * tfSslFreeSession is called.
 *
 * PARAMETER:
 * sslClientHandle:                    the pointer to the SSL client handle.
 * sslConnectHandlePtr                 will return a new connection handle
 * servCommonNamePtr                   need this information to verify
 *                                     certificate
 * serverSockAddrPtr                   user should get the server's socket
 *                                     address by themselves. (dns, see
 *                                     getaddrinfo for details
 * blockMode                           TM_BLOCKING_ON or TM_BLOCKING_OFF
 * timeout                             How long wait for a connection to
 *                                     complete
 * Returns
 * Value            Meaning
 * TM_EINPROGRESS    for non-blocking mode, the tfSslConnect will return
 *                   immidiately. This value means the connectioin is still
 *                   in progress
 * TM_EINVAL         invalid parameter
 * TM_ENOERROR       Connection finished. For blocking mode, if successfully
 *            connected, TM_ENOERROR will return. For non-blocking mode
 *            if the connection was there before this call, TM_ENOEROR
 *            will be returned;
 */



/* process one negotiation record
 * dataPtr points to the real content after the handshake header
 */
int tfSslClientProcessHandShake(tt8Bit                hsType,
                                ttSslConnectStatePtr  connStatePtr,
                                tt8BitPtr             dataPtr,
                                tt16Bit               msgSize)
{
    int                       errorCode;
    tt8Bit                    event;
    tt8Bit                    resume;

    tm_trace_func_entry(tfSslClientProcessHandShake);
    tm_debug_log1("tfSslClientProcessHandShake sd=%u",
          connStatePtr->scsTcpVectPtr->tcpsSocketEntry.socIndex);
#ifdef TM_LOG_SSL_DEBUG
    tfEnhancedLogWrite(
            TM_LOG_MODULE_SSL,
            TM_LOG_LEVEL_DEBUG,
            "tfSslClientProcessHandShake: sd=%u",
            connStatePtr->scsTcpVectPtr->tcpsSocketEntry.socIndex);
#endif /* TM_LOG_SSL_DEBUG */

    event = TM_SSL_CE_NOEVENT;
    errorCode = TM_EINVAL;
    if (    (hsType == TM_SSL_HELLO_REQUEST)
         || (connStatePtr->scsState >= TM_SSL_CT_SENT_CHELLO)
       )
    {
        switch(hsType)
        {
        case TM_SSL_HELLO_REQUEST:
            if (    (connStatePtr->scsState == TM_SSL_CT_INITIAL)
                 || (connStatePtr->scsState == TM_SSL_CT_OPENED) )
            {
                errorCode = TM_ENOERROR;
                event = TM_SSL_CE_OUTCH;
                connStatePtr->scsRunFlags |= TM_SSL_RUNFLAG_REHANDSHAKE;
/* free the cache because we have to rehandshake */
                (void)tfSslFreeCache(connStatePtr->scsSessionPtr,
                                     connStatePtr->scsSessionId,
                                     connStatePtr->scsSessionIdLength);
            }
            break;
        case TM_SSL_SERVER_HELLO:
            if (connStatePtr->scsState == TM_SSL_CT_SENT_CHELLO)
            {
                errorCode = tfSslParseServerHello(connStatePtr,
                                                  dataPtr,
                                                  &resume,
                                                  msgSize);
                if (errorCode == TM_ENOERROR)
                {
                    if (resume)
                    {
                        event = TM_SSL_CE_INSH2;
                    }
                    else
                    {
                        event = TM_SSL_CE_INSH1;
                    }
                }
            }
            break;
        case TM_SSL_CERTIFICATE:
            if (connStatePtr->scsState == TM_SSL_CT_RECV_SH1)
            {
                errorCode = tfSslParseCertificate(connStatePtr, dataPtr, msgSize);
                if (errorCode == TM_ENOERROR)
                {
                    event = TM_SSL_CE_INCERT;
                }
            }
            break;
        case TM_SSL_CERTIFICATE_REQUEST:
            if (    (connStatePtr->scsState == TM_SSL_CT_RECV_CERT)
                 || (connStatePtr->scsState == TM_SSL_CT_RECV_SKE) )
            {
                errorCode = tfSslParseCertificateRequest(connStatePtr,
                                                         dataPtr,
                                                         msgSize);
                if (errorCode == TM_ENOERROR)
                {
                    event = TM_SSL_CE_INCERTREQ;
                }
            }
            break;
        case TM_SSL_SERVER_KEY_EXCHANGE:
            if (connStatePtr->scsState == TM_SSL_CT_RECV_CERT)
            {
                errorCode = tfSslParseServerKeyExchange(connStatePtr,
                                                        dataPtr,
                                                        msgSize);
                if (errorCode == TM_ENOERROR)
                {
                    event = TM_SSL_CE_INSKE;
                }
            }
            break;
        case TM_SSL_SERVER_HELLO_DONE:
            if (    (connStatePtr->scsState == TM_SSL_CT_RECV_CERT)
                 || (connStatePtr->scsState == TM_SSL_CT_RECV_SKE)
                 || (connStatePtr->scsState == TM_SSL_CT_RECV_CERTREQ) )
            {
                if (    ((connStatePtr->scsRunFlags & TM_SSL_RUNFLAG_SERVER_SENDEDH) == 0)
                     || (connStatePtr->scsDHInfoPtr != (ttDiffieHellmanPtr)0) )
                {
                    errorCode = TM_ENOERROR;
                    event = TM_SSL_CE_INDONE;
                }
            }
            break;
        case TM_SSL_FINISHED:
            if (    (connStatePtr->scsState == TM_SSL_CT_RECV_CCS1)
                 || (connStatePtr->scsState == TM_SSL_CT_RECV_CCS2) )
            {
                errorCode = tfSslParseFinished(connStatePtr, dataPtr, msgSize);
                if (errorCode == TM_ENOERROR)
                {
                    event = TM_SSL_CE_INFINISH;
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
                }
            }
            break;
        default:
            errorCode = TM_EINVAL;
            break;
        }
        if (    (errorCode == TM_ENOERROR)
             || (errorCode == TM_SSL_ALT_NO_CERTIFICATE) )
        {
            if (hsType != TM_SSL_HELLO_REQUEST)
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
            }

            if (event != TM_SSL_CE_NOEVENT)
            {
                errorCode = tfSslClientStateMachine(connStatePtr,
                                                    0,
                                                    event);
            }
        }
    }

    tm_trace_func_exit_rc(tfSslClientProcessHandShake, errorCode);
    return errorCode;
}


 /*
 * State machine main function. Advance to next state and calls the
 * appropriate state function(s) based on state vector current State,
 * and event. Returns error value.
 *
 * Parameters:
 * connStatePtr  ssl state
 * paramPtr     possible parameter to call state functions
 * event       Event to trigger state machine
 * Return value
 * errorCode
 */
static int tfSslClientStateMachine (ttSslConnectStatePtr connStatePtr,
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
                                    &tlSslClientMatrix[curState][event];
/* Store next state for the event */
    connStatePtr->scsState = matrixEntryPtr->smtxTransition;
/* Index into state function table */
    procIndex = matrixEntryPtr->smtxProcedureIndex;
/* Check whether there is any function to call */
    if (procIndex < TM_SSL_CM_LAST_INDEX)
    {
/* State function table entry pointer */
        sslProcEntryPtr = (ttSslStateProcedureEntryPtr)
                           &tlSslClientProcedureEntry[procIndex];
/* Call all functions corresponding to index */
        do
        {
            errCode = tlSslClientFuncTable[sslProcEntryPtr->sstaFunctIndex]
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

#else /* !TM_USE_SSL_CLIENT */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_SSL_CLIENT not defined */
int tlSslClientDummy = 0;
#endif /* TM_USE_SSL_CLIENT */

/***************** End Of File *****************/
