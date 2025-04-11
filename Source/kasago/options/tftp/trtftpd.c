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
 * Description: TFTP server Protocol
 * Filename: trtftpd.c
 * Author: 
 * Date Created: 10/27/98
 * $Source: source/trtftpd.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:48:54JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_TFTPD
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/* Suppress PC-LINT compile error for unreferenced macros */
#ifdef TM_LINT
LINT_UNREF_MACRO(TM_TFTP_TIMEOUT_VAL)
LINT_UNREF_MACRO(TM_TFTP_RETRY_TIME)
#endif /* TM_LINT */

#define TM_TFTPD_PORT tm_const_htons(69)

#define TM_TFTP_TIMEOUT_VAL 5
#define TM_TFTP_RETRY_TIME  5
#define TM_TFTP_CB_TIMEOUT  0x4000

/* Packet Size */
#define TM_TFTP_SEGSIZE 512
#define TM_TFTP_PKTSIZE (TM_TFTP_SEGSIZE+4)

/*
 * TFTP commands:
 * RRQ, WRQ, DATA, ACK, ERROR
 */

/* TFTP commands (used as index in tlFtpdCmdArray[]) */
#define TM_TFTP_RRQ   1
#define TM_TFTP_WRQ   2
#define TM_TFTP_DATA  3
#define TM_TFTP_ACK   4
#define TM_TFTP_ERR   5

/*
 * Error codes.
 */
#define TM_TFTP_EUNDEF     0       /* not defined */
#define TM_TFTP_ENOTFOUND  1       /* file not found */
#define TM_TFTP_EACCESS    2       /* access violation */
#define TM_TFTP_ENOSPACE   3       /* disk full or allocation exceeded */
#define TM_TFTP_EBADOP     4       /* illegal TFTP operation */
#define TM_TFTP_EBADID     5       /* unknown transfer ID */
#define TM_TFTP_EEXISTS    6       /* file already exists */
#define TM_TFTP_ENOUSER    7       /* no such user */

struct tsErrmsg {
    int         e_code;
    ttCharPtr   e_msg;
};

static const struct tsErrmsg TM_CONST_QLF tlTftpErrmsgs[] = {
    { TM_TFTP_EUNDEF,   "Undefined error code" },
    { TM_TFTP_ENOTFOUND,    "File not found" },
    { TM_TFTP_EACCESS,  "Access violation" },
    { TM_TFTP_ENOSPACE, "Disk full or allocation exceeded" },
    { TM_TFTP_EBADOP,   "Illegal TFTP operation" },
    { TM_TFTP_EBADID,   "Unknown transfer ID" },
    { TM_TFTP_EEXISTS,  "File already exists" },
    { TM_TFTP_ENOUSER,  "No such user" },
    { -1,       0 }
};

/* Tftp server data */
typedef struct tsTftpSrv
{
/* Queue head */
    ttList           fTftpdConQHead;
/* Tftp connections Idle timeout */
    tt32Bit          fTftpdIdleTimeout;
/* Tftp Retry Max Time */
    tt32Bit          fTftpdMaxTimeout;
/* Pend counting semaphore for the FTPD */ 
    ttPendEntry      fTftpdPendSem;
#if ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED)) )
    ttLockEntry      fTftpdExecuteLockEntry;
#endif  /* ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED))  */ 
/* TFTPD blocking or non blocking */
    int              fTftpdBlockingState;
/* TFTPD listening descriptor */
    int              fTftpdListenDesc;
    int              fTftpdPendingRecv;
/* Max Connections */
    int              fTftpdMaxConnections;   
/* Tftp Work Buffer */
    char             fTftpdBuf[TM_TFTP_PKTSIZE];
/* Indicates whether the TFTPD is executing */
    tt8Bit           fTftpdExecuting;
/* Indicates whether the TFTPD is shutdown */
    tt8Bit           fTftpdShutdown;
} ttTftpSrv;

typedef ttTftpSrv  TM_FAR * ttTftpSrvPtr;


typedef struct  tsTftpHdr {
    tt16Bit th_opcode;      /* packet type */
    union {
        tt16Bit tu_block;   /* block # */
        tt16Bit tu_code;    /* error code */
        ttUChar tu_stuff[1];    /* request packet stuff */
    } th_u;
    ttUChar th_data[1];     /* data or error string */
}ttTftpHdr;

#define th_block    th_u.tu_block
#define th_code     th_u.tu_code
#define th_stuff    th_u.tu_stuff
#define th_msg      th_data

typedef ttTftpHdr TM_FAR * ttTftpHdrPtr;

#ifdef TM_LINT
LINT_UNREF_MEMBER_BEGIN
#endif /* TM_LINT */
typedef struct tsTftpdConnEntry
{
/* open connection list. Has to be first. */
    ttNode                          fconNode;
/* One timer on the connection */
    ttTimerPtr                      fconTimerPtr;
    ttVoidPtr                       fconFilePtr;
    ttVoid                          (TM_CODE_FAR * fconFunction)
                                        (struct tsTftpdConnEntry TM_FAR *, 
                                         ttTftpHdrPtr,
                                         int,
                                         ttTftpSrvPtr);
    struct sockaddr_storage         fconPeerAddress;
    struct sockaddr_storage         fconLocalAddress;    
    tt32Bit                         fconTimeCount;
    tt16Bit                         fconCBFlag;
    tt16Bit                         fconEndFlag;
    tt16Bit                         fconBlockNum;
    tt16Bit                         fconMode;
    char                            fconSendBuf[TM_TFTP_PKTSIZE];
    int                             fconSendSize; 
    tt8Bit                          fconNewline;
    char                            fconPrevchar;
} ttTftpdConnEntry;
#ifdef TM_LINT
LINT_UNREF_MEMBER_END
#endif /* TM_LINT */

typedef ttTftpdConnEntry TM_FAR * ttTftpdConnEntryPtr;

static void tfTftpdExecute (ttTftpSrvPtr tftpSrvPtr);
static void tfTftpdCloseListen (ttTftpSrvPtr tftpSrvPtr);
static void tfTftpdSignalRecvCtrl(int listenDesc, int flags);
static void tfTftpdRecvCtrl(ttTftpSrvPtr tftpSrvPtr);
static void tfTftpXmitfile(ttTftpdConnEntryPtr connEntryPtr, 
                           ttTftpHdrPtr tp, 
                           int size,
                           ttTftpSrvPtr tftpSrvPtr);
static void tfTftpRecvfile(ttTftpdConnEntryPtr connEntryPtr, 
                           ttTftpHdrPtr tp, 
                           int size,
                           ttTftpSrvPtr tftpSrvPtr);
static int  tfTftpAckSend(ttTftpdConnEntryPtr connEntryPtr, 
                          int retry, 
                          ttTftpSrvPtr tftpSrvPtr);
static int  tfTftpDataSend(ttTftpdConnEntryPtr connEntryPtr, 
                           int TM_FAR * size, 
                           ttTftpSrvPtr tftpSrvPtr);
static void tfTftpErrorSend(
    struct sockaddr_storage TM_FAR * peerAddressPtr,
    struct sockaddr_storage TM_FAR * localAddressPtr,
    int                              error,
    ttTftpSrvPtr                     tftpSrvPtr);
static int  tfTftpRetrySend(ttTftpdConnEntryPtr connEntryPtr, 
                            ttTftpSrvPtr tftpSrvPtr);

void tfTftpdSignalTimeout(ttVoidPtr      timerBlockPtr,
                          ttGenericUnion userParm1,
                          ttGenericUnion userParm2);

static void tfTftpFreeConnection ( ttTftpSrvPtr        tftpSrvPtr,
                                   ttTftpdConnEntryPtr connEntryPtr );

