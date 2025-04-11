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
 * Description: Treck telnet server stub functions
 * Filename: stubs/trtelntd.c
 * Author: Jason
 * Date Created: 6/15/99
 * $Source: source/stubs/telnet/trtelntd.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:46:28JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>

int tfTeldUserStop(void) 
{
    return TM_EPROTONOSUPPORT;
}

int tfTeldUserExecute(void)
{
    return TM_EPROTONOSUPPORT;
}

int tfTeldUserStart( int telnetOptionsAllowed,
                     int maxConnections,
                     int maxBackLog,
                     int blockingState )
{
    telnetOptionsAllowed=telnetOptionsAllowed;
    maxConnections=maxConnections;
    maxBackLog=maxBackLog;
    blockingState=blockingState;
    return TM_EPROTONOSUPPORT;
}



int tfTeldUserSend( ttUserTeldHandle teldHandle,
                    char TM_FAR *    teldSendBuf,
                    int              teldBytes,
                    int              eolFlag )
{
    teldHandle=teldHandle;
    teldSendBuf=teldSendBuf;
    teldBytes=teldBytes;
    eolFlag=eolFlag;
    return TM_EPROTONOSUPPORT;
}


int tfTeldUserClose( ttUserTeldHandle teldHandle ) 
{
    teldHandle=teldHandle;
    return TM_EPROTONOSUPPORT;
}
