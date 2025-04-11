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
 * Description:  HTTPD stub function
 * Filename:     stub/trhttpd.c
 * Author:       Jin Shang
 * Date Created: 08/17/03
 * $Source: source/stubs/http/trhttpd.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:36JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

#ifdef TM_USE_HTTPD

TM_PROTO_EXTERN int tfHttpdUserInit(int numberServers)
{
    TM_UNREF_IN_ARG(numberServers);
}

ttHttpdUserServerHandle tfHttpdUserStart(
                int                         maxConnection,
                int                         idleTimeOut,
                int                         blockingState,
                char             *          rootNamePtr,
                ttUserCgiFuncPtr            cgiFuncPtr,
                struct sockaddr_storage *   sockAddrPtr,
                char             *          domainNamePtr )
{
    idleTimeOut     = idleTimeOut;
    maxConnections  = maxConnections;
    blockingState   = blockingState;
    rootNamePtr     = rootNamePtr;
    cgiFuncPtr      = cgiFuncPtr;
    sockAddrPtr     = sockAddrPtr;
    domainNamePtr   = domainNamePtr;
    
    return (ttUserVoidPtr)0;
}

int tfHttpdUserExecute(
    ttHttpdUserServerHandle userServerHandle);
{
    TM_UNREF_IN_ARG(userServerHandle);
    return TM_EPROTONOSUPPORT;
}

TM_PROTO_EXTERN int tfHttpdUserStop (
    ttHttpdUserServerHandle userServerHandle)
{
    TM_UNREF_IN_ARG(userServerHandle);
    return TM_EPROTONOSUPPORT;
}

int tfHttpdUserAuthConfig( 
    void *    authSessionPtr,
    char *    authObjPtr,
    char *    authRealm,
    char *    authUserName,
    char *    authPassword,
    void *    authDomainPtr,
    int       authStaleTime,
    int       authFlags )
{
    authSessionPtr  = authSessionPtr;
    authObjPtr      = authObjPtr;
    authRealm       = authRealm;
    authUserName    = authUserName;
    authPassword    = authPassword;
    authDomainPtr   = authDomainPtr;
    authStaleTime   = authStaleTime;
    authFlags       = authFlags;

    return TM_EPROTONOSUPPORT;
}

TM_PROTO_EXTERN int tfHttpdUserSend(
    ttHttpdUserConnHandle   connHandle,
    char TM_FAR *           bufferPtr,
    int                     bufferLen,
    int TM_FAR *            errorCodePtr)
{
    connHandle  = connHandle;
    bufferPtr   = bufferPtr;
    bufferLen   = bufferLen;
    errorCodePtr= TM_EPROTONOSUPPORT;

    return 0;
}

const char * tfHttpdUserGetEnv(  ttVoidPtr   connectionPtr,
                             int         envName   )
{
    connectionPtr   = connectionPtr;
    envName         = envName;

    return (const char*)0;
}

ttVoidPtr tfHttpdUserRetrive(ttVoidPtr connectionPtr)
{
    connectionPtr   = connectionPtr;
    
    return (ttVoidPtr)0;
}

void  ttHttpdUserStore(ttVoidPtr connectionPtr, ttVoidPtr varPtr)
{
    connectionPtr   = connectionPtr;
    varPtr          = varPtr;

    return;
}


#else /* ! TM_USE_HTTPD */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_HTTPD is not defined */
int tvHttpd = 0;

#endif /* TM_USE_HTTPD */