/*
 * tfTftpdInit function description:
 *
 * Parameters       Meaning
 *  none             none
 *
 * Return values    Meaning
 *  none             none
 */
void tfTftpdInit (void)
{
    ttTftpSrvPtr                tftpSrvPtr;
   
    if (tm_context(tvTftpSrvPtr) == (ttVoidPtr)0)
    {
        tftpSrvPtr = (ttTftpSrvPtr)tm_kernel_malloc(sizeof(ttTftpSrv));
        if(tftpSrvPtr != (ttTftpSrvPtr)0)
        {
            tm_context(tvTftpSrvPtr) = (ttVoidPtr)tftpSrvPtr;
/* 
 * Note: fTftpdExecuting and the execute lock set to zero in tm_bzero() call.
 */
            tm_bzero((ttVoidPtr)tftpSrvPtr, sizeof(ttTftpSrv));   
/* No ftp server listening socket yet */
            tftpSrvPtr->fTftpdListenDesc = TM_SOCKET_ERROR;
/* Initialize List of open TFTPD connections so it is empty */
            tfListInit(&tftpSrvPtr->fTftpdConQHead);
        }
    }
}

/*
 * tfTftpdUserStart function description:
 *  Starts the TFTP server.  Called by the user.
 *
 *  Parameters       Meaning
 *  maxConnections   maximum number of TFTP connections
 *  sendTimeout      transmit timeout
 *  timeoutTime      recieve timeout
 *  blockingState    TM_BLOCKING_ON or TM_BLOCKING_OFF
 *
 * Return values
 *  error code
 */
int tfTftpdUserStart (  int maxConnections, 
                        int sendTimeout,
                        int timeoutTime,
                        int blockingState)
{
    struct sockaddr_storage sockAddress;
    int                     addressLength;
    int                     errorCode;
    int                     listenDesc;
    int                     tempInt;
    ttSockAddrPtrUnion      tempSockAddr;
    ttTftpSrvPtr            tftpSrvPtr;

    tftpSrvPtr = (ttTftpSrvPtr)tm_context(tvTftpSrvPtr);
    if(tftpSrvPtr != (ttTftpSrvPtr)0)
    {
        tempSockAddr.sockNgPtr = &sockAddress;
        if( (maxConnections <= 0) ||
            ((blockingState != TM_BLOCKING_ON) && 
            (blockingState != TM_BLOCKING_OFF)) ||
            ((blockingState == TM_BLOCKING_ON) &&  
            (tvPendPostEnabled == TM_8BIT_ZERO)) )
        {
            errorCode = TM_EINVAL;
        }
        else
        {
            tm_call_lock_wait(&tftpSrvPtr->fTftpdExecuteLockEntry);
            tftpSrvPtr->fTftpdShutdown = 0;
/* In milliseconds */
            tftpSrvPtr->fTftpdIdleTimeout = 
                    (tt32Bit)((tt32Bit)sendTimeout*TM_UL(1000));
            tftpSrvPtr->fTftpdMaxTimeout = (tt32Bit)timeoutTime;
/* Blocking state */
            tftpSrvPtr->fTftpdBlockingState = blockingState;
/* Initialize pending sempahore */
            tm_bzero(&tftpSrvPtr->fTftpdPendSem, sizeof(ttPendEntry));
/* No connections yet */
            tftpSrvPtr->fTftpdMaxConnections = maxConnections;
            tftpSrvPtr->fTftpdPendingRecv = 0;

/*
 * If IPv6 is enabled, open an AF_INET6 socket, which can accept both IPv4 and
 * IPv4 connections.  Otherwise, just open a regular AF_INET socket.
 */ 
#ifdef TM_USE_IPV6
            listenDesc = socket(AF_INET6, SOCK_DGRAM, IP_PROTOUDP);        
#else /* !TM_USE_IPV6 */
            listenDesc = socket(AF_INET, SOCK_DGRAM, IP_PROTOUDP);        
#endif /* TM_USE_IPV6 */
        

            if (listenDesc == TM_SOCKET_ERROR)
            {
                errorCode = TM_SOCKET_ERROR;
            }
            else
            {        
                tftpSrvPtr->fTftpdListenDesc = listenDesc;
/* Non blocking state */
                (void)tfBlockingState(listenDesc, TM_BLOCKING_OFF);
                addressLength = sizeof(struct sockaddr_storage);
                errorCode = getsockname(listenDesc,
                                        tempSockAddr.sockPtr,
                                        &addressLength);
                if (errorCode == TM_ENOERROR)
                {
/*
 * Need to set SO_REUSEADDR in case there is still an accept socket from a
 * previous server session that is still in time wait state.
 */
                    tempInt = 1;
                    errorCode = setsockopt( listenDesc,
                                            SOL_SOCKET,
                                            SO_REUSEADDR,
                                            (const char TM_FAR *)&tempInt,
                                            sizeof(int) );
                    if (errorCode == TM_ENOERROR)
                    {

#ifdef TM_USE_IPV6
/* IPv6 and (possibly) IPv4 */
                        sockAddress.ss_family = AF_INET6;
#else /* !TM_USE_IPV6 */
/* IPv4 only */
                        sockAddress.ss_family = AF_INET;
#endif /* TM_USE_IPV6 */

/* TFTP server Port number is already in network order. */
                        sockAddress.ss_port = TM_TFTPD_PORT; 
/* Bind to server port */
                        errorCode = bind( listenDesc,
                                          tempSockAddr.sockPtr,
                                          sizeof(struct sockaddr_storage));
                        if (errorCode == TM_ENOERROR)
                        {
/* Note: order of calls does not matter, since we own the execute lock */
/* Register CB_RECV */
                            errorCode = 
                                    tfRegisterSocketCB(listenDesc,
                                                       tfTftpdSignalRecvCtrl,
                                                       TM_CB_RECV);                                                       
                            if (errorCode == TM_ENOERROR)
                            {
#ifdef TM_PEND_POST_NEEDED
                                if (blockingState != TM_BLOCKING_OFF)
                                {
                                    tfTftpdExecute(tftpSrvPtr);
                                }
#endif /* TM_PEND_POST_NEEDED */
                            }
                        }
                    }
                }
            }
        
            if(errorCode != TM_ENOERROR)
            {
                errorCode = tfGetSocketError(listenDesc);
/* We unlock, and re-lock in this routine */
                if (listenDesc != TM_SOCKET_ERROR)
                {
                    tfTftpdCloseListen(tftpSrvPtr);
                }
            }
            tm_call_unlock(&tftpSrvPtr->fTftpdExecuteLockEntry);
        }
     }
     else
     {
         errorCode = TM_ENOENT;
     }
    return errorCode;
}



/*
 * tfTftpdUserStop function description:
 *  Begins shutdown of TFTP server.  Called by the user.
 *
 * Parameters       Meaning
 *  none             none
 *
 * Return values    Meaning
 *  none             none
 */
