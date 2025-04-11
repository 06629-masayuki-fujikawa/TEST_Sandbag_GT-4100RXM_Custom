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
 * Author: Jason
 * Date Created: 01/13/99
 * $Source: source/stubs/ftp/trftp.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:34JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>


/* Allocate and initialize a new session */
ttUserFtpHandle tfFtpNewSession (int           fileSystemFlags,     
                                 int           blockingState,
                                 char TM_FAR * fsUsernamePtr,
                                 char TM_FAR * fsPasswordPtr)
{
    fileSystemFlags=fileSystemFlags;
    blockingState=blockingState;
    fsUsernamePtr=fsUsernamePtr;
    fsPasswordPtr=fsPasswordPtr;

    return ((ttUserFtpHandle) 0);
}                                 
                            
                                 
/* Deallocate a session */
int tfFtpFreeSession(ttUserFtpHandle ftpSessionPtr)
{
    ftpSessionPtr=ftpSessionPtr;

    return TM_ENOPROTOOPT;
}

/*
 * PORT command. Turn on, or off the port usage. Default is on with FTP.
 * If onFlag is 1, turns on the FTP port usage. If onFlag is 0
 * turns it off.
 */
int tfFtpTurnPort(ttUserFtpHandle ftpSessionPtr,
                  int             onFlag)
{
    ftpSessionPtr=ftpSessionPtr;
    onFlag=onFlag;

    return TM_ENOPROTOOPT;
}                  

/*
 * PASV command. Turn on, or off the pasv usage. Default is off with FTP.
 * If onFlag is 1, turns on the FTP port usage. If onFlag is 0
 * turns it off.
 */
int tfFtpTurnPasv( ttUserFtpHandle ftpSessionPtr,
                   int             onFlag )
{
    ftpSessionPtr=ftpSessionPtr;
    onFlag=onFlag;

    return TM_ENOPROTOOPT;
}

/*
 * Close an FTP client socket, without sending a QUIT command. Useful in
 * case the connection failed, and hence the tfFtpQuit() could not be
 * executed, or tfFtpQuit() failed for any other reason.
 */
int tfFtpClose(ttUserFtpHandle ftpSessionPtr)
{
    ftpSessionPtr=ftpSessionPtr;

    return TM_ENOPROTOOPT;
}

/* Connect to an FTP server */
int tfFtpConnect(ttUserFtpHandle ftpSessionPtr,
                 char TM_FAR   * ipAddressPtr )
{
    ftpSessionPtr=ftpSessionPtr;
    ipAddressPtr=ipAddressPtr;

    return TM_ENOPROTOOPT;
}                 

/* Login command */
int tfFtpLogin(ttUserFtpHandle ftpsessionPtr,
               char   TM_FAR * userNamePtr,
               char   TM_FAR * passwordNamePtr,
               char   TM_FAR * accountNamePtr)
{
    ftpsessionPtr=ftpsessionPtr;
    userNamePtr=userNamePtr;
    passwordNamePtr=passwordNamePtr;
    accountNamePtr=accountNamePtr;

    return TM_ENOPROTOOPT;
}               

/* CWD command */
int tfFtpCwd(ttUserFtpHandle ftpSessionPtr,
             char   TM_FAR * pathNamePtr)
{
    ftpSessionPtr=ftpSessionPtr;
    pathNamePtr=pathNamePtr;

    return TM_ENOPROTOOPT;
}             


/* CDUP command */
int tfFtpCdup(ttUserFtpHandle ftpSessionPtr)
{
    ftpSessionPtr=ftpSessionPtr;

    return TM_ENOPROTOOPT;
}

/* QUIT command */
int tfFtpQuit(ttUserFtpHandle ftpSessionPtr)
{
    ftpSessionPtr=ftpSessionPtr;

    return TM_ENOPROTOOPT;
}

/* SMNT (structure mount) command */
int tfFtpSmnt(ttUserFtpHandle ftpSessionPtr,
              char   TM_FAR * mountPathPtr)
{
    ftpSessionPtr=ftpSessionPtr;
    mountPathPtr=mountPathPtr;

    return TM_ENOPROTOOPT;
}              

/* REIN (reinitialize) command */
int tfFtpRein(ttUserFtpHandle ftpSessionPtr)
{
    ftpSessionPtr=ftpSessionPtr;

    return TM_ENOPROTOOPT;
}


