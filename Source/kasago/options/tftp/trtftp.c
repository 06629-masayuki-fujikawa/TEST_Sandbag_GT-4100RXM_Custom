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
 * Description: TFTP Client Protocol
 * Filename: trtftp.c
 * Author: 
 * Date Created: 
 * $Source: source/trtftp.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:48:53JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_TFTP
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

#define TM_TFTP_PORT tm_const_htons(2000)

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
/* No error code present in packet */
#define TM_TFTP_EUNDEF     (tt16Bit)0  /* not defined */
#define TM_TFTP_ENOTFOUND  (tt16Bit)1  /* file not found */
#define TM_TFTP_EACCESS    (tt16Bit)2  /* access violation */
#define TM_TFTP_ENOSPACE   (tt16Bit)3  /* disk full or allocation exceeded */
#define TM_TFTP_EBADOP     (tt16Bit)4  /* illegal TFTP operation */
#define TM_TFTP_EBADID     (tt16Bit)5  /* unknown transfer ID */
#define TM_TFTP_EEXISTS    (tt16Bit)6  /* file already exists */
#define TM_TFTP_ENOUSER    (tt16Bit)7  /* no such user */

struct tsErrmsg {
    int       e_code;
    ttCharPtr e_msg;
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

struct tsTftpClient;

#ifdef TM_LINT
LINT_UNREF_MEMBER_BEGIN
#endif /* TM_LINT */
typedef struct tsTftpConnEntry
{
    ttTimerPtr                      fconTimerPtr;
    ttCharPtr                       fconBufferPtrTop;
    ttCharPtr                       fconBufferPtr;
    struct sockaddr_storage         fconRemoteAddress;    
    ttUser32Bit                     fconBufferSize;
    ttUser32Bit                     fconRcvdBytes;
    tt32Bit                         fconTimeCount;
    int                             fconSendSize;
    int                             fconCBFlag;
    tt16Bit                         fconEndFlag;
    tt16Bit                         fconBlockNum;
    tt16Bit                         fconMode;
    char                            fconTftpBuf[TM_TFTP_PKTSIZE];
    char                            fconPrevChar;
    tt8Bit                          fconNewLine;
    tt8Bit                          fconRequest;
} ttTftpConnEntry;
#ifdef TM_LINT
LINT_UNREF_MEMBER_END
#endif /* TM_LINT */

typedef ttTftpConnEntry TM_FAR * ttTftpConnEntryPtr;


/* TFTP client data */
typedef struct tsTftpClient
{
    ttTftpConnEntryPtr  fcliTftpConnHeadPtr;
/* Tftp connections Idle timeout */
    tt32Bit             fcliTftpIdleTimeout;
/* Tftp Retry Max Time */
    tt32Bit             fcliTftpRetry;
    ttUser32Bit         fcliTftpRetCode;
/* Pend counting semaphore for the FTPD */
    ttPendEntry         fcliTftpPendSem; 
#if ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED)) )
    ttLockEntry         fcliTftpExecuteLockEntry;
/* Tftp blocking or non blocking */
    int                 fcliTftpBlockingState;
#endif  /* ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED))  */
/* Tftp listening descriptor */
    int                 fcliTftpListenDesc;
/* Tftp Recv Pending Num*/
    int                 fcliTftpPendingRecv;
/* Indicates whether the TFTP is executing */
    tt8Bit              fcliTftpExecuting;
/* Indicates whether a TFTP transfer (put or get) is in progress */
    tt8Bit              fcliTftpTransfer;
} ttTftpClient;

typedef ttTftpClient  TM_FAR * ttTftpClientPtr;

/*
 * TFTP Packet Formats (RFC 1350):
 *
 * RRQ/WRQ Packet:
 *
 *     2 bytes    string    1 byte     string    1 byte
 *   ----------------------------------------------------
 *   | opcode | filename |    0    |   Mode   |    0    |
 *   ----------------------------------------------------
 *
 * DATA Packet:
 *
 *     2 bytes    2 bytes      n bytes
 *   ------------------------------------
 *   | opcode  |  block #  |  data      |
 *   ------------------------------------
 *
 * ACK Packet:
 * 
 *     2 bytes    2 bytes 
 *   ----------------------
 *   | opcode  |  block # |
 *   ----------------------
 *
 * ERROR Packet:
 *
 *     2 bytes    2 bytes       string     1 byte
 *   ----------------------------------------------
 *   | opcode  | errorcode |  errormesg  |   0    |
 *   ---------------------------------------------- 
 *
 */  


typedef struct  tsTftpHdr {
    tt16Bit th_opcode;      /* packet type */
    union {
        tt16Bit tu_block;   /* block # */
        tt16Bit tu_code;    /* error code */
        ttUChar tu_stuff[1];    /* request packet stuff */
    } th_u;
    ttUChar th_data[1];     /* data or error string */
} ttTftpHdr;

#define th_block    th_u.tu_block
#define th_code     th_u.tu_code
#define th_stuff    th_u.tu_stuff
#define th_msg      th_data

typedef ttTftpHdr TM_FAR * ttTftpHdrPtr;

static void tfTftpXmitFile(ttTftpClientPtr tftpClientPtr);
static void tfTftpRecvFile(ttTftpClientPtr tftpClientPtr, int rcvdBytes);
static int  tfTftpAckSend(int retry, ttTftpClientPtr tftpClientPtr);
static int  tfTftpDataSend(ttIntPtr        sizePtr,
                           ttTftpClientPtr tftpClientPtr);
static void tfTftpCloseListen(ttTftpClientPtr tftpClientPtr,
                              ttUser32Bit     retCode,
                              tt8Bit          lockOn);
static void tfTftpSignalRecvCtrl(int listenDesc, int flags);
static void tfTftpExecute (ttTftpClientPtr tftpClientPtr);
static int  tfTftpRetrySend(ttTftpClientPtr tftpClientPtr);
static void tfTftpErrorSend(ttTftpClientPtr tftpClientPtr,
                            tt16Bit         netErrorCode); 
static ttUser32Bit tfTftpUserError(tt16Bit netError);
static ttUser32Bit tfTftpStartTransfer(char TM_FAR *            filename, 
                                       struct sockaddr TM_FAR * remoteAddrPtr,
                                       char TM_FAR *            tftpbuf, 
                                       ttUser32Bit              bufsize, 
                                       unsigned short           mode,
                                       int                      blocking,
                                       int                      request );
void tfTftpSignalTimeout (ttVoidPtr       timerBlockPtr,
                          ttGenericUnion  userParm1,
                          ttGenericUnion  userParm2);

/*
 * tfTftpInit function description:
 * 1. Initialize module variables and the execute lock.
 *
 * Parameters       Meaning
 * None             None
 *
 * Return values    Meaning
 * None             None
 */
void tfTftpInit (void)
{    
    ttTftpClientPtr tftpClientPtr;    

    if (tm_context(tvTftpClientPtr) == (ttVoidPtr)0)
    {
/* Permanent allocation */
        tftpClientPtr = 
                (ttTftpClientPtr)tm_kernel_malloc(sizeof(ttTftpClient));
        if (tftpClientPtr != (ttTftpClientPtr)0)
        {
            tm_context(tvTftpClientPtr) = (ttVoidPtr)tftpClientPtr;
            tm_bzero((ttVoidPtr)tftpClientPtr, sizeof(ttTftpClient));
            tftpClientPtr->fcliTftpListenDesc = TM_SOCKET_ERROR;
            tftpClientPtr->fcliTftpIdleTimeout = 
                    (tt32Bit)((tt32Bit)TM_TFTP_TIMEOUT_VAL*TM_UL(1000));
            tftpClientPtr->fcliTftpRetry = TM_TFTP_RETRY_TIME; 
/* fcliTftpTransfer, fcliTftpExecuting, fcliTftpConnHeadPtr and the execute
 * lock doesn't need to be reset since it's already zero because the tm_bzero
 * call above
 */
        }
    } 
}


/* 
 * tfTftpUserError
 * Given an error code from a TFTP error message, map it to a user
 * error code that will be returned to the user, and return it.
 * 
 * RETURNS:
 *      TM_TFTP_UNDEF       not defined
 *      TM_TFTP_NOTFOUND    file not found
 *      TM_TFTP_ACCESS      access violation
 *      TM_TFTP_NOSPACE     disk full or allocation exceeded
 *      TM_TFTP_BADOP       illegal TFTP operation
 *      TM_TFTP_BADID       unknown transfer ID
 *      TM_TFTP_FILEEXISTS  file already exists
 *      TM_TFTP_NOUSER      no such user
 *
 */
static ttUser32Bit tfTftpUserError(tt16Bit netError)
{
    ttUser32Bit retCode;

    switch(netError)
    {
        case TM_TFTP_EUNDEF:
            retCode = TM_TFTP_UNDEF;
            break;
        case TM_TFTP_ENOTFOUND:
            retCode = TM_TFTP_NOTFOUND;
            break;
        case TM_TFTP_EACCESS:
            retCode = TM_TFTP_ACCESS;
            break;
        case TM_TFTP_ENOSPACE:
            retCode = TM_TFTP_NOSPACE;
            break;
        case TM_TFTP_EBADOP:
            retCode = TM_TFTP_BADOP;
            break;
        case TM_TFTP_EBADID:
            retCode = TM_TFTP_BADID;
            break;
        case TM_TFTP_EEXISTS:
            retCode = TM_TFTP_FILEEXISTS;
            break;
        case TM_TFTP_ENOUSER:
            retCode = TM_TFTP_NOUSER;
            break;
        default:
            retCode = TM_TFTP_ERROR;
    }
    return retCode;
}

/*
 * tfTftpGet function description:
 *  1. Check for valid parameters:
 *       a. mode is either ascii or binary
 *       b. valid blocking mode (on or off)
 *       c. if user wants blocking make sure that pending/posting is enabled
 *     If any of these checks fail, return TM_EINVAL.
 *  2. Lock on TftpExecute.
 *  3. Make sure that no one else is in the middle of TFTP session 
 *  4. Indicate that we're in the middle of a TFTP session 
 *  5. Open and bind to a listening socket, and register recieve callback.
 
 *  7. If we're in blocking mode, call tfTftpExecute, which will return only
 *     when we're done with this connection.  If in non-blocking mode, return
 *     immediately, and allow the user to call tfTftpUserExecute.  
 *  8. Close listening socket.
 *
 * Parameters       Meaning
 * filename         name of file to get from peer
 * remote_addr      address of remote peer
 * tftpbuf          buffer to recieve file into
 * bufsize          size of tftpbuf
 * mode             TM_TYPE_ASCII or TM_TYPE_BINARY
 * blocking         TM_BLOCKING_ON or TM_BLOCKING_OFF
 *
 * Return value
 * amount of data (in bytes) received from peer.
 */
ttUser32Bit tfTftpGet(char TM_FAR *            filename, 
                      struct sockaddr TM_FAR * remoteAddrPtr,
                      char TM_FAR *            tftpbuf, 
                      ttUser32Bit              bufsize, 
                      unsigned short           mode,
                      int                      blocking)
{
    return tfTftpStartTransfer(filename, remoteAddrPtr, tftpbuf, bufsize, mode,
                               blocking, TM_TFTP_RRQ);
}


/*
 * tfTftpPut function description:
 *  1. Check for valid parameters:
 *       a. mode is either ascii or binary
 *       b. valid blocking mode (on or off)
 *       c. if user wants blocking make sure that pending/posting is enabled
 *     If any of these checks fail, return TM_EINVAL.
 *  2. Lock on TftpExecute.
 *  3. Make sure that no one else is in the middle of TFTP session 
 *  4. Indicate that we're in the middle of a TFTP session 
 *  5. Open and bind to a listening socket, and register recieve callback.
 *  6. Send write request (WRQ) to peer.
 *  7. If we're in blocking mode, call tfTftpExecute, which will return only
 *     when we're done with this connection.  If in non-blocking mode, return
 *     immediately, and allow the user to call tfTftpUserExecute. 
 *  8. Close listening socket.
 *
 * Parameters       Meaning
 *  filename         name of file to send to server
 *  remote_addr      address of remote server 
 *  tftpbuf          buffer containing the data to transmit
 *  bufsize          size of tftpbuf (amount of data to send)
 *  mode             TM_TYPE_ASCII or TM_TYPE_BINARY
 *  blocking         TM_BLOCKING_ON or TM_BLOCKING_OFF
 *
 * Return value
 *  amount of data sent to remote server
 */
ttUser32Bit tfTftpPut(char TM_FAR *            filename, 
                      struct sockaddr TM_FAR * remoteAddrPtr,
                      char TM_FAR *            tftpbuf, 
                      ttUser32Bit              bufsize, 
                      unsigned short           mode,
                      int                      blocking)
{
    return tfTftpStartTransfer(filename, remoteAddrPtr, tftpbuf, bufsize, mode,
                        blocking, TM_TFTP_WRQ);
}

/*
 * tfTftpStartTransfer function description:
 *  1. Check for valid parameters:
 *       a. mode is either ascii or binary
 *       b. valid blocking mode (on or off)
 *       c. if user wants blocking make sure that pending/posting is enabled
 *     If any of these checks fail, return TM_EINVAL.
 *  2. Lock on TftpExecute.
 *  3. Make sure that no one else is in the middle of TFTP session 
 *  4. Indicate that we're in the middle of a TFTP session 
 *  5. Open and bind to a listening socket, and register receive callback.
 *  6. Send read request (RRQ) (GET) or write request (WRQ) (PUT) to peer.
 *  7. If we're in blocking mode, call tfTftpExecute, which will return only
 *     when we're done with this connection.  If in non-blocking mode, return
 *     immediately, and allow the user to call tfTftpUserExecute. 
 *  8. Close listening socket if done (blocking mode) or if an error occured.
 *
 * Parameters       Meaning
 *  filename         name of file to send to server
 *  remote_addr      address of remote server 
 *  tftpbuf          buffer containing the data to transmit
 *  bufsize          size of tftpbuf (amount of data to send)
 *  mode             TM_TYPE_ASCII or TM_TYPE_BINARY
 *  blocking         TM_BLOCKING_ON or TM_BLOCKING_OFF
 *
 * Return value
 *  amount of data sent to remote server
 */
static ttUser32Bit tfTftpStartTransfer(char TM_FAR *     filename, 
                                struct sockaddr TM_FAR * remoteAddrPtr,
                                char TM_FAR *            tftpbuf, 
                                ttUser32Bit              bufsize, 
                                unsigned short           mode,
                                int                      blocking,
                                int                      request )
{
    ttTftpConnEntryPtr  tftpConnEntryPtr;
    ttUser32Bit         retCode;
    ttCharPtr           cp;
    ttTftpHdrPtr        tp;
    ttTftpClientPtr     tftpClientPtr; 
#ifdef TM_LOCK_NEEDED
    ttLockEntryPtr      lockEntryPtr;
#endif /* TM_LOCK_NEEDED */
    struct sockaddr_storage sockAddress;
    ttSockAddrPtrUnion  tempSockAddr;
    ttGenericUnion      timerParm1;
    int                 listenDesc;
    int                 addressLength;
    int                 tempInt;
    int                 size;
    int                 reqSentBytes;
    int                 errorCode;

    tftpClientPtr = (ttTftpClientPtr)tm_context(tvTftpClientPtr);

    if (tftpClientPtr != (ttTftpClientPtr)0)
    {
#ifdef TM_LOCK_NEEDED
        lockEntryPtr = &tftpClientPtr->fcliTftpExecuteLockEntry;
#endif /* TM_LOCK_NEEDED */

/*
 *  1. Check for valid parameters:
 *       a. mode is either ascii or binary
 *       b. valid blocking mode (on or off)
 *       c. if user wants blocking make sure that pending/posting is enabled
 *     If any of these checks fail, return TM_EINVAL.
 */
        if ( ((mode != TM_TYPE_ASCII) && (mode != TM_TYPE_BINARY)) ||
             ((blocking != TM_BLOCKING_ON) && (blocking != TM_BLOCKING_OFF)) ||
             (    (blocking == TM_BLOCKING_ON) 
               && (tvPendPostEnabled == TM_8BIT_ZERO)) ||
             (remoteAddrPtr == (struct sockaddr TM_FAR *)0) )
        {
            retCode = TM_TFTP_EINVAL;
            goto tftpTransferReturn;
        }
        if ( (remoteAddrPtr->sa_len > sizeof(struct sockaddr_storage)) ||
             (remoteAddrPtr->sa_len == 0) )
        {
            retCode = TM_TFTP_EINVAL;
            goto tftpTransferReturn;
        }
        
#ifdef TM_LOCK_NEEDED
/*  2. Lock on TftpExecute. */  
        tm_call_lock_wait(&tftpClientPtr->fcliTftpExecuteLockEntry);
#endif /* TM_LOCK_NEEDED */

/* 
 * 3. Make sure that no one else is in the middle of a TFTP session. Check
 *    needs to be made with lock on to prevent race conditions.
 */
        if(tftpClientPtr->fcliTftpTransfer == 1)
        {
            retCode = TM_TFTP_EXECUT;
            tm_call_unlock(lockEntryPtr);
            goto tftpTransferReturn;
        }

/*  4. Indicate that we're in the middle of a TFTP session  */
        tftpClientPtr->fcliTftpTransfer = 1;
        tftpClientPtr->fcliTftpExecuting = TM_8BIT_YES;
#ifdef TM_PEND_POST_NEEDED    
        tftpClientPtr->fcliTftpBlockingState = blocking;
#endif /* TM_PEND_POST_NEEDED */    

/*  5. Open and bind to a listening socket, and register receive callback. */
        listenDesc = socket(remoteAddrPtr->sa_family, SOCK_DGRAM, IP_PROTOUDP);
/* Initialize before unlocking */
        tftpClientPtr->fcliTftpListenDesc = listenDesc;
        tm_call_unlock(&tftpClientPtr->fcliTftpExecuteLockEntry);
        if (listenDesc != TM_SOCKET_ERROR)
        {
/* Non blocking state */
            (void)tfBlockingState(listenDesc, TM_BLOCKING_OFF);
            
            addressLength = sizeof(struct sockaddr_storage);
            tempSockAddr.sockNgPtr = &sockAddress;        
            errorCode = getsockname(listenDesc,
                                    tempSockAddr.sockPtr,
                                    &addressLength);
            if (errorCode == TM_ENOERROR)
            {
                tempInt = 1;
                errorCode = setsockopt(
                    listenDesc,
                    SOL_SOCKET,
                    SO_REUSEADDR,
                    (const char TM_FAR *)&tempInt,
                    sizeof(int) );
                if (errorCode == TM_ENOERROR)
                {
/* TFTP client port */
                    sockAddress.ss_family = remoteAddrPtr->sa_family;
                    sockAddress.ss_port = TM_TFTP_PORT;
/* Bind to client port */
                    tempSockAddr.sockNgPtr = &sockAddress;                
                    errorCode = bind( listenDesc,
                                      tempSockAddr.sockPtr,
                                      sockAddress.ss_len );
                    if (errorCode == TM_ENOERROR)
                    {
                        tftpConnEntryPtr = (ttTftpConnEntryPtr)
                                   tm_get_raw_buffer(sizeof(ttTftpConnEntry));
                        if( tftpConnEntryPtr != (ttTftpConnEntryPtr)0)
                        {
                            tm_bzero(tftpConnEntryPtr,
                                     sizeof(ttTftpConnEntry));
                            tm_bcopy( remoteAddrPtr,
                                      &tftpConnEntryPtr->fconRemoteAddress,
                                      remoteAddrPtr->sa_len );
                            tftpConnEntryPtr->fconBufferPtr = tftpbuf;
                            tftpConnEntryPtr->fconBufferSize = bufsize;
                            tftpConnEntryPtr->fconPrevChar = (char)-1;
                            tftpConnEntryPtr->fconMode = mode;
                            tftpConnEntryPtr->fconRequest = (tt8Bit)request;
                            tftpClientPtr->fcliTftpConnHeadPtr =
                                                             tftpConnEntryPtr;
                            timerParm1.genVoidParmPtr = 
                                    (ttVoidPtr) tftpConnEntryPtr;
                            tftpConnEntryPtr->fconTimerPtr = 
                                tfTimerAdd( tfTftpSignalTimeout,
                                            timerParm1,
                                            timerParm1, /* unused */
                                            tftpClientPtr->fcliTftpIdleTimeout,
                                            0);                        
                            errorCode = tfRegisterSocketCB(listenDesc,
                                            tfTftpSignalRecvCtrl,
                                            TM_CB_RECV);
                            if (errorCode == TM_ENOERROR)
                            {
                            
/*  6. Send write/read request (request == WRQ/RRQ) to peer. */
                                tp = (ttTftpHdrPtr)
                                        tftpConnEntryPtr->fconTftpBuf;
                                tp->th_opcode = htons((tt16Bit)request);
                                cp = (ttCharPtr)tp->th_stuff;
                                tm_strcpy(cp, filename);
                                cp += tm_strlen(filename);
                                cp++;
                                if(mode == TM_TYPE_ASCII)
                                {
                                    tm_strcpy(cp, "NETASCII");
                                    cp += tm_strlen("NETASCII");
                                }
                                else
                                {
                                    tm_strcpy(cp, "OCTET");
                                    cp += tm_strlen("OCTET");
                                }
                                cp++;
                                size = (int)(cp - (ttCharPtr)tp);
                                tftpConnEntryPtr->fconSendSize = size;
                                reqSentBytes = sendto( listenDesc,
                                                       (ttCharPtr)tp, 
                                                       size,
                                                       MSG_DONTWAIT,
                                                       remoteAddrPtr,
                                                       remoteAddrPtr->sa_len );
                                if(reqSentBytes == size)
                                {
                                    retCode = TM_ENOERROR;
/*
 *  7. If we're in blocking mode, call tfTftpExecute, which will return only
 *     when we're done with this connection.  If in non-blocking mode, return
 *     immediately, and allow the user to call tfTftpUserExecute. 
 */
#ifdef TM_PEND_POST_NEEDED
                                    if (blocking != TM_BLOCKING_OFF)
                                    {
#ifdef TM_LOCK_NEEDED
                                        tm_call_lock_wait(lockEntryPtr);
#endif /* TM_LOCK_NEEDED */
                                        tfTftpExecute(tftpClientPtr);
                                        retCode =
                                               tftpClientPtr->fcliTftpRetCode;
#ifdef TM_LOCK_NEEDED
                                        tm_call_unlock(lockEntryPtr);
#endif /* TM_LOCK_NEEDED */
/* Return */
                                    }
#endif /* TM_PEND_POST_NEEDED */
                                    goto tftpTransferReturn;
                                }
                            }
                        }
                    }
                }
            }
        }
        retCode = TM_TFTP_ESOCK; /* socket error occured */
        if (listenDesc != TM_SOCKET_ERROR)
        {
/*  8. Close listening socket. */
/* Call tfTftpCloseListen with lock off */
            tfTftpCloseListen(tftpClientPtr, retCode, TM_8BIT_ZERO);
        }
tftpTransferReturn:  
#ifdef TM_LOCK_NEEDED
        tm_call_lock_wait(lockEntryPtr);
#endif /* TM_LOCK_NEEDED */
        tftpClientPtr->fcliTftpExecuting = TM_8BIT_NO;
#ifdef TM_LOCK_NEEDED
        tm_call_unlock(lockEntryPtr);
#endif /* TM_LOCK_NEEDED */
    }
    else
    {
        retCode = TM_TFTP_EINVAL;
    }
    return retCode;
}

/*
 * tfTftpSignalRecvCtrl function description:
 *  This routine is called when data is recieved.
 *  1. Lock executeLockEntry.
 *  2. If we're in blocking state, post to wake up recieve processing
 *     task.
 *  3. Unlock executeLockEntry.
 *
 * Parameters       Meaning
 *  listenDesc      listening socket corresponding to this signal
 *  flags            
 *
 * Return values    Meaning
 * None             None
 */
static void tfTftpSignalRecvCtrl(int listenDesc, int flags)
{
    
    ttTftpClientPtr tftpClientPtr;

    tftpClientPtr = (ttTftpClientPtr)tm_context(tvTftpClientPtr);

    if (tftpClientPtr != (ttTftpClientPtr)0)
    {
        TM_UNREF_IN_ARG(flags);

        tm_call_lock_wait(&tftpClientPtr->fcliTftpExecuteLockEntry);
        if(tftpClientPtr->fcliTftpListenDesc == listenDesc)
        {
            tftpClientPtr->fcliTftpPendingRecv++;
#ifdef TM_PEND_POST_NEEDED
            if (tftpClientPtr->fcliTftpBlockingState != TM_BLOCKING_OFF)
            {
/* Post */
                tm_post( &tftpClientPtr->fcliTftpPendSem,
                         &tftpClientPtr->fcliTftpExecuteLockEntry,
                         TM_ENOERROR );
            }
#endif /* TM_PEND_POST_NEEDED */
        }
        tm_call_unlock(&tftpClientPtr->fcliTftpExecuteLockEntry); 
    }
}


/*
 * tfTftpUserExecute function description:
 *  Execute the TFTP client main loop. Call valid only if tfTftpUserStart had
 *  been called in non blocking mode, and nobody is currently executing the
 *  TFTP client code main loop.
 *
 * Parameters       Meaning
 * None             None
 *
 * Return value
 *  error code 
 */
ttUser32Bit tfTftpUserExecute (void)
{
    ttUser32Bit         retCode;
    ttTftpClientPtr     tftpClientPtr;

    tftpClientPtr = (ttTftpClientPtr)tm_context(tvTftpClientPtr);

    if (tftpClientPtr != (ttTftpClientPtr)0)
    {
        tm_call_lock_wait(&tftpClientPtr->fcliTftpExecuteLockEntry);
        if(tftpClientPtr->fcliTftpTransfer == 1)
        {
/* Transfer not done yet */
            if (    (tftpClientPtr->fcliTftpListenDesc != TM_SOCKET_ERROR)
                 && (tftpClientPtr->fcliTftpExecuting == TM_8BIT_ZERO) )
            {
                tfTftpExecute(tftpClientPtr);
            }
            retCode = TM_TFTP_EXECUT; /* in progress */
        }
        else
        {
/* Return code from finished transfer */
            retCode = tftpClientPtr->fcliTftpRetCode;
        }
        tm_call_unlock(&tftpClientPtr->fcliTftpExecuteLockEntry);
    }
    else
    {
        retCode = TM_TFTP_EINVAL; /* Init not called */
    }
    return retCode;
}

/*
 * tfTftpExecute function description:
 *  Performs all TFTP processing.  If in blocking mode, this routine will exit
 *  only when we're done with the current transfer.  If in non-blocking mode,
 *  we'll return immediately.
 *
 * Parameters       Meaning
 * tftpClientPtr    ptr to client data structure      
 *
 * Return values    Meaning
 * None             None
 */
static void tfTftpExecute (ttTftpClientPtr tftpClientPtr)
{    
    ttTftpConnEntryPtr  tftpConnEntryPtr;
    ttSockAddrPtrUnion  tempSockAddr;
    ttTimerPtr          timerPtr;
    int                 rcvdBytes;
    int                 fromlen;
    int                 errorCode;
    int                 request;

    tftpConnEntryPtr = tftpClientPtr->fcliTftpConnHeadPtr; 
    tftpClientPtr->fcliTftpExecuting = TM_8BIT_YES;
    do
    {
        while (tftpClientPtr->fcliTftpPendingRecv != 0)
        {
            tftpClientPtr->fcliTftpPendingRecv--;
            request = (int)tftpConnEntryPtr->fconRequest;
            timerPtr = tftpConnEntryPtr->fconTimerPtr;
            if (timerPtr != (ttTimerPtr)0)
            {
                tm_timer_remove(timerPtr);
                tftpConnEntryPtr->fconTimerPtr = TM_TMR_NULL_PTR;
            }
            tftpConnEntryPtr->fconCBFlag = 0;
            tm_call_unlock(&tftpClientPtr->fcliTftpExecuteLockEntry);

            tempSockAddr.sockNgPtr = &tftpConnEntryPtr->fconRemoteAddress;

            fromlen = sizeof(struct sockaddr_storage);
            rcvdBytes = recvfrom( tftpClientPtr->fcliTftpListenDesc,
/* Received TFTP packet */
                                 tftpConnEntryPtr->fconTftpBuf,
                                 sizeof(tftpConnEntryPtr->fconTftpBuf),
                                 0,
                                 tempSockAddr.sockPtr, 
                                 &fromlen );
            if (rcvdBytes < 4)
            {
                tfTftpCloseListen(tftpClientPtr, TM_TFTP_ERROR, TM_8BIT_ZERO);
            }
            else
            {
                if (request == TM_TFTP_RRQ)
                {
                    tfTftpRecvFile(tftpClientPtr, rcvdBytes);
                }
                else
                {
                    tfTftpXmitFile(tftpClientPtr);
                }
            }
            tm_call_lock_wait(&tftpClientPtr->fcliTftpExecuteLockEntry);
        }
        if (     (tftpClientPtr->fcliTftpListenDesc != TM_SOCKET_ERROR)
             &&  (tftpConnEntryPtr->fconCBFlag == TM_TFTP_CB_TIMEOUT) )
        {
            tftpConnEntryPtr->fconCBFlag = 0;
            tftpConnEntryPtr->fconTimeCount++;
            if(tftpConnEntryPtr->fconTimeCount > tftpClientPtr->fcliTftpRetry)  /* time out  */
            {
/* Call tfTftpCloseListen with lock on */
                tfTftpCloseListen(tftpClientPtr, TM_TFTP_TIMEOUT, TM_8BIT_YES);
            }
            else
            {
/* retry */
                timerPtr = tftpConnEntryPtr->fconTimerPtr;
                if (timerPtr != (ttTimerPtr)0)
                {
                    tm_timer_remove(timerPtr);
                    tftpConnEntryPtr->fconTimerPtr = TM_TMR_NULL_PTR;
                }
                tm_call_unlock(&tftpClientPtr->fcliTftpExecuteLockEntry);
                errorCode = tfTftpRetrySend(tftpClientPtr);
                tm_call_lock_wait(&tftpClientPtr->fcliTftpExecuteLockEntry);
                if(errorCode != TM_ENOERROR )
                {
/* Call tfTftpCloseListen with lock on */
                    tfTftpCloseListen(tftpClientPtr, TM_TFTP_ERROR,
                                      TM_8BIT_YES);
                }
            }
        }

/*
 * No more work to do. Check whether the connection is closed (because
 * of timeout or error).
 */
        if (tftpClientPtr->fcliTftpListenDesc == TM_SOCKET_ERROR)
        {
/* TFTP client stopped */
            tftpClientPtr->fcliTftpExecuting = TM_8BIT_ZERO;
        }
        else
/* No more work to do, and TFTP client still up, pend if allowed */
        {
#ifdef TM_PEND_POST_NEEDED
            if (tftpClientPtr->fcliTftpBlockingState != TM_BLOCKING_OFF)
            {
                (void)tm_pend(&tftpClientPtr->fcliTftpPendSem, 
                              &tftpClientPtr->fcliTftpExecuteLockEntry);
            }
            else
#endif /* TM_PEND_POST_NEEDED */
            {
/* otherwise, exit the loop */
                tftpClientPtr->fcliTftpExecuting = TM_8BIT_ZERO;
            }
        }
    } while ( tftpClientPtr->fcliTftpExecuting != TM_8BIT_ZERO );
}

/*
 * tfTftpSetTimeout function description:
 *  Set TFTP timeout and retry parameters.
 *
 * Parameters       Meaning
 *  timeout          time (in seconds) before resending a packet
 *  retry            maximum amount of time (in seconds) to wait for a packet
 *
 * Return value
 *  error code
 */
int tfTftpSetTimeout(int timeout, int retry)
{
    ttUser32Bit                 errorCode;
    ttTftpClientPtr     tftpClientPtr;

    tftpClientPtr = (ttTftpClientPtr)tm_context(tvTftpClientPtr);
    if (tftpClientPtr != (ttTftpClientPtr)0)
    {
        tm_call_lock_wait(&tftpClientPtr->fcliTftpExecuteLockEntry);
        if(tftpClientPtr->fcliTftpTransfer == 0)
        {
            tftpClientPtr->fcliTftpIdleTimeout = 
                    (tt32Bit)((tt32Bit)timeout*TM_UL(1000));
            tftpClientPtr->fcliTftpRetry = (tt32Bit)retry;
            errorCode = TM_TFTP_SUCCESS;
        }
        else
        {
            errorCode = TM_TFTP_EXECUT;
        }
        tm_call_unlock(&tftpClientPtr->fcliTftpExecuteLockEntry);
    }
    else
    {
        errorCode = TM_TFTP_EINVAL;
    }
    return (int)errorCode;
}

/*
 * tfTftpRecvFile function description:
 *  Called from TFTP main execution loop.  Recieves TFTP data and does 
 *  appropriate translation (if ASCII) and processing (if error,xRQ,etc).
 *
 *
 * Parameters       Meaning
 * tftpClientPtr    ptr to client data structure
 *
 * Return values    Meaning
 * None             None
 */
static void tfTftpRecvFile(ttTftpClientPtr tftpClientPtr, int rcvdBytes)
{
    int                 retry;
    int                 cnt;
    tt16Bit             netErrorCode;
    int                 errorCode;
    char                c;
    ttCharPtr           p;
    ttTftpHdrPtr        rp;
    ttTftpConnEntryPtr  tftpConnEntryPtr;
    ttUser32Bit         retCode;
    ttCharPtr           bufferPtr;
    ttUser32Bit         bufferSize;
    ttUser32Bit         conRcvdBytes;
    tt8Bit              needFreeConnection;
    char                prevChar;

    needFreeConnection = TM_8BIT_ZERO;

    tftpConnEntryPtr = tftpClientPtr->fcliTftpConnHeadPtr;
    conRcvdBytes = tftpConnEntryPtr->fconRcvdBytes;

    retCode = TM_TFTP_ERROR; /* only use in 2 failure cases below */
    bufferPtr = tftpConnEntryPtr->fconBufferPtr;
    bufferSize = tftpConnEntryPtr->fconBufferSize;
    prevChar =  tftpConnEntryPtr->fconPrevChar;

    rcvdBytes = rcvdBytes - 4;
/* Received TFTP packet */
    rp = (ttTftpHdrPtr)(ttVoidPtr)tftpConnEntryPtr->fconTftpBuf;
    rp->th_opcode = ntohs(rp->th_opcode);
    rp->th_block = ntohs(rp->th_block);
    if(rp->th_opcode == TM_TFTP_ERR)
    {
/* 
 * Received packet is an error packet, map to user error code 
 */
        netErrorCode = rp->th_u.tu_code;
        retCode = tfTftpUserError(netErrorCode);

        needFreeConnection = TM_8BIT_YES;
        goto tfTftpRecvReturn;
    }
    if( (rp->th_opcode == TM_TFTP_DATA) && 
            (rp->th_block == 
            ((tftpConnEntryPtr->fconBlockNum + 1)&0xFFFF)) )
    {
        tftpConnEntryPtr->fconBlockNum = 
            (unsigned short) 
            ((tftpConnEntryPtr->fconBlockNum+1) & 0xFFFF);
        if((ttUser32Bit) rcvdBytes > bufferSize)
        {
            retCode = TM_TFTP_EBUF;
            needFreeConnection = TM_8BIT_YES;
            goto tfTftpRecvReturn;
        }
        if(tftpConnEntryPtr->fconMode == TM_TYPE_BINARY)
        {
            tm_bcopy(rp->th_data, bufferPtr, rcvdBytes);
            bufferSize = bufferSize - (ttUser32Bit)rcvdBytes;
            bufferPtr = bufferPtr + rcvdBytes;
            conRcvdBytes  = conRcvdBytes + (ttUser32Bit)rcvdBytes;
        }
        else
        {
            p = (ttCharPtr)rp->th_data;
            cnt = rcvdBytes;
            while(cnt)
            {
                cnt--;
                c = *p++;
                if(prevChar == '\r')
                {
                    if(c == '\n')
                    {
                        *bufferPtr++ = c;
                        bufferSize --;
                        conRcvdBytes ++;
                        prevChar = c;
                        continue;
                    }
                    else
                    {
                        if(c == '\0')
                        {
                            *bufferPtr++ = prevChar;
                            bufferSize--;
                            conRcvdBytes++;
                            prevChar = c;
                            continue;
                        }
                        else
                        {
                            *bufferPtr++ = prevChar;
                            bufferSize --;
                            conRcvdBytes ++;
                        }
                    }
                }
                if(c != '\r'){
                    *bufferPtr++ = c;
                    bufferSize --;
                    conRcvdBytes ++;
                }
                prevChar = c;
            }
        }
        if(rcvdBytes == TM_TFTP_SEGSIZE)
        {
            retry = 1;
        }
        else
        {
            retry = 0;
        }
        errorCode = tfTftpAckSend(retry, tftpClientPtr);
        if( (rcvdBytes < TM_TFTP_SEGSIZE) || (errorCode != TM_ENOERROR) )
        {
            retCode = conRcvdBytes;
            needFreeConnection = TM_8BIT_YES;
        }
    }
    else
    {
        if( (rp->th_opcode == TM_TFTP_DATA) && 
            (tftpConnEntryPtr->fconBlockNum == rp->th_block) )
        {
            errorCode = tfTftpRetrySend(tftpClientPtr);
            if(errorCode != TM_ENOERROR )
            {
/* Use retCode set at top to TM_TFTP_ERROR */
                needFreeConnection = TM_8BIT_YES;
            }
        }
        else
        {
            if (rp->th_opcode != TM_TFTP_ERR)
            {
                tfTftpErrorSend(tftpClientPtr, TM_TFTP_EBADOP);
            }
/* Use retCode set at top to TM_TFTP_ERROR */
            needFreeConnection = TM_8BIT_YES;
        }
    }
tfTftpRecvReturn:
    if (needFreeConnection == TM_8BIT_ZERO)
    {
        tftpConnEntryPtr->fconBufferSize = bufferSize;
        tftpConnEntryPtr->fconBufferPtr = bufferPtr;
        tftpConnEntryPtr->fconRcvdBytes = conRcvdBytes;
        tftpConnEntryPtr->fconPrevChar = prevChar;
    }
    else
    {
/* Call tfTftpCloseListen with lock off */
        tfTftpCloseListen(tftpClientPtr, retCode, TM_8BIT_ZERO);
    }
    return;
}

/*
 * tfTftpXmitFile function description:
 *  Called from TFTP main execution loop.  Transmits data to TFTP server.
 *
 * Parameters       Meaning
 * tftpClientPtr    ptr to client data structure
 *
 * Return values    Meaning
 * None             None
 */
static void tfTftpXmitFile(ttTftpClientPtr tftpClientPtr)
{
    ttTftpHdrPtr        tp;
    ttTftpConnEntryPtr  tftpConnEntryPtr;
    ttUser32Bit         retCode;
    int                 sntBytes;
    int                 errorCode;
    tt16Bit             netErrorCode;
    tt8Bit              needFreeConnection;

    needFreeConnection = TM_8BIT_ZERO;
    retCode = TM_TFTP_SUCCESS; 

    tftpConnEntryPtr = tftpClientPtr->fcliTftpConnHeadPtr;
/* Received TFTP packet */
    tp = (ttTftpHdrPtr)(ttVoidPtr)tftpConnEntryPtr->fconTftpBuf;
    tp->th_opcode = ntohs(tp->th_opcode);
    tp->th_block = ntohs(tp->th_block);

    if( (tp->th_opcode == TM_TFTP_ACK) && 
        (tftpConnEntryPtr->fconBlockNum == tp->th_block) )
    {
        if(tftpConnEntryPtr->fconEndFlag != 1)
        {
            tftpConnEntryPtr->fconBlockNum = 
                (unsigned short)
                ((tftpConnEntryPtr->fconBlockNum + 1) & 0xFFFF);

            sntBytes = 0; /* compiler warning */
            errorCode = tfTftpDataSend(&sntBytes, tftpClientPtr);
            if( (sntBytes < TM_TFTP_SEGSIZE) || (errorCode != TM_ENOERROR) )
            {
                tftpConnEntryPtr->fconEndFlag = 1;
            }
        }
        else
        {
/* Success */
            needFreeConnection = TM_8BIT_YES;
        }
    }
    else
    {
        if( (tp->th_opcode == TM_TFTP_ACK) &&
                 ((tftpConnEntryPtr->fconBlockNum - 1) == tp->th_block) )
        {
            (void)tfTftpRetrySend(tftpClientPtr);
        }
        else
        {
            if (tp->th_opcode != TM_TFTP_ERR) 
            {
                tfTftpErrorSend(tftpClientPtr, TM_TFTP_EBADOP);
                netErrorCode = TM_TFTP_EBADOP;
            }
            else
            {
/* 
 * Received packet is an error packet
 */
                netErrorCode = tp->th_u.tu_code;
            }
/* 
 * map TFTP error code to user error code 
 */
            retCode = tfTftpUserError(netErrorCode);
            needFreeConnection = TM_8BIT_YES;
        }
    }
    if (needFreeConnection != TM_8BIT_ZERO)
    {
/* Call tfTftpCloseListen with lock off */
        tfTftpCloseListen(tftpClientPtr, retCode, TM_8BIT_ZERO);
    }
}

/*
 * tfTftpAckSend function description:
 *  Send a ACK to TFTP server - will retransmit if necessary.
 *
 * Parameters       Meaning
 *  retry            boolean value indicating whether this is a retransmission
 *  tftpClientPtr    ptr to client data structure
 *
 * Return values    Meaning
 * None             None
 */
static int tfTftpAckSend(int retry, ttTftpClientPtr tftpClientPtr)
{
    ttTftpHdrPtr        tp;
    int                 retCode;
    int                 errorCode;
    ttSockAddrPtrUnion  tempSockAddr;
    ttGenericUnion      timerParm1;
    ttTftpConnEntryPtr  tftpConnEntryPtr;

    tftpConnEntryPtr = tftpClientPtr->fcliTftpConnHeadPtr;
    tempSockAddr.sockNgPtr = &tftpConnEntryPtr->fconRemoteAddress;

    errorCode = TM_ENOERROR;
    tp = (ttTftpHdrPtr)tftpConnEntryPtr->fconTftpBuf;
    tp->th_opcode = htons(TM_TFTP_ACK);
    tp->th_block = htons(tftpConnEntryPtr->fconBlockNum);
    retCode = sendto( tftpClientPtr->fcliTftpListenDesc,
                      (ttCharPtr)tp,
                      4,
                      MSG_DONTWAIT, 
                      tempSockAddr.sockPtr,
                      tempSockAddr.sockPtr->sa_len );
    if(retCode != 4)
    {
        if (retCode == TM_SOCKET_ERROR)
        {
            errorCode = tfGetSocketError(tftpClientPtr->fcliTftpListenDesc);
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
            tftpConnEntryPtr->fconSendSize = 4;
            tftpConnEntryPtr->fconTimeCount = 0;
            timerParm1.genVoidParmPtr = (ttVoidPtr)tftpConnEntryPtr;
            tftpConnEntryPtr->fconTimerPtr =
                tfTimerAdd( tfTftpSignalTimeout,
                            timerParm1,
                            timerParm1, /* unused */
                            tftpClientPtr->fcliTftpIdleTimeout,
                            0);
        }
    } 
    return errorCode;
}

/*
 * tfTftpDataSend function description:
 *  Sends raw data to TFTP server, doing translation if necessary (ASCII).
 *
 * Parameters       Meaning
 *  size             returns the size of data sent (<=512)
 *  tftpClientPtr    ptr to client data structure
 *
 * Return value
 *  error code 
 *
 */
static int tfTftpDataSend(int TM_FAR * sizePtr, ttTftpClientPtr tftpClientPtr)
{
    ttTftpHdrPtr        tp;
    ttCharPtr           p;
    ttTftpConnEntryPtr  tftpConnEntryPtr;    
    ttCharPtr           bufferPtr;
    ttUser32Bit         bufferSize;
    ttSockAddrPtrUnion  tempSockAddr;
    ttGenericUnion      timerParm1;
    int                 i;
    int                 retCode;
    int                 errorCode;
    char                c;
    char                prevChar;
    tt8Bit              newLine;

    tftpConnEntryPtr = tftpClientPtr->fcliTftpConnHeadPtr; 
    bufferPtr = tftpConnEntryPtr->fconBufferPtr;
    bufferSize = tftpConnEntryPtr->fconBufferSize;
    prevChar = tftpConnEntryPtr->fconPrevChar; 
    newLine = tftpConnEntryPtr->fconNewLine; 
    tempSockAddr.sockNgPtr = &tftpConnEntryPtr->fconRemoteAddress;

    errorCode = TM_ENOERROR;
    tp = (ttTftpHdrPtr)tftpConnEntryPtr->fconTftpBuf;
    tp->th_opcode = htons(TM_TFTP_DATA);
    tp->th_block = htons(tftpConnEntryPtr->fconBlockNum);
    if(tftpConnEntryPtr->fconMode == TM_TYPE_BINARY)
    {
        if(bufferSize >= TM_TFTP_SEGSIZE)
        {
            *sizePtr = TM_TFTP_SEGSIZE;
        }
        else
        {
            *sizePtr = (int)bufferSize;
        }
        tm_bcopy(bufferPtr, tp->th_data, *sizePtr);
        bufferPtr = bufferPtr + *sizePtr;
        bufferSize = bufferSize - (ttUser32Bit)*sizePtr;
    }
    else
    {
        p = (ttCharPtr)tp->th_data;
        for(i=0; i<TM_TFTP_SEGSIZE; i++)
        {
            if(newLine)
            {
                if(prevChar == '\n')
                {
                    c = '\n';
                }
                else
                {
                    c = '\0';
                }
                newLine = 0;
            }
            else
            {
                if(bufferSize == 0)
                {
                    *sizePtr = 0;
                    break;
                }
                else
                {
                    if(*sizePtr == 0)
                        break;
                    c = *bufferPtr++; 
                    bufferSize--;
                }
                if( (c == '\n') || (c == '\r') )
                {
                    prevChar = c;
                    c = '\r';
                    newLine = 1;
                } 
            }
            *p++ = c;
        }
        *sizePtr = (int)(p - (ttCharPtr)tp->th_data);
    }

    retCode = sendto( tftpClientPtr->fcliTftpListenDesc,
                      (ttCharPtr)tp,
                      (*sizePtr+4),
                      MSG_DONTWAIT, 
                      tempSockAddr.sockPtr,
                      tempSockAddr.sockPtr->sa_len );

    if(retCode != (*sizePtr+4))
    {
        if (retCode == TM_SOCKET_ERROR)
        {
            errorCode = tfGetSocketError(tftpClientPtr->fcliTftpListenDesc);
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
        tftpConnEntryPtr->fconSendSize = *sizePtr + 4;
        tftpConnEntryPtr->fconTimeCount = 0;
        timerParm1.genVoidParmPtr = (ttVoidPtr)tftpConnEntryPtr;
        tftpConnEntryPtr->fconTimerPtr =
            tfTimerAdd( tfTftpSignalTimeout,
                        timerParm1,
                        timerParm1, /* unused */
                        tftpClientPtr->fcliTftpIdleTimeout,
                        0);
    }
    tftpConnEntryPtr->fconBufferPtr = bufferPtr;
    tftpConnEntryPtr->fconBufferSize = bufferSize;
    tftpConnEntryPtr->fconPrevChar = prevChar;
    tftpConnEntryPtr->fconNewLine = newLine;
    return errorCode;
}

/*
 * tfTftpErrorSend function description:
 *  Sends an error packet to the TFTP server.
 *
 * Parameters       Meaning
 *  conEntryPtr     TFTP connection
 *  netErrorCode    error code to send to server
 *
 * Return values    Meaning
 * None             None
 */
static void tfTftpErrorSend(ttTftpClientPtr tftpClientPtr,
                            tt16Bit         netErrorCode)
{
    ttTftpHdrPtr                 tp;
    const struct tsErrmsg TM_FAR * pe;
    ttSockAddrPtrUnion           tempSockAddr;
    ttTftpConnEntryPtr           tftpConnEntryPtr;
    int                          length;

    tftpConnEntryPtr = tftpClientPtr->fcliTftpConnHeadPtr;

    tempSockAddr.sockNgPtr = &tftpConnEntryPtr->fconRemoteAddress;

    tp = (ttTftpHdrPtr)tftpConnEntryPtr->fconTftpBuf;
    tp->th_opcode = htons(TM_TFTP_ERR);
    tp->th_code = htons(netErrorCode);
    for(pe = tlTftpErrmsgs; pe->e_code >= 0; pe++)
    {
        if(pe->e_code == (int)netErrorCode)
            break;
    }
    if(pe->e_code < 0)
    {
        pe = tlTftpErrmsgs;
        tp->th_code = TM_TFTP_EUNDEF;
    }
    tm_strcpy(tp->th_msg, pe->e_msg);
    length = (int)tm_strlen(pe->e_msg);
    tp->th_msg[length] = '\0';
    length += 5;
    (void)sendto( tftpClientPtr->fcliTftpListenDesc,
                  (ttCharPtr)tp,
                  length,
                  MSG_DONTWAIT, 
                  tempSockAddr.sockPtr,
                  tempSockAddr.sockPtr->sa_len );
    
}

/*
 * tfTftpRetrySend function description:
 *  Retries transmitting a previously sent packet.
 *
 * Parameters       Meaning
 *  tftpClientPtr    ptr to client data structure
 *
 * Return values    Meaning
 * None             None
 */
static int tfTftpRetrySend(ttTftpClientPtr tftpClientPtr)
{
    int                 retCode;
    int                 errorCode;
    ttSockAddrPtrUnion  tempSockAddr;
    ttGenericUnion      timerParm1;
    ttTftpConnEntryPtr  tftpConnEntryPtr;

    tftpConnEntryPtr = tftpClientPtr->fcliTftpConnHeadPtr;
    tempSockAddr.sockNgPtr = 
            &tftpConnEntryPtr->fconRemoteAddress;

    errorCode = TM_ENOERROR;
    retCode = sendto( tftpClientPtr->fcliTftpListenDesc,
                      (ttCharPtr)tftpConnEntryPtr->fconTftpBuf, 
                      tftpConnEntryPtr->fconSendSize,
                      MSG_DONTWAIT, 
                      tempSockAddr.sockPtr,
                      tempSockAddr.sockPtr->sa_len );
    if(retCode != tftpConnEntryPtr->fconSendSize)
    {
        if (retCode == TM_SOCKET_ERROR)
        {
            errorCode = tfGetSocketError(tftpClientPtr->fcliTftpListenDesc);
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
        timerParm1.genVoidParmPtr = 
                (ttVoidPtr)tftpConnEntryPtr;
        tftpConnEntryPtr->fconTimerPtr =
            tfTimerAdd( tfTftpSignalTimeout,
                        timerParm1,
                        timerParm1, /* unused */
                        tftpClientPtr->fcliTftpIdleTimeout,
                        0);
    }    
    return  errorCode;
}

/*
 * tfTftpSignalTimeout function description:
 *
 * Parameters       Meaning
 *  ParamPtr
 *
 * Return values    Meaning
 * None             None
 */
void tfTftpSignalTimeout (ttVoidPtr       timerBlockPtr,
                          ttGenericUnion  userParm1,
                          ttGenericUnion  userParm2)
{
    ttTftpConnEntryPtr  connEntryPtr;    
    ttTftpClientPtr     tftpClientPtr;

    tftpClientPtr = (ttTftpClientPtr)tm_context(tvTftpClientPtr);

    if (tftpClientPtr != (ttTftpClientPtr)0)
    {          
/* Avoid compiler warnings about unused parameters */
        TM_UNREF_IN_ARG(userParm2);

        connEntryPtr = (ttTftpConnEntryPtr) userParm1.genVoidParmPtr;

        tm_lock_wait(&tftpClientPtr->fcliTftpExecuteLockEntry);
        if (    (connEntryPtr != (ttTftpConnEntryPtr)0)
             && (connEntryPtr->fconTimerPtr == timerBlockPtr) )
        {
            connEntryPtr->fconTimerPtr = (ttTimerPtr)0; /* non auto timer */
            connEntryPtr->fconCBFlag = TM_TFTP_CB_TIMEOUT;
#ifdef TM_PEND_POST_NEEDED
            if (tftpClientPtr->fcliTftpBlockingState != TM_BLOCKING_OFF)
            {
/* Post */
                tm_post( &tftpClientPtr->fcliTftpPendSem,
                         &tftpClientPtr->fcliTftpExecuteLockEntry,
                         TM_ENOERROR);
            }
#endif /* TM_PEND_POST_NEEDED */
        }
        tm_unlock(&tftpClientPtr->fcliTftpExecuteLockEntry);
    }
}


/*
 * tfTftpCloseListen function description:
 *
 * Always called from the execute loop thread.
 * Close a TFTP connection.
 * 1. Close the listening socket
 * 2. Reset the TFTP connection fields 
 * 3. Free the allocated connection structure.
 * Parameters       Meaning
 *  tftpClientPtr   ptr to client data structure
 *
 * Return values    Meaning
 * None             None
 */
static void tfTftpCloseListen(ttTftpClientPtr tftpClientPtr, ttUser32Bit retCode, tt8Bit lockOn)
{    
    ttTimerPtr                   timerPtr;
    ttTftpConnEntryPtr           tftpConnEntryPtr;

    if (lockOn != TM_8BIT_ZERO)
    {
        tm_call_unlock(&tftpClientPtr->fcliTftpExecuteLockEntry);  
    }
    (void)tfRegisterSocketCB(tftpClientPtr->fcliTftpListenDesc,
                              (ttUserSocketCBFuncPtr)0,
                              0);
    (void)tfClose(tftpClientPtr->fcliTftpListenDesc); 
    tm_call_lock_wait(&tftpClientPtr->fcliTftpExecuteLockEntry);     
    tftpClientPtr->fcliTftpRetCode = retCode;
    tftpClientPtr->fcliTftpListenDesc = TM_SOCKET_ERROR;     
    tftpClientPtr->fcliTftpTransfer = 0;
    tftpClientPtr->fcliTftpPendingRecv = 0;
    tftpConnEntryPtr = tftpClientPtr->fcliTftpConnHeadPtr;
    timerPtr = tftpConnEntryPtr->fconTimerPtr;
    if (timerPtr != (ttTimerPtr)0)
    {
        tm_timer_remove(timerPtr);
        tftpConnEntryPtr->fconTimerPtr = TM_TMR_NULL_PTR;
    }
    if (tftpClientPtr->fcliTftpConnHeadPtr != (ttTftpConnEntryPtr)0)
    {
        tm_free_raw_buffer(tftpClientPtr->fcliTftpConnHeadPtr);
        tftpClientPtr->fcliTftpConnHeadPtr = (ttTftpConnEntryPtr)0;
    }
    if (lockOn == TM_8BIT_ZERO)
    {
        tm_call_unlock(&tftpClientPtr->fcliTftpExecuteLockEntry);  
    }
}

#ifdef TM_USE_STOP_TRECK
void tfTftpDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                )
{
    if (tm_context_var(tvTftpClientPtr) != (ttVoidPtr)0)
    {
        tm_kernel_free(tm_context(tvTftpClientPtr)); 
        tm_context_var(tvTftpClientPtr) = (ttVoidPtr)0;
    }
}
#endif /* TM_USE_STOP_TRECK */

#else /* !TM_USE_TFTP */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */


/* To allow link for builds when TM_USE_TFTP is not defined */
int tvTftpDummy = 0;

#endif /* !TM_USE_TFTP */

