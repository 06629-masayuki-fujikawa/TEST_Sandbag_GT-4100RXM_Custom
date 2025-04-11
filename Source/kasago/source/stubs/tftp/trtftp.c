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
 * Filename: trtftpd.c
 * Author: 
 * Date Created: 
 * $Source: source/stubs/tftp/trtftp.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:46:29JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h> 
 
 /* Initialize the TFTP client */
void tfTftpInit(void)
{
    return;
}

/* Retrieve a file from the server */
ttUser32Bit tfTftpGet(char TM_FAR* filename, 
                      struct sockaddr TM_FAR* remote_addr,
                      char TM_FAR* tftpbuf, 
                      ttUser32Bit bufsize, 
                      unsigned short mode,
                      int blocking)
{
    filename=filename;
    remote_addr=remote_addr;
    tftpbuf=tftpbuf;
    bufsize=bufsize;
    mode=mode;
    blocking=blocking;
    
    return TM_UL(0);
}                       


/* Transmit a file to the server */                        
ttUser32Bit tfTftpPut(char TM_FAR* filename, 
                      struct sockaddr TM_FAR* remote_addr,
                      char TM_FAR* tftpbuf, 
                      ttUser32Bit bufsize, 
                      unsigned short mode,
                      int blocking)
{
    filename=filename;
    remote_addr=remote_addr;
    tftpbuf=tftpbuf;
    bufsize=bufsize;
    mode=mode;
    blocking=blocking;
    return TM_UL(0);
}

                        
/* Execute TFTP main loop iteration (non-blocking only) */                        
ttUser32Bit tfTftpUserExecute (void)
{
    return TM_UL(0);
}

/* Set timeout value for TFTP client */
int tfTftpSetTimeout(int timeout, int retry)
{
    timeout=timeout;
    retry=retry;
    return TM_ENOERROR;
}