int tfTftpdUserStop (void)
{
    int             errorCode;
    ttTftpSrvPtr    tftpSrvPtr;

    tftpSrvPtr = (ttTftpSrvPtr)tm_context(tvTftpSrvPtr);
    if(tftpSrvPtr != (ttTftpSrvPtr)0)
    {
   
        tm_call_lock_wait(&tftpSrvPtr->fTftpdExecuteLockEntry);
        if (tftpSrvPtr->fTftpdListenDesc == TM_SOCKET_ERROR)
        {
            errorCode = TM_EALREADY;
        }
        else
        {
            tftpSrvPtr->fTftpdShutdown = 1;
            if (tftpSrvPtr->fTftpdBlockingState == TM_BLOCKING_OFF)
            {
/* clean up */
                tfTftpdExecute(tftpSrvPtr);
            }
#ifdef TM_PEND_POST_NEEDED
            else
            {
/*
 * Post in case there is no ftp connections (they would post on
 * TM_CB_RESET)
 */
                tm_post( &tftpSrvPtr->fTftpdPendSem,
                                &tftpSrvPtr->fTftpdExecuteLockEntry,
                                TM_ENOERROR );
            }
#endif /* TM_PEND_POST_NEEDED */
            errorCode = TM_ENOERROR;
        }
        tm_call_unlock(&tftpSrvPtr->fTftpdExecuteLockEntry);
    }
    else
    {
        errorCode = TM_ENOENT;
    }
    return errorCode;
}


/*
 * tfTftpdUserExecute function description:
 *  Called by the user in non-blocking mode in main line loop.  Locks, and 
 *  then calls tfTftpdExecute.
 *
 * Parameters       Meaning
 *  none             none
 *
 * Return values    Meaning
 *  TM_ENOERROR      Success.
 *  TM_EPERM         tfTftpdUserStart has not been called or tfTftpdUserExecute
 *                   is currently executing.
 *  TM_ENOENT        tfTftpdInit has not been called.
 */
int tfTftpdUserExecute (void)
{
    int errorCode;
    ttTftpSrvPtr    tftpSrvPtr;

    tftpSrvPtr = (ttTftpSrvPtr)tm_context(tvTftpSrvPtr);
    if(tftpSrvPtr != (ttTftpSrvPtr)0)
    {      

        tm_call_lock_wait(&tftpSrvPtr->fTftpdExecuteLockEntry);
        if ( (tftpSrvPtr->fTftpdListenDesc != TM_SOCKET_ERROR)
            && (tftpSrvPtr->fTftpdExecuting == TM_8BIT_ZERO) )
        {
/*
 * tfTftpdUserStart has been called, and nobody is executing the server main
 * loop
 */
            tfTftpdExecute(tftpSrvPtr);
            errorCode = TM_ENOERROR;
        }
        else
        {
/* tfTftpdUserStart has not been called, or tfTftpdUserExecute is executing */
            errorCode = TM_EPERM;
        }
        tm_call_unlock(&tftpSrvPtr->fTftpdExecuteLockEntry);
    }
    else
    {
        errorCode = TM_ENOENT;
    }

    return errorCode;
}


/*
 * tfTftpdExecute function description:
 *  We continue executing this loop until tlTftpExecuting is reset.
 *  1. Check if we are in the middle of a shutdown.  If so, close the listening
 *     socket, and close all files.
 *  2. Wait to recieve and process some data.
 *  3. If timeout occurs check to see if the server is still up.
 *  4. If TFTP server is still up, and non-blocking mode, then pend.
 *  5. Otherwise, return to user.
 *
 * Parameters       Meaning
 * tftpSrvPtr       TFTP server data structure pointer
 *
 * Return values    Meaning
 *  none             none
 */
static void tfTftpdExecute (ttTftpSrvPtr tftpSrvPtr)
{
    ttTftpdConnEntryPtr connEntryPtr;
    ttTftpdConnEntryPtr nextConnEntryPtr;
    ttTimerPtr          timerPtr;
    ttNodePtr           tftpdConQHeadNodePtr;
#if ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED)) )
    ttLockEntryPtr      tftpdExecuteLockEntryPtr;
#endif  /* ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED))  */
    int                 errorCode;
    tt8Bit              needFreeConnection;

#if ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED)) )
    tftpdExecuteLockEntryPtr = &tftpSrvPtr->fTftpdExecuteLockEntry;
#endif  /* ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED))  */

    tftpSrvPtr->fTftpdExecuting = TM_8BIT_YES;
    tftpdConQHeadNodePtr = &tftpSrvPtr->fTftpdConQHead.listHeadNode;
    do
    {
/* Check if there is any TFTPD work queued in */
        if (tftpSrvPtr->fTftpdShutdown != TM_8BIT_ZERO)
        {
/* Close the listening socket */
            tfTftpdCloseListen(tftpSrvPtr);
            connEntryPtr = (ttTftpdConnEntryPtr)(ttVoidPtr)
                                         (tftpdConQHeadNodePtr->nodeNextPtr);
            while (connEntryPtr
                   != (ttTftpdConnEntryPtr)(ttVoidPtr)tftpdConQHeadNodePtr)
            {
                tfTftpFreeConnection(tftpSrvPtr, connEntryPtr);
                connEntryPtr =
                  (ttTftpdConnEntryPtr)(ttVoidPtr)
                         (tftpdConQHeadNodePtr->nodeNextPtr);
            }
            tftpSrvPtr->fTftpdShutdown = 0;
        }

        while (tftpSrvPtr->fTftpdPendingRecv != 0)
        {
            tftpSrvPtr->fTftpdPendingRecv--;
            tm_call_unlock(tftpdExecuteLockEntryPtr);
            tfTftpdRecvCtrl(tftpSrvPtr);
            tm_call_lock_wait(tftpdExecuteLockEntryPtr);
        }
        connEntryPtr = (ttTftpdConnEntryPtr)(ttVoidPtr)
                                         (tftpdConQHeadNodePtr->nodeNextPtr);
        while ( connEntryPtr != 
                    (ttTftpdConnEntryPtr)(ttVoidPtr)tftpdConQHeadNodePtr)
        {
            nextConnEntryPtr = (ttTftpdConnEntryPtr)(ttVoidPtr)
                                        (connEntryPtr->fconNode.nodeNextPtr);
            if(connEntryPtr->fconCBFlag == TM_TFTP_CB_TIMEOUT)
            {
                connEntryPtr->fconCBFlag = 0;
                needFreeConnection = TM_8BIT_ZERO;
                connEntryPtr->fconTimeCount++;
                tm_debug_log1("fconTimeCount=%d", connEntryPtr->fconTimeCount);
                if(connEntryPtr->fconTimeCount >= tftpSrvPtr->fTftpdMaxTimeout)   /* time out */
                {
                    tm_debug_log0("time out");
                    needFreeConnection = TM_8BIT_YES;
                }
                else
                {                                           /* retry */
                    tm_debug_log0("retry");
                    timerPtr = connEntryPtr->fconTimerPtr;
                    if (timerPtr != TM_TMR_NULL_PTR)
                    {
                        tm_timer_remove(timerPtr);
                        connEntryPtr->fconTimerPtr = TM_TMR_NULL_PTR;
                    }
                    errorCode = tfTftpRetrySend(connEntryPtr, tftpSrvPtr);
                    if(errorCode != TM_ENOERROR )
                    {
                        needFreeConnection = TM_8BIT_YES;
                    }
                }
                if (needFreeConnection != TM_8BIT_ZERO)
                {
                    tfTftpFreeConnection(tftpSrvPtr, connEntryPtr);
                }
            }
            connEntryPtr = nextConnEntryPtr;
        }

/* No more work to do. Check whether the user has stopped the TFTP server */
        if (tftpSrvPtr->fTftpdListenDesc == TM_SOCKET_ERROR)
        {
/* User stopped the TFTP server. */
            tftpSrvPtr->fTftpdExecuting = TM_8BIT_ZERO;
        }
        else
/* No more work to do, and TFTP server still up, pend if allowed */
        {
#ifdef TM_PEND_POST_NEEDED
            if (tftpSrvPtr->fTftpdBlockingState != TM_BLOCKING_OFF)
            {
                (void)tm_pend(&tftpSrvPtr->fTftpdPendSem, 
                              tftpdExecuteLockEntryPtr);
            }
            else
#endif /* TM_PEND_POST_NEEDED */
            {
/* otherwise, exit the loop */
                tftpSrvPtr->fTftpdExecuting = TM_8BIT_ZERO;
            }
        }
    }
    while ( tftpSrvPtr->fTftpdExecuting != TM_8BIT_ZERO );    
}


