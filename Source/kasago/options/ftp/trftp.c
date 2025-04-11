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
 * Description: FTP client Protocol
 * Filename: trftp.c
 * Author: S.Fujiwara
 * Date Created: 01/13/99
 * $Source: source/trftp.c $
 *
 * Modification History
 * $Revision: 6.0.2.20 $
 * $Date: 2016/05/18 15:53:13JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include        <trsocket.h>
#include        <trmacro.h>

#ifdef TM_USE_FTP
#include        <trtype.h>
#include        <trproto.h>
#if (defined(TM_DEBUG_LOGGING) || defined(TM_ERROR_CHECKING))
#include        <trglobal.h>
#endif /* TM_DEBUG_LOGGING || TM_ERROR_CHECKING */
#include 		"ftpctrl.h"
#ifdef TM_USE_FTP_SSL
#include        <trssl.h>
#endif /* TM_USE_FTP_SSL */

/* Define unreferenced macro for PC-LINT compile error suppression */
#ifdef TM_LINT
LINT_UNREF_MACRO(TM_MODE_BLOCK)
LINT_UNREF_MACRO(TM_MODE_COMPRESSED)
LINT_UNREF_MACRO(TM_FORM_TELNET)
LINT_UNREF_MACRO(TM_FORM_ASA)
LINT_UNREF_MACRO(TM_NVT_NULL)
LINT_UNREF_MACRO(TM_NVT_SPACE)
LINT_UNREF_MACRO(TM_TC_DONT)
LINT_UNREF_MACRO(TM_TC_DO)
LINT_UNREF_MACRO(TM_TC_WONT)
LINT_UNREF_MACRO(TM_TC_WILL)
LINT_UNREF_MACRO(tm_is_reply_1xx)
LINT_UNREF_MACRO(tm_is_reply_3xx)
LINT_UNREF_MACRO(tm_is_reply_4xx)
LINT_UNREF_MACRO(tm_is_reply_5xx)
LINT_UNREF_MACRO(TM_CMD_SMNT)
LINT_UNREF_MACRO(TM_CMD_PASV)
LINT_UNREF_MACRO(TM_CMD_ALLO)
LINT_UNREF_MACRO(TM_CMD_REST)
LINT_UNREF_MACRO(TM_CMD_SITE)
LINT_UNREF_MACRO(TM_FTPS_CONNECT)
#endif /* TM_LINT */


/*
 * This module contains the FTP client implementation, and follows the FTP
 * protocol from RFC 959, and RFC 1123.
 *
 * Features of the FTP client implementation:
 * . Allows the user to operate in blocking or non blocking mode
 * . In blocking mode the FTP client will execute in its own thread. This
 *   is the preferred method on a multi-tasking system.
 *
 * . In non blocking mode each FTP client command will return immediately.
 *   If the command has not finished, then the return code will be
 *   TM_EWOULDBLOCK. When the command returns TM_EWOULDBLOCK, then
 *   the user should call tfFtpExecute() until the return value from
 *   tfFtpExecute() is no longer TM_EWOULDBLOCK, which
 *   indicates that the command has completed. The return value from
 *   tfFtpExecute() gives the status of the command: 0 for success,
 *   TM_EWOULDBLOCK if the command has not completed,
 *   any other value for failure. If the user were to call a
 *   new command before the current one has completed, then the
 *   command will not be executed or queued, and the return code
 *   will be TM_EACCES. The only exception is for an abort command
 *   tfFtpAbor(), a QUIT command tfFtpQuit(), or STAT with no argument
 *   tfFtpStat() command that will be sent right away.
 *
 * 1. First the user calls tfFtpNewSession() to open an FTP session.
 *    The user gets a pointer to a FTP session on success, null pointer
 *    otherwise. This pointer is to be used as first argument of all
 *    following FTP API calls.
 * 2. Next the user could call tfFtpTurnPort() locally to turn on/or
 *    off usage of PORT command during data transfer
 *    Since we set the default to on as recommended by RFC 1123, the
 *    user would probably never use tfFtpTurnPort().
 * 3. Next the user calls tfFtpConnect() to connect to an FTP server.
 * 4. Next the user calls tfFtpLogin() to login on the remote server
 *    (this executes USER, and possibly PASS, ACCT commands).
 * 5. Then the user can call any other FTP command:
 *
 *    CWD command:  tfFtpCwd()
 *    CDUP command: tfFtpCdup()
 *    SMNT command: tfFtpSmnt()
 *    REIN command: tfFtpRein()
 *    QUIT command: tfFtpQuit()
 *
 *    PORT command: tfFtpPort()
 *    TYPE command: tfFtpType()
 *    STRU command: tfFtpStru()
 *
 *    RETR command: tfFtpRetr()
 *    STOR command: tfFtpStor()
 *    STOU command: tfFtpStou()
 *    APPE command: tfFtpAppe()
 *    ALLO command: tfFtpAllo()
 *    RNFR command, and RNTO commands: tfFtpRename()
 *    ABOR command: tfFtpAbor()
 *    DELE command: tfFtpDele()
 *    RMD  command: tfFtpRmd()
 *    MKD  command: tfFtpMkd()
 *    PWD  command: tfFtpPwd()
 *    LIST, and NLST commands: tfFtpDirList()
 *    SITE command: tfFtpSite()
 *    SYST command: tfFtpSyst()
 *    STAT command: tfFtpStat()
 *    HELP command: tfFtpHelp()
 *    NOOP command: tfFtpNoop()
 *
 * 6. When done the user should call tfFtpQuit() to disconnect from
 *    the server.
 * 7. If tfFtpQuit() fails, then
 *    the user can call tfFtpClose() to close the control socket.
 * 8. The user could reuse the allocated FTP session to connect again,
 *    or if there is no longer any need for an FTP session can call
 *    tfFtpFreeSession() to free the session allocated in tfFtpNewSession().
 *
 * Internals of the design:
 * . In blocking mode all opened sockets are kept to their default blocking
 *   mode.
 * . In non blocking mode all opened sockets are set in non blocking mode,
 *   and call backs are registered for events that we would have waited for
 *   if we were blocking. When the user calls tfFtpExecute() we check
 *   for call back events that have occured.
 *
 * In each user FTP API command, the function tfFtpCheckSession() is called
 * to check that the session parameter is valid, that the command is
 * valid for the FTP state (connected, logged in), that the previous
 * command has completed. The only exceptions are for tfFtpTurnPort() since
 * it is executed locally, or for commands that
 * can be sent to the server while it is processing a data transfer command
 * (i.e STAT with no argument, QUIT, or ABORT), in which case the function
 * tfFtpCheckInterSession() is called instead. (tfFtpCheckInterSession() itself
 * calls tfFtpCheckSession(), but allows one extra command to be sent if
 * the current command is a data transfer command.)
 * Then the parameters are
 * checked, some copied to the FTP session if they need to be remembered
 * for later use. Then the common function tfFtpCommand() is called.
 * tfFtpCommand() will format the command, send it on the control connection,
 * and then if no error occurs, call
 * tfFtpRecvCtrl(). tfFtpRecvCtrl() will receive the data on the control
 * connection using a subset of the TELNET NVT transformation. Note
 * that in non blocking mode tfFtpRecvCtrl() will return with a
 * TM_EWOULBLOCK error, and will be called later in the tfFtpExecute()
 * thread if a CB recv has occured. When a complete reply has been
 * received, then the function tfFtpScanReply will be called from
 * tfFtpRecvCtrl().
 * Note that in most of the routines (when no extra paramter is used) a
 * common function tfFtpCheckCommand()
 * is called that calls both tfFtpCheckSession() and tfFtpCommand() and
 * check on a non null parameter if appropriate.
 *
 *
 *
 *
 * Description of the FTP protocol from RFC 959:
 *
 * server-FTP process
 * A process or set of processes which perform the function of
 * file transfer in cooperation with a user-FTP process and,
 * possibly, another server.  The functions consist of a protocol
 * interpreter (PI) and a data transfer process (DTP).
 *
 * server-PI
 * The server protocol interpreter "listens" on Port L for a
 * connection from a user-PI and establishes a control
 * communication connection.  It receives standard FTP commands
 * from the user-PI, sends replies, and governs the server-DTP.
 *
 * server-DTP
 * The data transfer process, in its normal "active" state,
 * establishes the data connection with the "listening" data port.
 * It sets up parameters for transfer and storage, and transfers
 * data on command from its PI.  The DTP can be placed in a
 * "passive" state to listen for, rather than initiate a
 *
 * user-FTP process
 * A set of functions including a protocol interpreter, a data
 * transfer process and a user interface which together perform
 * the function of file transfer in cooperation with one or more
 * server-FTP processes.  The user interface allows a local
 * language to be used in the command-reply dialogue with the
 * user.
 *
 * user-DTP
 * The data transfer process "listens" on the data port for a
 * connection from a server-FTP process.  If two servers are
 * transferring data between them, the user-DTP is inactive.
 *
 * user-PI
 * The user protocol interpreter initiates the control connection
 * from its port U to the server-FTP process, initiates FTP
 * commands, and governs the user-DTP if that process is part of
 * the file transfer.
 *
 *
 * THE FTP MODEL
 *                                          -------------
 *                                          |/---------\|
 *                                          ||   User  ||    --------
 *                                          ||Interface|<--->| User |
 *                                          |\----^----/|    --------
 *                ----------                |     |     |
 *                |/------\|  FTP Commands  |/----V----\|
 *                ||Server|<---------------->|   User  ||
 *                ||  PI  ||   FTP Replies  ||    PI   ||
 *                |\--^---/|                |\----^----/|
 *                |   |    |                |     |     |
 *    --------    |/--V---\|      Data      |/----V----\|    --------
 *    | File |<--->|Server|<---------------->|  User   |<--->| File |
 *    |System|    || DTP  ||   Connection   ||   DTP   ||    |System|
 *    --------    |\------/|                |\---------/|    --------
 *                ----------                -------------
 *
 *                Server-FTP                   USER-FTP
 *
 *    NOTES: 1. The data connection may be used in either direction.
 *           2. The data connection need not exist all of the time.
 *
 *                    Figure 1  Model for FTP Use
 *
 *    In the model described in Figure 1, the user-protocol interpreter
 *    initiates the control connection.  The control connection follows
 *    the Telnet protocol.  At the initiation of the user, standard FTP
 *    commands are generated by the user-PI and transmitted to the
 *    server process via the control connection.  (The user may
 *    establish a direct control connection to the server-FTP, from a
 *    TAC terminal for example, and generate standard FTP commands
 *    independently, bypassing the user-FTP process.) Standard replies
 *    are sent from the server-PI to the user-PI over the control
 *    connection in response to the commands.
 *
 *    The FTP commands specify the parameters for the data connection
 *    (data port, transfer mode, representation type, and structure) and
 *    the nature of file system operation (store, retrieve, append,
 *    delete, etc.).  The user-DTP or its designate should "listen" on
 *    the specified data port, and the server initiate the data
 *    connection and data transfer in accordance with the specified
 *    parameters.  It should be noted that the data port need not be in
 *    the same host that initiates the FTP commands via the control
 *    connection, but the user or the user-FTP process must ensure a
 *    "listen" on the specified data port.  It ought to also be noted
 *    that the data connection may be used for simultaneous sending and
 *    receiving.
 *
 *    In another situation a user might wish to transfer files between
 *    two hosts, neither of which is a local host. The user sets up
 *    control connections to the two servers and then arranges for a
 *    data connection between them.  In this manner, control information
 *    is passed to the user-PI but data is transferred between the
 *    server data transfer processes.  Following is a model of this
 *    server-server interaction.
 *
 *
 *                  Control     ------------   Control
 *                  ---------->| User-FTP |<-----------
 *                  |          | User-PI  |           |
 *                  |          |   "C"    |           |
 *                  V          ------------           V
 *          --------------                        --------------
 *          | Server-FTP |   Data Connection      | Server-FTP |
 *          |    "A"     |<---------------------->|    "B"     |
 *          -------------- Port (A)      Port (B) --------------
 *
 *
 *                               Figure 2
 *
 *    The protocol requires that the control connections be open while
 *    data transfer is in progress.  It is the responsibility of the
 *    user to request the closing of the control connections when
 *    finished using the FTP service, while it is the server who takes
 *    the action.  The server may abort data transfer if the control
 *    connections are closed without command.
 *
 *    The Relationship between FTP and Telnet:
 *
 *       The FTP uses the Telnet protocol on the control connection.
 *       This can be achieved in two ways: first, the user-PI or the
 *       server-PI may implement the rules of the Telnet Protocol
 *       directly in their own procedures; or, second, the user-PI or
 *       the server-PI may make use of the existing Telnet module in the
 *       system.
 *
 *       Ease of implementaion, sharing code, and modular programming
 *       argue for the second approach.  Efficiency and independence
 *       argue for the first approach.  In practice, FTP relies on very
 *       little of the Telnet Protocol, so the first approach does not
 *       necessarily involve a large amount of code.
 *
 * FTP commands
 *          USER <SP> <username> <CRLF>
 *          PASS <SP> <password> <CRLF>
 *          ACCT <SP> <account-information> <CRLF>
 *          CWD  <SP> <pathname> <CRLF>
 * Change to parent directory:
 *          CDUP <CRLF>
 * Structure Mount:
 *          SMNT <SP> <pathname> <CRLF>
 *          QUIT <CRLF>
 *          REIN <CRLF>
 *          PORT <SP> <host-port> <CRLF>
 *          PASV <CRLF>
 *          TYPE <SP> <type-code> <CRLF>
 *          STRU <SP> <structure-code> <CRLF>
 *          MODE <SP> <mode-code> <CRLF>
 *          RETR <SP> <pathname> <CRLF>
 *          STOR <SP> <pathname> <CRLF>
 * Store Unique:
 *          STOU <CRLF>
 *          APPE <SP> <pathname> <CRLF>
 *          ALLO <SP> <decimal-integer>
 *              [<SP> R <SP> <decimal-integer>] <CRLF>
 *          REST <SP> <marker> <CRLF>
 *          RNFR <SP> <pathname> <CRLF>
 *          RNTO <SP> <pathname> <CRLF>
 *          ABOR <CRLF>
 *          DELE <SP> <pathname> <CRLF>
 * Remove Directory:
 *          RMD  <SP> <pathname> <CRLF>
 * Make Directory:
 *          MKD  <SP> <pathname> <CRLF>
 * Print Directory:
 *          PWD  <CRLF>
 *          LIST [<SP> <pathname>] <CRLF>
 *          NLST [<SP> <pathname>] <CRLF>
 *          SITE <SP> <string> <CRLF>
 * System:
 *          SYST <CRLF>
 *          STAT [<SP> <pathname>] <CRLF>
 *          HELP [<SP> <string>] <CRLF>
 *          NOOP <CRLF>
 *
 *  STATE DIAGRAMS
 *
 * Here we present state diagrams for a very simple minded FTP
 * implementation.  Only the first digit of the reply codes is used.
 * There is one state diagram for each group of FTP commands or command
 * sequences.
 *
 * The command groupings were determined by constructing a model for
 * each command then collecting together the commands with structurally
 * identical models.
 *
 * For each command or command sequence there are three possible
 * outcomes: success (S), failure (F), and error (E).  In the state
 * diagrams below we use the symbol B for "begin", and the symbol W for
 * "wait for reply".
 *
 * We first present the diagram that represents the largest group of FTP
 * commands:
 *
 *
 *                             1,3    +---+
 *                        ----------->| E |
 *                       |            +---+
 *                       |
 *    +---+    cmd    +---+    2      +---+
 *    | B |---------->| W |---------->| S |
 *    +---+           +---+           +---+
 *                       |
 *                       |     4,5    +---+
 *                        ----------->| F |
 *                                    +---+
 *
 *
 *    This diagram models the commands:
 *
 *       ABOR, ALLO, DELE, CWD, CDUP, SMNT, HELP, MODE, NOOP, PASV,
 *       QUIT, SITE, PORT, SYST, STAT, RMD, MKD, PWD, STRU, and TYPE.
 *
 * The other large group of commands is represented by a very similar
 * diagram:
 *
 *
 *                             3      +---+
 *                        ----------->| E |
 *                       |            +---+
 *                       |
 *    +---+    cmd    +---+    2      +---+
 *    | B |---------->| W |---------->| S |
 *    +---+       --->+---+           +---+
 *               |     | |
 *               |     | |     4,5    +---+
 *               |  1  |  ----------->| F |
 *                -----               +---+
 *
 *
 *    This diagram models the commands:
 *
 *       APPE, LIST, NLST, REIN, RETR, STOR, and STOU.
 *
 * Note that this second model could also be used to represent the first
 * group of commands, the only difference being that in the first group
 * the 100 series replies are unexpected and therefore treated as error,
 * while the second group expects (some may require) 100 series replies.
 * Remember that at most, one 100 series reply is allowed per command.
 *
 * The remaining diagrams model command sequences, perhaps the simplest
 * of these is the rename sequence:
 *
 *
 *    +---+   RNFR    +---+    1,2    +---+
 *    | B |---------->| W |---------->| E |
 *    +---+           +---+        -->+---+
 *                     | |        |
 *              3      | | 4,5    |
 *       --------------  ------   |
 *      |                      |  |   +---+
 *      |               ------------->| S |
 *      |              |   1,3 |  |   +---+
 *      |             2|  --------
 *      |              | |     |
 *      V              | |     |
 *    +---+   RNTO    +---+ 4,5 ----->+---+
 *    |   |---------->| W |---------->| F |
 *    +---+           +---+           +---+
 *
 *
 *  The next diagram is a simple model of the Restart command:
 *
 *
 *    +---+   REST    +---+    1,2    +---+
 *    | B |---------->| W |---------->| E |
 *    +---+           +---+        -->+---+
 *                     | |        |
 *              3      | | 4,5    |
 *       --------------  ------   |
 *      |                      |  |   +---+
 *      |               ------------->| S |
 *      |              |   3   |  |   +---+
 *      |             2|  --------
 *      |              | |     |
 *      V              | |     |
 *    +---+   cmd     +---+ 4,5 ----->+---+
 *    |   |---------->| W |---------->| F |
 *    +---+        -->+---+           +---+
 *                |      |
 *                |  1   |
 *                 ------
 *
 *
 *       Where "cmd" is APPE, STOR, or RETR.
 *
 * We note that the above three models are similar.  The Restart differs
 * from the Rename two only in the treatment of 100 series replies at
 * the second stage, while the second group expects (some may require)
 * 100 series replies.  Remember that at most, one 100 series reply is
 * allowed per command.
 *
 *
 *                          1
 *    +---+   USER    +---+------------->+---+
 *    | B |---------->| W | 2       ---->| E |
 *    +---+           +---+------  |  -->+---+
 *                     | |       | | |
 *                   3 | | 4,5   | | |
 *       --------------   -----  | | |
 *      |                      | | | |
 *      |                      | | | |
 *      |                 ---------  |
 *      |               1|     | |   |
 *      V                |     | |   |
 *    +---+   PASS    +---+ 2  |  ------>+---+
 *    |   |---------->| W |------------->| S |
 *    +---+           +---+   ---------->+---+
 *                     | |   | |     |
 *                   3 | |4,5| |     |
 *       --------------   --------   |
 *      |                    | |  |  |
 *      |                    | |  |  |
 *      |                 -----------
 *      |             1,3|   | |  |
 *      V                |  2| |  |
 *    +---+   ACCT    +---+--  |   ----->+---+
 *    |   |---------->| W | 4,5 -------->| F |
 *    +---+           +---+------------->+---+
 *
 *
 * Finally, we present a generalized diagram that could be used to model
 * the command and reply interchange:
 *
 *
 *             ------------------------------------
 *            |                                    |
 *    Begin   |                                    |
 *      |     V                                    |
 *      |   +---+  cmd   +---+ 2         +---+     |
 *       -->|   |------->|   |---------->|   |     |
 *          |   |        | W |           | S |-----|
 *       -->|   |     -->|   |-----      |   |     |
 *      |   +---+    |   +---+ 4,5 |     +---+     |
 *      |     |      |    | |      |               |
 *      |     |      |   1| |3     |     +---+     |
 *      |     |      |    | |      |     |   |     |
 *      |     |       ----  |       ---->| F |-----
 *      |     |             |            |   |
 *      |     |             |            +---+
 *       -------------------
 *            |
 *            |
 *            V
 *           End
 *
 *
 *
 *
 * TM_STRING
 *            <string> ::= <char> | <char><string>
 *            <char> ::= any of the 128 ASCII characters except <CR> and
 *           <LF>
 *
 *
 * TM_STRINGOPT
 *            <string> ::= [string]
 *
 *
 * TM_HOSTPORT
 *           <host-port> ::= <host-number>,<port-number>
 *           <host-number> ::= <number>,<number>,<number>,<number>
 *           <port-number> ::= <number>,<number>
 *           <number> ::= any decimal integer 1 through 255
 *
 *
 * TM_STRUCTCODE
 *  <structure-code> ::= F | R | P
 *
 *
 *
 * TM_TYPECODE
 *
 *           <number> ::= any decimal integer 1 through 255
 *           <byte-size> ::= <number>
 *           <form-code> ::= N | T | C
 *           <type-code> ::= A [<sp> <form-code>]
 *                         | E [<sp> <form-code>]
 *                         | I
 *                         | L <sp> <byte-size>
 *
 *
 * TM_MODECODE
 *
 *           <mode-code> ::= S | B | C
 *
 *
 * TM_DECIMAL_R
 *          <decimal-integer> ::= any decimal integer
 *  <SP> <decimal-integer> [<SP> R <SP> <decimal-integer>] <CRLF>
 *
 *
 * TM_MARKER
 *           <marker> ::= <pr-string>
 *           <pr-string> ::= <pr-char> | <pr-char><pr-string>
 *           <pr-char> ::= printable characters, any
 *                         ASCII code 33 through 126
 *
 */


/* Default TCP ports */
#define TM_REMOTE_CTRL_PORT 21
/*
 * Setting the local data port to 0 will force the client to use the port
 * number obtained when the socket is created.  The tfFtpPortIf API can be
 * used to specify a port number.
 */
#define TM_LOCAL_DATA_PORT  0

/* Base value for FTP reply codes */
#define TM_REPLY_BASE       8000

/* Additional FTP reply code, indicating aborted operation */
#define TM_REPLY_ABOR       426

/* Default resolution value for FTP timer (in milliseconds) - 1 second */
#define TM_FTP_TIMER_VALUE  TM_UL(1000)

/* Default values for FTP timeouts (in seconds) */
#define TM_CONNECT_TIMEOUT  90
#define TM_REPLY_TIMEOUT    90
#define TM_ACCEPT_TIMEOUT   90
#define TM_RECV_TIMEOUT     90
#define TM_SEND_TIMEOUT     90

/* FTP transfer modes (default = stream) */
#define TM_MODE_STREAM       0
#define TM_MODE_BLOCK        1
#define TM_MODE_COMPRESSED   2

/* Second parameter to TYPE (default = nonprint) */
#define TM_FORM_NONPRINT     0
#define TM_FORM_TELNET       1
#define TM_FORM_ASA          2

/*
 * Telnet NVT special characters
 */
/* No operation */
#define TM_NVT_NULL          0
/* Next print line, same horizontal position (10) */
#define TM_NVT_LF         '\n'
/* Left margin of current line (13) */
#define TM_NVT_CR         '\r'
/* Space */
#define TM_NVT_SPACE       ' '


/*
 * Telnet states
 */
/* Processing regular characters */
#define TM_TEL_DATA_STATE     0
/* Carriage return as been received */
#define TM_TEL_CR_STATE    0x10

/*
 * Telnet command
 */
/* Interpret as command escape */
#define TM_TC_IAC           (tt8Bit) 255
/* Request peer not to do option */
#define TM_TC_DONT          (tt8Bit) 254
/* Request peer to do option */
#define TM_TC_DO            (tt8Bit) 253
/* Indicates refusal to do option */
#define TM_TC_WONT          (tt8Bit) 252
/* Indicates request to do option */
#define TM_TC_WILL          (tt8Bit) 251
/* Interrupt process */
#define TM_TC_IP            (tt8Bit) 244
/* Data mark-- Synch signal (with TCP URG) */
#define TM_TC_DM            (tt8Bit) 242

static const tt8Bit TM_CONST_QLF tlInterData[4] =
    {TM_TC_IAC, TM_TC_IP, TM_TC_IAC, TM_TC_DM};
/*
 * Default size limits
 */
/* Maximum length of reply line */
#define TM_CMD_LINE_SIZE    256
/* Send buffer size */
#define TM_BUF_SEND_SIZE   1460
/* Receive buffer size */
#define TM_BUF_RECV_SIZE   4096

/*
 * File check return values
 */
/* File exists */
#define TM_FILCHK_EXIST     0
/* File does not exist, but could be created */
#define TM_FILCHK_NEXISTFIL (-1)
/* Directory does not exist */
#define TM_FILCHK_NEXISTDIR (-2)
/* Device does not exist */
#define TM_FILCHK_NEXISTDEV (-3)


/*
 * Macros to check reply codes for general type (100, 200, etc)
 */
#define tm_is_reply_1xx(x)  ( ((x) >= 100) && ((x) <= 199) )
#define tm_is_reply_2xx(x)  ( ((x) >= 200) && ((x) <= 299) )
#define tm_is_reply_3xx(x)  ( ((x) >= 300) && ((x) <= 399) )
#define tm_is_reply_4xx(x)  ( ((x) >= 400) && ((x) <= 499) )
#define tm_is_reply_5xx(x)  ( ((x) >= 500) && ((x) <= 599) )
#define tm_is_reply_code(x) ( ((x) >= 100) && ((x) <= 599) )

#define TM_REPLY_CODE_ERR   0


/*
 * FTP Client Commands
 */
#define TM_CMD_IDLE             0

#define TM_CMD_CONN             1
#define TM_CMD_CLOS             2

/* Access Control Commands */
#define TM_CMD_USER             3
#define TM_CMD_PASS             4
#define TM_CMD_ACCT             5
#define TM_CMD_CWD              6
#define TM_CMD_CDUP             7
#define TM_CMD_SMNT             8
#define TM_CMD_REIN             9
#define TM_CMD_QUIT            10

/* Transfer Parameter Commands */
#define TM_CMD_PORT            11
#define TM_CMD_PASV            12
#define TM_CMD_TYPE            13
#define TM_CMD_STRU            14
#define TM_CMD_MODE            15

/* FTP Service Commands */
#define TM_CMD_RETR            16
#define TM_CMD_STOR            17
#define TM_CMD_STOU            18
#define TM_CMD_APPE            19
#define TM_CMD_ALLO            20
#define TM_CMD_REST            21
#define TM_CMD_RNFR            22
#define TM_CMD_RNTO            23
#define TM_CMD_ABOR            24
#define TM_CMD_DELE            25
#define TM_CMD_RMD             26
#define TM_CMD_MKD             27
#define TM_CMD_PWD             28
#define TM_CMD_LIST            29
#define TM_CMD_NLST            30
#define TM_CMD_SITE            31
#define TM_CMD_SYST            32
#define TM_CMD_STAT            33
#define TM_CMD_HELP            34
#define TM_CMD_NOOP            35
#define TM_CMD_EPRT            36
#define TM_CMD_EPSV            37

#ifndef TM_USE_FTP_SSL
#define TM_CMD_LAST            37
#endif /* !TM_USE_FTP_SSL */

#ifdef TM_USE_FTP_SSL
/*
 * The SSL command macros must be defined differently if IPv6 is disabled.
 * If IPv6 is disabled, TM_CMD_EPRT and TM_CMD_EPSV are not included in
 * tlFtpCmdTable and the SSL commands must start at 36 (TM_CMD_EPRT).
 */
#ifdef TM_USE_IPV6
#define TM_CMD_AUTH            38
#define TM_CMD_PROT            39
#define TM_CMD_PBSZ            40
#define TM_CMD_CCC             41
#define TM_CMD_LAST            41
#else  /* !TM_USE_IPV6 */
#define TM_CMD_AUTH            36
#define TM_CMD_PROT            37
#define TM_CMD_PBSZ            38
#define TM_CMD_CCC             39
#define TM_CMD_LAST            39
#endif /* !TM_USE_IPV6 */

/* SSL Callback Flags */
#define TM_FTP_SSL_CB_F (TM_CB_SSL_ESTABLISHED | TM_CB_SSL_SEND_CLOSE | \
                         TM_CB_SSL_RECV_CLOSE | TM_CB_SSL_HANDSHK_FAILURE | \
                         TM_CB_SSL_BADCERTIFICATE | TM_CB_SSL_HANDSHK_PROCESS)

/*
 * The SSL negotiation flags are used to indicate the status of SSL for the
 * control and data connections. The FTP session variables,
 * ftpSslNegFlagsCtrlConn and ftpSslNegFlagsDataConn, are used to store
 * these values.
 */
#define TM_FTP_SSL_NOT_NEG      0
#define TM_FTP_SSL_NEG_SUCCESS  2
#define TM_FTP_SSL_NEG_FAILURE  4
#endif /* TM_USE_FTP_SSL */

/* FSM matrices (see RFC above) */
#define TM_FTP_MATRIX0          0
#define TM_FTP_MATRIX1          1
#define TM_FTP_MATRIX2          2
#define TM_FTP_MATRIX3          3
#define TM_FTP_MATRIX4          4

#define TM_FTP_MATRIX_SIZE      5

/* FSM phases (states) */
#define TM_FTPP_INITIAL         0
#define TM_FTPP_CONNECT_WAIT    1
#define TM_FTPP_REPLY_WAIT      2
#define TM_FTPP_DATA_CONN       3
#define TM_FTPP_DATA_RECV       4
#define TM_FTPP_DATA_SEND       5
#define TM_FTPP_SEND_WAIT       6

#define TM_FTPP_SIZE            7

/*
 * FTP client events
 * CONNECT is a request to connect (CONNECT command = open)
 * CLOSE is a request to close session (CLOSE command used in emergency)
 * COMMAND is a user request for other commands (LOGIN, CWD, RETR, etc)
 * ABORT is a request to quit session or abort transfer
 *      (normal QUIT or ABOR command)
 * CTRL_CONN is notification that control connection has been established
 * DATA_CONN is notification that data connection has been established
 * CTRL_RECV is notification that data has been received on control port
 * CTRL_IRECV is notification that data has been received on control port but
 *      command is IDLE
 *      note: IDLE is the "command" when waiting for the next user command
 * DATA_RECV is notification that data has been received on data port
 * SEND_REQ indicates there is data to send on the data port (SEND command)
 * WRITE_RDY indicates all data has been sent on the data port (SEND command)
 * CTRL_ERROR is notification that control port has been reset or closed by
 *      server or a socket error has occured
 * DATA_ERROR is notification that data port has been reset or closed by server
 *      or a socket error has occured
 * TIMEOUT is notification of a timeout
 */

#define TM_FTPE_CONNECT         0
#define TM_FTPE_CLOSE           1
#define TM_FTPE_COMMAND         2
#define TM_FTPE_ABORT           3
#define TM_FTPE_CTRL_CONN       4
#define TM_FTPE_DATA_CONN       5
#define TM_FTPE_CTRL_RECV       6
#define TM_FTPE_CTRL_IRECV      7
#define TM_FTPE_DATA_RECV       8
#define TM_FTPE_SEND_REQ        9
#define TM_FTPE_WRITE_RDY      10
#define TM_FTPE_CTRL_ERROR     11
#define TM_FTPE_DATA_ERROR     12
#define TM_FTPE_TIMEOUT        13

#define TM_FTPE_SIZE           (TM_FTPE_TIMEOUT + 1)

/* FTP client action flags */
#define TM_FTPS_CONNECT              0
#define TM_FTPS_CMDREQ          0x0001
#define TM_FTPS_CTRLCONN        0x0002
#define TM_FTPS_DATACONN        0x0004
#define TM_FTPS_CTRLRECV        0x0008
#define TM_FTPS_DATARECV        0x0010
#define TM_FTPS_SENDREQ         0x0020
#define TM_FTPS_SENDCOMP        0x0040
#define TM_FTPS_CTRLERRO        0x0080
#define TM_FTPS_DATAERRO        0x0100
#define TM_FTPS_TIMEOUT         0x0200

/*
 * Socket descriptor constants
 */
#define TM_FTP_CTRL_SOCKET           0
#define TM_FTP_DATA_SOCKET           1
#define TM_FTP_LISTEN_SOCKET         2

/*
 * FTP status flags (control connection connected, logged in, passive mode,
 *                   data connection, connected, directory listing received,
 *                   reply received)
 */
#define TM_FTP_F_CTRL_CONN      0x0001
#define TM_FTP_F_LOGGED_IN      0x0002

#define TM_FTP_F_DATA_PASV      0x0004

#define TM_FTP_F_DATA_CONN      0x0010

#define TM_FTP_F_DIRLIST_RECVED 0x2000
#define TM_FTP_F_REPLY_RECVED   0x4000
#define TM_FTP_F_DATA_PORT      0x8000

#define TM_USER_F_PARM          TM_FTP_F_DATA_PORT


/*
 * Send status flags (see ftpSendRequest below)
 */
#define TM_SEND_REQ_ON          1
#define TM_SEND_REQ_OFF         0

#define TM_FTP_TIMER_ON         1
#define TM_FTP_TIMER_OFF        0


/* Common Data Command flags */
#define TM_FTP_CMD_F_TO_SERVER_A    0x01 /* File transfer to server A */
#define TM_FTP_CMD_F_FROM_SERVER_A  0x02 /* File transfer from server A */


/*
 * Get the command code for passive data transfer:
 *      INPUT               OUTPUT
 *      TM_CMD_RETR         TM_CMD_STOR
 *      TM_CMD_STOR         TM_CMD_RETR
 *      TM_CMD_STOU         TM_CMD_RETR
 *      TM_CMD_APPE         TM_CMD_RETR
 */
#define tm_get_pasv_cmd_code(cmdCode) \
            (tlFtpPasvCmdCode[cmdCode-TM_CMD_RETR])
/*
 * Union used to convert IP address to text
 */
union tuFtpIpWork
{
    tt4IpAddress     ftpIpAddress;
    ttUChar         ftpIpAddr[4];
};

/*
 * Union used to convert TCP port numbers to text
 */
union tuFtpPortWork
{
    ttIpPort        ftpPortNo;
    ttUChar         ftpPort[2];
};

/*
 * Main FTP client session vector
 */
#ifdef TM_LINT
LINT_UNREF_MEMBER_BEGIN
#endif /* TM_LINT */

typedef struct  tsFtpSession
{
/* Pointer to the 2nd FTP session for passive mode operation */
    struct tsFtpSession TM_FAR *    ftp2ndSessionPtr;

/* Address of the client's control and data connections */
    struct sockaddr_storage         ftpClientCtrlAddress;
    struct sockaddr_storage         ftpClientDataAddress;

/* Addresses of the server's control and data connections */
    struct sockaddr_storage         ftpServerCtrlAddress;
    struct sockaddr_storage         ftpServerDataAddress;

/* Handle to filesystem, retrieved by tfFSUserLogin */
    void TM_FAR *                   ftpUserDataPtr;

/* FTP timeout timer handle */
    ttTimerPtr                      ftpTimerBlockPtr;

/*
 * Username, password and account information for this session.  Used by
 * USER, PASS and ACCT commands.
 */
    ttCharPtr                       ftpUserNamePtr;
    ttCharPtr                       ftpUserPasswordNamePtr;
    ttCharPtr                       ftpUserAccountNamePtr;

/*  Current pathname being used by CWD, SMNT, DELE, RMD and MKD */
    ttCharPtr                       ftpPathNamePtr;

/* 'TO' and 'FROM' filenames used when renaming a remote file */
    ttCharPtr                       ftpFromNamePtr;
    ttCharPtr                       ftpToNamePtr;

/* Argument to HELP passed in by user */
    ttCharPtr                       ftpCommandPtr;
/* User function to call with HELP info when command has completed */
    ttFtpCBFuncPtr                  ftpUserCBFuncPtr;

/* Remote and local files used for STOR and RETR */
    ttCharPtr                       ftpLocalFileNamePtr;
    ttCharPtr                       ftpRemoteFileNamePtr;

/* Generic FS pointer to current file being read or written to */
    void TM_FAR *                   ftpFilePointer;

/*
 * Information about a user's buffer that was passed in.  It is used to return
 * data such as current directory (MKD,PWD,SYST,STAT).
 */
    ttCharPtr                       ftpBufferPtr;
    int                             ftpBufferSize;
    int                             ftpBufferIndex;

/* Blocking state of the FTP client: TM_BLOCKING_ON or TM_BLOCKING_OFF */
    int                             ftpBlockingState;

/* Socket descriptors for the control, data and listening sockets */
    int                             ftpSocketDesc[3];

/* FTP transfer mode, type, form and structure (see RFC) */
    int                             ftpTransMode;
    int                             ftpTransType;
    int                             ftpTransForm;
    int                             ftpTransStructure;

/* Temporary holder for transfer parameter (type, mode, etc) */
    int                             ftpTransParm;

/* Current numeric reply code */
    int                             ftpReplyCode;

/* Index into the directory listing buffer */
    int                             ftpDirListIndex;

/* Index into data buffer, indicating amount read and processed */
    int                             ftpDataBufReadIndex;
    int                             ftpDataBufProcessIndex;
/* Total size of send buffer */
    int                             ftpDataBufSendSize;
/* Amount of data sent from buffer */
    int                             ftpDataBufSendIndex;

/* Buffer index, used to process reply line */
    int                             ftpTelBufReadIndex;
    int                             ftpTelBufProcessIndex;

/* Indicates the port to be used for future client control connections */
    ttIpPort                        ftpPortNo;

/* The client control port that is currently in use */
    ttIpPort                        ftpNowPortNo;

    ttIpPort                        ftpPortParm;

/* Flags indicating the status of the client (logged in, connected, etc) */
    tt16Bit                         ftpFlags;

/* Callback flags for the control, data and listening sockets */
    tt16Bit                         ftpCBFlags[3];

/* Current index into the reply line buffer */
    tt16Bit                         ftpReplyLineIndex;

/* Timeout counter: when this value reaches zero, a timeout has occured */
    tt16Bit                         ftpTimerCounter;

/* Current command requested by user */
    tt8Bit                          ftpCmdRequest;

/* Boolean indicating whether data should be sent or not */
    tt8Bit                          ftpSendRequest;

/* Current FSM matrix, phase and event */
    tt8Bit                          ftpMatrixNo;
    tt8Bit                          ftpMtrxPhase;
    tt8Bit                          ftpMtrxEvent;

/* Commands to send to FTP server: current, next and intermediate */
    tt8Bit                          ftpCommandCode;
    tt8Bit                          ftpNextCommandCode;
    tt8Bit                          ftpInterCommandCode;

/*
 * Boolean indicating whether the timer is active or not: TM_FTP_TIMER_OFF or
 * TM_FTP_TIMER_ON.
 */
    tt8Bit                          ftpTimerFlag;

/*
 * State of processing in control and data buffers (last character is or is not
 * a CR).
 */
    tt8Bit                          ftpTelState;
    tt8Bit                          ftpDataBufTelState;

/* Pad to 4 byte boundary */
    tt8Bit                          ftpPad[2];

/* Current (parsed) reply line received from the server */
    tt8Bit                          ftpReplyLine[TM_CMD_LINE_SIZE];

/*
 * Buffer to contain data received from FTP server, before it is parsed and
 * processed into a reply line, an index indicating the start of unparsed data,
 * and a index indicating the start of unparsed data.
 */
    tt8Bit                          ftpTelInputBuf[TM_CMD_LINE_SIZE];

/* Buffer used to hold outgoing FTP command */
    tt8Bit                          ftpCmdSendBuf[TM_CMD_LINE_SIZE];

/* Buffer used to hold remote directory listing */
    tt8Bit                          ftpDirList[TM_CMD_LINE_SIZE];

/* Buffer used to hold incoming socket data */
    tt8Bit                          ftpDataRecvBuf[TM_BUF_RECV_SIZE];

/* Buffer used to hold outgoing socket data */
    tt8Bit                          ftpDataSendBuf[TM_BUF_SEND_SIZE];

#ifdef TM_USE_FTP_SSL
/* SSL related variables */
/* Array holding acceptable cipher suites. */
    int                              ftpPropArray[TM_SSL_PROPOSAL_NUMBER];

/* String representing the certificate identity. */
    ttCharPtr                        ftpCertIdentityPtr;

/* Value holding the SSL server name */
    ttCharPtr                        ftpSslServerName;

/* Holds the number of cipher suites the user is using. */
    int                              ftpPropArraySize;

/* Indicates the SSL version. */
    int                              ftpSslVersion;

/* The SSL session handle. */
    int                              ftpSslSessionId;

/* The SSL session options. */
    tt16Bit                          ftpSslOptions;

/* Flag indicating whether SSL is enabled on the control connection. */
    tt8Bit                           ftpSslEnabledCtrlConn;

/* Flag indicating whether SSL is enabled on the data connection. */
    tt8Bit                           ftpSslEnabledDataConn;

/* Flag indicating SSL negotiation status on the control connection. */
    tt8Bit                           ftpSslNegFlagsCtrlConn;

/* Flag indicating SSL negotiation status on the data connection. */
    tt8Bit                           ftpSslNegFlagsDataConn;

/* Holds the data channel protection level, i.e. 'P' or 'C' */
    tt8Bit                           ftpDataProtLevel;


/* Indicates whether the server requires SSL for the data connection */
    tt8Bit                           ftpSslDataConnReq;

/* Indicates whether SSL negotiation is being carried out. */
    tt8Bit                           ftpSslNegInProgress;

/* Waiting for SSL close notify */
    tt8Bit                           ftpSslWaitCloseNotify;
#endif /* TM_USE_FTP_SSL */

} ttFtpSession;
#ifdef TM_LINT
LINT_UNREF_MEMBER_END
#endif /* TM_LINT */


typedef ttFtpSession TM_FAR *   ttFtpSessionPtr;

/* FTP command handlers: forms command string and sends to FTP server */
static int tfFtpCmdFuncNone(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncUser(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncPass(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncAcct(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncCwd(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncCdup(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncRein(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncQuit(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncPort(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncPasv(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncType(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncStru(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncMode(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncRetr(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncStor(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncAppe(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncRnfr(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncRnto(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncAbor(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncDele(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncRmd(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncMkd(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncPwd(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncList(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncNlst(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncSyst(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncHelp(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncNoop(ttFtpSessionPtr ftpSessionPtr);

#ifdef TM_USE_IPV6
static int tfFtpCmdFuncEprt(ttFtpSessionPtr ftpSessionPtr);
#endif /* TM_USE_IPV6 */

#ifdef TM_USE_FTP_SSL
static int tfFtpCmdFuncAuth(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncProt(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncPbsz(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdFuncCcc(ttFtpSessionPtr ftpSessionPtr);
#endif /* TM_USE_FTP_SSL */

/*
 * Reply code handlers for a variety of commands (Stor, Abor, etc) and
 * codes (1xx,3xx,etc).
 */
static int tfFtpConnReply_1345(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpConnReply_2xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpLoginReply_Error(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpLoginReply_Success(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpUserReply_3xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpLoginReply_Failure(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpPassReply_3xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpReinReply_2xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpQuitReply_1345(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpQuitReply_2xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpPortReply_2xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpPasvReply_2xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpTypeReply_2xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpStruReply_2xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpModeReply_2xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpRetrReply_1xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpStorReply_1xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpStouReply_1xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpAppeReply_1xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpRnfrReply_3xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpAborReply_2xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpXxxxReply_BufCopy(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpListReply_1xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpNlstReply_1xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpHelpReply_2xx(ttFtpSessionPtr ftpSessionPtr);
#ifdef TM_USE_FTP_SSL
static int tfFtpAuthReply_3xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpAuthReply_4xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpAuthReply_5xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpAuthReply_2xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCccReply_5xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCccReply_2xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpProtReply_2xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpProtReply_4xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpProtReply_5xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpPbszReply_2xx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpPbszReply_5xx(ttFtpSessionPtr ftpSessionPtr);
#endif /* TM_USE_FTP_SSL */
static int tfFtpXxxxReply_xxx(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpXxxxReply_Error(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpXxxxReply_Success(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpXxxxReply_Failure(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpXxxxReply_Wait(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpDataReply_Failure(ttFtpSessionPtr ftpSessionPtr);

/* State transition functions */
static int tfFtpMtrxAlready(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpMtrxCommand(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpMtrxCtrlConnect(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpMtrxGetCtrlInfo(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpMtrxCtrlRecv(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpMtrxCtrlReplyRecv(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpMtrxPermDenied(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpMtrxInterCommand(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpMtrxCtrlReplyRecvIdle( ttFtpSessionPtr ftpSessionPtr );
static int tfFtpMtrxDataConnect(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpMtrxDataPreRecv(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpMtrxDirDataRecv(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpMtrxCloseCtrl(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpMtrxCloseCtrlData(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpMtrxCloseData(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpMtrxCloseFileData(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpMtrxDataRecv(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpMtrxDataSend(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpMtrxSendComplete(ttFtpSessionPtr ftpSessionPtr);

/* Initialization functions */
static void tfFtpDataDefault(ttFtpSessionPtr ftpSessionPtr,
                             int             blockingState);
static void tfFtpDataDefault2(ttFtpSessionPtr ftpSessionPtr);

/* Sets the current, next and intermediate command codes */
static void tfFtpSetCommandCode(ttFtpSessionPtr ftpSessionPtr,
                                tt8Bit          commandCode,
                                tt8Bit          nextCommandCode);

/* Command reply processing functions */
static int tfFtpCmdReplyCheck(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdReplyLineReceive(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdReplyReceive(ttFtpSessionPtr ftpSessionPtr);
static int tfFtpCmdReplyCodeCheck(ttFtpSessionPtr ftpSessionPtr);

/* Receive data from socket */
static int tfFtpSocketReceive(ttFtpSessionPtr ftpSessionPtr,
                              int             socketDesc,
                              ttCharPtr       bufferPtr,
                              int             bufferSize,
                              ttIntPtr        recvSizePtr);

/* Send command to remote server */
static int tfFtpCommandSend(ttFtpSessionPtr ftpSessionPtr);

static int tfFtpCtrlCharSend(ttFtpSessionPtr ftpSessionPtr,
                             ttCharPtr       ctrlCharPtr,
                             int             sendSize,
                             int             flags);

/* Closes various FTP sockets */
static void tfFtpCloseCtrlSocket(ttFtpSessionPtr ftpSessionPtr);
/* Close FTP data sockets, and data file */
static void tfFtpCloseData(ttFtpSessionPtr ftpSessionPtr);
static void tfFtpCloseSocket(int socketDesc);

/* FTP signal (callback) handler */
static void tfFtpSignal(int socketDesc, ttVoidPtr ftpPtr, int CBFlags);

/* Signal manipulation routines */
static int  tfFtpGetSignal(ttFtpSessionPtr ftpSessionPtr, int socketDesc);
static void tfFtpSetSignal(ttFtpSessionPtr ftpSessionPtr,
                           int             socketDesc,
                           int             setSignal);
static void tfFtpClearSignal(ttFtpSessionPtr ftpSessionPtr,
                             int             socketDesc,
                             int             clearSignal);

/* Error checking routines: verify validity of requested command */
static int  tfFtpCheckSession(ttFtpSessionPtr ftpSessionPtr, int flags);
static int  tfFtpCheckInterSession(ttFtpSessionPtr ftpSessionPtr, int flags);
static int  tfFtpCheckCommand(ttFtpSessionPtr ftpSessionPtr,
                              int             userCommand,
                              ttCharPtr       argPtr,
                              int             flags);
static int  tfFtpCheckBufferCommand(ttFtpSessionPtr ftpSessionPtr,
                                    int             userCommand,
                                    ttCharPtr       argPtr,
                                    int             flags,
                                    ttCharPtr       bufferPtr,
                                    int             bufferSize );

/* Setups up sockets for incoming data connection */
static int  tfFtpDataConnRequest(ttFtpSessionPtr ftpSessionPtr);

/* Setups up sockets and connect to peer FTP server for data transfer */
static int tfFtpPasvModeDataConnect(ttFtpSessionPtr ftpSessionPtr);

/* FTP Matrix manipulation and execution functions */
static void tfFtpSetMatrixPhase(ttFtpSessionPtr ftpSessionPtr,
                                 tt8Bit          matrixPhase);
static int  tfFtpMatrix(ttFtpSessionPtr ftpSessionPtr);
static int  tfFtpMatrixSession(ttFtpSessionPtr ftpSessionPtr);
static void tfFtpMatrixCheckBlocking(ttFtpSessionPtr ftpSessionPtr);
static int  tfFtpMatrixCmdReq(ttFtpSessionPtr ftpSessionPtr);
static int  tfFtpMatrixCtrlConn(ttFtpSessionPtr ftpSessionPtr);
static int  tfFtpMatrixDataConn(ttFtpSessionPtr ftpSessionPtr);
static int  tfFtpMatrixCtrlRecv(ttFtpSessionPtr ftpSessionPtr);
static int  tfFtpMatrixDataRecv(ttFtpSessionPtr ftpSessionPtr);
static int  tfFtpMatrixSendReq(ttFtpSessionPtr ftpSessionPtr);
static int  tfFtpMatrixSendComp(ttFtpSessionPtr ftpSessionPtr);
static int  tfFtpMatrixCtrlErro(ttFtpSessionPtr ftpSessionPtr);
static int  tfFtpMatrixDataErro(ttFtpSessionPtr ftpSessionPtr);
static int  tfFtpMatrixTimeOut(ttFtpSessionPtr ftpSessionPtr);
static int  tfFtpMatrixExecute(ttFtpSessionPtr ftpSessionPtr);

/* Retrieves FTP reply code */
static int  tfFtpGetReplyCode(ttFtpSessionPtr ftpSesionPtr);

/* Timer signal handler */
static void tfFtpTimerCBFunction(ttVoidPtr       timerBlockPtr,
                                  ttGenericUnion  userParm1,
                                  ttGenericUnion  userParm2);

/* Timer manipulation routines */
static void tfFtpStartTimer(ttFtpSessionPtr ftpSessionPtr, tt16Bit timerSec);
static void tfFtpCancelTimer(ttFtpSessionPtr ftpSessionPtr);
static tt16Bit tfFtpGetTimerCounter(ttFtpSessionPtr ftpSessionPtr);

/* Verifies that file exists or can be created */
static int  tfFtpCheckFile(void TM_FAR * userDataPtr, ttCharPtr fileNamePtr);

/* Closes data file */
static void tfFtpCloseDataFile(ttFtpSessionPtr ftpSessionPtr);

/* String manipulation routines */
static ttCharPtr tfFtpSetString(ttCharPtr setBufPtr, ttCharPtr stringPtr);
#ifdef TM_USE_IPV4
static ttCharPtr tf4FtpSetAddress(ttCharPtr                   buffPtr,
                                  struct sockaddr_in TM_FAR * addressPtr);
#endif /* TM_USE_IPV4 */

static int tfFtpMtrxNoop(ttFtpSessionPtr ftpSessionPtr);

/* Common Data Transfer Command function: RETR, STOR, STOU, APPE */
static int tfFtpCommonDataCommand( tt8Bit                  command,
                                   ttFtpSessionPtr         ftpSessionAPtr,
                                   ttFtpSessionPtr         ftpSessionBPtr,
                                   char TM_FAR *           fromFileNamePtr,
                                   char TM_FAR *           toFileNamePtr,
                                   tt8Bit                  flags);

/* Common Data Reply function, for RETR, STOR, STOU, APPE */
static int tfFtpCommonDataReply( ttFtpSessionPtr   ftpSessionAPtr,
                                 int               fileReadWrite);

/* Set socket options for data connection */
static int tfFtpSetDataOptions( ttFtpSessionPtr    ftpSessionPtr,
                                int                cbFlags);

/* Get socket address from PASV reply */
static int tfFtpGetAddress( ttCharPtr                        replyLinePtr,
                            struct sockaddr_storage TM_FAR * addressPtr );

#ifdef TM_USE_FTP_SSL
/* SSL certificate callback function */
static int tfFtpSslCertCB(int sessionId, int sockDesc,
                          ttCertificateEntryPtr entryArray, int arraySize);

/* Start SSL negotiation on the given socket */
static int tfFtpStartSslNeg(ttFtpSessionPtr ftpSessionPtr, int sockDesc);
#endif /* TM_USE_FTP_SSL */

#ifdef TM_DEBUG_LOGGING
static const char TM_FAR * tfFtpStrMatrixNo( int matrixNo );
static const char TM_FAR * tfFtpStrMatrixEvent( int matrixEvent );
static const char TM_FAR * tfFtpStrMatrixPhase( int matrixPhase );
#endif /* TM_DEBUG_LOGGING */

/* Pointer to a FTP state function */
typedef int (*ttFtpFuncPtr)(ttFtpSessionPtr ftpSessionPtr);

/*
 * Passive command code table, only used for two FTP server model :
 * Use this table to find the corresponding data transfer command
 * code.
 */
static const tt8Bit  TM_CONST_QLF tlFtpPasvCmdCode[TM_CMD_APPE-TM_CMD_RETR+1] =
{
    TM_CMD_STOR,
    TM_CMD_RETR,
    TM_CMD_RETR,
    TM_CMD_RETR,
};


/*
 * This structure describes a FTP command, including the text of the command,
 * and both the request and reply handling routines.
 */
typedef struct tsFtpCmdTable
{

/* Pointer to command handling routine */
    ttFtpFuncPtr    fctCmdFuncPtr;

/*
 * Array of pointers to functions for handling replies from FTP server
 * index of array is first digit of reply code (i.e. reply 425 will use
 * tsFtpCmdTable.fctCmdReplyPtr[4] handling routine.  0 is for invalid replies
 */
    ttFtpFuncPtr    fctCmdReplyPtr[6];

/* FSM matrix corresponding to this command */
    tt8Bit          fctMatrixNo;

/* Text of command (USER, PASS, RETR, etc) */
    char            fctCmdName[7];

} ttFtpCmdTable;



static const ttFtpCmdTable  TM_CONST_QLF tlFtpCmdTable[ TM_CMD_LAST + 1 ] =
{
/*
 *    Command Command Func.     Matrix No.         Reply Func.(xxx)
 *                                                 Reply Func.(1xx)
 *                                                 Reply Func.(2xx)
 *                                                 Reply Func.(3xx)
 *                                                 Reply Func.(4xx)
 *                                                 Reply Func.(5xx)
 */
/* TM_CMD_IDLE: none */
    { tfFtpCmdFuncNone, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Success,

                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure  }, TM_FTP_MATRIX0, "" },

/* TM_CMD_CONN: none */
    { tfFtpCmdFuncNone, { tfFtpXxxxReply_xxx,
                          tfFtpConnReply_1345,
                          tfFtpConnReply_2xx,
                          tfFtpConnReply_1345,
                          tfFtpConnReply_1345,
                          tfFtpConnReply_1345 },     TM_FTP_MATRIX0, "" },

/* TM_CMD_CLOS: none */
    { tfFtpCmdFuncNone, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Success,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX0, "" },

/* TM_CMD_USER: USER <SP> <username> <CRLF> */
    { tfFtpCmdFuncUser, { tfFtpXxxxReply_xxx,
                          tfFtpLoginReply_Error,
                          tfFtpLoginReply_Success,
                          tfFtpUserReply_3xx,
                          tfFtpLoginReply_Failure,
                          tfFtpLoginReply_Failure }, TM_FTP_MATRIX1, "USER" },

/* TM_CMD_PASS: PASS <SP> <password> <CRLF> */
    { tfFtpCmdFuncPass, { tfFtpXxxxReply_xxx,
                          tfFtpLoginReply_Error,
                          tfFtpLoginReply_Success,
                          tfFtpPassReply_3xx,
                          tfFtpLoginReply_Failure,
                          tfFtpLoginReply_Failure }, TM_FTP_MATRIX1, "PASS" },

/* TM_CMD_ACCT: ACCT <SP> <account-information> <CRLF> */
    { tfFtpCmdFuncAcct, { tfFtpXxxxReply_xxx,
                          tfFtpLoginReply_Error,
                          tfFtpLoginReply_Success,
                          tfFtpLoginReply_Error,
                          tfFtpLoginReply_Failure,
                          tfFtpLoginReply_Failure }, TM_FTP_MATRIX1, "ACCT" },

/* TM_CMD_CWD: CWD  <SP> <pathname> <CRLF> */
    { tfFtpCmdFuncCwd,  { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Success,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "CWD" },

/* TM_CMD_CDUP: CDUP <CRLF> (Change to parent directory:) */
    { tfFtpCmdFuncCdup, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Success,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "CDUP" },

/* TM_CMD_SMNT: SMNT <SP> <pathname> <CRLF> (Structure Mount:) */
    { tfFtpCmdFuncNone, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Success,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "SMNT" },

/* TM_CMD_REIN: REIN <CRLF> */
    { tfFtpCmdFuncRein, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Wait,
                          tfFtpReinReply_2xx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "REIN" },

/* TM_CMD_QUIT: QUIT <CRLF> */
    { tfFtpCmdFuncQuit, { tfFtpXxxxReply_xxx,
                          tfFtpQuitReply_1345,
                          tfFtpQuitReply_2xx,
                          tfFtpQuitReply_1345,
                          tfFtpQuitReply_1345,
                          tfFtpQuitReply_1345 },     TM_FTP_MATRIX1, "QUIT" },

/* TM_CMD_PORT: PORT <SP> <host-port> <CRLF> */
    { tfFtpCmdFuncPort, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpPortReply_2xx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "PORT" },

/* TM_CMD_PASV: PASV <CRLF> */
    { tfFtpCmdFuncPasv, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpPasvReply_2xx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "PASV" },

/* TM_CMD_TYPE: TYPE <SP> <type-code> <CRLF> */
    { tfFtpCmdFuncType, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpTypeReply_2xx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "TYPE" },

/* TM_CMD_STRU: STRU <SP> <structure-code> <CRLF> */
    { tfFtpCmdFuncStru, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpStruReply_2xx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "STRU" },

/* TM_CMD_MODE: MODE <SP> <mode-code> <CRLF> */
    { tfFtpCmdFuncMode, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpModeReply_2xx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "MODE" },

/* TM_CMD_RETR: RETR <SP> <pathname> <CRLF> */
    { tfFtpCmdFuncRetr, { tfFtpXxxxReply_xxx,
                          tfFtpRetrReply_1xx,
                          tfFtpXxxxReply_Success,
                          tfFtpXxxxReply_Error,
                          tfFtpDataReply_Failure,
                          tfFtpDataReply_Failure },  TM_FTP_MATRIX3, "RETR" },

/* TM_CMD_STOR: STOR <SP> <pathname> <CRLF> */
    { tfFtpCmdFuncStor, { tfFtpXxxxReply_xxx,
                          tfFtpStorReply_1xx,
                          tfFtpXxxxReply_Success,
                          tfFtpXxxxReply_Error,
                          tfFtpDataReply_Failure,
                          tfFtpDataReply_Failure },  TM_FTP_MATRIX4, "STOR" },

/* TM_CMD_STOU: STOU <CRLF> (Store Unique:) */
    { tfFtpCmdFuncNone, { tfFtpXxxxReply_xxx,
                          tfFtpStouReply_1xx,
                          tfFtpXxxxReply_Success,
                          tfFtpXxxxReply_Error,
                          tfFtpDataReply_Failure,
                          tfFtpDataReply_Failure },  TM_FTP_MATRIX4, "STOU" },

/* TM_CMD_APPE: APPE <SP> <pathname> <CRLF> */
    { tfFtpCmdFuncAppe, { tfFtpXxxxReply_xxx,
                          tfFtpAppeReply_1xx,
                          tfFtpXxxxReply_Success,
                          tfFtpXxxxReply_Error,
                          tfFtpDataReply_Failure,
                          tfFtpDataReply_Failure },  TM_FTP_MATRIX4, "APPE" },

/* TM_CMD_ALLO: ALLO <SP> <decimal-integer>
 *                   [<SP> R <SP> <decimal-integer>] <CRLF>
 */
    { tfFtpCmdFuncNone, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Success,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "ALLO" },

/* TM_CMD_REST: REST <SP> <marker> <CRLF> */
    { tfFtpCmdFuncNone, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Success,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "REST" },

/* TM_CMD_RNFR: RNFR <SP> <pathname> <CRLF> */
    { tfFtpCmdFuncRnfr, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Error,
                          tfFtpRnfrReply_3xx,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "RNFR" },

/* TM_CMD_RNTO: RNTO <SP> <pathname> <CRLF> */
    { tfFtpCmdFuncRnto, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Success,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "RNTO" },

/* TM_CMD_ABOR: ABOR <CRLF> */
    { tfFtpCmdFuncAbor, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpAborReply_2xx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "ABOR" },

/* TM_CMD_DELE: DELE <SP> <pathname> <CRLF> */
    { tfFtpCmdFuncDele, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Success,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "DELE" },

/* TM_CMD_RMD: RMD <SP> <pathname> <CRLF> (Remove Directory:) */
    { tfFtpCmdFuncRmd,  { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Success,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "RMD" },

/* TM_CMD_MKD: MKD  <SP> <pathname> <CRLF> (Make Directory:) */
    { tfFtpCmdFuncMkd,  { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_BufCopy,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "MKD" },

/* TM_CMD_PWD: PWD  <CRLF> (Print Directory:) */
    { tfFtpCmdFuncPwd,  { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_BufCopy,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "PWD" },

/* TM_CMD_LIST: LIST [<SP> <pathname>] <CRLF> */
    { tfFtpCmdFuncList,  { tfFtpXxxxReply_xxx,
                           tfFtpListReply_1xx,
                           tfFtpXxxxReply_Success,
                           tfFtpXxxxReply_Error,
                           tfFtpDataReply_Failure,
                           tfFtpDataReply_Failure }, TM_FTP_MATRIX2, "LIST" },

/* TM_CMD_NLST: NLST [<SP> <pathname>] <CRLF> */
    { tfFtpCmdFuncNlst, { tfFtpXxxxReply_xxx,
                          tfFtpNlstReply_1xx,
                          tfFtpXxxxReply_Success,
                          tfFtpXxxxReply_Error,
                          tfFtpDataReply_Failure,
                          tfFtpDataReply_Failure },  TM_FTP_MATRIX2, "NLST" },

/* TM_CMD_SITE: SITE <SP> <string> <CRLF> */
    { tfFtpCmdFuncNone, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Success,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "SITE" },

/* TM_CMD_SYST: SYST <CRLF> (System:) */
    { tfFtpCmdFuncSyst, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_BufCopy,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "SYST" },

/* TM_CMD_STAT: STAT [<SP> <pathname>] <CRLF> */
    { tfFtpCmdFuncNone, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_BufCopy,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "STAT" },

/* TM_CMD_HELP: HELP [<SP> <string>] <CRLF> */
    { tfFtpCmdFuncHelp, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpHelpReply_2xx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "HELP" },

/* TM_CMD_NOOP: NOOP <CRLF> */
    { tfFtpCmdFuncNoop, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Success,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "NOOP" }
#ifdef TM_USE_IPV6
    ,

/* TM_CMD_EPRT: EPRT <SP> <d><net-prt><d><net-addr><d><tcp-port><d> <CRLF> */
    { tfFtpCmdFuncEprt, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpPortReply_2xx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "EPRT" },

/* TM_CMD_EPSV: EPSV <CRLF> */
    { tfFtpCmdFuncPasv, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpPasvReply_2xx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpXxxxReply_Failure },  TM_FTP_MATRIX1, "EPSV" }
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_FTP_SSL
    ,
/* TM_CMD_AUTH */
    { tfFtpCmdFuncAuth, { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpAuthReply_2xx,
                          tfFtpAuthReply_3xx,
                          tfFtpAuthReply_4xx,
                          tfFtpAuthReply_5xx },      TM_FTP_MATRIX1, "AUTH" },
/* TM_CMD_PROT */
    { tfFtpCmdFuncProt, {tfFtpXxxxReply_xxx,
                         tfFtpXxxxReply_Error,
                         tfFtpProtReply_2xx,
                         tfFtpXxxxReply_Error,
                         tfFtpProtReply_4xx,
                         tfFtpProtReply_5xx},        TM_FTP_MATRIX1, "PROT" },
/* TM_CMD_PBSZ */
    { tfFtpCmdFuncPbsz, {tfFtpXxxxReply_xxx,
                         tfFtpXxxxReply_Error,
                         tfFtpPbszReply_2xx,
                         tfFtpXxxxReply_Error,
                         tfFtpXxxxReply_Failure,
                         tfFtpPbszReply_5xx},        TM_FTP_MATRIX1, "PBSZ" },
/* TM_CMD_CCC */
    { tfFtpCmdFuncCcc,  { tfFtpXxxxReply_xxx,
                          tfFtpXxxxReply_Error,
                          tfFtpCccReply_2xx,
                          tfFtpXxxxReply_Error,
                          tfFtpXxxxReply_Failure,
                          tfFtpCccReply_5xx },       TM_FTP_MATRIX1, "CCC"  }
#endif /* TM_USE_FTP_SSL */
};

typedef int (*ttFtpMatrixTable)(ttFtpSessionPtr ftpSessionPtr);


/*
 * const int (*(tlFtpMatrixTable)[TM_FTP_MATRIX_SIZE][TM_FTPE_SIZE]
 *                               [TM_FTPP_SIZE])
 *                          (ttFtpSessionPtr ftpSessionPtr) =
 */

static const ttFtpMatrixTable TM_CONST_QLF
    tlFtpMatrixTable[TM_FTP_MATRIX_SIZE][TM_FTPE_SIZE][TM_FTPP_SIZE] =
{
/*
 * TM_FTP_MATRIX0 used by IDLE, CONNECT, CLOSE
 * These FTP Client commands are not in state diagrams in comments
 */
    {
/* Event: CONNECT                          State:       */
        {   tfFtpMtrxCtrlConnect,       /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop  },           /* TIMEOUT      */

/* Event: CLOSE */
        {   tfFtpMtrxCloseCtrl,         /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop  },           /* TIMEOUT      */

/* Event: COMMAND */
        {   tfFtpMtrxAlready,           /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop  },           /* TIMEOUT      */

/* Event: ABORT */
        {   tfFtpMtrxAlready,           /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop  },           /* TIMEOUT      */

/* Event: CTRL_CONN */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxGetCtrlInfo,       /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop  },           /* TIMEOUT      */

 /* Event: DATA_CONN */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop  },           /* TIMEOUT      */

/* Event: CTRL_RECV */
        {   tfFtpMtrxCtrlRecv,          /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxCtrlReplyRecv,     /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop  },           /* TIMEOUT      */

/* Event: CTRL_IRECV */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop  },           /* TIMEOUT      */

/* Event: DATA_RECV */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: SEND_REQ */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: SEND_COMP */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: CTRL_ERROR */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxCloseCtrl,         /* CONNECT_WAIT */
            tfFtpMtrxCloseCtrl,         /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: DATA_ERROR */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: TIMEOUT */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxCloseCtrl,         /* CONNECT_WAIT */
            tfFtpMtrxCloseCtrl,         /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     }         /* TIMEOUT      */
    },

/*
 * TM_FTP_MATRIX1 used by ABOR, ALLO, DELE, CWD, CDUP, SMNT, HELP,
 * MODE, NOOP, PASV, QUIT, SITE, PORT, SYST, STAT, RMD, MKD, PWD,
 * STRU, and TYPE and modeled in first state diagram in comments
 * also ACCT, EPRT, EPSV, PASS, REIN, REST, RNFR, RNTO, USER modeled
 * in the third, fourth, and fifth state diagrams in the comments
 * These commands involve a control connection only
 */
    {
/* Event: CONNECT                          State:       */
        {   tfFtpMtrxPermDenied,        /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: CLOSE */
        {   tfFtpMtrxCloseCtrl,         /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },

/* Event: COMMAND */
        {   tfFtpMtrxCommand,           /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: ABORT */
        {   tfFtpMtrxInterCommand,      /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxInterCommand,      /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: CTRL_CONN */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: DATA_CONN */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: CTRL_RECV */
        {   tfFtpMtrxCtrlRecv,          /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxCtrlReplyRecv,     /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: CTRL_IRECV */
        {   tfFtpMtrxCtrlRecv,          /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxCtrlReplyRecvIdle, /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: DATA_RECV */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: SEND_REQ */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: SEND_COMP */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: CTRL_ERROR */
        {   tfFtpMtrxCloseCtrl,         /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxCloseCtrl,         /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: DATA_ERROR */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: TIMEOUT */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxCloseCtrl,         /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     }         /* TIMEOUT      */
    },


/*
 * TM_FTP_MATRIX2 used by LIST, and NLST as in the second state
 * diagram in comments
 * These commands use a control and a data connection but no files
 */

    {
/* Event: CONNECT                          State:       */
        {   tfFtpMtrxPermDenied,        /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: CLOSE */
        {   tfFtpMtrxCloseCtrl,         /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: COMMAND */
        {   tfFtpMtrxCommand,           /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: ABORT */
        {   tfFtpMtrxInterCommand,      /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxPermDenied,        /* REPLY_WAIT   */
            tfFtpMtrxPermDenied,        /* DATA_CONN    */
            tfFtpMtrxInterCommand,      /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: CTRL_CONN */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: DATA_CONN */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxDataConnect,       /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: CTRL_RECV */
        {   tfFtpMtrxCtrlRecv,          /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxCtrlReplyRecv,     /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: CTRL_IRECV */
        {   tfFtpMtrxCtrlRecv,          /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxCtrlReplyRecvIdle, /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: DATA_RECV */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxDataPreRecv,       /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxDirDataRecv,       /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: SEND_REQ */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: SEND_COMP */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: CTRL_ERROR */
        {   tfFtpMtrxCloseCtrl,         /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxCloseCtrlData,     /* REPLY_WAIT   */
            tfFtpMtrxCloseCtrlData,     /* DATA_CONN    */
            tfFtpMtrxCloseCtrlData,     /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: DATA_ERROR */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxCloseData,         /* DATA_CONN    */
            tfFtpMtrxCloseData,         /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* TIMEOUT      */

/* Event: TIMEOUT */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxCloseCtrlData,     /* REPLY_WAIT   */
            tfFtpMtrxCloseData,         /* DATA_CONN    */
            tfFtpMtrxCloseData,         /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     }         /* TIMEOUT      */
    },

/*
 * TM_FTP_MATRIX3 used by RETR as in the second state
 * diagram in comments
 * These commands use a control and an incoming data connection
 * and file writes
 */
    {

/* Event: CONNECT                          State:       */
        {   tfFtpMtrxPermDenied,        /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* SEND_WAIT    */

/* Event: CLOSE */
        {   tfFtpMtrxCloseCtrl,         /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* SEND_WAIT    */

/* Event: COMMAND */
        {   tfFtpMtrxCommand,           /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* SEND_WAIT    */

/* Event: ABORT */
        {   tfFtpMtrxInterCommand,      /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxPermDenied,        /* REPLY_WAIT   */
            tfFtpMtrxPermDenied,        /* DATA_CONN    */
            tfFtpMtrxInterCommand,      /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* SEND_WAIT    */

/* Event: CTRL_CONN */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* SEND_WAIT    */

/* Event: DATA_CONN */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxDataConnect,       /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* SEND_WAIT    */

/* Event: CTRL_RECV */
        {   tfFtpMtrxCtrlRecv,          /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxCtrlReplyRecv,     /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* SEND_WAIT    */

/* Event: CTRL_IRECV */
        {   tfFtpMtrxCtrlRecv,          /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxCtrlReplyRecvIdle, /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* SEND_WAIT    */

/* Event: DATA_RECV */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxDataPreRecv,       /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxDataRecv,          /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* SEND_WAIT    */

/* Event: SEND_REQ */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* SEND_WAIT    */

/* Event: SEND_COMP */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* SEND_WAIT    */

/* Event: CTRL_ERROR */
        {   tfFtpMtrxCloseCtrl,         /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxCloseCtrlData,          /* REPLY_WAIT   */
            tfFtpMtrxCloseCtrlData,          /* DATA_CONN    */
            tfFtpMtrxCloseCtrlData,          /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* SEND_WAIT    */

/* Event: DATA_ERROR */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxCloseFileData,     /* DATA_CONN    */
            tfFtpMtrxCloseFileData,     /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* SEND_WAIT    */

/* Event: TIMEOUT */
        {   tfFtpMtrxNoop,              /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxCloseCtrlData,          /* REPLY_WAIT   */
            tfFtpMtrxCloseFileData,     /* DATA_CONN    */
            tfFtpMtrxCloseFileData,     /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     }         /* SEND_WAIT    */
    },

/*
 * TM_FTP_MATRIX4 used by APPE, STOR, and STOU as in the second state
 * diagram in comments
 * These commands use a control and an outgoing data connection
 * and file reads
 */
    {

/* Event: CONNECT                          State:       */
        {   tfFtpMtrxPermDenied,        /* INITIAL      */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* SEND_WAIT    */

/* Event: CLOSE */
        {   tfFtpMtrxCloseCtrl,         /* INTIAL       */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* SEND_WAIT    */

/* Event: COMMAND */
        {   tfFtpMtrxCommand,           /* INTIAL       */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* SEND_WAIT    */

/* Event: ABORT */
        {   tfFtpMtrxInterCommand,      /* INTIAL       */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxPermDenied,        /* REPLY_WAIT   */
            tfFtpMtrxPermDenied,        /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxCloseFileData,     /* DATA_SEND    */
            tfFtpMtrxCloseFileData  },  /* SEND_WAIT    */

/* Event: CTRL_CONN */
        {   tfFtpMtrxNoop,              /* INTIAL       */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* SEND_WAIT    */

/* Event: DATA_CONN */
        {   tfFtpMtrxNoop,              /* INTIAL       */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxDataConnect,       /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* SEND_WAIT    */

/* Event: CTRL_RECV */
        {   tfFtpMtrxCtrlRecv,          /* INTIAL       */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxCtrlReplyRecv,     /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* SEND_WAIT    */

/* Event: CTRL_IRECV */
        {   tfFtpMtrxCtrlRecv,          /* INTIAL       */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxCtrlReplyRecvIdle, /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* SEND_WAIT    */

/* Event: DATA_RECV */
        {   tfFtpMtrxNoop,              /* INTIAL       */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* SEND_WAIT    */

/* Event: SEND_REQ */
        {   tfFtpMtrxNoop,              /* INTIAL       */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxDataSend,          /* DATA_SEND    */
            tfFtpMtrxNoop     },        /* SEND_WAIT    */

/* Event: SEND_COMP */
        {   tfFtpMtrxNoop,              /* INTIAL       */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxNoop,              /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxSendComplete  },   /* SEND_WAIT    */

/* Event: CTRL_ERROR */
        {   tfFtpMtrxCloseCtrl,         /* INTIAL       */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxCloseCtrl,         /* REPLY_WAIT   */
            tfFtpMtrxCloseCtrl,         /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxCloseCtrlData  },       /* SEND_WAIT    */

/* Event: DATA_ERROR */
        {   tfFtpMtrxNoop,              /* INTIAL       */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxNoop,              /* REPLY_WAIT   */
            tfFtpMtrxCloseFileData,     /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxCloseFileData  },  /* SEND_WAIT    */

/* Event: TIMEOUT */
        {   tfFtpMtrxNoop,              /* INTIAL       */
            tfFtpMtrxNoop,              /* CONNECT_WAIT */
            tfFtpMtrxCloseCtrlData,          /* REPLY_WAIT   */
            tfFtpMtrxCloseFileData,     /* DATA_CONN    */
            tfFtpMtrxNoop,              /* DATA_RECV    */
            tfFtpMtrxNoop,              /* DATA_SEND    */
            tfFtpMtrxCloseFileData  }   /* SEND_WAIT    */
    }
};


/*
 * FTP Matrix Event Table
 */
static const tt16Bit  TM_CONST_QLF tlFtpMatrixEventTable
                                          [TM_FTP_MATRIX_SIZE][TM_FTPP_SIZE] =
{
/* TM_FTP_MATRIX0 */
    {
             TM_FTPS_CMDREQ,
             TM_FTPS_CTRLCONN | TM_FTPS_CTRLERRO | TM_FTPS_TIMEOUT,
             TM_FTPS_CTRLRECV | TM_FTPS_CTRLERRO | TM_FTPS_TIMEOUT,
             0,
             0,
             0,
             0
    },

/* TM_FTP_MATRIX1 */
    {
             TM_FTPS_CMDREQ,
             TM_FTPS_CTRLCONN | TM_FTPS_CTRLERRO | TM_FTPS_TIMEOUT,
             TM_FTPS_CTRLRECV | TM_FTPS_CTRLERRO | TM_FTPS_TIMEOUT,
             0,
             0,
             0,
             0
    },

/* TM_FTP_MATRIX2 */
    {
             TM_FTPS_CMDREQ,
             0,
             TM_FTPS_CMDREQ | TM_FTPS_CTRLRECV |
                TM_FTPS_CTRLERRO | TM_FTPS_TIMEOUT,
             TM_FTPS_CMDREQ | TM_FTPS_DATACONN | TM_FTPS_CTRLERRO |
                TM_FTPS_DATAERRO | TM_FTPS_TIMEOUT,
             TM_FTPS_CMDREQ | TM_FTPS_DATARECV | TM_FTPS_CTRLERRO |
                TM_FTPS_DATAERRO | TM_FTPS_TIMEOUT,
             0,
             0
    },

/* TM_FTP_MATRIX3 */
    {
             TM_FTPS_CMDREQ,
             0,
             TM_FTPS_CMDREQ | TM_FTPS_CTRLRECV |
                TM_FTPS_CTRLERRO | TM_FTPS_TIMEOUT,
             TM_FTPS_CMDREQ | TM_FTPS_DATACONN | TM_FTPS_CTRLERRO |
                TM_FTPS_DATAERRO | TM_FTPS_TIMEOUT,
             TM_FTPS_CMDREQ | TM_FTPS_DATARECV | TM_FTPS_CTRLERRO |
                TM_FTPS_DATAERRO | TM_FTPS_TIMEOUT,
             0,
             0
    },

/* TM_FTP_MATRIX4 */
    {
             TM_FTPS_CMDREQ,
             0,
             TM_FTPS_CMDREQ | TM_FTPS_CTRLRECV | TM_FTPS_CTRLERRO |
                TM_FTPS_TIMEOUT,
             TM_FTPS_CMDREQ | TM_FTPS_DATACONN | TM_FTPS_CTRLERRO |
                TM_FTPS_DATAERRO | TM_FTPS_TIMEOUT,
             0,
             TM_FTPS_CMDREQ | TM_FTPS_SENDREQ,
             TM_FTPS_CMDREQ | TM_FTPS_SENDCOMP | TM_FTPS_CTRLERRO |
                TM_FTPS_DATAERRO | TM_FTPS_TIMEOUT

    }
};

/*
 * tfFtpNewSession description:
 * Creates a new FTP client session: allocate and initialize buffer for state
 * vector, login to filesystem and start FTP timer.
 *
 * Parameters:
 *  Value               Meaning
 *  fileSystemFlags     Flags for file structure supported by the system
 *  blockingState       Blocking or non blocking
 *                        (TM_BLOCKING_ON or TM_BLOCKING_OFF)
 *
 * Returns:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to newly allocated FTP session structure.
 *  (void TM_FAR *)0    Null pointer on failure
 */
ttUserFtpHandle tfFtpNewSession (int           fileSystemFlags,
                                 int           blockingState,
                                 char TM_FAR * fsUsernamePtr,
                                 char TM_FAR * fsPasswordPtr)
{
    ttFtpSessionPtr ftpSessionPtr;
    ttGenericUnion  timerParm1;
    tt8Bit          newSession;

    TM_UNREF_IN_ARG(fileSystemFlags);
    newSession = TM_8BIT_NO;
    ftpSessionPtr = (ttFtpSessionPtr)0;
    if ((blockingState == TM_BLOCKING_ON) ||
        (blockingState == TM_BLOCKING_OFF) )
    {
/* Get buffer for FTP session information */
        ftpSessionPtr = (ttFtpSessionPtr)tm_get_raw_buffer(
            sizeof(ttFtpSession));

        if (ftpSessionPtr != (ttFtpSessionPtr) 0)
        {
/* Set FTP session structure to initial state */
            tfFtpDataDefault( ftpSessionPtr, blockingState );
/*
 * Log in to file system and retrieve handle to filesystem.  If login fails,
 * return NULL pointer (indicating failure) to user.
 */
            ftpSessionPtr->ftpUserDataPtr = tfFSUserLogin (fsUsernamePtr,
                                                           fsPasswordPtr);
            if (ftpSessionPtr->ftpUserDataPtr != 0)
            {
                timerParm1.genVoidParmPtr = (ttVoidPtr) ftpSessionPtr;
                ftpSessionPtr->ftpTimerBlockPtr =
                    tfTimerAdd (tfFtpTimerCBFunction,
                                timerParm1,
                                timerParm1, /* unused */
                                TM_FTP_TIMER_VALUE,
                                TM_TIM_AUTO );

                if (ftpSessionPtr->ftpTimerBlockPtr != (ttTimerPtr)0)
                {
                    newSession = TM_8BIT_YES;
                }
            }
        }
    }
    if (    (newSession == TM_8BIT_NO)
         && (ftpSessionPtr != (ttFtpSessionPtr)0) )
    {
        tfFtpFreeSession((ttUserFtpHandle)ftpSessionPtr);
        ftpSessionPtr = (ttFtpSessionPtr)0;
    }
    return (ttUserFtpHandle) ftpSessionPtr;
}

/*
 * tfFtpFreeSession description:
 * Decontructs FTP session: Closes control connection, logs out of filesystem,
 * removes timer and frees the state vector.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpUserHandle       Pointer to FTP session handle
 *
 * Returns:
 *  Value               Meaning
 *  TM_EINVAL           Invalid ftpUserHandle
 *  TM_ENOERROR         Success
 */
int tfFtpFreeSession (ttUserFtpHandle ftpUserHandle)
{
#define ftpSessionPtr ((ttFtpSessionPtr) ftpUserHandle)
    int result;

    if (ftpUserHandle != (ttUserFtpHandle) 0)
    {
/* Close data file, data and listening sockets if any */
        tfFtpCloseData(ftpSessionPtr);
#ifdef TM_USE_FTP_SSL
        if (ftpSessionPtr->ftpSslSessionId >= 0)
        {
/* Close the SSL session */
            tfSslSessionUserClose(ftpSessionPtr->ftpSslSessionId);
        }
#endif /* TM_USE_FTP_SSL */

/* Logout of filesystem */
        if (ftpSessionPtr->ftpUserDataPtr != (ttVoidPtr)0)
        {
            tfFSUserLogout (ftpSessionPtr->ftpUserDataPtr);
        }

/* Remove timer */
        if (ftpSessionPtr->ftpTimerBlockPtr != (ttTimerPtr)0)
        {
            tfTimerRemove(ftpSessionPtr->ftpTimerBlockPtr);
        }
/* Close control connection */
        tfFtpCloseCtrlSocket(ftpSessionPtr);
/* Free state vector */
        tm_free_raw_buffer((ttRawBufferPtr) ftpSessionPtr);

        result = TM_ENOERROR;
    }
    else
    {
        result = TM_EINVAL;
    }

    return (result);

#undef  ftpSessionPtr
}


/*
 * tfFtpTurnPort description:
 *   Operation currently not supported; always returns TM_EOPNOTSUPP.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  onFlag              set to 1, means turn off Port usage
 *                      set to 0, means turn on Port usage.
 *
 * Returns:
 *  Value               Meaning
 *  TM_EOPNOTSUPP       Command not supported by the user
 */
int tfFtpTurnPort (ttUserFtpHandle ftpUserHandle, int onFlag)
{
    int result;

    TM_UNREF_IN_ARG(ftpUserHandle);
    onFlag        = onFlag;

    result = TM_EOPNOTSUPP;
    return (result);
}


/*
 * tfFtpTurnPasv description:
 *   Turn on/off FTP passive mode.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  onFlag              set to 1, means turn off Pasv usage.
 *                      set to 0, means turn on Pasv usage.
 *
 * Returns:
 *  Value               Meaning
 *  TM_EINVAL           Invalid ftpUserHandle or onFlag
 *  TM_ENOERROR (0)     Success
 */
int tfFtpTurnPasv (ttUserFtpHandle ftpUserHandle, int onFlag)
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;

    if (ftpUserHandle != (ttUserFtpHandle) 0)
    {
        result = TM_ENOERROR;
        ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;

        switch( onFlag )
        {
        case TM_FTP_PASSIVE_MODE_ON:
/* Set the flag to indicate that session is in passive mode */
            ftpSessionPtr->ftpFlags |= TM_FTP_F_DATA_PASV;
            break;

        case TM_FTP_PASSIVE_MODE_OFF:
/* Clear the flag to indicate that session is not in passive mode */
            tm_16bit_clr_bit(ftpSessionPtr->ftpFlags, TM_FTP_F_DATA_PASV);
            break;

        default:
            result = TM_EINVAL;
            break;
        }
    }
    else
    {
        result = TM_EINVAL;
    }

    return result;
}


/*
 * tfFtpClose description:
 *   Close an FTP session.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EINVAL           Invalid argument (ftpUserHandle).
 *  TM_ENOERROR         Success
 */
int tfFtpClose (ttUserFtpHandle ftpUserHandle)
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;
    if (ftpUserHandle != (ttUserFtpHandle) 0)
    {

/* command = CLOSE, next = IDLE */
        tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_CLOS, TM_CMD_IDLE);
        ftpSessionPtr->ftpCmdRequest = TM_CMD_CLOS;
        ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_CLOSE;
        result = tfFtpMatrix (ftpSessionPtr);
    }
    else
    {
        result = TM_EINVAL;
    }

    return (result);
}


#ifdef TM_USE_IPV4
/*
 * tfFtpConnect description:
 *   Connect to remote FTP server.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  ipAddressPtr        pointer to an IP address (i.e "128.56.10.1")
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EALREADY         command in progress (earlier call did not finish).
 *  TM_EACCES           Trying to connect to a different server without
 *                      disconnecting first.
 *  TM_EINVAL           parameter invalid.
 *  TM_ENOERROR         Success.
 */
int tfFtpConnect (ttUserFtpHandle ftpUserHandle, char TM_FAR * ipAddressPtr)
{
    int                         result;
    ttFtpSessionPtr             ftpSessionPtr;
    struct sockaddr_in TM_FAR * ftpAddrPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;
    if (ftpUserHandle != (ttUserFtpHandle) 0)
    {
        if (ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET] == TM_SOCKET_ERROR)
        {

/* Save the server's IP address in the state vector */
            ftpAddrPtr = &ftpSessionPtr->ftpClientCtrlAddress.addr.ipv4;

/* tfFtpConnect assumes IPv4 */
            ftpAddrPtr->sin_family      = AF_INET;
            ftpAddrPtr->sin_addr.s_addr = inet_addr(ipAddressPtr);
            ftpAddrPtr->sin_len         = sizeof(struct sockaddr_in);

/* command = CONNECT, next = IDLE */
            tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_CONN, TM_CMD_IDLE);

/* Kick the state machine */
            ftpSessionPtr->ftpCmdRequest = TM_CMD_CONN;
            ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_CONNECT;
            result = tfFtpMatrix (ftpSessionPtr);
        }
        else
        {
            result = TM_EACCES;
        }
    }
    else
    {
        result = TM_EINVAL;
    }

    return (result);
}
#endif /* TM_USE_IPV4 */

/*
 * tfFtpLogin description:
 *   Logins to a remote FTP server.
 *
 * FTP Command          USER,PASS,(ACCT)
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  userNamePtr         Pointer to User name string (could be null).
 *  passwordNamePtr     Pointer to password name string (coud be null).
 *  accountNamePtr      Pointer to account name string (could be null).
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_ENOERROR         Success.
 */
int tfFtpLogin ( ttUserFtpHandle ftpUserHandle,
                 char TM_FAR *   userNamePtr,
                 char TM_FAR *   passwordNamePtr,
                 char TM_FAR *   accountNamePtr)
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;
    result = tfFtpCheckSession (ftpSessionPtr, TM_FTP_F_CTRL_CONN);
    if (result == TM_ENOERROR)
    {

/* set username/password info in session structure */
        ftpSessionPtr->ftpUserNamePtr         = userNamePtr;
        ftpSessionPtr->ftpUserPasswordNamePtr = passwordNamePtr;
        ftpSessionPtr->ftpUserAccountNamePtr  = accountNamePtr;

/* command = USER, next = IDLE */
        tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_USER, TM_CMD_IDLE);

/* kick the state machine */
        ftpSessionPtr->ftpCmdRequest = TM_CMD_USER;
        ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
        result = tfFtpMatrix (ftpSessionPtr);
    }

    return (result);
}

/*
 * tfFtpCwd description:
 *   Change current working directory (ala CD).
 *
 * FTP Command          CWD  <SP> <pathname> <CRLF>
 * Command reply        250
 *                      500, 501, 502, 421, 530, 550
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  pathNamePtr         Pointer to a new directory
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpCwd (ttUserFtpHandle ftpUserHandle, char TM_FAR * pathNamePtr)
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;
/* Check validity of command (must be connected, logged in, good param) */
    result = tfFtpCheckCommand(ftpSessionPtr, TM_CMD_CWD, pathNamePtr,
                               TM_USER_F_PARM |
                                 TM_FTP_F_CTRL_CONN |
                                 TM_FTP_F_LOGGED_IN );

    if (result == TM_ENOERROR)
    {

/* Sets session pathname parameter */
        ftpSessionPtr->ftpPathNamePtr = pathNamePtr;

/* command = CWD, next = IDLE */
        tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_CWD, TM_CMD_IDLE);

/* kick the state machine */
        ftpSessionPtr->ftpCmdRequest = TM_CMD_CWD;
        ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
        result = tfFtpMatrix (ftpSessionPtr);
    }

    return (result);
}


/*
 * tfFtpCdup description:
 *  Changes to paranet directory (ala, 'cd ..').
 *
 * FTP Command          CDUP <CRLF> (Change to parent directory:)
 * Command reply        250
 *                      500, 501, 502, 421, 530, 550
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpCdup (ttUserFtpHandle ftpUserHandle)
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;
/* Check validity of command (must be connected, logged in) */
    result = tfFtpCheckCommand (ftpSessionPtr, TM_CMD_CDUP, (ttCharPtr) "",
                                TM_FTP_F_CTRL_CONN | TM_FTP_F_LOGGED_IN );

    if (result == TM_ENOERROR)
    {

/* command = CDUP, next = IDLE */
        tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_CDUP, TM_CMD_IDLE);

/* kick the state machine */
        ftpSessionPtr->ftpCmdRequest = TM_CMD_CWD;
        ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
        result = tfFtpMatrix (ftpSessionPtr);
    }

    return (result);
}


/*
 * tfFtpSmnt description:
 *   Structure mount - not currently supported.
 *
 * FTP Command          SMNT <SP> <pathname> <CRLF> (Structure Mount:)
 * Command reply        202, 250
 *                      500, 501, 502, 421, 530, 550
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  mountPathPtr        Pointer to mount path name string.
 *
 * Returns:
 *  Value               Meaning
 *  TM_EOPNOTSUPP       Command not currently supported.
 */
int tfFtpSmnt (ttUserFtpHandle ftpUserHandle,
               char TM_FAR     *mountPathPtr)
{
    TM_UNREF_IN_ARG(ftpUserHandle);
    mountPathPtr  = mountPathPtr;

    return (TM_EOPNOTSUPP);
}


/*
 * tfFtpRein description:
 *   Reinitializes the FTP session.
 *
 * FTP Command          REIN <CRLF>
 * Command reply        120
 *                          220
 *                      220
 *                      421
 *                      500, 502
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpRein (ttUserFtpHandle ftpUserHandle)
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;
/* Check validity of command (must be connected, logged in) */
    result = tfFtpCheckCommand (ftpSessionPtr, TM_CMD_REIN, ( ttCharPtr )"",
                                TM_FTP_F_CTRL_CONN | TM_FTP_F_LOGGED_IN );

    if (result == TM_ENOERROR)
    {

/* command = REIN, next = IDLE */
        tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_REIN, TM_CMD_IDLE);

/* Kick the state machine */
        ftpSessionPtr->ftpCmdRequest = TM_CMD_REIN;
        ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
        result = tfFtpMatrix (ftpSessionPtr);
    }

    return (result);
}


/*
 * tfFtpQuit description:
 *   Starts termination of the FTP connection.
 *
 * FTP Command          QUIT <CRLF>
 * Command reply        221
 *                      500
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Outstanding ABORT/QUIT/STAT command has not finished
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpQuit (ttUserFtpHandle ftpUserHandle)
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;
/* Check validity of command (must be connected). */
    result = tfFtpCheckInterSession (ftpSessionPtr, TM_FTP_F_CTRL_CONN);

    if (result == TM_ENOERROR)
    {

/* command = QUIT, next = IDLE */
        tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_QUIT, TM_CMD_IDLE);

/*
 * If the FTP client is not currently executing another command (such as RETR
 * or STOR) set the command to QUIT.  This prevents the client  from becoming
 * stuck in an IDLE state if the server sends a multi-line  response to the
 * QUIT command.
 */
        if (ftpSessionPtr->ftpCommandCode == TM_CMD_IDLE)
        {
            ftpSessionPtr->ftpCommandCode = TM_CMD_QUIT;
        }

/* kick the state machine */
        ftpSessionPtr->ftpCmdRequest = TM_CMD_QUIT;
        ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_ABORT;
        result = tfFtpMatrix (ftpSessionPtr);
    }

    if(result == TM_ESHUTDOWN)
    {
        result = TM_ENOERROR;
    }

    return (result);
}

/*
 * tfFtpPort description:
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  portNo              Poer no.
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpPort (ttUserFtpHandle ftpUserHandle, ttIpPort ftpPortNo)
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;
/* Check validity of command (must be connected, logged in) */
    result = tfFtpCheckSession (ftpSessionPtr,
                                TM_FTP_F_CTRL_CONN | TM_FTP_F_LOGGED_IN);

    if (result == TM_ENOERROR)
    {

/* If the port number is valid, set the port number in the session structure */
        if ((tt16Bit) ftpPortNo < TM_WILD_PORT)
        {
            ftpSessionPtr->ftpPortParm = ftpPortNo;

            ftpSessionPtr->ftpPortNo    = ftpSessionPtr->ftpPortParm;
            ftpSessionPtr->ftpNowPortNo = ftpSessionPtr->ftpPortNo;

            ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
            ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

            result = TM_ENOERROR;
        }
        else
        {
            result = TM_EINVAL;
        }
    }

    return (result);
}


/*
 * tfFtpType function description:
 *   Sets the transfer type, ASCII or BINARY.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  type                One of TM_TYPE_ASCII, or TM_TYPE_BINARY
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpType(ttUserFtpHandle ftpUserHandle, int type)
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;
/* Currently only the ASCII and BINARY transfer types are supported */
    if ((type == TM_TYPE_ASCII) || (type == TM_TYPE_BINARY))
    {

/* Check validity of command (must be connected, logged in) */
        result = tfFtpCheckSession (ftpSessionPtr,
                                    TM_FTP_F_CTRL_CONN | TM_FTP_F_LOGGED_IN);

        if (result == TM_ENOERROR)
        {

            ftpSessionPtr->ftpTransParm = type;

/* command = TYPE, next = IDLE */
            tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_TYPE, TM_CMD_IDLE);

/* kick the state machine */
            ftpSessionPtr->ftpCmdRequest = TM_CMD_TYPE;
            ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
            result = tfFtpMatrix (ftpSessionPtr);
        }
    }
    else
    {

/* Types not ASCII or BINARY are not supported */
        result = TM_EINVAL;
    }

    return (result);

#undef ftpSessionPtr
}


/*
 * tfFtpStru  function description:
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  structure           One of TM_STRU_STREAM, or TM_STRU_RECORD
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpStru (ttUserFtpHandle ftpUserHandle, int structure)
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;
    if (structure == TM_STRU_STREAM)
    {

/* Check validity of command (must be connected, logged in) */
        result = tfFtpCheckSession (ftpSessionPtr,
                                    TM_FTP_F_CTRL_CONN | TM_FTP_F_LOGGED_IN );

        if (result == TM_ENOERROR)
        {
            ftpSessionPtr->ftpTransParm = structure;

/* command = STRU, next = IDLE */
            tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_STRU, TM_CMD_IDLE);

/* kick the state machine */
            ftpSessionPtr->ftpCmdRequest = TM_CMD_STRU;
            ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
            result = tfFtpMatrix (ftpSessionPtr);
        }
    }
    else
    {
        result = TM_EINVAL;
    }

    return (result);
}


/*
 * tfFtpMode  function description:
 *   Sets the transfer mode for this session (only type STREAM is currently
 *   supported).
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  mode                TM_MODE_STREAM or TM_MODE_BLOCK or TM_MODE_COMPRESSED
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpMode (ttUserFtpHandle ftpUserHandle, int mode)
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;
/* Currently only a mode of STREAM is supported */
    if ( mode == TM_MODE_STREAM )
    {

/* Check validity of command (must be connected, logged in)( */
        result = tfFtpCheckSession (ftpSessionPtr,
                                    TM_FTP_F_CTRL_CONN | TM_FTP_F_LOGGED_IN);

        if (result == TM_ENOERROR)
        {

            ftpSessionPtr->ftpTransParm = mode;

/* command = MODE, next = IDLE */
            tfFtpSetCommandCode ( ftpSessionPtr, TM_CMD_MODE, TM_CMD_IDLE );

/* Kick the state machine */
            ftpSessionPtr->ftpCmdRequest = TM_CMD_MODE;
            ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
            result = tfFtpMatrix (ftpSessionPtr);
        }
    }
    else
    {
        result = TM_EINVAL;
    }

    return (result);
}


/*
 * tfFtpRetr description:
 *   Retrieves a file from the remote FTP server.
 *
 * FTP Command          RETR <SP> <pathname> <CRLF>
 * Command reply        125, 150
 *                          (110)
 *                          226, 250
 *                          425, 426, 451, 551, 552
 *                      532, 450, 452, 553
 *                      500, 501, 421, 530
 *
 * Parameters:
 *  Value               Meaning
 *  ftpUserHandleA      FTP session handle
 *  ftpUserHandleB      The 2nd FTP session handle, this parameter is needed
 *                      for passive mode:
 *                          0         : one FTP server model;
 *                          otherwise : two FTP server model.
 *  fromFileNamePtr     Pointer to source file name string
 *  toFileNamePtr       Pointer to destination file name string
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr or bad filename.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpRetr ( ttUserFtpHandle             ftpUserHandleA,
                ttUserFtpHandle             ftpUserHandleB,
                char TM_FAR *               fromFileNamePtr,
                char TM_FAR *               toFileNamePtr)
{
    int             result;
    ttFtpSessionPtr ftpSessionAPtr;
    ttFtpSessionPtr ftpSessionBPtr;

    ftpSessionAPtr = (ttFtpSessionPtr)ftpUserHandleA;
    ftpSessionBPtr = (ttFtpSessionPtr)ftpUserHandleB;

#ifdef TM_USE_FTP_SSL
    if ( (ftpSessionAPtr->ftpSslDataConnReq == TM_8BIT_YES)
      && (ftpSessionAPtr->ftpSslEnabledDataConn == TM_8BIT_NO) )
    {
/*
 * The server requires that a security data exchange be completed
 * first. The user must secure the data conn before calling this function.
 */
        result = TM_FTP_BADCMDSEQ;
    }
    else
#endif /* TM_USE_FTP_SSL */
    {
        result = tfFtpCommonDataCommand( TM_CMD_RETR,
                                         ftpSessionAPtr,
                                         ftpSessionBPtr,
                                         fromFileNamePtr,
                                         toFileNamePtr,
                                         TM_FTP_CMD_F_FROM_SERVER_A );
    }

    return (result);
}


/*
 * tfFtpStor description:
 *   Transmit file to remote FTP system.
 *
 * FTP Command          STOR <SP> <pathname> <CRLF>
 * Command reply        125, 150
 *                          (110)
 *                          226, 250
 *                          425, 426, 451, 551, 552
 *                      532, 450, 452, 553
 *                      500, 501, 421, 530
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  portAddrPtr         Pointer to socket address structure containing
 *                      a server port address if transfer is to take
 *                      place between our server, and another remote server,
 *                      otherwise this pointer should be null.
 *  localFileNamePtr    Pointer to local file name string
 *  remoteFileNamePtr   Pointer to server file name string.
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr or bad filename.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpStor ( ttUserFtpHandle             ftpUserHandleA,
                ttUserFtpHandle             ftpUserHandleB,
                char TM_FAR *               fromFileNamePtr,
                char TM_FAR *               toFileNamePtr)
{
    int             result;
    ttFtpSessionPtr ftpSessionAPtr;
    ttFtpSessionPtr ftpSessionBPtr;

    ftpSessionAPtr = (ttFtpSessionPtr)ftpUserHandleA;
    ftpSessionBPtr = (ttFtpSessionPtr)ftpUserHandleB;

#ifdef TM_USE_FTP_SSL
    if ( (ftpSessionAPtr->ftpSslDataConnReq == TM_8BIT_YES)
      && (ftpSessionAPtr->ftpSslEnabledDataConn == TM_8BIT_NO) )
    {
/*
 * The server requires that a security data exchange be completed
 * first. The user must secure the data conn before calling this function.
 */
        result = TM_FTP_BADCMDSEQ;
    }
    else
#endif /* TM_USE_FTP_SSL */
    {
        result = tfFtpCommonDataCommand( TM_CMD_STOR,
                                         ftpSessionAPtr,
                                         ftpSessionBPtr,
                                         fromFileNamePtr,
                                         toFileNamePtr,
                                         TM_FTP_CMD_F_TO_SERVER_A );

    }
    return (result);
}


/*
 * tfFtpStou description:
 *   Not currently supported.
 *
 * FTP Command          STOU <CRLF>
 * Command reply        125, 150
 *                          (110)
 *                          226, 250
 *                          425, 426, 451, 551, 552
 *                      532, 450, 452, 553
 *                      500, 501, 421, 530
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  portAddrPtr         Pointer to socket address structure containing
 *                      a server port address if transfer is to take
 *                      place between our server, and another remote server,
 *                      otherwise this pointer should be null.
 *  localFileNamePtr    Pointer to local file name string
 *  bufferPtr           Pointer to user allocated buffer where to store
 *                      server filename string.
 *  bufferSize          size of bufferPtr.
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpStou ( ttUserFtpHandle             ftpUserHandleA,
                ttUserFtpHandle             ftpUserHandleB,
                char TM_FAR *               fromFileNamePtr,
                char TM_FAR *               toFileNamePtr,
                int                         bufferSize )
{
    ftpUserHandleA    = ftpUserHandleA;
    ftpUserHandleB    = ftpUserHandleB;
    fromFileNamePtr   = fromFileNamePtr;
    toFileNamePtr     = toFileNamePtr;
    bufferSize        = bufferSize;

    return (TM_EOPNOTSUPP);
}


/*
 * tfFtpAppe description:
 *   Append data to specified file.
 *
 * FTP Command          APPE <SP> <pathname> <CRLF>
 * Command reply        125, 150
 *                          (110)
 *                          226, 250
 *                          425, 426, 451, 551, 552
 *                      532, 450, 452, 553
 *                      500, 501, 421, 530
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  portAddrPtr         Pointer to socket address structure containing
 *                      a server port address if transfer is to take
 *                      place between our server, and another remote server,
 *                      otherwise this pointer should be null.
 *  localFileNamePtr    Pointer to local file name string
 *  remoteFileNamePtr   Pinter to server file name string.
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpAppe ( ttUserFtpHandle             ftpUserHandleA,
                ttUserFtpHandle             ftpUserHandleB,
                char TM_FAR *               fromFileNamePtr,
                char TM_FAR *               toFileNamePtr)
{
    int             result;
    ttFtpSessionPtr ftpSessionAPtr;
    ttFtpSessionPtr ftpSessionBPtr;

    ftpSessionAPtr = (ttFtpSessionPtr)ftpUserHandleA;
    ftpSessionBPtr = (ttFtpSessionPtr)ftpUserHandleB;

#ifdef TM_USE_FTP_SSL
    if ( (ftpSessionAPtr->ftpSslDataConnReq == TM_8BIT_YES)
      && (ftpSessionAPtr->ftpSslEnabledDataConn == TM_8BIT_NO) )
    {
/*
 * The server requires that a security data exchange be completed
 * first. The user must secure the data conn before calling this function.
 */
        result = TM_FTP_BADCMDSEQ;
    }
    else
#endif /* TM_USE_FTP_SSL */
    {
        result = tfFtpCommonDataCommand( TM_CMD_APPE,
                                         ftpSessionAPtr,
                                         ftpSessionBPtr,
                                         fromFileNamePtr,
                                         toFileNamePtr,
                                         TM_FTP_CMD_F_TO_SERVER_A );
    }
    return result;
}


/*
 * tfFtpAllo description:
 *   currently not supported.
 *
 * FTP Command          ALLO <SP> <decimal-integer>
 *                        [<SP> R <SP> <decimal-integer>] <CRLF>
 * Command reply        200
 *                      202
 *                      500, 501, 504, 421, 530
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  sizeBytes           number of bytes or records to allocate on the server
 *                      for a file transfer.
 *  recordSizeBytes     record size in bytes for record structure transfer.
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpAllo ( ttUserFtpHandle ftpUserHandle,
                ttUser32Bit     sizeBytes,
                int             recordSizeBytes )
{
    ftpUserHandle   = ftpUserHandle;
    sizeBytes       = sizeBytes;
    TM_UNREF_IN_ARG(recordSizeBytes);

    return (TM_EOPNOTSUPP);
}


/*
 * tfFtpRename description:
 *
 * FTP Command          RNFR, RNTO
 *
 * FTP Command          RNFR <SP> <pathname> <CRLF>
 * Command reply        450, 550
 *                      500, 501, 502, 421, 530
 *                      350
 *
 * FTP Command          RNTO <SP> <pathname> <CRLF>
 * Command reply        250
 *                      532, 553
 *                      500, 501, 502, 503, 421, 530
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  fromNamePtr         Pointer to file name string to rename
 *  toNamePtr           Pointer to new file name string
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpRename ( ttUserFtpHandle ftpUserHandle,
                  char TM_FAR *   fromNamePtr,
                  char TM_FAR *   toNamePtr )
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;

/* Verify that the user passed in a correct buffer and that the user is
 * connected and logged in.
 */
    result = tfFtpCheckBufferCommand (ftpSessionPtr,
                                      TM_CMD_RNFR,
                                      fromNamePtr,
                                      TM_USER_F_PARM | TM_FTP_F_CTRL_CONN |
                                        TM_FTP_F_LOGGED_IN,
                                      toNamePtr,
                                      1 );
    if (result == TM_ENOERROR)
    {

        ftpSessionPtr->ftpFromNamePtr = fromNamePtr;
        ftpSessionPtr->ftpToNamePtr   = toNamePtr;

/* command = RNFR, next = IDLE */
        tfFtpSetCommandCode ( ftpSessionPtr, TM_CMD_RNFR, TM_CMD_IDLE );

/* kick the state machine */
        ftpSessionPtr->ftpCmdRequest = TM_CMD_RNFR;
        ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
        result = tfFtpMatrix (ftpSessionPtr);
    }

    return (result);
}


/*
 * tfFtpAbor description:
 *   Aborts the current FTP operation.
 *
 * FTP Command          ABOR <CRLF>
 * Command reply        225, 226
 *                      500, 501, 502, 421
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Outstanding ABORT/QUIT/STAT command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpAbor (ttUserFtpHandle ftpUserHandle)
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;

/* Check validity of command (must be connected) */
    result = tfFtpCheckInterSession (ftpSessionPtr, TM_FTP_F_CTRL_CONN);

    if (result == TM_ENOERROR)
    {

/* command = ABOR, next = IDLE */
        tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_ABOR, TM_CMD_IDLE);

/* kick the state machine */
        ftpSessionPtr->ftpCmdRequest = TM_CMD_ABOR;
        ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_ABORT;
        result = tfFtpMatrix (ftpSessionPtr);
    }

    return (result);
}


/*
 * tfFtpDele description:
 *   Removes a file from the remote FTP system.
 *
 * FTP Command          DELE <SP> <pathname> <CRLF>
 * Command reply        250
 *                      450, 550
 *                      500, 501, 502, 421, 530
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  pathNamePtr         Pointer to file name to delete
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpDele (ttUserFtpHandle ftpUserHandle, char TM_FAR * pathNamePtr)
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;

/* Check validity of command (must be connected, logged in, filename must be
 * specified.
 */
    result = tfFtpCheckCommand (ftpSessionPtr, TM_CMD_DELE, pathNamePtr,
                                TM_USER_F_PARM | TM_FTP_F_CTRL_CONN |
                                  TM_FTP_F_LOGGED_IN );
    if (result == TM_ENOERROR)
    {

        ftpSessionPtr->ftpPathNamePtr = pathNamePtr;

/* command = DELE, next = IDLE */
        tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_DELE, TM_CMD_IDLE);

/* kick the state machine */
        ftpSessionPtr->ftpCmdRequest = TM_CMD_DELE;
        ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
        result = tfFtpMatrix (ftpSessionPtr);
    }

    return (result);
}


/*
 * tfFtpRmd description:
 *   Removes directory from remote FTP server.
 *
 * FTP Command          RMD <SP> <pathname> <CRLF> (Remove Directory:)
 * Command reply        250
 *                      500, 501, 502, 421, 530, 550
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  directoryPathNamePtr Pointer to directory name to remove.
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */

int tfFtpRmd (ttUserFtpHandle ftpUserHandle,
              char TM_FAR *   directoryPathNamePtr )
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;

/* Check validity of command (must be connected, logged in, directory must be
 * specified
 */
    result = tfFtpCheckCommand (ftpSessionPtr, TM_CMD_RMD, directoryPathNamePtr,
                                TM_USER_F_PARM | TM_FTP_F_CTRL_CONN |
                                  TM_FTP_F_LOGGED_IN );

    if (result == TM_ENOERROR)
    {

        ftpSessionPtr->ftpPathNamePtr = directoryPathNamePtr;

/* command = RMD, next = IDLE */
        tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_RMD, TM_CMD_IDLE);

/* kick the state machine */
        ftpSessionPtr->ftpCmdRequest = TM_CMD_RMD;
        ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
        result = tfFtpMatrix (ftpSessionPtr);
    }

    return (result);
}


/*
 * tfFtpMkd description:
 *   Creates directory on remote FTP server.
 *
 * FTP Command          MKD  <SP> <pathname> <CRLF> (Make Directory:)
 * Command reply        257
 *                      500, 501, 502, 421, 530, 550
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  directoryPathNamePtr Pointer to directory name string to create.
 *  bufferPtr           Pointer to buffer where to store the result (path
 *                      on the remote)
 *  bufferSize          size of the buffer
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpMkd (ttUserFtpHandle ftpUserHandle,
              char TM_FAR *   directoryPathNamePtr,
              char TM_FAR *   bufferPtr,
              int             bufferSize)
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;

/* Check validity of command (must be connected, logged in, directory must be
 * specified.
 */
    result = tfFtpCheckBufferCommand (ftpSessionPtr, TM_CMD_MKD,
                                      directoryPathNamePtr,
                                      TM_USER_F_PARM | TM_FTP_F_CTRL_CONN |
                                        TM_FTP_F_LOGGED_IN,
                                      bufferPtr, bufferSize );
    if (result == TM_ENOERROR)
    {

        ftpSessionPtr->ftpPathNamePtr = directoryPathNamePtr;
        ftpSessionPtr->ftpBufferPtr   = bufferPtr;
        ftpSessionPtr->ftpBufferSize  = bufferSize;
        ftpSessionPtr->ftpBufferIndex = 0;

/* command = MKD, next = IDLE */
        tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_MKD, TM_CMD_IDLE);

/* kick the state machine */
        ftpSessionPtr->ftpCmdRequest = TM_CMD_MKD;
        ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
        result = tfFtpMatrix (ftpSessionPtr);
    }

    return (result);
}


/*
 * tfFtpPwd description:
 *   Retrieve present working directory.
 *
 * FTP Command          PWD <CRLF> (Print Directory:)
 * Command reply        257
 *                      500, 501, 502, 421, 550
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  bufferPtr           Pointer to buffer where to store the result (path
 *                      on the remote)
 *  bufferSize          size of the buffer
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpPwd (ttUserFtpHandle ftpUserHandle,
              char TM_FAR *   bufferPtr,
              int             bufferSize)
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;

/* Check validity of command (must be connected, logged in) */
    result = tfFtpCheckBufferCommand (ftpSessionPtr, TM_CMD_PWD, (ttCharPtr) "",
                                      TM_FTP_F_CTRL_CONN | TM_FTP_F_LOGGED_IN,
                                      bufferPtr, bufferSize);

    if (result == TM_ENOERROR)
    {

        ftpSessionPtr->ftpBufferPtr   = bufferPtr;
        ftpSessionPtr->ftpBufferSize  = bufferSize;
        ftpSessionPtr->ftpBufferIndex = ( int )0;

/* command = PWD, next = IDLE */
        tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_PWD, TM_CMD_IDLE);

/* kick the state machine */
        ftpSessionPtr->ftpCmdRequest = TM_CMD_PWD;
        ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
        result = tfFtpMatrix (ftpSessionPtr);
    }

    return (result);
}


/*
 * tfFtpDirList function description:
 *   Return directory listing.
 *
 * FTP Command          LIST [<SP> <pathname>] <CRLF>
 * Command reply        125, 150
 *                          226, 250
 *                          425, 426, 451
 *                      450
 *                      500, 501, 502, 421, 530
 *
 * FTP Command          NLST [<SP> <pathname>] <CRLF>
 * Command reply        125, 150
 *                          226, 250
 *                          425, 426, 451
 *                      450
 *                      500, 501, 502, 421, 530
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  pathNamePtr         Pointer to directory path name string
 *  directoryFlag       TM_DIR_SHORT (for NLST), TM_DIR_LONG (for LIST)
 *  ftpUserCBFuncPtr    Pointer to a call back function, to be called
 *                      for each directory line. The call back
 *                      function's parameters include ftpSessionPtr, a
 *                      bufferPtr, and the size of the buffer.
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpDirList (ttUserFtpHandle ftpUserHandle,
                  char TM_FAR *   pathNamePtr,
                  int             directoryFlag,
                  ttFtpCBFuncPtr  ftpUserCBFuncPtr )
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;
    tt8Bit          command;
    tt8Bit          dirCommand;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;

/* Parameter check:  callback function must be valid (not NULL) and
 * the directory type should either be short or long.
 */
    if ((ftpUserCBFuncPtr == (ttFtpCBFuncPtr) 0) ||
        ((directoryFlag != TM_DIR_SHORT) && (directoryFlag != TM_DIR_LONG)) )
    {
        result = TM_EINVAL;
    }
    else
    {

/* Check validity of command (must be connected, logged in) */
        result = tfFtpCheckSession (ftpSessionPtr,
                                    TM_FTP_F_CTRL_CONN | TM_FTP_F_LOGGED_IN);
        if (result == TM_ENOERROR)
        {

#ifdef TM_USE_FTP_SSL
            if ( (ftpSessionPtr->ftpSslDataConnReq == TM_8BIT_YES)
              && (ftpSessionPtr->ftpSslEnabledDataConn == TM_8BIT_NO) )
            {
/*
 * The server requires that a security data exchange be completed
 * first. The user must secure the data conn before calling this function.
 */
                result = TM_FTP_BADCMDSEQ;
            }
            else
#endif /* TM_USE_FTP_SSL */
            {
                ftpSessionPtr->ftpPathNamePtr   = pathNamePtr;
                ftpSessionPtr->ftpUserCBFuncPtr = ftpUserCBFuncPtr;

/*
 * command = PORT or PASV/EPSV; next command is NLST for short listings, LIST
 * for long  listing.
 */
                if (ftpSessionPtr->ftpFlags & TM_FTP_F_DATA_PASV )
                {
/* Passive mode: 1st command = PASV or EPSV */
#ifdef TM_USE_IPV6
                    if (ftpSessionPtr->ftpClientCtrlAddress.ss_family == AF_INET6)
                    {
                        command = TM_CMD_EPSV;
                    }
                    else
#endif /* TM_USE_IPV6 */
                    {
/* IPv4 */
                        command = TM_CMD_PASV;
                    }
                }
                else
                {
/* Active mode: 1st command = PORT */
                    command = TM_CMD_PORT;
                }
                if (directoryFlag == TM_DIR_SHORT)
                {
/* Short listing: 2nd command NLST */
                    dirCommand = TM_CMD_NLST;
                }
                else
                {
/* long listing: 2nd command LIST */
                    dirCommand = TM_CMD_LIST;
                }
/*
 * command = PORT or PASV/EPSV; next command is NLST for short listings, LIST
 * for long listing.
 */
                tfFtpSetCommandCode (ftpSessionPtr, command, dirCommand);
/* Kick the state machine */
                ftpSessionPtr->ftpCmdRequest = command; /* PORT or PASV/EPSV */
                ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
                result = tfFtpMatrix (ftpSessionPtr);
            }
        }
    }
    return (result);
}


/*
 * tfFtpSite description:
 *   Command not currently supported.
 *
 *
 * FTP Command          SITE <SP> <string> <CRLF>
 * Command reply        200
 *                      202
 *                      500, 501, 530
 *
 * SITE command
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  sitePtr             Pointer to a string containing the site command
 *                      argument.
 * Returns:
 *  Value               Meaning
 *  TM_EOPNOTSUPP       Command not supported by the user
 */
int tfFtpSite (ttUserFtpHandle ftpUserHandle,
               char TM_FAR *   sitePtr)
{

    TM_UNREF_IN_ARG(ftpUserHandle);
    sitePtr       = sitePtr;

    return(TM_EOPNOTSUPP);

}


/*
 * tfFtpSyst description:
 *
 * FTP Command          SYST <CRLF> (System:)
 * Command reply        215
 *                      500, 501, 502, 421
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpSyst (ttUserFtpHandle ftpUserHandle,
               char TM_FAR *   bufferPtr,
               int             bufferSize )
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;

/* Check validity of command (must be connected, buffer must be valid) */
    result = tfFtpCheckBufferCommand (ftpSessionPtr, TM_CMD_SYST, (ttCharPtr)"",
                                      TM_FTP_F_CTRL_CONN, bufferPtr,
                                      bufferSize);
    if (result == TM_ENOERROR)
    {

        ftpSessionPtr->ftpBufferPtr   = bufferPtr;
        ftpSessionPtr->ftpBufferSize  = bufferSize;
        ftpSessionPtr->ftpBufferIndex = ( int )0;

/* command = SYST, next = IDLE */
        tfFtpSetCommandCode ( ftpSessionPtr, TM_CMD_SYST, TM_CMD_IDLE );

/* kick the state machine */
        ftpSessionPtr->ftpCmdRequest = TM_CMD_SYST;
        ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
        result = tfFtpMatrix (ftpSessionPtr);
    }

    return (result);
}

/*
 * tfFtpStat description:
 *   Command not currently supported.
 *
 * FTP Command          STAT [<SP> <pathname>] <CRLF>
 * Command reply        211, 212, 213
 *                      450
 *                      500, 501, 502, 421, 530
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Outstanding ABORT/QUIT/STAT command has not finished
 *                      if parameter is fileNamePtr is null
 *  TM_EACCES           Previous command has not finished if fileNamePtr is
 *                      non null
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *                      (non null fileNamePtr only)
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpStat (ttUserFtpHandle ftpUserHandle,
               char TM_FAR *   fileNamePtr,
               char TM_FAR *   bufferPtr,
               int             bufferSize)
{

    TM_UNREF_IN_ARG(ftpUserHandle);
    fileNamePtr   = fileNamePtr;
    bufferPtr     = bufferPtr;
    bufferSize    = bufferSize;

    return (TM_EOPNOTSUPP);

}


/*
 * tfFtpHelp description:
 *   Retrieves help from the remote FTP server.
 *
 * FTP Command          HELP [<SP> <string>] <CRLF>
 * Command reply        211, 214
 *                      500, 501, 502, 421
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  helpArgPtr          Pointer to a string containing item user want help
 *                      on. (Could be null)
 *  ftpUserCBFuncPtr    Pointer to a user function to be called for
 *                      each received line of help.
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpHelp (ttUserFtpHandle ftpUserHandle,
               char TM_FAR *   commandPtr,
               ttFtpCBFuncPtr  ftpUserCBFuncPtr )
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;

/* Parameter check: callback function must be valid */
    if (ftpUserCBFuncPtr == (ttFtpCBFuncPtr) 0)
    {
        result = TM_EINVAL;
    }
    else
    {

/* Check validity of command (must be connected) */
        result = tfFtpCheckSession (ftpSessionPtr, TM_FTP_F_CTRL_CONN);

        if (result == TM_ENOERROR)
        {

            ftpSessionPtr->ftpCommandPtr    = commandPtr;
            ftpSessionPtr->ftpUserCBFuncPtr = ftpUserCBFuncPtr;

/* command = HELP, next = IDLE */
            tfFtpSetCommandCode ( ftpSessionPtr, TM_CMD_HELP, TM_CMD_IDLE );

/* Kick the state machine */
            ftpSessionPtr->ftpCmdRequest = TM_CMD_HELP;
            ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
            result = tfFtpMatrix (ftpSessionPtr);
        }
    }

    return (result);
}


/*
 * tfFtpNoop description:
 *   No operation.
 *
 * FTP Command          NOOP <CRLF>
 * Command reply        200
 *                      500, 421
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpNoop (ttUserFtpHandle ftpUserHandle)
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;

/* Check validity of command (must be connected) */
    result = tfFtpCheckSession(ftpSessionPtr, TM_FTP_F_CTRL_CONN);

    if (result == TM_ENOERROR)
    {

/* command = NOOP, next = IDLE */
        tfFtpSetCommandCode ( ftpSessionPtr, TM_CMD_NOOP, TM_CMD_IDLE );


/* kick the state machine */
        ftpSessionPtr->ftpCmdRequest = TM_CMD_NOOP;
        ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
        result = tfFtpMatrix (ftpSessionPtr);
    }

    return (result);
}


/*
 * tfFtpExecute description:
 *   Used in non-blocking mode to execute FTP commands.  Should be run
 *   periodically in the main loop of the application program.
 *
 * FTP Command          none
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ESHUTDOWN        Socket close
 *  TM_ETIMEDOUT        Connection timed out
 *  TM_ENOERROR (0)     No error (Success.)
 */
int tfFtpUserExecute (ttUserFtpHandle ftpUserHandle)
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;

/* Parameter check: FTP handle should be valid */
    if (ftpUserHandle != (ttUserFtpHandle) 0)
    {

/* If called in blocking mode, return EACCES. */
        if (ftpSessionPtr->ftpBlockingState == TM_BLOCKING_OFF)
        {
            result = tfFtpMatrix (ftpSessionPtr);
        }
        else
        {
            result = TM_EACCES;
        }
    }
    else
    {
        result = TM_EINVAL;
    }

    return (result);
}


/*
 * tfFtpCmdFuncNone description:
 *   Function does nothing, spaceholder function for state machine.
 *
 * FTP Command          none
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_ENOERROR (0)     No error. (Success.)
 */
int tfFtpGetReplyText(ttUserFtpHandle ftpUserHandle,
                      char TM_FAR *   replyStrPtr,
                      int             replyStrLen)
{
    int             result;
    unsigned int    replyLength;
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;

    result = 0;
    if (replyStrPtr != (char TM_FAR *) 0)
    {

        replyLength = (unsigned int)
            tm_strlen((char TM_FAR *) ftpSessionPtr->ftpReplyLine);
        (void)tm_strncpy(replyStrPtr,
                         (char TM_FAR *) ftpSessionPtr->ftpReplyLine,
                         replyStrLen);

        if (replyLength > (unsigned)replyStrLen)
        {
            result = replyStrLen;
        }
        else
        {
            result = (int)replyLength;
        }

    }

    return (result);
}


/*
 * tfFtpCmdFuncNone description:
 *   Function does nothing, spaceholder function for state machine.
 *
 * FTP Command          none
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncNone (ttFtpSessionPtr ftpSessionPtr)
{
    TM_UNREF_IN_ARG(ftpSessionPtr);

    return (TM_ENOERROR);
}


/*
 * tfFtpCmdFuncUser description:
 *
 * FTP Command          USER <SP> <username> <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncUser (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr cmdNamePtr;
    ttCharPtr setBufPtr;
    int       result;
    unsigned  int tempUInt;

/* Prepare command string */
    setBufPtr = (ttCharPtr) &ftpSessionPtr->ftpCmdSendBuf[0];

    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString(setBufPtr, cmdNamePtr);

    tempUInt = tm_strlen(ftpSessionPtr->ftpUserNamePtr);
    setBufPtr = tfFtpSetString (setBufPtr, " ");
    if (tempUInt != 0)
    {
        setBufPtr = tfFtpSetString (setBufPtr, ftpSessionPtr->ftpUserNamePtr);
    }

    setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

/* Send command string to server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}


/*
 * tfFtpCmdFuncPass description:
 *
 * FTP Command          PASS <SP> <password> <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncPass (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr cmdNamePtr;
    ttCharPtr setBufPtr;
    int       result;
    unsigned int tempUInt;

/* Prepare command string */
    setBufPtr = (ttCharPtr)&ftpSessionPtr->ftpCmdSendBuf[0];

    cmdNamePtr =
        (ttCharPtr) &tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0];
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);

    setBufPtr = tfFtpSetString (setBufPtr, " ");

    tempUInt = tm_strlen(ftpSessionPtr->ftpUserPasswordNamePtr);
    if (tempUInt != 0)
    {
        setBufPtr = tfFtpSetString (setBufPtr,
                                    ftpSessionPtr->ftpUserPasswordNamePtr);
    }

    setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

/* Send command string to server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}


/*
 * tfFtpCmdFuncAcct description:
 *
 * FTP Command          ACCT <SP> <account-information> <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncAcct (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr cmdNamePtr;
    ttCharPtr setBufPtr;
    int       result;
    unsigned int tempUInt;

/* Prepare command string */
    setBufPtr = (ttCharPtr)&ftpSessionPtr->ftpCmdSendBuf[0];

    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);

    tempUInt = tm_strlen(ftpSessionPtr->ftpUserAccountNamePtr);
    if (tempUInt != 0)
    {
        setBufPtr = tfFtpSetString (setBufPtr, " ");
        setBufPtr = tfFtpSetString (setBufPtr,
                                    ftpSessionPtr->ftpUserAccountNamePtr);
    }

    setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}


/*
 * tfFtpCmdFuncCwd description:
 *
 * FTP Command          CWD  <SP> <pathname> <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncCwd (ttFtpSessionPtr  ftpSessionPtr)
{
    ttCharPtr cmdNamePtr;
    ttCharPtr setBufPtr;
    int       result;

/* Preparse command string */
    setBufPtr = (ttCharPtr)&ftpSessionPtr->ftpCmdSendBuf[0];

    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);
    setBufPtr = tfFtpSetString (setBufPtr, " " );
    setBufPtr = tfFtpSetString (setBufPtr, ftpSessionPtr->ftpPathNamePtr);
    setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}


/*
 * tfFtpCmdFuncCdup description:
 *
 * FTP Command          CDUP <CRLF> (Change to parent directory:)
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncCdup (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr cmdNamePtr;
    ttCharPtr setBufPtr;
    int       result;

/* Prepare command string */
    setBufPtr = (ttCharPtr) &ftpSessionPtr->ftpCmdSendBuf[0];

    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);
    setBufPtr = tfFtpSetString (setBufPtr, "\r\n" );

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}

/*
 * tfFtpCmdFuncRein description:
 *
 * FTP Command          REIN <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncRein (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr cmdNamePtr;
    ttCharPtr setBufPtr;
    int       result;

/* Prepare command string */
    setBufPtr = (ttCharPtr)&ftpSessionPtr->ftpCmdSendBuf[0];

    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);
    setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}


/*
 * tfFtpCmdFuncQuit description:
 *
 * FTP Command          QUIT <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncQuit (ttFtpSessionPtr ftpSessionPtr)
{

    ttCharPtr cmdNamePtr;
    ttCharPtr setBufPtr;
    int       result;

/* Prepare command string */
    setBufPtr = (ttCharPtr)&ftpSessionPtr->ftpCmdSendBuf[0];

    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpInterCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);
    setBufPtr = tfFtpSetString ( setBufPtr, "\r\n" );

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}


/*
 * tfFtpCmdFuncPort description:
 *
 * FTP Command          PORT <SP> <host-port> <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncPort (ttFtpSessionPtr ftpSessionPtr)
{
#ifdef TM_USE_IPV6
    ttFtpFuncPtr       cmdFuncPtr;
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
    ttCharPtr          setBufPtr;
    ttCharPtr          cmdNamePtr;
    ttIpPort           nowPortNo;
    ttSockAddrPtrUnion clientAddr;
#endif /* TM_USE_IPV4 */
    int                result;
    int                sockAddrSize;

    sockAddrSize = sizeof(ftpSessionPtr->ftpClientDataAddress);
    result = TM_ENOERROR;

#ifdef TM_USE_IPV6
/* If this is an IPv6 connection, we need to send the EPRT (Extended PoRT)
   command, rather than the PORT command. */
    if (ftpSessionPtr->ftpClientCtrlAddress.ss_family == AF_INET6)
    {
        ftpSessionPtr->ftpCommandCode = TM_CMD_EPRT;

/* Call command handler for TM_CMD_EPRT */
        cmdFuncPtr = tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdFuncPtr;
        result = (*(cmdFuncPtr))(ftpSessionPtr);

    }
    else
#endif /* TM_USE_IPV6 */
    {

#ifdef TM_USE_IPV4
        setBufPtr = (ttCharPtr) &ftpSessionPtr->ftpCmdSendBuf[0];
        nowPortNo = ftpSessionPtr->ftpNowPortNo;

        if (ftpSessionPtr->ftpNextCommandCode != TM_CMD_IDLE)
        {
            if( ftpSessionPtr->ftp2ndSessionPtr == (ttFtpSessionPtr)0 )
            {
/*
 * Prepare for an incoming data connection (socket, bind, etc)
 * Bind to the port chosen by socket() or as specified by the user via
 * the tfFtpPort API
 */
                ftpSessionPtr->ftpClientDataAddress.ss_port = htons(nowPortNo);
                result = tfFtpDataConnRequest (ftpSessionPtr);
/*
 * Reset ftpNowPortNo to 0 so we will use the port obtained from socket() next
 * time unless the user calls tfFtpPort again to set the port number
 */
                ftpSessionPtr->ftpNowPortNo = 0;
            }

/* Send PORT command to server */
            if (result == TM_ENOERROR)
            {
/* Retrieve the socket that we bound to for the PORT message */
                clientAddr.sockNgPtr = &ftpSessionPtr->ftpClientDataAddress;
                (void)getsockname(
                            ftpSessionPtr->ftpSocketDesc[TM_FTP_LISTEN_SOCKET],
                            clientAddr.sockPtr,
                            &sockAddrSize);

                cmdNamePtr = (ttCharPtr)
                   &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
                setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);
                setBufPtr = tfFtpSetString (setBufPtr, " " );
                setBufPtr = tf4FtpSetAddress ( setBufPtr, clientAddr.sockInPtr );
                setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

                result = tfFtpCommandSend (ftpSessionPtr);
            }
        }
        else
        {
            ftpSessionPtr->ftpClientDataAddress.ss_port =
                htons (ftpSessionPtr->ftpPortParm);

/* Prepare command string */
            cmdNamePtr = (ttCharPtr)
                &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
            setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);
            setBufPtr = tfFtpSetString (setBufPtr, " " );
            clientAddr.sockNgPtr = &ftpSessionPtr->ftpClientDataAddress;
            setBufPtr = tf4FtpSetAddress ( setBufPtr, clientAddr.sockInPtr );
            setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

/* Send command string to FTP server */
            result = tfFtpCommandSend (ftpSessionPtr);
        }
#endif /* TM_USE_IPV4 */
    }

    return (result);
}


/*
 * tfFtpCmdFuncPasv description:
 *   Handles both PASV and EPSV
 * FTP Command          PASV <CRLF>
 *                      EPSV <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncPasv (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr setBufPtr;
    ttCharPtr cmdNamePtr;
    int       result;

    setBufPtr = (ttCharPtr) &ftpSessionPtr->ftpCmdSendBuf[0];

/* Prepare command string */
    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);
    setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);

    return (result);
}


/*
 * tfFtpCmdFuncType description:
 *
 * FTP Command          TYPE <SP> <type-code> <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncType (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr  setBufPtr;
    ttCharPtr  cmdNamePtr;
    int        result;
    char       buf[2];

/* A: ASCII, E: EBCDIC, I: IMAGE */
    static const char typeID[3] = {'A','E','I'};

/* Prepare command string */
    setBufPtr = (ttCharPtr)&ftpSessionPtr->ftpCmdSendBuf[0];

    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);

    setBufPtr = tfFtpSetString (setBufPtr, " ");

    buf[0] = typeID[ftpSessionPtr->ftpTransParm];
    buf[1] = 0x00;
    setBufPtr = tfFtpSetString (setBufPtr, &buf[0]);
    setBufPtr = tfFtpSetString (setBufPtr, "\r\n" );

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}


/*
 * tfFtpCmdFuncStru description:
 *
 * FTP Command          STRU <SP> <structure-code> <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncStru (ttFtpSessionPtr ftpSessionPtr )
{
    ttCharPtr  setBufPtr;
    ttCharPtr  cmdNamePtr;
    int        result;
    char       buf[2];
    static const char struID[3] = {'F','R','P'};  /* F:File, R:Record, P:Page */

/* Prepare command string */
    setBufPtr = (ttCharPtr)&ftpSessionPtr->ftpCmdSendBuf[0];

    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);

    setBufPtr = tfFtpSetString (setBufPtr, " ");

    buf[0] = struID[ftpSessionPtr->ftpTransParm];
    buf[1] = 0x00;
    setBufPtr = tfFtpSetString (setBufPtr, &buf[0]);
    setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}


/*
 * tfFtpCmdFuncMode description:
 *
 * FTP Command          MODE <SP> <mode-code> <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncMode (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr  setBufPtr;
    ttCharPtr  cmdNamePtr;
    int        result;
    char       buf[2];
/* S: Stream, B: Block, C: Compressed */
    static const char modeID[3] = {'S','B','C'};

/* Prepare command string */
    setBufPtr = (ttCharPtr)&ftpSessionPtr->ftpCmdSendBuf[0];

    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);
    setBufPtr = tfFtpSetString (setBufPtr, " ");

    buf[0] = modeID[ftpSessionPtr->ftpTransParm];
    buf[1] = 0x00;
    setBufPtr = tfFtpSetString (setBufPtr, &buf[0]);
    setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}


/*
 * tfFtpCmdFuncRetr description:
 *
 * FTP Command          RETR <SP> <pathname> <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncRetr (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr setBufPtr;
    ttCharPtr cmdNamePtr;
    int       result;

/* Prepare command string */
    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = (ttCharPtr) &ftpSessionPtr->ftpCmdSendBuf[0];

    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);

    setBufPtr = tfFtpSetString (setBufPtr, " " );
    setBufPtr = tfFtpSetString (setBufPtr, ftpSessionPtr->ftpRemoteFileNamePtr);
    setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}


/*
 * tfFtpCmdFuncStor description:
 *
 * FTP Command          STOR <SP> <pathname> <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncStor (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr cmdNamePtr;
    ttCharPtr setBufPtr;
    int       result;

/* Prepare command string */
    setBufPtr = (ttCharPtr)&ftpSessionPtr->ftpCmdSendBuf[0];

    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);

    setBufPtr = tfFtpSetString (setBufPtr, " " );
    setBufPtr = tfFtpSetString (setBufPtr, ftpSessionPtr->ftpRemoteFileNamePtr);
    setBufPtr = tfFtpSetString (setBufPtr, "\r\n" );

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}


/*
 * tfFtpCmdFuncAppe description:
 *
 * FTP Command          APPE <SP> <pathname> <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncAppe (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr cmdNamePtr;
    ttCharPtr setBufPtr;
    int       result;

/* Prepare command string */
    setBufPtr = (ttCharPtr)&ftpSessionPtr->ftpCmdSendBuf[0];

    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);

    setBufPtr = tfFtpSetString (setBufPtr, " " );
    setBufPtr = tfFtpSetString (setBufPtr, ftpSessionPtr->ftpRemoteFileNamePtr);
    setBufPtr = tfFtpSetString (setBufPtr, "\r\n" );

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}


/*
 * tfFtpCmdFuncRnfr description:
 *
 * FTP Command          RNFR <SP> <pathname> <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/ Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncRnfr (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr setBufPtr;
    ttCharPtr cmdNamePtr;
    int       result;

/* Prepare command string */
    setBufPtr = (ttCharPtr)&ftpSessionPtr->ftpCmdSendBuf[0];

    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);

    setBufPtr = tfFtpSetString (setBufPtr, " ");
    setBufPtr = tfFtpSetString (setBufPtr, ftpSessionPtr->ftpFromNamePtr);
    setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}


/*
 * tfFtpCmdFuncRnto description:
 *
 * FTP Command          RNTO <SP> <pathname> <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/ Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncRnto (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr setBufPtr;
    ttCharPtr cmdNamePtr;
    int       result;

/* Prepare command string */
    setBufPtr = (ttCharPtr) &ftpSessionPtr->ftpCmdSendBuf[0];

    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);

    setBufPtr = tfFtpSetString (setBufPtr, " ");
    setBufPtr = tfFtpSetString (setBufPtr, ftpSessionPtr->ftpToNamePtr);
    setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}


/*
 * tfFtpCmdFuncAbor description:
 *
 * FTP Command          ABOR <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/ Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncAbor (ttFtpSessionPtr ftpSessionPtr)
{

    ttCharPtr  setBufPtr;
    ttCharPtr  cmdNamePtr;
    int        result;

/* Prepare command string */
    setBufPtr = (ttCharPtr)&ftpSessionPtr->ftpCmdSendBuf[0];

    if (ftpSessionPtr->ftpMtrxPhase != TM_FTPP_INITIAL)
    {
            (void)tfFtpCtrlCharSend(ftpSessionPtr,
                                    (ttCharPtr) &tlInterData[0], 3, MSG_OOB);
            (void)tfFtpCtrlCharSend(ftpSessionPtr,
                                    (ttCharPtr) &tlInterData[3], 1,
                                    MSG_DONTWAIT);
    }

    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpInterCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);
    setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}


/*
 * tfFtpCmdFuncDele description:
 *
 * FTP Command          DELE <SP> <pathname> <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncDele (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr setBufPtr;
    ttCharPtr cmdNamePtr;
    int       result;

/* Prepare command string */
    setBufPtr = (ttCharPtr)&ftpSessionPtr->ftpCmdSendBuf[0];

    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);

    setBufPtr = tfFtpSetString (setBufPtr, " ");
    setBufPtr = tfFtpSetString (setBufPtr, ftpSessionPtr->ftpPathNamePtr);
    setBufPtr = tfFtpSetString (setBufPtr, "\r\n" );

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}

/*
 * tfFtpCmdFuncRmd description:
 *
 * FTP Command          RMD <SP> <pathname> <CRLF> (Remove Directory:)
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncRmd (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr setBufPtr;
    ttCharPtr cmdNamePtr;
    int       result;

/* Prepare command string */
    setBufPtr = (ttCharPtr)&ftpSessionPtr->ftpCmdSendBuf[0];

    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);

    setBufPtr = tfFtpSetString (setBufPtr, " ");
    setBufPtr = tfFtpSetString (setBufPtr, ftpSessionPtr->ftpPathNamePtr);
    setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}


/*
 * tfFtpCmdFuncMkd description:
 *
 * FTP Command          MKD  <SP> <pathname> <CRLF> (Make Directory:)
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncMkd (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr setBufPtr;
    ttCharPtr cmdNamePtr;
    int       result;

/* Prepare command string */
    setBufPtr = (ttCharPtr)&ftpSessionPtr->ftpCmdSendBuf[0];

    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);

    setBufPtr = tfFtpSetString (setBufPtr, " ");
    setBufPtr = tfFtpSetString (setBufPtr, ftpSessionPtr->ftpPathNamePtr);
    setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}


/*
 * tfFtpCmdFuncPwd description:
 *
 * FTP Command          PWD <CRLF> (Print Directory:)
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncPwd (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr setBufPtr;
    ttCharPtr cmdNamePtr;
    int       result;

/* Prepare command string */
    setBufPtr = (ttCharPtr)&ftpSessionPtr->ftpCmdSendBuf[0];

    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);
    setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}


/*
 * tfFtpCmdFuncList description:
 *
 * FTP Command          LIST [<SP> <pathname>] <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncList (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr cmdNamePtr;
    ttCharPtr setBufPtr;
    int       result;
    unsigned int tempUInt;

/* Prepare command string */

    setBufPtr = (ttCharPtr) &ftpSessionPtr->ftpCmdSendBuf[0];
    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);

    tempUInt = tm_strlen(ftpSessionPtr->ftpPathNamePtr);
    if (tempUInt != 0)
    {
        setBufPtr = tfFtpSetString (setBufPtr, " ");
        setBufPtr = tfFtpSetString (setBufPtr, ftpSessionPtr->ftpPathNamePtr);
    }

    setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}


/*
 * tfFtpCmdFuncNlst description:
 *
 * FTP Command          NLST [<SP> <pathname>] <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncNlst (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr cmdNamePtr;
    ttCharPtr setBufPtr;
    int       result;
    unsigned int tempUInt;

/* Prepare command string */
    setBufPtr = (ttCharPtr) &ftpSessionPtr->ftpCmdSendBuf[0];

    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);

    tempUInt = tm_strlen(ftpSessionPtr->ftpPathNamePtr);
    if (tempUInt != 0)
    {
        setBufPtr = tfFtpSetString (setBufPtr, " ");
        setBufPtr = tfFtpSetString (setBufPtr, ftpSessionPtr->ftpPathNamePtr);
    }

    setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}


/*
 * tfFtpCmdFuncSyst description:
 *
 * FTP Command          SYST <CRLF> (System:)
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncSyst (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr setBufPtr;
    ttCharPtr cmdFuncPtr;
    int       result;

/* Prepare command string */
    setBufPtr = (ttCharPtr) &ftpSessionPtr->ftpCmdSendBuf[0];

    cmdFuncPtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdFuncPtr);
    setBufPtr = tfFtpSetString ( setBufPtr, "\r\n" );

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}

/*
 * tfFtpCmdFuncHelp description:
 *
 * FTP Command          HELP [<SP> <string>] <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncHelp (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr setBufPtr;
    ttCharPtr cmdNamePtr;
    int       result;
    unsigned int tempUInt;

/* Prepare command string */
    setBufPtr = (ttCharPtr) &ftpSessionPtr->ftpCmdSendBuf[0];

    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);

    tempUInt = tm_strlen(ftpSessionPtr->ftpCommandPtr);
    if (tempUInt != 0)
    {
        setBufPtr = tfFtpSetString (setBufPtr, " ");
        setBufPtr = tfFtpSetString (setBufPtr, ftpSessionPtr->ftpCommandPtr);
    }

    setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}


/*
 * tfFtpCmdFuncNoop description:
 *
 * FTP Command          NOOP <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncNoop (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr cmdNamePtr;
    ttCharPtr setBufPtr;
    int       result;

/* Prepare command string */
    setBufPtr = (ttCharPtr) &ftpSessionPtr->ftpCmdSendBuf[0];

    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);
    setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

/* Send command string to FTP */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}

#ifdef TM_USE_FTP_SSL
/*
 * tfFtpCmdFuncAuth description:
 *
 * FTP Command          AUTH <mode> <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncAuth (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr cmdNamePtr;
    ttCharPtr setBufPtr;
    int       result;

/* Preparse command string */
    setBufPtr = (ttCharPtr)&ftpSessionPtr->ftpCmdSendBuf[0];

    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);
    setBufPtr = tfFtpSetString (setBufPtr, " " );
/* According to the newest FTP over SSL draft draft-murray-auth-
 * ftp-ssl-15.txt, the parameter of AUTH command must be 'TLS'.
 * As the SSL/TLS protocols self-negotiate their levels there is no
 * need to distinguish SSL vs TLS in the application layer.
 * However, some FTPSSL server still requires us to send 'AUTH SSL"
 */
#ifndef TM_FTPSSL_USE_AUTHSSL
    setBufPtr = tfFtpSetString (setBufPtr, "TLS");
#else /* TM_FTPSSL_USE_AUTHSSL */
    setBufPtr = tfFtpSetString (setBufPtr, "SSL");
#endif /* TM_FTPSSL_USE_AUTHSSL */
    setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}

/*
 * tfFtpCmdFuncProt description:
 *
 * FTP Command          PROT <SP> <mode> <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncProt (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr cmdNamePtr;
    ttCharPtr setBufPtr;
    int       result;

/* Prepare command string */
    setBufPtr = (ttCharPtr) &ftpSessionPtr->ftpCmdSendBuf[0];
    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);

    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);
    setBufPtr = tfFtpSetString(setBufPtr, " ");
    setBufPtr = tfFtpSetString(
                          setBufPtr,
                          (ttCharPtr)(&(ftpSessionPtr->ftpDataProtLevel)));
    setBufPtr = tfFtpSetString(setBufPtr, "\r\n");

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);

    return (result);
}

/*
 * tfFtpCmdFuncPbsz description:
 *
 * FTP Command          PBSZ <SP> <INT VAL> <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncPbsz (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr cmdNamePtr;
    ttCharPtr setBufPtr;
    int       result;

/* Prepare command string */
    setBufPtr = (ttCharPtr) &ftpSessionPtr->ftpCmdSendBuf[0];
    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);

    setBufPtr = tfFtpSetString(setBufPtr, cmdNamePtr);
    setBufPtr = tfFtpSetString(setBufPtr, " ");
    setBufPtr = tfFtpSetString(setBufPtr, "0");
    setBufPtr = tfFtpSetString(setBufPtr, "\r\n");

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);

    return (result);
}

/*
 * tfFtpCmdFuncCcc description:
 *
 * FTP Command          CCC <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncCcc (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr cmdNamePtr;
    ttCharPtr setBufPtr;
    int       result;

/* Preparse command string */
    setBufPtr = (ttCharPtr)&ftpSessionPtr->ftpCmdSendBuf[0];

    cmdNamePtr = (ttCharPtr)
        &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
    setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);
    setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

/* Send command string to FTP server */
    result = tfFtpCommandSend (ftpSessionPtr);
    return (result);
}
#endif /* TM_USE_FTP_SSL */

/*
 * tfFtpConnReply_1345 description:
 *   Called when a initial connection returns response code 1xx,3xx,4xx,5xx.
 *   This indicates that there was an error upon connection, so close the
 *   control connection and set the phase back to 0 (initial).
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpConnReply_1345 (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' '))
    {
        result = tfFtpGetReplyCode (ftpSessionPtr);
/* Close the control connection and reset the state machine */
        tfFtpCloseCtrlSocket(ftpSessionPtr);

        ftpSessionPtr->ftpCommandCode      = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode  = TM_CMD_IDLE;
        ftpSessionPtr->ftpInterCommandCode = TM_CMD_IDLE;

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpConnReply_2xx description:
 *   Connection was successful, so set the connected flag and move to phase 0.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpConnReply_2xx (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ' ) )
    {
/* Connection was successful, so indicate this in the flags and move to state
 * zero (initial).
 */
        ftpSessionPtr->ftpFlags |= TM_FTP_F_CTRL_CONN;

        ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

        result = tfFtpGetReplyCode (ftpSessionPtr);
    }
    else
    {
        tfFtpSetMatrixPhase ( ftpSessionPtr, TM_FTPP_REPLY_WAIT );
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpLoginReply_Error description:
 *   Error code returned from FTP server in response to our login request.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 */
static int tfFtpLoginReply_Error (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

/* An error occured, set the phase back to zero */
    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {
        ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

        result = TM_ENOTLOGIN;
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpLoginReply_Success description:
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 */
static int tfFtpLoginReply_Success (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {

/* Set flags to indicate that we're logged in */
        ftpSessionPtr->ftpFlags |= TM_FTP_F_LOGGED_IN;

        ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

/* Set phase back to initial */
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

/* Get and return reply code */
        result = tfFtpGetReplyCode (ftpSessionPtr);
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpUserReply_3xx description:
 *   USER command was successful, but requires authentication, so start the
 *   state machine over and issue the PASS command to the FTP server.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 */
static int tfFtpUserReply_3xx (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

        tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_PASS, TM_CMD_IDLE);

        ftpSessionPtr->ftpCmdRequest = TM_CMD_PASS;
        ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;

        result = TM_EWOULDBLOCK;
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpLoginReply_Failure description:
 *   Login attempt failed; return error and set phase back to initial.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 */
static int tfFtpLoginReply_Failure (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {
        ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

        result = TM_ENOTLOGIN;
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpPassReply_3xx description:
 *   The PASS command was successful but requires account info.  Start the
 *   state machine over and issue the ACCT command.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 */
static int tfFtpPassReply_3xx (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if ((ftpSessionPtr->ftpReplyLine[0] != ' ') &&
        (ftpSessionPtr->ftpReplyLine[3] == ' '))
    {

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

        tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_ACCT, TM_CMD_IDLE);

        ftpSessionPtr->ftpCmdRequest = TM_CMD_ACCT;
        ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;

        result = TM_EWOULDBLOCK;
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpReinReply_2xx description:
 *   REIN command successful, indicate in flags that we are no longer logged
 *   in.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpReinReply_2xx (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {

#ifdef TM_USE_FTP_SSL
/* Close SSL sessions (if any) on the control and data connections. */
        if (ftpSessionPtr->ftpSslSessionId >= 0)
        {
/* The SSL session is active. Remove it and reset all related flags. */

/* Disable SSL on the control connection. */
            tfSslConnectUserClose(
                ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET]);

            if (ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET]
               != TM_SOCKET_ERROR)
            {
/* The data connection is opened. Disable SSL on the data connection. */
                tfSslConnectUserClose(
                    ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET]);
            }

            ftpSessionPtr->ftpSslWaitCloseNotify   = TM_8BIT_YES;
            tm_16bit_clr_bit(ftpSessionPtr->ftpFlags, TM_FTP_F_LOGGED_IN);
            result = TM_EWOULDBLOCK;
        }
        else
#endif /* TM_USE_FTP_SSL */
        {
/* Indicate that we're not logged in any more */
            tm_16bit_clr_bit(ftpSessionPtr->ftpFlags, TM_FTP_F_LOGGED_IN);

            ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
            ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

            tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

            result = tfFtpGetReplyCode (ftpSessionPtr);
        }
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpQuitReply_1345 description:
 *   QUIT command failed, close control socket.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpQuitReply_1345 (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {
        result = tfFtpGetReplyCode (ftpSessionPtr);
        tfFtpCloseCtrlSocket(ftpSessionPtr);

        ftpSessionPtr->ftpCommandCode      = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode  = TM_CMD_IDLE;
        ftpSessionPtr->ftpInterCommandCode = TM_CMD_IDLE;

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpQuitReply_2xx description:
 *   QUIT command successful, close control socket.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpQuitReply_2xx (ttFtpSessionPtr ftpSessionPtr)
{
    int result;


    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {

        result = tfFtpGetReplyCode (ftpSessionPtr);

        tfFtpCloseCtrlSocket(ftpSessionPtr);

        ftpSessionPtr->ftpCommandCode      = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode  = TM_CMD_IDLE;
        ftpSessionPtr->ftpInterCommandCode = TM_CMD_IDLE;

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpPortReply_2xx description:
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpPortReply_2xx (ttFtpSessionPtr ftpSessionPtr)
{
    tt8Bit              nextCommandCode;
    int                 result;
    ttFtpSessionPtr     ftp2ndSessionPtr;

    if ((ftpSessionPtr->ftpReplyLine[0] != ' ') &&
        (ftpSessionPtr->ftpReplyLine[3] == ' '))
    {
        if (ftpSessionPtr->ftpNextCommandCode != TM_CMD_IDLE)
        {
/*
 * Port command completed successfully, move on to next command.
 * If in one FTP server model, move on to the next command only
 * within this session; if in two FTP server model, need to move
 * on to the next command in the both two sessions
 */
            if ( ftpSessionPtr->ftp2ndSessionPtr != (ttFtpSessionPtr)0 )
            {
/* Two-server model: move on to next FTP command on both sessions */
                nextCommandCode = ftpSessionPtr->ftpNextCommandCode;
                tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);
                tfFtpSetCommandCode (ftpSessionPtr, nextCommandCode,
                                                        TM_CMD_IDLE);
                ftpSessionPtr->ftpCmdRequest = nextCommandCode;
                ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;

                ftp2ndSessionPtr = ftpSessionPtr->ftp2ndSessionPtr;
                nextCommandCode = ftp2ndSessionPtr->ftpNextCommandCode;
                tfFtpSetMatrixPhase (ftp2ndSessionPtr, TM_FTPP_INITIAL);
                tfFtpSetCommandCode (ftp2ndSessionPtr, nextCommandCode,
                                                        TM_CMD_IDLE);
                ftp2ndSessionPtr->ftpCmdRequest = nextCommandCode;
                ftp2ndSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
            }
            else
            {
/* One-server model: move on to the next command in this session */
                nextCommandCode = ftpSessionPtr->ftpNextCommandCode;
                tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);
                tfFtpSetCommandCode (ftpSessionPtr, nextCommandCode,
                                                        TM_CMD_IDLE);
                ftpSessionPtr->ftpCmdRequest = nextCommandCode;
                ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
            }

            result = TM_EWOULDBLOCK;
        }
        else
        {
            ftpSessionPtr->ftpPortNo    = ftpSessionPtr->ftpPortParm;
            ftpSessionPtr->ftpNowPortNo = ftpSessionPtr->ftpPortNo;

            ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
            ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

            result = tfFtpGetReplyCode (ftpSessionPtr);
        }
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpPasvReply_2xx description:
 *  Handles both
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpPasvReply_2xx (ttFtpSessionPtr ftpSessionPtr)
{
    tt8Bit                  nextCommandCode;
    ttFtpSessionPtr         ftp2ndSessionPtr;
    int                     result;
    tt8Bit                  addrFamily;


    if (ftpSessionPtr->ftpCommandCode == TM_CMD_EPSV)
    {
        addrFamily = AF_INET6;
/* When using EPSV, the data address is the same as the client address. */
        tm_bcopy( &ftpSessionPtr->ftpClientCtrlAddress,
                  &ftpSessionPtr->ftpServerDataAddress,
                  sizeof(ftpSessionPtr->ftpServerDataAddress) );
    }
    else
    {
        addrFamily = AF_INET;
        ftpSessionPtr->ftpServerDataAddress.ss_len = sizeof(struct sockaddr_in);
    }

    ftpSessionPtr->ftpServerDataAddress.ss_family = addrFamily;

/*
 * Store listenning address/port of server A to ftpServerDataAddress
 * of this session for connect() use.
 */
    result = tfFtpGetAddress( (ttCharPtr)ftpSessionPtr->ftpReplyLine,
                              &ftpSessionPtr->ftpServerDataAddress);

    if ((ftpSessionPtr->ftpReplyLine[0] != ' ') &&
        (ftpSessionPtr->ftpReplyLine[3] == ' ') &&
        (result == TM_ENOERROR) )
    {
        if ( ftpSessionPtr->ftp2ndSessionPtr != (ttFtpSessionPtr)0 )
        {
/*
 * Passive two-server model: Store listenning address/port of server A to
 * the 2nd session for PORT command use.
 */
            ftp2ndSessionPtr = ftpSessionPtr->ftp2ndSessionPtr;
            ftp2ndSessionPtr->ftpClientDataAddress.ss_family =
                        ftpSessionPtr->ftpServerDataAddress.ss_family;
            ftp2ndSessionPtr->ftpClientDataAddress.ss_port =
                        ftpSessionPtr->ftpServerDataAddress.ss_port;
#ifdef TM_USE_IPV6
            if (addrFamily == AF_INET6)
            {
/* For IPv6, the address used to establish the data connection will be the same
   network address used for the control connection. */
                ftpSessionPtr->ftpServerDataAddress.addr.ipv6.sin6_len =
                    sizeof(struct sockaddr_in6);
                tm_6_ip_copy_structs(
                    ftpSessionPtr->ftpServerDataAddress.addr.ipv6.sin6_addr,
                    ftp2ndSessionPtr->ftpClientDataAddress.addr.ipv6.sin6_addr);
            }
            else
#endif /* TM_USE_IPV6 */
            {
#ifdef TM_USE_IPV4
/* AF_INET */
                ftpSessionPtr->ftpServerDataAddress.addr.ipv4.sin_len =
                    sizeof(struct sockaddr_in);
                ftpSessionPtr->ftpClientDataAddress.addr.ipv4.sin_len =
                    sizeof(struct sockaddr_in);
                ftp2ndSessionPtr->ftpClientDataAddress.addr.ipv4.sin_addr =
                        ftpSessionPtr->ftpServerDataAddress.addr.ipv4.sin_addr;
#endif /* TM_USE_IPV4 */
            }

        }
        else
        {
/* Passive one-server model: clear ftp2ndSessionPtr */
            ftp2ndSessionPtr = (ttFtpSessionPtr)0;
        }

        if (ftpSessionPtr->ftpNextCommandCode != TM_CMD_IDLE)
        {
            if ( ftp2ndSessionPtr != (ttFtpSessionPtr)0 )
            {
/* Passive two-server model */
/*
 * Set session A FSM phase to REPLY_WAIT, wait for session B executing
 * command PORT and XXXX ( ex. STOR/RETR )
 */
                nextCommandCode = ftpSessionPtr->ftpNextCommandCode;
                tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
                ftpSessionPtr->ftpCmdRequest = TM_CMD_IDLE;

/*
 * Set session B FSM phase to INITIAL, and execute command PORT and
 * XXXX( ex. STOR/RETR ) within session B
 */
                nextCommandCode = tm_get_pasv_cmd_code(nextCommandCode);
                tfFtpSetMatrixPhase (ftp2ndSessionPtr, TM_FTPP_INITIAL);
                tfFtpSetCommandCode (ftp2ndSessionPtr, TM_CMD_PORT,
                                                          nextCommandCode);

                ftp2ndSessionPtr->ftpCmdRequest = TM_CMD_PORT;
                ftp2ndSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;

                result = TM_EWOULDBLOCK;
            }
            else
            {
/* Passive one-server model: connect to the FTP server */
                result = tfFtpPasvModeDataConnect(ftpSessionPtr);

                if ( result == TM_ENOERROR )
                {
/* PASV command complete successfully, move on to the next command */
                    nextCommandCode = ftpSessionPtr->ftpNextCommandCode;
                    tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);
                    tfFtpSetCommandCode (ftpSessionPtr, nextCommandCode,
                                                            TM_CMD_IDLE);
                    ftpSessionPtr->ftpCmdRequest = nextCommandCode;
                    ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;

                    result = TM_EWOULDBLOCK;
                }
            }
        }
        else
        {
/* PASV command complete, no more command to execute for now */
            ftpSessionPtr->ftpPortNo    = ftpSessionPtr->ftpPortParm;
            ftpSessionPtr->ftpNowPortNo = ftpSessionPtr->ftpPortNo;

            ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
            ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

            result = tfFtpGetReplyCode (ftpSessionPtr);
        }
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpTypeReply_2xx description:
 *   TYPE command completed successfully.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpTypeReply_2xx (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ' ))
    {

/* Set type field in FTP session structure */
        ftpSessionPtr->ftpTransType = ftpSessionPtr->ftpTransParm;

        ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

        result = tfFtpGetReplyCode (ftpSessionPtr);
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpStruReply_2xx description:
 *   STRU command completed successfully.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpStruReply_2xx (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {

/* Set transport structure type in FTP session info */
        ftpSessionPtr->ftpTransStructure = ftpSessionPtr->ftpTransParm;

        ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

        result = tfFtpGetReplyCode (ftpSessionPtr);
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpModeReply_2xx description:
 *   MODE command completed successfully.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpModeReply_2xx (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {

/* Set mode in FTP session structure */
        ftpSessionPtr->ftpTransMode = ftpSessionPtr->ftpTransParm;

        ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

        result = tfFtpGetReplyCode (ftpSessionPtr);
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpRetrReply_1xx description:
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 */
static int tfFtpRetrReply_1xx (ttFtpSessionPtr ftpSessionPtr)
{
    int           result;

    result = tfFtpCommonDataReply ( ftpSessionPtr, TM_FS_WRITE);

    return (result);
}


/*
 * tfFtpStorReply_1xx description:
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 */
static int tfFtpStorReply_1xx (ttFtpSessionPtr ftpSessionPtr)
{
    int           result;

    result = tfFtpCommonDataReply ( ftpSessionPtr, TM_FS_READ);

    return (result);
}


/*
 * tfFtpAppeReply_1xx description:
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 */
static int tfFtpAppeReply_1xx (ttFtpSessionPtr ftpSessionPtr)
{
    int           result;

    result = tfFtpCommonDataReply ( ftpSessionPtr, TM_FS_READ);

    return (result);
}


/*
 * tfFtpRnfrReply_3xx description:
 *   RNFR command acceptable, but needs RNTO request to follow.  Set command
 *   to RNTO and kick the FSM.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 */
static int tfFtpRnfrReply_3xx (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);
        tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_RNTO, TM_CMD_IDLE);

        ftpSessionPtr->ftpCmdRequest = TM_CMD_RNTO;
        ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;

        result = TM_EWOULDBLOCK;
    }
    else
    {
        tfFtpSetMatrixPhase ( ftpSessionPtr, TM_FTPP_REPLY_WAIT );
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpAborReply_2xx description:
 *   ABOR command successful, so close data connection.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpAborReply_2xx (ttFtpSessionPtr ftpSessionPtr)
{
    int result;


    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {

/* Close data file, data and listening sockets if any */
        tfFtpCloseData(ftpSessionPtr);

        ftpSessionPtr->ftpCommandCode      = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode  = TM_CMD_IDLE;
        ftpSessionPtr->ftpInterCommandCode = TM_CMD_IDLE;

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

        result = tfFtpGetReplyCode (ftpSessionPtr);
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpXxxxReply_BufCopy description:
 *   Copy reply line to buffer passed in by user.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpXxxxReply_BufCopy (ttFtpSessionPtr ftpSessionPtr)
{
    int             result;
    unsigned int    copySize;
    unsigned int    bufferSize;
    unsigned int    bufferIndex;

    bufferSize  = (unsigned)ftpSessionPtr->ftpBufferSize;
    bufferIndex = (unsigned)ftpSessionPtr->ftpBufferIndex;

/*
 * Copy as much data as we can, either the length of the reply line or the
 * size of our buffer.
 */
    copySize = (unsigned int)
        (tm_strlen((ttCharPtr) &ftpSessionPtr->ftpReplyLine[0]) + 2);

    if (bufferSize > bufferIndex)
    {
        if ((bufferSize - bufferIndex) < copySize)
        {
            copySize = bufferSize - bufferIndex;
        }

        if (copySize > 0)
        {
            tm_bcopy(&ftpSessionPtr->ftpReplyLine[0],
                     &ftpSessionPtr->ftpBufferPtr[bufferIndex],
                     copySize);
        }

        ftpSessionPtr->ftpBufferIndex += (int)copySize;

        ftpSessionPtr->ftpBufferPtr[ftpSessionPtr->ftpBufferIndex - 2] = '\r';
        ftpSessionPtr->ftpBufferPtr[ftpSessionPtr->ftpBufferIndex - 1] = '\n';
    }

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {
        ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

        result = tfFtpGetReplyCode (ftpSessionPtr);
    }
    else
    {
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpListReply_1xx description:
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 */
static int tfFtpListReply_1xx (ttFtpSessionPtr ftpSessionPtr)
{
    int result;


    if ((ftpSessionPtr->ftpReplyLine[0] != ' ') &&
        (ftpSessionPtr->ftpReplyLine[3] == ' '))
    {

        ftpSessionPtr->ftpReplyCode = -1;

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_DATA_CONN);
        result = TM_EWOULDBLOCK;
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpNlstReply_1xx description:
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 */
static int tfFtpNlstReply_1xx (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if ((ftpSessionPtr->ftpReplyLine[0] != ' ') &&
        (ftpSessionPtr->ftpReplyLine[3] == ' '))
    {
        ftpSessionPtr->ftpReplyCode = -1;

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_DATA_CONN);
        result = TM_EWOULDBLOCK;
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}

static int tfFtpStouReply_1xx (ttFtpSessionPtr ftpSessionPtr)
{
    ftpSessionPtr=ftpSessionPtr;

    return (TM_EWOULDBLOCK);
}

/*
 * tfFtpHelpReply_2xx description:
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpHelpReply_2xx (ttFtpSessionPtr ftpSessionPtr)
{
    int             result;
    unsigned int    replySize;

    replySize = (unsigned int)
        tm_strlen((ttCharPtr) &ftpSessionPtr->ftpReplyLine[0]);

    ftpSessionPtr->ftpReplyLine[replySize]     = '\r';
    ftpSessionPtr->ftpReplyLine[replySize + 1] = '\n';
    ftpSessionPtr->ftpReplyLine[replySize + 2] = 0x00;

/* If the reply line looks good, return the entire reply (which includes the
 * help) to the user.
 */
    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {
/* Call the user's callback function */
        (void)(*(ftpSessionPtr->ftpUserCBFuncPtr))
            (ftpSessionPtr,
             (ttCharPtr) &ftpSessionPtr->ftpReplyLine[0],
             (int)tm_strlen((ttCharPtr) &ftpSessionPtr->ftpReplyLine[0]) );

        ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

        result = tfFtpGetReplyCode (ftpSessionPtr);
    }
    else
    {
/* Call the user's callback function */
        (void)(*(ftpSessionPtr->ftpUserCBFuncPtr))
            (ftpSessionPtr,
             (ttCharPtr)&ftpSessionPtr->ftpReplyLine[0],
             (int)tm_strlen ((ttCharPtr) &ftpSessionPtr->ftpReplyLine[0]) );

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}

#ifdef TM_USE_FTP_SSL
/*
 * tfFtpAuthReply_3xx description:
 *    Security mechanism accepted, but security data is required (334)
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpAuthReply_3xx (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {
        result = tfFtpGetReplyCode(ftpSessionPtr);

        if (result == TM_FTP_ADATREQ)
        {
/*
 * When the server requires security data, a 334 reply is sent to initiate
 * an ADAT message exchange.  This FTP client implementation does NOT support
 * ADAT, so close the control connection and return the error.
 */
            tfFtpCloseCtrlSocket(ftpSessionPtr);
            ftpSessionPtr->ftpCommandCode      = TM_CMD_IDLE;
            ftpSessionPtr->ftpNextCommandCode  = TM_CMD_IDLE;
            ftpSessionPtr->ftpInterCommandCode = TM_CMD_IDLE;
            tfFtpSetMatrixPhase(ftpSessionPtr, TM_FTPP_INITIAL);
        }
    }
    else
    {
        tfFtpSetMatrixPhase(ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}

/*
 * tfFtpAuthReply_4xx description: Resource not available (431)
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpAuthReply_4xx (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {
        ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);
        result = tfFtpGetReplyCode (ftpSessionPtr);
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}

/*
 * tfFtpAuthReply_5xx description:
 *    500: AUTH command not recognized
 *    502: SSL not implemented
 *    504: Given security mechanism unsupported
 *    534: Given security mechanism denied
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpAuthReply_5xx (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {
        ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);
        result = tfFtpGetReplyCode(ftpSessionPtr);

        if ( (result == TM_FTP_SYNTAXCMD) || (result == TM_FTP_NOCMD) )
        {
/* AUTH command not supported by the server */
            result = TM_ENOPROTOOPT;
        }
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}

/*
 * tfFtpAuthReply_2xx description:
 *    Security mechanism accepted and security data not required (234)
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpAuthReply_2xx (ttFtpSessionPtr ftpSessionPtr)
{
    int result;
    int sockDesc;

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {
/* Enable SSL on the control connection */
        ftpSessionPtr->ftpSslEnabledCtrlConn  = TM_8BIT_YES;
        ftpSessionPtr->ftpSslNegFlagsCtrlConn = TM_FTP_SSL_NOT_NEG;
        sockDesc = ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET];
        result = tfFtpStartSslNeg(ftpSessionPtr, sockDesc);

/* Verify if there is not another command to execute. */
        if (ftpSessionPtr->ftpNextCommandCode == TM_CMD_IDLE)
        {
/* Return to idle state. */
            ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
            ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;
            tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);
        }

        if (result == TM_ENOERROR)
        {
/* Block until SSL negotiation for the control connection completes. */
            ftpSessionPtr->ftpSslNegInProgress = TM_8BIT_YES;
            result = TM_EWOULDBLOCK;
        }
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}

/*
 * tfFtpCccReply_5xx description:
 *    533: Command not integrity protected
 *    534: Server not willing to turn off integrity requirement
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCccReply_5xx (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {
        ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

        result = tfFtpGetReplyCode(ftpSessionPtr);
        if ((result == TM_FTP_SYNTAXCMD) || (result == TM_FTP_NOCMD))
        {
/* CCC command not supported by the server */
            result = TM_EOPNOTSUPP;
        }
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}

/*
 * tfFtpCccReply_2xx description:
 *     Clear communication may be used on the control channel (200)
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCccReply_2xx (ttFtpSessionPtr ftpSessionPtr)
{
    int result;
    int sockDesc;
    tt8Bit nextCommandCode;

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {
/* Disable SSL on the control connection */
        sockDesc = ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET];
        tfSslConnectUserClose(sockDesc);
/* Update connection flags */
        ftpSessionPtr->ftpSslEnabledCtrlConn  = TM_8BIT_NO;
        ftpSessionPtr->ftpSslNegFlagsCtrlConn = TM_FTP_SSL_NOT_NEG;

        nextCommandCode = ftpSessionPtr->ftpNextCommandCode;

        if (nextCommandCode == TM_CMD_IDLE)
        {
/* Return to the user */
            ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
            ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

            tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);
            result = tfFtpGetReplyCode (ftpSessionPtr);
        }
        else
        {
/* Do the next command */
            tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);
            tfFtpSetCommandCode (ftpSessionPtr, nextCommandCode,
                                 TM_CMD_IDLE);
            ftpSessionPtr->ftpCmdRequest = nextCommandCode;
            ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;

            result = TM_EWOULDBLOCK;
        }
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}

/*
 * tfFtpProtReply_2xx description:
 *
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpProtReply_2xx(ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {
/* The server has accepted the specified protection level. */

        if (ftpSessionPtr->ftpDataProtLevel == 'P')
        {
/* Enable SSL on the data connection. */
            ftpSessionPtr->ftpSslEnabledDataConn  = TM_8BIT_YES;
        }
        else
        {
/* Disable SSL on the data connection. */
            ftpSessionPtr->ftpSslEnabledDataConn  = TM_8BIT_NO;
        }

        ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

        result = tfFtpGetReplyCode (ftpSessionPtr);
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}

/*
 * tfFtpProtReply_4xx description:
 *  431: The server is unable to accept the specified protection level.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpProtReply_4xx (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {
/* The server was unable to accept the specified protection level. */

        ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

        result = tfFtpGetReplyCode (ftpSessionPtr);
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}

/*
 * tfFtpProtReply_5xx description:
 *  503: The server has not completed a protection buffer size negotiation
 *       with the client.
 *  504: The server does not understand the specified protection level.
 *  534: The server is not willing to accept the specified protection level.
 *  536: The current security mechanism does not support the specified
 *       protection level.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpProtReply_5xx (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {
        ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

        result = tfFtpGetReplyCode (ftpSessionPtr);
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}

/*
 * tfFtpPbszReply_2xx description:
 *  200: The server has accepted the specified command and argument.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpPbszReply_2xx (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {
/*
 * The server has accepted the protection buffer size command. Set the
 * state machine to its initial state and next execute the PROT command.
 */
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

        tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_PROT, TM_CMD_IDLE);

        ftpSessionPtr->ftpCmdRequest = TM_CMD_PROT;
        ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;

        result = TM_EWOULDBLOCK;
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}

/*
 * tfFtpPbszReply_5xx description:
 *  501: The server cannot parse the specified argument.
 *  503: The server has not completed a security data exchange with
 *       the client.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpPbszReply_5xx (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {
        ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

        result = tfFtpGetReplyCode (ftpSessionPtr);
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}

#endif /* TM_USE_FTP_SSL */


/*
 * tfFtpXxxxReply_xxx description:
 *   Handler for a bad reply code received from FTP server.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpXxxxReply_xxx (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
    ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

/* Set FSM phase back to 0 (initial) */
    tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

/* Return reply code */
    result = tfFtpGetReplyCode (ftpSessionPtr);
    return (result);
}


/*
 * tfFtpXxxxReply_Error description:
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpXxxxReply_Error (ttFtpSessionPtr ftpSessionPtr)
{
    int result;


    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {
        ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

/* Set FSM phase back to 0 (initial) */
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

/* Return reply code */
        result = tfFtpGetReplyCode (ftpSessionPtr);
    }
    else
    {
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpXxxxReply_Success description:
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpXxxxReply_Success (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {
        ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

/* Set FSM phase back to 0 (initial) */
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

/* Return reply code */
        result = tfFtpGetReplyCode (ftpSessionPtr);
    }
    else
    {
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/* JNS/1-28-00 BUGFIX 473
 *
 * Listening socket was not being closed correctly when data connection
 * establishment failed (eg, when the server returned a 550 reply).
 *
 */
/*
 * tfFtpDataReply_Failure description:
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpDataReply_Failure (ttFtpSessionPtr ftpSessionPtr)
{
    int result;


    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {
/* Close data file, data and listening sockets if any */
        tfFtpCloseData(ftpSessionPtr);
        ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

/* Set FSM phase back to 0 (initial) */
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

/* Return reply code */
        result = tfFtpGetReplyCode (ftpSessionPtr);

#ifdef TM_USE_FTP_SSL
/*
 * If the server requires that the data channel be protected before
 * sending or receiving any data, it will reply with an invalid
 * protection level error code.
 */
        if (result == TM_FTP_INVALPROTLEVEL)
        {
/* The current protection level on the data channel is inappropriate.
 * The user must enable SSL/TLS on the data channel first. */
            ftpSessionPtr->ftpSslDataConnReq = TM_8BIT_YES;
        }
#endif /* TM_USE_FTP_SSL */

    }
    else
    {
        result = TM_EWOULDBLOCK;
    }

    return (result);
}

/*
 * tfFtpXxxxReply_Failure description:
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpXxxxReply_Failure (ttFtpSessionPtr ftpSessionPtr)
{
    int result;


    if ( (ftpSessionPtr->ftpReplyLine[0] != ' ') &&
         (ftpSessionPtr->ftpReplyLine[3] == ' ') )
    {
        ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
        ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;

/* Set FSM phase back to 0 (initial) */
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

/* Return reply code */
        result = tfFtpGetReplyCode (ftpSessionPtr);
    }
    else
    {
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpXxxxReply_Wait description:
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 */
static int tfFtpXxxxReply_Wait (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
    result = TM_EWOULDBLOCK;
    return (result);
}


/*
 * tfFtpMtrxCtrlConnect description:
 *   Attempt to establish a control connection with the remote FTP server.
 *
 */
static int tfFtpMtrxCtrlConnect (ttFtpSessionPtr ftpSessionPtr)
{
    int                result;
    int                socketDesc;
    int                addrLength;
    int                tempInt;
    ttSockAddrPtrUnion dataAddr,ctrlAddr;

    dataAddr.sockNgPtr = &(ftpSessionPtr->ftpClientDataAddress);
    ctrlAddr.sockNgPtr = &(ftpSessionPtr->ftpClientCtrlAddress);

    if ((ctrlAddr.sockNgPtr)->ss_port == 0)
    {
        (ctrlAddr.sockNgPtr)->ss_port = htons(TM_REMOTE_CTRL_PORT);
    }

/* Control address and address family already set in tfFtpConnect or
   tfNgFtpConnect */

/* Create socket for control connection */
    result = socket (ctrlAddr.sockNgPtr->ss_family, SOCK_STREAM, IP_PROTOTCP);

    if (result != TM_SOCKET_ERROR)
    {

/* Socket creation was successful, so get socket info and place in FTP session
 * structure, set the blocking mode, register callbacks and attempt to connect
 * to the FTP server's control port.
 */
        socketDesc = result;
        ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET] = result;

/* Change Time Wait to minimum value */
        tempInt = 0;
        (void)setsockopt( socketDesc,
                          IPPROTO_TCP,
                          TM_TCP_2MSLTIME,
                          (const char TM_FAR *)&tempInt,
                          sizeof(int) );

        addrLength = sizeof (ftpSessionPtr->ftpClientDataAddress);
        (void)getsockname(socketDesc, dataAddr.sockPtr, &addrLength);

/* Set appropriate blocking state for the control socket */
        if ( ftpSessionPtr->ftpBlockingState == TM_BLOCKING_OFF )
        {
            (void)tfBlockingState(socketDesc, TM_BLOCKING_OFF);
        }

        ftpSessionPtr->ftpCBFlags[TM_FTP_CTRL_SOCKET] = 0;

/* Register connect, receive, close and reset callbacks for control
 * connection.
 */
        (void)tfRegisterSocketCBParam(socketDesc,
                                      tfFtpSignal,
                                      ftpSessionPtr,
                                      TM_CB_CONNECT_COMPLT | TM_CB_RECV
                                      | TM_CB_REMOTE_CLOSE | TM_CB_RESET
                                      | TM_CB_SOCKET_ERROR
#ifdef TM_USE_FTP_SSL
                                      | TM_FTP_SSL_CB_F
#endif /* TM_USE_FTP_SSL */
                                      );

/* Establish control connection */
        result = connect (socketDesc,
                          ctrlAddr.sockPtr,
                          ctrlAddr.sockPtr->sa_len);

        if (result == TM_ENOERROR)
        {

/*
 * Connect was successful:  get connection info, move to next FSM phase and
 * return EWOULDBLOCK.
 */
            addrLength = dataAddr.sockNgPtr->ss_len;
            (void)getsockname(socketDesc, dataAddr.sockPtr, &addrLength);

            tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
            result = TM_EWOULDBLOCK;
        }
        else
        {
            result = tfGetSocketError (socketDesc);

            if ((result == TM_EINPROGRESS) || (result == TM_ENOERROR))
            {
/* Non-blocking mode, so connect is still in progress */
                tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_CONNECT_WAIT);
                result = TM_EWOULDBLOCK;
            }
            else
            {
/* An error occured during connect, so close the control socket, set phase back
 * to previous phase and return the socket error to the user.
 */
                tfFtpCloseCtrlSocket(ftpSessionPtr);
                tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);
            }
        }
    }
    else
    {
/* An error occured while trying to create the socket: set phase back to
 * previous phase and return the socket error to the user.
 */
        result = tfGetSocketError (result);
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);
    }

    return (result);
}


/*
 * tfFtpMtrxCloseCtrl description:
 *   Close FTP connection: close control socket and reset FSM.
 *
 */
static int tfFtpMtrxCloseCtrl (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    result = tfFtpMtrxCloseCtrlData(ftpSessionPtr);
    if (ftpSessionPtr->ftpMtrxEvent == TM_FTPE_CLOSE)
    {
        result = TM_ENOERROR;
    }
    return (result);
}


/*
 * tfFtpMtrxAlready description:
 *   Operation already in progress.
 *
 */
static int tfFtpMtrxAlready (ttFtpSessionPtr ftpSessionPtr)
{
    TM_UNREF_IN_ARG(ftpSessionPtr);

    return (TM_EALREADY);
}


/*
 * tfFtpMtrxGetCtrlInfo description:
 *   Called when the control connection is established; get socket info and
 *   move to next state.
 *
 */
static int tfFtpMtrxGetCtrlInfo (ttFtpSessionPtr ftpSessionPtr)
{
    int                 result;
    int                 socketDesc;
    int                 addrLength;
    ttSockAddrPtrUnion  sockAddr;

    socketDesc = ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET];
    sockAddr.sockNgPtr = &ftpSessionPtr->ftpClientDataAddress;

/* Get socket info and place it into the FTP session structure */
    addrLength = sockAddr.sockNgPtr->ss_len;
    (void)getsockname(socketDesc, sockAddr.sockPtr, &addrLength);

/* Move to FSM phase 2 */
    tfFtpSetMatrixPhase(ftpSessionPtr, TM_FTPP_REPLY_WAIT);

    result = TM_EWOULDBLOCK;
    return (result);
}


/*
 * tfFtpMtrxCtrlRecv description:
 *
 */
static int tfFtpMtrxCtrlRecv (ttFtpSessionPtr ftpSessionPtr)
{
    int result;
    int socketDesc;
    int recvSize;

/* Receive data on control socket into input buffer */
    socketDesc = ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET];

    (void)tfFtpSocketReceive(ftpSessionPtr, socketDesc,
                             (ttCharPtr) &ftpSessionPtr->ftpTelInputBuf[0],
                             TM_CMD_LINE_SIZE, &recvSize);

    ftpSessionPtr->ftpTelBufReadIndex = 0;
    ftpSessionPtr->ftpTelBufProcessIndex = 0;

    tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);
    result = TM_EWOULDBLOCK;

    return (result);
}


/*
 * tfFtpMtrxCtrlReplyRecv description:
 *   Called when reply has been received from FTP server.
 *
 */
static int tfFtpMtrxCtrlReplyRecv (ttFtpSessionPtr ftpSessionPtr)
{
    int result;
    int matrixNo;

    matrixNo = ftpSessionPtr->ftpMatrixNo;
    result = tfFtpCmdReplyCheck (ftpSessionPtr);

    if (ftpSessionPtr->ftpInterCommandCode != TM_CMD_IDLE)
    {
        if ((matrixNo <= TM_FTP_MATRIX1) &&
            (ftpSessionPtr->ftpMtrxPhase == TM_FTPP_INITIAL))
        {
            tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        }
        else
        {
            if ((matrixNo >= TM_FTP_MATRIX2) &&
                (ftpSessionPtr->ftpInterCommandCode == TM_CMD_ABOR))
            {
/* Close data file, data and listening sockets if any */
                tfFtpCloseData(ftpSessionPtr);
                result = TM_REPLY_BASE + TM_REPLY_ABOR;
            }
        }
    }

    return (result);
}




/*
 * tfFtpMtrxPermDenied description:
 *
 */
static int tfFtpMtrxPermDenied (ttFtpSessionPtr ftpSessionPtr)
{
    TM_UNREF_IN_ARG(ftpSessionPtr);

    return (TM_EACCES);
}


/*
 * tfFtpMtrxCommand description:
 *   Call command handler and set next phase.
 *
 */
static int tfFtpMtrxCommand (ttFtpSessionPtr ftpSessionPtr)
{
    ttFtpFuncPtr cmdFuncPtr;
    int          result;

/* Call command handler */
    cmdFuncPtr = tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdFuncPtr;
    result = (*(cmdFuncPtr))(ftpSessionPtr);

    if (result == TM_ENOERROR)
    {
        ftpSessionPtr->ftpReplyCode = -1;

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpMtrxInterCommand description:
 *   Executes intermediate command handler and moves to next state.
 *
 */
static int tfFtpMtrxInterCommand (ttFtpSessionPtr ftpSessionPtr)
{
    ttFtpFuncPtr cmdFuncPtr;
    int          result;

/* Call command handler for intermediate function */
    cmdFuncPtr =
        tlFtpCmdTable[ftpSessionPtr->ftpInterCommandCode].fctCmdFuncPtr;
    result = (*(cmdFuncPtr))(ftpSessionPtr);

    if (result == TM_ENOERROR)
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}




/*
 * tfFtpMtrxDataConnect description:
 *   In active mode, called for an incoming data connection(TM_CB_ACCEPT);
 *   In passive one server mode, called for completeing data connection(
 *         (TM_CB_CONNECT_COMPLT).
 *
 */
static int tfFtpMtrxDataConnect (ttFtpSessionPtr ftpSessionPtr)
{
    int                 result;
    int                 socketDesc;
    int                 addrLength;
    ttSockAddrPtrUnion  sockAddr;
/*
 * Two cases: the FTP session is in active mode(TM_CB_ACCEPT), the session is in
 * passive mode(TM_CB_CONNECT_COMPLT).
 */

    result = TM_EWOULDBLOCK;

    if(tm_16bit_bits_not_set(ftpSessionPtr->ftpFlags,TM_FTP_F_DATA_PASV))
    {
/* Active mode (TM_CB_ACCEPT) */
/* Accept an incoming data connection */
#ifdef TM_USE_FTP_SSL
/* Verify that a new connection has not already been accepted. */
        if(ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET] == TM_SOCKET_ERROR)
#endif /* TM_USE_FTP_SSL */
        {
/* Data connection has not been established. Accept the new connection. */
            socketDesc = ftpSessionPtr->ftpSocketDesc[TM_FTP_LISTEN_SOCKET];

            sockAddr.sockNgPtr = &ftpSessionPtr->ftpServerDataAddress;
            addrLength = ftpSessionPtr->ftpClientCtrlAddress.ss_len;

            result = accept (socketDesc, sockAddr.sockPtr, &addrLength );

            if (result >= 0)
            {
                socketDesc = result;
                ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET] = result;
#ifdef TM_USE_TCP_REXMIT_TEST
                tm_context(tvFtpSockDesc) = result;
#endif /* TM_USE_TCP_REXMIT_TEST */

                (void)tfFtpSetDataOptions(ftpSessionPtr,
                                          TM_CB_WRITE_READY  | TM_CB_RECV
                                        | TM_CB_REMOTE_CLOSE | TM_CB_RESET
                                        | TM_CB_SOCKET_ERROR
#ifdef TM_USE_FTP_SSL
                                        | TM_FTP_SSL_CB_F
#endif /* TM_USE_FTP_SSL */
                                         );
            }
            else
            {
/* An error occured when we tried to accept a connection: close the data
* socket and move back a state.
*/
                result = tfGetSocketError (socketDesc);
#ifdef TM_USE_FTP_SSL
                socketDesc = TM_SOCKET_ERROR;
#endif /* TM_USE_FTP_SSL */
                if (result != TM_EWOULDBLOCK)
                {

/* Close data file, data and listening sockets if any */
                    tfFtpCloseData(ftpSessionPtr);

                    tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
                    result = TM_EWOULDBLOCK;
                }
                else
                {
                    tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_DATA_CONN);
                    result = TM_EWOULDBLOCK;
                }
            }
        }
#ifdef TM_USE_FTP_SSL
        else
        {
/* Data connection has already been established. Use it. */
            socketDesc = ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET];
        }

        if ( (ftpSessionPtr->ftpSslEnabledDataConn)
          && (ftpSessionPtr->ftpSslNegFlagsDataConn == TM_FTP_SSL_NOT_NEG)
          && (ftpSessionPtr->ftpSslNegInProgress == TM_8BIT_NO)
          && (socketDesc != TM_SOCKET_ERROR) )
        {
/*
* SSL is enabled on the data connection but negotiations have not yet
* taken place. Start SSL negotiation.
*/
            result = tfFtpStartSslNeg(ftpSessionPtr, socketDesc);

            if (result == TM_ENOERROR)
            {
/* SSL negotiation completed successfully. Wait for reply. */
                ftpSessionPtr->ftpSslNegInProgress = TM_8BIT_YES;
                result = TM_EWOULDBLOCK;
            }
            else
            {
/*
 * SSL negotiation was not completed successfully. Close the data file if any,
 * data and listening sockets.
 */
                tfFtpCloseData(ftpSessionPtr);

                tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_DATA_CONN);
            }

        }
        else if(socketDesc != TM_SOCKET_ERROR)
/* SSL has been established or SSL is not enabled. */
#endif /* TM_USE_FTP_SSL */
        {

            if (ftpSessionPtr->ftpMatrixNo == TM_FTP_MATRIX4)
            {
                tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_DATA_SEND);
            }
            else
            {
                tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_DATA_RECV);
            }

#ifdef TM_USE_FTP_SSL
            if(ftpSessionPtr->ftpSslNegFlagsDataConn == TM_FTP_SSL_NEG_SUCCESS)
            {
/*
 * SSL was established successfully and the matrix phase changed to data
 * send/receive. Reset the SSL negotiation flags.
 */
                ftpSessionPtr->ftpSslNegFlagsDataConn = TM_FTP_SSL_NOT_NEG;
                ftpSessionPtr->ftpSslNegInProgress    = TM_8BIT_NO;
            }
#endif /* TM_USE_FTP_SSL */

            result = TM_EWOULDBLOCK;
        }
    }
    else
    {
/* Passive mode (TM_CB_CONNECT_COMPLT) */
        socketDesc = ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET];
        sockAddr.sockNgPtr = &ftpSessionPtr->ftpClientDataAddress;

/* Get socket info and place it into the FTP session structure */
        addrLength = sockAddr.sockNgPtr->ss_len;
        (void)getsockname(socketDesc, sockAddr.sockPtr, &addrLength);

#ifdef TM_USE_FTP_SSL
        if ( (ftpSessionPtr->ftpSslEnabledDataConn)
          && (ftpSessionPtr->ftpSslNegFlagsDataConn == TM_FTP_SSL_NOT_NEG)
          && (ftpSessionPtr->ftpSslNegInProgress == TM_8BIT_NO) )
        {
/*
 * SSL is enabled on the data connection but negotiations have not yet
 * taken place. Start SSL negotiation.
 */
            result = tfFtpStartSslNeg(ftpSessionPtr, socketDesc);

            if (result == TM_ENOERROR)
            {
/* SSL negotiation completed successfully. Wait for reply. */
                ftpSessionPtr->ftpSslNegInProgress = TM_8BIT_YES;
                result = TM_EWOULDBLOCK;
            }
            else
            {
/*
 * SSL negotiation was not completed successfully. Close the data file if any,
 * data and listening sockets.
 */
                tfFtpCloseData(ftpSessionPtr);

                tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_DATA_CONN);
            }
        }
        else
/* SSL has been established or SSL is not enabled. */
#endif /* TM_USE_FTP_SSL */
        {
            if (ftpSessionPtr->ftpMatrixNo == TM_FTP_MATRIX4)
            {
                tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_DATA_SEND);
            }
            else
            {
                tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_DATA_RECV);
            }

#ifdef TM_USE_FTP_SSL
            if(ftpSessionPtr->ftpSslNegFlagsDataConn == TM_FTP_SSL_NEG_SUCCESS)
            {
/*
 * SSL was established successfully and the matrix phase changed to data
 * send/receive. Reset the SSL negotiation flags.
 */
                ftpSessionPtr->ftpSslNegFlagsDataConn = TM_FTP_SSL_NOT_NEG;
                ftpSessionPtr->ftpSslNegInProgress    = TM_8BIT_NO;
            }
#endif /* TM_USE_FTP_SSL */

            result = TM_EWOULDBLOCK;
        }
    }

    return (result);
}


/*
 * tfFtpMtrxDataPreRecv description:
 *
 */
static int tfFtpMtrxDataPreRecv (ttFtpSessionPtr ftpSessionPtr)
{
    int result;
    int socketDesc;
    int recvSize;

    socketDesc = ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET];

    (void)tfFtpSocketReceive(ftpSessionPtr, socketDesc,
                             (ttCharPtr) &ftpSessionPtr->ftpDataRecvBuf[0],
                             TM_BUF_RECV_SIZE, &recvSize);

    ftpSessionPtr->ftpDataBufReadIndex    = 0;
    ftpSessionPtr->ftpDataBufProcessIndex = 0;

    tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
    result = TM_EWOULDBLOCK;

    return (result);
}


/*
 * tfFtpMtrxDirDataRecv description:
 *   Called when data is available on the data socket, for directory listings
 *
 */
static int tfFtpMtrxDirDataRecv (ttFtpSessionPtr ftpSessionPtr)
{
    tt8Bit    telChar;
    tt8BitPtr dataPtr;
    int       index;
    int       result;
    int       socketDesc;
    int       recvSize;
    int       unprocessedBytes;

    socketDesc = ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET];

/* Get data from socket */
    index = ftpSessionPtr->ftpDataBufReadIndex;
    dataPtr = &(ftpSessionPtr->ftpDataRecvBuf[index]);
    recvSize = 0; /* compiler warning */
    result = tfFtpSocketReceive (
        ftpSessionPtr, socketDesc, (ttCharPtr) dataPtr,
        TM_BUF_RECV_SIZE - index, &recvSize);

    if ((result == TM_ENOERROR) || (result == TM_EWOULDBLOCK))
    {
        ftpSessionPtr->ftpDataBufReadIndex += recvSize;

/*
 * Parse all unprocessed data in the receive buffer
 */
        while (ftpSessionPtr->ftpDataBufProcessIndex <
            ftpSessionPtr->ftpDataBufReadIndex)
        {
            telChar = ftpSessionPtr->ftpDataRecvBuf
                [ftpSessionPtr->ftpDataBufProcessIndex];

            ftpSessionPtr->ftpDataBufProcessIndex++;

            switch (ftpSessionPtr->ftpDataBufTelState)
            {
/* Previous character was a carriage return */
                case TM_TEL_CR_STATE:
                    ftpSessionPtr->ftpDataBufTelState = TM_TEL_DATA_STATE;

/*
 * CR-LF pair:  add CR-LF and end of string character to directory listing,
 * and set flags to indicate that the directory listing has been received.
 */
                    if (telChar == TM_NVT_LF)
                    {
                        ftpSessionPtr->ftpDirList
                            [ftpSessionPtr->ftpDirListIndex] = TM_NVT_CR;
                        ftpSessionPtr->ftpDirListIndex++;
                        ftpSessionPtr->ftpDirList
                            [ftpSessionPtr->ftpDirListIndex ] = TM_NVT_LF;
                        ftpSessionPtr->ftpDirListIndex++;

                        ftpSessionPtr->ftpDirList
                            [ftpSessionPtr->ftpDirListIndex] = '\0';
                        ftpSessionPtr->ftpDirListIndex++;

                        ftpSessionPtr->ftpFlags |=
                            (tt16Bit) TM_FTP_F_DIRLIST_RECVED;
                    }
                    else
                    {
/*
 * The current character is NOT an LF, so we didn't get a CR-LF pair and the
 * directory listing has not yet ended.  Add previous <CR> character to our
 * directory listing buffer. If the next character is a null, ignore it,
 * otherwise go backwards on our incoming buffer to process the current
 * character with the correct change of state in the next iteration of
 * the loop.
 */
                        ftpSessionPtr->ftpDirList
                            [ftpSessionPtr->ftpDirListIndex] = TM_NVT_CR;
                        ftpSessionPtr->ftpDirListIndex++;
                        if (telChar != '\0')
                        {
/*
 * <CR><not \0 && not LF> maps to <CR><not \0 && not LF>. Don't place the
 * current character in the directory listing buffer. Process the current
 * character in the next iteration of the loop.
 */
                            ftpSessionPtr->ftpDataBufProcessIndex--;
                        }
/* else <CR><\0> maps to <CR>, so ignore the <\0> */
                    }
                    break;

                case TM_TEL_DATA_STATE:
                    if (telChar == TM_NVT_CR)
                    {
/* CR: Don't place in directory listing, and wait for next char */
                        ftpSessionPtr->ftpDataBufTelState = TM_TEL_CR_STATE;
                    }
                    else
                    {
/* Normal data char: insert into directory listing */
                        ftpSessionPtr->ftpDirList
                            [ftpSessionPtr->ftpDirListIndex] = telChar;
                        ftpSessionPtr->ftpDirListIndex++;
                    }
                    break;
                default:
                    break;
            }


            if (tm_16bit_one_bit_set (ftpSessionPtr->ftpFlags,
                                      TM_FTP_F_DIRLIST_RECVED))
            {
/* Directory listing received successfully: fix buffers and return data */

                if (ftpSessionPtr->ftpDirListIndex == TM_CMD_LINE_SIZE)
                {
                    ftpSessionPtr->ftpDirList[TM_CMD_LINE_SIZE - 1] = '\0';
                }

/*
 * Move all unprocessed data to front of receive buffer (which removes all
 * processed data from this buffer).
 */
                unprocessedBytes = ftpSessionPtr->ftpDataBufReadIndex -
                    ftpSessionPtr->ftpDataBufProcessIndex;

                if (unprocessedBytes != 0)
                {
                    index = ftpSessionPtr->ftpDataBufProcessIndex;
                    dataPtr = &(ftpSessionPtr->ftpDataRecvBuf[index]);
                    tm_bcopy(dataPtr, &ftpSessionPtr->ftpDataRecvBuf[0],
                             unprocessedBytes );

                    ftpSessionPtr->ftpDataBufProcessIndex = 0;
                    ftpSessionPtr->ftpDataBufReadIndex    = unprocessedBytes;
                }
                else
                {
                    ftpSessionPtr->ftpDataBufProcessIndex = 0;
                    ftpSessionPtr->ftpDataBufReadIndex    = 0;
                }

/* Return the directory listing to the user via their callback function */
                (void)(*(ftpSessionPtr->ftpUserCBFuncPtr))(ftpSessionPtr,
                    (ttCharPtr) &ftpSessionPtr->ftpDirList[0],
                    ftpSessionPtr->ftpDirListIndex);

                ftpSessionPtr->ftpDirListIndex = 0;

                tm_16bit_clr_bit(ftpSessionPtr->ftpFlags, TM_FTP_F_DIRLIST_RECVED);
            }
        }

        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_DATA_RECV);
        result = TM_EWOULDBLOCK;
    }
    else
    {
/* Close data file, data and listening sockets if any */
        tfFtpCloseData(ftpSessionPtr);
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpMtrxCloseCtrlData description:
 *
 */
static int tfFtpMtrxCloseCtrlData (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

/*
 * tfFtpCloseCtrlSocket will close data file, data and listening sockets
 * if any
 */
    tfFtpCloseCtrlSocket(ftpSessionPtr);

    tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);
    switch (ftpSessionPtr->ftpMtrxEvent)
    {
        case TM_FTPE_CTRL_ERROR:
            result = TM_ESHUTDOWN;
            break;

        case TM_FTPE_TIMEOUT:
            result = TM_ETIMEDOUT;
            break;

        default:
            result = TM_EINVAL;
    }

    return (result);
}

/*
 * tfFtpMtrxCloseData description:
 *
 */
static int tfFtpMtrxCloseData (ttFtpSessionPtr ftpSessionPtr)
{

/* Close data file, data and listening sockets if any */
    tfFtpCloseData(ftpSessionPtr);

    tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);

    return (TM_EWOULDBLOCK);
}


/*
 * tfFtpMtrxDataRecv description:
 *
 */
static int tfFtpMtrxDataRecv (ttFtpSessionPtr ftpSessionPtr)
{
    int result;
    int recvSize;

    recvSize = 0;

/* Get data form data connection */
    result = tfFtpSocketReceive (ftpSessionPtr,
                                 ftpSessionPtr->
                                    ftpSocketDesc[TM_FTP_DATA_SOCKET],
                                 (ttCharPtr)&ftpSessionPtr->ftpDataRecvBuf[0],
                                 TM_BUF_RECV_SIZE, &recvSize);

    if (result == TM_ENOERROR)
    {
/* Data received OK, write to file */
        result = tfFSWriteFile (ftpSessionPtr->ftpUserDataPtr,
                                ftpSessionPtr->ftpFilePointer,
                                (ttCharPtr)&ftpSessionPtr->ftpDataRecvBuf[0],
                                recvSize);
        if (result != TM_SOCKET_ERROR)
        {
/* We wrote the data to the file system successfully */
            tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_DATA_RECV);
        }
    }
    else
    {
/* Error occured on recv call */
        if (result == TM_EWOULDBLOCK)
        {
/*
 * There was no data to recv on the socket. Set up the state to recv more
 * data later.
 */
            result = TM_ENOERROR;
            tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_DATA_RECV);
        }
    }
    if (result != TM_ENOERROR)
    {
/*
 * Error (other than EWOULDBLOCK) occurred while receiving data, or
 * writing to the file system:
 * Close data file, data and listening sockets if any.
 * Move back a state.
 */
        tfFtpCloseData(ftpSessionPtr);
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
    }
    return TM_EWOULDBLOCK;
}

/*
 * tfFtpMtrxCloseFileData description:
 *
 */
static int tfFtpMtrxCloseFileData (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

/* Close data file, data and listening sockets if any. Go back to REPLY_WAIT */
    (void)tfFtpMtrxCloseData(ftpSessionPtr);

    switch (ftpSessionPtr->ftpMtrxEvent)
    {
        case TM_FTPE_ABORT:
            result = TM_ENOERROR;
            break;
        case TM_FTPE_DATA_ERROR:
            result = TM_EWOULDBLOCK;
            break;
        case TM_FTPE_TIMEOUT:
            result = TM_ETIMEDOUT;
            break;
        default:
            result = TM_EINVAL;
    }
    return (result);
}


/*
 * tfFtpMtrxCtrlReplyRecvIdle description:
 *
 */
static int tfFtpMtrxCtrlReplyRecvIdle (ttFtpSessionPtr ftpSessionPtr)
{
    int result;
    int matrixNo;

/* Process command */
    matrixNo = ftpSessionPtr->ftpMatrixNo;
    result = tfFtpCmdReplyCheck (ftpSessionPtr);

    if ((matrixNo > TM_FTP_MATRIX1) &&
        (ftpSessionPtr->ftpInterCommandCode == TM_CMD_ABOR))
    {
/* Error occured: close data socket and (if necessary) close data file */
        tfFtpCloseData(ftpSessionPtr);
    }

    tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);
    return (result);
}


/*
 * tfFtpMtrxDataSend description:
 *
 */
static int tfFtpMtrxDataSend (ttFtpSessionPtr ftpSessionPtr)
{
    int result;
    int socketDesc;
    int sendSize;
    int errorCode;

    if (ftpSessionPtr->ftpDataBufSendSize == ftpSessionPtr->ftpDataBufSendIndex)
    {

/* Read data from file to send */
        result = tfFSReadFile (ftpSessionPtr->ftpUserDataPtr,
                               ftpSessionPtr->ftpFilePointer,
                               (ttCharPtr) &ftpSessionPtr->ftpDataSendBuf[0],
                               TM_BUF_SEND_SIZE);
        if (result > 0)
        {
/* File read OK, set send buffer size and index */
            ftpSessionPtr->ftpDataBufSendSize  = result;
            ftpSessionPtr->ftpDataBufSendIndex = 0;
            tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_DATA_SEND);
            errorCode = TM_EWOULDBLOCK;
        }
        else
        {
/* An error occured, so close data socket and data file, and reset FSM */
            if (result < 0)
            {
                socketDesc = ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET];
                tfResetConnection(socketDesc);
            }
            tfFtpCloseData(ftpSessionPtr);
            tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
            errorCode = TM_EWOULDBLOCK;
        }
    }
    else
    {
/* Send some data from the send buffer to the server */
        socketDesc = ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET];
        sendSize   = ftpSessionPtr->ftpDataBufSendSize -
            ftpSessionPtr->ftpDataBufSendIndex;

        tfFtpClearSignal (ftpSessionPtr, socketDesc, TM_CB_WRITE_READY);

        result = send (socketDesc,
                       (ttCharPtr) &ftpSessionPtr->ftpDataSendBuf
                            [ftpSessionPtr->ftpDataBufSendIndex],
                       sendSize, MSG_DONTWAIT );

        if (result == sendSize)
        {
/* All data sent successfully: move to next state */

            ftpSessionPtr->ftpDataBufSendIndex += result;

            errorCode = TM_EWOULDBLOCK;
        }
        else
        {
            if (result < 0)
            {
                errorCode = tfGetSocketError (socketDesc);
                if (errorCode == TM_EWOULDBLOCK)
                {
                    tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_SEND_WAIT);
                }
                else
                {
/* An error occurred: close data socket and data file, reset FSM */
                    tfFtpCloseData(ftpSessionPtr);
                    tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
                    errorCode = TM_EWOULDBLOCK;
                }
            }
            else
            {

                ftpSessionPtr->ftpDataBufSendIndex += result;

                errorCode = TM_EWOULDBLOCK;
            }
        }
    }

    return (errorCode);
}


/*
 * tfFtpMtrxSendComplete description:
 *   Called when data send has completed:  moves to the Data-Send state.
 *
 */
static int tfFtpMtrxSendComplete (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_DATA_SEND);

    result = TM_EWOULDBLOCK;
    return (result);
}

/*
 * tfFtpMtrxNoop description:
 *   Place state function; does nothing.
 *
 */
static int tfFtpMtrxNoop (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    TM_UNREF_IN_ARG(ftpSessionPtr);

    result = TM_EWOULDBLOCK;
    return (result);
}

/*
 * tfFtpDataDefault description:
 * Initializes FTP session structure.  Called by tfFtpNewSession().
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session
 *  blockingState       Blocking or non blocking
 *                          (TM_BLOCKING_ON or TM_BLOCKING_OFF)
 *
 * Return:
 *  none
 */
static void tfFtpDataDefault (ttFtpSessionPtr ftpSessionPtr, int blockingState)
{
/* Clear FTP session structure */
    tm_bzero (ftpSessionPtr, sizeof (ttFtpSession));
/*
 * Following fields are all zeroes.
 * No initial command request
 *   ftpSessionPtr->ftpCmdRequest = TM_CMD_IDLE;
 *
 *   ftpSessionPtr->ftpSendRequest = TM_SEND_REQ_OFF;
 *
 *   ftpSessionPtr->ftpMatrixNo  = TM_FTP_MATRIX0;
 *   ftpSessionPtr->ftpMtrxPhase = TM_FTPP_INITIAL;
 *   ftpSessionPtr->ftpMtrxEvent = TM_FTPE_CONNECT;
 *
 *   ftpSessionPtr->ftpCommandCode      = TM_CMD_IDLE;
 *   ftpSessionPtr->ftpNextCommandCode  = TM_CMD_IDLE;
 *   ftpSessionPtr->ftpInterCommandCode = TM_CMD_IDLE;
 */
/*  ftpSessionPtr->ftpTimerFlag     = TM_FTP_TIMER_OFF; */ /* zero */
/*  ftpSessionPtr->ftpTelState        = TM_TEL_DATA_STATE; */ /* zero */
/*  ftpSessionPtr->ftpDataBufTelState = TM_TEL_DATA_STATE; */ /* zero */
/*  ftpSessionPtr->ftpTransMode      = TM_MODE_STREAM; *//* zero*/
/*  ftpSessionPtr->ftpTransType      = TM_TYPE_ASCII; */ /* zero */
/*  ftpSessionPtr->ftpTransForm      = TM_FORM_NONPRINT; */ /* zero */
/*  ftpSessionPtr->ftpTransStructure = TM_STRU_STREAM; */ /* zero */
/* Set blocking state, according to user */
    ftpSessionPtr->ftpBlockingState = blockingState;
/* All sockets are initially closed (i.e., equal to TM_SOCKET_ERROR) */
    ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET]   = TM_SOCKET_ERROR;
    ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET]   = TM_SOCKET_ERROR;
    ftpSessionPtr->ftpSocketDesc[TM_FTP_LISTEN_SOCKET] = TM_SOCKET_ERROR;
#ifdef TM_USE_TCP_REXMIT_TEST
    tm_context(tvFtpSockDesc) = TM_SOCKET_ERROR;
#endif /* TM_USE_TCP_REXMIT_TEST */
/* Set default port values (overridden by PORT command) */
    ftpSessionPtr->ftpPortNo    = (ttIpPort) TM_LOCAL_DATA_PORT;
    ftpSessionPtr->ftpNowPortNo = (ttIpPort) TM_LOCAL_DATA_PORT;
    ftpSessionPtr->ftpPortParm  = (ttIpPort) TM_LOCAL_DATA_PORT;
    ftpSessionPtr->ftpReplyCode      = -1;
#ifdef TM_USE_FTP_SSL
/*  ftpSessionPtr->ftpSslEnabledCtrlConn = TM_8BIT_NO; */ /* zero */
/*  ftpSessionPtr->ftpSslEnabledDataConn = TM_8BIT_NO; */ /* zero */
/*  ftpSessionPtr->ftpSslNegFlagsCtrlConn = TM_FTP_SSL_NOT_NEG; */ /* zero */
/*  ftpSessionPtr->ftpSslNegFlagsDataConn = TM_FTP_SSL_NOT_NEG; *//* zero */
/*  ftpSessionPtr->ftpSslNegInProgress    = TM_8BIT_NO; */ /* zero */
/*  ftpSessionPtr->ftpSslDataConnReq = TM_8BIT_NO; */ /* zero */
    ftpSessionPtr->ftpDataProtLevel = 'C';
    ftpSessionPtr->ftpSslSessionId    = -1;
#ifdef TM_SSL_CLIENT_RESPONSE_NO_CERT
    ftpSessionPtr->ftpSslOptions  = TM_SSL_OPT_CLIENT_NOCERT;
#endif /* TM_SSL_CLIENT_RESPONSE_NO_CERT */
#endif /* TM_USE_FTP_SSL */
}

/*
 * tfFtpDataDefault2 description:
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session
 *
 * Return:
 *  none
 */
static void    tfFtpDataDefault2 (
            ttFtpSessionPtr                 ftpSessionPtr )
{
    ftpSessionPtr->ftpCommandCode      = TM_CMD_IDLE;
    ftpSessionPtr->ftpNextCommandCode  = TM_CMD_IDLE;
    ftpSessionPtr->ftpInterCommandCode = TM_CMD_IDLE;


    ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET]   = TM_SOCKET_ERROR;
    ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET]   = TM_SOCKET_ERROR;
    ftpSessionPtr->ftpSocketDesc[TM_FTP_LISTEN_SOCKET] = TM_SOCKET_ERROR;
#ifdef TM_USE_TCP_REXMIT_TEST
    tm_context(tvFtpSockDesc) = TM_SOCKET_ERROR;
#endif /* TM_USE_TCP_REXMIT_TEST */

    ftpSessionPtr->ftpCBFlags[TM_FTP_CTRL_SOCKET]   = (tt16Bit) 0;
    ftpSessionPtr->ftpCBFlags[TM_FTP_DATA_SOCKET]   = (tt16Bit) 0;
    ftpSessionPtr->ftpCBFlags[TM_FTP_LISTEN_SOCKET] = (tt16Bit) 0;

    ftpSessionPtr->ftpUserNamePtr         = (ttCharPtr) 0;
    ftpSessionPtr->ftpUserPasswordNamePtr = (ttCharPtr) 0;
    ftpSessionPtr->ftpUserAccountNamePtr  = (ttCharPtr) 0;

    ftpSessionPtr->ftpPathNamePtr = (ttCharPtr) 0;

    ftpSessionPtr->ftpFromNamePtr = (ttCharPtr) 0;
    ftpSessionPtr->ftpToNamePtr   = (ttCharPtr) 0;

    ftpSessionPtr->ftpBufferPtr   = (ttCharPtr) 0;
    ftpSessionPtr->ftpBufferSize  = 0;
    ftpSessionPtr->ftpBufferIndex = 0;

    ftpSessionPtr->ftpCommandPtr = (ttCharPtr) 0;

    ftpSessionPtr->ftpUserCBFuncPtr = (ttFtpCBFuncPtr)0;

    ftpSessionPtr->ftpLocalFileNamePtr  = (ttCharPtr)0;
    ftpSessionPtr->ftpRemoteFileNamePtr = (ttCharPtr)0;


    ftpSessionPtr->ftpFilePointer = (void TM_FAR *) 0;

    ftpSessionPtr->ftpTelState        = TM_TEL_DATA_STATE;
    ftpSessionPtr->ftpDataBufTelState = TM_TEL_DATA_STATE;

    ftpSessionPtr->ftpTransParm      = 0;

    ftpSessionPtr->ftpReplyLineIndex = (tt16Bit)0;
    ftpSessionPtr->ftpReplyCode      = -1;

    ftpSessionPtr->ftpTelBufReadIndex    = 0;
    ftpSessionPtr->ftpTelBufProcessIndex = 0;

    ftpSessionPtr->ftpDirListIndex = 0;

    ftpSessionPtr->ftpDataBufReadIndex    = 0;
    ftpSessionPtr->ftpDataBufProcessIndex = 0;

    ftpSessionPtr->ftpDataBufSendSize  = 0;
    ftpSessionPtr->ftpDataBufSendIndex = 0;

#ifdef TM_USE_FTP_SSL
    ftpSessionPtr->ftpSslEnabledCtrlConn = TM_8BIT_NO;
    ftpSessionPtr->ftpSslEnabledDataConn = TM_8BIT_NO;

    ftpSessionPtr->ftpSslNegFlagsCtrlConn = TM_FTP_SSL_NOT_NEG;
    ftpSessionPtr->ftpSslNegFlagsDataConn = TM_FTP_SSL_NOT_NEG;
    ftpSessionPtr->ftpSslNegInProgress    = TM_8BIT_NO;
    ftpSessionPtr->ftpSslWaitCloseNotify  = TM_8BIT_NO;

    ftpSessionPtr->ftpDataProtLevel = 'C';

    ftpSessionPtr->ftpCertIdentityPtr = (ttCharPtr)0;
    ftpSessionPtr->ftpSslVersion      = 0;
    ftpSessionPtr->ftpSslServerName   = (ttCharPtr)0;

    ftpSessionPtr->ftpSslDataConnReq = TM_8BIT_NO;
#endif /* TM_USE_FTP_SSL */
}


/*
 * tfFtpSetCommandCode description:
 *   Sets the current command code (such as LOGIN,ABOR,etc) as well as the
 *   next command code and intermediate command code in the current FTP
 *   session.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  commandCode         Command code
 *  nextCommandCode     Next command code
 *
 * Returns:
 *  none
 */
static void tfFtpSetCommandCode (ttFtpSessionPtr ftpSessionPtr,
                          tt8Bit          commandCode,
                          tt8Bit          nextCommandCode)
{

/* If the command is QUIT,ABOR or STATE, simply set the intermediate command
 * code to QUIT,ABOR,STATE.
 */
    if ((commandCode == TM_CMD_QUIT) ||
        (commandCode == TM_CMD_ABOR) ||
        (commandCode == TM_CMD_STAT))
    {
        ftpSessionPtr->ftpInterCommandCode = commandCode;
    }
    else
    {
/* Otherwise, set the command codes to what the user specified. */
        ftpSessionPtr->ftpCommandCode      = commandCode;
        ftpSessionPtr->ftpNextCommandCode  = nextCommandCode;
        ftpSessionPtr->ftpInterCommandCode = TM_CMD_IDLE;
    }

}

/*
 * tfFtpCmdReplyCheck description:
 *   Receives, parses and processes a reply by examining and validating the
 *   reply code and calling the correct reply handler for the current command.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_EALREADY         Operation already in progress.
 *  TM_EINVAL           Invalid argument.
 *  TM_EACCES           Permission denied.
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_EOPNOTSUPP       Operation not supported.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdReplyCheck (ttFtpSessionPtr ftpSessionPtr)
{
    ttFtpFuncPtr replyFuncPtr;
    tt8Bit       replyIndex;
    int          result;
    int          commandCode;

/* Get reply line from control socket */
    result = tfFtpCmdReplyLineReceive (ftpSessionPtr);

    if (result == TM_ENOERROR)
    {
        if (tm_16bit_one_bit_set (ftpSessionPtr->ftpFlags,
                                  TM_FTP_F_REPLY_RECVED ) )
        {
            if ( ftpSessionPtr->ftpCommandCode != TM_CMD_IDLE )
            {
                commandCode = ftpSessionPtr->ftpCommandCode;
            }
            else
            {
                commandCode = ftpSessionPtr->ftpInterCommandCode;
            }

            if (tm_is_reply_code(ftpSessionPtr->ftpReplyCode))
            {
/*
 * Valid reply code, so call the reply handler function for this command
 * and reply code.
 */
                replyIndex = (tt8Bit)(ftpSessionPtr->ftpReplyCode / 100);
                replyFuncPtr =
                    tlFtpCmdTable[commandCode].fctCmdReplyPtr[replyIndex];
                result = (*(replyFuncPtr))(ftpSessionPtr);
            }
            else
            {
/* Invalid code, so call the error handler function for this command */
                replyFuncPtr =
                   tlFtpCmdTable[commandCode].fctCmdReplyPtr[TM_REPLY_CODE_ERR];
                result = (*(replyFuncPtr))(ftpSessionPtr);
            }

/* Indicate that this reply has been received and processed */
            tm_16bit_clr_bit(ftpSessionPtr->ftpFlags, TM_FTP_F_REPLY_RECVED);
        }
        else
        {
            result = TM_EWOULDBLOCK;
        }
    }

    return (result);
}


/*
 * tfFtpCmdReplyLineReceive description:
 *   Receive a reply from the control connection and get the reply code
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_EALREADY         Operation already in progress.
 *  TM_EINVAL           Invalid argument.
 *  TM_EACCES           Permission denied.
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_EOPNOTSUPP       Operation not supported.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdReplyLineReceive (ttFtpSessionPtr ftpSessionPtr)
{
    int result;
    int replyCode;

/* Get a reply from control socket and parse it */
    result = tfFtpCmdReplyReceive (ftpSessionPtr);

    if ( (result == TM_ENOERROR) || (result == TM_EWOULDBLOCK) )
    {
        if ( tm_16bit_one_bit_set(ftpSessionPtr->ftpFlags,
                                  TM_FTP_F_REPLY_RECVED) )
        {

/* Retrieve code from reply */
            replyCode = tfFtpCmdReplyCodeCheck (ftpSessionPtr);
            if (replyCode != -1)
            {
                ftpSessionPtr->ftpReplyCode = replyCode;
            }

            result = TM_ENOERROR;
        }
    }

    return (result);
}


/*
 * tfFtpCmdReplyReceive description:
 *   Receives a reply from the FTP server.  This includes receiving the data
 *   from the socket and parsing the data (in telnet format).
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_EALREADY         Operation already in progress.
 *  TM_EINVAL           Invalid argument.
 *  TM_EACCES           Permission denied.
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_EOPNOTSUPP       Operation not supported.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdReplyReceive (ttFtpSessionPtr ftpSessionPtr)
{
    ttCharPtr bufferPtr;
    tt8BitPtr telInputBufferPtr;
    tt8Bit    telChar;
    int       result;
    int       socketDesc;
    int       recvSize;
    int       unprocessedBytes;
    int       bufferSize;
    int       i;
    int       waitingBytes;
    tt8Bit    processCurrentChar;


    socketDesc = ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET];

/* Receive outstanding data on the control socket */
    bufferSize = TM_CMD_LINE_SIZE - ftpSessionPtr->ftpTelBufReadIndex;
    telInputBufferPtr = &ftpSessionPtr->ftpTelInputBuf[0];
    bufferPtr  = (ttCharPtr)
        &(telInputBufferPtr[ftpSessionPtr->ftpTelBufReadIndex]);
    recvSize = 0; /* compiler warning */
    waitingBytes = tfGetWaitingBytes(socketDesc);
    if (waitingBytes > 0)
    {
        result = tfFtpSocketReceive (ftpSessionPtr,
                                     socketDesc,
                                     bufferPtr,
                                     bufferSize,
                                     &recvSize );
    }
    else
    {
        result = TM_ENOERROR;
    }

    if ( (result == TM_ENOERROR) || (result == TM_EWOULDBLOCK) )
    {

        ftpSessionPtr->ftpTelBufReadIndex =
            ftpSessionPtr->ftpTelBufReadIndex + recvSize;

/*
 * FTP control data is encoded in the standard 'telnet' format, so we have to
 * parse the reply we received
 */
        while (ftpSessionPtr->ftpTelBufProcessIndex <
            ftpSessionPtr->ftpTelBufReadIndex)
        {
            telChar = telInputBufferPtr[ftpSessionPtr->ftpTelBufProcessIndex];
            ftpSessionPtr->ftpTelBufProcessIndex ++;
/* processCurrentChar will be set below as needed. */
            processCurrentChar = TM_8BIT_ZERO;
            switch (ftpSessionPtr->ftpTelState)
            {
/*
 * The last character processed was a CR.  If the current character is a LF,
 * this signifies the end of a line, and therefore the end of our reply:  set
 * the end of string in our reply buffer and set flags to indicate that a
 * reply has been received.
 */
                case TM_TEL_CR_STATE:
                    processCurrentChar = TM_8BIT_YES;
                    ftpSessionPtr->ftpTelState = TM_TEL_DATA_STATE;
                    if (telChar == TM_NVT_LF)
                    {
/* '\r\n' maps to '\0' (our end of line) */
                        telChar = '\0';
                        ftpSessionPtr->ftpFlags |=
                            (tt16Bit)TM_FTP_F_REPLY_RECVED;
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
                            ftpSessionPtr->ftpTelBufProcessIndex--;
                        }
                    }
                    break;

                case TM_TEL_DATA_STATE:
/*
 * If current character is a CR, change states so on the next character we can
 * check for a CR-LF pair;  otherwise, just place this data in our reply
 * buffer.
 */
                    if (telChar == TM_NVT_CR)
                    {
                        ftpSessionPtr->ftpTelState = TM_TEL_CR_STATE;
                    }
                    else
                    {
/* Character is not '\r' and we are in data state, process it */
                        processCurrentChar = TM_8BIT_YES;
                    }
                    break;
                default:
                    break;
            }
            if (processCurrentChar != TM_8BIT_ZERO)
            {
/* Process current character. */
                if (ftpSessionPtr->ftpReplyLineIndex < TM_CMD_LINE_SIZE)
                {
                    ftpSessionPtr->ftpReplyLine
                            [ftpSessionPtr->ftpReplyLineIndex] = telChar;
                    ftpSessionPtr->ftpReplyLineIndex++;
                }
            }
/*
 * A complete reply has been received, so set string terminator and deal with
 * unprocessed data.
 */
            if ( tm_16bit_one_bit_set (ftpSessionPtr->ftpFlags,
                                       TM_FTP_F_REPLY_RECVED) )
            {
                if (ftpSessionPtr->ftpReplyLineIndex == TM_CMD_LINE_SIZE)
                {
/*
 * If we overflowed our input buffer, make sure the command is null
 * terminated.
 */
                    ftpSessionPtr->ftpReplyLine
                            [TM_CMD_LINE_SIZE - 1] = '\0';
                }
                ftpSessionPtr->ftpReplyLineIndex = 0;
                break;
            }
        }
/*
 * If there is any unprocessed data, copy it into our receive buffer.  Set
 * index values accordingly.
 */
        unprocessedBytes = ftpSessionPtr->ftpTelBufReadIndex -
            ftpSessionPtr->ftpTelBufProcessIndex;

        if (unprocessedBytes != 0)
        {
            for (i = 0; i < unprocessedBytes; i++)
            {
/* Avoid overlapping bcopy by copying up one byte at a time */
                telInputBufferPtr[i] =
                  telInputBufferPtr[ftpSessionPtr->ftpTelBufProcessIndex + i];
            }

            ftpSessionPtr->ftpTelBufProcessIndex = 0;
            ftpSessionPtr->ftpTelBufReadIndex = unprocessedBytes;
        }
        else
        {
            ftpSessionPtr->ftpTelBufProcessIndex = 0;
            ftpSessionPtr->ftpTelBufReadIndex    = 0;
        }
    }

    return (result);
}


/*
 * tfFtpCmdReplyCodeCheck description:
 *   Parses reply and returns the numerical value of the reply code.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *                      Reply code
 */
static int tfFtpCmdReplyCodeCheck (ttFtpSessionPtr ftpSessionPtr )
{
    int replyCode;

/* Check to make sure the data fits the format [0-9][0-9][0-9] */
    if ( ( (ftpSessionPtr->ftpReplyLine[0] >= '0') &&
                (ftpSessionPtr->ftpReplyLine[0] <= '9') ) &&
         ( (ftpSessionPtr->ftpReplyLine[1] >= '0') &&
                (ftpSessionPtr->ftpReplyLine[1] <= '9') ) &&
         ( (ftpSessionPtr->ftpReplyLine[2] >= '0') &&
                (ftpSessionPtr->ftpReplyLine[2] <= '9') ) )
    {

/* Convert reply code (ASCII) to an integer */
        replyCode = ( ftpSessionPtr->ftpReplyLine[0] & 0x0f ) * 100 +
                    ( ftpSessionPtr->ftpReplyLine[1] & 0x0f ) * 10 +
                    ( ftpSessionPtr->ftpReplyLine[2] & 0x0f );
    }
    else
    {

/* Invalid reply code (not of above format) */
        replyCode = -1;
    }

    return (replyCode);
}


/*
 * tfFtpSocketReceive description:
 *   FTP receive routine: recv + error checking.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  socketDesc          Socket descriptor
 *  bufferPtr           Pointer to receive buffer
 *  bufferSize          Receive buffer size
 *  recvSizePtr         Pointer to received size
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_EALREADY         Operation already in progress.
 *  TM_EINVAL           Invalid argument.
 *  TM_EACCES           Permission denied.
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_EOPNOTSUPP       Operation not supported.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpSocketReceive (ttFtpSessionPtr ftpSessionPtr,
                        int             socketDesc,
                        ttCharPtr       bufferPtr,
                        int             bufferSize,
                        ttIntPtr        recvSizePtr )
{
    int result;
    int errorCode;

    TM_UNREF_IN_ARG(ftpSessionPtr);

    result = recv(socketDesc, bufferPtr, bufferSize, MSG_DONTWAIT);

/* If an error occurred, return error and a length of zero */
    if (result == TM_SOCKET_ERROR)
    {
        errorCode = tfGetSocketError (socketDesc);
        if (errorCode == TM_EWOULDBLOCK)
        {
            *recvSizePtr = 0;
        }

    }
    else
    {
        *recvSizePtr = result;
        errorCode    = TM_ENOERROR;
    }

    return (errorCode);
}


/*
 * tfFtpCommandSend description:
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCommandSend (ttFtpSessionPtr ftpSessionPtr)
{
    int result;
    int socketDesc;
    int length;
    int errorCode;

/* send command over the control connection */
    socketDesc = ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET];

    length = (int)tm_strlen((ttCharPtr) ftpSessionPtr->ftpCmdSendBuf);

    result = send (socketDesc,
                   (ttCharPtr) &ftpSessionPtr->ftpCmdSendBuf[0],
                   length,
                   MSG_DONTWAIT);

/* Return the result to the user */
    if (result != length)
    {
        if (result == TM_SOCKET_ERROR)
        {
            errorCode = tfGetSocketError (socketDesc);
        }
        else
        {
            errorCode = TM_EWOULDBLOCK;
        }
    }
    else
    {
        errorCode = TM_ENOERROR;
    }

    return (errorCode);
}


/*
 * tfFtpCtrlCharSend description:
 *   Send control character to FTP server.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  ctrlCharPtr         Pointer to send buffer
 *  sendSize            Size of the zero copy send buffer
 *  flags
 * Returns:
 *  Value               Meaning
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCtrlCharSend (ttFtpSessionPtr ftpSessionPtr,
                       ttCharPtr       ctrlCharPtr,
                       int             sendSize,
                       int             flags)
{
    int result;
    int socketDesc;
    int errorCode;

    socketDesc = ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET];

    result = send (socketDesc, ctrlCharPtr, sendSize, flags);
    if (result != sendSize)
    {
        if (result == TM_SOCKET_ERROR)
        {
            errorCode = tfGetSocketError (socketDesc);
        }
        else
        {
            errorCode = TM_EWOULDBLOCK;
        }
    }
    else
    {
        errorCode = TM_ENOERROR;
    }

    return ( errorCode );
}

/*
 * tfFtpCloseCtrlSocket description:
 *   If not already closed, close the control socket and reinitialize the
 *   session.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  none
 */
static void tfFtpCloseCtrlSocket (ttFtpSessionPtr ftpSessionPtr)
{
    if (ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET] != TM_SOCKET_ERROR)
    {

/* Close data file, data and listening sockets if any */
        tfFtpCloseData(ftpSessionPtr);

/* Close control socket */
        tfFtpCloseSocket(ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET]);

        ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET] = TM_SOCKET_ERROR;
        ftpSessionPtr->ftpCBFlags[TM_FTP_CTRL_SOCKET]    = 0;

        tfFtpDataDefault2 (ftpSessionPtr);

        tm_16bit_clr_bit(ftpSessionPtr->ftpFlags, TM_FTP_F_CTRL_CONN);
    }
}

/*
 * tfFtpCloseData description:
 *   Call tfFtpCloseDataFile() to close the data file.
 *   If not already closed, close the data and listening socket and set flags
 *   to indicate that the data connection no longer open.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  none
 */
static void tfFtpCloseData (ttFtpSessionPtr ftpSessionPtr)
{
    tfFtpCloseDataFile(ftpSessionPtr);
    if (ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET] != TM_SOCKET_ERROR)
    {
        tfFtpCloseSocket (ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET]);
        ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET] = TM_SOCKET_ERROR;
        ftpSessionPtr->ftpCBFlags[TM_FTP_DATA_SOCKET] = 0;
#ifdef TM_USE_TCP_REXMIT_TEST
        tm_context(tvFtpSockDesc) = TM_SOCKET_ERROR;
#endif /* TM_USE_TCP_REXMIT_TEST */
    }

    if (ftpSessionPtr->ftpSocketDesc[TM_FTP_LISTEN_SOCKET] != TM_SOCKET_ERROR)
    {
        tfFtpCloseSocket (ftpSessionPtr->ftpSocketDesc[TM_FTP_LISTEN_SOCKET]);
        ftpSessionPtr->ftpSocketDesc[TM_FTP_LISTEN_SOCKET] = TM_SOCKET_ERROR;
        ftpSessionPtr->ftpCBFlags[TM_FTP_LISTEN_SOCKET] = 0;
    }
    tm_16bit_clr_bit(ftpSessionPtr->ftpFlags, TM_FTP_F_DATA_CONN);
}


/*
 * tfFtpCloseSocket description:
 *   Removes socket callbacks and closes socket.
 *
 * Parameters:
 *  Value               Meaning
 *  socketDesc          Socket descriptor to close.
 *
 * Returns:
 *  none
 */
static void tfFtpCloseSocket (int socketDesc)
{
    (void) tfRegisterSocketCBParam (socketDesc,
                                    (ttUserSocketCBParamFuncPtr) 0,
                                    (void TM_FAR *)0,
                                    0);
    (void) tfClose (socketDesc);
}



/*
 * tfFtpSignal function description:
 *   Socket callback function:  called when certain events happen on the
 *   control, data or listening sockets.  When this occurs, we set the
 *   appropriate flags in the FTP state vector, which will indicate that this
 *   event has occurred.
 *
 * Parameters:
 *  Value               Meaning
 *  socketDesc          socket (data or control) CB occurs on
 *  ftpPtr              pointer to the corresponding FTP session structure.
 *  CBflags             call back flags on.
 *
 * Returns:
 *  none
 */
static void tfFtpSignal (int socketDesc, ttVoidPtr ftpPtr, int CBFlags)
{
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpPtr;
    if (ftpSessionPtr != (ttFtpSessionPtr) 0)
    {
        if (socketDesc == ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET])
        {
                ftpSessionPtr->ftpCBFlags[
                    TM_FTP_CTRL_SOCKET] |= (tt16Bit)CBFlags;
        }
        else
        {
            if (socketDesc == ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET])
            {
                ftpSessionPtr->ftpCBFlags[
                    TM_FTP_DATA_SOCKET] |= (tt16Bit)CBFlags;
            }
            else
            {
                if (socketDesc ==
                        ftpSessionPtr->ftpSocketDesc[TM_FTP_LISTEN_SOCKET])
                {
                    ftpSessionPtr->ftpCBFlags[
                        TM_FTP_LISTEN_SOCKET] |= (tt16Bit)CBFlags;
                }
            }
        }
    }
}


/*
 * tfFtpGetSignal description:
 *   returns the callback flags associated with a particular type of FTP socket
 *   (control, data, etc).
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  socketDesc          Socket descriptor
 *
 * Returns:
 *  Value               Meaning
 *  0                   invalid socketDesc or ftpSessionPtr
 *  >=0                 Socket signal
 */

static int tfFtpGetSignal (ttFtpSessionPtr ftpSessionPtr, int socketDesc)
{
    int signal;

    signal = 0;
    if (    (ftpSessionPtr != (ttFtpSessionPtr) 0)
         && (socketDesc != TM_SOCKET_ERROR) )
    {
        if (socketDesc == ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET])
        {
            signal = ftpSessionPtr->ftpCBFlags[TM_FTP_CTRL_SOCKET];
        }
        else
        {
            if (socketDesc == ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET])
            {
                signal = ftpSessionPtr->ftpCBFlags[TM_FTP_DATA_SOCKET];
            }
            else
            {
                if (socketDesc ==
                        ftpSessionPtr->ftpSocketDesc[TM_FTP_LISTEN_SOCKET])
                {
                    signal = ftpSessionPtr->ftpCBFlags[TM_FTP_LISTEN_SOCKET];
                }

            }
        }
    }

    return (signal);
}


/*
 * tfFtpSetSignal description:
 *   Enables the specified signal (callback flag) in the specified socket
 *   (data, control, etc).
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  socketDesc          Socket descriptor
 *  setSignal           Set signal
 *
 * Returns:
 *  none
 */

static void tfFtpSetSignal (ttFtpSessionPtr ftpSessionPtr,
                     int             socketDesc,
                     int             setSignal )
{
    if (    (ftpSessionPtr != (ttFtpSessionPtr) 0)
         && (socketDesc != TM_SOCKET_ERROR) )
    {
        if (socketDesc ==  ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET])
        {
            ftpSessionPtr->ftpCBFlags[TM_FTP_CTRL_SOCKET] |= (tt16Bit)setSignal;
        }
        else
        {
            if (socketDesc == ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET])
            {
                ftpSessionPtr->ftpCBFlags[
                    TM_FTP_DATA_SOCKET] |= (tt16Bit)setSignal;
            }
            else
            {
                if (socketDesc ==
                      ftpSessionPtr->ftpSocketDesc[TM_FTP_LISTEN_SOCKET])
                {
                    ftpSessionPtr->ftpCBFlags[TM_FTP_LISTEN_SOCKET] |=
                        (tt16Bit)setSignal;
                }
            }
        }
    }
}


/*
 * tfFtpClearSignal description:
 *   Disables the specified signal (callback flag) in the specified socket
 *   (data, control, etc).
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  socketDesc          Socket descriptor
 *  clearSignal         Clear signal
 *
 * Returns:
 *  none
 */

static void tfFtpClearSignal (ttFtpSessionPtr ftpSessionPtr,
                       int             socketDesc,
                       int             clearSignal)
{


    if (ftpSessionPtr != (ttFtpSessionPtr) 0)
    {
        if (socketDesc ==  ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET])
        {
            ftpSessionPtr->ftpCBFlags[
                TM_FTP_CTRL_SOCKET] &= ~((tt16Bit)clearSignal);
        }
        else
        {
            if (socketDesc == ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET])
            {
                ftpSessionPtr->ftpCBFlags[
                    TM_FTP_DATA_SOCKET] &= ~((tt16Bit)clearSignal);
            }
            else
            {
                if (socketDesc ==
                      ftpSessionPtr->ftpSocketDesc[TM_FTP_LISTEN_SOCKET])
                {
                    ftpSessionPtr->ftpCBFlags[TM_FTP_LISTEN_SOCKET] &=
                        ~((tt16Bit)clearSignal);
                }
            }
        }
    }
}

/*
 * tfFtpCheckSession description:
 *   Checks the status of the FTP session to verify that the connection
 *   is established and/or the user is connected.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  flags               TM_FTP_F_LOGGED_IN: Check if user is logged in.
 *                      TM_FTP_F_CTRL_CONN: Check if connection is established.
 *
 * Returns:
 *  Value               Meaning
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_ENOERROR (0)     No error (Success.)
 */
static int tfFtpCheckSession (ttFtpSessionPtr ftpSessionPtr, int flags)
{
    int result;

    if (ftpSessionPtr != 0)
    {
        result = TM_ENOERROR;

        if (ftpSessionPtr->ftpMtrxPhase == TM_FTPP_INITIAL)
        {

/* If checking for login status, and not logged in, return TM_ENOTLOGIN */
            if (flags & TM_FTP_F_LOGGED_IN)
            {
                if ( tm_16bit_bits_not_set(ftpSessionPtr->ftpFlags,
                                           TM_FTP_F_LOGGED_IN) )
                {
                    result = TM_ENOTLOGIN;
                }
            }

/* If checking for connection status, and no control connection,
 * return TM_ENOTCONN
 */
            if (flags & TM_FTP_F_CTRL_CONN)
            {
                if ( tm_16bit_bits_not_set (ftpSessionPtr->ftpFlags,
                                            TM_FTP_F_CTRL_CONN) )
                {
                    result = TM_ENOTCONN;
                }
            }
        }
        else
        {
/* If we're in an incorrect phase, return TM_EACCES. */
            result = TM_EACCES;
        }
    }
    else
    {
        result = TM_EINVAL;
    }

    return (result);
}


/*
 * tfFtpCheckInterSession description:
 *   First check the session: if access was denied and the current
 *   command is RETR,STOR,STOU,APPE, NLST or LIST, result is NOERROR; otherwise,
 *   result is EACCES.  Basically a special case of tfFtpCheckSession.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  flags
 *
 * Returns:
 *  Value               Meaning
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_ENOERROR (0)     No error (Success.)
 */
static int tfFtpCheckInterSession (ttFtpSessionPtr ftpSessionPtr, int flags)
{
    int result;
    int userCommand;

/* Verify FTP session */
    result = tfFtpCheckSession (ftpSessionPtr, flags);

/* If access was denied and the current command is RETR,STOR,STOU,APPE,
 * NLST or LIST, result is NOERROR; otherwise, result is EACCES.
 */
    if (result == TM_EACCES)
    {

        userCommand = (int) ftpSessionPtr->ftpCommandCode;

        if ( (userCommand == TM_CMD_RETR) || (userCommand == TM_CMD_STOR) ||
             (userCommand == TM_CMD_STOU) || (userCommand == TM_CMD_APPE) ||
             (userCommand == TM_CMD_NLST) || (userCommand == TM_CMD_LIST) )
        {
            if (ftpSessionPtr->ftpInterCommandCode == TM_CMD_IDLE)
            {
                result = TM_ENOERROR;
            }
        }
    }

    return (result);
}


/*
 * tfFtpCheckCommand description:
 *   Does everything that tfFtpCheckSession does (connection status, login
 *   status) plus checks for a NULL parameter passed in by the user.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  userCommand         FTP command to check
 *  argPtr              Argument to above command
 *  flags               TM_USER_F_PARM: Parameter (argPtr) specified.
 *
 * Returns:
 *  Value               Meaning
 *  TM_EINVAL           Invalid ftpSessionPtr.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_ENOERROR (0)     No error (Success.)
 */
static int tfFtpCheckCommand (ttFtpSessionPtr ftpSessionPtr,
                       int             userCommand,
                       ttCharPtr       argPtr,
                       int             flags)
{
    int result;
    unsigned int tempUInt;

/* WHY IS THIS PARAMETER EVEN INCLUDED???? */
    TM_UNREF_IN_ARG(userCommand);

/* Check for NULL parameter passed in from user */
    tempUInt = tm_strlen(argPtr);
    if ((flags & TM_USER_F_PARM) && (tempUInt == 0))
    {
        result = TM_EINVAL;
    }
    else
    {
        result = tfFtpCheckSession(ftpSessionPtr, flags);
    }

    return (result);
}


/*
 * tfFtpCheckBufferCommand description:
 *   Validate that the user passed in a correct buffer and check the session
 *   for connection status, login status, etc.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EINVAL           Invalid ftpSessionPtr, or userNamePtr null.
 *  TM_ENOERROR (0)     No error (Success.)
 */
static int tfFtpCheckBufferCommand (ttFtpSessionPtr ftpSessionPtr,
                             int             userCommand,
                             ttCharPtr       argPtr,
                             int             flags,
                             ttCharPtr       bufferPtr,
                             int             bufferSize )
{
    int errorCode;
    unsigned int tempUInt;

    TM_UNREF_IN_ARG(userCommand);

/* Validate that the user passed in a correct buffer */
    tempUInt = tm_strlen(argPtr);
    if ( ( (flags & TM_USER_F_PARM) && (tempUInt == 0) ) ||
         (bufferPtr == (ttCharPtr) 0) ||
         (bufferSize <= 0) )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
/* Validate the session for connection status, login status, etc. */
        errorCode = tfFtpCheckSession (ftpSessionPtr, flags);
        if ( errorCode != TM_ENOERROR )
        {
            errorCode = TM_EINVAL;
        }
    }

    return (errorCode);
}


/*
 * tfFtpDataConnRequest  function description:
 *   Prepare for an incoming connection from the server:  create a socket,
 *   bind and listen to this socket, and record all necessary information
 *   in the FTP state vector.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr, or userNamePtr null.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_ENOERROR (0)     No error (Success.)
 */
static int tfFtpDataConnRequest (ttFtpSessionPtr ftpSessionPtr)
{
    int                 result;
    int                 errorCode;
    int                 socketDesc;
    int                 tempInt;
    int                 addrLength;
    ttSockAddrPtrUnion  sockAddr;

    ftpSessionPtr->ftpSocketDesc[TM_FTP_LISTEN_SOCKET] = TM_SOCKET_ERROR;
    ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET]   = TM_SOCKET_ERROR;
#ifdef TM_USE_TCP_REXMIT_TEST
    tm_context(tvFtpSockDesc) = TM_SOCKET_ERROR;
#endif /* TM_USE_TCP_REXMIT_TEST */

/* Create the data socket */
    result = socket ( ftpSessionPtr->ftpClientDataAddress.ss_family,
                      SOCK_STREAM,
                      IP_PROTOTCP );

    if (result >= 0)
    {
        socketDesc = result;
        ftpSessionPtr->ftpSocketDesc[TM_FTP_LISTEN_SOCKET] = result;
        tempInt = 1;
        (void)setsockopt(socketDesc, SOL_SOCKET, SO_REUSEADDR,
                         (const char TM_FAR *) &tempInt, sizeof (int));

        if (ftpSessionPtr->ftpBlockingState == TM_BLOCKING_OFF)
        {
            (void)tfBlockingState(socketDesc, TM_BLOCKING_OFF);
        }

        ftpSessionPtr->ftpCBFlags[TM_FTP_LISTEN_SOCKET] = 0;

/* Register a callback for the data socket on connection accept */
        (void)tfRegisterSocketCBParam(socketDesc,
                                      (ttUserSocketCBParamFuncPtr) tfFtpSignal,
                                      ftpSessionPtr,
                                      TM_CB_ACCEPT);

        sockAddr.sockNgPtr = &ftpSessionPtr->ftpClientDataAddress;
        addrLength = sockAddr.sockNgPtr->ss_len;
        result = bind (socketDesc, sockAddr.sockPtr, addrLength);

        if (result == TM_ENOERROR)
        {
            result = listen (socketDesc, 1);

            if (result == 0)
            {
                errorCode = TM_ENOERROR;
            }
            else
            {
/* If any errors occurred, close the data socket */
                errorCode = tfGetSocketError (socketDesc);
                tfFtpCloseData(ftpSessionPtr);
            }
        }
        else
        {
/* If any errors occurred, close the data socket */
            errorCode = tfGetSocketError (socketDesc);
            tfFtpCloseData(ftpSessionPtr);
        }
    }
    else
    {
        errorCode = tfGetSocketError (TM_SOCKET_ERROR);
    }

    return (errorCode);
}


/*
 * tfFtpPasvModeDataConnect description:
 *   Perform operation for data transfer command.
 *
 * 1. Create the data socket for this ftp session
 * 2. Set reuse addr option
 * 3. Bind socket with client ftp address
 * 4. set TCP_NOPUSH TCP option and set call back flags
 * 5. Connect with FTP server
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionAPtr      Pointer to FTP session structure
 *
 * Return:
 * result               error Code on data connection
 */
static int tfFtpPasvModeDataConnect(ttFtpSessionPtr ftpSessionPtr)
{
    int                 result;
    int                 socketDesc;
    int                 tempInt;
    int                 addrLength;
    ttSockAddrPtrUnion  tempSockAddr;

    result = TM_SOCKET_ERROR;
    ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET] = TM_SOCKET_ERROR;
#ifdef TM_USE_TCP_REXMIT_TEST
    tm_context(tvFtpSockDesc) = TM_SOCKET_ERROR;
#endif /* TM_USE_TCP_REXMIT_TEST */

    tempSockAddr.sockNgPtr = &ftpSessionPtr->ftpClientDataAddress;

/* Open a socket for the data connection that we need to connect on */
    socketDesc = socket(tempSockAddr.sockNgPtr->ss_family,
                        SOCK_STREAM,
                        IP_PROTOTCP);
    if (socketDesc != TM_SOCKET_ERROR)
    {
        ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET] = socketDesc;
#ifdef TM_USE_TCP_REXMIT_TEST
    tm_context(tvFtpSockDesc) = socketDesc;
#endif /* TM_USE_TCP_REXMIT_TEST */
        tempInt = 1;
/* Need to ask for reuse addr option */
        result = setsockopt( socketDesc,
                              SOL_SOCKET,
                              SO_REUSEADDR,
                              (const char TM_FAR *)&tempInt,
                              sizeof(int) );
        if (result == TM_ENOERROR)
        {
            addrLength = tempSockAddr.sockNgPtr->ss_len;
            result = bind(  socketDesc,
                            tempSockAddr.sockPtr,
                            addrLength );
            if (result == TM_ENOERROR)
            {
/*
 * Reduce time wait time, set TCP_NOPUSH, set socket to non blocking,
 * set call back flags.
 */
                result = tfFtpSetDataOptions(  ftpSessionPtr,
                                                 TM_CB_CONNECT_COMPLT
                                               | TM_CB_RECV
                                               | TM_CB_WRITE_READY
                                               | TM_CB_REMOTE_CLOSE
                                               | TM_CB_RESET
                                               | TM_CB_SOCKET_ERROR
#ifdef TM_USE_FTP_SSL
                                               | TM_FTP_SSL_CB_F
#endif /* TM_USE_FTP_SSL */
                                               );
                if (result == TM_ENOERROR)
                {
                    tempSockAddr.sockNgPtr =
                        &ftpSessionPtr->ftpServerDataAddress;
                    addrLength = tempSockAddr.sockNgPtr->ss_len;

                    result = connect( socketDesc,
                                      tempSockAddr.sockPtr,
                                      addrLength );
                    if (result == TM_ENOERROR)
                    {
/* Set CONNECT_COMPLT bit to let FSM know data connection established */
                        ftpSessionPtr->ftpCBFlags[TM_FTP_DATA_SOCKET] |=
                                                        TM_CB_CONNECT_COMPLT;
                    }
                    else
                    {
                        result = tfGetSocketError(socketDesc);
                        if (result == TM_EINPROGRESS)
                        {
                            result = TM_ENOERROR;
/* Connection not established yet. */
                        }
                    }
                }
                else
                {
                    result = tfGetSocketError(socketDesc);
                }
            }
            else
            {
/* If any errors occurred, close the data socket */
                result = tfGetSocketError (socketDesc);
                tfFtpCloseData(ftpSessionPtr);
            }
        }
        else
        {
            result = tfGetSocketError(socketDesc);
        }
    }
    else
    {
        result = tfGetSocketError(socketDesc);
    }

    return result;
}


/*
 * tfFtpSetMatrixPhase description:
 *   Performs matrix phase transition: cancel old timers and start new ones,
 *   set send request flag and set matrix phase.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  matrixPhase         Matrix phase
 *
 * Returns:
 *  none
 */
static void tfFtpSetMatrixPhase (ttFtpSessionPtr ftpSessionPtr,
                                 tt8Bit          matrixPhase)
{

/*
 * In each case, cancel any outstanding timers and set the send request flag
 * appropriately.  If neccessary, start a new phase timer.
 */
    switch (matrixPhase)
    {

        case TM_FTPP_INITIAL:
            ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
            ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;
            ftpSessionPtr->ftpSendRequest = TM_SEND_REQ_OFF;

            tfFtpCancelTimer (ftpSessionPtr);
            ftpSessionPtr->ftpMtrxPhase = matrixPhase;
            break;

        case TM_FTPP_CONNECT_WAIT:
            ftpSessionPtr->ftpSendRequest = TM_SEND_REQ_OFF;

            tfFtpCancelTimer (ftpSessionPtr);
            tfFtpStartTimer (ftpSessionPtr, TM_CONNECT_TIMEOUT);
            ftpSessionPtr->ftpMtrxPhase = matrixPhase;
            break;

        case TM_FTPP_REPLY_WAIT:
            ftpSessionPtr->ftpSendRequest = TM_SEND_REQ_OFF;

            tfFtpCancelTimer (ftpSessionPtr);
            tfFtpStartTimer (ftpSessionPtr, TM_REPLY_TIMEOUT);

            ftpSessionPtr->ftpMtrxPhase = matrixPhase;
            break;


        case TM_FTPP_DATA_CONN:

            ftpSessionPtr->ftpSendRequest = TM_SEND_REQ_OFF;


            tfFtpCancelTimer (ftpSessionPtr);
            tfFtpStartTimer (ftpSessionPtr, TM_ACCEPT_TIMEOUT);

            ftpSessionPtr->ftpMtrxPhase = matrixPhase;
            break;

        case TM_FTPP_DATA_RECV:
            ftpSessionPtr->ftpSendRequest = TM_SEND_REQ_OFF;

            tfFtpCancelTimer (ftpSessionPtr);
            tfFtpStartTimer (ftpSessionPtr, TM_RECV_TIMEOUT);

            ftpSessionPtr->ftpMtrxPhase = matrixPhase;
            break;

        case TM_FTPP_DATA_SEND:

            ftpSessionPtr->ftpSendRequest = TM_SEND_REQ_ON;

            tfFtpCancelTimer (ftpSessionPtr);
            ftpSessionPtr->ftpMtrxPhase = matrixPhase;
            break;

        case TM_FTPP_SEND_WAIT:

            ftpSessionPtr->ftpSendRequest = TM_SEND_REQ_OFF;

            tfFtpCancelTimer (ftpSessionPtr);
            tfFtpStartTimer (ftpSessionPtr, TM_SEND_TIMEOUT);

            ftpSessionPtr->ftpMtrxPhase = matrixPhase;
            break;

        default:
            break;
    }
}


/*
 * tfFtpMatrix description:
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_ENOERROR (0)     No error (Success.)
 */
static int tfFtpMatrix (ttFtpSessionPtr ftpSessionPtr)
{
    int resultA;
    int resultB;

    resultA = TM_EWOULDBLOCK;

    do
    {
/* Execute the Matrix functions */
        resultA = tfFtpMatrixSession ( ftpSessionPtr );
        if ( ((resultA == TM_EWOULDBLOCK)||(resultA == TM_ENOERROR)) &&
             (ftpSessionPtr->ftp2ndSessionPtr != (ttFtpSessionPtr)0) )
        {
/*
 * In two-server model, execute the Matrix functions for the 2nd FTP session
 */
            resultB = tfFtpMatrixSession( ftpSessionPtr->ftp2ndSessionPtr );
            if ( resultB == TM_EWOULDBLOCK )
            {
                resultA = TM_EWOULDBLOCK;
            }
            else
            {
                if ( resultB == TM_ENOERROR )
                {
                    continue;
                }
                else
                {
                    resultA = resultB;
                }
            }
        }
    } while ((resultA == TM_EWOULDBLOCK) &&
             (ftpSessionPtr->ftpBlockingState == TM_BLOCKING_ON));

    return (resultA);
}


/*
 * tfFtpMatrixSession description:
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_ENOERROR (0)     No error (Success.)
 */
static int tfFtpMatrixSession (ttFtpSessionPtr ftpSessionPtr)
{
    int result;
#ifdef TM_USE_FTP_SSL
    tt8Bit nextCommandCode;
#endif /* TM_USE_FTP_SSL */

/* If we're in blocking mode, call select on necessary socket; timeout after
 * 1 second
 */
    if (ftpSessionPtr->ftpBlockingState == TM_BLOCKING_ON)
    {
        tfFtpMatrixCheckBlocking(ftpSessionPtr);
    }

/* Execute the necessary (if any) state function */
    result = tfFtpMatrixCmdReq(ftpSessionPtr);

/* If the control socket has its connect complete callback flag set,
 * execute the TM_FTPE_CTRL_CONN state event
 */
    if (result == TM_EWOULDBLOCK)
    {
        result = tfFtpMatrixCtrlConn(ftpSessionPtr);
    }

/* If the listening socket has its accept callback flag set,
 * execute the TM_FTPE_DATA_CONN state event
 */
    if (result == TM_EWOULDBLOCK)
    {
        result = tfFtpMatrixDataConn (ftpSessionPtr);
    }

/* If there is any data available, or ???, execute either TM_FTPE_CTRL_RECV
 * or TM_FTPE_CTRL_IRECV (IDLE)
 */
    if (result == TM_EWOULDBLOCK)
    {
        result = tfFtpMatrixCtrlRecv (ftpSessionPtr);
    }

/* If the data socket has its receive callback flag set, execute the
 * TM_FTPE_DATA_RECV state event.
 */
    if (result == TM_EWOULDBLOCK)
    {
        result = tfFtpMatrixDataRecv (ftpSessionPtr);
    }

/* If the current matrix phase applies to sending a request, execute the
 * TM_FTPE_SEND_REQ state event.
 */
    if (result == TM_EWOULDBLOCK)
    {
        result = tfFtpMatrixSendReq (ftpSessionPtr);
    }

/* If the current matrix phase applies to a send completing, execute the
 * TM_FTPE_WRITE_RDY state event.
 */
    if (result == TM_EWOULDBLOCK)
    {
        result = tfFtpMatrixSendComp (ftpSessionPtr);
    }

/* If the current matrix phase applies to a control error, and the control
 * socket has its remote close or reset signals set, execute the
 * TM_FTPE_CTRL_ERROR state event.
 */
    if (result == TM_EWOULDBLOCK)
    {
        result = tfFtpMatrixCtrlErro (ftpSessionPtr);
    }

/* If the current matrix phase applies to a data error, and the data socket has
 * its receive signal set, execute state event TM_FTPE_DATA_RECV, otherwise if
 * the remote close or reset signal is set, execute state event
 * TM_FTPE_DATA_ERROR.
 */
    if (result == TM_EWOULDBLOCK)
    {
        result = tfFtpMatrixDataErro (ftpSessionPtr);
    }

/* If the current event is a timeout and the timer is current running,
 * execute state event TM_FTPE_TIMEOUT..
 */
    if (result == TM_EWOULDBLOCK)
    {
        result = tfFtpMatrixTimeOut (ftpSessionPtr);
    }

    if ((result == TM_EWOULDBLOCK) &&
        (ftpSessionPtr->ftpMtrxPhase == TM_FTPP_INITIAL) &&
        (ftpSessionPtr->ftpCmdRequest == TM_CMD_IDLE))
    {
#ifdef TM_USE_FTP_SSL
/*
 * When the state machine is in an idle state, we do not want to return
 * to the user if we are waiting for SSL negotiation to complete. If
 * there is no SSL negotiation taking place, go ahead and return TM_ENOERROR
 * to the user.
 */
        if (ftpSessionPtr->ftpSslNegInProgress == TM_8BIT_NO)
#endif /* TM_USE_FTP_SSL */
        {
            result = TM_ENOERROR;
        }
    }

#ifdef TM_USE_FTP_SSL
    if (ftpSessionPtr->ftpFlags & TM_FTP_F_CTRL_CONN)
    {
        if (ftpSessionPtr->ftpCBFlags[TM_FTP_CTRL_SOCKET]
                 & TM_CB_SSL_HANDSHK_PROCESS )
        {
            ftpSessionPtr->ftpCBFlags[TM_FTP_CTRL_SOCKET] &=
                                                   ~TM_CB_SSL_HANDSHK_PROCESS;
            tfSslUserProcessHandshake(
                        ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET]);
        }
        if (ftpSessionPtr->ftpCBFlags[TM_FTP_CTRL_SOCKET]
                     & TM_CB_SSL_RECV_CLOSE  )
        {
            ftpSessionPtr->ftpCBFlags[TM_FTP_CTRL_SOCKET] &=
                                                   ~TM_CB_SSL_RECV_CLOSE ;
            if (ftpSessionPtr->ftpSslWaitCloseNotify == TM_8BIT_YES)
            {
                ftpSessionPtr->ftpSslWaitCloseNotify   = TM_8BIT_NO;
                if ((ftpSessionPtr->ftpMatrixNo == TM_FTP_MATRIX1) &&
                    (ftpSessionPtr->ftpMtrxPhase == TM_FTPP_REPLY_WAIT) &&
                    (ftpSessionPtr->ftpCommandCode == TM_CMD_REIN))
                {
/* Remove the SSL session. */
                    tfSslSessionUserClose(ftpSessionPtr->ftpSslSessionId);
/* Reset all related flags. */
                    ftpSessionPtr->ftpSslSessionId         = -1;
                    ftpSessionPtr->ftpSslNegFlagsCtrlConn  = TM_FTP_SSL_NOT_NEG;
                    ftpSessionPtr->ftpSslNegFlagsDataConn  = TM_FTP_SSL_NOT_NEG;
                    ftpSessionPtr->ftpSslEnabledCtrlConn   = TM_8BIT_NO;
                    ftpSessionPtr->ftpSslEnabledDataConn   = TM_8BIT_NO;
                    ftpSessionPtr->ftpSslNegInProgress     = TM_8BIT_NO;

                    ftpSessionPtr->ftpCommandCode     = TM_CMD_IDLE;
                    ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;
                    tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);

                    result = TM_ENOERROR;
                }
            }
        }
        if ((ftpSessionPtr->ftpSslEnabledCtrlConn == TM_8BIT_YES) &&
            (ftpSessionPtr->ftpSslNegFlagsCtrlConn == TM_FTP_SSL_NOT_NEG))
        {
            if ( ftpSessionPtr->ftpCBFlags[TM_FTP_CTRL_SOCKET]
                & TM_CB_SSL_ESTABLISHED )
            {
/* SSL established on the control connection */
                ftpSessionPtr->ftpCBFlags[TM_FTP_CTRL_SOCKET] &=
                                                       ~TM_CB_SSL_ESTABLISHED;
                ftpSessionPtr->ftpSslNegFlagsCtrlConn = TM_FTP_SSL_NEG_SUCCESS;

                ftpSessionPtr->ftpSslNegInProgress = TM_8BIT_NO;

                if (ftpSessionPtr->ftpNextCommandCode != TM_CMD_IDLE)
                {
/* Do the next command */
                    nextCommandCode = ftpSessionPtr->ftpNextCommandCode;
                    tfFtpSetMatrixPhase(ftpSessionPtr, TM_FTPP_INITIAL);
                    tfFtpSetCommandCode(ftpSessionPtr, nextCommandCode,
                                                        TM_CMD_IDLE);
                    ftpSessionPtr->ftpCmdRequest = nextCommandCode;
                    ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;

                    result = TM_EWOULDBLOCK;
                }
                else
                {
                    result = TM_ENOERROR;
                }
            }
            else if ( ftpSessionPtr->ftpCBFlags[TM_FTP_CTRL_SOCKET]
                     & TM_CB_SSL_HANDSHK_FAILURE )
            {
/* SSL establishment failed on the control connection */
                ftpSessionPtr->ftpCBFlags[TM_FTP_CTRL_SOCKET] &=
                                                   ~TM_CB_SSL_HANDSHK_FAILURE;
                ftpSessionPtr->ftpSslNegFlagsCtrlConn = TM_FTP_SSL_NEG_FAILURE;
                ftpSessionPtr->ftpSslNegInProgress    = TM_8BIT_NO;
                ftpSessionPtr->ftpSslEnabledCtrlConn  = TM_8BIT_NO;

/* Close the control connection. */
                tfFtpCloseCtrlSocket(ftpSessionPtr);
                ftpSessionPtr->ftpCommandCode      = TM_CMD_IDLE;
                ftpSessionPtr->ftpNextCommandCode  = TM_CMD_IDLE;
                ftpSessionPtr->ftpInterCommandCode = TM_CMD_IDLE;
                tfFtpSetMatrixPhase(ftpSessionPtr, TM_FTPP_INITIAL);

                result = TM_ENOPROTOOPT;
            }
        }
    }
    if ( ( (ftpSessionPtr->ftpFlags & TM_FTP_F_DATA_PASV)
        || (ftpSessionPtr->ftpFlags & TM_FTPP_DATA_CONN) ) )
    {
        if ( ftpSessionPtr->ftpCBFlags[TM_FTP_DATA_SOCKET]
                 & TM_CB_SSL_HANDSHK_PROCESS )
        {
            ftpSessionPtr->ftpCBFlags[TM_FTP_DATA_SOCKET] &=
                                               ~TM_CB_SSL_HANDSHK_PROCESS;
            tfSslUserProcessHandshake(
                        ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET]);
        }
        if (   (ftpSessionPtr->ftpSslEnabledDataConn == TM_8BIT_YES)
            && (ftpSessionPtr->ftpSslNegFlagsDataConn == TM_FTP_SSL_NOT_NEG) )
        {
            if (ftpSessionPtr->ftpCBFlags[TM_FTP_DATA_SOCKET]
                & TM_CB_SSL_ESTABLISHED)
            {
/* SSL has been established on the data connection. */
                ftpSessionPtr->ftpSslNegFlagsDataConn = TM_FTP_SSL_NEG_SUCCESS;
/* Unset the flag. */
                ftpSessionPtr->ftpCBFlags[TM_FTP_DATA_SOCKET]
                    &= ~(TM_CB_SSL_ESTABLISHED);
                result = TM_EWOULDBLOCK;
            }
            else if (ftpSessionPtr->ftpCBFlags[TM_FTP_DATA_SOCKET]
                     & TM_CB_SSL_HANDSHK_FAILURE)
            {
/*
 * SSL was not established on the data connection.
 * Set the SSL flags, close the socket, and return an error.
 */
                ftpSessionPtr->ftpSslNegFlagsDataConn = TM_FTP_SSL_NEG_FAILURE;
                ftpSessionPtr->ftpSslNegInProgress    = TM_8BIT_NO;

/* Close the data and listening sockets. */
                tfFtpCloseData(ftpSessionPtr);

/* Transition the state machine back to its initial phase. */
                ftpSessionPtr->ftpCommandCode      = TM_CMD_IDLE;
                ftpSessionPtr->ftpNextCommandCode  = TM_CMD_IDLE;
                ftpSessionPtr->ftpInterCommandCode = TM_CMD_IDLE;

                tfFtpSetMatrixPhase(ftpSessionPtr, TM_FTPP_INITIAL);

                result = TM_ENOPROTOOPT;
            }
        }
    }
#endif /* TM_USE_FTP_SSL */

    return (result);
}


/*
 * tfFtpMatrixCheckBlocking description:
 *   Based on the current event, blocks until event (either send or receive)
 *   occurs on necessary socket (control, data, etc).  Only called in blocking
 *   mode.  Timeouts after 1 millisecond.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_ENOERROR (0)     No error (Success.)
 */
static void tfFtpMatrixCheckBlocking (ttFtpSessionPtr ftpSessionPtr)
{
    struct timeval timeOut;
    tt16Bit        matrixEvent;
    int            numberSockets;
    int            numberReadSockets;
    int            numberWriteSockets;
    fd_set         fdsetRead;
    fd_set         fdsetWrite;
#ifdef TM_USE_FTP_SSL
    fd_set         fdsetOther;
#endif /* TM_USE_FTP_SSL */
    int            dataSocket;
    int            ctrlSocket;
    int            listenSocket;


/* Set timeout value to 1 millisecond. */
    timeOut.tv_sec  = 1;
    timeOut.tv_usec = 0;

/* Initialize the read and write file descriptor sets */
    FD_ZERO ( &fdsetRead );
    FD_ZERO ( &fdsetWrite );
#ifdef TM_USE_FTP_SSL
/* For SSL handshake process */
    FD_ZERO ( &fdsetOther );
#endif /* TM_USE_FTP_SSL */

    numberReadSockets  = 0;
    numberWriteSockets = 0;

/* Retrieve the type of matrix event currently being processed */
    matrixEvent = tlFtpMatrixEventTable[ftpSessionPtr->ftpMatrixNo]
                    [ftpSessionPtr->ftpMtrxPhase];


    ctrlSocket = ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET];
    dataSocket = ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET];
    listenSocket = ftpSessionPtr->ftpSocketDesc[TM_FTP_LISTEN_SOCKET];
/*
 * Set up the file descriptor bits so that we select on the proper events.  For
 * instance, if the event is control-receive, enable bits in fdset such that
 * we select on data received on the control socket.  Also compute the maximum
 * socket number that we wish to select on.
 */
    if (matrixEvent & TM_FTPS_CTRLRECV)
    {
        if (ctrlSocket != TM_SOCKET_ERROR)
        {
            FD_SET (ctrlSocket, (&fdsetRead));
#ifdef TM_USE_FTP_SSL
/* For SSL handshake process */
            FD_SET (ctrlSocket, (&fdsetOther));
#endif /* TM_USE_FTP_SSL */
            numberReadSockets = ctrlSocket + 1;
        }
    }

    if (matrixEvent & TM_FTPS_DATARECV)
    {
        if (dataSocket != TM_SOCKET_ERROR)
        {
            FD_SET(dataSocket, &fdsetRead);
#ifdef TM_USE_FTP_SSL
/* For handshake process */
            FD_SET (dataSocket, (&fdsetOther));
#endif /* TM_USE_FTP_SSL */

            if (numberReadSockets <= dataSocket)
            {
                numberReadSockets = dataSocket + 1;
            }
        }
    }

/*
 * JNS/1-27-00 BUGFIX 471
 *
 * While waiting for a incoming data connection, we were not blocking via
 * select so we would just spin in tfFtpMatrix until a timeout occured.  This
 * would effectively lock the receive task from running.  This bug would occur
 * on any non-preemptive OS, or on a pre-emptive OS with the FTP task at a
 * higher priority than the receive task.
 *
 */
    if (matrixEvent & TM_FTPS_DATACONN)
    {
        if (listenSocket != TM_SOCKET_ERROR)
        {
            FD_SET(listenSocket, &fdsetRead);
            if (numberReadSockets <= listenSocket)
            {
                numberReadSockets = listenSocket + 1;
            }
        }
    }

    if (matrixEvent & (TM_FTPS_SENDREQ | TM_FTPS_SENDCOMP))
    {
        if (dataSocket != TM_SOCKET_ERROR)
        {
#ifdef TM_USE_FTP_SSL
/* For SSL handshake process */
            FD_SET (dataSocket, (&fdsetOther));
#endif /* TM_USE_FTP_SSL */
            FD_SET(dataSocket, &fdsetWrite);
            numberWriteSockets = dataSocket + 1;
        }
    }

/* Compute number maximum socket number to block on */
    if (numberReadSockets > numberWriteSockets)
    {
        numberSockets = numberReadSockets;
    }
    else
    {
        numberSockets = numberWriteSockets;
    }

/* Block until some event (specified above) occurs */
    if ( numberSockets != 0 )
    {
#ifdef TM_USE_FTP_SSL
        (void)select( numberSockets,
                      &fdsetRead,
                      &fdsetWrite,
                      &fdsetOther,
                      &timeOut );
#else /* !TM_USE_FTP_SSL */
        (void)select( numberSockets,
                      &fdsetRead,
                      &fdsetWrite,
                      (fd_set *) 0,
                      &timeOut );
#endif /* !TM_USE_FTP_SSL */
    }
}


/*
 * tfFtpMatrixCmdReq description:
 *   If the current state requires a state function call, do so by calling
 *   off to tfFtpMatrixExecute.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_ENOERROR (0)     No error (Success.)
 */
static int tfFtpMatrixCmdReq (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    result = TM_EWOULDBLOCK;

/* If the current matrix event is a command and... */
    if (tlFtpMatrixEventTable[ftpSessionPtr->ftpMatrixNo]
            [ftpSessionPtr->ftpMtrxPhase] & TM_FTPS_CMDREQ)
    {

/* ...the current command is not IDLE */
        if (ftpSessionPtr->ftpCmdRequest != TM_CMD_IDLE)
        {

/* If the current event is not _E03 set matrix number for this command */
            if (ftpSessionPtr->ftpMtrxEvent != TM_FTPE_ABORT)
            {
                ftpSessionPtr->ftpMatrixNo =
                    tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctMatrixNo;
            }

/* Execute the current FTP state function */
            result = tfFtpMatrixExecute(ftpSessionPtr);

/* Next command is idle */
            ftpSessionPtr->ftpCmdRequest = TM_CMD_IDLE;
        }
    }

    return (result);
}


/*
 * tfFtpMatrixCtrlConn description:
 *   If the control socket has its callback flags (signal) set to connection
 *   complete, set the event to TM_FTPE_CTRL_CONN and execute the state
 *   functions.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_ENOERROR (0)     No error (Success.)
 */
static int tfFtpMatrixCtrlConn (ttFtpSessionPtr ftpSessionPtr)
{
    int result;
/*  int socketDesc; */
    int signal;

    result = TM_EWOULDBLOCK;

/* If the current matrix phase applies to the control connection... */
    if (tlFtpMatrixEventTable[ftpSessionPtr->ftpMatrixNo]
            [ftpSessionPtr->ftpMtrxPhase] & TM_FTPS_CTRLCONN)
    {

/* Get the callback flags (signal) associated with the control connection */
/* socketDesc = ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET]; */
        signal = ftpSessionPtr->ftpCBFlags[TM_FTP_CTRL_SOCKET];
/* signal = tfFtpGetSignal(ftpSessionPtr, socketDesc); */

        if (signal & TM_CB_CONNECT_COMPLT)
        {

/*
 * If the signal from above is connect complete, set state event to
 * TM_FTPE_CTRL_CONN, execute the state function and clear the connect complete
 * signal from the control socket
 */
            ftpSessionPtr->ftpMtrxEvent = TM_FTPE_CTRL_CONN;
            result = tfFtpMatrixExecute(ftpSessionPtr);

            tm_16bit_clr_bit(ftpSessionPtr->ftpCBFlags[TM_FTP_CTRL_SOCKET],
                             TM_CB_CONNECT_COMPLT);

        }
    }

    return (result);
}


/*
 * tfFtpMatrixDataConn description:
 *   If the listening socket has its callback flags (singal) set to accept,
 *   set the event to TM_FTPE_DATA_CONN and execute the state functions.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_ENOERROR (0)     No error (Success.)
 */
static int tfFtpMatrixDataConn (ttFtpSessionPtr ftpSessionPtr)
{
    int result;
    int socketDesc;
    int signalData;

    result = TM_EWOULDBLOCK;

/* If the current matrix phase applies to the data connection... */
    if (tlFtpMatrixEventTable[ftpSessionPtr->ftpMatrixNo]
            [ftpSessionPtr->ftpMtrxPhase] & TM_FTPS_DATACONN)
    {

/* Get the callback flags (signal) associated with the listening socket */
        socketDesc = ftpSessionPtr->ftpSocketDesc[TM_FTP_LISTEN_SOCKET];
        signalData = tfFtpGetSignal(ftpSessionPtr, socketDesc);

        if (!(signalData & TM_CB_ACCEPT))
        {
/* Get the callback flags (signal) associated with the data socket */
            socketDesc = ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET];
            signalData = tfFtpGetSignal(ftpSessionPtr, socketDesc);
        }

/*
 * If the signal from above is accept, set state event to TM_FTPE_DATA_CONN,
 * execute the state function and clear the accept signal from the listen
 * socket.
 */
        if (signalData & (TM_CB_ACCEPT|TM_CB_CONNECT_COMPLT))
        {
#ifdef TM_USE_FTP_SSL
            if( (ftpSessionPtr->ftpSslEnabledDataConn)
             && (ftpSessionPtr->ftpSslNegFlagsDataConn == TM_FTP_SSL_NOT_NEG) )
            {
/*
 * SSL is enabled on the data connection but negotiations have not yet
 * taken place. Start SSL negotiation.
 */
                if (ftpSessionPtr->ftpSslNegInProgress == TM_8BIT_NO)
                {
/* Change the event to data so that SSL negotiations can begin. */
                    ftpSessionPtr->ftpMtrxEvent = TM_FTPE_DATA_CONN;
                    result = tfFtpMatrixExecute (ftpSessionPtr);
                }

            }
            else
#endif /* TM_USE_FTP_SSL */
            {
/*
 * SSL negotiation is now complete or SSL is not enabled.
 * Change the event to data so that data can be received or sent.
 */
                ftpSessionPtr->ftpMtrxEvent = TM_FTPE_DATA_CONN;
                result = tfFtpMatrixExecute (ftpSessionPtr);

                tfFtpClearSignal ( ftpSessionPtr, socketDesc,
                                   (TM_CB_ACCEPT|TM_CB_CONNECT_COMPLT) );
            }

        }
    }

    return (result);
}


/*
 * tfFtpMatrixCtrlRecv description:
 *   If there is any waiting data, or ????, execute either TM_FTPE_CTRL_IRECV
 *   (for IDLE) or TM_FTPE_CTRL_RECV (for any other commands)
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_ENOERROR (0)     No error (Success.)
 */
static int tfFtpMatrixCtrlRecv (ttFtpSessionPtr ftpSessionPtr)
{
    int result;
    int socketDesc;
    int waitingBytes;

    result = TM_EWOULDBLOCK;

/* If the current matrix phase applies to receiving on the control
 * connection...
 */
    if (tlFtpMatrixEventTable[ftpSessionPtr->ftpMatrixNo]
            [ftpSessionPtr->ftpMtrxPhase] & TM_FTPS_CTRLRECV)
    {
        socketDesc = ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET];
        waitingBytes = tfGetWaitingBytes(socketDesc);

/* If there is any waiting data, or ????, execute either TM_FTPE_CTRL_IRECV
 * (for IDLE) or TM_FTPE_DATA_RECV (for any other commands).
 */
        if ((waitingBytes > 0) || (ftpSessionPtr->ftpTelBufReadIndex > 0))
        {
            if (ftpSessionPtr->ftpCommandCode != TM_CMD_IDLE)
            {
                ftpSessionPtr->ftpMtrxEvent = TM_FTPE_CTRL_RECV;
            }
            else
            {
                ftpSessionPtr->ftpMtrxEvent = TM_FTPE_CTRL_IRECV;
            }

            result = tfFtpMatrixExecute(ftpSessionPtr);
        }
    }

    return (result);
}


/*
 * tfFtpMatrixDataRecv description:
 *   If a receive callback has occured on the data connection, execute
 *   matrix event TM_FTPE_DATA_RECV and set the callback flags if necessary.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_ENOERROR (0)     No error (Success.)
 */
static int tfFtpMatrixDataRecv (ttFtpSessionPtr ftpSessionPtr)
{
    int result;
    int socketDesc;
    int waitingBytes;
    int signal;

    result = TM_EWOULDBLOCK;

/* If the current matrix phase applies to receiving on the data connection... */
    if (tlFtpMatrixEventTable[ftpSessionPtr->ftpMatrixNo]
            [ftpSessionPtr->ftpMtrxPhase] & TM_FTPS_DATARECV)
    {
        socketDesc = ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET];
        signal = tfFtpGetSignal (ftpSessionPtr, socketDesc);

        if (signal & TM_CB_RECV)
        {

/* Execute matrix event TM_FTPE_DATA_RECV and clear receive signal from this
 * socket
 */
            ftpSessionPtr->ftpMtrxEvent = TM_FTPE_DATA_RECV;
            result = tfFtpMatrixExecute(ftpSessionPtr);
            tfFtpClearSignal(ftpSessionPtr, socketDesc, TM_CB_RECV);

/*
 * If there is more available data on the socket, set the receive callback flag
 * which will cause us to receive more data on the next pass through the loop.
 */
            waitingBytes = tfGetWaitingBytes (socketDesc);
            if (waitingBytes > 0)
            {
                tfFtpSetSignal(ftpSessionPtr, socketDesc, TM_CB_RECV);
            }
        }
    }

    return (result);
}


/*
 * tfFtpMatrixSendReq description:
 *   If the current matrix event applies to sending requests, and this session's
 *   send request flag is set, execute state event TM_FTPE_SEND.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_ENOERROR (0)     No error (Success.)
 */
static int tfFtpMatrixSendReq (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    result = TM_EWOULDBLOCK;

/*
 * If the current matrix event applies to sending requests, and this session's
 * send request flag is set, execute state event TM_FTPE_SEND_REQ.
 */
    if (tlFtpMatrixEventTable[ftpSessionPtr->ftpMatrixNo]
            [ftpSessionPtr->ftpMtrxPhase] & TM_FTPS_SENDREQ)
    {
        if (ftpSessionPtr->ftpSendRequest == TM_SEND_REQ_ON)
        {
            ftpSessionPtr->ftpMtrxEvent = TM_FTPE_SEND_REQ;
            result = tfFtpMatrixExecute(ftpSessionPtr);
        }
    }

    return (result);
}


/*
 * tfFtpMatrixSendComp description:
 *
 * If the current event applies to send completes, and the data socket has
 * its signal set to send completes, execute matrix event TM_FTPE_WRITE_RDY.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_ENOERROR (0)     No error (Success.)
 */
static int tfFtpMatrixSendComp (ttFtpSessionPtr ftpSessionPtr)
{
    int result;
    int socketDesc;
    int signal;

    result = TM_EWOULDBLOCK;

/*
 * If the current event applies to send completes, and the data socket has
 * its signal set to send completes, execute matrix event TM_FTPE_WRITE_RDY.
 */
    if (tlFtpMatrixEventTable[ftpSessionPtr->ftpMatrixNo]
            [ftpSessionPtr->ftpMtrxPhase] & TM_FTPS_SENDCOMP)
    {
        socketDesc = ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET];
        signal = tfFtpGetSignal (ftpSessionPtr, socketDesc);

        if (signal & TM_CB_WRITE_READY)
        {
            tfFtpClearSignal (ftpSessionPtr, socketDesc, TM_CB_WRITE_READY);
            ftpSessionPtr->ftpMtrxEvent = TM_FTPE_WRITE_RDY;
            result = tfFtpMatrixExecute (ftpSessionPtr);
        }
    }

    return (result);
}


/*
 * tfFtpMatrixCtrlErro description:
 *
 * If the current event applies to a control error, and the control connection
 * has its signal set to remote close or reset, execute event TM_FTPE_CTRL_ERROR
 * and clear these events.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_ENOERROR (0)     No error (Success.)
 */
static int tfFtpMatrixCtrlErro (ttFtpSessionPtr ftpSessionPtr)
{
    int result;
    int socketDesc;
    int signal;
    int waitingBytes;

    result = TM_EWOULDBLOCK;

/*
 * If the current event applies to a control error, and the control connection
 * has its signal set to remote close or reset, execute event TM_FTPE_CTRL_ERROR
 * and clear these events.
 */
    if (tlFtpMatrixEventTable[ftpSessionPtr->ftpMatrixNo]
            [ftpSessionPtr->ftpMtrxPhase] & TM_FTPS_CTRLERRO)
    {
        socketDesc = ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET];
        signal = tfFtpGetSignal (ftpSessionPtr, socketDesc);

/* If the server reset or closed the control connection, make sure that
   we've received all of the data sent by the server. */
        waitingBytes = tfGetWaitingBytes(socketDesc);
        if ((waitingBytes > 0) || (ftpSessionPtr->ftpTelBufReadIndex > 0))
        {
            goto ftpMatrixCtrlErrorReturn;
        }

        if (   signal
             & (TM_CB_REMOTE_CLOSE | TM_CB_RESET | TM_CB_SOCKET_ERROR ) )
        {
            tfFtpClearSignal(
                ftpSessionPtr,
                socketDesc,
                TM_CB_REMOTE_CLOSE | TM_CB_RESET | TM_CB_SOCKET_ERROR );

            ftpSessionPtr->ftpMtrxEvent = TM_FTPE_CTRL_ERROR;
            result = tfFtpMatrixExecute (ftpSessionPtr);
        }
    }

ftpMatrixCtrlErrorReturn:
    return (result);
}


/*
 * tfFtpMatrixDataErro description:
 *   If the data socket's receive signal is set, execute state event
 *   TM_FTPE_DATA_RECV, otherwise if the remote close or reset signal is set,
 *   execute state event TM_FTPE_DATA_ERROR.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_ENOERROR (0)     No error (Success.)
 */
static int tfFtpMatrixDataErro (ttFtpSessionPtr ftpSessionPtr)
{
    int result;
    int socketDesc;
    int waitingBytes;
    int signal;

    result = TM_EWOULDBLOCK;

/* If the current event applies to a data error... */
    if (tlFtpMatrixEventTable[ftpSessionPtr->ftpMatrixNo]
            [ftpSessionPtr->ftpMtrxPhase] & TM_FTPS_DATAERRO)
    {
        socketDesc = ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET];
        signal = tfFtpGetSignal (ftpSessionPtr, socketDesc);

        if (signal & TM_CB_RECV)
        {

/* If the data socket has its receive signal set, execute event
 * TM_FTPE_DATA_RECV
 */
            ftpSessionPtr->ftpMtrxEvent = TM_FTPE_DATA_RECV;
            result = tfFtpMatrixExecute (ftpSessionPtr);

/* Clear the receive signal from this socket */
            tfFtpClearSignal (ftpSessionPtr, socketDesc, TM_CB_RECV);

            waitingBytes = tfGetWaitingBytes (socketDesc);
            if ( waitingBytes > 0 )
            {
                tfFtpSetSignal (ftpSessionPtr, socketDesc, TM_CB_RECV);
            }
        }
/* If the data socket does not have the receive signal set... */
        else
        {

            if (   signal
                 & (TM_CB_REMOTE_CLOSE | TM_CB_RESET | TM_CB_SOCKET_ERROR
#ifdef TM_USE_FTP_SSL
                    | TM_CB_SSL_RECV_CLOSE | TM_CB_SSL_SEND_CLOSE
#endif /* TM_USE_FTP_SSL */
                   ) )
            {
#ifdef TM_USE_FTP_SSL
/* Check for an SSL record that has not been processed yet */
                waitingBytes = tfGetWaitingBytes(socketDesc);
                if ( waitingBytes > 0 )
/* receive the just processed SSL record. */
                {
                    ftpSessionPtr->ftpMtrxEvent = TM_FTPE_DATA_RECV;
                }
                else
#endif /* TM_USE_FTP_SSL */
                {

/* If the remote close or reset signals were set, clear them and execute state
 * function TM_FTPE_DATA_ERROR
 */
                    tfFtpClearSignal(
                        ftpSessionPtr,
                        socketDesc,
                        TM_CB_REMOTE_CLOSE | TM_CB_RESET | TM_CB_SOCKET_ERROR
#ifdef TM_USE_FTP_SSL
                        | TM_CB_SSL_RECV_CLOSE | TM_CB_SSL_SEND_CLOSE
#endif /* TM_USE_FTP_SSL */
                        );
                    ftpSessionPtr->ftpMtrxEvent = TM_FTPE_DATA_ERROR;
                }
                result = tfFtpMatrixExecute (ftpSessionPtr);
            }
        }
    }

    return (result);
}


/*
 * tfFtpMatrixTimeOut description:
 *
 * If the current event is a timeout and the timer is current running,
 * execute state event TM_FTPE_TIMEOUT..
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_ENOERROR (0)     No error (Success.)
 */
static int tfFtpMatrixTimeOut (ttFtpSessionPtr ftpSessionPtr)
{
    int     result;
    tt16Bit timerCounter;

    result = TM_EWOULDBLOCK;

/*
 * If the current event is a timeout and the timer is current running,
 * execute state event TM_FTPE_TIMEOUT.
 */
    if (tlFtpMatrixEventTable[ftpSessionPtr->ftpMatrixNo]
            [ftpSessionPtr->ftpMtrxPhase] & TM_FTPS_TIMEOUT)
    {
        timerCounter = tfFtpGetTimerCounter (ftpSessionPtr);
        if (timerCounter == 0)
        {
            ftpSessionPtr->ftpMtrxEvent = TM_FTPE_TIMEOUT;
            result = tfFtpMatrixExecute (ftpSessionPtr);
        }
    }

    return (result);
}


/*
 * tfFtpMatrixExecute description:
 *   Execute the function associated with the current matrix number, event
 *   and phase.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_ENOERROR (0)     No error (Success.)
 */
static int tfFtpMatrixExecute (ttFtpSessionPtr ftpSessionPtr)
{
    int          result;
    ttFtpFuncPtr stateFuncPtr;

#ifdef TM_DEBUG_LOGGING
    tm_debug_log3("tfFtpMatrixExecute: %s, %s, %s\r\n",
        tfFtpStrMatrixNo(ftpSessionPtr->ftpMatrixNo),
        tfFtpStrMatrixEvent(ftpSessionPtr->ftpMtrxEvent),
        tfFtpStrMatrixPhase(ftpSessionPtr->ftpMtrxPhase));
#endif /* TM_DEBUG_LOGGING */
    stateFuncPtr = tlFtpMatrixTable[ftpSessionPtr->ftpMatrixNo]
        [ftpSessionPtr->ftpMtrxEvent][ftpSessionPtr->ftpMtrxPhase];

    if (stateFuncPtr != 0)
    {
        result = (*(stateFuncPtr))(ftpSessionPtr);
    }
    else
    {
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpGetReplyCode description:
 *   Returns reply code in the form of an integer error code.a
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  8000 + reply code
 *  TM_ENOERROR (0)     No error (Success.)
 */
static int tfFtpGetReplyCode (ttFtpSessionPtr ftpSessionPtr)
{
    int result;

    if (tm_is_reply_2xx(ftpSessionPtr->ftpReplyCode))
    {
        result = TM_ENOERROR;
    }
    else
    {
        if (ftpSessionPtr->ftpReplyCode != -1)
        {
            result = TM_REPLY_BASE + ftpSessionPtr->ftpReplyCode;
        }
        else
        {
            result = TM_FTP_SYNTAXARG;
        }
        tfFtpCloseData(ftpSessionPtr);
    }

    return (result);
}


/*
 * tfFtpTimerCBFunction description:
 *   FTP Timer callback function
 *
 * Parameters:
 *  Value               Meaning
 *  argPtr              Pointer to the corresponding FTP session structure.
 *
 * Returns:
 *  none
 */
static void tfFtpTimerCBFunction (ttVoidPtr       timerBlockPtr,
                                  ttGenericUnion  userParm1,
                                  ttGenericUnion  userParm2)
{
    ttFtpSessionPtr ftpSessionPtr;
    ttTimerPtr      timerPtr;

/* Avoid compiler warning about unused parameters */
    TM_UNREF_IN_ARG(userParm2);

    timerPtr = (ttTimerPtr)timerBlockPtr;
    if (tm_timer_not_reinit(timerPtr))
    {
        ftpSessionPtr = (ttFtpSessionPtr)userParm1.genVoidParmPtr;
        if (ftpSessionPtr->ftpTimerFlag == TM_FTP_TIMER_ON)
        {
            if (ftpSessionPtr->ftpTimerCounter > 0)
            {
                ftpSessionPtr->ftpTimerCounter --;
            }
        }
    }
}


/*
 * tfFtpStartTimer description:
 *   Starts FTP timer.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  timerSec            Timer value (s)
 *
 * Returns:
 *  none
 */
static void tfFtpStartTimer (ttFtpSessionPtr ftpSessionPtr, tt16Bit timerSec)
{
    if ( ftpSessionPtr->ftpTimerBlockPtr != ( ttTimerPtr )0 )
    {
        ftpSessionPtr->ftpTimerFlag = TM_FTP_TIMER_ON;

        ftpSessionPtr->ftpTimerCounter = timerSec;
    }
}


/*
 * tfFtpCancelTimer description:
 *   Disables FTP timer.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  none
 */
static void tfFtpCancelTimer (ttFtpSessionPtr ftpSessionPtr)
{
    if (ftpSessionPtr->ftpTimerBlockPtr != (ttTimerPtr) 0)
    {
        ftpSessionPtr->ftpTimerFlag = TM_FTP_TIMER_OFF;
    }
}


/*
 * tfFtpGetTimerCounter description:
 *   Returns the value of the timer counter;  if not valid, returns 0xffff.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  timer value
 */
static tt16Bit tfFtpGetTimerCounter (ttFtpSessionPtr ftpSessionPtr)
{
    tt16Bit timerCounter;

    timerCounter = 0xffff;

    if ( ftpSessionPtr->ftpTimerBlockPtr != (ttTimerPtr) 0)
    {
        if ( ftpSessionPtr->ftpTimerFlag == TM_FTP_TIMER_ON )
        {
            timerCounter = ftpSessionPtr->ftpTimerCounter;
        }
    }

    return (timerCounter);
}


/*
 * tfFtpCheckFile description:
 *   Determines if a certain file already exists or can be created.
 *
 * Parameters:
 *  Value               Meaning
 *  fileNamePtr         String specifying the filename in question
 *
 * Returns:
 *  Value               Meaning
 *  TM_FILCHK_EXIST     File exists
 *  TM_FILCHK_NEXISTFIL File does not exist
 *  TM_FILCHK_NEXISTDIR Directory does not exist
 *  TM_FILCHK_NEXISTDEV Device does not exists
 */
static int tfFtpCheckFile (void TM_FAR *userDataPtr,
                    ttCharPtr    fileNamePtr)
{
    int          i;
    int          result;
    int          pathSize;
    void TM_FAR *filePointer;
    void TM_FAR *dirDataPtr;
    char         pathName[256];

    pathSize = (int)tm_strlen (fileNamePtr);
    tm_strcpy (&pathName[0], fileNamePtr);

/*
 * If the last character is a ':', then the file name is simply a 'device'
 * (at least in the DOS world) and therefore can't be opened on its own, so
 * return TM_FILCHK_NEXISTDEV.  Otherwise, continue on...
 */
    if (fileNamePtr[pathSize - 1] != ':')
    {

/* Iterate backwards through the filename until we encounter a slash
 * (either direction), moving the end of the string back once each time,
 * giving us the directory name at the end.
*/
        for ( i = pathSize; i > 0; i-- )
        {
            if ((fileNamePtr[i - 1] != '/') &&
                (fileNamePtr[i - 1] != '\\'))
            {
                pathName[i - 1] = (char) 0x00;
            }
            else
            {
                pathName[i - 1] = (char) 0x00;
                break;
            }
        }
        result = TM_FILCHK_EXIST; /* assume file exists */
        if (pathName[0] != (char) 0x00)
        {

/* Attempt to open the directory; if this fails return TM_FILCHK_NEXISTDIR. */
            dirDataPtr = tfFSOpenDir(userDataPtr,
                                     (char TM_FAR *) &pathName[0],
                                     TM_DIR_LONG);

            if ( dirDataPtr != ( void TM_FAR * )0 )
            {
                tfFSCloseDir ( userDataPtr, dirDataPtr );
            }
            else
            {
                result = TM_FILCHK_NEXISTDIR;
            }
        }

/*
 * Directory exists, so attempt to open the file.  If successful, return
 * TM_FILCHK_EXIST; otherwise return TM_FILCHK_NEXISTFIL.
 */
        if (result != TM_FILCHK_NEXISTDIR)
        {

            filePointer = tfFSOpenFile (userDataPtr,
                                        fileNamePtr,
                                        TM_FS_READ,
                                        TM_TYPE_BINARY,
                                        TM_STRU_STREAM );

            if (filePointer != (void TM_FAR *) 0)
            {
                (void)tfFSCloseFile (userDataPtr, filePointer);
                result = TM_FILCHK_EXIST;
            }
            else
            {
                result = TM_FILCHK_NEXISTFIL;
            }
        }
    }
    else
    {
        result = TM_FILCHK_NEXISTDEV;
    }

    return (result);
}

/*
 * tfFtpCloseDataFile description:
 *   Closes the FTP data file.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  none
 */
static void tfFtpCloseDataFile (ttFtpSessionPtr ftpSessionPtr)
{
    if (ftpSessionPtr->ftpFilePointer != (void TM_FAR *) 0)
    {
        (void)tfFSCloseFile(ftpSessionPtr->ftpUserDataPtr,
                            ftpSessionPtr->ftpFilePointer);

        ftpSessionPtr->ftpFilePointer = (void TM_FAR *) 0;
    }
}

/*
 * tfFtpStString description:
 *   Copies string 'stringPtr' to 'setBufPtr' and returns a pointer to the
 *   very end of the new string in setBufPtr.
 *
 * Parameters:
 *  Value               Meaning
 *  setBufPtr           destination pointer
 *  stringPtr           source string pointer
 *
 * Returns:
 *  Value               Meaning
 *                      pointer to the end of setBufPtr
 */
static ttCharPtr tfFtpSetString (ttCharPtr setBufPtr, ttCharPtr stringPtr)
{
    int i;
    int size;

    size = (int)tm_strlen (stringPtr);

    for ( i = 0; i < size; i ++ )
    {
        setBufPtr[i] = stringPtr[i];
    }

    setBufPtr[i] = (char) 0x00;

    return (setBufPtr + i);
}

/*
 * tf4FtpSetAddress description:
 *   Converts an IP address and port into ASCII and places them in setBufPtr.
 *
 * Parameters:
 *  Value               Meaning
 *  buffPtr             string buffer to place converted IP address/port into
 *  addressPtr          pointer to address structure
 *
 * Returns:
 *  Value               Meaning
 *                      pointer to *new* end of string data
 */
#ifdef TM_USE_IPV4
static ttCharPtr tf4FtpSetAddress (ttCharPtr buffPtr,
                                  struct sockaddr_in TM_FAR * addressPtr)
{
    union tuFtpIpWork   ipWork;
    union tuFtpPortWork portWork;
    ttCharPtr           setBufPtr;

    ipWork.ftpIpAddress = addressPtr->sin_addr.s_addr;
    portWork.ftpPortNo  = addressPtr->sin_port;

    setBufPtr = buffPtr;

    setBufPtr = tfUlongDecimalToString(setBufPtr,
                                      (tt32Bit) ipWork.ftpIpAddr[0]);
    setBufPtr = tfFtpSetString (setBufPtr, "," );
    setBufPtr = tfUlongDecimalToString (setBufPtr,
                                        (tt32Bit) ipWork.ftpIpAddr[1]);
    setBufPtr = tfFtpSetString (setBufPtr, "," );
    setBufPtr = tfUlongDecimalToString(setBufPtr,
                                       (tt32Bit) ipWork.ftpIpAddr[2]);
    setBufPtr = tfFtpSetString (setBufPtr, "," );
    setBufPtr = tfUlongDecimalToString(setBufPtr,
                                       (tt32Bit)ipWork.ftpIpAddr[3]);
    setBufPtr = tfFtpSetString (setBufPtr, "," );
    setBufPtr = tfUlongDecimalToString(setBufPtr,
                                       (tt32Bit) portWork.ftpPort[0]);
    setBufPtr = tfFtpSetString (setBufPtr, "," );
    setBufPtr = tfUlongDecimalToString(setBufPtr,
                                       (tt32Bit)portWork.ftpPort[1]);

    return (setBufPtr);
}
#endif /* TM_USE_IPV4 */

/*
 * tfFtpCommonDataCommand description:
 *   Perform operation for data transfer command.
 *
 * Parameters:
 *  Value               Meaning
 *  command             Command need to be execute
 *  ftpSessionAPtr      Pointer to FTP session structure(main FTP server)
 *  ftpSessionBPtr      Pointer to FTP session structure, this parameter
 *                      is needed for passive mode:
 *                          0 for one FTP server model;
 *                          otherwise for two-server model.
 *  fromFileNamePtr     Pointer to source file name string
 *  toFileNamePtr       Pointer to destination file name string
 *  flags               Operation flags
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EINVAL           Invalid ftpSessionPtr or bad filename.
 *  TM_EACCES           Previous command has not finished
 *  TM_ENOTLOGIN        Command requires user to be loggedin, and user is not.
 *  TM_ENOTCONN         Command requires connection, and user is not connected
 *  TM_EOPNOTSUPP       Command not supported by the user
 *  TM_ENOERROR (0)     No error (Success.)
 */
static int tfFtpCommonDataCommand ( tt8Bit             command,
                                    ttFtpSessionPtr    ftpSessionAPtr,
                                    ttFtpSessionPtr    ftpSessionBPtr,
                                    char TM_FAR *      fromFileNamePtr,
                                    char TM_FAR *      toFileNamePtr,
                                    tt8Bit             flags)
{
    int    result;
    unsigned int tempUInt;
    tt8Bit pasvCommand;

    result = tfFtpCheckSession (ftpSessionAPtr,
                                TM_FTP_F_CTRL_CONN | TM_FTP_F_LOGGED_IN );

    if ( result == TM_ENOERROR )
    {
/* Source file name must be specified */
        tempUInt = tm_strlen(fromFileNamePtr);
        if( tempUInt == 0 )
        {
            result = TM_EINVAL;
        }
        else
        {
/* If the destination filename was not specified, use the source filename */
            tempUInt = tm_strlen(toFileNamePtr);
            if ( tempUInt == 0 )
            {
                toFileNamePtr = fromFileNamePtr;
            }
        }
    }

    if(    (result == TM_ENOERROR)
        && ( ftpSessionBPtr == (ttFtpSessionPtr)0 ) )
    {
/* One FTP server model: Clean up the 2nd FTP session pointer */
        ftpSessionAPtr->ftp2ndSessionPtr = (ttFtpSessionPtr)0;

        if( flags & TM_FTP_CMD_F_TO_SERVER_A )
        {
            ftpSessionAPtr->ftpLocalFileNamePtr  = fromFileNamePtr;
            ftpSessionAPtr->ftpRemoteFileNamePtr = toFileNamePtr;
/*
 * Verify that the local file exists: if it does, continue, otherwise return
 * EINVAL.
 */
            result = tfFtpCheckFile (ftpSessionAPtr->ftpUserDataPtr,
                                     ftpSessionAPtr->ftpLocalFileNamePtr);

            if (result == TM_FILCHK_EXIST)
            {
                result = TM_ENOERROR;
            }
            else
            {
                result = TM_EINVAL;
            }
        }

        if(    (result == TM_ENOERROR)
            && ( flags & TM_FTP_CMD_F_FROM_SERVER_A ) )
        {
            ftpSessionAPtr->ftpLocalFileNamePtr  = toFileNamePtr;
            ftpSessionAPtr->ftpRemoteFileNamePtr = fromFileNamePtr;
/*
 * Verify that the local file exists: if it does, continue, otherwise return
 * EINVAL.
 */
            result = tfFtpCheckFile (ftpSessionAPtr->ftpUserDataPtr,
                                     ftpSessionAPtr->ftpLocalFileNamePtr);

/* If the file exists or can be created (ie, the directory exists),
 * execute the command; otherwise return TM_EINVAL.
 */
            if ((result == TM_FILCHK_EXIST) || (result == TM_FILCHK_NEXISTFIL))
            {
                result = TM_ENOERROR;
            }
            else
            {
                result = TM_EINVAL;
            }
        }
    }

    if(    (result == TM_ENOERROR)
        && ( ftpSessionBPtr != (ttFtpSessionPtr)0 ) )
    {
/* Two FTP server model, check the 2nd FTP session: connected, logged in  */
        result = tfFtpCheckSession (ftpSessionBPtr,
                                TM_FTP_F_CTRL_CONN | TM_FTP_F_LOGGED_IN );

        if( result == TM_ENOERROR )
        {
/*
 * Check if the 2nd FTP seesion is in PASV mode: required to be in ACTV mode
 */
            if ( tm_16bit_bits_not_set( ftpSessionBPtr->ftpFlags,
                                        TM_FTP_F_DATA_PASV ) )
            {
/* Record the 2nd FTP session for future use */
                ftpSessionAPtr->ftp2ndSessionPtr = ftpSessionBPtr;
/* Clean up ftp2ndSessionPtr of ftpSessionBPtr */
                ftpSessionBPtr->ftp2ndSessionPtr = ftpSessionAPtr;
/* Clean up commands of ftpSessionBPtr to inhibit activity of session B */
                tfFtpSetCommandCode ( ftpSessionBPtr, TM_CMD_IDLE,
                                                      TM_CMD_IDLE );
                ftpSessionBPtr->ftpCmdRequest = TM_CMD_IDLE;
            }
            else
            {
                result = TM_EINVAL;
            }
        }

        if(    (result == TM_ENOERROR)
            && ( flags & TM_FTP_CMD_F_TO_SERVER_A ) )
        {
/* File is transfered from server B to server A */
            ftpSessionAPtr->ftpRemoteFileNamePtr = toFileNamePtr;
            ftpSessionBPtr->ftpRemoteFileNamePtr = fromFileNamePtr;
        }

        if(    (result == TM_ENOERROR)
            && ( flags & TM_FTP_CMD_F_FROM_SERVER_A ) )
        {
/* File is transfered from server A to server B */
            ftpSessionAPtr->ftpRemoteFileNamePtr = fromFileNamePtr;
            ftpSessionBPtr->ftpRemoteFileNamePtr = toFileNamePtr;
        }
    }

    if(    (result == TM_ENOERROR)
        && ( ftpSessionAPtr->ftpFlags & TM_FTP_F_DATA_PASV ) )
    {
#ifdef TM_USE_IPV6
        if (ftpSessionAPtr->ftpClientCtrlAddress.ss_family == AF_INET6)
        {
            pasvCommand = TM_CMD_EPSV;
        }
        else
#endif /* TM_USE_IPV6 */
        {
/* IPv4 */
            pasvCommand = TM_CMD_PASV;
        }

/* Passive mode: 1st command = PASV or EPSV, 2nd command = command */
        tfFtpSetCommandCode ( ftpSessionAPtr, pasvCommand, command );

/* kick the state machine */
        ftpSessionAPtr->ftpCmdRequest = pasvCommand;
        ftpSessionAPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
        result = tfFtpMatrix (ftpSessionAPtr);
    }

    if(   ( result == TM_ENOERROR )
       && ( tm_16bit_bits_not_set( ftpSessionAPtr->ftpFlags,
                                   TM_FTP_F_DATA_PASV ) ) )
    {
/* Active mode: clean up ftp2ndSessionPtr of session A */
        ftpSessionAPtr->ftp2ndSessionPtr = (ttFtpSessionPtr)0;

/* Active mode: 1st command = PORT, 2nd command = command */
        tfFtpSetCommandCode ( ftpSessionAPtr, TM_CMD_PORT, command );

/* kick the state machine */
        ftpSessionAPtr->ftpCmdRequest = TM_CMD_PORT;
        ftpSessionAPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
        result = tfFtpMatrix (ftpSessionAPtr);
    }

    return result;
}


/*
 * tfFtpCommonDataReply description:
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  fileReadWrite       File read/write flag
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 */
static int tfFtpCommonDataReply ( ttFtpSessionPtr   ftpSessionPtr,
                                  int               fileReadWrite)
{
    int           result;
    void TM_FAR * filePointer;

    if ((ftpSessionPtr->ftpReplyLine[0] != ' ') &&
        (ftpSessionPtr->ftpReplyLine[3] == ' '))
    {
/*
 * Good response received from server: If in one FTP server model, open
 * file and move to next state; if in two FTP server model, stop executing,
 * move FSM to INITIAL status
 */
        if (ftpSessionPtr->ftp2ndSessionPtr == (ttFtpSessionPtr)0)
        {
/* One FTP server model: open file and move to next state */
            filePointer = tfFSOpenFile (ftpSessionPtr->ftpUserDataPtr,
                                        ftpSessionPtr->ftpLocalFileNamePtr,
                                        fileReadWrite,
                                        TM_TYPE_BINARY,TM_STRU_STREAM);

            if (filePointer != (void TM_FAR *) 0)
            {
                ftpSessionPtr->ftpFilePointer = filePointer;

                ftpSessionPtr->ftpReplyCode = -1;

                tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_DATA_CONN);
                result = TM_EWOULDBLOCK;
            }
            else
            {
/* Error opening file, reset FSM */
                tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);
                result = TM_EINVAL;
            }
        }
        else
        {
/* Two FTP server model: stop executing and move FSM to INITIAL status */
            ftpSessionPtr->ftpCommandCode = TM_CMD_IDLE;
            ftpSessionPtr->ftpNextCommandCode = TM_CMD_IDLE;
            ftpSessionPtr->ftpCmdRequest = TM_CMD_IDLE;
            tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_INITIAL);
            result = TM_EWOULDBLOCK;
        }
    }
    else
    {
        tfFtpSetMatrixPhase (ftpSessionPtr, TM_FTPP_REPLY_WAIT);
        result = TM_EWOULDBLOCK;
    }

    return (result);
}


/*
 * tfFtpSetDataOptions function description:
 *
 * 1. Fetch the data socket in the ftpSession entry.
 * 2. Reduce the TCP time wait state time to the minimum.
 * 3. set TCP_NOPUSH TCP option
 * 4. Set call back flags to passed parameter
 * 5. set socket to non blocking mode
 * Parameters:
 * Value                Meaning
 * ftpSessionPtr        Pointer to FTP session entry
 * cbFlags              call back flags to be set on the socket
 * Return:
 * result               error Code on data connection
 */
static int tfFtpSetDataOptions ( ttFtpSessionPtr    ftpSessionPtr,
                                 int                cbFlags)
{
    int socketDesc;
    int tempInt;
    int result;

/* Data socket descriptor */
    socketDesc = ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET];
/* Change Time Wait to minimum value */
    tempInt = 0;
    result = setsockopt( socketDesc,
                         IPPROTO_TCP,
                         TM_TCP_2MSLTIME,
                         (const char TM_FAR *)&tempInt,
                         sizeof(int) );
    if (result == TM_ENOERROR)
    {
/* TCP_NOPUSH option for throughput */
        tempInt = 1;
        result = setsockopt( socketDesc,
                             IPPROTO_TCP,
                             TCP_NOPUSH,
                             (const char TM_FAR *)&tempInt,
                             sizeof(int) );
        if (result == TM_ENOERROR)
        {
/* Register socket call back */
            result = tfRegisterSocketCBParam(socketDesc,
                                             tfFtpSignal,
                                             ftpSessionPtr,
                                             cbFlags);
            if (result == TM_ENOERROR)
            {
                if ( ftpSessionPtr->ftpBlockingState == TM_BLOCKING_OFF )
                {
/* Set socket to non blocking mode */
                    result = tfBlockingState (socketDesc, TM_BLOCKING_OFF);
                }
            }
        }
    }
    return result;
}


/*
 * tfFtpGetAddress description:
 *   Converts an IP address and port into ASCII and places them in setBufPtr.
 *
 * Parameters:
 *  Value               Meaning
 *  replyLinePtr        reply line buffer to read IP address/port from
 *  addressPtr          address structure pointer to return IP address/port
 *
 * Returns:
 *  Value               Meaning
 *  TM_EINVAL           invalid reply line
 *  TM_ENOERROR (0)     success
 */
static int tfFtpGetAddress ( ttCharPtr                        replyLinePtr,
                             struct sockaddr_storage TM_FAR * addressPtr)
{
    int                 result;
    ttSockAddrPtrUnion  tmpSockPtr;
#ifdef TM_USE_IPV6
    tt16Bit             portNumber;
    ttCharPtr           endCharPtr;
    char                delimChar;
#endif /* TM_USE_IPV6 */

    result = TM_ENOERROR;
    tmpSockPtr.sockNgPtr = addressPtr;

/* Override the reply code(2xx) */
    replyLinePtr +=4;

#ifdef TM_USE_IPV6
    if (tmpSockPtr.sockNgPtr->ss_family == AF_INET6)
    {
        while (*replyLinePtr != '(')
        {
/*
 * If get the end of reply line, return TM_EINVAL.
 */
            if( ( *replyLinePtr == '\r' ) || ( *replyLinePtr == '\n' ) )
            {
                result = TM_EINVAL;
                break;
            }
            replyLinePtr++;
        }

        if (result == TM_ENOERROR)
        {
/* Get the delimiter character - used below to verify that the port number
   is formatted correctly. */
            delimChar = *(replyLinePtr+1);

/* Move past delimiters to get to port number */
            replyLinePtr += 4;

/* Get port number from EPSV reply */
            portNumber = (tt16Bit) tm_strtoul(replyLinePtr,
                                              &endCharPtr,
                                              10);

            if (*endCharPtr == delimChar)
            {
                tm_htons(portNumber, addressPtr->addr.ipv6.sin6_port);
            }
            else
            {
/* Port number not formatted correctly. */
                result = TM_EINVAL;
            }
        }
    }
    else
#endif /* TM_USE_IPV6 */
    {
/* AF_INET */
#ifdef TM_USE_IPV4
/* Find the first char of address */
        while ( (*replyLinePtr<'0') || (*replyLinePtr>'9') )
        {
/*
 * If get the end of reply line, return TM_EINVAL.
 */
            if( ( *replyLinePtr == '\r' ) || ( *replyLinePtr == '\n' ) )
            {
                result = TM_EINVAL;
                break;
            }
            replyLinePtr++;
        }

        if (result == TM_ENOERROR)
        {
/* Get the IP address and port */
            result = tfFtpHxPxFormatToSockAddr ( replyLinePtr,
                                                 tmpSockPtr.sockTInPtr );
            if( result > 0 )
            {
/* Address got is correct */
                result = TM_ENOERROR;
            }
            else
            {
/* Address got is not correct */
                result = TM_EINVAL;
            }
        }
#endif /* TM_USE_IPV4 */
    }

    return result;
}

#ifdef TM_USE_IPV6

/*
 * tfFtpCmdFuncPort description:
 *
 * FTP Command          PORT <SP> <host-port> <CRLF>
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      Operation would block/Resource temporarily unavailable.
 *  TM_ENOTCONN         Socket is not connected.
 *  TM_ENOERROR (0)     No error. (Success.)
 */
static int tfFtpCmdFuncEprt (ttFtpSessionPtr ftpSessionPtr)
{
    ttSockAddrPtrUnion clientAddr;
    ttCharPtr          setBufPtr;
    ttCharPtr          cmdNamePtr;
    ttIpPort           nowPortNo;
    ttIpPort           tempPortNo;
    int                result;
    int                sockAddrSize;
    char TM_FAR *      addrStrBuf;

    sockAddrSize = sizeof(ftpSessionPtr->ftpClientDataAddress);
    result = TM_ENOERROR;
    setBufPtr = (ttCharPtr) &ftpSessionPtr->ftpCmdSendBuf[0];
    nowPortNo = ftpSessionPtr->ftpNowPortNo;

/* Allocate temporary buffer for IPv6 address string. */
    addrStrBuf = (char TM_FAR *) tm_get_raw_buffer(INET6_ADDRSTRLEN);
    if (addrStrBuf == (char TM_FAR *) 0)
    {
        result = TM_ENOBUFS;
        goto cmdFuncEprtReturn;
    }

    if (ftpSessionPtr->ftpNextCommandCode != TM_CMD_IDLE)
    {
        if( ftpSessionPtr->ftp2ndSessionPtr == (ttFtpSessionPtr)0 )
        {
/*
 * Prepare for an incoming data connection (socket, bind, etc)
 * Bind to the port chosen by socket() or as specified by the user via
 * the tfFtpPort API
 */
            ftpSessionPtr->ftpClientDataAddress.ss_port = nowPortNo;
            result = tfFtpDataConnRequest (ftpSessionPtr);

/*
 * Reset ftpNowPortNo to 0 so we will use the port obtained from socket() next
 * time unless the user calls tfFtpPort again to set the port number
 */
            ftpSessionPtr->ftpNowPortNo = 0;
        }

/* Send EPRT command to server */
        if (result == TM_ENOERROR)
        {
/* Retrieve the port that we bound to for the EPRT command */
            clientAddr.sockNgPtr = &ftpSessionPtr->ftpClientDataAddress;
            (void)getsockname(
                            ftpSessionPtr->ftpSocketDesc[TM_FTP_LISTEN_SOCKET],
                            clientAddr.sockPtr,
                            &sockAddrSize);
            cmdNamePtr = (ttCharPtr)
                &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
            setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);
            setBufPtr = tfFtpSetString (setBufPtr, " " );
            setBufPtr = tfFtpSetString (setBufPtr, "|2|");

            inet_ntop(AF_INET6,
                      (ttVoidPtr) &clientAddr.sockIn6Ptr->sin6_addr,
                      addrStrBuf,
                      INET6_ADDRSTRLEN);

            setBufPtr = tfFtpSetString(setBufPtr, addrStrBuf);
            setBufPtr = tfFtpSetString(setBufPtr, "|");
            tempPortNo = ntohs(clientAddr.sockNgPtr->ss_port);
            setBufPtr = tfUlongDecimalToString( setBufPtr, tempPortNo );
            setBufPtr = tfFtpSetString(setBufPtr, "|");
            setBufPtr = tfFtpSetString(setBufPtr, "\r\n");
            result = tfFtpCommandSend(ftpSessionPtr);
        }
    }
    else
    {
        ftpSessionPtr->ftpClientDataAddress.ss_port =
            htons (ftpSessionPtr->ftpPortParm);

/* Prepare command string */
        cmdNamePtr = (ttCharPtr)
            &(tlFtpCmdTable[ftpSessionPtr->ftpCommandCode].fctCmdName[0]);
        setBufPtr = tfFtpSetString (setBufPtr, cmdNamePtr);
        setBufPtr = tfFtpSetString (setBufPtr, " " );
        setBufPtr = tfFtpSetString (setBufPtr, "|2|");

        clientAddr.sockNgPtr = &ftpSessionPtr->ftpClientDataAddress;
        inet_ntop(AF_INET6,
                  (ttVoidPtr) &clientAddr.sockIn6Ptr->sin6_addr,
                  addrStrBuf,
                  INET6_ADDRSTRLEN);

        setBufPtr = tfFtpSetString(setBufPtr, addrStrBuf);
        setBufPtr = tfFtpSetString(setBufPtr, "|");
        tempPortNo = ntohs(clientAddr.sockIn6Ptr->sin6_port);
        setBufPtr = tfUlongDecimalToString(setBufPtr, tempPortNo);
        setBufPtr = tfFtpSetString (setBufPtr, "|");

        setBufPtr = tfFtpSetString (setBufPtr, "\r\n");

/* Send command string to FTP server */
        result = tfFtpCommandSend (ftpSessionPtr);
    }

    if (addrStrBuf != (char TM_FAR *)0)
    {
        tm_free_raw_buffer((ttRawBufferPtr)addrStrBuf);
    }

cmdFuncEprtReturn:
    return (result);
}

#endif /* TM_USE_IPV6 */

/*
 * tfNgFtpConnect description:
 *   Connect to remote FTP server.
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  ipAddressPtr        pointer to an IP address (i.e "128.56.10.1")
 *
 * Returns:
 *  Value               Meaning
 *  TM_EWOULDBLOCK      The call is non blocking and did not complete.
 *  TM_EALREADY         command in progress (earlier call did not finish).
 *  TM_EACCES           Trying to connect to a different server without
 *                      disconnecting first.
 *  TM_EINVAL           parameter invalid.
 *  TM_ENOERROR         Success.
 */
int tfNgFtpConnect (ttUserFtpHandle                  ftpUserHandle,
                    struct sockaddr_storage TM_FAR * ipAddrPtr)
{
    int                              result;
    ttFtpSessionPtr                  ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;
    if (ftpUserHandle != (ttUserFtpHandle) 0)
    {
        if (ftpSessionPtr->ftpSocketDesc[TM_FTP_CTRL_SOCKET]
            == TM_SOCKET_ERROR)
        {
/* Save the server's IP address in the state vector */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
            if ((ipAddrPtr->ss_family == AF_INET6) &&
                IN6_IS_ADDR_V4MAPPED(&ipAddrPtr->addr.ipv6.sin6_addr))
            {
/* If we are using dual stack, and the server IP address is specified as
   an IPv4-mapped IPv6 address, then store it as an IPv4 address */
                tm_bzero(
                    &ftpSessionPtr->ftpClientCtrlAddress,
                    sizeof(struct sockaddr_storage));
                ftpSessionPtr->ftpClientCtrlAddress.addr.ipv4.sin_family
                    = AF_INET;
                ftpSessionPtr->ftpClientCtrlAddress.addr.ipv4.sin_len
                    = sizeof(struct sockaddr_storage);
                ftpSessionPtr->ftpClientCtrlAddress.addr.ipv4.sin_addr.s_addr
                    = tm_4_ip_addr(ipAddrPtr->addr.ipv6.sin6_addr);
                ftpSessionPtr->ftpClientCtrlAddress.addr.ipv4.sin_port
                    = ipAddrPtr->addr.ipv4.sin_port;
            }
            else
#endif /* TM_USE_IPV4 */
#endif /* TM_USE_IPV6 */
            {
                tm_bcopy(ipAddrPtr, &ftpSessionPtr->ftpClientCtrlAddress,
                         sizeof(struct sockaddr_storage) );
            }

/* command = CONNECT, next = IDLE */
            tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_CONN, TM_CMD_IDLE);

/* Kick the state machine */
            ftpSessionPtr->ftpCmdRequest = TM_CMD_CONN;
            ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_CONNECT;
            result = tfFtpMatrix (ftpSessionPtr);
        }
        else
        {
            result = TM_EACCES;
        }
    }
    else
    {
        result = TM_EINVAL;
    }

    return (result);
}

#ifdef TM_USE_FTP_SSL

/*
 * tfFtpSslCertCB description:
 *   Handles SSL certificate callbacks
 *
 * Parameters:
 *  Value               Meaning
 *
 * Returns:
 *
 */
static int tfFtpSslCertCB (int sessionId, int sockDesc,
                    ttCertificateEntryPtr entryArray, int arraySize)
{
    sessionId  = sessionId;
    sockDesc   = sockDesc;
    entryArray = entryArray;
    arraySize  = arraySize;

/*
 * Allow any certificate to be accepted by the client.
 */

    return TM_ENOERROR;
}

/*
 * tfFtpStartSslNeg description:
 *   Starts SSL negotiation on the given socket
 *
 * Parameters:
 *  Value               Meaning
 *  ftpSessionPtr       Pointer to FTP session structure
 *  sockDesc            Socket descriptor to start SSL on
 *
 * Returns:
 *  Error code from SSL
 */
static int tfFtpStartSslNeg (ttFtpSessionPtr ftpSessionPtr, int sockDesc)
{
    int sessionId;
    int enableClient;
    int errorCode;

/* Check the socket descriptor */
    if (sockDesc == TM_SOCKET_ERROR)
    {
        errorCode = TM_SOCKET_ERROR;
        goto startSslDataConn_End;
    }

/* Create a new SSL session, if needed */
    sessionId = ftpSessionPtr->ftpSslSessionId;
    if (sessionId < 0)
    {
        sessionId = tfSslNewSession(ftpSessionPtr->ftpCertIdentityPtr,
                                    32,
                                    ftpSessionPtr->ftpSslVersion,
                                    0);

        if (sessionId >= 0)
        {
            if (ftpSessionPtr->ftpSslServerName == (ttCharPtr)0)
            {
                tfSslRegisterCertificateCB(sessionId, tfFtpSslCertCB);
            }
            ftpSessionPtr->ftpSslSessionId = sessionId;
        }
        else
        {
/* Error codes are returned as negative values from tfSslNewSession */
            errorCode = -sessionId;
            ftpSessionPtr->ftpSslSessionId = -1;
            goto startSslDataConn_End;
        }
    }

/* Set the SSL session proposals, if set by the user */
    if (ftpSessionPtr->ftpPropArraySize > 0)
    {
        errorCode = tfSslSetSessionProposals(sessionId,
                                         ftpSessionPtr->ftpPropArray,
                                         ftpSessionPtr->ftpPropArraySize);
        if (errorCode != TM_ENOERROR)
        {
            goto startSslDataConn_End;
        }
    }

/* Enable the TM_TCP_SSL_CLIENT option */
    enableClient = 1;
    errorCode = setsockopt(sockDesc, IP_PROTOTCP, TM_TCP_SSL_CLIENT,
                          (const char TM_FAR *)&enableClient,
                           sizeof(int));
    if (errorCode != TM_ENOERROR)
    {
        goto startSslDataConn_End;
    }

/* Set the session ID */
    errorCode = setsockopt(sockDesc, IP_PROTOTCP, TM_TCP_SSLSESSION,
                          (const char TM_FAR *)&sessionId,
                           sizeof(int));
    if (errorCode != TM_ENOERROR)
    {
        goto startSslDataConn_End;
    }

/* Execute handshake code in the context of the user */
    (void)setsockopt( sockDesc,
                      IPPROTO_TCP,
                      TM_TCP_SSL_USER_PROCESS,
                      (const char TM_FAR *)&enableClient,
                      sizeof(int) );

    if (    (sockDesc == ftpSessionPtr->ftpSocketDesc[TM_FTP_DATA_SOCKET])
         && (tm_16bit_one_bit_set(ftpSessionPtr->ftpSslOptions,
                                           TM_SSL_OPT_CLIENT_CACHEID)))
    {
        errorCode =  tfSslSetSessionOptions(sessionId,
                                            TM_SSL_OPTNAM_CLIENT_CACHEID,
                                            enableClient);
        if (errorCode != TM_ENOERROR)
        {
            goto startSslDataConn_End;
        }
    }

#ifdef TM_SSL_CLIENT_RESPONSE_NO_CERT
    if ( tm_16bit_bits_not_set( ftpSessionPtr->ftpSslOptions,
                                TM_SSL_OPT_CLIENT_NOCERT ) )
    {
        enableClient = 0;
        errorCode =  tfSslSetSessionOptions(sessionId,
                                            TM_SSL_OPTNAM_CLIENT_NOCERT,
                                            enableClient);
        if (errorCode != TM_ENOERROR)
        {
            goto startSslDataConn_End;
        }
    }
#endif /* TM_SSL_CLIENT_RESPONSE_NO_CERT */

/* Start the SSL negotiation process */
    errorCode = tfSslClientUserStart(sockDesc,
                                     ftpSessionPtr->ftpSslServerName);

startSslDataConn_End:
    return errorCode;
}

/* tfFtpSslEnable
 *
 * Function description:
 * This function enables SSL/TLS for the current FTP session.  tfFtpSslEnable
 * must be called after tfFtpConnect has been called and completed
 * successfully.  It may be called before or after tfFtpLogin.
 *
 * Parameters:
 * Parameter        Description
 * ftpHandle        FTP session handle
 * certIdentity     The SSL client's own certificate name.
 * sslVersion       The SSL version number that the user wants to support for
 *                  this session. Valid values can be TM_SSL_VERSION_30,
 *                  TM_SSL_VERSION_31 or TM_SSL_VERSION_30|TM_SSL_VERSION_31.
 * serverName       Name of the FTP server (e.g. "www.treck.com").  This field
 *                  may be NULL.
 * secureDataConn   A Boolean value indicating whether future data connections
 *                  should be secure (i.e. a PROT command is issued after
 *                  authentication).
 *
 * Returns:
 * Value            Meaning
 * TM_ENOERROR      SSL/TLS successfully enabled on the current FTP session.
 * TM_EWOULDBLOCK   The FTP session is non-blocking and the call did not
 *                  complete.
 * TM_ENOTCONN      The user is not currently connected to a FTP server.
 * TM_EINVAL        Invalid FTP session handle.
 * TM_EPERM         SSL is not initialized; tfUseSsl must be called first.
 * TM_ENOBUFS       Not enough memory to complete the requested operation.
 * TM_ENOPROTOOPT   SSL/TLS is not supported by the FTP server.
 */
int tfFtpSslEnable (ttUserFtpHandle ftpHandle,
                    char TM_FAR *   certIdentity,
                    int             sslVersion,
                    char TM_FAR *   serverName,
                    int             secureDataConn)
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpHandle;

    if (tm_context(tvSslTlsPtr) == 0)
    {
/* SSL has not been initialized */
        result = TM_EPERM;
        goto sslEnable_End;
    }

/* Check validity of command (must be connected) */
    result = tfFtpCheckCommand(
        ftpSessionPtr, TM_CMD_AUTH, (ttCharPtr)"", TM_FTP_F_CTRL_CONN);

    if (result == TM_ENOERROR)
    {
        ftpSessionPtr->ftpCertIdentityPtr = certIdentity;
        ftpSessionPtr->ftpSslVersion      = sslVersion;
        ftpSessionPtr->ftpSslServerName   = serverName;

        if ( (secureDataConn == 1)
          && (ftpSessionPtr->ftpSslEnabledDataConn == TM_8BIT_NO) )
        {
/* Set the data protection level indicator to Private. */
            ftpSessionPtr->ftpDataProtLevel = 'P';
/* command = AUTH, next = PBSZ */
            tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_AUTH, TM_CMD_PBSZ);
        }
        else if( (secureDataConn == 0)
              && (ftpSessionPtr->ftpSslEnabledDataConn == TM_8BIT_YES) )
        {
/* Set the data protection level indicator to Private. */
            ftpSessionPtr->ftpDataProtLevel = 'C';
/* command = AUTH, next = PBSZ */
            tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_AUTH, TM_CMD_PBSZ);
        }
        else
        {
/* command = AUTH, next = IDLE */
            tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_AUTH, TM_CMD_IDLE);
        }

/* FTP client sends "AUTH" command with "TLS" parameter */
/* kick the state machine */
        ftpSessionPtr->ftpCmdRequest = TM_CMD_AUTH;
        ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
        result = tfFtpMatrix (ftpSessionPtr);
    }

sslEnable_End:
    return result;
}

/* tfFtpSslDisable
 *
 * Function description:
 * This function disables SSL/TLS for the specified FTP session.  Calling this
 * routine will cause the control connection to revert to plaintext and future
 * data connections will not be protected.
 *
 * Note: This routine utilizes the FTP CCC command to revert the control
 *       connection to plaintext.  Since the CCC is an optional command the
 *       FTP server may not support it, in which case this command may fail.
 *
 * Parameters:
 * Parameter       Description
 * ftpHandle       FTP session handle
 * secureDataConn  A Boolean value indicating whether future data connections
 *                 should be secure (i.e. a 'PROT C' command is issued).
 *
 * Returns:
 * Value            Meaning
 * TM_ENOERROR      SSL/TLS successfully enabled on the current FTP session.
 * TM_EWOULDBLOCK   The FTP session is non-blocking and the call did not
 *                  complete.
 * TM_ENOTCONN      The user is not currently connected to a FTP server.
 * TM_EINVAL        Invalid FTP session handle.
 * TM_EPERM         SSL is not initialized; tfUseSsl must be called first.
 * TM_ENOBUFS       Not enough memory to complete the requested operation.
 * TM_EOPNOTSUPP    CCC command is not supported by the FTP server.
 */
int tfFtpSslDisable (ttUserFtpHandle ftpHandle, int secureDataConn)
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;

    ftpSessionPtr = (ttFtpSessionPtr)ftpHandle;

    if (tm_context(tvSslTlsPtr) == 0)
    {
/* SSL has not been initialized */
        result = TM_EPERM;
        goto sslDisable_End;
    }

/* Check validity of command (must be connected) */
    result = tfFtpCheckCommand(
        ftpSessionPtr, TM_CMD_CCC, (ttCharPtr)"", TM_FTP_F_CTRL_CONN);

    if (result == TM_ENOERROR)
    {
        if ( (secureDataConn == 0) && (ftpSessionPtr->ftpSslEnabledDataConn) )
        {
/* Send a "PROT C" after the CCC */
/* Set the data protection level indicator to Clear. */
            ftpSessionPtr->ftpDataProtLevel = 'C';
/* command = CCC, next = PBSZ */
            tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_CCC, TM_CMD_PBSZ);
        }
        else
        {
/* command = CCC, next = IDLE */
            tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_CCC, TM_CMD_IDLE);
        }

/* FTP client sends "CCC" command */
/* kick the state machine */
        ftpSessionPtr->ftpCmdRequest = TM_CMD_CCC;
        ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
        result = tfFtpMatrix (ftpSessionPtr);
    }

sslDisable_End:
    return result;
}

/* tfFtpSslSetProposals
 *
 * Function description:
 * This function sets the acceptable SSL cipher suites for the specified FTP
 * session.  If this routine is called, it must be called before calling
 * tfFtpSslEnable.  If this function is not called, the default ciphers are
 * used.  For exportable version, the default cipher suites are:
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
 *
 *
 * Parameters:
 * Parameter       Description
 * ftpUserHandle   FTP session handle
 * propArray       Array of acceptable cipher suites.
 * propSize        Number of entries in the proposal array.
 *
 * Returns:
 * Value        Meaning
 * TM_ENOERROR  Cipher suite proposal successfully set for the FTP session.
 * TM_EINVAL    Invalid FTP session handle.
 * TM_EPERM     SSL is not initialized; tfUseSsl must be called first.
 * TM_EINVAL    Invalid proposal in the input array.
 */
int tfFtpSslSetProposals (ttUserFtpHandle ftpUserHandle,
                          int TM_FAR *    propArray,
                          int             propSize)
{
    int             result;
    ttFtpSessionPtr ftpSessionPtr;
#if (!defined(TM_SSL_REJECT_EXPORTABLE_CIPHER) || !defined(TM_EXPORT_RESTRICT))
#define TM_SSL_TEMP_CASE_EXISTS
    int              i;
    tt16Bit          proposal;
#endif /* (!(TM_SSL_REJECT_EXPORTABLE_CIPHER) || !(TM_EXPORT_RESTRICT)) */

    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;

/* Parameter checking */
    if (ftpSessionPtr == 0)
    {
        result = TM_EINVAL;
        goto sslSetProposals_End;
    }

    if ( (propArray == 0) || (propSize < 1) )
    {
        result = TM_EINVAL;
        goto sslSetProposals_End;
    }

    if (tm_context(tvSslTlsPtr) == 0)
    {
/* SSL has not been initialized */
        result = TM_EPERM;
        goto sslSetProposals_End;
    }

/* Reset the proposal array size if too large */
    if (propSize > TM_SSL_PROPOSAL_NUMBER)
    {
        propSize = TM_SSL_PROPOSAL_NUMBER;
    }

/* Assume proposal array is valid */
    result = TM_ENOERROR;
    ftpSessionPtr->ftpPropArraySize = propSize;

#ifndef TM_SSL_TEMP_CASE_EXISTS
    TM_UNREF_IN_ARG(propArray);

    if (propSize > 0)
    {
/* Proposals provided but NO algorithms are supported! */
            result = TM_EINVAL;
    }
#else /* TM_SSL_TEMP_CASE_EXISTS */
    for (i = 0; i < propSize; ++i)
    {
/* Check for a valid proposal */
        proposal = (tt16Bit)(propArray[i]);
        switch(proposal)
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
/* Invalid proposal */
            ftpSessionPtr->ftpPropArraySize = 0;
            result = TM_EINVAL;
            goto sslSetProposals_End;
        }

        ftpSessionPtr->ftpPropArray[i] = propArray[i];
    }
#endif /* TM_SSL_TEMP_CASE_EXISTS */
sslSetProposals_End:
    return result;
}

/* 
 * tfFtpSslSetOptions
 * 
 * Function description:
 * This function sets the SSL options for the specified FTP session.
 * If this routine is called, it must be called before calling tfFtpSslEnable.
 * 
 * Parameters:
 * Parameter       Description
 * ftpUserHandle   FTP session handle
 * optionValue     option value
 * 
 * Returns:
 * Value           Meaning
 * TM_ENOERROR     success.
 * TM_EINVAL       Invalid FTP session handle.
 * TM_EPERM        SSL is not initialized; tfUseSsl must be called first.
 */
int tfFtpSslSetOptions (ttUserFtpHandle ftpUserHandle,
                        int             optionValue)
{
    int             errorCode;
    ttFtpSessionPtr ftpSessionPtr;

    errorCode     = TM_ENOERROR;
    ftpSessionPtr = (ttFtpSessionPtr)ftpUserHandle;

    if (ftpSessionPtr == (ttFtpSessionPtr)0)
    {
/* Invalid FTP session handle. */
        errorCode = TM_EINVAL;
        goto ftpSslSetOptionsExit;
    }

    if ((optionValue
            & ~(TM_SSL_OPT_CLIENT_NOCERT | TM_SSL_OPT_CLIENT_CACHEID)) != 0)
    {
/* Invalid option value. */
        errorCode = TM_EINVAL;
        goto ftpSslSetOptionsExit;
    }

/* Verify that SSL has been initialized. */
    if (tm_context(tvSslTlsPtr) == 0)
    {
/* SSL is not initialized */
        errorCode = TM_EPERM; 
        goto ftpSslSetOptionsExit;
    }

/* Set option value. */
    ftpSessionPtr->ftpSslOptions  = (tt16Bit)optionValue;

ftpSslSetOptionsExit:
    return errorCode;
}

/* tfFtpSslSecureData
 *
 * Function description:
 * This function causes all future data connections to be secure (i.e. to use
 * SSL/TLS).  Specifically, this routine will issue a 'PROT P' command to the
 * server.
 *
 * Parameters:
 * Parameter    Description
 * ftpHandle    FTP session handle
 *
 * Returns:
 * Value           Meaning
 * TM_ENOERROR     SSL/TLS successfully enabled on the current FTP session.
 * TM_EWOULDBLOCK  The FTP session is non-blocking and the call did not
 *                 complete.
 * TM_ENOTCONN     The user is not currently connected to a FTP server.
 * TM_EINVAL       Invalid FTP session handle.
 * TM_EPERM        SSL is not initialized; tfUseSsl must be called first.
 * TM_ENOBUFS      Not enough memory to complete the requested operation.
 * TM_EOPNOTSUPP   SSL/TLS is not supported by the FTP server.
 */
int tfFtpSslSecureData (ttUserFtpHandle ftpHandle)
{
    int             errorCode;
    ttFtpSessionPtr ftpSessionPtr;

/* Assume no error */
    errorCode     = TM_ENOERROR;
    ftpSessionPtr = (ttFtpSessionPtr) ftpHandle;

    if (ftpSessionPtr == (ttFtpSessionPtr)0)
    {
/* Invalid FTP session handle. */
        errorCode = TM_EINVAL;
        goto ftpSslSecureDataExit;
    }

    errorCode = tfFtpCheckSession(ftpSessionPtr, TM_FTP_F_CTRL_CONN);

    if (errorCode != TM_ENOERROR)
    {
/* The user is not currently connected to an FTP server. */
        goto ftpSslSecureDataExit;
    }

/* Verify that SSL has been initialized. */
    if (tm_context(tvSslTlsPtr) == 0)
    {
/* SSL is not initialized */
        errorCode = TM_EPERM;
        goto ftpSslSecureDataExit;
    }

/*
 * Prepare to send the "PROT P" command.
 * The PBSZ command will be sent first. If successful, the PROT P command
 * will be sent next.
 */

/* Set the data protection level indicator to Private. */
    ftpSessionPtr->ftpDataProtLevel = 'P';
/* Send the "PBSZ 0" command and wait for reply. */
    tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_PBSZ, TM_CMD_IDLE);

/* Kick the state machine. */
    ftpSessionPtr->ftpCmdRequest = TM_CMD_PBSZ;
    ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
    errorCode = tfFtpMatrix(ftpSessionPtr);

ftpSslSecureDataExit:
    return errorCode;
}

/* tfFtpSslUnsecureData
 *
 * Function description:
 * This function causes all future data connections to be insecure (i.e. not
 * to use SSL/TLS).  Specifically, this routine will issue a 'PROT C' command
 * to the server.
 *
 * Parameters:
 * Parameter    Description
 * ftpHandle    FTP session handle
 *
 * Returns:
 * Value            Meaning
 * TM_ENOERROR      SSL/TLS successfully enabled on the current FTP session.
 * TM_EWOULDBLOCK   The FTP session is non-blocking and the call did not
 *                  complete.
 * TM_ENOTCONN      The user is not currently connected to a FTP server.
 * TM_EINVAL        Invalid FTP session handle.
 * TM_EPERM         SSL is not initialized; tfUseSsl must be called first.
 * TM_ENOBUFS       Not enough memory to complete the requested operation.
 * TM_EOPNOTSUPP    SSL/TLS is not supported by the FTP server.
 */
int tfFtpSslUnsecureData (ttUserFtpHandle ftpHandle)
{
    int             errorCode;
    ttFtpSessionPtr ftpSessionPtr;

/* Assume no error */
    errorCode     = TM_ENOERROR;
    ftpSessionPtr = (ttFtpSessionPtr) ftpHandle;

    if(ftpSessionPtr == (ttFtpSessionPtr)0)
    {
/* Invalid FTP session handle. */
        errorCode = TM_EINVAL;
        goto ftpSslSecureDataExit;
    }

    errorCode = tfFtpCheckSession(ftpSessionPtr, TM_FTP_F_CTRL_CONN);

    if (errorCode != TM_ENOERROR)
    {
/* The user is not currently connected to an FTP server. */
        goto ftpSslSecureDataExit;
    }

/* Verify that SSL has been initialized. */
    if (tm_context(tvSslTlsPtr) == 0)
    {
/* SSL is not initialized */
        errorCode = TM_EPERM;
        goto ftpSslSecureDataExit;
    }

/*
 * Prepare to send the "PROT C" command.
 * The PBSZ command will be sent first. If successful, the PROT P command
 * will be sent next.
 */

/* Set the data protection level indicator to Clear. */
    ftpSessionPtr->ftpDataProtLevel = 'C';
/* Send the "PBSZ 0" command and wait for reply. */
    tfFtpSetCommandCode (ftpSessionPtr, TM_CMD_PBSZ, TM_CMD_IDLE);

/* Kick the state machine. */
    ftpSessionPtr->ftpCmdRequest = TM_CMD_PBSZ;
    ftpSessionPtr->ftpMtrxEvent  = TM_FTPE_COMMAND;
    errorCode = tfFtpMatrix(ftpSessionPtr);

ftpSslSecureDataExit:
    return errorCode;
}

#endif /* TM_USE_FTP_SSL */

//[]----------------------------------------------------------------------[]
///	@brief			ハンドル情報の取得/FTPステータスがタイムアウト状態かどうかの判定
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-03-30<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
char	GetFTP_Handle_Status( void *pHandle, char	type )
{
	char	ret = 0;
    ttFtpSessionPtr ftpSessionPtr = (ttFtpSessionPtr)pHandle;

	if( pHandle != NULL ){
		if( type ){
			ret = ftpSessionPtr->ftpMtrxEvent;
		}else{
			if( ftpSessionPtr->ftpMtrxEvent == TM_FTPE_TIMEOUT ){
				ret = 1;
			}
		}
	}
	return ret;
}

#ifdef TM_DEBUG_LOGGING
static const char TM_FAR * tfFtpStrMatrixNo( int matrixNo )
{
    const char TM_FAR * retPtr;

    switch (matrixNo)
    {
        case TM_FTP_MATRIX0:
            retPtr = &tlFtpStrMatrixNo0[0];
            break;
        case TM_FTP_MATRIX1:
            retPtr = &tlFtpStrMatrixNo1[0];
            break;
        case TM_FTP_MATRIX2:
            retPtr = &tlFtpStrMatrixNo2[0];
            break;
        case TM_FTP_MATRIX3:
            retPtr = &tlFtpStrMatrixNo3[0];
            break;
        case TM_FTP_MATRIX4:
            retPtr = &tlFtpStrMatrixNo4[0];
            break;
        default:
            retPtr = &tlFtpStrUnknown[0];
            break;
    }
    return retPtr;
}
static const char TM_FAR * tfFtpStrMatrixEvent( int matrixEvent )
{
    const char TM_FAR * retPtr;

    switch (matrixEvent)
    {
        case TM_FTPE_CONNECT:
            retPtr = &tlFtpStrMatrixEvent0[0];
            break;
        case TM_FTPE_CLOSE:
            retPtr = &tlFtpStrMatrixEvent1[0];
            break;
        case TM_FTPE_COMMAND:
            retPtr = &tlFtpStrMatrixEvent2[0];
            break;
        case TM_FTPE_ABORT:
            retPtr = &tlFtpStrMatrixEvent3[0];
            break;
        case TM_FTPE_CTRL_CONN:
            retPtr = &tlFtpStrMatrixEvent4[0];
            break;
        case TM_FTPE_DATA_CONN:
            retPtr = &tlFtpStrMatrixEvent5[0];
            break;
        case TM_FTPE_CTRL_RECV:
            retPtr = &tlFtpStrMatrixEvent6[0];
            break;
        case TM_FTPE_CTRL_IRECV:
            retPtr = &tlFtpStrMatrixEvent7[0];
            break;
        case TM_FTPE_DATA_RECV:
            retPtr = &tlFtpStrMatrixEvent8[0];
            break;
        case TM_FTPE_SEND_REQ:
            retPtr = &tlFtpStrMatrixEvent9[0];
            break;
        case TM_FTPE_WRITE_RDY:
            retPtr = &tlFtpStrMatrixEvent10[0];
            break;
        case TM_FTPE_CTRL_ERROR:
            retPtr = &tlFtpStrMatrixEvent11[0];
            break;
        case TM_FTPE_DATA_ERROR:
            retPtr = &tlFtpStrMatrixEvent12[0];
            break;
        case TM_FTPE_TIMEOUT:
            retPtr = &tlFtpStrMatrixEvent13[0];
            break;
        default:
            retPtr = &tlFtpStrUnknown[0];
            break;
    }

    return retPtr;
}
static const char TM_FAR * tfFtpStrMatrixPhase( int matrixPhase )
{
    const char TM_FAR * retPtr;

    switch (matrixPhase)
    {
        case TM_FTPP_INITIAL:
            retPtr = &tlFtpStrMatrixPhase0[0];
            break;
        case TM_FTPP_CONNECT_WAIT:
            retPtr = &tlFtpStrMatrixPhase1[0];
            break;
        case TM_FTPP_REPLY_WAIT:
            retPtr = &tlFtpStrMatrixPhase2[0];
            break;
        case TM_FTPP_DATA_CONN:
            retPtr = &tlFtpStrMatrixPhase3[0];
            break;
        case TM_FTPP_DATA_RECV:
            retPtr = &tlFtpStrMatrixPhase4[0];
            break;
        case TM_FTPP_DATA_SEND:
            retPtr = &tlFtpStrMatrixPhase5[0];
            break;
        case TM_FTPP_SEND_WAIT:
            retPtr = &tlFtpStrMatrixPhase6[0];
            break;
        default:
            retPtr = &tlFtpStrUnknown[0];
            break;
    }

    return retPtr;
}
#endif /* TM_DEBUG_LOGGING */

#else /* !TM_USE_FTP */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */


/* To allow link for builds when TM_USE_FTP is not defined */
int tvFtpDummy = 0;

#endif /* !TM_USE_FTP */