/* PASV (passive) command */
int tfFtpPasv(ttUserFtpHandle             ftpSessionPtr,
              struct sockaddr_in TM_FAR * portAddrPtr)
{
    ftpSessionPtr=ftpSessionPtr;
    portAddrPtr=portAddrPtr;

    return TM_ENOPROTOOPT;
}              

/* TYPE command, type one of TM_ASCII, TM_BINARY */
int tfFtpType(ttUserFtpHandle ftpSessionPtr,
              int             type)
{
    ftpSessionPtr=ftpSessionPtr;
    type=type;

    return TM_ENOPROTOOPT;
}              

/* STRU command, structure one of TM_STRU_STREAM, TM_STRU_RECORD */
int tfFtpStru(ttUserFtpHandle ftpSessionPtr,
              int             structure)
{
    ftpSessionPtr=ftpSessionPtr;
    structure=structure;

    return TM_ENOPROTOOPT;
}              

/* RETR command */
int tfFtpRetr ( ttUserFtpHandle             ftpUserHandleA,
                ttUserFtpHandle             ftpUserHandleB,
                char TM_FAR *               fromFileNamePtr,
                char TM_FAR *               toFileNamePtr)
{
    ftpUserHandleA  = ftpUserHandleA;
    ftpUserHandleB  = ftpUserHandleB;
    TM_UNREF_IN_ARG(fromFileNamePtr);
    toFileNamePtr   = toFileNamePtr;

    return TM_ENOPROTOOPT;
}              

/* STOR command */
int tfFtpStor ( ttUserFtpHandle             ftpUserHandleA,
                ttUserFtpHandle             ftpUserHandleB,
                char TM_FAR *               fromFileNamePtr,
                char TM_FAR *               toFileNamePtr)
{
    ftpUserHandleA  = ftpUserHandleA;
    ftpUserHandleB  = ftpUserHandleB;
    TM_UNREF_IN_ARG(fromFileNamePtr);
    toFileNamePtr   = toFileNamePtr;
        
    return TM_ENOPROTOOPT;
}              

/* STOU command */
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

    return TM_ENOPROTOOPT;
}              

/* APPE command */
int tfFtpAppe ( ttUserFtpHandle             ftpUserHandleA,
                ttUserFtpHandle             ftpUserHandleB,
                char TM_FAR *               fromFileNamePtr,
                char TM_FAR *               toFileNamePtr)
{
    ftpUserHandleA  = ftpUserHandleA;
    ftpUserHandleB  = ftpUserHandleB;
    TM_UNREF_IN_ARG(fromFileNamePtr);
    toFileNamePtr   = toFileNamePtr;
    
    return TM_ENOPROTOOPT;
}              

/* ALLO command */
int tfFtpAllo ( ttUserFtpHandle ftpUserHandle,
                ttUser32Bit     sizeBytes,
                int             recordSizeBytes )
{
    ftpUserHandle=ftpUserHandle;
    sizeBytes=sizeBytes;
    recordSizeBytes=recordSizeBytes;

    return TM_ENOPROTOOPT;
}              

/* RNFR, RNTO command */
int tfFtpRename(ttUserFtpHandle ftpSessionPtr,
                char   TM_FAR * fromNamePtr,
                char   TM_FAR * toNamePtr)
{
    ftpSessionPtr=ftpSessionPtr;
    fromNamePtr=fromNamePtr;
    toNamePtr=toNamePtr;

    return TM_ENOPROTOOPT;
}                

/* ABOR command */
int tfFtpAbor(ttUserFtpHandle ftpSessionPtr)
{
    ftpSessionPtr=ftpSessionPtr;

    return TM_ENOPROTOOPT;
}

/* DELE command */
int tfFtpDele(ttUserFtpHandle ftpSessionPtr,
              char   TM_FAR * pathNamePtr)
{
    ftpSessionPtr=ftpSessionPtr;
    pathNamePtr=pathNamePtr;

    return TM_ENOPROTOOPT;
}              

/* RMD command */
int tfFtpRmd(ttUserFtpHandle ftpSessionPtr,
             char   TM_FAR * directoryPathNamePtr)
{
    ftpSessionPtr=ftpSessionPtr;
    directoryPathNamePtr=directoryPathNamePtr;

    return TM_ENOPROTOOPT;
}             