/*
 * tfTftpdCloseListen function description:
 *  1. Unlock executeLock (since we are locked when called).
 *  2. Close the socket.
 *  3. Lock executeLock.
 *
 * Parameters       Meaning
 * tftpSrvPtr       TFTP server data structure pointer
 *
 * Return values    Meaning
 *  none             none
 */
static void tfTftpdCloseListen (ttTftpSrvPtr tftpSrvPtr)
{
    int             socketDesc; 
            
    tm_trace_func_entry(tfTftpdCloseListen); 

    socketDesc = tftpSrvPtr->fTftpdListenDesc;
    tftpSrvPtr->fTftpdListenDesc = TM_SOCKET_ERROR;
    tm_call_unlock(&tftpSrvPtr->fTftpdExecuteLockEntry);
/* Removes socket callback. */
    (void)tfRegisterSocketCB(socketDesc, (ttUserSocketCBFuncPtr)0, 0);
/* Close socket. */
    (void)tfClose(socketDesc);        
    tm_call_lock_wait(&tftpSrvPtr->fTftpdExecuteLockEntry);
    tftpSrvPtr->fTftpdPendingRecv = 0;
    
    tm_trace_func_exit(tfTftpdCloseListen);
}

#ifdef TM_LINT
LINT_UNREF_MEMBER_BEGIN
#endif /* TM_LINT */
struct tsFormat {
    char TM_FAR *                        fmtModePtr;
    void (TM_CODE_FAR * fmtSendFuncPtr) (ttTftpdConnEntryPtr ent, 
                                         ttTftpHdrPtr        pt, 
                                         int                 size, 
                                         ttTftpSrvPtr        tftpSrvPtr);
    void (TM_CODE_FAR * fmtRecvFuncPtr) (ttTftpdConnEntryPtr ent, 
                                         ttTftpHdrPtr        pt, 
                                         int                 size, 
                                         ttTftpSrvPtr        tftpSrvPtr);
};

static const struct tsFormat TM_CONST_QLF tlTftpdFormats[] = {
    { "NETASCII",   tfTftpXmitfile, tfTftpRecvfile},
    { "OCTET",      tfTftpXmitfile, tfTftpRecvfile},
    { 0, 0, 0 }
};
#ifdef TM_LINT
LINT_UNREF_MEMBER_END
#endif /* TM_LINT */

/*
 * tfTftpdSignalRecvCtrl function description:
 *  Signals the receieve semaphore that there is data available.
 *
 * Parameters       Meaning
 *  listenDesc       listen descriptor corresponding to this receieve 
 *  flags            not used
 *
 * Return values    Meaning
 *  none             none
 */
static void tfTftpdSignalRecvCtrl(int listenDesc, int flags)
{    
    ttTftpSrvPtr tftpSrvPtr;

    tm_trace_func_entry(tfTftpdSignalRecvCtrl);

    tftpSrvPtr = (ttTftpSrvPtr)tm_context(tvTftpSrvPtr);
    if (tftpSrvPtr != (ttTftpSrvPtr)0)
    { 
        TM_UNREF_IN_ARG(flags);
        tm_call_lock_wait(&tftpSrvPtr->fTftpdExecuteLockEntry);
        if (    (tftpSrvPtr->fTftpdShutdown == 0) 
             && (tftpSrvPtr->fTftpdListenDesc == listenDesc) )
        {
            tftpSrvPtr->fTftpdPendingRecv++;
#ifdef TM_PEND_POST_NEEDED
            if (tftpSrvPtr->fTftpdBlockingState != TM_BLOCKING_OFF)
            {
/* Post */
                tm_post( &tftpSrvPtr->fTftpdPendSem,
                               &tftpSrvPtr->fTftpdExecuteLockEntry,
                               TM_ENOERROR );
            }
#endif /* TM_PEND_POST_NEEDED */
        }
        tm_call_unlock(&tftpSrvPtr->fTftpdExecuteLockEntry);
    }
    tm_trace_func_exit(tfTftpdSignalRecvCtrl);
}


/*
 * tfTftpdRecvCtrl function description:
 *  Recieve callback function.
 *
 * Parameters       Meaning
 * tftpSrvPtr       TFTP server data structure pointer
 *
 * Return values    Meaning
 *  none             none
 */
