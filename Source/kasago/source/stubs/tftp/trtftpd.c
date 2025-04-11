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
 * $Source: source/stubs/tftp/trtftpd.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:46:30JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>  
 
 /* 
 * TFTP Server User API 
 */
 
/* Initialize the TFTP server */
void tfTftpdInit (void)
{
    return;
}

/* Start the TFTP server */
int tfTftpdUserStart (int maxConnections, 
                      int sendTimeout,
                      int timeoutTime,
                      int blockingState)
{
    maxConnections=maxConnections;
        sendTimeout=sendTimeout;
        timeoutTime=timeoutTime;
        blockingState=blockingState;
        
        return TM_ENOERROR;
}                                         


/* Stop the TFTP server */                      
int tfTftpdUserStop (void)
{
    return TM_ENOERROR;
}


/* Execute TFTP server main loop iteration (non-blocking only) */
int tfTftpdUserExecute (void)
{
    return TM_ENOERROR;
}
