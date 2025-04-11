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
 * Description: FTP server Protocol
 * Filename: trftpd.c
 * Author: Odile
 * Date Created: 10/27/98
 * $Source: source/trftpd.c $
 *
 * Modification History
 * $Revision: 6.0.2.9 $
 * $Date: 2013/03/02 14:11:22JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_FTPD
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>
#include "system.h"
#include "ftpctrl.h"
#ifdef TM_USE_FTPD_SSL
#include <trssl.h>
#endif /* TM_USE_FTPD_SSL */

/*
 * This module contains the FTP server implementation, and follows the FTP
 * protocol from RFC 959, and RFC 1123.
 *
 * Features of the FTP server implementation:
 *
 * . Task requirement:
 * The FTP server can be either run in the context of a task (for
 * multi-tasking systems), or from an executive loop (for single-tasking
 * systems). In both cases it allows concurrent incoming connections.
 * For the multi-tasking systems, only one task (the FTP server task)
 * is required.
 *
 * . Managing concurrent imcoming connections. Ftp server run q:
 * To manage concurrent incoming connections, The FTP server uses a run q
 * of active connection blocks and sends and recvs data (for control and
 * data connections) to and from the network in non-blocking mode.
 * It uses the call back feature of the Treck TCP/IP stack to enter active
 * blocks in the run q.
 *
 * . FTP control connections:
 * The FTP server does its own Telnet protocol transformation on the
 * control connections (given that the RFC requires only a subset of the
 * telnet protocol).
 *
 * . FTP data connections:
 * The Ftp server uses the zero copy send and receive features of the Treck
 * TCP/IP stack on each data connection. The size of the zero copy send buffer
 * is determined as the effective maximum TCP segment size, and the macro
 * TM_BUF_RECV_SIZE gives the maximum size of the zero copy recv buffer.
 * Their values are optimized for an Ethernet or PPP link layer.
 *
 * Limitations:
 * In the 2.x release, the following FTP commands are not implemented:
 * SITE, ACCT, HELP, ALLO, RESTart, STAT, TYPE Ebcdic, STRUcture Page,
 * FORM Telnet, FORM Asa, MODE Compressed, and MODE Block.
 */

/*
 * Local macros
 */

/*
 * FTP commands:
 * USER, PASS, ACCT,
 * PORT, PASV,
 * TYPE, MODE, STRU,
 * RETR, STOR, APPE,
 * RNFR, RNTO, DELE,
 * CWD,  CDUP, RMD,  MKD,  PWD,
 * LIST, NLST,
 * SYST, STAT,
 * HELP, NOOP, QUIT.
 */

/* Ftp commands (used as index in tlFtpdCmdArray[]) */
#define TM_USER                 0
#define TM_PASS                 1
#define TM_ACCT                 2
#define TM_CWD                  3
#define TM_CDUP                 4
#define TM_QUIT                 5
#define TM_SMNT                 6
#define TM_REIN                 7
#define TM_PORT                 8
#define TM_PASV                 9
#define TM_TYPE                 10
#define TM_STRU                 11
#define TM_MODE                 12
#define TM_RETR                 13
#define TM_STOR                 14
#define TM_STOU                 15
#define TM_APPE                 16
#define TM_ALLO                 17
#define TM_REST                 18
#define TM_RFNR                 19
#define TM_RNTO                 20
#define TM_ABOR                 21
#define TM_DELE                 22
#define TM_RMD                  23
#define TM_MKD                  24
#define TM_PWD                  25
#define TM_LIST                 26
#define TM_NLST                 27
#define TM_SITE                 28
#define TM_SYST                 29
#define TM_STAT                 30
#define TM_HELP                 31
#define TM_XCWD                 32
#define TM_XCUP                 33
#define TM_XRMD                 34
#define TM_XMKD                 35
#define TM_XPWD                 36
#define TM_EPRT                 37
#define TM_EPSV                 38

#ifndef TM_USE_FTPD_SSL
#define TM_NOOP                 39
#else /* TM_USE_FTPD_SSL */
/* The follwing commands are described in RFC 2228 */
#define TM_AUTH                 39
#define TM_ADAT                 40
#define TM_PROT                 41
#define TM_PBSZ                 42
#define TM_CCC                  43
#define TM_ENC                  44
#define TM_FEAT                 45
#define TM_NOOP                 46
#endif /* TM_USE_FTPD_SSL */

/* FTP command argument type (string, no argument, optional argument) */
#define TM_STRING               0 /* string argument required */
#define TM_NOARG                1 /* no argument */
#define TM_STRINGOPT            2 /* optional string argument */

/*
 * Ftp command argument flags (login required, upper case required,
 * not supported)
 */
#define TM_NOLOGIN              0x0 /* no login required to execute command */
#define TM_LOGIN                0x1 /* login required to execute command */
#define TM_LOGINARG             0x2 /* login only required with argument */
#define TM_UPPER                0x4 /* Change all arguments to upper case */
#define TM_NOSUP                0x8 /* non supported commands */

#ifdef TM_USE_FTPD_SSL
/* The Control Channel must be SSL encrypted */
#define TM_AUTH_CTRL_REQUIRED   0x10 
/* The Data Channel must be SSL encrypted */
#define TM_PROT_DATA_REQUIRED   0x20 
#endif /* TM_USE_FTPD_SSL */

/* FTP command length */
#define TM_FTP_MAX_CMD_LENGTH   4

/* Transfer mode */
#define TM_MODE_STREAM      0  /* default */
#define TM_MODE_BLOCK       1  /* Not implemented */
#define TM_MODE_COMPRESSED  2  /* Not implemented */

/* Transfer type (first parameter of TYPE command)*/
/* #define TM_TYPE_ASCII     0 */ /* Ascii (default), defined in trsocket.h */
#define TM_TYPE_EBCDIC    1 /* EBCDIC, not implemented */
/* #define TM_TYPE_BINARY    2 */ /* Image, defined in trsocket.h */
#define TM_TYPE_LOCAL       4  /* Local */
#define TM_TYPE_FLAG        0x0F /* 0, 1, 2, or 4 */
#define TM_TYPE_BINARY_FLAG 0x10 /* Indicates that file system EOL is CR,LF */

/* Transfer form (Second (optional parameter) of TYPE command ) */
#define TM_FORM_NONPRINT    0 /* Non print (default) */
#define TM_FORM_TELNET      1 /* printer telnet cntrl chars, not implemented */
#define TM_FORM_ASA         2 /* printer ASA cntrl chars, not implemented */

/* Transfer file structure */
/* #define TM_STRU_STREAM    0 */ /* File (default), defined in trsocket.h */
/* #define TM_STRU_RECORD    1 */ /* Record, defined in trsocket.h */
#define TM_STRU_PAGE      2  /* Page (not implemented) */

/* record structure FTP translation (from RFC 959) */
#define TM_RECORD_ESCAPE   (tt8Bit)255
#define TM_RECORD_EOR      (tt8Bit)0x1
#define TM_RECORD_EOF      (tt8Bit)0x2
#define TM_RECORD_EOR_EOF  (tt8Bit)(TM_RECORD_EOR|TM_RECORD_EOF)

/* Ftp server listening port */
#define TM_FTPD_PORT            tm_const_htons(21)

/* RFC 1123 states that the minimum idle timeout should be 5 minutes */
#define TM_FTPD_MIN_TIMEOUT     300

/* Run queue and list of FTP connections */
#define TM_FTPD_RUN_Q           0
#define TM_FTPD_CONN_LIST       1


/* Queue not empty */
#define tm_ftpd_q_not_empty(i)                                       \
  (tm_context(tvFtpdConQHead).servQHeadCount[i] != 0)
/* Queue empty */
#define tm_ftpd_q_empty(i)                                           \
  (tm_context(tvFtpdConQHead).servQHeadCount[i] == 0)

/* BUG ID 37: added new macro tm_ftpd_single_in_q */
/* Is the given FTP connection entry, the only entry in a queue? */
#define tm_ftpd_single_in_q(ftpdConnEntryPtr, i)                             \
  (    (tm_context(tvFtpdConQHead).servQHeadCount[i] == 1)                   \
    && (ftpdConnEntryPtr == (ttFtpdConnEntryPtr)                             \
                   (tm_context(tvFtpdConQHead).servHeadNextQEntryPtr(i))))
/* End of BUG ID 37 */

/* Timeout call back */
#define TM_CB_TIMEOUT         0x4000

/*
 * command line size
 */
#define TM_CMD_LINE_SIZE        256
/*
 * Temporary buffer size (for user login, rename file pointer, store unique,
 * and file input buffer for Ascii type or file record structure).
 */
#define TM_TEMP_BUF_SIZE        256

/*
 * Buffer size for zero copy network recv buffer.
 */
#define TM_BUF_RECV_SIZE        4096

/*
 * Telnet NVT special characters
 */
#define TM_NVT_NULL   0 /* no operation */
#define TM_NVT_LF   '\n'/* next print line, same horizontal position (10) */
#define TM_NVT_CR   '\r'/* left margin of current line (13) */
#define TM_NVT_SPACE ' ' /* space */

/*
 * Telnet command codes.
 */
#define TM_TC_IAC    (tt8Bit)255 /* interpret as command escape */
#define TM_TC_DONT   (tt8Bit)254 /* request peer not to do option */
#define TM_TC_DO     (tt8Bit)253 /* request peer to do option */
#define TM_TC_WONT   (tt8Bit)252 /* indicates refusal to do option */
#define TM_TC_WILL   (tt8Bit)251 /* indicates request to do option */
#define TM_TC_IP     (tt8Bit)244 /* Interrupt process */
#define TM_TC_DM     (tt8Bit)242 /* data mark-- Synch signal (with TCP URG) */

/*
 * Telnet states
 */
 /* Processing regular characters */
#define TM_TEL_DATA_STATE       (tt8Bit)0x00
/* An IAC has been received */
#define TM_TEL_IAC_STATE        (tt8Bit)0x01
/* A WILL/WONT option has been received */
#define TM_TEL_WOPT_STATE       (tt8Bit)0x02
/* A DO/DONT option has been received */
#define TM_TEL_DOPT_STATE       (tt8Bit)0x04
/* TCP Urgent mode, or sequence IAC, IP (interrupt process) received */
#define TM_TEL_URGENT_STATE     (tt8Bit)0x08
 /* IAC in urgent mode */
#define TM_TEL_URGENT_IAC_STATE (tt8Bit)(TM_TEL_IAC_STATE|TM_TEL_URGENT_STATE)
/* Carriage return as been received */
#define TM_TEL_CR_STATE         (tt8Bit)0x10

/*
 * Data transfer states used when transferring ASCII, or record structure
 * data on the data connection:
 */
/* No special character has been received */
#define TM_DATA_STATE           (tt8Bit)0x00
/*
 * TM_DATA_CR_STATE state:
 * '\r' (for ASCII transfer) or 0xFF (for record structure) has been received.
 */
#define TM_DATA_CR_STATE        (tt8Bit)0x01
/*
 * TM_DATA_FLUSH state:
 * For ASCII transfer: either '\r\n' (end of line), or '\r\0'
 * (escape '\r') has been received.
 * For record structure: either 0xFF,0x01 (end of record), or 0xFF,0x01
 * (escape 0xFF) has been received.
 * In that state, we flush the data buffer to the file.
 */
#define TM_DATA_FLUSH            (tt8Bit)0x40

/*
 * Ftp command flags
 */
/* A full ftp command is buffered in the FTP input buffer */
#define TM_FTPF_COMMAND_BUFFERED   (tt16Bit)0x0001
/* An FTPD data transfer is taking place */
#define TM_FTPF_DATA_TRANSFER      (tt16Bit)0x0002
/* A user name command has been received */
#define TM_FTPF_USER_NAME_IN       (tt16Bit)0x0004
/* An FTP client user is logged in */
#define TM_FTPF_USER_LOGGED_IN     (tt16Bit)0x0008
/* An FTP rename from command has been received */
#define TM_FTPF_RNFR_RECEIVED      (tt16Bit)0x0010
/* An FTP PORT command has been issued */
#define TM_FTPF_DATA_PORT          (tt16Bit)0x0020
/* An FTP PASV command has been issued, and an FTP passive socket is open */
#define TM_FTPF_DATA_PASV          (tt16Bit)0x0040
/* A connection is established on the FTP data socket */
#define TM_FTPF_DATA_CON           (tt16Bit)0x0080
/* Add a data connection timer */
#define TM_FTPF_DATA_TIMER         (tt16Bit)0x0100
/* Opened a file */
#define TM_FTPF_DATA_FILE          (tt16Bit)0x0200
/* Opened a directory */
#define TM_FTPF_DATA_DIR           (tt16Bit)0x0400
/* Read from a file */
#define TM_FTPF_DATA_READ          (tt16Bit)0x0800
/* Read from a file is done */
#define TM_FTPF_DATA_READ_DONE     (tt16Bit)0x1000
/* Write to a file */
#define TM_FTPF_DATA_WRITE         (tt16Bit)0x2000

#ifdef TM_USE_FTPD_SSL
/*
 * SSL command flags
 */
/* SSL Session activated */
#define TM_FTPD_SSL_SESSION_ACTIVE      (tt16Bit)0x0001
/* Data connection SSL encrypted */
#define TM_FTPD_SSL_DATA_ENCRYPTED      (tt16Bit)0x0002
/* Control connection SSL encrypted */
#define TM_FTPD_SSL_CTRL_ENCRYPTED      (tt16Bit)0x0004
/* We are enabling SSL on the Data connection */
#define TM_FTPD_SSL_ENCRYPT_DATA        (tt16Bit)0x0008
/* We are enabling SSL, turn on all encryption */
#define TM_FTPD_SSL_CHANGE_CTRL         (tt16Bit)0x0010
/* SSL is still being negotiated */
#define TM_FTPD_SSL_NEGOTIATING         (tt16Bit)0x0020
/* The maximum protected buffer size for PBSZ. SSL/TLS requires 0 */
#define TM_FTPD_SSL_PBSZ                0

#endif /* TM_USE_FTPD_SSL */

/* Following flags are reset when data transfer ends */
#define TM_FTPF_DATA_TRANSFER_FLAGS \
 (tt16Bit)(   TM_FTPF_DATA_TRANSFER | TM_FTPF_DATA_FILE | TM_FTPF_DATA_DIR \
            | TM_FTPF_DATA_READ | TM_FTPF_DATA_READ_DONE \
            | TM_FTPF_DATA_WRITE )

/* Following flags reset when data connection is closed */
#define TM_FTPF_DATA_FLAGS \
 (tt16Bit)(   TM_FTPF_DATA_TRANSFER_FLAGS | TM_FTPF_DATA_PORT \
            | TM_FTPF_DATA_PASV | TM_FTPF_DATA_CON | TM_FTPF_DATA_TIMER )

/* FTP control socket index */
#define TM_FTP_CTRL_SOCKET 0
/* FTP data socket index */
#define TM_FTP_DATA_SOCKET 1
/* FTP invalid socket index */
#define TM_FTP_NO_SOCKET  -1

/* EPRT command protocol numbers (from RFC-2428) */
#define TM_FTP_EPRT_INET  1
#define TM_FTP_EPRT_INET6 2

/*
 * Local types
 */
/* Connection entry data type */
#ifdef TM_LINT
LINT_UNREF_MEMBER_BEGIN
#endif /* TM_LINT */

#ifdef TM_USE_FTPD_SSL
#define TM_MAX_SECURITIES_LENGTH 3
/* 
 * This list contains valid arguments for the AUTH command.
 */
static const ttConstCharPtr tlSecuritiesArray[] = {"SSL", "TLS"};
#endif /* TM_USE_FTPD_SSL */

typedef struct tsFtpdConnEntry
{
#define fconNextQEntryPtr(qIndex)     \
  fconQueueEntry.servQEntry[qIndex].qNextEntryPtr
/* #define fconPrevQEntryPtr(qIndex)     \
  fconQueueEntry.servQEntry[qIndex].qPrevEntryPtr */
/* Run q, and open connection list */
    ttServerQueue                 fconQueueEntry;
/*
 * One timer (either on the control connection if no data transfer is taking
 * place, or on the data connection (only for passive accept)).
 */
    ttTimerPtr                     fconTimerPtr;
/*
 * User data pointer as returned by the file system for login,
 * directory bookkeeping
 */
    ttVoidPtr                      fconUserDataPtr;
/* File pointer or directory pointer. */
    ttVoidPtr                      fconFileDirPtr;
/* zero copy Send User message (on data connection) */
    ttUserMessage                  fconDataSendMessage;
/* zero copy Send buffer pointer (on data connection) */
    ttCharPtr                      fconDataSendPtr;
#ifdef TM_USE_FTPD_SSL
/* Buffer to hold the FEAT command reply. */
    ttCharPtr                      fconFeatBuffer;
#endif /* TM_USE_FTPD_SSL */
/* our default data address (non passive) */
    struct sockaddr_storage        fconSrvrDataAddress;
/* Our passive data address */
    struct sockaddr_storage        fconSrvrPasvDataAddress;
/* non port remote data address, same as peer control address. */
    struct sockaddr_storage        fconClientCtrlAddress;
/* port remote data address, as given to us by a port command. */
    struct sockaddr_storage        fconPortClientDataAddress;
    tt32Bit                        fconTotalBytesTransferred;
/* Flags from user, indicating what type of connection (IPv4, IPv6) will be
   allowed */
    tt32Bit                        fconUserFlags;
#ifdef TM_USE_FTPD_SSL
/* 
 * Protection Buffer -NOT USED BY SSL/TLS- Kept for ease of future 
 * implimentation [RFC 4217] [RFC 2228]
 */
    tt32Bit                        fconSslProtBuffer;
#endif /* TM_USE_FTPD_SSL */
/* Socket descriptors of the control connection and data connection */
    int                            fconDesc[2];
/* Current index in command line on control socket */
    int                            fconCmdLineIndex;
/* Current read index in telnet input buffer on control socket */
    int                            fconTelBufReadIndex;
/* Current processing index in telnet input buffer on control socket */
    int                            fconTelBufProcessIndex;
/* Error code on the control connection while executing FTP commands */
    int                            fconErrorCode;
/* Length of store unique file name plus prefix (for reply to FTP client) */
    int                            fconStouLength;
/* Network Send data buffer data length (data connection) */
    int                            fconDataSendLength;
/* File input buffer data index */
    int                            fconFileInputProcessIndex;
/* File input buffer end data index */
    int                            fconFileInputEndIndex;
/* CB flags for control and data connection */
    tt16Bit                        fconCBFlags[2];
/* command and data flags */
    tt16Bit                        fconFlags;
#ifdef TM_USE_FTPD_SSL
/* SSL control and data connection flags */
    tt16Bit                        fconSslFlags;
#endif /* TM_USE_FTPD_SSL */
/* Current FTP command */
    tt16Bit                        fconCommand;
/* send buffer size */
    tt16Bit                        fconSendBufferSize;
/* network control input buffer after NVT transformation */
    tt8Bit                         fconCmdLine[TM_CMD_LINE_SIZE];
/* network control input buffer before NVT transformation */
    tt8Bit                         fconTelInputBuf[TM_CMD_LINE_SIZE];
/*
 * Temporary buffer (for user login, rename file pointer, store unique,
 * and file data input buffer (ascii type, or record structure)).
 * Also used for directory listings to hold the current working dir.
 */
    tt8Bit                         fconTempBuf[TM_TEMP_BUF_SIZE];
/*
 * Used to send all xxx reply strings on control connection:
 * 550, 250, 504, 503, 502, 202, 425, 200). Size is size of the biggest
 * of these replies (i.e. 202).
 * Also used for sending telnet commands on control connection.
 */
    char                           fconReplyBuf[68];
/*
 * Current telnet state on the control network input buffer
 * (necessary for NVT)
 */
    tt8Bit                         fconTelState;
/* Actual transfer type */
    tt8Bit                         fconActualTxType;
/* Actual transfer structure */
    tt8Bit                         fconActualTxStru;
    tt8Bit                         fconTxType; /* transfer type */
    tt8Bit                         fconTxStru; /* transfer structure */
/* First end of record, i.e CR for ascii, 0xFF for record structure */
    tt8Bit                         fconFirstEndOfRecord;
/* Second end of record, i.e LF for ascii, 0x1 for record structure */
    tt8Bit                         fconSecondEndOfRecord;
/* Ignore end of record, i.e '\0' for ascii, 0xFF for record structure */
    tt8Bit                         fconEscapeEndOfRecord;
/* state of transfer (for ascii transformation on data connection) */
    tt8Bit                         fconTxState;
/* Outstanding (non flushed) file input End of record, End of file */
    tt8Bit                         fconFileInputEorEof;
    tt8Bit                         fconTimerType; /* control, or data timer */
/* Make it a multiple of 4 */
    tt8Bit                         fconFiller;
} ttFtpdConnEntry;
#ifdef TM_LINT
LINT_UNREF_MEMBER_BEGIN
#endif /* TM_LINT */


typedef  ttFtpdConnEntry TM_FAR * ttFtpdConnEntryPtr;


/* FTP action type, called when an FTP command is received */
typedef ttCharPtr (*ttFdcFuncPtr)( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                                   ttCharPtr          ftpdCmdLinePtr);
union tuftpCommand
{
    char        ftpuName[4];
    tt32Bit     ftpu32Bit;
};

/* Ftp server command entry data type */
typedef struct tsFtpdCmdEntry
{
/* Name of the FTP command (i.e STOR, QUIT, etc..) */
    union tuftpCommand fdcu;
/* Ftp function corresponding to the command */
    ttFdcFuncPtr       fdcFuncPtr;
/* Which file system flag should be turned on for us to accept the command */
    tt16Bit            fdcFsFlags;
/* FTP command argument type (string, no argument, optional argument) */
    tt8Bit             fdcArgType;
/* Ftp command argument flags (login required, upper case required) */
    tt8Bit             fdcArgFlags;
} ttFtpdCmdEntry;

#define fdcName    fdcu.ftpuName
#define fdc32Bit   fdcu.ftpu32Bit


/* Pointer to an FTP server command entry data type */
typedef const ttFtpdCmdEntry TM_FAR * ttFtpdCmdEntryPtr;

/*
 * Call back functions.
 */

/* Signal the FTP server process with call back events */
TM_NEARCALL void tfFtpdSignal( int               socketDesc,
                   ttVoidPtr         ftpdConnEntryPtr,
                   int               CBFlags );

/* Signal the FTP server process with client FTP connection request */
void tfFtpdSignalAcceptCtrl(int listenDesc, int flags);

/* Idle connection timeout FTP function */
TM_NEARCALL void tfFtpdIdleTimeout (ttVoidPtr      timerBlockPtr,
                        ttGenericUnion userParm1,
                        ttGenericUnion userParm2);


/*
 * Local functions
 */

/* Execute the main FTP server loop */
static void tfFtpdExecute (void);

/* Close a socket, resetting CB flags */
static void tfFtpdCloseSocket (int socketDesc);

/* Close the listening socket */
static void tfFtpdCloseListen (void);

/* Open a file, then call tfFtpdOpenData() */
static ttCharPtr tfFtpdOpenFileData (ttFtpdConnEntryPtr  ftpdConnEntryPtr,
                                     ttCharPtr           fileNamePtr,
                                     int                 flag );

/* Open a Directory, change mode to ascii then call tfFtpdOpenData() */
static ttCharPtr tfFtpdOpenDirData (ttFtpdConnEntryPtr  ftpdConnEntryPtr,
                                    ttCharPtr           dirNamePtr,
                                    int                 flag );

/* Open a data connection, return reply */
static ttCharPtr tfFtpdOpenData(ttFtpdConnEntryPtr ftpdConnEntryPtr);

/* Accept on a passive data connection */
static int tfFtpdAcceptData(ttFtpdConnEntryPtr ftpdConnEntryPtr);

/* Set common socket options, used by tfFtpdOpenData, and tfFtpdAcceptData */
static int tfFtpdSetDataOptions(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                                int                conFlags);

/* Transfer data. Called when we want to start transfering data. */
static int tfFtpdTransferData(ttFtpdConnEntryPtr ftpdConnEntryPtr);

/* Recv data on the data connection, write to a file */
static int tfFtpdRecvData(ttFtpdConnEntryPtr ftpdConnEntryPtr);

/* Read from a file, send data on the data connection */
static int tfFtpdSendData(ttFtpdConnEntryPtr ftpdConnEntryPtr);

/* Close a data connection. */
static int tfFtpdCloseData(ttFtpdConnEntryPtr ftpdConnEntryPtr);

/* Close an open directory or file at the end of a data transfer */
static int tfFtpdCloseDirFile(ttFtpdConnEntryPtr ftpdConnEntryPtr);

/*
 * End a data connection, closing it and send appropriate reply code string
 * to the client.
 */
static int tfFtpdEndData( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                          ttCharPtr          replyPtr );

/* Accept a new connection request */
static void tfFtpdAcceptCtrl(void);

/* Receive data on the control connection */
static int tfFtpdRecvCtrl(ttFtpdConnEntryPtr ftpdConnEntryPtr);

/* Send reply on control connection */
static int tfFtpdSendCtrl (int       ctrlSocketDesc,
                           ttCharPtr stringPtr,
                           int       currentErrorCode);

/* Close an FTP connection, freeing the FTP connection block. */
static void tfFtpdCloseCtrl( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                             int                errorCode);

/* Initialize with Default connection attributes */
static void tfFtpdDataDefault(ttFtpdConnEntryPtr ftpdConnEntryPtr);

/*
 * Scan FTP command line, map to action function and execute client FTP
 * commands
 */
static int tfFtpdScanCmd(ttFtpdConnEntryPtr ftpdConnEntryPtr);
/*
 * Queue an FTPD connection entry to the run Q, if there is any unscanned
 * FTP command buffered in.
 */
static void tfFtpdQueueUnScannedCtrlInput(
                                ttFtpdConnEntryPtr ftpdConnEntryPtr );

/* Remove an FTP connection timeout timer */
static void tfFtpdTimerRemove(ttVoidPtr timerArgPtr);

/* Add an FTP connection timeout timer */
static void tfFtpdTimerAdd(ttFtpdConnEntryPtr ftpdConnEntryPtr);

/*
 * Functions to execute client FTP commands
 */