#ifdef TM_LINT
LINT_NULL_PTR_BEGIN(cp)
#endif /* TM_LINT */
static void tfTftpdRecvCtrl(ttTftpSrvPtr tftpSrvPtr)
{
    struct sockaddr_storage TM_FAR * entryAddrPtr;    
    ttTftpdConnEntryPtr     connEntryPtr;
    struct sockaddr_storage peerAddress;
    struct sockaddr_storage localAddress;
    ttTftpHdrPtr            tp;
    ttCharPtr               rcvBufPtr;
    int                     fromlen;
    int                     size;
    int                     flg;
    int                     first;
    int                     errorCode;
    int                     retCode;
    ttCharPtr               cp;
    ttCharPtr               filename;
    ttCharPtr               mode;
    ttVoidPtr               file;
    const struct tsFormat TM_FAR * pf;
    ttSockAddrPtrUnion      peerSockAddr;
    ttSockAddrPtrUnion      localSockAddr;
    ttListPtr               tftpdConQHeadPtr; 
    ttNodePtr               tftpdConQHeadNodePtr;

    tm_trace_func_entry(tfTftpdRecvCtrl);

    peerSockAddr.sockNgPtr = &peerAddress;
    localSockAddr.sockNgPtr = &localAddress;
    fromlen = sizeof(struct sockaddr_storage);
    rcvBufPtr = tftpSrvPtr->fTftpdBuf;
    size = tfRecvFromTo(
            tftpSrvPtr->fTftpdListenDesc, rcvBufPtr,
            sizeof(tftpSrvPtr->fTftpdBuf), 0, 
            peerSockAddr.sockPtr, &fromlen, localSockAddr.sockPtr);

    tm_lock_wait(&tftpSrvPtr->fTftpdExecuteLockEntry);
    tftpdConQHeadPtr = &tftpSrvPtr->fTftpdConQHead;
    tftpdConQHeadNodePtr = &tftpdConQHeadPtr->listHeadNode;
    if ((size > 0) && (tftpSrvPtr->fTftpdShutdown == 0))
    {
        connEntryPtr = (ttTftpdConnEntryPtr)(ttVoidPtr)
                         (tftpdConQHeadNodePtr->nodeNextPtr);
        flg = 0;
        while ( connEntryPtr != (ttTftpdConnEntryPtr)(ttVoidPtr)
                                     tftpdConQHeadNodePtr )
        {
            entryAddrPtr = &connEntryPtr->fconPeerAddress;

#ifdef TM_USE_IPV6
/* IPv6 and (possibly) IPv4.  
 * IPv4 address stored in IPv4-compat IPv6 format. 
 */
            if (    (tm_6_ip_match(&entryAddrPtr->addr.ipv6.sin6_addr,
                                   &peerAddress.addr.ipv6.sin6_addr))
                 && (entryAddrPtr->ss_port == peerAddress.ss_port))
#else /* TM_USE_IPV6 */
/* IPv4 only */
            if (    (tm_ip_match(entryAddrPtr->addr.ipv4.sin_addr.s_addr,
                                 peerAddress.addr.ipv4.sin_addr.s_addr))
                 && (entryAddrPtr->ss_port== peerAddress.ss_port) )
#endif /* TM_USE_IPV6 */
            {
                flg = 1;
                break;
            }
            connEntryPtr = (ttTftpdConnEntryPtr)(ttVoidPtr)
                                        (connEntryPtr->fconNode.nodeNextPtr);
        }
        if(flg == 0)
        {
            tp = (ttTftpHdrPtr)rcvBufPtr;
            tp->th_opcode = ntohs(tp->th_opcode);
            if( (tp->th_opcode == TM_TFTP_RRQ) || 
                (tp->th_opcode == TM_TFTP_WRQ) )
            {
/* Avoid warnings by some compilers */                        
                mode=0;                 
            
                first = 1;
                filename = cp = (ttCharPtr)tp->th_stuff;
again:
                while(cp < rcvBufPtr + size)
                {
                    if(*cp == 0)
                    {
                        break;
                    }
                    cp++;
                }
                if(*cp != 0)
                {
                    tfTftpErrorSend(&peerAddress, 
                                    &localAddress, 
                                    TM_TFTP_EBADOP,
                                    tftpSrvPtr);
                    goto tftpdRecvCtrlExit;
                }
                if( first)
                {
                    mode = ++cp;
                    first = 0;
                    goto again;
                }
                for(cp = mode ; *cp; cp++)
                {
                    if(tm_islower(*cp))
                    {
                        *cp = (char)tm_toupper(*cp);
                    }
                }
                for(pf = tlTftpdFormats; pf->fmtModePtr; pf++)
                {
                    retCode = tm_strcmp(pf->fmtModePtr, mode);
                    if(retCode == 0)
                    {
                        break;
                    }
                }
                if(pf->fmtModePtr == 0)
                {
                    tfTftpErrorSend(&peerAddress, 
                                    &localAddress, 
                                    TM_TFTP_EBADOP, 
                                    tftpSrvPtr);
                    goto tftpdRecvCtrlExit;
                }
                if(tp->th_opcode == TM_TFTP_RRQ)
                {
                    retCode = tm_strcmp(pf->fmtModePtr, "NETASCII");
                    if(retCode == 0)
                    {
                        file = tfFSOpenFile((void TM_FAR*)0, filename, 
                            TM_FS_READ, TM_TYPE_ASCII, TM_STRU_STREAM );
                    }
                    else
                    {
                        file = tfFSOpenFile((void TM_FAR*)0, filename, 
                            TM_FS_READ, TM_TYPE_BINARY, TM_STRU_STREAM );
                    }
                    if(file == 0)   /* file open error */
                    {
                        tfTftpErrorSend(&peerAddress, 
                                        &localAddress, 
                                        TM_TFTP_ENOTFOUND, 
                                        tftpSrvPtr);
                    }
                }
                else
                {
                    retCode = tm_strcmp(pf->fmtModePtr, "NETASCII");
                    if(retCode == 0)
                    {
                        file = tfFSOpenFile((void TM_FAR*)0, filename, 
                            TM_FS_WRITE, TM_TYPE_ASCII, TM_STRU_STREAM );
                    }
                    else
                    {
                        file = tfFSOpenFile((void TM_FAR*)0, filename, 
                            TM_FS_WRITE, TM_TYPE_BINARY, TM_STRU_STREAM );
                    }
                    if(file == 0)   /* file allocation error */
                    {
                        tfTftpErrorSend(&peerAddress, 
                                        &localAddress, 
                                        TM_TFTP_ENOSPACE, 
                                        tftpSrvPtr);
                    }
                }

                if(file != 0)
                {
                    if(tftpdConQHeadPtr->listCount <= 
                               (unsigned int)tftpSrvPtr->fTftpdMaxConnections)
                    {
                        connEntryPtr = (ttTftpdConnEntryPtr)
                                 tm_get_raw_buffer(sizeof(ttTftpdConnEntry));
                        if ( connEntryPtr != (ttTftpdConnEntryPtr)0 )
                        {
                            tm_bzero(connEntryPtr,
                                     sizeof(ttTftpdConnEntry));
                            tfListAddToTail(
                                    tftpdConQHeadPtr,
                                    &connEntryPtr->fconNode);
                            connEntryPtr->fconPeerAddress = peerAddress;
                            connEntryPtr->fconLocalAddress = 
                                        localAddress;
                            connEntryPtr->fconFilePtr = file;
                            connEntryPtr->fconEndFlag = 0;
                            connEntryPtr->fconNewline = 0;
                            connEntryPtr->fconPrevchar = (char)-1;
                            connEntryPtr->fconCBFlag = 0;
                            retCode = tm_strcmp(pf->fmtModePtr, "NETASCII");
                            if(retCode == 0)
                            {
                                connEntryPtr->fconMode = 0;
                            }
                            else
                            {
                                connEntryPtr->fconMode = 1;
                            }
                            if(tp->th_opcode == TM_TFTP_RRQ)
                            {
                                connEntryPtr->fconBlockNum = 1;
                                connEntryPtr->fconFunction = 
                                        pf->fmtSendFuncPtr;
                                errorCode = tfTftpDataSend(connEntryPtr, 
                                                           &size,
                                                           tftpSrvPtr);                                  
                                if( ( size < TM_TFTP_SEGSIZE) || 
                                    (errorCode != TM_ENOERROR) )
                                {
                                    connEntryPtr->fconEndFlag = 1;
                                }
                            }
                            else
                            {
                                connEntryPtr->fconBlockNum = 0;
                                connEntryPtr->fconFunction = 
                                        pf->fmtRecvFuncPtr;
                                (void)tfTftpAckSend(connEntryPtr,
                                                    1, 
                                                    tftpSrvPtr);
                            }
                        }
                        else
                        {
                            tfTftpErrorSend(&peerAddress,
                                            &localAddress, 
                                            TM_TFTP_EUNDEF,
                                            tftpSrvPtr);
                        }
                    }
                    else
                    {
                        tfTftpErrorSend(&peerAddress, 
                                        &localAddress, 
                                        TM_TFTP_EUNDEF, 
                                        tftpSrvPtr);
                    }
                }
            }
            else
            {
                tfTftpErrorSend(
                    &peerAddress, &localAddress, TM_TFTP_EBADOP, tftpSrvPtr);
            }
        }
        else
        {
            (connEntryPtr->fconFunction)(connEntryPtr, 
                                (ttTftpHdrPtr)rcvBufPtr, size, tftpSrvPtr);
        }
    }

tftpdRecvCtrlExit:
    tm_unlock(&tftpSrvPtr->fTftpdExecuteLockEntry);    
    tm_trace_func_exit(tfTftpdRecvCtrl);
}
#ifdef TM_LINT
LINT_NULL_PTR_END(cp)
#endif /* TM_LINT */


/*
 * tfTftpXmitfile function description:
 *
 * Parameters       Meaning
 *  connEntryPtr     ptr to connection structure
 *  tp               ptr to packet header
 *  s                not used
 *  tftpSrvPtr       ptr to server data structure
 *
 * Return values    Meaning
 * None             None
 */
