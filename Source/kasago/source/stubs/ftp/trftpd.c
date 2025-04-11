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
 * Description:  FTPD stub function
 * Filename:     stub/trftpd.c
 * Author:       Odile
 * Date Created: 12/17/98
 * $Source: source/stubs/ftp/trftpd.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:35JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>


int tfFtpdUserStart ( int fileFlags,
                      int maxConnections,
                      int maxBackLog,
                      int idleTimeout,
                      int blockingState)
{
    TM_UNREF_IN_ARG(fileFlags);
    TM_UNREF_IN_ARG(maxConnections);
    TM_UNREF_IN_ARG(maxBackLog);
    TM_UNREF_IN_ARG(idleTimeout);
    TM_UNREF_IN_ARG(blockingState);
    return TM_EPROTONOSUPPORT;
}

int tfFtpdUserStop (void)
{
    return TM_EPROTONOSUPPORT;
}

int tfFtpdUserExecute (void)
{
    return TM_EPROTONOSUPPORT;
}