/* MKD command */
int tfFtpMkd(ttUserFtpHandle ftpSessionPtr,
             char   TM_FAR * directoryPathNamePtr,
             char   TM_FAR * bufferPtr,
             int             bufferSize)
{
    ftpSessionPtr=ftpSessionPtr;
    directoryPathNamePtr=directoryPathNamePtr;
    bufferPtr=bufferPtr;
    bufferSize=bufferSize;

    return TM_ENOPROTOOPT;
}

/* PWD command */
int tfFtpPwd(ttUserFtpHandle ftpSessionPtr,
             char   TM_FAR * bufferPtr,
             int             bufferSize)
{
    ftpSessionPtr=ftpSessionPtr;
    bufferPtr=bufferPtr;
    bufferSize=bufferSize;

    return TM_ENOPROTOOPT;
}

/*
 * LIST/NLST command. (*ftpDirCBFuncPtr)() called for each line of
 * directory entry
 */
int tfFtpDirList(ttUserFtpHandle     ftpSessionPtr,
                 char       TM_FAR * pathNamePtr,
                 int                 directoryFlag,
                 ttFtpCBFuncPtr      ftpDirCBFuncPtr)
{
    ftpSessionPtr=ftpSessionPtr;
    pathNamePtr=pathNamePtr;
    directoryFlag=directoryFlag;
    ftpDirCBFuncPtr=ftpDirCBFuncPtr;

    return TM_ENOPROTOOPT;
}                 


/* SITE command */
int tfFtpSite(ttUserFtpHandle ftpSessionPtr,
              char   TM_FAR * sitePtr)
{
    ftpSessionPtr=ftpSessionPtr;
    sitePtr=sitePtr;

    return TM_ENOPROTOOPT;
}              

/* SYST command */
int tfFtpSyst(ttUserFtpHandle ftpSessionPtr,
              char   TM_FAR * bufferPtr,
              int             bufferSize)
{
    ftpSessionPtr=ftpSessionPtr;
    bufferPtr=bufferPtr;
    bufferSize=bufferSize;

    return TM_ENOPROTOOPT;
}

/* STAT command */
int tfFtpStat(ttUserFtpHandle ftpSessionPtr,
              char   TM_FAR * fileNamePtr,
              char   TM_FAR * bufferPtr,
              int             bufferSize)
{
    ftpSessionPtr=ftpSessionPtr;
    fileNamePtr=fileNamePtr;
    bufferPtr=bufferPtr;
    bufferSize=bufferSize;

    return TM_ENOPROTOOPT;
}

/*
 * HELP command. (*ftpCBFuncPtr)() called for each help line.
 */
int tfFtpHelp(ttUserFtpHandle ftpSessionPtr,
              char   TM_FAR * commandPtr,
              ttFtpCBFuncPtr  ftpCBFuncPtr)
{
    ftpSessionPtr=ftpSessionPtr;
    commandPtr=commandPtr;
    ftpCBFuncPtr=ftpCBFuncPtr;

    return TM_ENOPROTOOPT;
}              

/* MODE command */
int tfFtpMode (ttUserFtpHandle ftpUserHandle, int mode)
{
    ftpUserHandle=ftpUserHandle;
    mode=mode;

    return TM_ENOPROTOOPT;
}

/* NOOP command */
int tfFtpNoop (ttUserFtpHandle ftpUserHandle)
{
    ftpUserHandle=ftpUserHandle;

    return TM_ENOPROTOOPT;
}

/* Sets the incoming data port for FTP client operations */
int tfFtpPort (ttUserFtpHandle ftpUserHandle, 
               ttUserIpPort    ftpPortNo)
{
    ftpUserHandle=ftpUserHandle;
    ftpPortNo=ftpPortNo;

    return TM_ENOPROTOOPT;
}

/* Returns full reply text for most recent command */
int tfFtpGetReplyText(ttUserFtpHandle ftpUserHandle,
                      char TM_FAR *   replyStrPtr,
                      int             replyStrLen)
{
    ftpUserHandle=ftpUserHandle;
    replyStrPtr=replyStrPtr;
    replyStrLen=replyStrLen;

    return TM_ENOPROTOOPT;
}

/*
 * Execute and check on the status of the last FTP client command
 * (non blocking mode only)
 */
 int tfFtpUserExecute(ttUserFtpHandle ftpSessionPtr)
 {
    ftpSessionPtr=ftpSessionPtr;

    return TM_ENOPROTOOPT;
}