static void tfTftpXmitfile(ttTftpdConnEntryPtr connEntryPtr, 
                           ttTftpHdrPtr tp, 
                           int s, 
                           ttTftpSrvPtr tftpSrvPtr)
{
    int         size;
    int         errorCode;
    ttTimerPtr  timerPtr;
    tt8Bit      needFreeConnection;

    needFreeConnection = TM_8BIT_ZERO;
    tm_trace_func_entry(tfTftpXmitfile);

    TM_UNREF_IN_ARG(s);  /* not used */
    tp->th_opcode = ntohs(tp->th_opcode);
    tp->th_block = ntohs(tp->th_block);

    timerPtr = connEntryPtr->fconTimerPtr;
    if (timerPtr != TM_TMR_NULL_PTR)
    {
        tm_timer_remove(timerPtr);
        connEntryPtr->fconTimerPtr = TM_TMR_NULL_PTR;
    }
    if((tp->th_opcode==TM_TFTP_ACK) && 
        (connEntryPtr->fconBlockNum == tp->th_block))
    {
        if(connEntryPtr->fconEndFlag == 1)
        {
            needFreeConnection = TM_8BIT_YES;
        }
        else
        {
            connEntryPtr->fconBlockNum = 
                (tt16Bit)((connEntryPtr->fconBlockNum + 1) & 0xFFFF);
            errorCode = tfTftpDataSend(connEntryPtr, &size, tftpSrvPtr);
            if( (size < TM_TFTP_SEGSIZE) || (errorCode != TM_ENOERROR) )
            {
                connEntryPtr->fconEndFlag = 1;
            }
        }
    }
    else
    {
        if( (tp->th_opcode == TM_TFTP_ACK) &&
                 ((connEntryPtr->fconBlockNum - 1) == tp->th_block) )
        {
            errorCode = tfTftpRetrySend(connEntryPtr, tftpSrvPtr);
            if(errorCode != TM_ENOERROR )
            {
                needFreeConnection = TM_8BIT_YES;
            }
        }
        else
        {
            tfTftpErrorSend(
                &connEntryPtr->fconPeerAddress,
                &connEntryPtr->fconLocalAddress,
                TM_TFTP_EBADOP,
                tftpSrvPtr);
            needFreeConnection = TM_8BIT_YES;
        }
    }
    if (needFreeConnection != TM_8BIT_ZERO)
    {
        tfTftpFreeConnection(tftpSrvPtr, connEntryPtr);
    }
    tm_trace_func_exit(tfTftpXmitfile);
}

static void tfTftpFreeConnection ( ttTftpSrvPtr         tftpSrvPtr,
                                   ttTftpdConnEntryPtr  connEntryPtr )
{
    ttTimerPtr timerPtr;

    timerPtr = connEntryPtr->fconTimerPtr;
    if (timerPtr != TM_TMR_NULL_PTR)
    {
        tm_timer_remove(timerPtr);
        connEntryPtr->fconTimerPtr = TM_TMR_NULL_PTR;
    }
    (void)tfFSCloseFile((void TM_FAR*)0, connEntryPtr->fconFilePtr);
    tfListRemove(&tftpSrvPtr->fTftpdConQHead,
                 &connEntryPtr->fconNode);
    tm_free_raw_buffer(connEntryPtr);
}

/*
 * tfTftpRecvfile function description:
 *  Recieves file from client and writes it to the filesystem.
 *
 * Parameters       Meaning
 *  connEntryPtr     ptr to connection structure
 *  tp               ptr to packet header
 *  size             length of data to receieve
 *  tftpSrvPtr       ptr to server data structure
 *
 * Return values    Meaning
 *  none             none
 */
static void tfTftpRecvfile(ttTftpdConnEntryPtr  connEntryPtr, 
                           ttTftpHdrPtr         tp,
                           int                  size,
                           ttTftpSrvPtr         tftpSrvPtr)
{
    int                              cnt;
    int                              w_rslt;
    int                              retryFlag;
    char                             c;
    ttCharPtr                        p;
    int                              errorCode;
    ttTimerPtr                       timerPtr;
    tt8Bit                           needFreeConnection;
    struct sockaddr_storage TM_FAR * peerAddressPtr;
    struct sockaddr_storage TM_FAR * localAddressPtr;
    ttVoidPtr                        conFilePtr;
  
    tm_trace_func_entry(tfTftpRecvfile);

    needFreeConnection = TM_8BIT_NO;
    peerAddressPtr = &connEntryPtr->fconPeerAddress;
    localAddressPtr = &connEntryPtr->fconLocalAddress;
    conFilePtr = connEntryPtr->fconFilePtr;

    tp->th_opcode = ntohs(tp->th_opcode);
    tp->th_block = ntohs(tp->th_block);
    size = size - 4;
    retryFlag = 1;

    timerPtr = connEntryPtr->fconTimerPtr;
    if (timerPtr != TM_TMR_NULL_PTR)
    {
        tm_timer_remove(timerPtr);
        connEntryPtr->fconTimerPtr = TM_TMR_NULL_PTR;
    }

    if( (tp->th_opcode == TM_TFTP_DATA) && 
            (((connEntryPtr->fconBlockNum + 1) & 0xFFFF) == tp->th_block))
    {
        connEntryPtr->fconBlockNum = 
                (tt16Bit)((connEntryPtr->fconBlockNum + 1) & 0xFFFF);

        if(connEntryPtr->fconMode == 1)
        {
            if(size != 0)
            {
                w_rslt = tfFSWriteFile((void TM_FAR*)0,
                        (void TM_FAR*) conFilePtr, 
                        (char TM_FAR*)tp->th_data, size);

                if(w_rslt == TM_SOCKET_ERROR)
                {
                    tfTftpErrorSend(
                        peerAddressPtr, 
                        localAddressPtr,
                        TM_TFTP_ENOSPACE,
                        tftpSrvPtr);
                    needFreeConnection = TM_8BIT_YES;
                    goto tfTftpRecvfileEnd;
                }
            }
        }
        else
        {
            p = (ttCharPtr)tp->th_data;
            cnt = size;
            while(cnt)
            {
                cnt--;
                c = *p++;
                if(connEntryPtr->fconPrevchar == '\r')
                {
                    if(c == '\n')
                    {
                        w_rslt = tfFSWriteFile((void TM_FAR*)0, 
                                (void TM_FAR*)conFilePtr,
                                    (char TM_FAR*)&c, 1);
                        if(w_rslt == TM_SOCKET_ERROR)
                        {
                            tfTftpErrorSend(
                                peerAddressPtr, 
                                localAddressPtr,
                                TM_TFTP_ENOSPACE,
                                tftpSrvPtr);
                            needFreeConnection = TM_8BIT_YES;
                            goto tfTftpRecvfileEnd;
                        }
                        goto tfTftpRecvfileSkip;
                    }
                    else
                    {
                        if(c == '\0')
                        {
                            w_rslt = tfFSWriteFile((void TM_FAR*)0, 
                                (void TM_FAR*)conFilePtr, 
                                (char TM_FAR*)&connEntryPtr->fconPrevchar,1);
                            if(w_rslt == TM_SOCKET_ERROR)
                            {
                                tfTftpErrorSend(
                                    peerAddressPtr, 
                                    localAddressPtr,
                                    TM_TFTP_ENOSPACE,
                                    tftpSrvPtr);
                                needFreeConnection = TM_8BIT_YES;
                                goto tfTftpRecvfileEnd;
                            }
                            goto tfTftpRecvfileSkip;
                        }
                        else
                        {
                            w_rslt = tfFSWriteFile(
                                (void TM_FAR*)0, 
                                (void TM_FAR*)conFilePtr, 
                                (char TM_FAR*)&connEntryPtr->fconPrevchar,1);
                            if(w_rslt == TM_SOCKET_ERROR)
                            {
                                tfTftpErrorSend(
                                    peerAddressPtr, 
                                    localAddressPtr,
                                    TM_TFTP_ENOSPACE,
                                    tftpSrvPtr);
                                needFreeConnection = TM_8BIT_YES;
                                goto tfTftpRecvfileEnd;
                            }
                    
                        }
                    }
                }
                if(c != '\r')
                {
                    w_rslt = tfFSWriteFile((void TM_FAR*)0, 
                                    (void TM_FAR*)conFilePtr, 
                                    (char TM_FAR*)&c, 1);
                    if(w_rslt == TM_SOCKET_ERROR)
                    {
                        tfTftpErrorSend(
                            peerAddressPtr, 
                            localAddressPtr,
                            TM_TFTP_ENOSPACE,
                            tftpSrvPtr);
                        needFreeConnection = TM_8BIT_YES;
                        goto tfTftpRecvfileEnd;
                    }
                }
tfTftpRecvfileSkip:
                connEntryPtr->fconPrevchar = c;
            }
        }
        if(size < TM_TFTP_SEGSIZE)
        {
            retryFlag = 0;
        }
        errorCode = tfTftpAckSend(connEntryPtr, retryFlag, tftpSrvPtr);
        if( (size < TM_TFTP_SEGSIZE) || (errorCode != TM_ENOERROR) )
        {
            needFreeConnection = TM_8BIT_YES;
        }
    }
    else
    {
        if( (tp->th_opcode == TM_TFTP_DATA) && 
            (connEntryPtr->fconBlockNum == tp->th_block) )
        {
            errorCode = tfTftpRetrySend(connEntryPtr, tftpSrvPtr);
            if(errorCode != TM_ENOERROR )
            {
                needFreeConnection = TM_8BIT_YES;
            }
        }
        else
        {
            tfTftpErrorSend(
                peerAddressPtr,
                localAddressPtr,
                TM_TFTP_EBADOP,
                tftpSrvPtr);
            needFreeConnection = TM_8BIT_YES;
        }
    }
tfTftpRecvfileEnd:
    if (needFreeConnection != TM_8BIT_ZERO)
    {
        tfTftpFreeConnection(tftpSrvPtr, connEntryPtr);
    }     
    tm_trace_func_exit(tfTftpRecvfile);
}