/* USER command */
static ttCharPtr tfFtpdUser(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* PASS command */
static ttCharPtr tfFtpdPass(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* ACCT command */
static ttCharPtr tfFtpdAcct(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* CWD,XCWD command */
static ttCharPtr tfFtpdCwd(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* CDUP command */
static ttCharPtr tfFtpdCdup(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* QUIT command */
static ttCharPtr tfFtpdQuit(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* SMNT command */
static ttCharPtr tfFtpdSmnt(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* REIN command */
static ttCharPtr tfFtpdRein(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* PORT command */
static ttCharPtr tfFtpdPort(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* PASV command */
static ttCharPtr tfFtpdPasv(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* TYPE command */
static ttCharPtr tfFtpdType(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* STRU command */
static ttCharPtr tfFtpdStru(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* MODE command */
static ttCharPtr tfFtpdMode(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* STOR command */
static ttCharPtr tfFtpdStor(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* STOU command */
static ttCharPtr tfFtpdStou(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* APPE command */
static ttCharPtr tfFtpdAppe(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* RNFR command */
static ttCharPtr tfFtpdRnfr(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* RNTO command */
static ttCharPtr tfFtpdRnto(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* ABOR command */
static ttCharPtr tfFtpdAbor(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* DELE command */
static ttCharPtr tfFtpdDele(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* RMD,XRMD command */
static ttCharPtr tfFtpdRmd(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr         ftpdCmdLinePtr);
/* MKD,XMKD command */
static ttCharPtr tfFtpdMkd(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr         ftpdCmdLinePtr);
/* PWD command */
static ttCharPtr tfFtpdPwd(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr         ftpdCmdLinePtr);
/* LIST command */
static ttCharPtr tfFtpdList(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* NLST command */
static ttCharPtr tfFtpdNlst(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* RETR command */
static ttCharPtr tfFtpdRetr(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* SITE command */
static ttCharPtr tfFtpdSite(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* SYST command */
static ttCharPtr tfFtpdSyst(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* EPRT command */
static ttCharPtr tfFtpdEprt(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* EPSV command */
static ttCharPtr tfFtpdEpsv(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* NOOP command */
static ttCharPtr tfFtpdNoop(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);

#ifdef TM_USE_FTPD_SSL
/* AUTH command */
static ttCharPtr tfFtpdAuth(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* ADAT command */
static ttCharPtr tfFtpdAdat(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* PROT command */
static ttCharPtr tfFtpdProt(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* PBSZ command */
static ttCharPtr tfFtpdPbsz(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* CCC command */
static ttCharPtr tfFtpdCcc (ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* ENC command */
static ttCharPtr tfFtpdEnc (ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* FEAT command */
static ttCharPtr tfFtpdFeat(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
#endif /* TM_USE_FTPD_SSL */

#ifndef TM_NOSUP
/* Non supported command */

/* ALLO command */
static ttCharPtr tfFtpdAllo(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* REST command */
static ttCharPtr tfFtpdRest(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* HELP command */
static ttCharPtr tfFtpdHelp(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
/* STAT command */
static ttCharPtr tfFtpdStat(ttFtpdConnEntryPtr ftpdConnEntryPtr,
                            ttCharPtr          ftpdCmdLinePtr);
#endif /* TM_NOSUP */

/* convert string from lower case to upper case up to maxLength */
static int tfStringToUpper (char TM_FAR * stringCharPtr, int maxLength);

#ifdef TM_USE_FTPD_SSL
/* Set the socket options for the specified socket */
static int tfFtpdSslSetSockOpt(int socketDescriptor);
#endif /* TM_USE_FTPD_SSL */

/*
 * Local variables
 */

/*
 * FTPD command array, Maps a user FTP command to an action function,
 * and required FS flags, argument, login.
 */
#ifdef TM_NO_NESTED_BRACES
static const ttFtpdCmdEntry  TM_CONST_QLF tlFtpdCmdArray[TM_NOOP + 1] =
{
/* TM_USER: USER <SP> <username> <CRLF> */
 { { 'U','S','E','R' },
         tfFtpdUser, 0,                TM_STRINGOPT,  TM_NOLOGIN
#ifdef TM_USE_FTPD_SSL
                                                    | TM_AUTH_CTRL_REQUIRED
#endif /* TM_USE_FTPD_SSL */
                                                                          },
/* TM_PASS: PASS <SP> <password> <CRLF> */
 { { 'P','A','S','S' },
         tfFtpdPass, 0,                TM_STRINGOPT,  TM_NOLOGIN
#ifdef TM_USE_FTPD_SSL
                                                    | TM_AUTH_CTRL_REQUIRED 
#endif /* TM_USE_FTPD_SSL */
                                                                          },
/* TM_ACCT: ACCT <SP> <account-information> <CRLF> */
 { { 'A','C','C','T' },
         tfFtpdAcct, 0,                TM_STRING,     TM_NOLOGIN          },
/* TM_CWD: CWD  <SP> <pathname> <CRLF> */
 { { 'C','W','D',' ' },
         tfFtpdCwd,  TM_FS_CWD_FLAG,   TM_STRING,     TM_LOGIN            },
/* TM_CDUP: CDUP <CRLF> (Change to parent directory:) */
 { { 'C','D','U','P' },
         tfFtpdCdup, TM_FS_CWD_FLAG,   TM_NOARG,      TM_LOGIN            },
/* TM_QUIT: QUIT <CRLF> */
 { { 'Q','U','I','T' },
         tfFtpdQuit, 0,                TM_NOARG,      TM_NOLOGIN          },
/* TM_SMNT: SMNT <SP> <pathname> <CRLF> (Structure Mount:) */
 { { 'S','M','N','T' },
         tfFtpdSmnt, TM_FS_SMNT_FLAG,  TM_STRING,     TM_LOGIN            },
/* TM_REIN: REIN <CRLF> */
 { { 'R','E','I','N' },
         tfFtpdRein, 0,                TM_NOARG,      TM_LOGIN            },
/* TM_PORT: PORT <SP> <host-port> <CRLF> */
 { { 'P','O','R','T' },
         tfFtpdPort, 0,                TM_STRING,     TM_LOGIN            },
/* TM_PASV: PASV <CRLF> */
 { { 'P','A','S','V' },
         tfFtpdPasv, 0,                TM_NOARG,      TM_LOGIN            },
/* TM_TYPE: TYPE <SP> <type-code> <CRLF> */
 { { 'T','Y','P','E' },
         tfFtpdType, 0,                TM_STRING,     TM_NOLOGIN|TM_UPPER },
/* TM_STRU: STRU <SP> <structure-code> <CRLF> */
 { { 'S','T','R','U' },
         tfFtpdStru, 0,                TM_STRING,     TM_NOLOGIN|TM_UPPER },
/* TM_MODE: MODE <SP> <mode-code> <CRLF> */
 { { 'M','O','D','E' },
         tfFtpdMode, 0,                TM_STRING,     TM_NOLOGIN|TM_UPPER },
/* TM_RETR: RETR <SP> <pathname> <CRLF> */
 { { 'R','E','T','R' },
         tfFtpdRetr, TM_FS_RETR_FLAG,  TM_STRING,     TM_LOGIN
#ifdef TM_USE_FTPD_SSL
                                                    | TM_PROT_DATA_REQUIRED
#endif /* TM_USE_FTPD_SSL */
                                                                          },
/* TM_STOR: STOR <SP> <pathname> <CRLF> */
 { { 'S','T','O','R' },
         tfFtpdStor, TM_FS_STOR_FLAG,  TM_STRING,     TM_LOGIN
#ifdef TM_USE_FTPD_SSL
                                                    | TM_PROT_DATA_REQUIRED
#endif /* TM_USE_FTPD_SSL */
                                                                          },
/* TM_STOU: STOU <CRLF> (Store Unique:) */
 { { 'S','T','O','U' },
         tfFtpdStou, TM_FS_STORU_FLAG, TM_STRINGOPT,  TM_LOGIN
#ifdef TM_USE_FTPD_SSL
                                                    | TM_PROT_DATA_REQUIRED
#endif /* TM_USE_FTPD_SSL */
                                                                          },
/* TM_APPE: APPE <SP> <pathname> <CRLF> */
 { { 'A','P','P','E' },
         tfFtpdAppe, TM_FS_APPEND_FLAG, TM_STRING,    TM_LOGIN
#ifdef TM_USE_FTPD_SSL
                                                    | TM_PROT_DATA_REQUIRED
#endif /* TM_USE_FTPD_SSL */
                                                                          },

/* TM_ALLO: ALLO <SP> <decimal-integer> [<SP> R <SP> <decimal-integer>] <CRLF>*/
 { { 'A','L','L','O' },
         tfFtpdNoop, 0,                 TM_STRING,    TM_LOGIN|TM_NOSUP   },
/* TM_REST: REST <SP> <marker> <CRLF> */
 { { 'R','E','S','T' },
         tfFtpdNoop, 0,                 TM_STRING,    TM_LOGIN|TM_NOSUP   },
/* TM_RNFR: RNFR <SP> <pathname> <CRLF> */
 { { 'R','N','F','R' },
         tfFtpdRnfr, TM_FS_RENAME_FLAG, TM_STRING,    TM_LOGIN            },
/* TM_RNTO: RNTO <SP> <pathname> <CRLF> */
 { { 'R','N','T','O' },
         tfFtpdRnto, TM_FS_RENAME_FLAG, TM_STRING,    TM_LOGIN            },
/* TM_ABOR: ABOR <CRLF> */
 { { 'A','B','O','R' },
         tfFtpdAbor, 0,                 TM_NOARG,     TM_LOGIN            },
/* TM_DELE: DELE <SP> <pathname> <CRLF> */
 { { 'D','E','L','E' },
         tfFtpdDele, TM_FS_DELETE_FLAG, TM_STRING,    TM_LOGIN            },
/* TM_RMD: RMD <SP> <pathname> <CRLF> (Remove Directory:) */
 { { 'R','M','D',' ' },
         tfFtpdRmd,  TM_FS_RMD_FLAG,    TM_STRING,    TM_LOGIN            },
/* TM_MKD: MKD  <SP> <pathname> <CRLF> (Make Directory:) */
 { { 'M','K','D',' ' },
         tfFtpdMkd,  TM_FS_MKD_FLAG,    TM_STRING,    TM_LOGIN            },
/* TM_PWD: PWD  <CRLF> (Print Directory:) */
 { { 'P','W','D',' ' },
         tfFtpdPwd,  TM_FS_PWD_FLAG,    TM_NOARG,     TM_LOGIN            },
/* TM_LIST: LIST [<SP> <pathname>] <CRLF> */
 { { 'L','I','S','T' },
         tfFtpdList, TM_FS_LIST_FLAG,   TM_STRINGOPT, TM_LOGIN
#ifdef TM_USE_FTPD_SSL
                                                    | TM_PROT_DATA_REQUIRED
#endif /* TM_USE_FTPD_SSL */
                                                                           },
/* TM_NLST: NLST [<SP> <pathname>] <CRLF> */
 { { 'N','L','S','T' },
         tfFtpdNlst, TM_FS_NLST_FLAG,   TM_STRINGOPT, TM_LOGIN
#ifdef TM_USE_FTPD_SSL
                                                    | TM_PROT_DATA_REQUIRED
#endif /* TM_USE_FTPD_SSL */
                                                                          },
/* TM_SITE: SITE <SP> <string> <CRLF> */
 { { 'S','I','T','E' },
         tfFtpdSite, 0,                 TM_STRING,    TM_LOGIN            },
/* TM_SYST: SYST <CRLF> (System:) */
 { { 'S','Y','S','T' },
         tfFtpdSyst, 0,                 TM_NOARG,     TM_NOLOGIN          },
/* TM_STAT: STAT [<SP> <pathname>] <CRLF> */
 { { 'S','T','A','T' },
         tfFtpdNoop, 0,                 TM_STRINGOPT, TM_LOGINARG|TM_NOSUP},
/* TM_HELP: HELP [<SP> <string>] <CRLF> */
 { { 'H','E','L','P' },
         tfFtpdNoop, 0,                 TM_STRINGOPT, TM_NOLOGIN|TM_NOSUP },
/* TM_XCWD: XCWD  <SP> <pathname> <CRLF> */
 { { 'X','C','W','D' },
         tfFtpdCwd,  TM_FS_CWD_FLAG,    TM_STRING,    TM_LOGIN            },
/* TM_XCUP: XCUP <CRLF> (Change to parent directory:) */
 { { 'X','C','U','P' },
         tfFtpdCdup, TM_FS_CWD_FLAG,    TM_NOARG,     TM_LOGIN            },
/* TM_XRMD: XRMD <SP> <pathname> <CRLF> (Remove Directory:) */
 { { 'X','R','M','D' },
         tfFtpdRmd,  TM_FS_RMD_FLAG,    TM_STRING,    TM_LOGIN            },
/* TM_XMKD: XMKD  <SP> <pathname> <CRLF> (Make Directory:) */
 { { 'X','M','K','D' },
         tfFtpdMkd,  TM_FS_MKD_FLAG,    TM_STRING,    TM_LOGIN            },
/* TM_XPWD: XPWD  <CRLF> (Print Directory:) */
 { { 'X','P','W','D' },
         tfFtpdPwd,  TM_FS_PWD_FLAG,    TM_NOARG,     TM_LOGIN            },
/* TM_EPRT: EPRT <SP> <D><proto><D><addr><D><host-port><D> <CRLF> */
 { { 'E','P','R','T' },
         tfFtpdEprt, 0,                 TM_STRING,    TM_LOGIN            },
/* TM_EPSV: EPSV [ALL | <proto>] <CRLF> */
 { { 'E','P','S','V' },
         tfFtpdEpsv, 0,                 TM_STRINGOPT, TM_LOGIN            },
#ifdef TM_USE_FTPD_SSL
/* TM_AUTH: AUTH <SP> <authentication mechanism> <CRLF> */
 { { 'A','U','T','H' },
         tfFtpdAuth, 0,                 TM_STRING,    TM_NOLOGIN          },
/* TM_ADAT: ADAT <SP> <security data> <CRLF> */
 { { 'A','D','A','T' },
         tfFtpdAdat, 0,                 TM_STRING,    TM_LOGIN|TM_NOSUP   },
/* TM_PROT: PROT <SP> <channel protection level> <CRLF> */
 { { 'P','R','O','T' },
         tfFtpdProt, 0,                 TM_STRING,    TM_LOGIN|TM_UPPER   },
/* TM_PBSZ: PBSZ <SP> <buffer size> <CRLF> */
 { { 'P','B','S','Z' },
         tfFtpdPbsz, 0,                 TM_STRING,    TM_NOLOGIN          },
/* TM_CCC: CCC <CRLF> */
 { { 'C','C','C',' ' },
         tfFtpdCcc, 0,                  TM_NOARG,     TM_NOLOGIN          },
/* TM_ENC: ENC <SP> <private message> <CRLF> */
 { { 'E','N','C',' ' },
         tfFtpdEnc, 0,                  TM_STRINGOPT, TM_LOGIN            },
/* TM_FEAT: FEAT <CRLF> */
 { { 'F','E','A','T' },
         tfFtpdFeat, 0,                 TM_NOARG,     TM_NOLOGIN          },
#endif /* TM_USE_FTPD_SSL */
/* TM_NOOP: NOOP <CRLF> */
 { { 'N','O','O','P' },
         tfFtpdNoop, 0,                 TM_NOARG,     TM_NOLOGIN          }
};
#else /* !TM_NO_NESTED_BRACES */
static const ttFtpdCmdEntry  TM_CONST_QLF tlFtpdCmdArray[TM_NOOP + 1] =
{
/* TM_USER: USER <SP> <username> <CRLF> */
 { {{ 'U','S','E','R' }},
         tfFtpdUser, 0,                TM_STRINGOPT,  TM_NOLOGIN
#ifdef TM_USE_FTPD_SSL
                                                    |  TM_AUTH_CTRL_REQUIRED
#endif /* TM_USE_FTPD_SSL */
                                                                          },
/* TM_PASS: PASS <SP> <password> <CRLF> */
 { {{ 'P','A','S','S' }},
         tfFtpdPass, 0,                TM_STRINGOPT,  TM_NOLOGIN
#ifdef TM_USE_FTPD_SSL
                                                    | TM_AUTH_CTRL_REQUIRED
#endif /* TM_USE_FTPD_SSL */
                                                                          },
/* TM_ACCT: ACCT <SP> <account-information> <CRLF> */
 { {{ 'A','C','C','T' }},
         tfFtpdAcct, 0,                TM_STRING,     TM_NOLOGIN          },
/* TM_CWD: CWD  <SP> <pathname> <CRLF> */
 { {{ 'C','W','D',' ' }},
         tfFtpdCwd,  TM_FS_CWD_FLAG,   TM_STRING,     TM_LOGIN            },
/* TM_CDUP: CDUP <CRLF> (Change to parent directory:) */
 { {{ 'C','D','U','P' }},
         tfFtpdCdup, TM_FS_CWD_FLAG,   TM_NOARG,      TM_LOGIN            },
/* TM_QUIT: QUIT <CRLF> */
 { {{ 'Q','U','I','T' }},
         tfFtpdQuit, 0,                TM_NOARG,      TM_NOLOGIN          },
/* TM_SMNT: SMNT <SP> <pathname> <CRLF> (Structure Mount:) */
 { {{ 'S','M','N','T' }},
         tfFtpdSmnt, TM_FS_SMNT_FLAG,  TM_STRING,     TM_LOGIN            },
/* TM_REIN: REIN <CRLF> */
 { {{ 'R','E','I','N' }},
         tfFtpdRein, 0,                TM_NOARG,      TM_LOGIN            },
/* TM_PORT: PORT <SP> <host-port> <CRLF> */
 { {{ 'P','O','R','T' }},
         tfFtpdPort, 0,                TM_STRING,     TM_LOGIN            },
/* TM_PASV: PASV <CRLF> */
 { {{ 'P','A','S','V' }},
         tfFtpdPasv, 0,                TM_NOARG,      TM_LOGIN            },
/* TM_TYPE: TYPE <SP> <type-code> <CRLF> */
 { {{ 'T','Y','P','E' }},
         tfFtpdType, 0,                TM_STRING,     TM_NOLOGIN|TM_UPPER },
/* TM_STRU: STRU <SP> <structure-code> <CRLF> */
 { {{ 'S','T','R','U' }},
         tfFtpdStru, 0,                TM_STRING,     TM_NOLOGIN|TM_UPPER },
/* TM_MODE: MODE <SP> <mode-code> <CRLF> */
 { {{ 'M','O','D','E' }},
         tfFtpdMode, 0,                TM_STRING,     TM_NOLOGIN|TM_UPPER },
/* TM_RETR: RETR <SP> <pathname> <CRLF> */
 { {{ 'R','E','T','R' }},
         tfFtpdRetr, TM_FS_RETR_FLAG,  TM_STRING,     TM_LOGIN
#ifdef TM_USE_FTPD_SSL
                                                    | TM_PROT_DATA_REQUIRED
#endif /* TM_USE_FTPD_SSL */
                                                                          },
/* TM_STOR: STOR <SP> <pathname> <CRLF> */
 { {{ 'S','T','O','R' }},
         tfFtpdStor, TM_FS_STOR_FLAG,  TM_STRING,     TM_LOGIN
#ifdef TM_USE_FTPD_SSL
                                                    | TM_PROT_DATA_REQUIRED
#endif /* TM_USE_FTPD_SSL */
                                                                          },
/* TM_STOU: STOU <CRLF> (Store Unique:) */
 { {{ 'S','T','O','U' }},
         tfFtpdStou, TM_FS_STORU_FLAG, TM_STRINGOPT,  TM_LOGIN
#ifdef TM_USE_FTPD_SSL
                                                    | TM_PROT_DATA_REQUIRED
#endif /* TM_USE_FTPD_SSL */
                                                                          },
/* TM_APPE: APPE <SP> <pathname> <CRLF> */
 { {{ 'A','P','P','E' }},
         tfFtpdAppe, TM_FS_APPEND_FLAG, TM_STRING,    TM_LOGIN
#ifdef TM_USE_FTPD_SSL
                                                    | TM_PROT_DATA_REQUIRED
#endif /* TM_USE_FTPD_SSL */
                                                                          },
/* TM_ALLO: ALLO <SP> <decimal-integer> [<SP> R <SP> <decimal-integer>] <CRLF>*/
 { {{ 'A','L','L','O' }},
         tfFtpdNoop, 0,                 TM_STRING,    TM_LOGIN|TM_NOSUP   },
/* TM_REST: REST <SP> <marker> <CRLF> */
 { {{ 'R','E','S','T' }},
         tfFtpdNoop, 0,                 TM_STRING,    TM_LOGIN|TM_NOSUP   },
/* TM_RNFR: RNFR <SP> <pathname> <CRLF> */
 { {{ 'R','N','F','R' }},
         tfFtpdRnfr, TM_FS_RENAME_FLAG, TM_STRING,    TM_LOGIN            },
/* TM_RNTO: RNTO <SP> <pathname> <CRLF> */
 { {{ 'R','N','T','O' }},
         tfFtpdRnto, TM_FS_RENAME_FLAG, TM_STRING,    TM_LOGIN            },
/* TM_ABOR: ABOR <CRLF> */
 { {{ 'A','B','O','R' }},
         tfFtpdAbor, 0,                 TM_NOARG,     TM_LOGIN            },
/* TM_DELE: DELE <SP> <pathname> <CRLF> */
 { {{ 'D','E','L','E' }},
         tfFtpdDele, TM_FS_DELETE_FLAG, TM_STRING,    TM_LOGIN            },
/* TM_RMD: RMD <SP> <pathname> <CRLF> (Remove Directory:) */
 { {{ 'R','M','D',' ' }},
         tfFtpdRmd,  TM_FS_RMD_FLAG,    TM_STRING,    TM_LOGIN            },
/* TM_MKD: MKD  <SP> <pathname> <CRLF> (Make Directory:) */
 { {{ 'M','K','D',' ' }},
         tfFtpdMkd,  TM_FS_MKD_FLAG,    TM_STRING,    TM_LOGIN            },
/* TM_PWD: PWD  <CRLF> (Print Directory:) */
 { {{ 'P','W','D',' ' }},
         tfFtpdPwd,  TM_FS_PWD_FLAG,    TM_NOARG,     TM_LOGIN            },
/* TM_LIST: LIST [<SP> <pathname>] <CRLF> */
 { {{ 'L','I','S','T' }},
         tfFtpdList, TM_FS_LIST_FLAG,   TM_STRINGOPT, TM_LOGIN
#ifdef TM_USE_FTPD_SSL
                                                    | TM_PROT_DATA_REQUIRED
#endif /* TM_USE_FTPD_SSL */
                                                                          },
/* TM_NLST: NLST [<SP> <pathname>] <CRLF> */
 { {{ 'N','L','S','T' }},
         tfFtpdNlst, TM_FS_NLST_FLAG,   TM_STRINGOPT, TM_LOGIN
#ifdef TM_USE_FTPD_SSL
                                                    | TM_PROT_DATA_REQUIRED
#endif /* TM_USE_FTPD_SSL */
                                                                          },
/* TM_SITE: SITE <SP> <string> <CRLF> */
 { {{ 'S','I','T','E' }},
         tfFtpdSite, 0,                 TM_STRING,    TM_LOGIN            },
/* TM_SYST: SYST <CRLF> (System:) */
 { {{ 'S','Y','S','T' }},
         tfFtpdSyst, 0,                 TM_NOARG,     TM_NOLOGIN          },
/* TM_STAT: STAT [<SP> <pathname>] <CRLF> */
 { {{ 'S','T','A','T' }},
         tfFtpdNoop, 0,                 TM_STRINGOPT, TM_LOGINARG|TM_NOSUP},
/* TM_HELP: HELP [<SP> <string>] <CRLF> */
 { {{ 'H','E','L','P' }},
         tfFtpdNoop, 0,                 TM_STRINGOPT, TM_NOLOGIN|TM_NOSUP },
/* TM_XCWD: XCWD  <SP> <pathname> <CRLF> */
 { {{ 'X','C','W','D' }},
         tfFtpdCwd,  TM_FS_CWD_FLAG,    TM_STRING,    TM_LOGIN            },
/* TM_XCUP: XCUP <CRLF> (Change to parent directory:) */
 { {{ 'X','C','U','P' }},
         tfFtpdCdup, TM_FS_CWD_FLAG,    TM_NOARG,     TM_LOGIN            },
/* TM_XRMD: XRMD <SP> <pathname> <CRLF> (Remove Directory:) */
 { {{ 'X','R','M','D' }},
         tfFtpdRmd,  TM_FS_RMD_FLAG,    TM_STRING,    TM_LOGIN            },
/* TM_XMKD: XMKD  <SP> <pathname> <CRLF> (Make Directory:) */
 { {{ 'X','M','K','D' }},
         tfFtpdMkd,  TM_FS_MKD_FLAG,    TM_STRING,    TM_LOGIN            },
/* TM_XPWD: XPWD  <CRLF> (Print Directory:) */
 { {{ 'X','P','W','D' }},
         tfFtpdPwd,  TM_FS_PWD_FLAG,    TM_NOARG,     TM_LOGIN            },
/* TM_EPRT: EPRT <SP> <D><proto><D><addr><D><host-port><D> <CRLF> */
 { {{ 'E','P','R','T' }},
         tfFtpdEprt, 0,                 TM_STRING,    TM_LOGIN            },
/* TM_EPSV: EPSV [ALL | <proto>] <CRLF> */
 { {{ 'E','P','S','V' }},
         tfFtpdEpsv, 0,                 TM_STRINGOPT, TM_LOGIN            },
#ifdef TM_USE_FTPD_SSL
/* TM_AUTH: AUTH <SP> <authentication mechanism> <CRLF> */
 { {{ 'A','U','T','H' }},
         tfFtpdAuth, 0,                 TM_STRING,    TM_NOLOGIN|TM_UPPER },
/* TM_ADAT: ADAT <SP> <security data> <CRLF> */
 { {{ 'A','D','A','T' }},
         tfFtpdAdat, 0,                 TM_STRING,    TM_NOLOGIN|TM_NOSUP },
/* TM_PROT: PROT <SP> <channel protection level> <CRLF> */
 { {{ 'P','R','O','T' }},
         tfFtpdProt, 0,                 TM_STRING,    TM_NOLOGIN|TM_UPPER },
/* TM_PBSZ: PBSZ <SP> <buffer size> <CRLF> */
 { {{ 'P','B','S','Z' }},
         tfFtpdPbsz, 0,                 TM_STRING,    TM_NOLOGIN          },
/* TM_CCC: CCC <CRLF> */
 { {{ 'C','C','C',' ' }},
         tfFtpdCcc, 0,                  TM_NOARG,     TM_NOLOGIN          },
/* TM_ENC: ENC <SP> <64 bit encoded arg> <CRLF> */
 { {{ 'E','N','C',' ' }},
         tfFtpdEnc, 0,                  TM_STRINGOPT, TM_NOLOGIN          },
/* TM_FEAT: FEAT <CRLF> */
 { {{ 'F','E','A','T' }},
         tfFtpdFeat, 0,                 TM_NOARG,     TM_NOLOGIN          },
#endif /* TM_USE_FTPD_SSL */
/* TM_NOOP: NOOP <CRLF> */
 { {{ 'N','O','O','P' }},
         tfFtpdNoop, 0,                 TM_NOARG,     TM_NOLOGIN          }
};
#endif /* !TM_NO_NESTED_BRACES */

/*
 * Null terminated reply strings.
 * Make sure every one of the reply string is a multiple of 4 bytes.
 */
static const char  TM_CONST_QLF tlFtpd200[] =
    "200 xxx  Command okay.\r\n"; /* 25 */
/*   12345678901234567890123456789012345678901234567890123456789 */
/*   0        1         2         3         4         5          */
static const char  TM_CONST_QLF tlFtpd220[] =
    "220 Treck FTP server ready.\r\n"; /* 30 */
static const char  TM_CONST_QLF tlFtpd421[] =
    "421 Service not available, closing control connection.\r\n"; /* 58 */
static const char  TM_CONST_QLF tlFtpd425[] =
    "425 xxx  can't open data connection.\r\n"; /* 38 */
static const char  TM_CONST_QLF tlFtpd501[] =
    "501 Syntax error in parameters or arguments.\r\n"; /* 47 */
static const char  TM_CONST_QLF tlFtpd202[] =
    "202 xxx  command not implemented, superfluous at this site.\r\n";/* 61 */
static const char  TM_CONST_QLF tlFtpd502[] =
    "502 xxx  command not implemented, or not allowed.\r\n"; /* 51 */
static const char  TM_CONST_QLF tlFtpd503[] =
    "503 xxx  bad sequence of commands.\r\n"; /* 38 */
static const char  TM_CONST_QLF tlFtpd530[] =
    "530 Not logged in.\r\n"; /* 21 */
static const char  TM_CONST_QLF tlFtpd504[] =
    "504 xxx  command not implemented for that parameter.\r\n"; /* 54 */
static const char  TM_CONST_QLF tlFtpd250[] =
    "250 xxx  requested file action okay, completed.\r\n"; /* 49 */
static const char  TM_CONST_QLF tlFtpd550[] =
    "550 xxx  requested action not taken.\r\n"; /* 38 */
static const char  TM_CONST_QLF tlFtpd221[] =
    "221 Goodbye.\r\n"; /* 15 */
static const char  TM_CONST_QLF tlFtpd426[] =
    "426 Connection closed; transfer aborted.\r\n"; /* 43 */
static const char  TM_CONST_QLF tlFtpd226[] =
    "226 Closing data connection.\r\n"; /* 31 */
static const char  TM_CONST_QLF tlFtpd230[] =
    "230 User logged in, proceed.\r\n"; /* 31 */
static const char  TM_CONST_QLF tlFtpd331[] =
    "331 User name okay, need password.\r\n"; /* 37 */
static const char  TM_CONST_QLF tlFtpd150[] =
    "150 File status okay; about to open data connection.\r\n"; /* 55 */
static const char  TM_CONST_QLF tlFtpd125[] =
    "125 Data connection already open; transfer starting.\r\n"; /* 55 */
static const char  TM_CONST_QLF tlFtpd350[] =
    "350 Requested file action pending further information.\r\n"; /* 57 */

#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
/* Both IPv4 and IPv6 are enabled. */
static const char  TM_CONST_QLF tlFtpd522_v4v6[] =
    "522 Network protocol not supported, use (1,2)\r\n"; /* 49 */
#else /* !TM_USE_IPV4 */
/* Just IPv6 is enabled. */
static const char  TM_CONST_QLF tlFtpd522_v6[] =
    "522 Network protocol not supported, use (2)\r\n"; /* 47 */
#endif /* TM_USE_IPV4 */
#endif /* TM_USE_IPV6 */

#ifdef TM_USE_IPV4
/* IPv4 is enabled */
static const char  TM_CONST_QLF tlFtpd522_v4[] =
    "522 Network protocol not supported, use (1)\r\n"; /* 47 */
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_FTPD_SSL
/* Security extention reply pointers. Defined in RFC 959 */
static const char  TM_CONST_QLF tlFtpdPbsz200[] =
    "200 PBSZ=0\r\n"; /* 12 */
const char  TM_CONST_QLF tlFtpd232[] =
    "232 User logged in, authorized by security data exchange.\r\n"; /* 59 */
const char  TM_CONST_QLF tlFtpd234[] =
    "234 AUTH xxx accepted.\r\n"; /* 24 */
const char  TM_CONST_QLF tlFtpd235[] =
    "235 Security data accepted.\r\n"; /* 29 */
const char  TM_CONST_QLF tlFtpd334[] =
    "334 xxx security mechanism accepted. Security data required.\r\n"; /* 62 */
const char  TM_CONST_QLF tlFtpd335[] =
    "335 Security data accepted but additional data is required.\r\n"; /* 61 */
const char  TM_CONST_QLF tlFtpd431[] =
    "431 Unable to use security mechanism.\r\n"; /* 39 */
const char  TM_CONST_QLF tlFtpd521[] =
    "521 Data connection cannot be opened with this PROT setting.\r\n"; /* 62 */
const char  TM_CONST_QLF tlFtpd534[] =
    "534 Not willing to use security mechanism.\r\n"; /* 44 */
const char  TM_CONST_QLF tlFtpd535[] =
    "535 Server rejects the security data.\r\n"; /* 29 */
const char  TM_CONST_QLF tlFtpd536[] =
    "536 Requested PROT level not supported by mechanism.\r\n"; /* 54 */
#endif /* TM_USE_FTPD_SSL */


// MH810100(S) K.Onodera 2019/11/13 車番チケットレス（Kasago6.0.1.33）
#if 0
/*
 * tfFSSystem function description:
 * Copy into bufferPtr (up to bufferSize bytes), the official system name,
 * as assigned in the list of "OPERATING SYSTEM NAMES" in the
 * "Assigned Numbers" RFC (RFC 1700). For example the DOS operating system
 * has been assigned DOS as system name. If the file system is a DOS file
 * system, then this function should copy "DOS" into bufferPtr. If the
 * system has not been assigned a system name in the RFC, then this function
 * should return -1.
 * Parameters
 * Parameter    Description
 * bufferPtr    Pointer to a buffer where to copy the system name
 * bufferSize   size in bytes of the buffer.
 * Returns
 * Value        Meaning
 * -1           failure
 *  > 0         number of bytes copied.
 */
static const char TM_CONST_QLF tlRamSystem[] = "Windows_NT";

int tfFSSystem (char TM_FAR * bufferPtr, int bufferSize)
{
	int size;

    size = tm_min((unsigned)bufferSize, (sizeof(tlRamSystem) - 1)); 
    (void)tm_strncpy(bufferPtr, tlRamSystem, size);
    return size;
}
#endif
// MH810100(E) K.Onodera 2019/11/13 車番チケットレス（Kasago6.0.1.33）

/*
 * tfNgFtpdUserStart function description:
 * Start the Ftp server. Should only be called once, or after tfFtpdUserStop.
 * 0.  Check on validity of parameters
 * 1.  Lock the ftp server execute (for re-entrancy)
 * 2.  Fail if server has already been started
 * 3.  Save passed parameters in local variables, and initialize
 *     local variables.
 * 4.  Open listening socket
 * 5.  Set listening socket to non blocking state
 * 6.  Bind to Ftp server port
 * 7.  Register for ACCEPT call back on listening socket.
 * 8.  Listen up to maximum number of connections allowed by the user
 * 9.  If all succeed and in blocking mode, create a Pend semaphore, and
 *     call the FTP server main loop tfFtpdExecute().
 * 10. If any error occurs, retrieve socket error if needed, and close
 *     the listening socket
 * 11. Unlock the FTP server execute
 * 12. Return errorCode
 *
 * If blockingState is TM_BLOCKING_ON, then tfNgFtpdUserStart will block
 * inside tfFtpdExecute() main loop and return only when tfFtpdUserStop
 * is called. If blockingState is TM_BLOCKING_OFF, the tfNgFtpdUserStart
 * will return, and the user will need to call tfFtpdUserExecute()
 * periodically to keep executing the ftp server code.
 * Parameters
 * Value          Meaning
 * fileFlags      Flags for file structure supported by the system
 * maxConnections Maximum number of incoming connections at any one time
 * blockingState  Blocking or non blocking
 * Return value
 * 0                    success
 * TM_EALREADY          Ftpd already running
 * other socket error   failure
 */
int tfNgFtpdUserStart ( int           fileFlags,
                        int           maxConnections,
                        int           maxBackLog,
                        int           idleTimeout,
                        int           blockingState,
                        ttUser32Bit   flags)
{
    struct sockaddr_storage sockAddress;
    ttS32Bit                availableConnections;
    int                     addressLength;
    int                     errorCode;
    int                     listenDesc;
    int                     tempInt;
    int                     addrFamily;
    ttSockAddrPtrUnion      sockAddr;

    TM_UNREF_IN_ARG(flags);/* avoid compiler warning in IPv4 only mode. */

    errorCode = TM_ENOERROR;
    if (maxConnections != 0)
    {
/*
 * one extra socket to allow us to send an error message when we reach the
 * maximum.
 */
        tempInt = 1;
    }
    else
    {
        tempInt = 0;
    }
/* Number of available sockets in the system */
    availableConnections = (ttS32Bit) (   tm_context(tvMaxNumberSockets)
                                    - tm_context(tvNumberOpenSockets) );
/*
 * Number of available connections for the FTP server:
 * Take away one for the listening socket, and one for transient listening
 * data socket for a passive connection, and one to allow to send a 421
 * if we have reached the maximum number of connections, divide by 2 to
 * allow for 2 sockets (control, and data) per FTP connection.
 */
    availableConnections = (availableConnections - 2 - tempInt) / 2;
    if (    ( (fileFlags & ~TM_FS_ALLCMND_MASK) != 0 )
         || ( maxBackLog <= 0 )
         || ( maxBackLog > (int)availableConnections )
         || (    ( maxConnections != 0 )
              && (    ( maxConnections < maxBackLog )
                   || ( maxConnections > (int)availableConnections ) ) )
         || (idleTimeout < TM_FTPD_MIN_TIMEOUT)
         || (    (blockingState != TM_BLOCKING_ON)
              && (blockingState != TM_BLOCKING_OFF) )
         || (     (blockingState == TM_BLOCKING_ON)
              &&  (tvPendPostEnabled) == TM_8BIT_ZERO) )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        tm_call_lock_wait(&tm_context(tvFtpdExecuteLockEntry));
/* Check if not already started */
        if (    (tm_context(tvFtpdExecuting) != 0)
             || (tm_context(tvFtpdListenDesc) != TM_SOCKET_ERROR) )
        {
            errorCode = TM_EALREADY;
        }
        else
        {
            tm_context(tvFtpdShutdown) = 0;
            tm_context(tvFtpdFsFlags) = fileFlags;
            if (tm_context(tvFtpdFsFlags) & TM_FS_CRLF_FLAG)
            {
/*
 * Add special flag if file system use CR, LF for end
 * of line, so that we can use speedier binary type file transfer algorithm
 * for ASCII data transfer.
 */
                tm_context(tvFtpdFsTypeFlag) = TM_TYPE_BINARY_FLAG;
            }
            else
            {
                tm_context(tvFtpdFsTypeFlag) = 0;
            }
            tm_context(tvFtpdMaxConnections) = maxConnections;
/* In milliseconds */
            tm_context(tvFtpdIdleTimeout) =
                (tt32Bit)((tt32Bit)idleTimeout*TM_UL(1000));
/* Blocking state */
            tm_context(tvFtpdBlockingState) = blockingState;
/* Initialize pending sempahore */
            tm_bzero(&tm_context(tvFtpdPendSem), sizeof(ttPendEntry));
/* Number of pending connections. */
            tm_context(tvFtpdPendingConnections) = 0;
/*
 * Determine what type of socket to use (IPv4 or IPv6), depending on how the
 * stack is built, and whether the user wants to restrict connections to a
 * certain protocol family.
 */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
            if (flags & TM_FTPD_IPV4_ONLY)
            {
                addrFamily = AF_INET;
            }
            else
#endif /* TM_USE_IPV4 */
            {
                addrFamily = AF_INET6;
            }
#else /* !TM_USE_IPV6 */
            addrFamily = AF_INET;
#endif /* TM_USE_IPV6 */

/* Open a listening socket */
            listenDesc = socket(addrFamily, SOCK_STREAM, IP_PROTOTCP);
            if (listenDesc == TM_SOCKET_ERROR)
            {
                errorCode = TM_SOCKET_ERROR;
            }
            else
            {
                tm_context(tvFtpdListenDesc) = listenDesc;
/* Non blocking state */
                (void)tfBlockingState(listenDesc, TM_BLOCKING_OFF);
                addressLength = sizeof(struct sockaddr_storage);
                sockAddr.sockNgPtr = &sockAddress;
                errorCode = getsockname(
                                     listenDesc,
                                     sockAddr.sockPtr,
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
/* FTP server port */
                        sockAddress.ss_port = TM_FTPD_PORT;
/* Bind to server port */
                        errorCode = bind(
                                 listenDesc,
                                 sockAddr.sockPtr,
                                 sockAddr.sockPtr->sa_len );
                        if (errorCode == TM_ENOERROR)
                        {
/* Note: order of calls does not matter, since we own the execute lock */
/* Register CB_ACCEPT */
                            errorCode = tfRegisterSocketCB(
                                                       listenDesc,
                                                       tfFtpdSignalAcceptCtrl,
                                                       TM_CB_ACCEPT);
                            if (errorCode == TM_ENOERROR)
                            {
/* Listen for incoming connections */
                                errorCode = listen(listenDesc, maxBackLog);
                                if (errorCode == TM_ENOERROR)
                                {
#ifdef TM_PEND_POST_NEEDED
                                    if (blockingState != TM_BLOCKING_OFF)
                                    {
                                        tfFtpdExecute();
                                    }
#endif /* TM_PEND_POST_NEEDED */
                                }
                            }
                        }
                    }
                }
            }

            if (errorCode != TM_ENOERROR)
            {
                errorCode = tfGetSocketError(listenDesc);
/* We unlock, and re-lock in this routine */
                if (listenDesc != TM_SOCKET_ERROR)
                {
                    tfFtpdCloseListen();
                }
            }
        }
        tm_call_unlock(&tm_context(tvFtpdExecuteLockEntry));
    }
    return errorCode;
}

/*
 * DEPRECATED: calls tfNgFtpdUserStart
 */
int tfFtpdUserStart( int fileSystemFlags,
                     int maxConnections,
                     int maxBackLog,
                     int idleTimeout,
                     int blockingState )
{
    return tfNgFtpdUserStart ( fileSystemFlags,
                               maxConnections,
                               maxBackLog,
                               idleTimeout,
                               blockingState,
                               TM_UL(0) ); /* flags */
}

#ifdef TM_USE_FTPD_SSL
/*
 * tfNgFtpdUserStart function description:
 * Starts the server and sets the global SSL permissions. Should only 
 * be called once to start the server, or after tfFtpdUserStop.
 * 1 .Call tfNgFtpdUserStart
 * 2 .If the server is not already running, set tvFtpdSslCtrlPermissions,
 *    tvFtpdSslDataPermissions, and tvFtpdSslSessionId.
 * 3. Return errorCode from tfNgFtpdUserStart
 * 
 * Parameters
 * Value                 Meaning
 * fileFlags             Flags for file structure supported by the system
 * maxConnections        Maximum number of incoming connections at any one time
 * blockingState         Blocking or non blocking
 * sslCtrlPermissions    Whether or not the server allows, requires, or does 
 *                       not allow authentication on the control channel
 * sslDataPermissions    Whether or not the server allows, requires, or does 
 *                       not allow authentication on the data channel
 * ftpdSslSessionId      Server SSL session ID
 * reserved              An unused parameter for future backwards compatibility
 * 
 * Return value
 * TM_ENOERROR          Success
 * TM_EALREADY          Ftpd already running
 * other socket error   failure
 */
int tfFtpdSslUserStart ( int                fileFlags,
                         int                maxConnections,
                         int                maxBackLog,
                         int                idleTimeout,
                         int                blockingState,
                         ttUser32Bit        flags,
                         int                sslCtrlPermissions,
                         int                sslDataPermissions,
                         int                ftpdSslSessionId,
                         ttUserGenericUnion reserved )
{
    int errorCode;

/* Dummy parameter currently not used */
    TM_UNREF_IN_ARG(reserved);

    if (    (tm_context(tvFtpdExecuting) != 0)
             || (tm_context(tvFtpdListenDesc) != TM_SOCKET_ERROR) )
    {
        errorCode = TM_EALREADY;
    }
    else
    {
/* Set Global SSL permissions, securites, and session ID's */
        tm_context(tvFtpdSslCtrlPermissions) = sslCtrlPermissions;
        tm_context(tvFtpdSslDataPermissions) = sslDataPermissions; 
        tm_context(tvFtpdSslSessionId)   = ftpdSslSessionId;
    }

/* Start the server */
    errorCode = tfNgFtpdUserStart ( fileFlags,
                                    maxConnections,
                                    maxBackLog,
                                    idleTimeout,
                                    blockingState,
                                    flags );
    return errorCode;
}
#endif /* TM_USE_FTPD_SSL */
/*
 * tfFtpdUserStop function description:
 * Stop execution of the Ftp server.
 * 1. Lock the FTP server execute.
 * 2. If server not running, set error code to TM_EALREADY.
 * 3. Else If server running:
 * 3.1  set the tvFtpdShutdown flag
 * 3.2  . if in non blocking mode, call tfFtpdExecute() to close all active
 *        connections if not already in the execute loop
 *      . else (blocking mode), post on the server pending
 *        semaphore to let the execute loop close all open connections.
 * 4. Unlock.
 * 5. Return error code.
 * Parameter:
 * none
 * Return value:
 * 0            success
 * TM_EALREADY  FTP server was already stopped.
 */
int tfFtpdUserStop (void)
{
    int errorCode;

    tm_call_lock_wait(&tm_context(tvFtpdExecuteLockEntry));
    if (tm_context(tvFtpdListenDesc) == TM_SOCKET_ERROR)
    {
        errorCode = TM_EALREADY;
    }
    else
    {
        tm_context(tvFtpdShutdown) = 1;
#ifdef TM_USE_FTPD_SSL
/* Reset SSL control and data permssions to non supported */
        tm_context(tvFtpdSslCtrlPermissions) = TM_FTPD_SSL_NOT_SUPPORTED;
        tm_context(tvFtpdSslDataPermissions) = TM_FTPD_SSL_NOT_SUPPORTED;
#endif /* TM_USE_FTPD_SSL */
        if (tm_context(tvFtpdBlockingState) == TM_BLOCKING_OFF)
        {
/* Non blocking mode */
            if (tm_context(tvFtpdExecuting) == 0)
/* Call execute loop if not already in there */
            {
/* clean up */
                tfFtpdExecute();
            }
        }
#ifdef TM_PEND_POST_NEEDED
        else
        {
/*
 * Post (for the shutdown event).
 */
            tm_post( &tm_context(tvFtpdPendSem),
                           &tm_context(tvFtpdExecuteLockEntry),
                           TM_ENOERROR );
        }
#endif /* TM_PEND_POST_NEEDED */
        errorCode = TM_ENOERROR;
    }
    tm_call_unlock(&tm_context(tvFtpdExecuteLockEntry));
    return errorCode;
}



/*
 * tfFtpdUserExecute function description:
 * Execute the Ftp server main loop. Call valid only if tfNgFtpdUserStart had
 * been called in non blocking mode, and nobody is currently executing the
 * FTP server code main loop.
 * Parameters
 * none
 * Return value
 * 0                    success
 * TM_EPERM             Ftp server executing or not started or stopped
 * error                failure
 *
 */
int tfFtpdUserExecute (void)
{
    int errorCode;

    tm_call_lock_wait(&tm_context(tvFtpdExecuteLockEntry));
    if (    (tm_context(tvFtpdListenDesc) != TM_SOCKET_ERROR)
         && (tm_context(tvFtpdExecuting) == 0) )
    {
/*
 * tfNgFtpdUserStart has been called, and nobody is executing the server main
 * loop
 */
        tfFtpdExecute();
        errorCode = TM_ENOERROR;
    }
    else
    {
/* tfNgFtpdUserStart has not been called, or tfFtpdExecute is running */
        errorCode = TM_EPERM;
    }
    tm_call_unlock(&tm_context(tvFtpdExecuteLockEntry));
    return errorCode;
}

/*
 * tfFtpdSignalAcceptCtrl function description:
 * Call back function when an incoming FTP control connection arrives.
 * 1 Obtain exclusive access to the FTP server.
 * 2 If the FTP server is not in the process of shutting down, increase
 *   the number of pending connections, and post to the FTP server
 * 3 Release access to the FTP server.
 * Parameters:
 * Value          Meaning
 * listenDesc   FTP server listening socket
 * flags        TM_CB_ACCEPT (only)
 * Return value:
 * no return
 */
void tfFtpdSignalAcceptCtrl (int listenDesc, int flags)
{
    TM_UNREF_IN_ARG(flags);
    tm_call_lock_wait(&tm_context(tvFtpdExecuteLockEntry));
    if (    (tm_context(tvFtpdShutdown) == 0)
         && (tm_context(tvFtpdListenDesc) == listenDesc) )
    {
        tm_context(tvFtpdPendingConnections)++;
#ifdef TM_PEND_POST_NEEDED
        if (tm_context(tvFtpdBlockingState) != TM_BLOCKING_OFF)
        {
/* Post */
            tm_post( &tm_context(tvFtpdPendSem),
                           &tm_context(tvFtpdExecuteLockEntry),
                           TM_ENOERROR );
        }
#endif /* TM_PEND_POST_NEEDED */
    }
    tm_call_unlock(&tm_context(tvFtpdExecuteLockEntry));
}


/*
 * tfFtpdSignal function description:
 * Call back function when one or more registered events occur either on the
 * control connection or on the data connection.
 * 1. Obtain exclusive access to the FTP server.
 * 2. If FTP server is not in the process of shutting down:
 * 2.a Map socket to socket type (control or data socket).
 * 2.b For a CB timeout reset the timer pointer to indicate that the timeout
 *     timer is gone. If there is any activity on the connection the timeout
 *     occurred, cancel the CB timeout by resetting the CB flags to 0.
 * 2.c If CB flags non zero, add the CB flags to the current CB flags
 *     of the connection. If FTP connection not already in the run Q, append
 *     to the run q, and post on the FTP server.
 * 3. Release exclusive access to the FTP server.
 * Parameters:
 * Value          Meaning
 * socketDesc   socket (data or control) CB occurs on
 * ftpdPtr      pointer to the corresponding FTPD connection entry.
 * CBflags      call back flags on.
 * Return:
 * no return value.
 */
void tfFtpdSignal ( int               socketDesc,
                    ttVoidPtr         ftpdPtr,
                    int               CBFlags )
{
    int                   socketType;
#define ftpdConnEntryPtr ((ttFtpdConnEntryPtr)ftpdPtr)

    if (ftpdConnEntryPtr != (ttFtpdConnEntryPtr)0)
    {
        tm_lock_wait(&tm_context(tvFtpdExecuteLockEntry));
        if (tm_context(tvFtpdShutdown) == 0)
/* Server not in the process of being shutdown */
        {
            if (ftpdConnEntryPtr->fconDesc[TM_FTP_CTRL_SOCKET] == socketDesc)
            {
                socketType = TM_FTP_CTRL_SOCKET;
            }
            else
            {
                if (    ftpdConnEntryPtr->fconDesc[TM_FTP_DATA_SOCKET]
                     == socketDesc)
                {
                    socketType = TM_FTP_DATA_SOCKET;
                }
                else
                {
                    socketType = TM_FTP_NO_SOCKET;
                }
            }
            if (socketType != TM_FTP_NO_SOCKET)
            {
                if (CBFlags == TM_CB_TIMEOUT)
                {
/*
 * Note that the following is coded here as opposed to in the time out
 * routine, since we have the execute lock on here, and not in the timeout
 * routine.
 */
/* If the execute loop had just removed the timer, ignore the timeout*/
                    if (ftpdConnEntryPtr->fconTimerPtr == TM_TMR_NULL_PTR)
                    {
                        CBFlags = 0;
                    }
                    else
                    {
/*
 * The timer is non auto, so it is automatically removed by the timer thread,
 * upon return from this routine. Just indicate that it is gone.
 */
                        ftpdConnEntryPtr->fconTimerPtr = TM_TMR_NULL_PTR;
/*
 * If there is any activity on the connection
 * reset the CB_TIMEOUT CBFLags to prevent timeout, no need to signal
 * or to add the timeout timer, since if any of these flags are non
 * zero, we are guaranteed to be in the run Queue, and the timer
 * is automatically added in the execute loop.
 */
                        if (ftpdConnEntryPtr->fconCBFlags[socketType] != 0)
                        {
                            CBFlags = 0;
                        }
                    }
                }
                if (CBFlags != 0)
                {
                    ftpdConnEntryPtr->fconCBFlags[socketType] |=
                        (tt16Bit)CBFlags;

/* tfServerAppendTo Queue checks whether the entry is already in the queue */
                    tfServerAppendToQueue( &(tm_context(tvFtpdConQHead)),
                                           (ttServerQueuePtr)ftpdConnEntryPtr,
                                           TM_FTPD_RUN_Q );
#ifdef TM_PEND_POST_NEEDED
                    if (tm_context(tvFtpdBlockingState) != TM_BLOCKING_OFF)
                    {
/* Post */
                        tm_post( &tm_context(tvFtpdPendSem),
                                       &tm_context(tvFtpdExecuteLockEntry),
                                       TM_ENOERROR );
                    }
#endif /* TM_PEND_POST_NEEDED */
                }
            }
        }
        tm_unlock(&tm_context(tvFtpdExecuteLockEntry));
    }
    return;
#undef ftpdConnEntryPtr
}

/*
 * tfFtpdIdleTimeout function description:
 * Idle timeout function. Called when an FTP connection has been
 * inactive for tvFtpdIdleTimeout milliseconds.
 * We just call tfFtpdSignal with a Call back timeout flag, to use
 * common code.
 * The fconTimerPtr is reset in tfFtpdSignal with
 * the execute lock on to avoid race conditions with the FTP server
 * execute loop.
 *
 * Parameters:
 * Value          Meaning
 * timerArgPtr  Pointer to an FTPD connection entry
 *
 * Return:
 * no return value
 */
void tfFtpdIdleTimeout (ttVoidPtr      timerBlockPtr,
                        ttGenericUnion userParm1,
                        ttGenericUnion userParm2)
{
/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);

#define ftpdConnEntryPtr    ((ttFtpdConnEntryPtr)(userParm1.genVoidParmPtr))
    tfFtpdSignal(
                 ftpdConnEntryPtr->fconDesc[ftpdConnEntryPtr->fconTimerType],
                 userParm1.genVoidParmPtr,
                 TM_CB_TIMEOUT );
#undef ftpdConnEntryPtr
}

/*
 * tfFtpdExecute function description:
 * This is the main FTP server loop. Called with the execute lock on from
 * either tfNgFtpdUserStart(), tfFtpdUserExecute(), or (in non blocking mode)
 * from tfFtpdUserStop().
 * A. First, set the tvFtpdExecuting variable and start executing a do while
 *    loop until tvFtpdExecuting is reset
 * B. While there is an entry in the run Q, or the user wants to shutdown the
 *    server, or there is any pending connection:
 * B.1. We first check whether the user wants to stop the FTP server. In
 *    that case, we close the listenining socket, reset the pending connection
 *    count, and close each open connection (we unlock and relock the
 *    execute lock to close each open connection).
 * B.2. While there is any pending connection, try and accept each one of
 *    them (we unlock and relock the execute lock to accept).
 * B.3. If there is an entry in the RUN Q:
 *   B.3.a Save the control and data sockets call back flags,
 *       reset them in the connection entry, and remove the entry from the
 *       run Q, and remove the idle timeout timer.
 *   B.3.b Unlock the execute lock.
 *   B.3.c First examine the CB flags on the control socket
 *       . If a timeout occurred, close the control connection with a timeout
 *         error, and go back to the beginning of the loop (B.3.e).
 *       . If a recv or oob recv occurred, call the control connection
 *         receive function. If any error occurred on the control connection
 *         in the control receive function, close the control connection
 *         with that error and go back to the beginning of the loop (B.3.e).
 *         (Note this also handles an FTP client QUIT command since the
 *         receive function returns TM_ESHUTDOWN if the client FTP sends a
 *         QUIT command.)
 *       . If a remote close or reset occurred, close the control connection
 *         with a reset error, and go back to the beginning of the loop
 *         (B.3.e).
 *   B.3.d Next examine the CB flags on the data socket if the data socket is
 *       still around (executing FTP client commands in the FTP server
 *       control socket receive function, could have closed the data socket):
 *       . If CB timeout or CB accept flag is on, try and accept a data
 *         connection (could happen in passive mode only).
 *       . If CB connect complete flag is on, the active data connection has
 *         completed. Start transfering data.
 *       . If CB recv flag is on, Try and recv data on the data connection.
 *       . If CB send complete flag is on, Try and send data on the data
 *         connection.
 *       . If CB remote close or reset on, close the data connection, sending
 *         the appropriate reply code string (if not already done so).
 *   B.3.e If any error occurred on the control connection while processing
 *        the above control and data connection call back flags,
 *        close the control connection with that error and go back to
 *        the beginning of the loop.
 *   B.3.f If no error occurred on the control connection, add an idle
 *        timeout timer if appropriate (on the control connection if no data
 *        transfer is taking place, or if data transfer is taking place only
 *        if we are in passive mode waiting for an incoming connection).
 *   B.3.c Relock the execute lock.
 * C. No more work to do. If the the user has stopped the FTP server, reset
 *    the execute variable, tvFtpdExecuting, to exit this function.
 * D. Otherwise If the user called tfNgFtpdUserStart in non blocking mode,
 *    also reset the execute variable, tvFtpdExecuting to exit this function.
 * E. Otherwise (tvFtpdExecuting still on) pend to wait for the call back
 *    functions to post to our FTPD pend semaphore.
 * F. Exit the do while loop (A), if tvFtpdExecuting is reset.
 *
 * Parameters
 * none
 * Return value
 * 0                    success
 * TM_EPERM             Ftp server running in blocking mode or not started
 * error                failure
 *
 */
static void tfFtpdExecute (void)
{
    ttFtpdConnEntryPtr ftpdConnEntryPtr;
    ttCharPtr          replyPtr;
    int                ctrlCBFlags;
    int                dataCBFlags;
    int                errorCode;
    int                length;          /* BUG ID 37 */
    int                dataDesc;        /* BUG ID 37 */

    tm_context(tvFtpdExecuting) = 1;
    do
    {
/* Check if there is any FTPD work queued in */
        while (    ( tm_ftpd_q_not_empty(TM_FTPD_RUN_Q) )
                || ( tm_context(tvFtpdPendingConnections) != 0 )
                || ( tm_context(tvFtpdShutdown) != TM_8BIT_ZERO  ) )
        {
            if (tm_context(tvFtpdShutdown) != TM_8BIT_ZERO)
            {
/* Close the listening socket */
                tfFtpdCloseListen();
                tm_context(tvFtpdPendingConnections) = 0;
                ftpdConnEntryPtr = (ttFtpdConnEntryPtr)
         (tm_context(tvFtpdConQHead).servHeadNextQEntryPtr(TM_FTPD_CONN_LIST));
                while (    ftpdConnEntryPtr
                        != (ttFtpdConnEntryPtr)&(tm_context(tvFtpdConQHead)) )
                {
                    tm_call_unlock(&tm_context(tvFtpdExecuteLockEntry));
                    tfFtpdCloseCtrl(ftpdConnEntryPtr, TM_ECONNABORTED);
                    tm_call_lock_wait(&tm_context(tvFtpdExecuteLockEntry));
                    ftpdConnEntryPtr = (ttFtpdConnEntryPtr)
        (tm_context(tvFtpdConQHead).servHeadNextQEntryPtr(TM_FTPD_CONN_LIST));
                }
                tm_context(tvFtpdShutdown) = 0;
            }
            while (tm_context(tvFtpdPendingConnections) != 0)
            {
                tm_call_unlock(&tm_context(tvFtpdExecuteLockEntry));
                tfFtpdAcceptCtrl();
                tm_call_lock_wait(&tm_context(tvFtpdExecuteLockEntry));
                tm_context(tvFtpdPendingConnections)--;
            }
            if (tm_ftpd_q_not_empty(TM_FTPD_RUN_Q))
            {
                ftpdConnEntryPtr = (ttFtpdConnEntryPtr)
                   (tm_context(tvFtpdConQHead).servHeadNextQEntryPtr(
                                                              TM_FTPD_RUN_Q));
                ctrlCBFlags =
                            ftpdConnEntryPtr->fconCBFlags[TM_FTP_CTRL_SOCKET];
                dataCBFlags =
                            ftpdConnEntryPtr->fconCBFlags[TM_FTP_DATA_SOCKET];
                ftpdConnEntryPtr->fconCBFlags[TM_FTP_CTRL_SOCKET] = 0;
                ftpdConnEntryPtr->fconCBFlags[TM_FTP_DATA_SOCKET] = 0;
                tfServerRemoveFromQueue( &(tm_context(tvFtpdConQHead)),
                                         (ttServerQueuePtr)ftpdConnEntryPtr,
                                         TM_FTPD_RUN_Q );
/*
 * If we did not timeout, remove the idle timeout timer (with execute
 * lock on to avoid race conditions with the timeout function)
 */
                tfFtpdTimerRemove(ftpdConnEntryPtr);
/* BUG ID 37: Read dataDesc with lock on. */
                dataDesc = ftpdConnEntryPtr->fconDesc[TM_FTP_DATA_SOCKET];
                tm_call_unlock(&tm_context(tvFtpdExecuteLockEntry));
                errorCode = TM_ENOERROR;
                if (ctrlCBFlags & TM_CB_TIMEOUT)
                {
                    errorCode = TM_ETIMEDOUT;
/* Process next control block */
                    goto nextControlBlock;
                }
                if ( ctrlCBFlags & (TM_CB_RECV_OOB|TM_CB_RECV) )
                {
                    errorCode = tfFtpdRecvCtrl(ftpdConnEntryPtr);
                    if (errorCode != TM_ENOERROR)
                    {
/* Process next control block */
                        goto nextControlBlock;
                    }
                }
                if ( ctrlCBFlags & (   TM_CB_REMOTE_CLOSE
                                     | TM_CB_RESET
                                     | TM_CB_SOCKET_ERROR ) )
                {
/* Connection closed by peer, or reset, or timed out */
                    errorCode = TM_ECONNRESET;
/* Process next control block */
                    goto nextControlBlock;
                }
/*
 * data socket could have been closed by a just scanned FTP command
 * (QUIT, ABORT, or data transfer command in passive mode)
 */
                if (    ftpdConnEntryPtr->fconDesc[TM_FTP_DATA_SOCKET]
                     != TM_SOCKET_ERROR )
                {
                    if ( dataCBFlags & (TM_CB_ACCEPT|TM_CB_TIMEOUT) )
                    {
/* Passive data connection being established, or timed out */
                        errorCode = tfFtpdAcceptData(ftpdConnEntryPtr);
/*
 * Note that on an listening data socket, we only set this CB flag, go
 * fetch the next entry in the run queue
 */
                        goto nextControlBlock;
                    }
                    if (dataCBFlags & TM_CB_CONNECT_COMPLT)
                    {
/* Active data connection being established */
                        if (tm_16bit_bits_not_set(
                                             ftpdConnEntryPtr->fconFlags,
                                             TM_FTPF_DATA_CON ) )
/* If connection not already established */
                        {
/* Connection is established */
                            ftpdConnEntryPtr->fconFlags =
                                  (tt16Bit)(  ftpdConnEntryPtr->fconFlags
                                            | TM_FTPF_DATA_CON );
#ifdef TM_USE_FTPD_SSL
                            if ( tm_16bit_bits_not_set ( 
                                    ftpdConnEntryPtr->fconSslFlags,
                                    TM_FTPD_SSL_ENCRYPT_DATA) )
/* We're not encrypting the data, so send the data */
                            {
                                errorCode = tfFtpdTransferData(
                                        ftpdConnEntryPtr);
                            }
                            else
/* The data is supposed to be encrypted */
                            {
                                if ( tm_16bit_bits_not_set ( 
                                        ftpdConnEntryPtr->fconSslFlags,
                                        TM_FTPD_SSL_DATA_ENCRYPTED ) )
                                {
/* Set the options on the data socket */
                                    tfFtpdSslSetSockOpt(
                                            ftpdConnEntryPtr->fconDesc
                                                    [TM_FTP_DATA_SOCKET] );

/* Add the SSL layer to the data socket */
                                    errorCode = tfSslServerUserCreate(
                                            ftpdConnEntryPtr->fconDesc
                                                    [TM_FTP_DATA_SOCKET]);
                                    
/* Let the state machine know we're waiting for Ssl negotiation to finish */
                                    ftpdConnEntryPtr->fconSslFlags 
                                            |= TM_FTPD_SSL_NEGOTIATING;

                                    goto nextControlBlock;
                                }
                                else
                                {
                                    errorCode = tfFtpdTransferData(
                                            ftpdConnEntryPtr);
                                }
                            }
#else  /* !TM_USE_FTPD_SSL  */
/* Send the data */
                            errorCode = tfFtpdTransferData(ftpdConnEntryPtr);
#endif /* TM_USE_FTPD_SSL  */
/* Process next control block */
                            if (errorCode != TM_ENOERROR)
                            {
                                goto nextControlBlock;
                            }
                        }
                    }
#ifdef TM_USE_FTPD_SSL
                    if (dataCBFlags & TM_CB_SSL_ESTABLISHED)
                    {
                       ftpdConnEntryPtr->fconSslFlags |= 
                               TM_FTPD_SSL_DATA_ENCRYPTED;
                       ftpdConnEntryPtr->fconSslFlags &= 
                               ~TM_FTPD_SSL_NEGOTIATING;
#ifdef TM_AUTO_TEST
/*
 * When SSL is being used, we need to make sure that the client gets an extra 
 * turn to complete the handshaking. In real time situations this isn't 
 * necessary though, because the client has as much time as it needs to 
 * comunicate the information
 */
                               tm_set_current_context(tvContextHandleClient);
                               errorCode = tfFtpUserExecute(
                                                tm_context(tvClientFtpHandle));
                               tm_assert( tfFtpUserExecute,
                                          errorCode==TM_ENOERROR
                                       || errorCode==TM_EWOULDBLOCK );
                               tm_set_current_context(tvContextHandleServer);
#endif /* TM_AUTO_TEST */
                        errorCode = tfFtpdTransferData(ftpdConnEntryPtr);
                        goto nextControlBlock;
                    }

                    if (dataCBFlags & TM_CB_SSL_HANDSHK_FAILURE)
/* SSL handshaking failed. Mark the connection for shutdown */
                    {
/* Close the Control and Data channels. */
                        ftpdConnEntryPtr->fconErrorCode = TM_ESHUTDOWN;
                        goto nextControlBlock;
                    }
#endif /* TM_USE_FTPD_SSL */
                    if (dataCBFlags & TM_CB_RECV)
                    {
                        errorCode = tfFtpdRecvData(ftpdConnEntryPtr);
/* Process next control block */
                        if (errorCode != TM_ENOERROR)
                        {
                            goto nextControlBlock;
                        }
                    }
                    if (dataCBFlags & TM_CB_WRITE_READY)
                    {
                        errorCode = tfFtpdSendData(ftpdConnEntryPtr);
/* Process next control block */
                        if (errorCode != TM_ENOERROR)
                        {
                            goto nextControlBlock;
                        }
                    }
                    if ( dataCBFlags & (   TM_CB_REMOTE_CLOSE
                                         | TM_CB_RESET
                                         | TM_CB_SOCKET_ERROR ) )
/* Data connection closed by peer, or reset, or timed out */
                    {
/*
 * BUG ID 37: Check if tfFtpdRecvData() did not get to finish the read,
 *              because we had to process another connection block in the
 *              middle of reading the data.
 */
                        if (dataCBFlags & TM_CB_RECV)
                        {
                            length = tfGetWaitingBytes(dataDesc);
                            if (length != 0)
                            {
/* Keep flags that need further processing */
                                dataCBFlags =   dataCBFlags
                                              & (   TM_CB_RECV
                                                  | TM_CB_REMOTE_CLOSE
                                                  | TM_CB_RESET
                                                  | TM_CB_SOCKET_ERROR );
                                tfFtpdSignal( dataDesc,
                                              (ttVoidPtr)ftpdConnEntryPtr,
                                              dataCBFlags );
                                goto nextControlBlock;
                            }
                        }
/* End of BUG ID 37 */
                        if (    dataCBFlags & TM_CB_REMOTE_CLOSE
                             && tm_16bit_bits_not_set(
                                                  ftpdConnEntryPtr->fconFlags,
                                                  TM_FTPF_DATA_READ) )
                        {
/*
 * Peer closed the data connection, and we are not reading data from a file,
 * i.e. not sending to the peer. Process end of file, send a 226.
 */
                            replyPtr = (ttCharPtr)&tlFtpd226[0];
                        }
                        else
                        {
/* Reset connection, send reply error code */
                            if ( tm_16bit_one_bit_set(
                                              ftpdConnEntryPtr->fconFlags,
                                              TM_FTPF_DATA_CON) )
                            {
                                if (dataCBFlags & TM_CB_REMOTE_CLOSE)
                                {
/* Peer closed the connection, while we send data, reset the connection */
                                    tfResetConnection(
                                            ftpdConnEntryPtr->fconDesc
                                                    [TM_FTP_DATA_SOCKET]);
                                }
/* Connection closed, reset by peer, or timed out */
                                replyPtr = (ttCharPtr)&tlFtpd426[0];
                            }
                            else
                            {
/* Connect timed out */
                                replyPtr = (ttCharPtr)&tlFtpd425[0];
                            }
                        }
                        errorCode = tfFtpdEndData( ftpdConnEntryPtr,
                                                   replyPtr );
                        if (errorCode != TM_ENOERROR)
                        {
                            goto nextControlBlock;
                        }
                    }
                }
nextControlBlock:
                if (errorCode != TM_ENOERROR)
                {
                    tfFtpdCloseCtrl(ftpdConnEntryPtr, errorCode);
                }
                else
                {
/*
 * Add idle timeout timer
 */
                    tfFtpdTimerAdd(ftpdConnEntryPtr);
                }
                tm_call_lock_wait(&tm_context(tvFtpdExecuteLockEntry));
            }
        }
/* No more work to do. Check whether the user has stopped the FTP server */
        if (tm_context(tvFtpdListenDesc) == TM_SOCKET_ERROR)
        {
/* User stopped the FTP server. */
            tm_context(tvFtpdExecuting) = 0;
        }
        else
/* No more work to do, and FTP server still up, pend if allowed */
        {
#ifdef TM_PEND_POST_NEEDED
            if (tm_context(tvFtpdBlockingState) != TM_BLOCKING_OFF)
            {
                (void)tm_pend( &tm_context(tvFtpdPendSem),
                               &tm_context(tvFtpdExecuteLockEntry) );
            }
            else
#endif /* TM_PEND_POST_NEEDED */
            {
/* otherwise, exit the loop */
                tm_context(tvFtpdExecuting) = 0;
            }
        }
    }
    while ( tm_context(tvFtpdExecuting) != 0 );
}

#ifdef TM_USE_FTPD_SSL
/* tfFtpdSslSetSockOpt function description:
 * Sets the socket connection options for SSL connections on both the SSL
 * server and the SSL session.
 *
 * 1. Set socket options on SSL server with option value 1.
 * 2. Set socket options on SSL session with option values obtained from
 *    the session ID.
 *
 * Parameters:
 * Value                Meaning
 * socketId             Socket descriptor on which to set the options.
 */
static int tfFtpdSslSetSockOpt(int socketDescriptor)
{
    int errorCode;
    int socketOptionValue = 1;

/* Set socket options on SSL server */
    errorCode = setsockopt( socketDescriptor,
                            IP_PROTOTCP,
                            TM_TCP_SSL_SERVER,
                            (const char TM_FAR *)&socketOptionValue,
                            sizeof(int) );
    if (errorCode == TM_ENOERROR)
    {
/* Set socket SSL session ID */
        socketOptionValue = tm_context(tvFtpdSslSessionId);
/* Set socket options on SSL session */
        errorCode = setsockopt( socketDescriptor,
                                IP_PROTOTCP,
                                TM_TCP_SSLSESSION,
                                (const char TM_FAR *)&socketOptionValue,
                                sizeof(int) );
    }
    return errorCode;
}
#endif /* TM_USE_FTPD_SSL */ 

/*
 * tfFtpdCloseCtrl function description:
 * Close an FTP control connection, sending the appropriate FTP reply code
 * string, closing data connection if any, logging out the FTP client user
 * if any, removing the FTP connection from any queue, removing idle
 * timeout timer if any, and then free the FTPD connection entry.
 *
 * If control socket is open:
 * 1. If there is no errorCode or errorCode is TM_ECONNRESET, set
 *    reply code string to 221.
 * 2. Else if the errorCode is not TM_ESHUTDOWN (result of a QUIT command)
 *    set reply code string to 421, to indicate that the server is shutting
 *    down the service.
 * 3. Next send the reply code string, and close the data connection if any.
 *    (We set the data transfer bit to force tfFtpdEndData to send the reply
 *    code.)
 * 4. Close the control socket, and reset the control socket field in
 *    the connection entry.
 * 5. If user was logged in, log him out. tfFsUserLogout() will free
 *    the user data pointer.
 * 6. Reset all connection flags.
 * 7. Lock the FTPD execute lock.
 * 8. Remove the idle timeout timer if any.
 * 9. Remove the FTPD connection entry from the run queue, and the open
 *    connections list.
 * 10. Unlock the FTPD execute lock.
 * 11. Free the FTPD connection entry.
 * 12. Decrease the number of open connections.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * errorCode            errorCode that happens on the control connection.
 * Return code:
 * no return.
 */
static void tfFtpdCloseCtrl (ttFtpdConnEntryPtr ftpdConnEntryPtr,
                             int                errorCode)
{
    ttCharPtr         replyPtr;
    int               ctrlSocketDesc;

    ctrlSocketDesc = ftpdConnEntryPtr->fconDesc[TM_FTP_CTRL_SOCKET];
    replyPtr = (ttCharPtr)0;
    if (ctrlSocketDesc != TM_SOCKET_ERROR)
    {
        if (errorCode == TM_ENOERROR || errorCode == TM_ECONNRESET)
        {
/*
 * No error, or peer closed or reset the connection without sending a
 * QUIT command.
 */
            replyPtr = (ttCharPtr)&tlFtpd221[0];
        }
        else
        {
            if (errorCode != TM_ESHUTDOWN)
/*
 * TM_ESHUTDOWN error code means that we received a QUIT command and
 * already responded with a 221.
 * TM_ESHUTDOWN also means that we are un-initializing the stack
 */
            {
/* An error occurred */
                replyPtr = (ttCharPtr)&tlFtpd421[0];
            }
        }
        if (replyPtr != (ttCharPtr)0)
        {
/* Turn on transfer bit, so that tfFtpdEndData send the reply code string */
            ftpdConnEntryPtr->fconFlags = (tt16Bit)
                 (ftpdConnEntryPtr->fconFlags | TM_FTPF_DATA_TRANSFER);
        }
        (void)tfFtpdEndData(ftpdConnEntryPtr, replyPtr);
/*
 * Although, this value is read by the call back routine in the recv task
 * thread, there is no need to protect this reset, since the call back
 * routine, compares this value with the socket call back argument to
 * validate it.
 */
        ftpdConnEntryPtr->fconDesc[TM_FTP_CTRL_SOCKET] = TM_SOCKET_ERROR;
        tfFtpdCloseSocket(ctrlSocketDesc);
#ifdef TM_USE_FTPD_SSL
/* Clear the ctrl encrypted flag */
        if ( tm_16bit_one_bit_set( ftpdConnEntryPtr->fconSslFlags,
                                   TM_FTPD_SSL_CTRL_ENCRYPTED ) )
        {
            tfSslConnectUserClose(
                    ftpdConnEntryPtr->fconDesc[ TM_FTP_CTRL_SOCKET ] );
           ftpdConnEntryPtr->fconSslFlags &= ~TM_FTPD_SSL_CTRL_ENCRYPTED;
        }
#endif /* TM_USE_FTPD_SSL */
        if ( tm_16bit_one_bit_set(ftpdConnEntryPtr->fconFlags,
                                  TM_FTPF_USER_LOGGED_IN) )
        {
            tfFSUserLogout(ftpdConnEntryPtr->fconUserDataPtr);
        }
        ftpdConnEntryPtr->fconFlags = TM_16BIT_ZERO;
        tm_call_lock_wait(&tm_context(tvFtpdExecuteLockEntry));
/*
 * Remove the timer with timer lock on to avoid race conditions with the
 * timeout function.
 */
        tfFtpdTimerRemove(ftpdConnEntryPtr);
/* Remove entry from runq, and open connections list */
        tfServerRemoveFromQueue( &(tm_context(tvFtpdConQHead)),
                                 (ttServerQueuePtr)ftpdConnEntryPtr,
                                 TM_FTPD_RUN_Q );
        tfServerRemoveFromQueue( &(tm_context(tvFtpdConQHead)),
                                 (ttServerQueuePtr)ftpdConnEntryPtr,
                                 TM_FTPD_CONN_LIST );
        tm_call_unlock(&tm_context(tvFtpdExecuteLockEntry));
/* Free the entry */
        tm_free_raw_buffer((ttRawBufferPtr)ftpdConnEntryPtr);
    }
}

/*
 * tfFtpdCloseData function description:
 * Close a data connection, at the end of a transfer for file structure,
 * or in case of an error, or if a port command is received, or if a
 * passive command is received, or if an abort command is received, or
 * if reinit command is received.
 * 1. Close directory or file if any open.
 * 1. If FTPD connection entry data descriptor field is valid, close the data
 *    socket, and reset the data descriptor field in the FTPD connection
 *    entry.
 * 2. Reset all the data flags in the connection flags field.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 *
 * Return:
 * no return.
 */
static int tfFtpdCloseData (ttFtpdConnEntryPtr ftpdConnEntryPtr)
{
    int               dataSocketDesc;
    int               retCode;
    tt16Bit           conFlags;

    dataSocketDesc = ftpdConnEntryPtr->fconDesc[TM_FTP_DATA_SOCKET];
    retCode = tfFtpdCloseDirFile(ftpdConnEntryPtr);
    if (dataSocketDesc != TM_SOCKET_ERROR)
    {
/*
 * Although, this value is read by the call back routine in the recv task
 * thread, there is no need to protect resetting this field, since the call
 * back routine, compares this value with the socket call back argument to
 * validate it.
 */
        ftpdConnEntryPtr->fconDesc[TM_FTP_DATA_SOCKET] = TM_SOCKET_ERROR;
        tfFtpdCloseSocket(dataSocketDesc);

#ifdef TM_USE_FTPD_SSL
/* Clear the data encrypted flag */
        if ( tm_16bit_one_bit_set( ftpdConnEntryPtr->fconSslFlags,
                                   TM_FTPD_SSL_DATA_ENCRYPTED ) )
        {
            tfSslConnectUserClose(
                    ftpdConnEntryPtr->fconDesc[ TM_FTP_DATA_SOCKET ] );
            ftpdConnEntryPtr->fconSslFlags &= ~TM_FTPD_SSL_DATA_ENCRYPTED;
        }
#endif /* TM_USE_FTPD_SSL */
    }
    conFlags = ftpdConnEntryPtr->fconFlags;
    tm_16bit_clr_bit(conFlags, TM_FTPF_DATA_FLAGS);
    ftpdConnEntryPtr->fconFlags = conFlags;
    return retCode;
}

/*
 * tfFtpdCloseDirFile function description:
 * Function closed from tfFtpdCloseData().
 * Close an open directory or file at the end of a data transfer, or
 * if an error occurs.
 * Note that we close each data connection
 * at the end of a data transfer, so we could inline this code inside
 * tfFtpdCloseData.
 *
 * 1. If a file or directory has been successfully opened, call the FS
 *    specific routine to close the file or directory.
 * 2. Reset all the data transfer flags.
 * 3. If there is any outstanding zero copy buffer allocated, free it.
 * 4. Re-initialize all file transfer variables.
 * 5. If we had buffered an FTP command, while we were processing the
 *    data transfer, queue it to the run queue.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 *
 * Return:
 * no return.
 */
static int tfFtpdCloseDirFile (ttFtpdConnEntryPtr ftpdConnEntryPtr)
{
    ttVoidPtr         allocPtr;
    int               retCode;
    tt16Bit           conFlags;

    conFlags = ftpdConnEntryPtr->fconFlags;
    retCode = 0;
    if ( tm_16bit_one_bit_set( conFlags,
                               TM_FTPF_DATA_FILE | TM_FTPF_DATA_DIR) )
    {
        allocPtr = ftpdConnEntryPtr->fconFileDirPtr;
        if ( allocPtr != (ttVoidPtr)0 )
        {
            if ( tm_16bit_one_bit_set(conFlags, TM_FTPF_DATA_FILE) )
            {
                retCode = tfFSCloseFile( ftpdConnEntryPtr->fconUserDataPtr,
                                         allocPtr);
            }
            if ( tm_16bit_one_bit_set(conFlags, TM_FTPF_DATA_DIR) )
            {
                tfFSCloseDir( ftpdConnEntryPtr->fconUserDataPtr,
                              allocPtr);
            }
            ftpdConnEntryPtr->fconFileDirPtr = (ttVoidPtr)0;
        }

/* reset all data transfer flags */
        tm_16bit_clr_bit(conFlags, TM_FTPF_DATA_TRANSFER_FLAGS);
        ftpdConnEntryPtr->fconFlags = conFlags;


/* Free zero copy send buffer if any allocated. */
        if (ftpdConnEntryPtr->fconDataSendMessage != (ttUserMessage)0)
        {
            (void)tfFreeZeroCopyBuffer(ftpdConnEntryPtr->fconDataSendMessage);
            ftpdConnEntryPtr->fconDataSendMessage = (ttUserMessage)0;
        }

/* Transfer state (for type ascii, or structure record) */
        ftpdConnEntryPtr->fconTxState = TM_DATA_STATE;

/* Empty file input buffer */
        ftpdConnEntryPtr->fconFileInputProcessIndex = 0;
        ftpdConnEntryPtr->fconFileInputEndIndex = 0;
        ftpdConnEntryPtr->fconFileInputEorEof = TM_8BIT_ZERO;

        if (ftpdConnEntryPtr->fconDesc[TM_FTP_CTRL_SOCKET] != TM_SOCKET_ERROR)
/*
 * If we had buffered an FTP command, while we were processing the
 * data transfer, queue it to the run queue
 */
        {
            tfFtpdQueueUnScannedCtrlInput(ftpdConnEntryPtr);
        }
    }
    return retCode;
}

/*
 * tfFtpdEndData function description:
 * Called when we want to end a data transfer. Send a reply code string to
 * mark the end of the data transfer (if not already done so), and close
 * the data connection.
 * 1. CLose the data connection.
 * 2. If there is a data transfer that had not been ended yet:
 * 2.1 Add FTP command to the reply if reply code string is 425.
 * 2.2 send reply code string.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * replyPtr             reply code string to send to the FTP client.
 *
 * Return:
 */
static int tfFtpdEndData( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                          ttCharPtr          replyPtr )
{
    ttFtpdCmdEntryPtr ftpCmdPtr;
    int               errorCode;
    int               retCode;
    tt16Bit           fconFlags;

    errorCode = TM_ENOERROR;
/*
 * Save the flags, so that we know whether the TM_FTPF_DATA_TRANSFER
 * bit was on when we entered this routine.
 */
    fconFlags = ftpdConnEntryPtr->fconFlags;
/* Close the data connection (if any) */
    retCode = tfFtpdCloseData(ftpdConnEntryPtr);
    if (retCode != TM_ENOERROR)
    {
/*
 * don't overwrite an existing negative reply
 * Note: negative reply codes start at 4xx (positive replies are 1xx/2xx/3xx)
 */
        if ((replyPtr != (ttCharPtr)0) && (*replyPtr < '4'))
        {
            replyPtr = (ttCharPtr)&tlFtpd426[0];
        }
    }
    if (    (tm_16bit_one_bit_set(fconFlags, TM_FTPF_DATA_TRANSFER))
         && (replyPtr != (ttCharPtr)0)
       )
    {
/*
 * Send reply code string, if we had started and not ended the data
 * transfer upon this routine entry.
 */
        if (replyPtr == (ttCharPtr)&tlFtpd425[0])
/* If reply code string is 425, need to copy the command back */
        {
            ftpCmdPtr = &tlFtpdCmdArray[(int)ftpdConnEntryPtr->fconCommand];
/* Copy const char array to our connection reply buffer array */
            tm_strcpy(&ftpdConnEntryPtr->fconReplyBuf[0], replyPtr);
/* Point to new non constant reply buffer array */
            replyPtr = &ftpdConnEntryPtr->fconReplyBuf[0];
/* Copy back the command that we fail. */
            tm_bcopy(ftpCmdPtr->fdcName, &replyPtr[4], 4);
        }
        errorCode = tfFtpdSendCtrl(
                              ftpdConnEntryPtr->fconDesc[TM_FTP_CTRL_SOCKET],
                              replyPtr,
                              TM_ENOERROR );
    }
    return errorCode;
}

/*
 * tfFtpdCloseListen function description:
 * Function called with execute lock on.
 * 1. Unlock the FTPD execute lock to prevent deadlock with call back
 *    function.
 * 2. CLose the listening socket. This will cause all pending connections
 *    to be reset.
 * 3. Relock the FTPD execute lock.
 * Parameters:
 * none
 * Return:
 * none
 */
static void tfFtpdCloseListen (void)
{
     int    socketDesc;

     socketDesc = tm_context(tvFtpdListenDesc);
     tm_context(tvFtpdListenDesc) = TM_SOCKET_ERROR;
     tm_call_unlock(&tm_context(tvFtpdExecuteLockEntry));
     tfFtpdCloseSocket(socketDesc);
     tm_call_lock_wait(&tm_context(tvFtpdExecuteLockEntry));
}

/*
 * tfFtpdCloseSocket function description:
 *
 * 1. Reset all call back flags.
 * 2. Close the socket.
 *
 * Parameters:
 * Value        Meaning
 * socketDesc   socket descriptor to close.
 *
 * Return:
 * none
 */
static void tfFtpdCloseSocket (int socketDesc)
{
/* shutdown for read: half-duplex close */
    (void)shutdown(socketDesc, 0);
    (void)tfRegisterSocketCB(socketDesc, (ttUserSocketCBFuncPtr)0, 0);
    (void)tfClose(socketDesc);
}

/*
 * tfFtpdAcceptCtrl function description:
 * Accept a control connection
 * 1. Call accept (in non blocking mode) on the listening socket, to accept
 *    an incoming FTP control connection.
 * 2. If we got a valid socket descriptor:
 * 2.1 Change time to wait to minimum value, and make the socket non blocking.
 * 2.2 Check that we did not exceed the maximum number of connections allowed.
 *     If we did not:
 * 2.2.1 Allocate a FTPD connection entry and initialize it.
 * 2.2.2 Increase number of open connections.
 * 2.2.3 Append to open connection entry list
 * 2.2.4 Store socket descriptor in control socket field of FTPD connection
 *       entry, and store SOCKET_ERROR for data socket.
 * 2.2.5 Store the FTP client control address (same as FTP client data
 *       address if the FTP client does not send a port command).
 * 2.2.6 Store our default active data socket IP address (non passive) and
 *       default passive data socket IP address. They are both the same as
 *       our control socket IP address). Default active data socket port
 *       is control socket - 1.
 * 2.2.7 Disable Nagle algorithm (so that the TCP send queue gets flushed
 *       to the network if there is no data in the TCP send queue waiting
 *       to be acked.
 * 2.2.8 Handle urgent data in line.
 * 2.2.9 Initialize FTP transfer parameters with default values.
 * 2.2.10 Register for socket call back for RECV, RECV OOB, REMOTE CLOSE,
 *        and RESET.
 * 2.2.11 If there was no error in the above set the reply code string to 220.
 * 2.2.12 If we could not allocate an FTPD connection entry set reply
 *        code to 421, and set errorCode so that we close the socket
 *        at the bottom of the routine.
 * 2.3 If we exceeded the number of available connections, set reply code
 *     string to 421, and set errorCode so that we close the socket at the
 *     bottom of the routine.
 * 2.4 Send reply on control connection (either from 2.2.11, 2.2.12, or 2.3)
 * 2.5 If an error occurred earlier close the control connection: if we
 *     had allocated an FTPD connection entry successfully, use
 *     tfFtpdCloseCtrl with error code. This will send a 421 reply code
 *     string as well. If we had not allocated an FTPD connection, use
 *     tfFtpdCloseSocket() (2.2.12, or 2.3); in that case the reply code
 *     string has already been sent in 2.4.
 *
 * Parameters:
 * none
 *
 * Return:
 * none
 */
static void tfFtpdAcceptCtrl (void)
{
    ttFtpdConnEntryPtr               ftpdConnEntryPtr;
    struct sockaddr_storage TM_FAR * sockAddressPtr;
    ttCharPtr                        replyPtr;
    struct sockaddr_storage          sockAddress;
    int                              acceptDesc;
    int                              sockAddrSize;
    int                              tempInt;
    int                              errorCode;
    tt16Bit                          ipPort;
    ttSockAddrPtrUnion               tempSockAddr;

    errorCode = TM_SOCKET_ERROR;
    ftpdConnEntryPtr = (ttFtpdConnEntryPtr)0;
    sockAddrSize = sizeof(struct sockaddr_storage);
    tempSockAddr.sockNgPtr = &sockAddress;
    acceptDesc = accept( tm_context(tvFtpdListenDesc),
                         tempSockAddr.sockPtr,
                         &sockAddrSize);
    if (acceptDesc != TM_SOCKET_ERROR)
    {
        replyPtr = (ttCharPtr)0;
/* Change Time Wait to minimum value */
        tempInt = 0;
        (void)setsockopt( acceptDesc,
                          IPPROTO_TCP,
                          TM_TCP_2MSLTIME,
                          (const char TM_FAR *)&tempInt,
                          sizeof(int) );
/* Connection socket is in non blocking state */
        (void)tfBlockingState(acceptDesc, TM_BLOCKING_OFF);
        if (    tm_context(tvFtpdMaxConnections) == 0
            ||  (   tm_context(tvFtpdConQHead).
                                servQHeadCount[TM_FTPD_CONN_LIST]
                  < tm_context(tvFtpdMaxConnections) ) )
        {
            ftpdConnEntryPtr = (ttFtpdConnEntryPtr)
                                  tm_get_raw_buffer(sizeof(ttFtpdConnEntry));
            if ( ftpdConnEntryPtr != (ttFtpdConnEntryPtr)0 )
            {
                tm_bzero(ftpdConnEntryPtr, sizeof(ttFtpdConnEntry));
/* Insert in the open connection entries queue */
                tfServerAppendToQueue( &(tm_context(tvFtpdConQHead)),
                                       (ttServerQueuePtr)ftpdConnEntryPtr,
                                       TM_FTPD_CONN_LIST );
                tfFtpdTimerAdd(ftpdConnEntryPtr);
                ftpdConnEntryPtr->fconDesc[TM_FTP_CTRL_SOCKET] = acceptDesc;
                ftpdConnEntryPtr->fconDesc[TM_FTP_DATA_SOCKET] =
                                                             TM_SOCKET_ERROR;
/*
 * Store default peer data socket address (same as peer control address)
 * in fconClientCtrlAddress.
 */
                tm_bcopy( &sockAddress,
                          &ftpdConnEntryPtr->fconClientCtrlAddress,
                          sizeof(ftpdConnEntryPtr->fconClientCtrlAddress) );
/*
 * Save our default (active) data socket address in fconSrvrDataAddress.
 * (It is the same as our control connection source IP address.)
 */
                sockAddressPtr = &ftpdConnEntryPtr->fconSrvrDataAddress;
                tempSockAddr.sockNgPtr = sockAddressPtr;
                errorCode = getsockname(
                                     acceptDesc,
                                     tempSockAddr.sockPtr,
                                     &sockAddrSize );
                if (errorCode == TM_ENOERROR)
                {
/* Our default (active) data Ip port is Control port - 1 */
                    tm_ntohs(sockAddressPtr->ss_port, ipPort);
                    ipPort--;
                    tm_htons(ipPort, sockAddressPtr->ss_port);
/*
 * Also copy our control connection source IP address in our passive data
 * address structure (will be used if the FTP client sends an FTP
 * pasv command).
 */
                    tm_bcopy(
                        sockAddressPtr,
                        &ftpdConnEntryPtr->fconSrvrPasvDataAddress,
                        sizeof(ftpdConnEntryPtr->fconSrvrPasvDataAddress) );

/* Disable Nagle algorithm (TCP_NODELAY on) */
                    tempInt = 1; /* ON */
                    errorCode = setsockopt( acceptDesc,
                                            IPPROTO_TCP,
                                            TCP_NODELAY,
                                            (const char TM_FAR *)&tempInt,
                                            sizeof(int) );
/* Handle Urgent Data inline */
                    if (errorCode == TM_ENOERROR)
                    {
/* SO_OOBINLINE on */
                        errorCode = setsockopt( acceptDesc,
                                                SOL_SOCKET,
                                                SO_OOBINLINE,
                                                (const char TM_FAR *)&tempInt,
                                                sizeof(int) );

                        if (errorCode == TM_ENOERROR)
                        {
/* Set up default type, and structure for file data transfer */
                            tfFtpdDataDefault(ftpdConnEntryPtr);
                            errorCode = tfRegisterSocketCBParam(
                                                        acceptDesc,
                                                        tfFtpdSignal,
                                                        ftpdConnEntryPtr,
                                                          TM_CB_RECV
                                                        | TM_CB_RECV_OOB
                                                        | TM_CB_REMOTE_CLOSE
                                                        | TM_CB_RESET
                                                        | TM_CB_SOCKET_ERROR);
                            if (errorCode == TM_ENOERROR)
                            {
/* Success reply */
                                replyPtr = (ttCharPtr)&tlFtpd220[0];
                            }
                        }
                    }
                }
            }
            else
            {
/*
 * We could not allocate an ftpConnEntry structure, Send a 421, and close
 * the connection with tfFtpdCloseSocket().
 */
                replyPtr = (ttCharPtr)&tlFtpd421[0];
                errorCode = TM_ENOBUFS;
            }
        }
        else
        {
/*
 * We reached the maximum number of connections. Send a 421, and close the
 * connection with tfFtpdCloseSocket().
 */
            replyPtr = (ttCharPtr)&tlFtpd421[0];
            errorCode = TM_EWOULDBLOCK;
        }
        errorCode = tfFtpdSendCtrl( acceptDesc,
                                    replyPtr,
                                    errorCode );
        if (errorCode != TM_ENOERROR)
        {
            if (ftpdConnEntryPtr != (ttFtpdConnEntryPtr)0)
            {
/* tfFtpdCloseCtrl will send a reply code string */
                tfFtpdCloseCtrl(ftpdConnEntryPtr, errorCode);
            }
            else
            {
/*
 * We either could not allocate an ftpConnEntry structure, or we reached
 * the maximum number of connections. We already sent a 421.
 */
                tfFtpdCloseSocket(acceptDesc);
            }
        }
    }
}

/*
 * tfFtpdDataDefault function description:
 * Initialize the FTPD connection entry with default values for
 * transfer type ascii and file structure stream.
 * For the transfer type, add special flag if file system use CR, LF for end
 * of line, so that we can use speedier binary type file transfer algorithm.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 *
 * Return:
 * none
 */
static void tfFtpdDataDefault(ttFtpdConnEntryPtr ftpdConnEntryPtr)
{
    int txType;

/* transfer mode: stream, unused since MODE_STREAM is only mode supported */
/*    ftpdConnEntryPtr->fconTxMode = TM_MODE_STREAM; */
/*
 * transfer type: ascii. Add special flag if file system use CR, LF for end
 * of line, so that we can use speedier binary type file transfer algorithm.
 */
    txType = TM_TYPE_ASCII | tm_context(tvFtpdFsTypeFlag);
    ftpdConnEntryPtr->fconTxType = (tt8Bit)txType;
/* transfer structure: file */
    ftpdConnEntryPtr->fconTxStru = TM_STRU_STREAM;
/*
 * transfer form: non print. Unused since NONPRINT is the only from supported
 */
/*    ftpdConnEntryPtr->fconTxForm = TM_FORM_NONPRINT; */
    return;
}

/*
 * tfFtpdRecvCtrl function description.
 * Recv characters on the control connection. Per RFC 1123 recommendations,
 * this routine handles telnet default options (and replies DONT/WONT to any
 * telnet options), telnet input NVT ascii EOL <CR,LF>, TCP and telnet
 * urgent data (with or without telnet IP), and multiple telnet SYNCH's
 * (to end urgent data), handles not receiving a full FTP command in
 * one recv call, handles receiving more than one FTP command in a recv call.
 * When a full FTP command has been received (the telnet EOL (i.e <CR,LF>
 * in telnet ascii mode has been received in our buffer), the
 * FTP command is scanned, and executed if appropriate.
 * We use 2 input buffers, one where we recv the raw telnet network data
 * input, and one FTP input buffer where we store the data after applying
 * the NVT telnet transformation to the raw telnet network data.
 * Details of the algorithm:
 * 0. If the input telnet buffer is full, and we do not have at least
 *    one buffered FTP command, we return an error
 *    to let the caller reset the connection, otherwise copy previous
 *    telnet state, and working buffer indexes, to local variables.
 * 1. In a do while loop until either we find a full FTP command, or there
 *    is nothing to read:
 * 1.a If we are not in telnet urgent mode, we check if TCP is in urgent
 *     mode, in which case we transition to telnet urgent mode, and reset
 *     the current FTP command (or portion thereof), and current telnet
 *     input buffer.
 * 1.b If there is a full buffered FTP command in the FTP input buffer
 *     (either from the bottom of the loop, or from a previous unprocessed
 *     FTP command that we had received while executing a data transfer),
 *     then we go ahead and execute the FTP command, and break out of the
 *     loop.
 * 1.c Otherwise we recv the input data into a telnet input buffer,
 *     keeping track of beginning and end of the telnet input buffer.
 * 1.d In an inner loop (while telnet input buffer is non empty):
 * 1.d.1 we process telnet characters, implementing a subset of the telnet
 *       protocol. We just reply DONT to WILL and WONT client telnet options,
 *       and reply WONT to DO and DONT. We do our internal NVT transformation
 *       transforming "\r\n" to "\0" (our end of line character), and
 *       "\r\0" to "\r" as specified in the telnet protocol. We check for
 *       the (IAC, IP) telnet interrupt process sequence, in which case
 *       we transition to urgent mode (we are in urgent mode if either the
 *       underlying TCP is in urgent mode of we see a telnet interrupt process
 *       (IAC, IP)). In urgent mode, we skip all characters until we reach a
 *       Telnet SYNCH (IAC, DM) which marks the end of telnet urgent data.
 * 1.d.2 Copy telnet data character (if any) in an FTP input buffer.
 * 1.d.3 When FTP input buffer contains a full FTP command ("\r\n" has
 *       been seen in the telnet input buffer), we make sure we save
 *       unprocessed telnet characters to the beginning of the telnet buffer,
 *       in case we had recv beyond one FTP command. We break out of the
 *       inner loop.
 * 1.e   Out of the inner loop:
 *       If the input telnet buffer is empty, we reset the telnet indexes to
 *       the beginning of the telnet buffer. We save all local indexes and
 *       variable back into the ftpd connection block. Go back to the outer
 *       loop (step 1.a above) if the previous recv did not fail.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 *
 * Return:
 * errorCode on control connection
 */
static int tfFtpdRecvCtrl (ttFtpdConnEntryPtr ftpdConnEntryPtr)
{
    tt8BitPtr   telInputBufferPtr;
    tt8BitPtr   ftpdCmdBufferPtr;
    tt8BitPtr   replyBufferPtr;
    tt8Bit      telCommandChar;
    int         socketDesc;
    int         recvBytes;
    int         ftpCmdLineIndex;
    int         telBufReadIndex;
    int         telBufProcessIndex;
    int         unprocessedBytes;
    int         i;
    int         retCode;
    int         errorCode;
    tt16Bit     conFlags;
    tt8Bit      telState;
    tt8Bit      telChar;
    tt8Bit      processCurrentChar;

/* End of FTP command has been reached ? */
    conFlags = ftpdConnEntryPtr->fconFlags;
/* Telnet read index in telnet input buffer */
    telBufReadIndex = ftpdConnEntryPtr->fconTelBufReadIndex;
    if (    (telBufReadIndex == TM_CMD_LINE_SIZE)
         && (tm_16bit_bits_not_set(conFlags, TM_FTPF_COMMAND_BUFFERED)) )
    {
/*
 * If input telnet buffer is full, and we have not been able to buffer
 * a single full FTP command, something wrong occurred on the connection
 * We will let the caller close the connection.
 */
        errorCode = TM_EWOULDBLOCK;
    }
    else
    {
        errorCode = TM_ENOERROR;
/* Ftp command line */
        ftpdCmdBufferPtr = &ftpdConnEntryPtr->fconCmdLine[0];
/* Telnet input buffer */
        telInputBufferPtr = &ftpdConnEntryPtr->fconTelInputBuf[0];
/* Ftp command line index */
        ftpCmdLineIndex = ftpdConnEntryPtr->fconCmdLineIndex;
/* Telnet processing index in telnet input buffer */
        telBufProcessIndex = ftpdConnEntryPtr->fconTelBufProcessIndex;
/* Remember previous Telnet state */
        telState = ftpdConnEntryPtr->fconTelState;
        socketDesc = ftpdConnEntryPtr->fconDesc[TM_FTP_CTRL_SOCKET];
        do
        {
/*
 * If we are not in telnet urgent state, check if we have urgent data in
 * the stream, in which case we need to flush our FTP input buffer,
 * telnet input buffer, and enter the telnet urgent state.
 */
            if ( tm_8bit_bits_not_set(telState, TM_TEL_URGENT_STATE) )
            {
/* check if we have received urgent data */
                retCode = tfGetOobDataOffset(socketDesc);
                if (retCode != TM_SOCKET_ERROR)
                {
                    telState = (tt8Bit)(
                                         (tt8Bit)(telState & TM_TEL_IAC_STATE)
                                       | TM_TEL_URGENT_STATE );
/* Reset current FTP command if any */
                    ftpCmdLineIndex = 0;
                    tm_16bit_clr_bit(conFlags, TM_FTPF_COMMAND_BUFFERED);
/* Reset Telnet input buffer also */
                    telBufProcessIndex = 0;
                    telBufReadIndex = 0;
                }
            }
            if ( tm_16bit_one_bit_set(conFlags, TM_FTPF_COMMAND_BUFFERED) )
/*
 * Process current FTP command. (We could have an unprocessed command from
 * a previous tfFtpdRecvCtrl(), if we read ahead while transfering data,
 * and the command was not ABORT, QUIT, or STAT.)
 */
            {
/* Flush FTP input command (since it is being executed in tfFtpdScanCmd) */
                tm_16bit_clr_bit(ftpdConnEntryPtr->fconFlags,
                                 TM_FTPF_COMMAND_BUFFERED);
/*
 * We could inline tfFtpdScanCmd() but that would make the code
 * really hard to read
 */
                errorCode = tfFtpdScanCmd(ftpdConnEntryPtr);
/* Just execute one command to give everybody else a chance at the CPU */
                break;
            }
/*
 * Don't need to check for 0 return (EOF) since we have registered for
 * remote close call back.
 */
            recvBytes = recv( socketDesc,
                              (ttCharPtr)&telInputBufferPtr[telBufReadIndex],
                              TM_CMD_LINE_SIZE - telBufReadIndex,
                              MSG_DONTWAIT );
            if (recvBytes > 0)
            {
/* Update read telnet index */
                telBufReadIndex += recvBytes;
            }
            while (telBufProcessIndex < telBufReadIndex)
            {
/* get a new character */
                telChar = telInputBufferPtr[telBufProcessIndex];
                telBufProcessIndex++;
/*
 * Assume current character is special telnet character. If it is not,
 * processCurrentChar will be set below.
 */
                processCurrentChar = TM_8BIT_ZERO;
                telCommandChar = (tt8Bit)0;
                switch (telState)
                {
                    case TM_TEL_IAC_STATE:
/*
 * We have seen an IAC character previously:
 * Process the Telnet command. Just implement a subset of the telnet
 * commands.
 */
                        switch (telChar)
                        {
                            case TM_TC_WILL:
                            case TM_TC_WONT:
/* IAC, WILL or IAC, WONT makes us transit to WOPT state */
                                telState = TM_TEL_WOPT_STATE;
                                break;
                            case TM_TC_DO:
                            case TM_TC_DONT:
/* IAC, DO or IAC, DONT makes us transit to DOPT state */
                                telState = TM_TEL_DOPT_STATE;
                                break;
                            case TM_TC_IP:
/* Interrupt process */
                                telState = TM_TEL_URGENT_STATE;
/* Flush FTP input buffer */
                                ftpCmdLineIndex = 0;
                                break;
                            case TM_TC_IAC:
/* IAC, IAC means escaped IAC character, process the second IAC */
                                processCurrentChar = TM_8BIT_YES;
/* fall through to default */
                            default:
/* Ignore (skip) all other commands. Go back to data state */
                                telState = TM_TEL_DATA_STATE;
                                break;
                        }
                        break;
                    case TM_TEL_WOPT_STATE:
/* IAC DONT telnet command. */
                        telCommandChar = TM_TC_DONT;
                        break;
                    case TM_TEL_DOPT_STATE:
/* IAC WONT telnet command. */
                        telCommandChar = TM_TC_WONT;
                        break;
                    case TM_TEL_URGENT_STATE:
                        if (telChar == TM_TC_IAC)
                        {
                            telState = TM_TEL_URGENT_IAC_STATE;
                        }
/*
 * BUG ID 2919:
 * Microsoft FTP client does not add TC_IAC before TC_DM, or does not send
 * TC_DM, after having sent TC_IAC+TC_IP.
 */
                        else
                        {
                            telState = TM_TEL_DATA_STATE;
                            if (telChar != TM_TC_DM)
                            {
                                processCurrentChar = TM_8BIT_YES;
                            }
/*
 * else Telnet SYNCH, signal end of urgent data, we will check if there is more
 * urgent data ahead at the top of the outer loop.
 */
                        }
/* End BUG ID 2919 */
                        break;
                    case TM_TEL_URGENT_IAC_STATE:
/* In urgent state, after an IAC, Look for a DM to end urgent data */
                        if (telChar == TM_TC_DM)
                        {
/*
 * Telnet SYNCH, signal end of urgent data, we will check if there is more
 * urgent data ahead at the top of the outer loop.
 */
                            telState = TM_TEL_DATA_STATE;
                        }
                        else
                        {
                            telState = TM_TEL_URGENT_STATE;
                        }
                        break;
                    case TM_TEL_CR_STATE:
                        processCurrentChar = TM_8BIT_YES;
                        telState = TM_TEL_DATA_STATE;
/*
 * "\r\n" is mapped to "\0". Mark end of FTP command.
 * "\r\0" is legal with the telnet protocol in telnet ascii mode, but the FTP
 * protocol specifies that "\r\n" marks the end of an FTP command, so
 * we should never see "\r\0" with FTP. We will just map it as '\r'.
 * "\r" followed by any other character is illegal in telnet ascii mode, but
 * we will just copy it.
 */
                        if (telChar == TM_NVT_LF)
                        {
/* '\r\n' maps to '\0' (our end of line) */
                            telChar = '\0';
                            conFlags =  (tt16Bit)
                                               (   conFlags
                                                 | TM_FTPF_COMMAND_BUFFERED);
                        }
                        else
                        {
/* This else part should never happen with FTP */
                            if (telChar == '\0')
                            {
/* '\r\0' maps to '\r'. Should never happen on an FTP connection */
                                telChar = TM_NVT_CR;
                            }
                            else
                            {
/*
 * '\r' followed by any other character does not require any mapping.
 * Just process the '\r', and process the current character
 * later.
 */
                                telChar = TM_NVT_CR;
                                telBufProcessIndex--;
                            }
                        }
                        break;
                    case TM_TEL_DATA_STATE:
                        if (telChar == TM_NVT_CR)
                        {
/* First '\r', just remember it (transition to CR_STATE state) */
                            telState = TM_TEL_CR_STATE;
                        }
                        else
                        {
                            if (telChar == TM_TC_IAC)
                            {
/* Current (IAC) character makes us transition to IAC state */
                                telState = TM_TEL_IAC_STATE;
                            }
                            else
                            {
/* Character is neither '\r' or IAC and we are in data state, process it */
                                processCurrentChar = TM_8BIT_YES;
                            }
                        }
                        break;
                    default:
                        break;
                }
                if (telCommandChar != (tt8Bit)0)
                {
/* Process DO, DONT, WILL, WONT commands */
/* Get the Telnet command following the IAC */
                    telState = TM_TEL_DATA_STATE;
                    replyBufferPtr = (tt8BitPtr)
                                     &ftpdConnEntryPtr->fconReplyBuf[0];
                    replyBufferPtr[0] = TM_TC_IAC;
                    replyBufferPtr[1] = telCommandChar;
                    replyBufferPtr[2] = telChar;
                    replyBufferPtr[3] = '\0';
                    errorCode = tfFtpdSendCtrl( socketDesc,
                                                (ttCharPtr)replyBufferPtr,
                                                TM_ENOERROR );
                    if (errorCode != TM_ENOERROR)
                    {
/* Fatal error for the connection, out of the loop */
                        break;
                    }
                }
                if (processCurrentChar != TM_8BIT_ZERO)
                {
/* Process current character. */
                    if (ftpCmdLineIndex < TM_CMD_LINE_SIZE)
                    {
                        ftpdCmdBufferPtr[ftpCmdLineIndex] = telChar;
                        ftpCmdLineIndex++;
                    }
                }
                if ( tm_16bit_one_bit_set( conFlags,
                                           TM_FTPF_COMMAND_BUFFERED ) )
                {
                    if (ftpCmdLineIndex == TM_CMD_LINE_SIZE)
                    {
/*
 * If we overflowed our input buffer, make sure the command is null
 * terminated.
 */
                        ftpdCmdBufferPtr[TM_CMD_LINE_SIZE - 1] = '\0';
                    }
                    unprocessedBytes = telBufReadIndex - telBufProcessIndex;
                    if (unprocessedBytes != 0)
                    {
/* Non empty telnet buffer, copy unprocessed bytes to beginning */
                        for (i = 0; i < unprocessedBytes; i++)
                        {
/* Avoid overlapping bcopy by copying up one byte at a time */
                            telInputBufferPtr[i] =  
                                telInputBufferPtr[telBufProcessIndex + i];
                        }
                        telBufProcessIndex = 0;
                        telBufReadIndex = unprocessedBytes;
                    }
/* Reset FTP input buffer */
                    ftpCmdLineIndex = 0;
                    break;
                }
            }
            if (telBufProcessIndex == telBufReadIndex)
/* If telnet input buffer is empty, reset to beginning */
            {
                telBufProcessIndex = 0;
                telBufReadIndex = 0;
            }
/* save current state */
            ftpdConnEntryPtr->fconTelState = telState;
/* Save current position in command line */
            ftpdConnEntryPtr->fconCmdLineIndex = ftpCmdLineIndex;
/* Save FTP command flags */
            ftpdConnEntryPtr->fconFlags = conFlags;
/* Save current Telnet indexes before executing the command */
            ftpdConnEntryPtr->fconTelBufReadIndex = telBufReadIndex;
            ftpdConnEntryPtr->fconTelBufProcessIndex = telBufProcessIndex;
/*
 * Try and read more data from the network.
 */
        }
        while ( (recvBytes > 0) );
    }
    return errorCode;
}

/*
 * tfFtpdScanCmd function description:
 * Scan the FTP command coming from the client, convert to command
 * to execute, then map to corresponding action function, check for
 * correct FTP syntax, then if syntax is correct and call allowed,
 * call the action function which returns the reply code string, and then
 * send reply code string to FTP client.
 * Detail implementation:
 * 1. Convert first 3 or 4 characters (up to space) to upper case
 * 2. Save the character in 4th position.
 * 3. If command is 3 characters long, temporarily stores a space
 *    character at the 4th position.
 * 4. Look up the Ftp command array for a command match for the
 *    first 4 characters. If look up is succesful we get back an FTP command
 *    array entry, otherwise we send a 500 reply code string at the bottom
 *    of this routine.
 * 5. Restore the character we saved in step 2.
 * 6. If data transfer is taking place, only allow ABORT, STAT, and QUIT.
 * 7. If we did get a Rename From, check that we did get a rename to command,
 *    or an abort. If we received any other command send a 503 reply code
 *    string at the bottom of this routine.
 * 8. If login is required for the command (or login required for the command
 *    with an argument) (we check the ftp command array entry flags field),
 *    and the user is not logged in, send a 530 reply code string at the
 *    bottom of this routine.
 * 9. If the command is a file system command (we check the file system flag
 *    field in the FTP command array entry), and if the file system
 *    does not support the command (as specified in tfNgFtpdUserStart),
 *    send a 502 reply code string at the bottom of this routine.
 * 10. Check argument list, skipping over any space (if any) after the
 *     3 to 4 characters FTP command. Verify syntax, by checking the FTP
 *     argument type field in the FTP command array against the argument
 *     itself. If there is any syntax error, send a 501 reply code string
 *     at the bottom of this routine.
 * 11. Convert arguments to MODE, TYPE, or STRU FTP command to upper case.
 *     (They all have the TM_UPPER flag bit set in the FTP command array
 *     entry flags field.)
 * 12. Call the action function corresponding to the FTP command array
 *     entry.
 * 13. Get the error code on the control connectino (if any) from the
 *     command execution. The action function itself returns the reply
 *     code (if any) to be sent at the bottom of this routine.
 * 14. For some reply code strings, copy back the command that we OKAY, or
 *     fail in the reply string.
 * 15. Send the reply code string (if any)
 * 16. If we had to postpone processing an FTP control connection command,
 *     queue the FTP connection entry it into the run queue.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 *
 * Return:
 * errorCode on control connection
 */
static int tfFtpdScanCmd (ttFtpdConnEntryPtr ftpdConnEntryPtr)
{
    ttFtpdCmdEntryPtr  ftpCmdPtr;
    ttCharPtr          ftpdCmdLinePtr;
    ttCharPtr          ftpdArgumentPtr;
    ttCharPtr          replyPtr;
    union tuftpCommand ftpCommand;
    int                ftpCmdLength;
    int                index;
    int                errorCode;
    int                ctrlSocketDesc;
    int                tempInt;
    tt16Bit            conFlags;
    tt16Bit            temp16Flags;
    char               ftpdArgumentChar;
    tt8Bit             queueFtpInputToRunQueue;
    tt8Bit             argumentPresent;

    ftpdCmdLinePtr = (ttCharPtr)&ftpdConnEntryPtr->fconCmdLine[0];
    queueFtpInputToRunQueue = TM_8BIT_YES;
    errorCode = TM_ENOERROR;
    replyPtr = (ttCharPtr)&tlFtpd550[0];
    ftpCmdPtr = (ttFtpdCmdEntryPtr)0;
    ctrlSocketDesc = ftpdConnEntryPtr->fconDesc[TM_FTP_CTRL_SOCKET];
    ftpCmdLength = tfStringToUpper(ftpdCmdLinePtr, TM_FTP_MAX_CMD_LENGTH);
    if (    (ftpCmdLength == TM_FTP_MAX_CMD_LENGTH)
         || (ftpCmdLength == TM_FTP_MAX_CMD_LENGTH - 1) )
    {
/* Point to FTP command argument */
        ftpdArgumentPtr = &ftpdCmdLinePtr[ftpCmdLength];
        ftpdArgumentChar = *ftpdArgumentPtr;
        if ( ftpCmdLength == (TM_FTP_MAX_CMD_LENGTH - 1) )
        {
/* Space terminate the command (for tlFtpdCmdArray comparison below) */
            *ftpdArgumentPtr = TM_NVT_SPACE;
        }
        tm_bcopy(ftpdCmdLinePtr, ftpCommand.ftpuName, 4);
        for ( index = 0; index <= TM_NOOP; index++ )
        {
            ftpCmdPtr = &tlFtpdCmdArray[index];
            if (ftpCommand.ftpu32Bit == ftpCmdPtr->fdc32Bit)
            {
                break;
            }
        }
/* Restore character */
        *ftpdArgumentPtr = ftpdArgumentChar;
        if (index <= TM_NOOP)
        {
            conFlags = ftpdConnEntryPtr->fconFlags;
            if ( tm_16bit_one_bit_set(conFlags, TM_FTPF_DATA_TRANSFER) )
            {
/* If data transfer is taking place, process only ABORT, STAT, and QUIT */
                if (    (index != TM_ABOR)
                     && (index != TM_STAT)
                     && (index != TM_QUIT) )
                {
/*
 * If command is neither ABORT, STAT, no QUIT, postpone processing of the
 * current command until we finish data transfer.
 */
                    ftpdConnEntryPtr->fconFlags =
                            (tt16Bit)(   ftpdConnEntryPtr->fconFlags
                                       | TM_FTPF_COMMAND_BUFFERED );
/*
 * Postpone requeueing this command to the run queue, until data transfer
 * finishes.
 */
                    queueFtpInputToRunQueue = TM_8BIT_ZERO;
                    replyPtr = (ttCharPtr)0;
                    goto exitScan;
                }
            }
/*
 * If we did get a Rename From, we need a rename to command. We will
 * allow only ABOR, or QUIT besides RNTO command.
 */
            if (     (tm_16bit_one_bit_set(conFlags, TM_FTPF_RNFR_RECEIVED))
                 &&  (index != TM_RNTO)
                 &&  (index != TM_ABOR)
                 &&  (index != TM_QUIT) )
            {
/*
 * Reset the RNFR received flag, so that the user has to resend the RNFR
 * if he wants to rename the file.
 */
                temp16Flags = conFlags;
                tm_16bit_clr_bit(temp16Flags, TM_FTPF_RNFR_RECEIVED);
                ftpdConnEntryPtr->fconFlags = temp16Flags;
                replyPtr = (ttCharPtr)&tlFtpd503[0];
                goto exitScan;
            }
/*
 * If we did get a user name, we need a passord command. We will allow
 * only ABOR and QUIT besides the password command
 */
            if (     (tm_16bit_one_bit_set(conFlags, TM_FTPF_USER_NAME_IN))
                 &&  (index != TM_PASS)
                 &&  (index != TM_ABOR)
                 &&  (index != TM_QUIT) )
            {
/*
 * Reset the user name in flag, so that the user has to resend the USER
 * command to login.
 */
                temp16Flags = conFlags;
                tm_16bit_clr_bit(temp16Flags, TM_FTPF_USER_NAME_IN);
                ftpdConnEntryPtr->fconFlags = temp16Flags;
                replyPtr = (ttCharPtr)&tlFtpd503[0];
                goto exitScan;
            }
/* Assume no argument */
            argumentPresent = TM_8BIT_ZERO;
            if (ftpdArgumentChar != '\0')
/* Argument present */
            {
/* Skip over extra space character */
                while (*ftpdArgumentPtr == ' ')
                {
                    ftpdArgumentPtr++;
                }
                if (*ftpdArgumentPtr != '\0')
                {
                    argumentPresent = TM_8BIT_YES;
                }
            }
            tempInt = (int)ftpCmdPtr->fdcArgFlags;
/*
 * If command is not supported, return 502, except ALLO that returns 202.
 */
            if ( (tempInt & TM_NOSUP) != 0 )
            {
                if (index == TM_ALLO)
                {
                    replyPtr = (ttCharPtr)&tlFtpd202[0];
                }
                else
                {
                    replyPtr = (ttCharPtr)&tlFtpd502[0];
                }
                goto exitScan;
            }
/*
 * If login is required for the command (or login required for the command
 * with an argument), and the user is not logged in, reply with 530 error
 * message.
 */
            if (    (    ( (tempInt & TM_LOGIN) != 0 )
                      || (    ( (tempInt & TM_LOGINARG) != 0 )
                           && ( argumentPresent != TM_8BIT_ZERO ) ) )
                 && ( tm_16bit_bits_not_set( conFlags,
                                             TM_FTPF_USER_LOGGED_IN ) ) )
            {
                replyPtr = (ttCharPtr)&tlFtpd530[0];
                goto exitScan;
            }
#ifdef TM_USE_FTPD_SSL
            if ( tm_context(tvFtpdSslCtrlPermissions) == TM_FTPD_SSL_REQUIRED )
/* SSL Required on the CTRL connection */
            {
                if (       (tempInt & TM_AUTH_CTRL_REQUIRED)
                        && (tm_8bit_bits_not_set(
                                 ftpdConnEntryPtr->fconSslFlags,
                                 TM_FTPD_SSL_SESSION_ACTIVE)) )
                {
                    replyPtr = (ttCharPtr)&tlFtpd503[0];
                    goto exitScan;
                }
            }

            if ( tm_context(tvFtpdSslDataPermissions) == TM_FTPD_SSL_REQUIRED )
            {
                if (       (tempInt & TM_PROT_DATA_REQUIRED)
                        && (tm_8bit_bits_not_set(
                                  ftpdConnEntryPtr->fconSslFlags,
                                  TM_FTPD_SSL_ENCRYPT_DATA)) )
/* This command requires PROT first (and hasn't got it) */
                {
                    replyPtr = (ttCharPtr)&tlFtpd521[0];
                    goto exitScan;
                }
            }
#endif /* TM_USE_FTPD_SSL */
/*
 * If the command is a file system command, check whether the file system
 * supports/allows the command.
 */
            tempInt = ((int)ftpCmdPtr->fdcFsFlags) & 0xFFFF;
            if (    ( tempInt != 0 )
                 && ( (tempInt & tm_context(tvFtpdFsFlags)) == 0 ) )
            {
                replyPtr = (ttCharPtr)&tlFtpd502[0];
                goto exitScan;
            }
/*
 * Verify syntax, for string argument we need an argument, for no argument
 * we need end of string.
 */
            tempInt = ftpCmdPtr->fdcArgType;
            if (    (    (tempInt == TM_STRING)
                      && (argumentPresent == TM_8BIT_ZERO) )
                 || (    (tempInt == TM_NOARG)
                      && (argumentPresent != TM_8BIT_ZERO) ) )
            {
/* Syntax error in arguments */
                replyPtr = (ttCharPtr)&tlFtpd501[0];
                goto exitScan;
            }
            tempInt = (int)ftpCmdPtr->fdcArgFlags;
            if ((tempInt & TM_UPPER) != 0)
            {
/* Mode, type, stru arguments */
                (void)tfStringToUpper(ftpdArgumentPtr, TM_CMD_LINE_SIZE);
            }
            ftpdConnEntryPtr->fconCommand = (tt16Bit)index;
            replyPtr = (*(ftpCmdPtr->fdcFuncPtr))( ftpdConnEntryPtr,
                                                   ftpdArgumentPtr );
/* Ftp command function error code */
            errorCode = ftpdConnEntryPtr->fconErrorCode;
        }
    }
exitScan:
    if (ftpCmdPtr != (ttFtpdCmdEntryPtr)0)
    {
        if (    (replyPtr == (ttCharPtr)&tlFtpd200[0])
             || (replyPtr == (ttCharPtr)&tlFtpd202[0])
             || (replyPtr == (ttCharPtr)&tlFtpd250[0])
             || (replyPtr == (ttCharPtr)&tlFtpd425[0])
             || (replyPtr == (ttCharPtr)&tlFtpd502[0])
             || (replyPtr == (ttCharPtr)&tlFtpd503[0])
             || (replyPtr == (ttCharPtr)&tlFtpd504[0])
             || (replyPtr == (ttCharPtr)&tlFtpd550[0]) )
        {
/* Copy const char array to our connection reply buffer array */
            tm_strcpy(&ftpdConnEntryPtr->fconReplyBuf[0], replyPtr);
/* Point to new non constant reply buffer array */
            replyPtr = &ftpdConnEntryPtr->fconReplyBuf[0];
/* Copy back the command that we OKAY, or fail. */
            tm_bcopy(ftpCmdPtr->fdcName, &replyPtr[4], 4);
        }
#ifdef TM_USE_FTPD_SSL
        else if ((replyPtr == (ttCharPtr)&tlFtpd234[0]))
        {
/* Copy const char array to our connection reply buffer array */
            tm_strcpy(&ftpdConnEntryPtr->fconReplyBuf[0], replyPtr);
/* Point to new non constant reply buffer array */
            replyPtr = &ftpdConnEntryPtr->fconReplyBuf[0];
/* Copy back the command that we OKAY, or fail. */
            tm_bcopy(ftpdConnEntryPtr->fconTempBuf, &replyPtr[9], 3);
        }
        else if ((replyPtr == (ttCharPtr)&tlFtpd504[0])
                ||(replyPtr == (ttCharPtr)&tlFtpd334[0]))
        {
/* Copy const char array to our connection reply buffer array */
            tm_strcpy(&ftpdConnEntryPtr->fconReplyBuf[0], replyPtr);
/* Point to new non constant reply buffer array */
            replyPtr = &ftpdConnEntryPtr->fconReplyBuf[0];
/* Copy back the command that we OKAY, or fail. */
            tm_bcopy(ftpdConnEntryPtr->fconTempBuf, &replyPtr[4], 3);
        }
#endif /* TM_USE_FTPD_SSL */
    }
/*
 * NOTE: replyPtr could be null for a Data transfer command in passive mode
 */
    errorCode = tfFtpdSendCtrl(ctrlSocketDesc, replyPtr, errorCode);
#ifdef TM_USE_FTPD_SSL
    if(tm_16bit_one_bit_set(ftpdConnEntryPtr->fconSslFlags,
                TM_FTPD_SSL_CHANGE_CTRL))
    {
/* Remove the "Change SSL CTRL" marker from our flags */
        ftpdConnEntryPtr->fconSslFlags &= ~TM_FTPD_SSL_CHANGE_CTRL;

        if (tm_16bit_all_bits_set(ftpdConnEntryPtr->fconSslFlags,
                                  TM_FTPD_SSL_CTRL_ENCRYPTED))
        {
            tfSslConnectUserClose(
                    ftpdConnEntryPtr->fconDesc[TM_FTP_CTRL_SOCKET]);

        }
        else
        {
/* 
 * An error code from tfSslServerUserCreate will be propogated back forcing a 
 * 421 error to be sent to the client and closing the control connection. 
 */
            errorCode = tfSslServerUserCreate(
                    ftpdConnEntryPtr->fconDesc[TM_FTP_CTRL_SOCKET]);
        }

        if (errorCode == TM_ENOERROR)
        {
            ftpdConnEntryPtr->fconSslFlags ^= TM_FTPD_SSL_CTRL_ENCRYPTED;
        }
    }
/* If we've just sent the feature list, free the buffer we created to do so */    
    if(ftpdConnEntryPtr->fconFeatBuffer != (ttCharPtr)0 )
    {
        tm_free_raw_buffer( ftpdConnEntryPtr->fconFeatBuffer );
        ftpdConnEntryPtr->fconFeatBuffer = (ttCharPtr)0;
    }
#endif /* TM_USE_FTPD_SSL */
    if (    (queueFtpInputToRunQueue != TM_8BIT_ZERO)
         && (errorCode == TM_ENOERROR) )
    {
        tfFtpdQueueUnScannedCtrlInput(ftpdConnEntryPtr);
    }
    return errorCode;
}

/*
 * tfFtpdQueueUnScannedCtrlInput function description:
 * This function is called after each FTP command execution, and at the
 * end of a data transfer, to check whether there is any outstanding
 * FTP control input that needs to be queued at the end of the FTPD run
 * Q, to give other connections a fair share of the CPU.
 * Detailed algorithm:
 * If our connection is not in the run queue for recv, and if there is a
 * buffered FTP command, or are unprocessed telnet input bytes, or more to
 * recv from the socket, we need to insert into the run queue for later
 * input processing.
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 *
 * Return:
 * no return value.
 */
static void tfFtpdQueueUnScannedCtrlInput (
                                        ttFtpdConnEntryPtr ftpdConnEntryPtr )
{
    int tempInt;
    int ctrlSocketDesc;

    ctrlSocketDesc = ftpdConnEntryPtr->fconDesc[TM_FTP_CTRL_SOCKET];
    if (   (   ftpdConnEntryPtr->fconCBFlags[TM_FTP_CTRL_SOCKET]
             & (TM_CB_RECV|TM_CB_RECV_OOB) ) == 0 )
/* Not in the run queue for recv */
    {
        if (     ( tm_16bit_one_bit_set( ftpdConnEntryPtr->fconFlags,
                                         TM_FTPF_COMMAND_BUFFERED ) )
             ||  (    ftpdConnEntryPtr->fconTelBufReadIndex
                   != ftpdConnEntryPtr->fconTelBufProcessIndex ) )
/* Buffered FTP command, or unprocessed telnet input buffer bytes */
        {
            tempInt = 1;
        }
        else
        {
/* Bytes in the socket recv queue */
            tempInt = tfGetWaitingBytes(ctrlSocketDesc);
        }
        if ( tempInt > 0 )
        {
/* Queue this entry in the run queue */
            tfFtpdSignal( ctrlSocketDesc,
                         (ttVoidPtr)ftpdConnEntryPtr,
                         TM_CB_RECV );
        }
    }
}

/*
 * tfFtpdSendCtrl function description:
 * Send a reply on the FTP connection channel if the reply string pointer
 * is non null.
 * 1. Compute the length of the string to be sent.
 * 2. Send the string on the control connection in non blocking mode.
 * 3. If the send failed, check the error. We also fail if we get
 *    a partial copy, as we ought not to fill the send queue on
 *    the control connection.
 * 4. Return error code. If the passed error code is non zero
 *    return that instead of the error code found in this routine.
 *
 * Parameters:
 * Value                Meaning
 * ctrlSocketDesc       Control socket descriptor
 * stringPtr            pointer to reply string to send
 * currentErrorCode     current error code on the control connection
 *
 * Return:
 * errorCode on the control connection
 */
static int tfFtpdSendCtrl (int       ctrlSocketDesc,
                           ttCharPtr stringPtr,
                           int       currentErrorCode)
{
    int     errorCode;
    int     retCode;
    int     length;

    errorCode = TM_ENOERROR; /* assume no error */
    if (stringPtr != (ttCharPtr)0)
    {
        length = (int)tm_strlen(stringPtr);
        retCode = send(ctrlSocketDesc, stringPtr, length, MSG_DONTWAIT);
        if (retCode != length)
        {
            if (retCode == TM_SOCKET_ERROR)
            {
                errorCode = tfGetSocketError(ctrlSocketDesc);
            }
            else
            {
                errorCode = TM_EWOULDBLOCK;
            }
        }
    }
    if (currentErrorCode != TM_ENOERROR)
    {
        errorCode = currentErrorCode;
    }
    return errorCode;
}

/*
 * tfFtpdAcceptData function description:
 * Accept a data connection (in passive mode). Called from the FTPD execute
 * function, when an FTP control block has been queued into the FTPD run Q,
 * because a CB accept (that was registered for when an FTP passive command
 * had arrived) has arrived on the data connection.
 * 1. Call accept in non blocking mode.
 * 2. If accept is successful, close the current data socket (listening
 *    socket).
 * 3. Store the new data socket in the control block.
 * 4. Turn off the data connection timeout timer.
 * 5. Call a common function to reduce the TCP time wait state time, set the
 *    TCP_NOPUSH option for maximum throughput, set socket to non blocking
 *    mode, and set call back flags (CB_RECV, CB_WRITE_READY, CB_REMOTE_CLOSE,
 *    CB_RESET, and CB_SOCKET_ERROR).
 * 6. Set flag to mark that the connection is established.
 * 7. If the data transfer command had already arrived, start transfering
 *    the data.
 * 8. If we failed on the data connection: if the remote already sent the
 *    data transfer command, send failure code (tfFtpdEndData() will send
 *    the reply code string only if data transfer bit is on), otherwise keep
 *    the TM_FTPF_DATA_PASV flag on after closing the data connection, so that
 *    when the data transfer command does arrive, we do not try and connect,
 *    and we send the appropriate reply code string. In both cases, close
 *    the data connection (done in tfFtpdEndData()) in this routine.
 * 9. Return error code (if any) on the control connection.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * Return:
 * errorCode on the control connection
 */
static int tfFtpdAcceptData (ttFtpdConnEntryPtr ftpdConnEntryPtr)
{
    ttCharPtr               replyPtr;
    struct sockaddr_storage sockAddress;
    int                     socketDesc;
    int                     tempInt;
    int                     errorCode; /* control socket error */
    int                     retCode; /* data socket error */
    tt16Bit                 conFlags;
    ttSockAddrPtrUnion      tempSockAddr;

    errorCode = TM_SOCKET_ERROR; /* control socket error */

    tempInt = sizeof(struct sockaddr_storage);
    socketDesc = ftpdConnEntryPtr->fconDesc[TM_FTP_DATA_SOCKET];
    errorCode = TM_ENOERROR;
/* Copy the flags */
    tempSockAddr.sockNgPtr = &sockAddress;
    retCode = accept( socketDesc,
                      tempSockAddr.sockPtr,
                      &tempInt);
    conFlags = ftpdConnEntryPtr->fconFlags;
    if (retCode != TM_SOCKET_ERROR)
    {
        tfFtpdCloseSocket(socketDesc);
        ftpdConnEntryPtr->fconDesc[TM_FTP_DATA_SOCKET] = retCode;
        retCode = TM_ENOERROR;
        if (tm_16bit_one_bit_set(conFlags, TM_FTPF_DATA_TRANSFER))
        {
/* Transfer command arrived before data connection request */
            if (tm_16bit_one_bit_set(conFlags, TM_FTPF_DATA_READ))
            {
/*
 * Turn off the timeout timer, but only if we are sending data (i.e reading
 * from the file system).
 */
                tm_16bit_clr_bit(conFlags, TM_FTPF_DATA_TIMER);
            }
/*
 * Reduce time wait time, set TCP_NOPUSH, set socket to non blocking,
 * set call back flags.
 */
            retCode = tfFtpdSetDataOptions(ftpdConnEntryPtr, conFlags);
            if (retCode == TM_ENOERROR)
            {
/*
 * Connection is established, and command has been received (in transfer mode).
 * Reset current passive flag (for next command). Set the
 * connection established flag.
 */
                conFlags = (tt16Bit)( conFlags | TM_FTPF_DATA_CON );
                tm_16bit_clr_bit(conFlags,TM_FTPF_DATA_PASV);
#ifdef TM_USE_FTPD_SSL            
/* Let the state machine know we're waiting for Ssl negotiation to finish */
                if( tm_16bit_one_bit_set( ftpdConnEntryPtr->fconSslFlags,
                                          TM_FTPD_SSL_ENCRYPT_DATA) )
                {
                    ftpdConnEntryPtr->fconSslFlags |= TM_FTPD_SSL_NEGOTIATING;
                    ftpdConnEntryPtr->fconFlags = conFlags;
                }
                else
                {
/* start data transfer (in tfFtpdTransferData()). */
                    errorCode = tfFtpdTransferData(ftpdConnEntryPtr);
                }
#else /* ! TM_USE_FTPD_SSL */
                ftpdConnEntryPtr->fconFlags = conFlags;
/* start data transfer (in tfFtpdTransferData()). */
                errorCode = tfFtpdTransferData(ftpdConnEntryPtr);
#endif /* TM_USE_FTPD_SSL */
            }
        }
        else
        {
/* Connection is established. Still waiting for the data transfer command */
            conFlags = (tt16Bit)(   conFlags
                                  | TM_FTPF_DATA_CON | TM_FTPF_DATA_PASV);
#ifdef TM_USE_FTPD_SSL            
/* Let the state machine know we're waiting for Ssl negotiation to finish */
            if( tm_16bit_one_bit_set( ftpdConnEntryPtr->fconSslFlags,
                                      TM_FTPD_SSL_ENCRYPT_DATA) )
            {
                ftpdConnEntryPtr->fconSslFlags |= TM_FTPD_SSL_NEGOTIATING;
            }
#endif /* TM_USE_FTPD_SSL */
            ftpdConnEntryPtr->fconFlags = conFlags;
        }
    }
    if (retCode != TM_ENOERROR)
    {
/*
 * If we failed on the data connection in this routine:
 * If the remote already sent the data transfer command, send failure code.
 * (tfFtpdEndData() will send the reply code string only if data transfer
 *  bit is on.)
 */
        replyPtr = (ttCharPtr)&tlFtpd425[0];
        if ( tm_16bit_bits_not_set(conFlags, TM_FTPF_DATA_TRANSFER) )
        {
            conFlags = (tt16Bit)0;
        }
        else
        {
/*
 * Otherwise, postpone sending a reply code string when the data transfer
 * command arrives, and keep the TM_FTPF_DATA_PASV flag on, so that when the
 * transfer command arrives, we know that the socket was passive, and will
 * not try to connect.
 */
            conFlags = TM_FTPF_DATA_PASV;
        }
        errorCode = tfFtpdEndData(ftpdConnEntryPtr, replyPtr);
        ftpdConnEntryPtr->fconFlags = (tt16Bit)
                                            (  ftpdConnEntryPtr->fconFlags
                                             | conFlags );
    }
    return errorCode;
}

/*
 * tfFtpdOpenFileData function description:
 * Called from any one of the FTP file data transfer command (i.e STOR, STOU,
 * APPE, RETR).
 * 1. Remove from the FTP connection type the special flag that indicates
 *    whether or not the file system used CR, LF for end of line.
 * 2. open the file passing the type (ascii, binary), the structure (file,
 *    record), and the mode of opening the file (READ, WRITE, APPEND) and
 *    get back a file pointer.
 * 3. If successful, store the file pointer in the FTP connection entry,
 *    indicate the file transfer, and the direction of the file transfer
 *    in the connection flags, call tfFtpdOpenData() to initiate the data
 *    connection and/or data transfer, return reply code string returned
 *    by tfFtpdOpenData().
 * 4. otherwise return 550 reply code string.
 * Parameters
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * fileNamePtr          Pointer to a null terminated string containing the
 *                      file name.
 * flag                 File sytem flag, either read, write or append.
 * Return:
 * replyPtr             Pointer to a reply string to be sent to the FTP
 *                      client.
 */
static ttCharPtr tfFtpdOpenFileData (ttFtpdConnEntryPtr  ftpdConnEntryPtr,
                                     ttCharPtr           fileNamePtr,
                                     int                 flag )
{
    ttVoidPtr filePtr;
    ttCharPtr replyPtr;
    tt16Bit   conFlags;
    int       txType;

    txType = (int)ftpdConnEntryPtr->fconTxType;
    txType = txType & TM_TYPE_FLAG;
    filePtr = tfFSOpenFile( ftpdConnEntryPtr->fconUserDataPtr,
                            fileNamePtr,
                            flag,
                            txType,
                            (int)ftpdConnEntryPtr->fconTxStru );
    if (filePtr != (ttCharPtr)0)
    {
/* Open file successful, now open the data connection */
        ftpdConnEntryPtr->fconFileDirPtr = filePtr;
        if (flag == TM_FS_READ)
        {
            conFlags = TM_FTPF_DATA_FILE | TM_FTPF_DATA_READ;
        }
        else
        {
/*
 * Set the TM_FTPF_DATA_TIMER flag so that we add a timeout timer
 * when we recv data from the peer.
 */
            conFlags =   TM_FTPF_DATA_FILE | TM_FTPF_DATA_WRITE
                       | TM_FTPF_DATA_TIMER;
        }
        ftpdConnEntryPtr->fconFlags = (tt16Bit)
                         (ftpdConnEntryPtr->fconFlags | conFlags);
        replyPtr = tfFtpdOpenData(ftpdConnEntryPtr);
    }
    else
    {
        replyPtr = (ttCharPtr)&tlFtpd550[0]; /* failure to open file */
    }
    return replyPtr;
}

/*
 * tfFtpdOpenDirData function description:
 * Called from any one of the FTP directory transfer command (i.e NLST, LIST)
 * 1. open the directory and get back a directory pointer.
 * 3. If successful, store the directory pointer in the FTP connection entry,
 *    indicate the directory transfer, and the file system read
 *    in the connection flags, call tfFtpdOpenData() to initiate the data
 *    connection and/or data transfer, return reply code string returned
 *    by tfFtpdOpenData().
 * 4. otherwise return 550 reply code string.
 * Parameters
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * dirNamePtr           Pointer to a null terminated string containing the
 *                      directory name.
 * flag                 Long or short directory listing.
 * Return:
 * replyPtr             Pointer to a reply string to be sent to the FTP
 *                      client.
 */
static ttCharPtr tfFtpdOpenDirData (ttFtpdConnEntryPtr  ftpdConnEntryPtr,
                                    ttCharPtr           dirNamePtr,
                                    int                 flag )
{
    ttVoidPtr dirPtr;
	ttCharPtr replyPtr;
    int       errorCode;
    tt8Bit    changeDir;
    tt8Bit    doWildcard;

    dirPtr = (ttCharPtr)0;
    changeDir = TM_8BIT_NO;
    doWildcard = TM_8BIT_NO;

	if( !GetCurrentDir(ftpdConnEntryPtr->fconUserDataPtr, dirNamePtr )){
	    if ( *dirNamePtr == '\0' )
	    {
	        doWildcard = TM_8BIT_YES;
	    }
	    else
	    {
	/*
	 * First try to lookup the argument as a directory.
	 * Save the current working directory in fconTempBuf.
	 */
	        tfFSGetWorkingDir(ftpdConnEntryPtr->fconUserDataPtr,
	                          (char*)ftpdConnEntryPtr->fconTempBuf,
	                          TM_TEMP_BUF_SIZE);
	        errorCode = tfFSChangeDir(ftpdConnEntryPtr->fconUserDataPtr,
	                                  dirNamePtr);
	        if (errorCode == TM_ENOERROR)
	        {
	            changeDir = TM_8BIT_YES;
	            doWildcard = TM_8BIT_YES;
	        }
	    }
		
	    if (doWildcard == TM_8BIT_YES)
	    {
	        dirNamePtr[0] = '/';
	        dirNamePtr[1] = 0;
	    }
	}
    dirPtr = tfFSOpenDir(ftpdConnEntryPtr->fconUserDataPtr,
                         dirNamePtr,
                         flag);

    if (changeDir == TM_8BIT_YES)
    {
        (void)tfFSChangeDir(ftpdConnEntryPtr->fconUserDataPtr,
                            (char*)ftpdConnEntryPtr->fconTempBuf);
    }

    if (dirPtr != (ttCharPtr)0)
    {
		ftp_make_dir( dirNamePtr );
        /* Open dir successful, now open the data connection */
        ftpdConnEntryPtr->fconFileDirPtr = dirPtr;
        ftpdConnEntryPtr->fconFlags = (tt16Bit)
                                   (   ftpdConnEntryPtr->fconFlags
                                     | TM_FTPF_DATA_DIR | TM_FTPF_DATA_READ );
        replyPtr = tfFtpdOpenData(ftpdConnEntryPtr);
    }
    else
    {
        replyPtr = (ttCharPtr)&tlFtpd550[0];
    }
    return replyPtr;
}

/*
 * tfFtpdOpenData function description:
 * Open a data connection. Function called when a data transfer FTP
 * command has been received either from tfFtpdOpenFileData() (i.e RETR,
 * STOR, STOU, APPE), or from tfFtpdOpenDirdata() (LIST, NLST).
 * Detailed implementation;
 * 1. For a directory transfer use type ASCII, and structure stream for
 *    the duration of the data transfer, otherwise use the connection
 *    values.
 * 2. Initialize all the transfer data variables for type ascii, or
 *    structure record: data transfer state, first end of record,
 *    second end of record, and escape first end of record. In addition,
 *    for a structure record, if the file system does not support
 *    record structure, we pretend that we are type binary, so that
 *    no translation occurs (RFC 1123), otherwise pretend we are type
 *    ascii, to have the record translation occur.
 * 3. In active mode, if we are not reusing a previous data connection,
 *    attempt to open a new data connection in non-blocking mode:
 *    bind to the FTP server data port (20), call a common function to
 *    reduce the TCP time wait state time, set the TCP_NOPUSH option
 *    for maximum throughput, set socket to non blocking
 *    mode, and set call back flags (CB_CONNECT_COMPLT, CB_RECV,
 *    CB_WRITE_READY, CB_REMOTE_CLOSE, CB_RESET, CB_SOCKET_ERROR). Use
 *    the appropriate remote IP address to connect to (default, or port
 *    address if port command has been used prior to this data transfer
 *    command).
 * 4. Else, in passive mode, check whether an error occurred in the
 *    accept phase, in which case we close the connection below in step 7.
 * 5. If command was STOU, we need to use a special reply string that
 *    was stored in our connection entry temporary buffer and
 *    replace the null termination (used to pass the argument to
 *    tfFtpdOpenFileData()) with the proper telnet termination.
 * 6. Otherwise, set the reply code string to either 125 (if we are already
 *    connected), or 150 (if connection has not finished yet, or failed) to
 *    show the connection attempt.
 * 7. If we failed in this routine send the 150 code that shows the
 *    connection attempt, and close the data connection, set the
 *    reply code string to 425.
 * 8. If we are connected, start to transfer the data.
 * 9. If an error occurred on the control connection during the data transfer
 *    set the error code in the connection entry.
 * 10.Return the reply code string (as set in 5, 6, or 7).
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * Return:
 * replyPtr             Pointer to a reply string to be sent to the FTP
 *                      client.
 */
static ttCharPtr tfFtpdOpenData (ttFtpdConnEntryPtr ftpdConnEntryPtr)
{
    struct sockaddr_storage TM_FAR * sockAddrPtr;
    ttCharPtr          replyPtr;
    int                socketDesc;
    int                tempInt;
    int                txType;
    int                txStru;
    int                retCode;             /* data socket error code */
    int                errorCode;           /* control socket error code */
    tt16Bit            conFlags;
    tt8Bit             firstEndOfRecord;
    tt8Bit             secondEndOfRecord;
    tt8Bit             escapeEndOfRecord;
    ttSockAddrPtrUnion tempSockAddr;

    conFlags = ftpdConnEntryPtr->fconFlags;
/* We just received a data transfer command */
    conFlags = (tt16Bit)(conFlags | TM_FTPF_DATA_TRANSFER);
    if ( tm_16bit_one_bit_set(conFlags, TM_FTPF_DATA_DIR) )
    {
/* directory listing, use ascii type, and CR,LF delimiter (file structure) */
        txType = TM_TYPE_ASCII;
        txStru = TM_STRU_STREAM;
    }
    else
    {
        txStru = ftpdConnEntryPtr->fconTxStru;
        txType = (int)ftpdConnEntryPtr->fconTxType; /* transfer type */
    }
    if (txStru == TM_STRU_RECORD)
    {
/*
 * File structure is record, use 0xFF,0x1 for record delimiter, and
 * 0xFF escape for 0xFF escape.
 */
        firstEndOfRecord = TM_RECORD_ESCAPE;
        secondEndOfRecord = TM_RECORD_EOR;
        escapeEndOfRecord = TM_RECORD_ESCAPE;
/*
 * RFC 1123:
 * If the file system does not support record structure, we copy the
 * byte stream literally, no translation.
 */
        if ( (tm_context(tvFtpdFsFlags) & TM_FS_RECORD_FLAG) == 0 )
        {
/* Pretend we are binary, no translation */
            txType = TM_TYPE_BINARY;
        }
        else
        {
/* Pretend we are ascii, to make the record translation happen */
            txType = TM_TYPE_ASCII;
        }
    }
    else
    {
/*
 * Used for type ascii when file structure is stream,
 * delimiters are CR,LF, and '\0' is CR escape.
 */
        firstEndOfRecord = TM_NVT_CR;
        secondEndOfRecord = TM_NVT_LF;
        escapeEndOfRecord = '\0'; /* CR escape */
    }
    ftpdConnEntryPtr->fconActualTxType = (tt8Bit)txType; /* transfer type */
    ftpdConnEntryPtr->fconActualTxStru = (tt8Bit)txStru; /* file structure */
    ftpdConnEntryPtr->fconFirstEndOfRecord = firstEndOfRecord;
    ftpdConnEntryPtr->fconSecondEndOfRecord = secondEndOfRecord;
    ftpdConnEntryPtr->fconEscapeEndOfRecord = escapeEndOfRecord;
    ftpdConnEntryPtr->fconTotalBytesTransferred = 0;
    ftpdConnEntryPtr->fconFlags = conFlags;
    errorCode = TM_ENOERROR;
    retCode = TM_ENOERROR;
    if ( tm_16bit_bits_not_set(conFlags, TM_FTPF_DATA_PASV|TM_FTPF_DATA_CON) )
/* In active mode, and not re-using a previous data connection */
    {
        if ( tm_16bit_one_bit_set( conFlags, TM_FTPF_DATA_PORT ) )
        {
/*
 * If we received a port command, use remote address given by the port
 * command
 */
            sockAddrPtr = &ftpdConnEntryPtr->fconPortClientDataAddress;
        }
        else
        {
/*
 * Otherwise use the same remote address and port as on the control
 * connection.
 */
            sockAddrPtr = &ftpdConnEntryPtr->fconClientCtrlAddress;
        }
/* Open a socket for the data connection that we need to connect on */
        socketDesc = socket(sockAddrPtr->ss_family, SOCK_STREAM, IP_PROTOTCP);
        retCode = TM_SOCKET_ERROR;
        if (socketDesc != TM_SOCKET_ERROR)
        {
            ftpdConnEntryPtr->fconDesc[TM_FTP_DATA_SOCKET] = socketDesc;
            tempInt = 1;
/*
 * Need to ask for reuse addr option, since every data socket will use
 * port 20, and we could have more than one data connection at a given
 * time.
 */
            retCode = setsockopt( socketDesc,
                                  SOL_SOCKET,
                                  SO_REUSEADDR,
                                  (const char TM_FAR *)&tempInt,
                                  sizeof(int) );
            if (retCode == TM_ENOERROR)
            {
/* Address to bind to */
                tempSockAddr.sockNgPtr =
                                 &(ftpdConnEntryPtr->fconSrvrDataAddress);
                retCode = bind( socketDesc,
                                tempSockAddr.sockPtr,
                                tempSockAddr.sockPtr->sa_len );
                if (retCode == TM_ENOERROR)
                {
/*
 * Reduce time wait time, set TCP_NOPUSH, set socket to non blocking,
 * set call back flags.
 */
                    retCode = tfFtpdSetDataOptions( ftpdConnEntryPtr,
                                                    conFlags );
                    if (retCode == TM_ENOERROR)
                    {
                        tempSockAddr.sockNgPtr = sockAddrPtr;
                        retCode = connect(
                                        socketDesc,
                                        tempSockAddr.sockPtr,
                                        sizeof(struct sockaddr_storage));
                        if (retCode != TM_ENOERROR)
                        {
                            retCode = tfGetSocketError(socketDesc);
                            if (retCode == TM_EINPROGRESS)
                            {
                                retCode = TM_ENOERROR;
/* Connection not established yet. */
                            }
                        }
                        else
                        {
/* Connection established */
                            conFlags = (tt16Bit)( conFlags
                                                 | TM_FTPF_DATA_CON );
                            ftpdConnEntryPtr->fconFlags = conFlags;
#ifdef TM_USE_FTPD_SSL        
/* Check if the data connection should be encrypted*/
                            if(   tm_16bit_one_bit_set(
                                      ftpdConnEntryPtr->fconSslFlags,
                                      TM_FTPD_SSL_ENCRYPT_DATA) 
                               && tm_16bit_bits_not_set(
                                      ftpdConnEntryPtr->fconSslFlags,
                                      TM_FTPD_SSL_DATA_ENCRYPTED
                                    | TM_FTPD_SSL_NEGOTIATING )   
                                )
                            {
/* If the socket is not already encrypted or negotiating encryption, encrypt it. */

/* Set the socket options on the data socket */
                                    (void)tfFtpdSslSetSockOpt(
                                            ftpdConnEntryPtr->fconDesc
                                                [ TM_FTP_DATA_SOCKET ] );


/* Add the SSL layer to the data socket */
                                    retCode = tfSslServerUserCreate(
                                              ftpdConnEntryPtr->fconDesc
                                                  [ TM_FTP_DATA_SOCKET ] );
                                    if (retCode == TM_ENOERROR)
                                    {
/* Let the state machine know we're waiting for Ssl negotiation to finish */
                                        ftpdConnEntryPtr->fconSslFlags |=
                                            TM_FTPD_SSL_NEGOTIATING;                                    
                                    }
                                    else
                                    {
                                        retCode = TM_SOCKET_ERROR;
                                    }
                            }
#endif /* TM_USE_FTPD_SSL */
                        }
                    }
                }
            }
        }
    }
    else
/* Re-using a previous connection or passive connection */
    {
/*
 * If an error occurred in passive mode, need to let the remote know about
 * it
 */
        if (    ftpdConnEntryPtr->fconDesc[TM_FTP_DATA_SOCKET]
             == TM_SOCKET_ERROR )
        {
            retCode = TM_SOCKET_ERROR;
        }
        else
        {
            if ( tm_16bit_one_bit_set(conFlags, TM_FTPF_DATA_CON) )
            {
/* Data connection request arrived before Transfer command (FTP(Kame)) */
                retCode = tfFtpdSetDataOptions( ftpdConnEntryPtr,
                                                conFlags );
                if (retCode == TM_ENOERROR)
                {
/*
 * Reset current passive flag (for next command)
 */
                    tm_16bit_clr_bit(conFlags, TM_FTPF_DATA_PASV);
                    if (tm_16bit_one_bit_set(conFlags, TM_FTPF_DATA_READ))
                    {
/*
 * Turn off the timeout timer, but only if we are sending data (i.e reading
 * from the file system).
 */
                        tm_16bit_clr_bit(conFlags, TM_FTPF_DATA_TIMER);
                    }
                }
            }
        }
    }
    ftpdConnEntryPtr->fconFlags = conFlags;
/* Set appropriate reply code string */
    if (ftpdConnEntryPtr->fconCommand == (tt16Bit)TM_STOU)
/* For a store unique command reply with the unique file name */
    {
/*
 * Replace the '\0' at the end of the file name with the overwritten
 * '\r'
 */
        ftpdConnEntryPtr->fconTempBuf[ftpdConnEntryPtr->fconStouLength] =
                                                                         '\r';
        replyPtr = (ttCharPtr)&ftpdConnEntryPtr->fconTempBuf[0];
    }
    else
    {
        if ( tm_16bit_one_bit_set(conFlags, TM_FTPF_DATA_CON) 
#ifdef TM_USE_FTPD_SSL            
                    && ( tm_16bit_bits_not_set(conFlags, 
                            TM_FTPD_SSL_NEGOTIATING) )
#endif /* TM_USE_FTPD_SSL */            
           )
        {
/*
 * Already connected: reusing previous (active or passive) socket
 * connection, or a passive socket has already been connected to,
 * or our connect just completed.
 */
            replyPtr = (ttCharPtr)&tlFtpd125[0];
        }
        else
        {
/* opening a new data connection */
            replyPtr = (ttCharPtr)&tlFtpd150[0];
        }
    }
    if (retCode != TM_ENOERROR)
    {
/* Send the 150 that shows the connection attempt */
        (void)tfFtpdEndData(ftpdConnEntryPtr, replyPtr);
/* Send the 425 that shows the connection failure */
        replyPtr = (ttCharPtr)&tlFtpd425[0];
    }
    if ( tm_16bit_one_bit_set(conFlags, TM_FTPF_DATA_CON) )
    {
/*
 * . If we are re-using a previous data socket,
 * . OR for a Passive command, if the remote already connected to us
 *   (connection already established in tfFtpdAcceptData()),
 * . OR for an Active command, if we just connected successfully,
 * then connection is complete, and data transfer can start.
 */
/* Send the reply right away to sending the 125/150 before the 226/426 */
        errorCode = tfFtpdSendCtrl(
                ftpdConnEntryPtr->fconDesc[TM_FTP_CTRL_SOCKET],
                replyPtr,
                TM_ENOERROR );
        replyPtr = (ttCharPtr)0;
        if (       errorCode == TM_ENOERROR
#ifdef TM_USE_FTPD_SSL
                && ( tm_16bit_bits_not_set(ftpdConnEntryPtr->fconSslFlags,
                                         TM_FTPD_SSL_NEGOTIATING) )
#endif /* TM_USE_FTPD_SSL */
            )
        {
            errorCode = tfFtpdTransferData(ftpdConnEntryPtr);
        }
    }
/*
 * Otherwise wait until connection is established in
 * tfFtpdAcceptData(), or when the connect complete call back occurs).
 */
    if (errorCode != TM_ENOERROR)
    {
        ftpdConnEntryPtr->fconErrorCode = errorCode;
    }
    return replyPtr;
}

/*
 * tfFtpSetDataOptions function description:
 *
 * 1. Fetch the data socket in the connection entry.
 * 2. Reduce the TCP time wait state time to the minimum.
 * 3. set TCP_NOPUSH TCP option
 * 4. Set call back flags according to passed parameter
 * 5. set socket to non blocking mode
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * cbFlags              call back flags to be set on the socket
 * Return:
 * errorCode            error Code on data connection
 */
static int tfFtpdSetDataOptions (ttFtpdConnEntryPtr ftpdConnEntryPtr,
                                 int                conFlags)
{
    int socketDesc;
    int tempInt;
    int retCode;
    int cbFlags;

/* Data socket descriptor */
    socketDesc = ftpdConnEntryPtr->fconDesc[TM_FTP_DATA_SOCKET];
/* Change Time Wait to minimum value */
    tempInt = 0;
    retCode = setsockopt( socketDesc,
                          IPPROTO_TCP,
                          TM_TCP_2MSLTIME,
                          (const char TM_FAR *)&tempInt,
                          sizeof(int) );
    if (retCode == TM_ENOERROR)
    {
/* TCP_NOPUSH option for throughput */
        tempInt = 1;
        retCode = setsockopt( socketDesc,
                              IPPROTO_TCP,
                              TCP_NOPUSH,
                              (const char TM_FAR *)&tempInt,
                              sizeof(int) );
        if (retCode == TM_ENOERROR)
        {
/*
 * Reduce time wait time, set TCP_NOPUSH, set socket to non blocking,
 * set call back flags.
 */
            if (tm_16bit_one_bit_set(conFlags, TM_FTPF_DATA_READ))
            {
/* Reading from a file/writing to the network */
                cbFlags = (  TM_CB_WRITE_READY
                           | TM_CB_REMOTE_CLOSE
                           | TM_CB_RESET
                           | TM_CB_SOCKET_ERROR );
            }
            else
            {
/* reading from the network /writing to a file */
                cbFlags = (  TM_CB_RECV
                           | TM_CB_REMOTE_CLOSE
                           | TM_CB_RESET
                           | TM_CB_SOCKET_ERROR );
            }
            if ( tm_16bit_bits_not_set(conFlags,
                                       TM_FTPF_DATA_PASV|TM_FTPF_DATA_CON) )
            {
/* In active mode, and not re-using a previous data connection */
                cbFlags = cbFlags | TM_CB_CONNECT_COMPLT;
            }
#ifdef TM_USE_FTPD_SSL
/* Using SSL encryption, and supposed to encrypt the data channel */
            if ( tm_16bit_all_bits_set(ftpdConnEntryPtr->fconSslFlags,
                                       TM_FTPD_SSL_ENCRYPT_DATA ) )
            {
                cbFlags |= TM_CB_SSL_ESTABLISHED;
            }
#endif /* TM_USE_FTPD_SSL */
/* Register socket call back */
            retCode = tfRegisterSocketCBParam(socketDesc,
                                              tfFtpdSignal,
                                              ftpdConnEntryPtr,
                                              cbFlags);
            if (retCode == TM_ENOERROR)
            {
/* Set socket to non blocking mode */
                retCode = tfBlockingState(socketDesc, TM_BLOCKING_OFF);
            }
        }
    }
    return retCode;
}

/*
 * tfFtpdTransferData function description:
 * Start data transfer. Called when the data transfer command has arrived,
 * and we are connected (either from tfFtpdExecute() when CB_CONNECT_COMPLT
 * happens for an active connection, or from tfFtpdAcceptData() when
 * CB_ACCEPT happens for a passive connection after the data transfer
 * command has arrived, or from tfFtpdOpenData() when the data transfer
 * command itself arrives if the accept had already occurred for a passive
 * connection, or if the connect was loop back for an active connection.
 * 1. If the data transfer command was a read (such as RETR, NLST, LIST)
 *    read data from a file or directory, send data to the network.
 * 2. If the data transfer command was a write (such as STOR, STOU, APPE),
 *    receive data from the network, write data to a file.
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * Return:
 * errorCode on the control connection
 */
static int tfFtpdTransferData (ttFtpdConnEntryPtr ftpdConnEntryPtr)
{
    int     errorCode;
    int     optLen;
    int     optValue;


/* start transfer */
    if (ftpdConnEntryPtr->fconFlags & TM_FTPF_DATA_READ)
    {
/* determin the send buffer size we should use
 * use the effective maximum TCP segment
 */
        optLen = sizeof(optValue);

/* We are sure that the input parameters are valid, therefore, no need to
 * check the return value
 */
        (void)getsockopt(
                ftpdConnEntryPtr->fconDesc[TM_FTP_DATA_SOCKET],
                IPPROTO_TCP,
                TM_TCP_EFF_MAXSEG,
                (ttCharPtr)&optValue,
                &optLen);

        ftpdConnEntryPtr->fconSendBufferSize = (tt16Bit)optValue;


/* Read from a file, send data on the network */
        errorCode = tfFtpdSendData(ftpdConnEntryPtr);
    }
    else
    {
/* Recv data from the network, write data to a file */
        errorCode = tfFtpdRecvData(ftpdConnEntryPtr);
    }
    return errorCode;
}

/*
 * Recv some data on a data socket, write to a file. Handle record structure,
 * type ascii, binary and local.
 * 1. Check if file has been opened.
 * 2. Save all FTP connection variables in local variables.
 * 3. In a do while loop while there is data to recv, do a zero copy recv
 *    to get data from the network.
 * 3.1. If there is any data
 * 3.1.1 If type ascii (which is also set for structure record if file
 *     system supports structure record (see tfFtpdOpenData)):
 *     For type ascci in structure file, map \r\n to eor and \r\0 to \r.
 *     For structure record map 0xFF,0x1 to EOR, 0xFF,0x2 to EOF
 *     0xFF, 0x3 to EOR and EOF, 0xFF,0xFF to 0xFF. We flush to
 *     the file system whenever we encounter an end of record, or end
 *     of line, or an escaped first end of record/end of line character.
 *     Deal with first end of record at the end of a buffer.
 * 3.1.2 if type binary (also for structure record when file system does
 *     not support record structure, and also for ascii type when file
 *     system uses CR, LF for end of line (see tfFtpdOpenData)), just
 *     flush buffer to file system.
 * 3.1.3 Free the zero copy buffer.
 * 3.1.4 break out of the loop if any error occurred while writing to the
 *     file, otherwise update the total bytes transferred.
 * 3.2 Otherwise (no recv data),
 * 3.2.1 if zero copy recv returned an error
 *       map to socket error, and set return code. If socket error is
 *       TM_EWOULDBLOCK, set return code to zero to ignore the error.
 * 3.2.2 If zero copy recv returned end of file (0 length), make sure we
 *       store a CR, if last seen character was a CR that we used to
 *       transition to DATA_CR_STATE. Set reply code string to 226.
 * 4. Out of the do while loop, if there was a network recv error (other than
 *    TM_EWOULDBLOCK), set reply code string to 426.
 * 5. If reply code string is set, call tfFtpdEndData() that will send the
 *    reply code string, close the file, and close the data connection.
 * 6. Return error code on the control connection if any (when we sent the
 *    reply code string).
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * Return:
 * errorCode on the control connection
 */
static int tfFtpdRecvData ( ttFtpdConnEntryPtr ftpdConnEntryPtr)
{
    ttUserMessage dataMessage;
    ttCharPtr     dataPtr;
    ttCharPtr     bufferPtr;
    ttCharPtr     replyPtr;
    ttVoidPtr     filePtr;
    ttVoidPtr     userDataPtr;
    int           dataDesc;
    int           length;
    int           dataLength;
    int           retCode;
    int           errorCode;
    int           txType;
    int           txStru;
    int           index;
    int           eor;
    tt8Bit        uByte;
    tt8Bit        txState;
    tt8Bit        firstEndOfRecord;
    tt8Bit        escapeEndOfRecord;
    tt8Bit        secondEndOfRecord;

    errorCode = TM_ENOERROR;
    filePtr = ftpdConnEntryPtr->fconFileDirPtr;
    if (filePtr != (ttVoidPtr)0)
    {
        retCode = TM_ENOERROR;
        replyPtr = (ttCharPtr)0;
        dataDesc = ftpdConnEntryPtr->fconDesc[TM_FTP_DATA_SOCKET];
        txType = (int)ftpdConnEntryPtr->fconActualTxType;
        txStru = (int)ftpdConnEntryPtr->fconActualTxStru;
/* Used for ASCII, and record */
        txState = ftpdConnEntryPtr->fconTxState;
        userDataPtr = ftpdConnEntryPtr->fconUserDataPtr;
        firstEndOfRecord = ftpdConnEntryPtr->fconFirstEndOfRecord;
        escapeEndOfRecord = ftpdConnEntryPtr->fconEscapeEndOfRecord;
        secondEndOfRecord = ftpdConnEntryPtr->fconSecondEndOfRecord;
        do
        {
            length = tfZeroCopyRecv(dataDesc,
                                    &dataMessage,
                                    &dataPtr,
                                    TM_BUF_RECV_SIZE,
                                    MSG_DONTWAIT);
            if (length > 0)
            {
                if (txType == TM_TYPE_ASCII)
/*
 * Ascii type, or structure record when the file system supports record
 * structures
 */
                {
/*
 * For type ascci in structure file, map \r\n to eor and \r\0 to \r.
 * For structure record map 0xFF,0x1 to EOR, 0xFF,0x2 to EOF
 * 0xFF, 0x3 to EOR and EOF, 0xFF,0xFF to 0xFF.
 */
                    index = 0;
                    dataLength = 0;
                    eor = 0;
                    while (index < length)
                    {
                        uByte = (tt8Bit)dataPtr[index];
                        index++;
                        if (txState == TM_DATA_STATE)
                        {
                            if (uByte == firstEndOfRecord)
                            {
                                txState = TM_DATA_CR_STATE;
                            }
                        }
                        else
                        {
                            if (uByte == secondEndOfRecord)
                            {
                                txState = TM_DATA_FLUSH;
                                eor = 1;
                                dataLength = index - 1;
                            }
                            else
                            {
                                if (uByte == escapeEndOfRecord)
                                {
                                    txState = TM_DATA_FLUSH;
                                    dataLength = index - 1;
                                }
                                else
                                {
/* Check for end of file */
                                    if (    (txStru == TM_STRU_RECORD)
                                         && (   (uByte == TM_RECORD_EOF)
                                             || (uByte == TM_RECORD_EOR_EOF)))
                                    {
                                        txState = TM_DATA_FLUSH;
                                        dataLength = index - 2;
                                        if (uByte == TM_RECORD_EOR_EOF)
                                        {
                                            eor = 1;
                                        }
                                        replyPtr = (ttCharPtr)&tlFtpd226[0];
                                    }
                                    else
                                    {
                                        txState = TM_DATA_STATE;
                                    }
                                }
                            }
                            if (txState == TM_DATA_FLUSH)
                            {
                                txState = TM_DATA_STATE;
/*
 * Translation for ascii file type only, or for record structure when
 * file system supports records.
 */
                                bufferPtr = dataPtr;
                                if (dataLength < 0)
                                {
/*
 * This could happend if ascii CR, or record 0xFF was at the end of a
 * previous recv buffer, and our current character is either escapeEndOfRecord
 * or secondEndOfRecord, or end of file.
 */
                                    if (uByte == escapeEndOfRecord)
                                    {
/* escapeEndOfRecord, restore firstEndOfRecord, copy with no translation */
                                        uByte = firstEndOfRecord;
                                        dataLength = 1;
                                        bufferPtr = (ttCharPtr)&uByte;
                                    }
                                    else
                                    {
/* SecondEndOfRecord, or end of file */
                                        dataLength = 0;
                                    }
                                }
                                retCode = tfFSWriteFileRecord(userDataPtr,
                                                              filePtr,
                                                              bufferPtr,
                                                              dataLength,
                                                              eor);
                                if (retCode == TM_SOCKET_ERROR)
                                {
                                    break;
                                }
                                dataPtr += index;
                                length -= index;
                                index = 0;
                                eor = 0;
                            }
                        }
                    }
                    if (    (length != 0)
                         && (retCode != TM_SOCKET_ERROR) )
                    {
/* Write the left over data. Watch out for CR_STATE state */
                        if (txState == TM_DATA_CR_STATE)
                        {
/*
 * For Ascii type, or record file structure when the file system supports
 * records. Do not write the last CR, or last 0xFF, since we use it
 * to transition.
 */
                            length--;
                        }
                        if (length != 0)
                        {
                            retCode = tfFSWriteFileRecord( userDataPtr,
                                                           filePtr,
                                                           dataPtr,
                                                           length,
                                                           0 );
                        }
                    }
                }
                else
                {
/*
 * Local, binary, ascii when file system uses CR,LF as end of line, and
 * record structure when the file system does not support record structures
 */
                    retCode = tfFSWriteFile( userDataPtr, filePtr, dataPtr,
                                             length );
                }
                (void)tfFreeZeroCopyBuffer(dataMessage);
                if (retCode == TM_SOCKET_ERROR)
                {
                    break;
                }
                ftpdConnEntryPtr->fconTotalBytesTransferred =
                      ftpdConnEntryPtr->fconTotalBytesTransferred
                    + (tt32Bit)length;
/*
 * BUG ID 11-656:
 * If other connection control blocks are queued, or if there are other
 * pending connections, or if there is a pending control command:
 * . If there is more data to be received, queue this control block at the
 *   end of the run Q,
 * . Exit.
 */
                if (    (    tm_ftpd_q_not_empty(TM_FTPD_RUN_Q)
/* BUG ID 37 added extra check for other connections in the run queue */
                          && (    (!(tm_ftpd_single_in_q(ftpdConnEntryPtr,
                                                         TM_FTPD_RUN_Q)))
/* BUG ID 2919: added extra check for control socket commands */
                               || (ftpdConnEntryPtr->fconCBFlags[
                                                   TM_FTP_CTRL_SOCKET] != 0)
                             )
                        )
                     || (tm_context(tvFtpdPendingConnections) != 0 )
                   )
                {

                    length = tfGetWaitingBytes(dataDesc);
                    if (length != 0)
                    {
/* Signal to make sure that the FTP server will read the remaining bytes */
                        tfFtpdSignal( dataDesc,
                                      (ttVoidPtr)ftpdConnEntryPtr,
                                      TM_CB_RECV );
                        length = 0; /* will cause us to break out of the loop */
                    }
                }
/* End of BUG ID 11-656 */
            }
            else
            {
                if (length == TM_SOCKET_ERROR)
                {
                    retCode = tfGetSocketError(dataDesc);
                    if (retCode == TM_EWOULDBLOCK)
                    {
                        retCode = TM_ENOERROR;
                    }
                }
                else
                {
                    if (length == 0)
/*
 * End of file from network, need to send a 226.
 * Note that the CB_REMOTE_CLOSE will also be set, but the call to
 * tfFtpdEndData from the main execute loop will not do anything, since
 * we will end the data transfer at the bottom of this routine.
 */
                    {
                        if (txState == TM_DATA_CR_STATE)
/*
 * If file end with CR, and we used it to transition, need to write it. This
 * could only happen with ascii type, and not with record structure, since
 * record structure files always end with 0x2, or 0x3.
 */
                        {
                            uByte = firstEndOfRecord;
                            retCode = tfFSWriteFile( userDataPtr, filePtr,
                                                     (ttCharPtr)&uByte, 1 );
                        }
                        replyPtr = (ttCharPtr)&tlFtpd226[0];
                    }
                }
            }
        }
        while (length > 0);
        ftpdConnEntryPtr->fconTxState = txState;
        if (retCode == TM_SOCKET_ERROR)
        {
            replyPtr = (ttCharPtr)&tlFtpd426[0];
/*
 * BUG ID 415: make sure that FTP client does not hang if it ignores our
 * 426 error on the control connection.
 */
            tfResetConnection(dataDesc);
        }
        if (replyPtr != (ttCharPtr)0)
        {
            errorCode = tfFtpdEndData(ftpdConnEntryPtr, replyPtr);
        }
    }
    return errorCode; /* control socket error */
}

/*
 * Read some data from a file or directory, send some data on a data socket
 * 1. Check if file is still opened.
 * 2. Copy all FTP connection variables in local variables.
 * 3. In a do while loop while there is data to send, and there is no
 *    error on the data connection, or with the file read.
 * 3.1 If there is a zero copy buffer that has not been sent, send it in
 *     non-blocking mode.
 * 3.2 If the send returns an error, get the socket error:
 * 3.2.1 If the socket error is TM_EWOULDBLOCK, reset it and store the
 *       buffer variables in the FTP connection entry, so that the buffer
 *       can be sent later on when there is room in the TCP send buffer
 *       queue.
 * 3.2.2 Otherwise reset the data message pointer. We no longer own the
 *       send buffer message.
 * 3.2.3 Break out of the loop to either postpone the data transfer
 *       until we get a CB_WRITE_READY (TM_EWOULDBLOCK), or to end with an
 *       error.
 * 3.3 Else (send did not fail), reset the data message pointer. We no
 *     longer own the zero copy buffer.
 * 3.4 If file input buffer has been completely flushed to the network, and
 *     we have reached the end of file (or directory), set the reply code
 *     string to 226 and break out of the loop. We are done with the
 *     data transfer.
 * 3.5 Get a zero copy buffer. If the call fails, break out of the loop
 *     with a TM_ENOBUFS error code on the data connection. Otherwise
 *     we will copy the file data into the zero copy buffer. For ascii
 *     type we use an intermediate file input buffer (our FTP connection
 *     temporary buffer) to do the transformation.
 * 3.6 If type is ascii (type ascii with file structure when file system
 *     does not use CR,LF for end of line, or record structure when file
 *     system supports record structure, or directory listing).
 *     In a do  while loop while there is data in the file input buffer and
 *     no error:
 * 3.6.1 while there is data in the file input buffer and room in the zero
 *       copy buffer, copy data from the file input buffer into the zero
 *       copy buffer; check for special character ('\r' for type ascii,
 *       0xFF for structure record, to map to '\r','\0' for type ascii,
 *       and 0xFF,0xFF for structure record in the zero copy buffer. Check
 *       for left over '\r' from a previous file input buffer.
 * 3.6.2 If the file input buffer has been entirely processed, reset the
 *       file input buffer process and end indexes.
 * 3.6.3 otherwise, break out of the 3.6 do while loop to flush the full
 *       zero copy buffer to the network.
 * 3.6.4 Next check if we have reached an end of line (or end of record/end
 *       of file). If we have and have room for it, copy the end of record/
 *       end of file to the zero copy buffer ('\r', '\n' for ascii type,
 *       0xFF, 0x01 for end of record, 0xFF, 0x02 for end of file or 0xFF,
 *       0x3 for end of file end of record combined as set in the local
 *       variables firstEndOfRecord, and secondEndOfRecord). If we do not have
 *       room for it, break out of the 3.6 do while loop to flush the zero
 *       copy buffer to the network.
 * 3.6.5 Next if we have not reached the end of the file or directory, copy
 *       more data from the file or directory into our file input buffer
 *       (temporary buffer).
 * 3.6.6 If we have reached end of file, mark the end of file in the
 *       FTP connection flags. For record structure only, mark the end of
 *       file in the file end of record variable, and set the second end
 *       of record to reflect the end of file so that the appropriate code
 *       gets copied into the zero copy buffer in step 3.6.4.
 * 3.6.7 Set the end index to the end of the file input buffer (temporary
 *       buffer).
 * 3.7 If type is binary (type ascii with file structure when file system
 *     uses CR, LF for end of line, or record structure when file system
 *     does not support record structure, or type binary, or type local)
 *     Read data from the file into zero copy buffer. If we have reached
 *     the file end of file, mark the end of file in the FTP connection
 *     flags.
 * 4. Out of the do while loop. If an error occurred on the data connection,
 *    set the reply code string to 426.
 * 5. If the reply code string is set, save the zero copy buffer message
 *    (if any in the data connection), and call tfFtpdEndData(), which
 *    will free the zero copy buffer if any, close the file/directory,
 *    send the reply code string, and close the data connection.
 * 6. Otherwise, save the file input variables in the FTP connection block,
 *    and transmission state (for ASCII type, and structure record mapping).
 * 7. Return error code on the control connection if any.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * Return:
 * errorCode on the control connection
 */
static int tfFtpdSendData ( ttFtpdConnEntryPtr ftpdConnEntryPtr )
{
    ttUserMessage dataMessage;
    ttCharPtr     dataPtr;
    ttCharPtr     copyDataPtr;
    ttCharPtr     bufferPtr;
    ttCharPtr     tempBufPtr;
    ttCharPtr     replyPtr;
    ttVoidPtr     fileDirPtr;
    ttVoidPtr     userDataPtr;
    int           dataDesc;
    int           dataLength;
    int           copyDataLength;
    int           retCode;
    int           errorCode;
    int           sendRetCode;
    int           txType;
    int           txStru;
    int           fileInputEorEof;
    int           fileInputProcessIndex;
    int           fileInputEndIndex;
    int           numberSend; /* BUG ID 11-656 */
    tt16Bit       conFlags;
    tt8Bit        txState;
    tt8Bit        firstEndOfRecord;
    tt8Bit        escapeEndOfRecord;
    tt8Bit        secondEndOfRecord;
    tt8Bit        uByte;

    errorCode = TM_ENOERROR;
    fileDirPtr = ftpdConnEntryPtr->fconFileDirPtr;
    numberSend = 0; /* BUG ID 11-656 */
    if (fileDirPtr != (ttVoidPtr)0)
    {
        dataDesc = ftpdConnEntryPtr->fconDesc[TM_FTP_DATA_SOCKET];
        retCode = TM_ENOERROR; /* == 0 */
        replyPtr = (ttCharPtr)0;
        conFlags = ftpdConnEntryPtr->fconFlags;
        txType = (int)ftpdConnEntryPtr->fconActualTxType;
        txStru = (int)ftpdConnEntryPtr->fconActualTxStru;
        {
/* Used for ASCII, and record only */
            firstEndOfRecord = ftpdConnEntryPtr->fconFirstEndOfRecord;
            escapeEndOfRecord = ftpdConnEntryPtr->fconEscapeEndOfRecord;
            secondEndOfRecord = ftpdConnEntryPtr->fconSecondEndOfRecord;
            tempBufPtr = (ttCharPtr)&ftpdConnEntryPtr->fconTempBuf[0];

            fileInputProcessIndex =
                                ftpdConnEntryPtr->fconFileInputProcessIndex;
            fileInputEndIndex = ftpdConnEntryPtr->fconFileInputEndIndex;
            fileInputEorEof = (int)ftpdConnEntryPtr->fconFileInputEorEof;
            txState = ftpdConnEntryPtr->fconTxState;
        }
        userDataPtr = ftpdConnEntryPtr->fconUserDataPtr;
        dataMessage = ftpdConnEntryPtr->fconDataSendMessage;
        dataPtr = ftpdConnEntryPtr->fconDataSendPtr;
        dataLength = ftpdConnEntryPtr->fconDataSendLength;
/* Reset previously saved message */
        ftpdConnEntryPtr->fconDataSendMessage = (ttUserMessage)0;
        do
        {
/* Try and flush previous buffered data */
/* flush to the network */
            if (    (dataMessage != (ttUserMessage)0)
                 && (dataLength > 0 ) )
            {
/*
 * BUG ID 11-656:
 * Only allow one send per control block if run queue is non empty with
 * other control blocks, or with control command,
 * or if there are other pending connections, to allow other connections
 * their fair share of the CPU.
 */
                if (    (numberSend > 0)
                     && (    (    tm_ftpd_q_not_empty(TM_FTPD_RUN_Q)
/* BUG ID 37 added extra check for other connections in the run queue */
                               && (    (!(tm_ftpd_single_in_q(ftpdConnEntryPtr,
                                                              TM_FTPD_RUN_Q)))
/* BUG ID 2919: added extra check for control socket commands */
                                    || (ftpdConnEntryPtr->fconCBFlags[
                                                   TM_FTP_CTRL_SOCKET] != 0) )
                             )
                          || ( tm_context(tvFtpdPendingConnections) != 0 )
                        )
                   )
                {
/*
 * We still own the buffer. Save buffer to be sent in control block.
 * Queue control block for later transmission in the run queue.
 */
                    ftpdConnEntryPtr->fconDataSendMessage = dataMessage;
                    ftpdConnEntryPtr->fconDataSendPtr = dataPtr;
                    ftpdConnEntryPtr->fconDataSendLength = dataLength;
                    tfFtpdSignal( dataDesc,
                                  (ttVoidPtr)ftpdConnEntryPtr,
                                  TM_CB_WRITE_READY );
                    break;
                }
                else
                {
/* End of BUG ID 11-656 */
                    sendRetCode = tfZeroCopySend( dataDesc,
                                                  dataMessage,
                                                  dataLength,
                                                  MSG_DONTWAIT );
                    numberSend++;
                    if (sendRetCode == TM_SOCKET_ERROR)
                    {
                        sendRetCode = tfGetSocketError(dataDesc);
                        if (sendRetCode == TM_EWOULDBLOCK)
                        {
/*
 * We still own the buffer. Queue for later transmission when we get some
 * room in the run queue.
 */
                            ftpdConnEntryPtr->fconDataSendMessage =
                                                                  dataMessage;
                            ftpdConnEntryPtr->fconDataSendPtr = dataPtr;
                            ftpdConnEntryPtr->fconDataSendLength = dataLength;
                        }
                        else
                        {
/* Reset buffer. We no longer own it. */
                            dataMessage = (ttUserMessage)0;
/* Store error returned from tfGetSocketError() */
                            retCode = sendRetCode;
                        }
                        break;
                    }
                    else
                    {
                        ftpdConnEntryPtr->fconTotalBytesTransferred =
                                 ftpdConnEntryPtr->fconTotalBytesTransferred
                               + (tt32Bit)sendRetCode;
/* Reset buffer. We no longer own it. */
                        dataMessage = (ttUserMessage)0;
                    }
                } /* BUG ID 11-656 */
            }
            if (   (    ( tm_16bit_one_bit_set( conFlags,
                                                TM_FTPF_DATA_READ_DONE ) )
                     && ( fileInputProcessIndex == fileInputEndIndex )
                     && ( fileInputEorEof == 0 ) )
                || ( retCode != TM_ENOERROR ) )
            {
/*
 * Done, and file input buffer flushed, or error reading file/directory
 * We need to break out of the loop, and send the appropriate reply
 * code and message. If no error occured, we should send a 226 reply.
 * If error occured (retCode != TM_ENOERROR), replyPtr will be set to
 * tlFtpd426 at the bottom of this function instead.
 */
                replyPtr = (ttCharPtr)&tlFtpd226[0];
                break;
            }

            dataMessage = tfGetZeroCopyBuffer(
                                    (int)ftpdConnEntryPtr->fconSendBufferSize,
                                    &dataPtr);
            if (dataMessage == (ttUserMessage)0)
            {
                retCode = TM_ENOBUFS;
                break;
            }
            dataLength = 0;
/* Copy new data in buffer */
            if (txType == TM_TYPE_ASCII)
/*
 * Directory read, or
 * Ascii type when CR,LF is not used for end of line,
 * or record structure when record is supported by the file system.
 * Use a temporary buffer for translation.
 */
            {
                bufferPtr = dataPtr;
                do
                {
/* Process, and flush previous file input buffer */
                    while (    (fileInputProcessIndex < fileInputEndIndex)
                            && (dataLength < (int)ftpdConnEntryPtr->
                                                  fconSendBufferSize) )
                    {
                        if (txState == TM_DATA_CR_STATE)
                        {
                            uByte = escapeEndOfRecord;
                            txState = TM_TEL_DATA_STATE;
                        }
                        else
                        {
                            uByte = (tt8Bit)tempBufPtr[fileInputProcessIndex];
                            fileInputProcessIndex++;
                            if (uByte == firstEndOfRecord)
                            {
                                txState = TM_DATA_CR_STATE;
                            }
	                        bufferPtr[dataLength] = (char)uByte;
	                        dataLength++;
                        }
                    }
                    if (fileInputProcessIndex == fileInputEndIndex)
                    {
                        fileInputProcessIndex = 0;
                        fileInputEndIndex = 0;
                    }
                    else
                    {
                        break; /* flush the send buffer */
                    }
                    if (fileInputEorEof != 0)
                    {
/* Flush previous end of record */
                        if (  dataLength
                            < (int)ftpdConnEntryPtr->fconSendBufferSize - 1)
                        {
                            bufferPtr[dataLength] = (char)firstEndOfRecord;
                            dataLength++;
/*
 * Note: secondEndOfRecord contains the appropriate value if an end of
 * file has been reached. See below.
 */
                            bufferPtr[dataLength] = (char)secondEndOfRecord;
                            dataLength++;
                            fileInputEorEof = 0;
                        }
                        else
                        {
                            break; /* flush the send buffer */
                        }
                    }
                    if (tm_16bit_bits_not_set( conFlags,
                                               TM_FTPF_DATA_READ_DONE ) )
/* If not done reading from a file, copy file input into temporary buffer */
                    {
                        if (   tm_16bit_one_bit_set( conFlags,
                                                     TM_FTPF_DATA_DIR) )
                        {
/* Directory listing */
                            retCode = tfFSGetNextDirEntry ( userDataPtr,
                                                            fileDirPtr,
                                                            tempBufPtr,
                                                            TM_TEMP_BUF_SIZE);
                            if (retCode > 0)
                            {
                                fileInputEorEof = TM_RECORD_EOR;
                            }
                        }
                        else
                        {
/* File read */
                            retCode = tfFSReadFileRecord ( userDataPtr,
                                                           fileDirPtr,
                                                           tempBufPtr,
                                                           TM_TEMP_BUF_SIZE,
                                                           &fileInputEorEof );
                        }

                        if ((retCode == 0) && (fileInputEorEof == 0) )
                        {
/* end of directory, file */
                            conFlags = (tt16Bit)(   conFlags
                                                  | TM_FTPF_DATA_READ_DONE );
                            if (txStru == TM_STRU_RECORD)
                            {
/* For record structure, add EOF */
                                fileInputEorEof =   fileInputEorEof
                                                  | TM_RECORD_EOF;
                                secondEndOfRecord = (tt8Bit)fileInputEorEof;
                            }
                        }
                        if (retCode > 0)
                        {
                            fileInputEndIndex = retCode;
                            retCode = 0;
                        }
                        if (retCode < 0)
                        {
                            break;
                        }
                    }
                }
                while (    ( fileInputProcessIndex != fileInputEndIndex )
                        || ( fileInputEorEof != 0 ) );
            }
            else
            {
/* File transfer with no tranlation */
/*
 * Copy as much data as we can from file in the zero copy send buffer.
 * (no directory read here, since we set the actual type to ascii in that
 *  case.)
 */
                copyDataLength = (int)ftpdConnEntryPtr->fconSendBufferSize;
                copyDataPtr = dataPtr;
                do
                {
                    retCode = tfFSReadFile( userDataPtr,
                                            fileDirPtr,
                                            copyDataPtr,
                                            copyDataLength );
                    if (retCode > 0)
                    {
#ifdef TM_ERROR_CHECKING
                        if (retCode > copyDataLength)
/* Should never happen. Stop reading from the file system */
                        {
                            retCode = -1;
                            break;
                        }
#endif /* TM_ERROR_CHECKING */
                        dataLength += retCode;
                        copyDataLength -= retCode;
                        copyDataPtr += retCode;
                        retCode = 0;
                    }
                    else
                    {
                        if (retCode == 0)
                        {
/* end of directory */
                            conFlags = (tt16Bit)(   conFlags
                                                  | TM_FTPF_DATA_READ_DONE );
                        }
                        break;
                    }
                }
                while ( copyDataLength != 0 );
            }
        }
        while (    (dataMessage != (ttUserMessage)0 )
                && (retCode == 0) );
        if (retCode != TM_ENOERROR)
/* Error on data connection */
        {
            replyPtr = (ttCharPtr)&tlFtpd426[0];
        }
        if (replyPtr != (ttCharPtr)0)
        {
            ftpdConnEntryPtr->fconDataSendMessage = dataMessage;
            errorCode = tfFtpdEndData(ftpdConnEntryPtr, replyPtr);
        }
        else
        {
            ftpdConnEntryPtr->fconFileInputProcessIndex =
                                                      fileInputProcessIndex;
            ftpdConnEntryPtr->fconFileInputEndIndex = fileInputEndIndex;
            ftpdConnEntryPtr->fconFileInputEorEof = (tt8Bit)fileInputEorEof;
            ftpdConnEntryPtr->fconTxState = txState;
            ftpdConnEntryPtr->fconFlags = conFlags;
        }
    }
    return errorCode;
}

/*
 * tfFtpdTimerRemove function description:
 * Remove and reset the idle connection timer, called with the execute
 * lock on to protect fconTimerPtr. Called from either tfFtpdCloseCtrl()
 * when a connection control is being closed, or from the execute loop
 * while processing an FTP connection entry.
 * 1. If the FTP connection entry timer pointer is non null, remove the timer.
 * 2. Reset the timer pointer in the FTP connection entry.
 * Parameters:
 * Value                Meaning
 * timerArgPtr          Pointer to FTPD connection entry
 * Return:
 * No return
 */
static void tfFtpdTimerRemove ( ttVoidPtr timerArgPtr )
{
#define ftpdConnEntryPtr    ((ttFtpdConnEntryPtr)timerArgPtr)
    ttTimerPtr  timerPtr;

    timerPtr = ftpdConnEntryPtr->fconTimerPtr;
    if (timerPtr != TM_TMR_NULL_PTR)
    {
        tm_timer_remove(timerPtr);
        ftpdConnEntryPtr->fconTimerPtr = TM_TMR_NULL_PTR;
    }
#undef ftpdConnEntryPtr
}

/*
 * tfFtpdTimerAdd function description
 * Add an idle timeout timer if appropriate:
 *  1. If data transfer is taking place, add an idle timeout timer on
 *     the data connection only if we are in passive mode waiting for
 *     an incoming connection, or if we are receiving data from the
 *     peer. (In all other cases the TCP itself will
 *     timeout and call us back with a CB_RESET, or CB_SOCKET_ERROR,
 *     call back flag.)
 *  2. Else, if no data transfer is taking place add an idle timeout timer
 *     on the control connection (RFC 1123 requirement).
 * Parameters:
 * ftpdConnEntryPtr   pointer to an FTPD connection entry
 * Return:
 * no return value.
 */
static void tfFtpdTimerAdd ( ttFtpdConnEntryPtr ftpdConnEntryPtr )
{
    tt32Bit        timeout;
    int            timerType;
    tt16Bit        conFlags;
    ttGenericUnion timerParm1;

    if (ftpdConnEntryPtr->fconTimerPtr == (ttTimerPtr)0)
    {
        timeout = tm_context(tvFtpdIdleTimeout);
        conFlags = ftpdConnEntryPtr->fconFlags;
        if ( tm_16bit_one_bit_set( conFlags,
                                   TM_FTPF_DATA_TRANSFER ) )
        {
/*
 * Note that there is usually no need for a timeout timer on the
 * data connection, since the TCP protocol can timeout on retransmission,
 * except when we want to accept a new connection (in data passive mode),
 * or we are receiving data from the peer.
 */
            if ( tm_16bit_one_bit_set( conFlags,
                                       TM_FTPF_DATA_TIMER ) )
            {
/*
 * Waiting for an incoming data connection, or we are receiving data
 * from the peer.
 */
                timerType = TM_FTP_DATA_SOCKET;
                if (    tm_16bit_one_bit_set( conFlags, TM_FTPF_DATA_PASV)
                     && tm_16bit_bits_not_set(conFlags, TM_FTPF_DATA_CON) )
                {
/*
 * Use 2 times the default connection TCP time out for a not yet connected
 * passive connection.
 */
                   timeout = 2 * tm_context(tvTcpConnTime);
                }
            }
            else
            {
/* Otherwise no need for a timer */
                timerType = TM_FTP_NO_SOCKET;
            }
        }
        else
        {
            timerType = TM_FTP_CTRL_SOCKET;
        }
        if (timerType >= 0)
/* TM_FTP_CTRL_SOCKET, or TM_FTP_DATA_SOCKET */
        {
            ftpdConnEntryPtr->fconTimerType = (tt8Bit)timerType;
            timerParm1.genVoidParmPtr = (ttVoidPtr)ftpdConnEntryPtr;
            ftpdConnEntryPtr->fconTimerPtr =
                              tfTimerAdd( tfFtpdIdleTimeout,
                                          timerParm1,
                                          timerParm1, /* unused */
                                          timeout,
                                          0 );
        }
    }
}

/*
 * tfStringToUpper function description:
 * Convert string from lower case to upper case up to maxLength
 * Parameters
 * Value            Meaning
 * stringCharPtr    Pointer to a string
 * maxLength        maximum number of bytes to process in the string
 * Return:
 * length           number of bytes converted from lower case to upper case
 */
static int tfStringToUpper (char TM_FAR * stringCharPtr, int maxLength)
{
    int     length;
    char    asciiChar;

    length = 0;
    while ( (*stringCharPtr != '\0') && (maxLength > 0) )
    {
        asciiChar = *stringCharPtr;
        if (tm_islower(asciiChar))
        {
            asciiChar = (char)tm_toupper(asciiChar);
            *stringCharPtr = asciiChar;
        }
        if (!tm_isupper(asciiChar))
        {
            break;
        }
        stringCharPtr++;
        maxLength--;
        length++;
    }
    return length;
}

/*
 * tfFtpdUser function description:
 * Execute the FTP USER command. Called from tfFtpdScanCmd().
 * 1. We call the file system specific check on the user.
 * 2. If successful, copy the user name in our temporary buffer.
 * 3. Return appropriate reply code string (530 for failure, 330 for success)
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdUser ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr replyPtr;
    int       retCode;

    replyPtr = (ttCharPtr)&tlFtpd530[0];
    retCode = tfFSUserAllowed(ftpdCmdLinePtr);
    if (retCode == 0)
    {
        retCode = (int)tm_strlen(ftpdCmdLinePtr);
        if (retCode < TM_TEMP_BUF_SIZE)
        {
            tm_strcpy(&ftpdConnEntryPtr->fconTempBuf[0], ftpdCmdLinePtr);
            ftpdConnEntryPtr->fconFlags = (tt16Bit)
                                            (   ftpdConnEntryPtr->fconFlags
                                              | TM_FTPF_USER_NAME_IN );
            replyPtr = (ttCharPtr)&tlFtpd331[0];
        }
    }
    return replyPtr;
}

/*
 * tfFtpdPass function description:
 * Execute the FTP PASSword command. Called from tfFtpdScanCmd().
 * 1. Call the file system specific user login function.
 * 2. If successful store the user data pointer in our FTP connection entry
 *    and set the user logged in flag.
 * 3. Return appropriate reply code string (530 for failure, 230 for success)
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdPass ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    ttVoidPtr userDataPtr;
    ttCharPtr replyPtr;

    if ( tm_16bit_one_bit_set( ftpdConnEntryPtr->fconFlags,
                               TM_FTPF_USER_NAME_IN ) )
    {
        userDataPtr = tfFSUserLogin(
                                 (ttCharPtr)&ftpdConnEntryPtr->fconTempBuf[0],
                                 ftpdCmdLinePtr );
        if ( userDataPtr != (ttVoidPtr)0 )
        {
            ftpdConnEntryPtr->fconUserDataPtr = userDataPtr;
/*
 * Login successful, no account command required (otherwise 332
 * should be returned, and TM_FTPF_USER_LOGGED_IN set in tfFtpdAcct on
 * successful acount)
 */
            ftpdConnEntryPtr->fconFlags = (tt16Bit)
                    (ftpdConnEntryPtr->fconFlags | TM_FTPF_USER_LOGGED_IN);
            replyPtr = (ttCharPtr)&tlFtpd230[0];
        }
        else
        {
            replyPtr = (ttCharPtr)&tlFtpd530[0];
        }
/* If password failed, user has to re-login again */
        tm_16bit_clr_bit(ftpdConnEntryPtr->fconFlags, TM_FTPF_USER_NAME_IN);
    }
    else
    {
        replyPtr = (ttCharPtr)&tlFtpd503[0];
    }
    return replyPtr;
}

/*
 * tfFtpdAcct function description:
 * Execute the FTP ACCT command. Called from tfFtpdScanCmd().
 * This command is superfluous on embedded systems. (see tfFtpdPass above)
 * We just return a 202 reply code string.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdAcct ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    TM_UNREF_IN_ARG(ftpdConnEntryPtr);
    TM_UNREF_IN_ARG(ftpdCmdLinePtr);
    return (ttCharPtr)&tlFtpd202[0];
}

/*
 * tfFtpdCwd function description:
 * Execute the FTP CWD (or XCWD) command. Called from tfFtpdScanCmd().
 * 1. Call the File system specific change working directory.
 * 2. Return a 250 reply code string on success, 550 on failure.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdCwd ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                             ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr   replyPtr;
    int         retCode;

    retCode = tfFSChangeDir(ftpdConnEntryPtr->fconUserDataPtr,
                            ftpdCmdLinePtr);
    if (retCode == 0)
    {
        replyPtr = (ttCharPtr)&tlFtpd250[0];
    }
    else
    {
        replyPtr = (ttCharPtr)&tlFtpd550[0];
    }
    return replyPtr;
}

/*
 * tfFtpdCdup function description:
 * Execute the FTP CDUP command. Called from tfFtpdScanCmd().
 * 1. Call the File system specific change working directory to parent
 *    directory.
 * 2. Return a 250 reply code string on success, 550 on failure.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdCdup ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr           replyPtr;
    int                 retCode;

    TM_UNREF_IN_ARG(ftpdCmdLinePtr);
    retCode = tfFSChangeParentDir(ftpdConnEntryPtr->fconUserDataPtr);
    if (retCode == 0)
    {
        replyPtr = (ttCharPtr)&tlFtpd250[0];
    }
    else
    {
        replyPtr = (ttCharPtr)&tlFtpd550[0];
    }
    return replyPtr;
}

/*
 * tfFtpdQuit function description:
 * Execute the FTP QUIT command. Called from tfFtpdScanCmd().
 * User FTP wants to quit, we set the errorCode so that the FTPD execute
 * loop closes the connection. We set the errorCode to TM_ESHUTDOWN, to
 * indicate to the close connection function that a 221 reply code string
 * has already been sent (in tfFtpdScanCmd), and return the 221 reply
 * code string to tfFtpdScanCmd.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdQuit ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
/*
 * Close the control connection, and indicate to the ftpdCloseCtrl routine
 * that we have already sent a 221 (see below).
 */
    TM_UNREF_IN_ARG(ftpdCmdLinePtr);
    ftpdConnEntryPtr->fconErrorCode = TM_ESHUTDOWN;
/* Send a 221 (good bye FTP server message) */
    return (ttCharPtr)&tlFtpd221[0];
}

/*
 * tfFtpdSmnt function description:
 * Execute the FTP SMNT command. Called from tfFtpdScanCmd().
 * 1. Call the File system specific structure mount.
 * 2. Return a 250 reply code string on success, 550 on failure.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdSmnt ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    int       retCode;
    ttCharPtr replyPtr;

    retCode = tfFSStructureMount( ftpdConnEntryPtr->fconUserDataPtr,
                                  ftpdCmdLinePtr);
    if (retCode == 0)
    {
        replyPtr = (ttCharPtr)&tlFtpd250[0];
    }
    else
    {
        replyPtr = (ttCharPtr)&tlFtpd550[0];
    }
    return replyPtr;
}

/*
 * tfFtpdRein function description:
 * Execute the FTP REIN command. Called from tfFtpdScanCmd().
 * Reinitialize.
 * 1. Close the data connection if any.
 * 2. logout the USER.
 * 3. flush all I/O and account information. Because of our architecture,
 *    when this command is executed, any data transfer has already finished,
 *    so we do not have to worry about waiting for a prior data transfer to
 *    finish. All parameters are reset to the default settings and the
 *    control connection is left open.
 * 4. Return a 220 reply code string.
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdRein ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    tt16Bit conFlags;

    TM_UNREF_IN_ARG(ftpdCmdLinePtr);

/* close data connection if any */
    (void)tfFtpdCloseData(ftpdConnEntryPtr);
    conFlags = (tt16Bit)ftpdConnEntryPtr->fconFlags;
#ifdef TM_USE_FTPD_SSL
    if (tm_16bit_one_bit_set(ftpdConnEntryPtr->fconSslFlags, 
                             TM_FTPD_SSL_CTRL_ENCRYPTED))
    {
/* Set flag to disable command encryption after return */
        ftpdConnEntryPtr->fconSslFlags |= TM_FTPD_SSL_CHANGE_CTRL;
    }
#endif /* TM_USE_FTPD_SSL */
/* Logout user if any */
    if (tm_16bit_one_bit_set(conFlags, TM_FTPF_USER_LOGGED_IN))
    {
        tfFSUserLogout(ftpdConnEntryPtr->fconUserDataPtr);
    }
/*
 * Reset all connection flags (login, passive, data transfer, etc..) except
 * TM_FTPF_COMMAND_BUFFERED, in case we have read ahead the next FTP command
 */
    conFlags = (tt16Bit)(conFlags & (tt16Bit)TM_FTPF_COMMAND_BUFFERED);
    ftpdConnEntryPtr->fconFlags = (tt16Bit)conFlags;
    tfFtpdDataDefault(ftpdConnEntryPtr);
    return (ttCharPtr)&tlFtpd220[0];
}

/*
 * tfFtpdPort function description:
 * Execute the FTP PORT command. Called from tfFtpdScanCmd().
 *
 * Execute the FTP PORT command: "PORT h1,h2,h3,h4,p1,p2
 * The port command gives the data destination IP address and IP port
 * that the server will connect to for the data connection.
 * ftpdCmdLinePtr argument is "h1,h2,h3,h4,p1,p2".
 * 1. Map h1,h2,h3,h4 to the internet address
 * 1.1 First convert h1,h2,h3,h4, to h1.h2.h3.h4\0
 * 1.2 Convert h1.h2.h3.h4 to an IP address in network byte order
 * 2. Map p1,p2 to the internet port
 * 3. If successful,
 * 3.1 Close the data socket if it is open, since we are about to open a
 *     new data connection.
 * 3.2 Store the peer data IP address given in the port command in
 *     fconPortClientDataAddress.
 * 3.3 Set the Ftp connection flag to indicate that we have received a
 *     port command (so that we use the fconPortClientDataAddress when
 *     we try and connect on the data connection).
 *
 * 4. If successful, return the 200 reply code string, otherwise 501.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdPort ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr                        replyPtr;
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPV6
    struct sockaddr_storage TM_FAR * sockAddrStorPtr;
    ttSockAddrIn                     tempSockAddr;
#endif /* TM_USE_IPV6 */
    ttSockAddrInPtr                  sockAddrPtr;
    ttSockAddrInPtr                  clientSockAddrPtr;
    int                              cmdLineIndex;
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV4
#ifdef TM_USE_IPV6
    if (ftpdConnEntryPtr->fconSrvrDataAddress.ss_family == AF_INET6)
    {
        sockAddrPtr = &tempSockAddr; 
        tm_bzero(sockAddrPtr, sizeof(ttSockAddrIn));
    }
    else
#endif /* !TM_USE_IPV6 */
    {
        sockAddrPtr = (ttSockAddrInPtr)
            &ftpdConnEntryPtr->fconPortClientDataAddress.addr.ipv4;
    }
/* Convert string h1,h2,h3,h4,p1,p2 to sockaddr */
    cmdLineIndex = tfFtpHxPxFormatToSockAddr( ftpdCmdLinePtr,
                                              sockAddrPtr);
    if (    (cmdLineIndex != 0)
         && (sockAddrPtr->ssin_port != 0) )
    {
        if (tm_ip_zero(sockAddrPtr->ssin_addr.s_addr))
        {
/*
 * If remote did not set its IP address in the port command, use the
 * one we already stored.
 */
            clientSockAddrPtr = (ttSockAddrInPtr)
                &ftpdConnEntryPtr->fconClientCtrlAddress.addr.ipv4;
            tm_ip_copy(
                clientSockAddrPtr->ssin_addr.s_addr,
                sockAddrPtr->ssin_addr.s_addr);
        }
/*
 * Close the data socket if it is open, since we are about to open a
 * new data connection.
 */
        (void)tfFtpdCloseData(ftpdConnEntryPtr);
#ifdef TM_USE_IPV6
        if (ftpdConnEntryPtr->fconSrvrDataAddress.ss_family == AF_INET6)
        {
            sockAddrStorPtr = &ftpdConnEntryPtr->fconPortClientDataAddress;
            tm_bzero(sockAddrStorPtr, sizeof(struct sockaddr_storage));
            sockAddrStorPtr->ss_family = AF_INET6;
            sockAddrStorPtr->ss_len = sizeof(struct sockaddr_storage);
            sockAddrStorPtr->ss_port = sockAddrPtr->ssin_port;
            tm_6_addr_to_ipv4_mapped(sockAddrPtr->ssin_addr.s_addr,
                                     &sockAddrStorPtr->addr.ipv6.sin6_addr);
        }
        else
#endif /* TM_USE_IPV6 */
        {
            sockAddrPtr->ssin_family = AF_INET;
        }
/* We have an active port socket */
        ftpdConnEntryPtr->fconFlags =
                (tt16Bit) (ftpdConnEntryPtr->fconFlags | TM_FTPF_DATA_PORT);
/* No error */
        replyPtr = (ttCharPtr)&tlFtpd200[0];
    }
    else
    {
/* Syntax error */
        replyPtr = (ttCharPtr)&tlFtpd501[0];
    }
#else /* !TM_USE_IPV4 */
/* Avoid compiler warnings */
    TM_UNREF_IN_ARG(ftpdConnEntryPtr);
    TM_UNREF_IN_ARG(ftpdCmdLinePtr);

/* Return "protocol not supported" if IPv4 is not enabled. */
    replyPtr = (ttCharPtr)&tlFtpd522_v6[0];
#endif /* TM_USE_IPV4 */

    return replyPtr;
}

/*
 * tfFtpdPasv function description:
 * Execute the FTP PASV command. Called from tfFtpdScanCmd().
 * The PASV command requests the server-DTP to listen on a data port
 * (which is not its default data port) and to wait for a connection
 * rather than initiate one upon receipt of a transfer command.
 * The response to this command includes the host and port address this
 * server is listening on.
 * 1. We open a socket, register an accept CB, and listen for one incoming
 *    connection. Get our sock name so that we can copy it in our 227 reply
 *    string.
 * 2. If successful, copy reply code 227 and copy our passive Ip address
 *    and port in our temporary buffer. Set reply code string to the
 *    temporary buffer. Set a flag to indicate that we need to add
 *    a timeout timer on the data connection at the end of the execute loop
 *    when the data transfer command comes from the remote. (This will allow
 *    us to time out on the data connection in case the client never tries
 *    and connects to our passive listening data socket.)
 * 3. If we fail we return a 425 reply code string although it is not in
 *    the RFC 959 list for PASV command since a 425 reply makes the most
 *    sense.
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdPasv ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr                    replyPtr;
    ttSockAddrPtrUnion           sockAddressPtr;
    int                          socketDesc;
    int                          length;
    int                          retCode;

    TM_UNREF_IN_ARG(ftpdCmdLinePtr);
/* Close current data connection if any */
    (void)tfFtpdCloseData(ftpdConnEntryPtr);
/* Open a socket for the data connection that we need to listen on */
    sockAddressPtr.sockNgPtr = &ftpdConnEntryPtr->fconSrvrPasvDataAddress;
    replyPtr = (ttCharPtr)&tlFtpd425[0]; /* assume failure */
    retCode = TM_SOCKET_ERROR;

#ifdef TM_USE_IPV6
    if (    (sockAddressPtr.sockNgPtr->ss_family == AF_INET6)
         && (!(tm_6_addr_is_ipv4_mapped(
             &(sockAddressPtr.sockNgPtr->addr.ipv6.sin6_addr)))))
    {
/* The PASV command is not supported for IPv6 */
        goto FTPD_PASV_EXIT;
    }
#endif /* TM_USE_IPV6 */

    socketDesc = socket(sockAddressPtr.sockNgPtr->ss_family,
                        SOCK_STREAM, IP_PROTOTCP);

    if (socketDesc != TM_SOCKET_ERROR)
    {
        ftpdConnEntryPtr->fconDesc[TM_FTP_DATA_SOCKET] = socketDesc;
        ftpdConnEntryPtr->fconFlags = (tt16Bit)
                 (ftpdConnEntryPtr->fconFlags | TM_FTPF_DATA_PASV);
        length = sockAddressPtr.sockPtr->sa_len;
/* Peek any port */
        sockAddressPtr.sockNgPtr->ss_port = 0;
/* Bind to our source address, pick any port */
        retCode = bind( socketDesc,
                        sockAddressPtr.sockPtr,
                        length);
        if (retCode == TM_ENOERROR)
        {
#ifdef TM_USE_FTPD_SSL            
            if( tm_16bit_one_bit_set( 
                       ftpdConnEntryPtr->fconSslFlags,
                       TM_FTPD_SSL_ENCRYPT_DATA) )
            {
/* Set the socket options on the data socket */
                retCode = tfFtpdSslSetSockOpt(
                              ftpdConnEntryPtr->fconDesc[TM_FTP_DATA_SOCKET] );
            }
            if (retCode == TM_ENOERROR)
#endif /* TM_USE_FTPD_SSL */
            {
                retCode = tfRegisterSocketCBParam( socketDesc,
                                                   tfFtpdSignal,
                                                   ftpdConnEntryPtr,
                                                   TM_CB_ACCEPT );
                if (retCode == TM_ENOERROR)
                {
                    (void)tfBlockingState(socketDesc, TM_BLOCKING_OFF);
                    retCode = listen(socketDesc, 1);
                    if (retCode == TM_ENOERROR)
                    {
/* Get our port */
                        retCode = getsockname( socketDesc,
                                               sockAddressPtr.sockPtr,
                                               &length);
                    }
                    if (retCode == TM_ENOERROR)
                    {
/*
 * Convert from socket address to h1,h1,h3,h4,p1,p2 string.
 * Use common routine with FTP client.
 */
/* where to start inserting IP address, IP port */
#define TM_PRE_PASV_BUF 27
                        replyPtr =
                                (ttCharPtr)&ftpdConnEntryPtr->fconTempBuf[0];
                        tm_bcopy("227 Entering Passive Mode (",
                                  replyPtr, TM_PRE_PASV_BUF);
#ifdef TM_USE_IPV6
                        if (sockAddressPtr.sockNgPtr->ss_family == AF_INET6)
                        {
                            sockAddressPtr.sockTInPtr->ssin_addr.s_addr =
                                tm_6_ip_mapped_6to4(&(sockAddressPtr.
                                             sockNgPtr->addr.ipv6.sin6_addr));
                        }
#endif /* TM_USE_IPV6 */
                        length = tfFtpSockAddrToHxPxFormat(
                                                sockAddressPtr.sockTInPtr,
                                                &replyPtr[TM_PRE_PASV_BUF]);
                        length += TM_PRE_PASV_BUF;
                        tm_strcpy(&replyPtr[length], ").\r\n");
/*
 * Setting this flag, will cause us to add a time out timer for accepting
 * the connection, when the data transfer command comes from the remote.
 */
                        ftpdConnEntryPtr->fconFlags = (tt16Bit)
                                            (   ftpdConnEntryPtr->fconFlags
                                              | TM_FTPF_DATA_TIMER );
                    }
                }
            }
        }
        if (retCode != TM_ENOERROR)
        {
            (void)tfFtpdCloseData(ftpdConnEntryPtr);
        }
    }

#ifdef TM_USE_IPV6
FTPD_PASV_EXIT:
#endif /* TM_USE_IPV6 */
    return replyPtr;
}

/*
 * tfFtpdType function description:
 * Execute the FTP TYPE command. Called from tfFtpdScanCmd().
 *
 *   The following codes are assigned for type:
 *
 *                        \    /
 *              A - ASCII |    | N - Non-print
 *                        |-><-| T - Telnet format effectors
 *              E - EBCDIC|    | C - Carriage Control (ASA)
 *                        /    \
 *              I - Image
 *
 *              L <byte size> - Local byte Byte size
 *
 * We accept the following subset:
 *                TYPE A
 *                TYPE A N or TYPE AN
 *                TYPE I
 *                TYPE L 8
 * 1. Check syntax.
 * 2. Save the type in our FTP control connection type field. For type
 *    Ascii, if the file system uses CR, LF for end of line characters,
 *    add a special flag to the type, so that we can use speedier binary type
 *    file transfer algorithm for ASCII data transfer.
 * 3. If a syntax error occurs we return the 501 reply code string
 *    If we do not accept the specified type we return the 504 reply code
 *    string. We return the 200 reply code string on success.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdType ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr   replyPtr;
    int         cmdLineIndex;
    int         txType;
    tt16Bit     currentShort;
    tt8Bit      digitCh;
    char        txTypeChar;

    txTypeChar = *ftpdCmdLinePtr;
    replyPtr = (ttCharPtr)&tlFtpd200[0];
    txType = -1;
    cmdLineIndex = 1;
/* point to second argument if any */
    while (ftpdCmdLinePtr[cmdLineIndex] == ' ')
    {
        cmdLineIndex++;
    }
    if (ftpdCmdLinePtr[cmdLineIndex] == '\0')
    {
/* No second argument */
        ftpdCmdLinePtr = (ttCharPtr)0;
    }
    switch (txTypeChar)
    {
        case 'A':
        case 'E':
            if (    (ftpdCmdLinePtr == (ttCharPtr)0)
                 || ( ftpdCmdLinePtr[cmdLineIndex] == 'N') )
            {
                if (txTypeChar == 'A')
/* Accept Type A or A N */
                {
/*
 * Add special flag if file system use CR, LF for end
 * of line, so that we can use speedier binary type file transfer algorithm
 * for ASCII data transfer.
 */
                    txType = TM_TYPE_ASCII | tm_context(tvFtpdFsTypeFlag);
                }
                else
                {
/* Type E, E N not implemented */
                    replyPtr = (ttCharPtr)&tlFtpd504[0];
                }
            }
            else
            {
                if (    (ftpdCmdLinePtr[cmdLineIndex] == 'T')
                     || (ftpdCmdLinePtr[cmdLineIndex] == 'C') )
                {
/* Type A T, A C, E T, or E C not implemented */
                    replyPtr = (ttCharPtr)&tlFtpd504[0];
                }
                else
                {
/*
 * Type A, or Type E followed by any character different from T, N, C,
 * syntax error
 */
                    replyPtr = (ttCharPtr)&tlFtpd501[0];
                }
            }
            break;
        case 'I':
            if (ftpdCmdLinePtr == (ttCharPtr)0)
            {
/* Accept TYPE I */
                txType = TM_TYPE_BINARY;
            }
            else
            {
/* Type I followed by any other character, syntax error */
                replyPtr = (ttCharPtr)&tlFtpd501[0];
            }
            break;
        case 'L':
            if (ftpdCmdLinePtr != (ttCharPtr)0)
            {
/* TYPE L with a second parameter */
                txType = TM_TYPE_LOCAL;
                currentShort = (tt16Bit)0;
                while ( ftpdCmdLinePtr[cmdLineIndex] != '\0' )
                {
                    digitCh = (tt8Bit)ftpdCmdLinePtr[cmdLineIndex];
                    if (tm_isdigit(digitCh))
                    {
                        digitCh = (tt8Bit)(digitCh - '0');
                        currentShort =   (tt16Bit)
                                  (   (tt16Bit)digitCh
                                    + (tt16Bit)((tt16Bit)currentShort * 10) );
                    }
                    else
                    {
/* Second argument not an ascii integer, syntax error */
                        replyPtr = (ttCharPtr)&tlFtpd501[0];
                        txType = -1;
                        break;
                    }
                    cmdLineIndex++;
                }
                if (txType == TM_TYPE_LOCAL)
                {
                    if (currentShort != 8)
                    {
/* Unimplemented for byte size != 8 */
                        replyPtr = (ttCharPtr)&tlFtpd504[0];
                        txType = -1;
                    }
                }
            }
            else
            {
/* TYPE L without a second parameter, syntax error */
                replyPtr = (ttCharPtr)&tlFtpd501[0];
            }
            break;
        default:
/*
 * Type followed by any character different from A, E, I, L, syntax error
 */
            replyPtr = (ttCharPtr)&tlFtpd501[0];
            break;
    }
    if (txType >= 0)
    {
        ftpdConnEntryPtr->fconTxType = (tt8Bit)txType;
    }
    return replyPtr;
}

/*
 * tfFtpdStru function description:
 * Execute the FTP STRU command. Called from tfFtpdScanCmd().
 * The following codes are assigned for structure:
 *  F - file
 *  R - record
 *  P - page
 * We accept F, and R
 * 1. Check syntax
 * 2. Save the structure in our FTP connection structure field.
 * 3. Return the 501 reply code string if a syntax error occurred, 504
 *    for page, or 200 on success.
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdStru ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr   replyPtr;
    char        savChar;

    savChar = *ftpdCmdLinePtr;
    replyPtr = (ttCharPtr)&tlFtpd200[0];
    if (savChar == 'F')
    {
/* transfer structure: file */
        ftpdConnEntryPtr->fconTxStru = TM_STRU_STREAM;
    }
    else
    {
        if (savChar == 'R')
        {
/* transfer structure: record */
            ftpdConnEntryPtr->fconTxStru = TM_STRU_RECORD;
        }
        else
        {
            if (savChar == 'P')
            {
/* not implemented for that parameter */
                replyPtr = (ttCharPtr)&tlFtpd504[0];
            }
            else
            {
                replyPtr = (ttCharPtr)&tlFtpd501[0];
            }
        }
    }
    return replyPtr;
}

/*
 * tfFtpdMode function description:
 * Execute the FTP MODE command. Called from tfFtpdScanCmd().
 * The following modes are possible for the MODE command
 *              S - Stream
 *              B - Block
 *              C - Compressed
 * We only accept S - Stream which is the default.
 * 1. Check syntax
 * 2. Return 200 on success, or 504 for B, or C, or 501 if there is a
 *    syntax error.
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdMode ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr replyPtr;
    char      modeChar;

    TM_UNREF_IN_ARG(ftpdConnEntryPtr);
    modeChar = *ftpdCmdLinePtr;
    if (modeChar == 'S')
    {
        replyPtr = (ttCharPtr)&tlFtpd200[0]; /* OK */
    }
    else
    {
        if (modeChar == 'B' || modeChar == 'C')
        {
/* not implemented for that parameter */
            replyPtr = (ttCharPtr)&tlFtpd504[0];
        }
        else
        {
            replyPtr = (ttCharPtr)&tlFtpd501[0];
        }
    }
    return replyPtr;
}


/*
 * tfFtpdRetr function description:
 * Execute the FTP RETR command. Called from tfFtpdScanCmd().
 * 1. Call the common function tfFtpdOpenFileData with a File system read
 *    flag.
 * 2. Return the reply code string returned by tfFtpdOpenFileData.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdRetr ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr replyPtr;

    replyPtr = tfFtpdOpenFileData( ftpdConnEntryPtr,
                                   ftpdCmdLinePtr,
                                   TM_FS_READ);
    return replyPtr;
}


/*
 * tfFtpdStor function description:
 * Execute the FTP STOR command. Called from tfFtpdScanCmd().
 * 1. Call the common function tfFtpdOpenFileData with a File system write
 *    flag.
 * 2. Return the reply code string returned by tfFtpdOpenFileData.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdStor ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr replyPtr;

    replyPtr = tfFtpdOpenFileData( ftpdConnEntryPtr,
                                   ftpdCmdLinePtr,
                                   TM_FS_WRITE );
    return replyPtr;
}

/*
 * tfFtpdStou function description:
 * Execute the FTP STOU command. Called from tfFtpdScanCmd().
 * 1. Get a unique file name from the file system into our temporary
 *    buffer.
 * 2.1 On success, prepend 150 FILE: or 125 FILE: before the unique file name,
 *     as required by RFC 1123.
 * 2.2 Call the common function tfFtpdOpenFileData with a File system write
 *     flag.
 * 2.3 Return the reply code string returned by tfFtpdOpenFileData which
 *     should be our temporary buffer if successful.
 *     (tfFtpdOpenFileData() calls tfFtpdOpenData() which knows about the
 *     special reply string in our temporary buffer for STOU.)
 * 3. On failure, return the 550 reply code string.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdStou ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr bufferPtr;
    ttCharPtr replyPtr;
    int       length;
#define TM_PRE_STOU_BUF   10
#define TM_POST_STOU_BUF  3

    TM_UNREF_IN_ARG(ftpdCmdLinePtr);
/* First retrieve unique file name */
    bufferPtr = (ttCharPtr)&ftpdConnEntryPtr->fconTempBuf[0],
    length =  tfFSGetUniqueFileName(
                          ftpdConnEntryPtr->fconUserDataPtr,
                          &bufferPtr[TM_PRE_STOU_BUF],
                          TM_TEMP_BUF_SIZE-TM_PRE_STOU_BUF-TM_POST_STOU_BUF);
    if (length > 0)
    {
        tm_bcopy("150 FILE: ", bufferPtr, TM_PRE_STOU_BUF);
        tm_strcpy(&bufferPtr[length+TM_PRE_STOU_BUF], "\r\n");
        ftpdConnEntryPtr->fconStouLength = length + TM_PRE_STOU_BUF;
        bufferPtr[ftpdConnEntryPtr->fconStouLength] = '\0';
        if ( tm_16bit_one_bit_set( ftpdConnEntryPtr->fconFlags,
                                   TM_FTPF_DATA_CON ) )
        {
/* Send a 125, if we are already connected */
            bufferPtr[1] = '2';
            bufferPtr[2] = '5';
        }
        replyPtr = tfFtpdOpenFileData( ftpdConnEntryPtr,
                                       &bufferPtr[TM_PRE_STOU_BUF],
                                       TM_FS_WRITE );
    }
    else
    {
        replyPtr = (ttCharPtr)&tlFtpd550[0];
    }
    return replyPtr;
}

/*
 * tfFtpdAppe function description:
 * Execute the FTP APPE command. Called from tfFtpdScanCmd().
 * 1. Call the common function tfFtpdOpenFileData with a File system append
 *    flag.
 * 2. Return the reply code string returned by tfFtpdOpenFileData.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdAppe ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr replyPtr;

    replyPtr = tfFtpdOpenFileData( ftpdConnEntryPtr,
                                   ftpdCmdLinePtr,
                                   TM_FS_APPEND);
    return replyPtr;
}



/*
 * tfFtpdRnfr function description:
 * Execute the FTP RNFR command. Called from tfFtpdScanCmd().
 * 1. Set the TM_FTPF_RNFR_RECEIVED flag
 * 2. copy the file name argument in our temporary buffer.
 * 3. Return 350 on success, 550 otherwise.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdRnfr ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr replyPtr;
    int       length;

    length = (int)tm_strlen(ftpdCmdLinePtr);
    if (length < TM_TEMP_BUF_SIZE)
    {
        ftpdConnEntryPtr->fconFlags = (tt16Bit)
                                      (   ftpdConnEntryPtr->fconFlags
                                        | TM_FTPF_RNFR_RECEIVED );
        tm_strcpy( (ttCharPtr)&ftpdConnEntryPtr->fconTempBuf[0],
                   ftpdCmdLinePtr );
        replyPtr = (ttCharPtr)&tlFtpd350[0];
    }
    else
    {
        replyPtr = (ttCharPtr)&tlFtpd550[0];
    }
    return replyPtr;
}

/*
 * tfFtpdRnto function description:
 * Execute the FTP RNTO command. Called from tfFtpdScanCmd().
 * 1. If a rename from has been received:
 * 1.1 Call the file system specific function to rename the file name
 *     that was copied in temporary buffer to the new file name in the
 *     argument.
 * 1.2. Return 250 on success, 550 on failure.
 * 2. Else (no rename from has been received), return a 503.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdRnto ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr replyPtr;
    int       retCode;
    tt16Bit   conFlags;

    conFlags = ftpdConnEntryPtr->fconFlags;
    if ( tm_16bit_one_bit_set( conFlags,
                               TM_FTPF_RNFR_RECEIVED ) )
/* If a rename from command has been received */
    {
        retCode = tfFSRenameFile(ftpdConnEntryPtr->fconUserDataPtr,
                                 (ttCharPtr)&ftpdConnEntryPtr->fconTempBuf[0],
                                 ftpdCmdLinePtr);
/* Reset the RNFR received flag */
        tm_16bit_clr_bit( conFlags, TM_FTPF_RNFR_RECEIVED);
        ftpdConnEntryPtr->fconFlags = conFlags;
        if (retCode == 0)
        {
            replyPtr = (ttCharPtr)&tlFtpd250[0];
        }
        else
        {
            replyPtr = (ttCharPtr)&tlFtpd550[0];
        }
    }
    else
/* No rename from preceded the rename to, send a 503 reply code string */
    {
        replyPtr = (ttCharPtr)&tlFtpd503[0];
    }
    return replyPtr;
}

/*
 * tfFtpdAbor function description:
 * Execute the FTP ABOR command. Called from tfFtpdScanCmd().
 *
 * Abort the previous command.
 * From RFC 959: there are 2 cases:
 * 1. If data transfer has already completed,
 *    then just close the data socket (if not already done so), and
 *    return 226.
 * 2. If data transfer is in progress, abort the transfer, close the
 *    data socket, send a 426, and then return 226.
 * Implementation:
 * 1. Call tfFtpdEndData() with a reply code 426. This will close
 *    the data connection if any and send the 426 reply code string
 *    if the data transfer bit was on (transfer in progress).
 *    Set control socket error code if any.
 * 2. return the 226 reply code string.
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdAbor ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    int       errorCode;

    TM_UNREF_IN_ARG(ftpdCmdLinePtr);
/* Forget any stacked command */
    tm_16bit_clr_bit( ftpdConnEntryPtr->fconFlags,
                      (TM_FTPF_USER_NAME_IN | TM_FTPF_RNFR_RECEIVED) );
/*
 * send a 426 to indicate that the transfer was aborted (only if data transfer
 * is taking place (tfFtpdEndData will only send the reply code string if
 * the data transfer bit is on)), and close the data socket if it is open.
 * This will also abort the transfer, since we transfer data in non-blocking
 * mode.
 */
    errorCode = tfFtpdEndData(ftpdConnEntryPtr, (ttCharPtr)&tlFtpd426[0]);
    if (errorCode != TM_ENOERROR)
    {
/* Close the control connection if we fail to send on it */
        ftpdConnEntryPtr->fconErrorCode = errorCode;
    }
    return (ttCharPtr)&tlFtpd226[0]; /* send ABORT successfully implemented */
}

/*
 * tfFtpdDele function description:
 * Execute the FTP DELE command. Called from tfFtpdScanCmd().
 * 1. Call the File system specific delete file function.
 * 2. Return a 250 reply code string on success, 550 on failure.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdDele ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr replyPtr;
    int       retCode;

    retCode = tfFSDeleteFile(ftpdConnEntryPtr->fconUserDataPtr,
                             ftpdCmdLinePtr);
    if (retCode == 0)
    {
        replyPtr = (ttCharPtr)&tlFtpd250[0];
    }
    else
    {
        replyPtr = (ttCharPtr)&tlFtpd550[0];
    }
    return replyPtr;
}

/*
 * tfFtpdRmd function description:
 * Execute the FTP RMD (or XRMD) command. Called from tfFtpdScanCmd().
 * 1. Call the File system specific remove directory.
 * 2. Return a 250 reply code string on success, 550 on failure.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdRmd ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                             ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr replyPtr;
    int       retCode;

    retCode = tfFSRemoveDir(ftpdConnEntryPtr->fconUserDataPtr,
                            ftpdCmdLinePtr);
    if (retCode == 0)
    {
        replyPtr = (ttCharPtr)&tlFtpd250[0];
    }
    else
    {
        replyPtr = (ttCharPtr)&tlFtpd550[0];
    }
    return replyPtr;
}

/*
 * tfFtpdMkd function description:
 * Execute the FTP MKD (or XMKD) command. Called from tfFtpdScanCmd().
 * 1. Call the File system specific make directory.
 * 2. Return a 250 reply code string on success, 550 on failure.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdMkd ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                             ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr bufferPtr;
    ttCharPtr replyPtr;
    int       retCode;
#define TM_PRE_MKD_BUF   5
#define TM_POST_MKD_BUF  13

    bufferPtr = (ttCharPtr)&ftpdConnEntryPtr->fconTempBuf[0];
    retCode = tfFSMakeDir(ftpdConnEntryPtr->fconUserDataPtr,
                          ftpdCmdLinePtr,
                          &bufferPtr[TM_PRE_MKD_BUF],
                          TM_TEMP_BUF_SIZE-TM_PRE_MKD_BUF-TM_POST_MKD_BUF);
    if (    (retCode > 0)
         && (retCode <= (TM_TEMP_BUF_SIZE-TM_PRE_MKD_BUF-TM_POST_MKD_BUF)) )
    {
        tm_bcopy("257 \"", bufferPtr, TM_PRE_MKD_BUF);
        tm_strcpy(&bufferPtr[retCode+TM_PRE_MKD_BUF], "\" created.\r\n");
        replyPtr = bufferPtr;
    }
    else
    {
        replyPtr = (ttCharPtr)&tlFtpd550[0];
    }
    return replyPtr;
}

/*
 * tfFtpdPwd function description:
 * Execute the FTP PWD (or XPWD) command. Called from tfFtpdScanCmd().
 * 1. Call the File system specific print working directory to store the
 *    working directory in our temporary buffer.
 * 2.1 On success, prepend the 257 reply code in our temporary buffer, and
 *     return temporary buffer.
 * 2.2 On failure, return 550 reply code string.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdPwd ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                             ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr bufferPtr;
    ttCharPtr replyPtr;
    int       retCode;
#define TM_PRE_PWD_BUF   5
#define TM_POST_PWD_BUF  26

    TM_UNREF_IN_ARG(ftpdCmdLinePtr);
    bufferPtr = (ttCharPtr)&ftpdConnEntryPtr->fconTempBuf[0];
    retCode = tfFSGetWorkingDir(
                             ftpdConnEntryPtr->fconUserDataPtr,
                             &bufferPtr[TM_PRE_PWD_BUF],
                             TM_TEMP_BUF_SIZE-TM_PRE_PWD_BUF-TM_POST_PWD_BUF);
    if (    (retCode > 0)
         && (retCode <= (TM_TEMP_BUF_SIZE-TM_PRE_PWD_BUF-TM_POST_PWD_BUF)) )
    {
        tm_bcopy("257 \"", bufferPtr, TM_PRE_PWD_BUF);
        tm_strcpy( &bufferPtr[retCode+TM_PRE_PWD_BUF],
                   "\" is current directory.\r\n");
        replyPtr = bufferPtr;
    }
    else
    {
        replyPtr = (ttCharPtr)&tlFtpd550[0];
    }
    return replyPtr;
}

/*
 * tfFtpdList function description:
 * Execute the FTP LIST command. Called from tfFtpdScanCmd().
 * 1. Call the common function tfFtpdOpenDirData with a File system long
 *    directory flag.
 * 2. Return the reply code string returned by tfFtpdOpenDirData.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdList ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr replyPtr;

    replyPtr = tfFtpdOpenDirData( ftpdConnEntryPtr,
                                  ftpdCmdLinePtr,
                                  TM_DIR_LONG );
    return replyPtr;
}

/*
 * tfFtpdNlst function description:
 * Execute the FTP NLST command. Called from tfFtpdScanCmd().
 * 1. Call the common function tfFtpdOpenDirData with a File system short
 *    directory flag.
 * 2. Return the reply code string returned by tfFtpdOpenDirData.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdNlst ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr replyPtr;

    replyPtr = tfFtpdOpenDirData( ftpdConnEntryPtr,
                                  ftpdCmdLinePtr,
                                  TM_DIR_SHORT );
    return replyPtr;
}

/*
 * tfFtpdSite function description:
 * Execute the FTP SITE command. Called from tfFtpdScanCmd().
 *
 * Site specific command. Not implemented, superfluous at this site.
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdSite ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    TM_UNREF_IN_ARG(ftpdConnEntryPtr);
    TM_UNREF_IN_ARG(ftpdCmdLinePtr);
    return (ttCharPtr)&tlFtpd202[0];
}

/*
 * tfFtpdSyst function description:
 * Execute the FTP SYST command. Called from tfFtpdScanCmd().
 * 1. Call the file system specific system function to store the system
 *    in our temporary buffer.
 * 2. On success prepend the 215 reply code to our temporary buffer,
 *    and return the temporary buffer for our reply code string.
 * 3. On failure, return the 502 reply code string.
 *
 * system
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdSyst ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr   replyPtr;
    ttCharPtr   bufferPtr;
    int         length;
#define TM_PRE_SYST_BUF   4 /* 2,1,5,<SP> */
#define TM_POST_SYST_BUF  3 /* \r,\n,\0 */

    TM_UNREF_IN_ARG(ftpdCmdLinePtr);
    bufferPtr = (ttCharPtr)&ftpdConnEntryPtr->fconTempBuf[0];
    tm_bcopy("215 ", bufferPtr, TM_PRE_SYST_BUF);
    length = tfFSSystem(&bufferPtr[TM_PRE_SYST_BUF],
                        TM_TEMP_BUF_SIZE-TM_PRE_SYST_BUF-TM_POST_SYST_BUF);
    if (length <= 0)
    {
        replyPtr = (ttCharPtr)&tlFtpd502[0];
    }
    else
    {
        tm_strcpy(&bufferPtr[length+TM_PRE_SYST_BUF], "\r\n");
        replyPtr = bufferPtr;
    }
    return replyPtr;
}

/*
 * tfFtpdEprt function description:
 * Execute the FTP EPRT (extended PORT)  command. Called from tfFtpdScanCmd().
 *
 * 1. Get to start of parameter string and determine the delimiter
 *    character.
 * 2. Get protcol number (IPv4, IPv6, etc).
 * 3. If protocol family is not supported, send 229 response code, including
 *    a list of the protocols supported.
 * 4. Convert the ASCII network address into binary.
 * 5. Find the TCP port parameter.
 * 6. Convert the ASCII TCP port parameter into binary.
 * 7. Set the protocol family, length, etc.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdEprt ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    struct sockaddr_storage TM_FAR * addrPtr;
    ttVoidPtr     rawAddrPtr;
    ttUser32Bit   protoNumber;
    ttCharPtr     replyPtr;
    ttCharPtr     endCharPtr;
    tt16Bit       portNumber;
    tt8Bit        goodProtoNum;
    tt8Bit        addrFamily;
    char          delimChar;
    int           cmdLineIndex;
    int           endCmdLineIndex;
    int           errorCode;
    int           nextDelimIndex;
#ifdef TM_USE_IPV6
    int           scopeDelimIndex;
#endif /* TM_USE_IPV6 */

/* Initialize to avoid compiler warnings. */
    rawAddrPtr = (ttVoidPtr) 0;

    cmdLineIndex    = 0;
    endCmdLineIndex = (int)tm_strlen(ftpdCmdLinePtr);
    addrPtr = &ftpdConnEntryPtr->fconPortClientDataAddress;
    tm_bzero(addrPtr, sizeof(struct sockaddr_storage));
/* 7. Set default protocol family, length */
    addrPtr->ss_family = AF_INET6;
    addrPtr->ss_len = sizeof(struct sockaddr_storage);
    addrFamily = AF_INET6;

/*
 * 1. Get to start of parameter string and determine the delimiter
 *    character.
 */

    while (    (ftpdCmdLinePtr[cmdLineIndex] == ' ')
            && (cmdLineIndex < endCmdLineIndex) )
    {
        cmdLineIndex++;
    }
    delimChar = ftpdCmdLinePtr[cmdLineIndex];
    cmdLineIndex++;

/* 2. Get protcol number (IPv4, IPv6, etc). */
    protoNumber = tm_strtoul( &ftpdCmdLinePtr[cmdLineIndex],
                              &endCharPtr,
                              10 );

/* Verify that strtoul() converted all of the way to the next delimiter. */
    if (*endCharPtr != delimChar)
    {
        replyPtr = (ttCharPtr)&tlFtpd501[0];
        goto exitFtpdEprt;
    }

/*
 * 3. If protocol family is not supported, send 229 response code, including
 *    a list of the protocols supported.
 */
    goodProtoNum = TM_8BIT_ZERO;

#ifdef TM_USE_IPV6
    if (     (protoNumber == TM_FTP_EPRT_INET6)
#ifdef TM_USE_IPV4
         && !(ftpdConnEntryPtr->fconUserFlags & TM_FTPD_IPV4_ONLY)
#endif /* TM_USE_IPV4 */
       )
    {
        rawAddrPtr = (ttVoidPtr)&addrPtr->addr.ipv6.sin6_addr;
        goodProtoNum = TM_8BIT_YES;
    }
#endif /* TM_USE_IPV6 */

#ifdef TM_USE_IPV4
    if (protoNumber == TM_FTP_EPRT_INET)
    {
        addrFamily = AF_INET; /* client address is IPv4 */
#ifdef TM_USE_IPV6
        if (ftpdConnEntryPtr->fconSrvrDataAddress.ss_family == AF_INET6)
        {
/* We need to open an IPv6 data socket */
            addrPtr->addr.ipv6.sin6_addr.s6LAddr[2] = TM_IPV4_3_MAP;
            rawAddrPtr = (ttVoidPtr)&(addrPtr->addr.ipv6.sin6_addr.s6LAddr[3]);
        }
        else
#endif /* TM_USE_IPV6 */
        {
/* We need to open an IPv4 data socket */
            addrPtr->ss_family = AF_INET;
            rawAddrPtr = (ttVoidPtr)&addrPtr->addr.ipv4.sin_addr;
        }
        goodProtoNum = TM_8BIT_YES;
    }
#endif /* TM_USE_IPV4 */

    if (goodProtoNum != TM_8BIT_YES)
    {
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
        if (ftpdConnEntryPtr->fconUserFlags & TM_FTPD_IPV4_ONLY)
        {
/* This flag should only be set if IPv4 is enabled (TM_USE_IPV4) */
            replyPtr = (ttCharPtr) &tlFtpd522_v4[0];
        }
        else
#endif /* TM_USE_IPV4 */
        {
#ifdef TM_USE_IPV4
/* Both IPv4 and IPv6 are enabled, and the user hasn't restricted us to IPv4. */
            replyPtr = (ttCharPtr) &tlFtpd522_v4v6[0];
#else /* !TM_USE_IPV4 */
/* Just IPv6 is enabled. */
            replyPtr = (ttCharPtr) &tlFtpd522_v6[0];
#endif /* TM_USE_IPV4 */
        }
#else /* !TM_USE_IPV6 */
/* Just IPv4 is enabled */
        replyPtr = (ttCharPtr) &tlFtpd522_v4[0];
#endif /* TM_USE_IPV6 */
    }
    else
    {

/* 4. Convert the ASCII network address into binary. */
        while (    (ftpdCmdLinePtr[cmdLineIndex] != delimChar)
                && (cmdLineIndex < endCmdLineIndex) )
        {
            cmdLineIndex++;
        }
        cmdLineIndex++;

/* Find the location of the next delimiter, because we need to set it to
   zero so the string will be properly formatted for inet_pton(). */
        nextDelimIndex = cmdLineIndex;
        while (    (ftpdCmdLinePtr[nextDelimIndex] != delimChar)
                && (nextDelimIndex < endCmdLineIndex) )
        {
            nextDelimIndex++;
        }
        ftpdCmdLinePtr[nextDelimIndex] = '\0';

/*
 * There is no standard way for handling scoped addresses with the EPRT
 * command.  Linux (and others) handle it by sending the interface name
 * along with the address (e.g. fe80::1%eth1).  This doesn't mean anything
 * to us, so we strip it off before passing it to inet_pton.  Then, we use
 * the scope ID of the control connection as the scope ID for the new
 * data connection.
 */
#ifdef TM_USE_IPV6
        if (addrFamily == AF_INET6)
        {
            scopeDelimIndex = cmdLineIndex;
            while (    (ftpdCmdLinePtr[scopeDelimIndex] != '%')
                    && (scopeDelimIndex < endCmdLineIndex) )
            {
                scopeDelimIndex++;
            }
            ftpdCmdLinePtr[scopeDelimIndex] = '\0';

            addrPtr->addr.ipv6.sin6_scope_id =
                ftpdConnEntryPtr->fconClientCtrlAddress.addr.ipv6.sin6_scope_id;
        }
#endif /* TM_USE_IPV6 */

        errorCode =
            inet_pton( addrFamily, &ftpdCmdLinePtr[cmdLineIndex], rawAddrPtr );

        if (errorCode != 1)
        {
            replyPtr = (ttCharPtr)&tlFtpd501[0];
            goto exitFtpdEprt;
        }

/* 5. Find the TCP port parameter (this was already found above). */
        cmdLineIndex = nextDelimIndex + 1;

/* 6. Convert the ASCII TCP port parameter into binary. */
        portNumber = (tt16Bit) tm_strtoul( &ftpdCmdLinePtr[cmdLineIndex],
                                           &endCharPtr,
                                           10 );

/* Verify that strtoul() converted all of the way to the next delimiter. */
        if (*endCharPtr != delimChar)
        {
            replyPtr = (ttCharPtr)&tlFtpd501[0];
            goto exitFtpdEprt;
        }

       tm_htons( portNumber, addrPtr->ss_port );
/*
 * Close the data socket if it is open, since we are about to open a
 * new data connection.
 */
        (void)tfFtpdCloseData(ftpdConnEntryPtr);

/* We have an active port socket */
        ftpdConnEntryPtr->fconFlags |= TM_FTPF_DATA_PORT;
/* No error */
        replyPtr = (ttCharPtr)&tlFtpd200[0];
    }

exitFtpdEprt:
    return replyPtr;
}


/*
 * tfFtpdEpsv function description:
 * Execute the FTP EPSV (extended PASV) command. Called from tfFtpdScanCmd().
 * The EPSV command requests the server-DTP to listen on a data port
 * (which is not its default data port) and to wait for a connection
 * rather than initiate one upon receipt of a transfer command.
 * The response to this command includes the port address this server is
 * listening on.
 * 1. We open a socket, register an accept CB, and listen for one incoming
 *    connection. Get our sock name so that we can copy it in our 229 reply
 *    string.
 * 2. If successful, copy reply code 229 and copy our passive port number
 *    into our temporary buffer. Set reply code string to the temporary buffer.
 *    Set a flag to indicate that we need to add a timeout timer on the data
 *    connection at the end of the execute loop when the data transfer command
 *    comes from the remote. (This will allow us to time out on the data
 *    connection in case the client never tries and connects to our passive
 *    listening data socket.)
 * 3. If we fail we return a 425 reply code string although it is not in
 *    the RFC 959 list for the command since a 425 reply makes the most
 *    sense.

 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdEpsv ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr                    replyPtr;
    ttSockAddrPtrUnion           sockAddressPtr;
    tt16Bit                      portNum;
    int                          socketDesc;
    int                          length;
    int                          retCode;
    ttCharPtr                    setBufPtr;

    ftpdCmdLinePtr=ftpdCmdLinePtr;
/* Close current data connection if any */
    (void)tfFtpdCloseData(ftpdConnEntryPtr);
/* Open a socket for the data connection that we need to listen on */
    sockAddressPtr.sockNgPtr = &ftpdConnEntryPtr->fconSrvrPasvDataAddress;
    socketDesc = socket(sockAddressPtr.sockNgPtr->ss_family,
                        SOCK_STREAM, IP_PROTOTCP);

    replyPtr = (ttCharPtr)&tlFtpd425[0]; /* assume failure */
    retCode = TM_SOCKET_ERROR;
    if (socketDesc != TM_SOCKET_ERROR)
    {
        ftpdConnEntryPtr->fconDesc[TM_FTP_DATA_SOCKET] = socketDesc;
        ftpdConnEntryPtr->fconFlags = (tt16Bit)
                 (ftpdConnEntryPtr->fconFlags | TM_FTPF_DATA_PASV);
        length = sockAddressPtr.sockPtr->sa_len;
/* Pick any port */
        sockAddressPtr.sockNgPtr->ss_port = 0;
/* Bind to our source address, pick any port */
        retCode = bind( socketDesc,
                        sockAddressPtr.sockPtr,
                        length);
        if (retCode == TM_ENOERROR)
        {
#ifdef TM_USE_FTPD_SSL            
            if(    (retCode == TM_ENOERROR) 
                && tm_16bit_one_bit_set( 
                       ftpdConnEntryPtr->fconSslFlags,
                       TM_FTPD_SSL_ENCRYPT_DATA) )
            {
/* Set the socket options on the data socket */
                retCode = tfFtpdSslSetSockOpt(
                              ftpdConnEntryPtr->fconDesc[TM_FTP_DATA_SOCKET] );
            }
            if (retCode == TM_ENOERROR)
#endif /* TM_USE_FTPD_SSL */
            {
                retCode = tfRegisterSocketCBParam( socketDesc,
                                                   tfFtpdSignal,
                                                   ftpdConnEntryPtr,
                                                   TM_CB_ACCEPT );
                if (retCode == TM_ENOERROR)
                {
                    (void)tfBlockingState(socketDesc, TM_BLOCKING_OFF);
                    retCode = listen(socketDesc, 1);
                    if (retCode == TM_ENOERROR)
                    {
/* Get our port */
                        retCode = getsockname( socketDesc,
                                               sockAddressPtr.sockPtr,
                                               &length);

                    }
                    if (retCode == TM_ENOERROR)
                    {
/* where to start inserting port number */
#define TM_PRE_EPSV_BUF 39
                        replyPtr =
                                (ttCharPtr)&ftpdConnEntryPtr->fconTempBuf[0];
                        tm_bcopy("229 Entering Extended Passive Mode (|||",
                                  replyPtr, TM_PRE_EPSV_BUF);
                        length = TM_PRE_EPSV_BUF;

/* Copy the port number into the reply. */
                        tm_ntohs(sockAddressPtr.sockNgPtr->ss_port, portNum);
                        setBufPtr = &replyPtr[length];
                        setBufPtr = tfUlongDecimalToString(setBufPtr, portNum);
                        length = (int) (setBufPtr - replyPtr);
                        tm_strcpy(&replyPtr[length], "|).\r\n");
/*
 * Setting this flag, will cause us to add a time out timer for accepting
 * the connection, when the data transfer command comes from the remote.
 */
                        ftpdConnEntryPtr->fconFlags = (tt16Bit)
                                            (   ftpdConnEntryPtr->fconFlags
                                              | TM_FTPF_DATA_TIMER );
                    }
                }
            }
        }

        if (retCode != TM_ENOERROR)
        {
            (void)tfFtpdCloseData(ftpdConnEntryPtr);
        }
    }
    return replyPtr;
}


/*
 * tfFtpdNoop function description:
 * Execute the FTP NOOP command. Called from tfFtpdScanCmd().
 * 1. Return 200 reply code string.
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdNoop ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    TM_UNREF_IN_ARG(ftpdConnEntryPtr);
    TM_UNREF_IN_ARG(ftpdCmdLinePtr);
    return (ttCharPtr)&tlFtpd200[0];
}

#ifdef TM_USE_FTPD_SSL
/*
 * tfFtpdAuth function description:
 * Execute the FTPS AUTH command. Called from tfFtpdScanCmd().
 *
 * 1. If SSL is not permitted AUTH will return a 431.
 * 2. If the supported securites does not match the given 
 *    security, reply with a 504.
 * 3. Start the SSL session on both the control and data channels.
 * 4. Set socket options for both channels.
 * 5. Initialize SSL flags.
 * 6. Return a 234 if negotiation is correct.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdAuth ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr    replyPtr;
    int          entries;
    int          retCode;
    int          i;

    if(tm_context(tvFtpdSslCtrlPermissions) != TM_FTPD_SSL_NOT_SUPPORTED)
    {
        retCode=-1;

        tm_strcpy(&ftpdConnEntryPtr->fconTempBuf[0], ftpdCmdLinePtr);

/* Test for a supported security type */
        entries = sizeof(tlSecuritiesArray)/sizeof(tlSecuritiesArray[0]);
        for(i=0; i<entries; i++)
        {
            if(tm_strcmp(ftpdCmdLinePtr, tlSecuritiesArray[i])==TM_STRCMP_EQUAL)
            {
                retCode=0;
                break;
            }
        }
          
        if(retCode==0)
        {
/* Set the response */
            replyPtr=(ttCharPtr)&tlFtpd234[0];

            if( tm_context(tvFtpdSslSessionId ) < 0 )
            {
/* We were never given a valid SSL session ID, return a 431 reply code */
                replyPtr=(ttCharPtr)&tlFtpd431[0];
            }
            else
            {
/* Set the socket options to allow SSL on the control channel's tcp socket */
                tfFtpdSslSetSockOpt(
                        ftpdConnEntryPtr->fconDesc[TM_FTP_CTRL_SOCKET] );
            
/* Tell exitscan to bind SSL to the control channel after sending the reply*/
                if ( tm_16bit_bits_not_set(ftpdConnEntryPtr->fconSslFlags,
                                           TM_FTPD_SSL_CTRL_ENCRYPTED) )
                {
                    ftpdConnEntryPtr->fconSslFlags |= TM_FTPD_SSL_CHANGE_CTRL;
                }
                ftpdConnEntryPtr->fconSslFlags |= TM_FTPD_SSL_SESSION_ACTIVE;
            }
        }
        else
        {
            replyPtr=(ttCharPtr)&tlFtpd504[0];
        }
    } 
    else
    {
        replyPtr=(ttCharPtr)&tlFtpd431[0];
    }
   return replyPtr;
}

/*
 * tfFtpdAdat function description:
 * Execute the FTP ADAT command. Called from tfFtpdScanCmd().
 * ADAT not implemented. Reply with 502.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdAdat ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    TM_UNREF_IN_ARG(ftpdConnEntryPtr);
    TM_UNREF_IN_ARG(ftpdCmdLinePtr);
    return (ttCharPtr)&tlFtpd502[0];
}

/*
 * tfFtpdProt function description:
 * Execute the FTP PROT command. Called from tfFtpdScanCmd().
 * 1. Verify if SSL is active, if not return a 534 reply code.
 * 2. If input is equal to C (Clear), turn off data encryption
 *    and reply with a 200 reply code.
 * 3. If input is equal to P (Private), turn on data encryption
 *    and reply with a 200 reply code.
 * 4. If the input is anything else reply with a 534 reply code.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdProt ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr   replyPtr;

    replyPtr = (ttCharPtr)0;

    if(tm_16bit_all_bits_set(ftpdConnEntryPtr->fconSslFlags, 
                        TM_FTPD_SSL_SESSION_ACTIVE))
    {
        if( tm_strcmp(ftpdCmdLinePtr, "C") == TM_STRCMP_EQUAL )
        {
/* Don't use encryption on any new data sockets */
            ftpdConnEntryPtr->fconSslFlags &= ~TM_FTPD_SSL_ENCRYPT_DATA;

/* Destroy the SSL layer on the currently open data socket */
            if(tm_16bit_one_bit_set(ftpdConnEntryPtr->fconSslFlags, 
                                    TM_FTPD_SSL_DATA_ENCRYPTED))
            {
                tfSslConnectUserClose( 
                        ftpdConnEntryPtr->fconDesc[TM_FTP_DATA_SOCKET] );

                    ftpdConnEntryPtr->fconSslFlags = 
                            (tt16Bit)(ftpdConnEntryPtr->fconSslFlags & 
                            ~TM_FTPD_SSL_DATA_ENCRYPTED);
            }

            replyPtr = (ttCharPtr)&tlFtpd200[0];
        }
        else if( tm_strcmp(ftpdCmdLinePtr, "P") == TM_STRCMP_EQUAL )
        {
/* Flag SSL encryption on the data channel */
            ftpdConnEntryPtr->fconSslFlags |= TM_FTPD_SSL_ENCRYPT_DATA;
            replyPtr = (ttCharPtr)&tlFtpd200[0];

        }
        else if (    tm_strcmp(ftpdCmdLinePtr, "S") == TM_STRCMP_EQUAL
                  || tm_strcmp(ftpdCmdLinePtr, "E") == TM_STRCMP_EQUAL )
        {
            replyPtr = (ttCharPtr)&tlFtpd536[0];
        }
        else
        {
            replyPtr = (ttCharPtr)&tlFtpd534[0];
        }
    }
    else
    {
        replyPtr = (ttCharPtr)&tlFtpd534[0];
    }
    return replyPtr;
}

/*
 * tfFtpdPbsz function description:
 * Execute the FTP PBSZ command. The only valid input for SSL/TLS is 0.
 * Called from tfFtpdScanCmd().
 * 1. Verify if SSL is active, if not, return a 503 reply code.
 * 2. Turn input into a float (double integer for size checking)
 * 3. If input is larger than 2^32-1, reutrn a 503.
 * 4. If the input is smaller than 2^32-1, but larger than the server maximum,
 *    (0, as dictated by RFC 2228) set protection buffer to the default size, 0 
 *    (from RFC 2228) and return a 501 reply code with "PBSZ=default size".
 * 5. If the input is smaller than the server maximum, set the protection
 *    buffer to this number and return a 200 reply code.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command 
 */
static ttCharPtr tfFtpdPbsz ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr        replyPtr;
    tt32Bit          bufferSize;

    replyPtr = (ttCharPtr)0;

    if(tm_16bit_one_bit_set(ftpdConnEntryPtr->fconSslFlags, 
                            TM_FTPD_SSL_SESSION_ACTIVE))
    {
        bufferSize = tm_strtoul(ftpdCmdLinePtr, (ttCharPtrPtr)0, 10);
        
        if(bufferSize == TM_32BIT_MAX)
        {
/* User input is too large */
            replyPtr = (ttCharPtr)&tlFtpd501[0];
        }
        else
        {
            if(bufferSize > TM_FTPD_SSL_PBSZ)
            {
/*
 * Since the requested size is larger than zero,
 * Reply with a 200 (success), but add on PBSZ=0.
 */             
                replyPtr = (ttCharPtr)&tlFtpdPbsz200[0];
            }
            else
            {
                replyPtr = (ttCharPtr)&tlFtpd200[0];
            }
/* Set the protection buffer to 0 */
            ftpdConnEntryPtr->fconSslProtBuffer = TM_FTPD_SSL_PBSZ;
        }
    }
    else
    {
        replyPtr = (ttCharPtr)&tlFtpd503[0];
    }
    return replyPtr;
}

/*
 * tfFtpdCcc function description:
 * Execute the FTP CCC command. Called from tfFtpdScanCmd().
 * 1. If SSL is inactive, return a 534 reply code.
 * 2. If SSL is active and the command channel is encrypted, turn off
 *    command channel encryption and return a 200 reply code. If the 
 *    command channel is not turned on, return a 200 reply code.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdCcc ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                             ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr        replyPtr;

    TM_UNREF_IN_ARG(ftpdCmdLinePtr);

    if(tm_16bit_one_bit_set(ftpdConnEntryPtr->fconSslFlags, 
                            TM_FTPD_SSL_SESSION_ACTIVE))
    {
        if(tm_16bit_one_bit_set(ftpdConnEntryPtr->fconSslFlags, 
                                TM_FTPD_SSL_CTRL_ENCRYPTED))
        {
/* Set flag to disable command encryption after return */
            ftpdConnEntryPtr->fconSslFlags |= TM_FTPD_SSL_CHANGE_CTRL;
        }
        replyPtr = (ttCharPtr)&tlFtpd200[0];
    }
    else 
    {
    
        replyPtr =(ttCharPtr)&tlFtpd534[0];
    }
    return replyPtr;
}

/*
 * tfFtpdEnc function description:
 * Execute the FTP ENC command. Called from tfFtpdScanCmd(). In RFC 2228
 * it is specified that this command take a '64bit encoded "private" message
 * produced by a security mechanism specific integrity and confidentiality
 * procedure.' However, we were unable to locate such a message for SSL.
 * The function accepts a parameter, but it is not required or used.
 *
 * 1. If SSL is inactive, return a 534 reply code.
 * 2. If SSL is active and the command channel is not encrypted, turn on
 *    command channel encryption and return a 200 reply code. If the 
 *    command channel is turned on, return a 200 reply code.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdEnc ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                             ttCharPtr          ftpdCmdLinePtr )
{
    ttCharPtr        replyPtr;

    TM_UNREF_IN_ARG(ftpdCmdLinePtr);
    if(tm_16bit_one_bit_set(ftpdConnEntryPtr->fconSslFlags, 
                            TM_FTPD_SSL_SESSION_ACTIVE))
    {
        if(tm_16bit_bits_not_set(ftpdConnEntryPtr->fconSslFlags, 
                                 TM_FTPD_SSL_CTRL_ENCRYPTED))
        {
/* Set flag to enable command encryption, after response */
            ftpdConnEntryPtr->fconSslFlags |= TM_FTPD_SSL_CHANGE_CTRL;
        }
/* The control channel is encrypted. Respond with 200 */
        replyPtr = (ttCharPtr)&tlFtpd200[0];
    }
    else
    {
        replyPtr = (ttCharPtr)&tlFtpd534[0];
    }
    return replyPtr;
}

/*
 * tfFtpdFeat function description:
 * Execute the FTP FEAT command. Called from tfFtpdScanCmd().
 * 1. Initialize buffer with RFC 2389 starting string.
 * 2. Add AUTH and the supported securities.
 * 3. Add remaining features.
 * 4. Add end line from RFC 2389.
 * 5. Return features from buffer.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */

static ttCharPtr tfFtpdFeat ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    
    ttCharPtr           bufferPtr;
    ttCharPtr           introString;
    ttCharPtr           endString;
    ttCharPtr           returnPtr;
    int                 i;
    int                 securitiesEntries;
    int                 currentLength;
    int                 nextLength;
    int                 introStringLength;
    int                 endStringLength;

    TM_UNREF_IN_ARG(ftpdCmdLinePtr);

    currentLength = 0;

/* This introduction line is set in RFC 2389 */
    introString = "211-Features:\r\n";
    introStringLength = (int)tm_strlen( introString );

/* This end line is set in RFC 2389 */
    endString   = "211 End\r\n";
    endStringLength = (int)tm_strlen( endString );


/* Get the number of entries from the securities list */
    securitiesEntries = sizeof(tlSecuritiesArray)/sizeof(tlSecuritiesArray[0]);
/* 
 * Allocate Buffer for return reply. Malloc allows dynamic sizing. 
 * This is freed in tfFtpdScanCmd().
 */
    ftpdConnEntryPtr->fconFeatBuffer  = (ttCharPtr) tm_get_raw_buffer( 
            (TM_NOOP * 7)                  /* 
                                            * TM_NOOP is also the number of 
                                            * commands we will list for FEAT.
                                            * 7 is the RFC-defined space (1), 
                                            * command length (4), and /r/n (2).
                                            */
            + introStringLength            /* Length of introduction line */
            + ( securitiesEntries          /* Number of security entries */
                * ( 6                      /* Length of " AUTH " */
                + TM_MAX_SECURITIES_LENGTH /* Maximum securities length */
                + 2 ) )                    /* /r/n */
            + endStringLength              /* Length of end line */
            + 1                            /* Null Terminator */
            ); 
    if (!(ftpdConnEntryPtr->fconFeatBuffer))
    {
        returnPtr = (ttCharPtr) &tlFtpd550[0];
/* Return a "550 xxx  requested action not taken." message */
        goto FTPD_FEAT_EXIT;
    }
    bufferPtr = ftpdConnEntryPtr->fconFeatBuffer;
/* Add the introduction string */
    tm_memcpy(&bufferPtr[currentLength], introString, introStringLength);
    currentLength += introStringLength;
  

/* This loop puts the securities into the list */
    for(i = 0; i < securitiesEntries; i++)
    {
        nextLength = 6;
        tm_memcpy(&bufferPtr[currentLength], " AUTH ", nextLength);
        currentLength += nextLength;

        nextLength = (int)tm_strlen(tlSecuritiesArray[i]);
        tm_memcpy(&bufferPtr[currentLength], tlSecuritiesArray[i], nextLength);
        currentLength += nextLength;
    
        nextLength = 2;
        tm_memcpy(&bufferPtr[currentLength], "\r\n", nextLength);
        currentLength += nextLength;
    }

/* This loop puts the features into the list */
    for(i = 0; i <= TM_NOOP; i++)
    {
#ifdef TM_AUTH
        if( i != TM_AUTH ) 
        {
#endif /* TM_AUTH */
/* Add a space for correct RFC formatting purposes. */
            bufferPtr[currentLength] = ' ';
            ++currentLength;
/* Add command name to features list.*/
            tm_memcpy(&bufferPtr[currentLength],
                ( tlFtpdCmdArray[i].fdcu ).ftpuName,
                4);
            currentLength += 4;

            tm_memcpy(&bufferPtr[currentLength], "\r\n", 2);
            currentLength += 2;
#ifdef TM_AUTH
        }
#endif /* TM_AUTH */
    }
/* Add the end string to the reply and null terminate */
    tm_memcpy( &bufferPtr[currentLength], endString, endStringLength );
    currentLength += endStringLength;
    bufferPtr[currentLength]='\0';
    returnPtr = ftpdConnEntryPtr->fconFeatBuffer;

FTPD_FEAT_EXIT:
    return returnPtr;
}
#endif /* TM_USE_FTPD_SSL*/

#ifndef TM_NOSUP
/* Non supported functions */

/*
 * tfFtpdAllo function description:
 * Execute the FTP ALLO command. Called from tfFtpdScanCmd().
 *
 * ALLO <SP> <decimal-integer>  [<SP> R <SP> <decimal-integer>] <CRLF>
 * Ignore ALLO command for the time being. Reply with command unimplemented,
 * unnecessary at this site.
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdAllo ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    TM_UNREF_IN_ARG(ftpdConnEntryPtr);
    TM_UNREF_IN_ARG(ftpdCmdLinePtr);
    return (ttCharPtr)&tlFtpd202[0];
}

/*
 * tfFtpdHelp function description:
 * Execute the FTP HELP command. Called from tfFtpdScanCmd().
 * This command is only useful to a human user, and most users use
 * the ftp client help. It is only really useful to get help on
 * site specific commands, like SITE, or TYPE with a site specific
 * file type. Since we did not implement any site specific commands,
 * we do not really need to implement this for the time being.
 *
 * 1. Not implemented. Return 502 reply code string.
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdHelp ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    TM_UNREF_IN_ARG(ftpdConnEntryPtr);
    TM_UNREF_IN_ARG(ftpdCmdLinePtr);
    return (ttCharPtr)&tlFtpd502[0];
}

/*
 * tfFtpdRest function description:
 * Execute the FTP REST command. Called from tfFtpdScanCmd().
 * Restart not implemented. Reply with 202.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdRest ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    TM_UNREF_IN_ARG(ftpdConnEntryPtr);
    TM_UNREF_IN_ARG(ftpdCmdLinePtr);
    return (ttCharPtr)&tlFtpd502[0];
}

/*
 * tfFtpdStat function description:
 * Execute the FTP STAT command. Called from tfFtpdScanCmd().
 *
 * 1. Not implemented. Return the 502 reply code string.
 *
 * Parameters:
 * Value                Meaning
 * ftpdConnEntryPtr     Pointer to FTPD connection entry
 * ftpdCmdLinePtr       pointer to null terminated string containing the FTP
 *                      command argument.
 * Return:
 * replyPtr             reply code string to be sent as a reply to
 *                      the command
 */
static ttCharPtr tfFtpdStat ( ttFtpdConnEntryPtr ftpdConnEntryPtr,
                              ttCharPtr          ftpdCmdLinePtr )
{
    TM_UNREF_IN_ARG(ftpdConnEntryPtr);
    TM_UNREF_IN_ARG(ftpdCmdLinePtr);
    if (*ftpdCmdLinePtr == '\0')
    {
/*
 * Status of the connection command, like type, structure, if data transfer
 * in progress, how many bytes have transferred so far, etc.
 */
    }
    else
    {
/* File Status. Need to use a file system specific call */
        replyPtr = (ttCharPtr)&tlFtpd502[0];
    }
    return (ttCharPtr)&tlFtpd502[0];
}

#endif /* TM_NOSUP */

#ifdef TM_USE_STOP_TRECK
void tfFtpdDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                )
{
    ttFtpdConnEntryPtr ftpdConnEntryPtr;

/* 
 * Cannot call tfFtpdUserStop because we want an immediate shutdown
 * Should be safe because user is supposed to have killed all the tasks.
 */
    tm_call_lock_wait(&tm_context(tvFtpdExecuteLockEntry));
    tm_context_var(tvFtpdShutdown) = 1;
#ifdef TM_USE_FTPD_SSL
/* Reset SSL control and data permssions to non supported */
    tm_context_var(tvFtpdSslCtrlPermissions) = TM_FTPD_SSL_NOT_SUPPORTED;
    tm_context_var(tvFtpdSslDataPermissions) = TM_FTPD_SSL_NOT_SUPPORTED;
#endif /* TM_USE_FTPD_SSL */
    if (tm_context(tvFtpdListenDesc) != TM_SOCKET_ERROR)
    {
        tfFtpdCloseListen();
    }
    tm_context_var(tvFtpdPendingConnections) = 0;
    ftpdConnEntryPtr = (ttFtpdConnEntryPtr)(tm_context_var(tvFtpdConQHead).
                                    servHeadNextQEntryPtr(TM_FTPD_CONN_LIST));
    while (ftpdConnEntryPtr != (ttFtpdConnEntryPtr)
                                    &(tm_context_var(tvFtpdConQHead)) )
    {
        tm_call_unlock(&tm_context_var(tvFtpdExecuteLockEntry));
        tfFtpdCloseCtrl(ftpdConnEntryPtr, TM_ESHUTDOWN);
        tm_call_lock_wait(&tm_context(tvFtpdExecuteLockEntry));
        ftpdConnEntryPtr = (ttFtpdConnEntryPtr)
                                    (tm_context_var(tvFtpdConQHead).
                                    servHeadNextQEntryPtr(TM_FTPD_CONN_LIST));
    }
    tm_call_unlock(&tm_context_var(tvFtpdExecuteLockEntry));
}
#endif /* TM_USE_STOP_TRECK */

#else /* !TM_USE_FTPD */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */


/* To allow link for builds when TM_USE_FTPD is not defined */
int tvFtpdDummy = 0;

#endif /* !TM_USE_FTPD */