/*
 * tfTftpAckSend function description:
 *  Send ACK to client.
 *
 * Parameters       Meaning
 *  connEntryPtr     ptr to connection structure
 *  retry            boolean value indicating whether this is a retransmission
 *  tftpSrvPtr       ptr to server data structure
 *
 * Return values    Meaning
 * None             None
 */
static int tfTftpAckSend(ttTftpdConnEntryPtr connEntryPtr, 
                         int retry, 
                         ttTftpSrvPtr tftpSrvPtr)
{
    ttTftpHdrPtr        tp;
    int                 retCode;
    int                 errorCode;
    ttSockAddrPtrUnion  peerSockAddr;
    ttSockAddrPtrUnion  localSockAddr;
    ttGenericUnion      timerParm1;

    tm_trace_func_entry(tfTftpdAckSend);

    peerSockAddr.sockNgPtr = &connEntryPtr->fconPeerAddress;
    localSockAddr.sockNgPtr = &connEntryPtr->fconLocalAddress;

    errorCode = TM_ENOERROR;
    tp = (ttTftpHdrPtr)connEntryPtr->fconSendBuf;
    tp->th_opcode = htons(TM_TFTP_ACK);
    tp->th_block = htons(connEntryPtr->fconBlockNum);
    retCode = tfSendToFrom(
        tftpSrvPtr->fTftpdListenDesc, (ttCharPtr) tp, 4, MSG_DONTWAIT, 
        peerSockAddr.sockPtr, sizeof(struct sockaddr_storage),
        localSockAddr.sockPtr);

    if(retCode != 4)
    {
        if (retCode == TM_SOCKET_ERROR)
        {
            errorCode = tfGetSocketError(tftpSrvPtr->fTftpdListenDesc);
            if (errorCode == TM_EIO)
            {
/* allow TFTP retries to occur, if device driver send failed */
                errorCode = TM_ENOERROR;
            }
        }
        else
        {
            errorCode = TM_EWOULDBLOCK;
        }
    }
    if (errorCode == TM_ENOERROR)
    {
        if(retry == 1)
        {
            connEntryPtr->fconSendSize = 4;
            connEntryPtr->fconTimeCount = 0;
            timerParm1.genVoidParmPtr = (ttVoidPtr)connEntryPtr;
            connEntryPtr->fconTimerPtr =
                tfTimerAdd( tfTftpdSignalTimeout,
                            timerParm1,
                            timerParm1, /* unused */
                            tftpSrvPtr->fTftpdIdleTimeout,
                            0);
        }
    }
    tm_trace_func_exit_rc(tfTftpdAckSend, errorCode);
    return errorCode;
}


/*
 * tfTftpDataSend function description:
 *  Reads file from file system and transmits it to the client.
 *
 * Parameters       Meaning
 *  connEntryPtr     ptr to connection structure
 *  size             length of data to send
 *  tftpSrvPtr       ptr to server data structure
 *
 * Return values    Meaning
 * None             None
 */
static int tfTftpDataSend(ttTftpdConnEntryPtr connEntryPtr, 
                          int TM_FAR *        size, 
                          ttTftpSrvPtr        tftpSrvPtr)
{
    ttTftpHdrPtr        tp;
    int                 i;
    char                c;
    ttCharPtr           p;
    int                 retCode;
    int                 errorCode;
    ttSockAddrPtrUnion  peerSockAddr;
    ttSockAddrPtrUnion  localSockAddr;
    ttGenericUnion      timerParm1;

    tm_trace_func_entry(tfTftpdDataSend);

    peerSockAddr.sockNgPtr = &connEntryPtr->fconPeerAddress;
    localSockAddr.sockNgPtr = &connEntryPtr->fconLocalAddress;

    errorCode = TM_ENOERROR;
    tp = (ttTftpHdrPtr)connEntryPtr->fconSendBuf;
    tp->th_opcode = htons(TM_TFTP_DATA);
    tp->th_block = htons(connEntryPtr->fconBlockNum);
    if(connEntryPtr->fconMode == 1)
    {
        *size = tfFSReadFile( (void TM_FAR*)0, 
                              (void TM_FAR*)connEntryPtr->fconFilePtr, 
                              (char TM_FAR*)tp->th_data,
                              TM_TFTP_SEGSIZE );
    }
    else
    {
        p = (ttCharPtr)tp->th_data;
        for(i=0; i<TM_TFTP_SEGSIZE; i++)
        {
            if(connEntryPtr->fconNewline)
            {
                if(connEntryPtr->fconPrevchar == '\n')
                    c = '\n';
                else
                    c = '\0';
                connEntryPtr->fconNewline = 0;
            }
            else
            {
                *size = tfFSReadFile((void TM_FAR*)0, 
                            (void TM_FAR*)connEntryPtr->fconFilePtr, 
                                (char TM_FAR*)&c, 1);
                if(*size == 0)
                    break;
                if( (c == '\n') || (c == '\r') )
                {
                    connEntryPtr->fconPrevchar = c;
                    c = '\r';
                    connEntryPtr->fconNewline = 1;
                } 
            }
            *p++ = c;
        }
        *size = (int)(p - (ttCharPtr)tp->th_data);
    }
    retCode = tfSendToFrom(
        tftpSrvPtr->fTftpdListenDesc, (ttCharPtr) tp, (*size+4), MSG_DONTWAIT, 
        peerSockAddr.sockPtr, sizeof(struct sockaddr_storage),
        localSockAddr.sockPtr);
    if(retCode != (*size+4))
    {
        if (retCode == TM_SOCKET_ERROR)
        {
            errorCode = tfGetSocketError(tftpSrvPtr->fTftpdListenDesc);
            if (errorCode == TM_EIO)
            {
/* allow TFTP retries to occur, if device driver send failed */
                errorCode = TM_ENOERROR;
            }
        }
        else
        {
            errorCode = TM_EWOULDBLOCK;
        }
    }
    if (errorCode == TM_ENOERROR)
    {
        connEntryPtr->fconSendSize = *size + 4;
        connEntryPtr->fconTimeCount = 0;
        timerParm1.genVoidParmPtr = (ttVoidPtr)connEntryPtr;
        connEntryPtr->fconTimerPtr =
            tfTimerAdd( tfTftpdSignalTimeout,
                        timerParm1,
                        timerParm1, /* unused */
                        tftpSrvPtr->fTftpdIdleTimeout,
                        0);
    }
    tm_trace_func_exit_rc(tfTftpdDataSend, errorCode);
    return errorCode;
}

/*
 * tfTftpErrorSend function description:
 *  Sends error packet to client.
 *
 * Parameters       Meaning
 *  FromAddress      client address
 *  error            error number to send to client
 *  tftpSrvPtr       ptr to server data structure
 *
 * Return values    Meaning
 * None             None
 */
static void tfTftpErrorSend(
    struct sockaddr_storage TM_FAR * peerAddressPtr,
    struct sockaddr_storage TM_FAR * localAddressPtr,
    int                              error,
    ttTftpSrvPtr                     tftpSrvPtr)
{
    ttTftpHdrPtr                  tp;
    int                           length;
    const struct tsErrmsg TM_FAR *pe;
    ttSockAddrPtrUnion            peerSockAddr;
    ttSockAddrPtrUnion            localSockAddr;

    tm_trace_func_entry(tfTftpdErrorSend);
 
    peerSockAddr.sockNgPtr = peerAddressPtr;
    localSockAddr.sockNgPtr = localAddressPtr;

    tp = (ttTftpHdrPtr)tftpSrvPtr->fTftpdBuf;
    tp->th_opcode = htons(TM_TFTP_ERR);
    tp->th_code = htons((unsigned)error);
    for(pe = tlTftpErrmsgs; pe->e_code >= 0; pe++)
        if(pe->e_code == error)
            break;
    if(pe->e_code < 0)
    {
        pe = tlTftpErrmsgs;
        tp->th_code = TM_TFTP_EUNDEF;
    }
    tm_strcpy(tp->th_msg, pe->e_msg);
    length = (int)tm_strlen(pe->e_msg);
    tp->th_msg[length] = '\0';
    length += 5;
    (void)tfSendToFrom(
        tftpSrvPtr->fTftpdListenDesc, (ttCharPtr)tp, length, MSG_DONTWAIT, 
        peerSockAddr.sockPtr, sizeof(struct sockaddr_storage),
        localSockAddr.sockPtr);
    
    tm_trace_func_exit(tfTftpdErrorSend);
}


/*
 * tfTftpRetrySend function description:
 *  Retransmits last packet sent, called from tfTftpdTimeout.
 *
 * Parameters       Meaning
 *  connEntryPtr     ptr to connection structure
 *  tftpSrvPtr       ptr to server data structure
 *
 * Return values    Meaning
 * None             None
 */
static int tfTftpRetrySend(ttTftpdConnEntryPtr connEntryPtr, 
                           ttTftpSrvPtr tftpSrvPtr)
{
    int                 retCode;
    int                 errorCode;
    ttSockAddrPtrUnion  peerSockAddr;
    ttSockAddrPtrUnion  localSockAddr;
    ttGenericUnion      timerParm1;   

    tm_trace_func_entry(tfTftpdRetrySend);

    peerSockAddr.sockNgPtr = &connEntryPtr->fconPeerAddress;
    localSockAddr.sockNgPtr = &connEntryPtr->fconLocalAddress;
    errorCode = TM_ENOERROR;
    retCode = tfSendToFrom(
        tftpSrvPtr->fTftpdListenDesc, (ttCharPtr)connEntryPtr->fconSendBuf, 
        connEntryPtr->fconSendSize, MSG_DONTWAIT, peerSockAddr.sockPtr, 
        sizeof(struct sockaddr_storage), localSockAddr.sockPtr);
    if(retCode != connEntryPtr->fconSendSize)
    {
        if (retCode == TM_SOCKET_ERROR)
        {
            errorCode = tfGetSocketError(tftpSrvPtr->fTftpdListenDesc);
            if (errorCode == TM_EIO)
            {
/* allow TFTP retries to occur, if device driver send failed */
                errorCode = TM_ENOERROR;
            }
        }
        else
        {
            errorCode = TM_EWOULDBLOCK;
        }
    }

    if (errorCode == TM_ENOERROR)
    {
        timerParm1.genVoidParmPtr = (ttVoidPtr)connEntryPtr;
        connEntryPtr->fconTimerPtr =
            tfTimerAdd( tfTftpdSignalTimeout,
                        timerParm1,
                        timerParm1, /* unused */
                        tftpSrvPtr->fTftpdIdleTimeout,
                        0);
    }
    tm_trace_func_exit_rc(tfTftpdRetrySend, errorCode);
    return  errorCode;
}


/*
 * tfTftpdSignalTimeout function description:
 *
 * Parameters       Meaning
 *  ParamPtr         ptr to connection structure
 *
 * Return values    Meaning
 * None             None
 */
void tfTftpdSignalTimeout(ttVoidPtr      timerBlockPtr,
                          ttGenericUnion userParm1,
                          ttGenericUnion userParm2)
{
    ttTftpdConnEntryPtr connEntryPtr;
    ttTftpSrvPtr        tftpSrvPtr;

    tm_trace_func_entry(tfTftpdSignalTimeout);

    tftpSrvPtr = (ttTftpSrvPtr)tm_context(tvTftpSrvPtr);
    if(tftpSrvPtr != (ttTftpSrvPtr)0)
    {
/* Avoid compiler warnings about unused parameters */
        TM_UNREF_IN_ARG(userParm2);
    
        connEntryPtr = userParm1.genVoidParmPtr;

        tm_lock_wait(&tftpSrvPtr->fTftpdExecuteLockEntry);
        if (    (connEntryPtr != (ttTftpdConnEntryPtr)0)
             && (connEntryPtr->fconTimerPtr == timerBlockPtr) )
        {
            connEntryPtr->fconTimerPtr = (ttTimerPtr)0;
            if (tftpSrvPtr->fTftpdShutdown == 0)
            {
                connEntryPtr->fconCBFlag = TM_TFTP_CB_TIMEOUT;
#ifdef TM_PEND_POST_NEEDED
                if (tftpSrvPtr->fTftpdBlockingState != TM_BLOCKING_OFF)
                {
/* Post */
                    tm_post( &tftpSrvPtr->fTftpdPendSem, 
                             &tftpSrvPtr->fTftpdExecuteLockEntry, 
                             TM_ENOERROR);
                }
#endif /* TM_PEND_POST_NEEDED */
            }
        }
        tm_unlock(&tftpSrvPtr->fTftpdExecuteLockEntry);
    }
    
    tm_trace_func_exit(tfTftpdSignalTimeout);
}

#ifdef TM_USE_STOP_TRECK
void tfTftpdDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                )
{
    if (tm_context_var(tvTftpSrvPtr) != (ttVoidPtr)0)
    {
        tm_kernel_free(tm_context(tvTftpSrvPtr)); 
        tm_context_var(tvTftpSrvPtr) = (ttVoidPtr)0;
    }
}
#endif /* TM_USE_STOP_TRECK */

#else /* !TM_USE_TFTPD */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */


/* To allow link for builds when TM_USE_TFTPD is not defined */
int tvTftpdDummy = 0;

#endif /* !TM_USE_TFTPD */
