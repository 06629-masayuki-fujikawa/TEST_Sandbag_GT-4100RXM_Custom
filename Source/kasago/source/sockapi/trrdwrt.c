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
 * Description: BSD Sockets Interface (tfread, tfwrite)
 *
 * Filename: trrdwrt.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trrdwrt.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:01JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


/*
 * Include
 */
#include <trsocket.h>

#ifndef TM_OPTIMIZE_SPEED

int tfRead(int socketDescriptor, char TM_FAR * bufferPtr, int bufferLength)
{
    return recv(socketDescriptor, bufferPtr, bufferLength, 0);
}

int tfWrite(int socketDescriptor, const char TM_FAR * bufferPtr, int bufferLength)
{
    return send(socketDescriptor, bufferPtr, bufferLength, 0);
}
#else /* TM_OPTIMIZE_SPEED */
int tlrdwrtDummy = 0;
#endif /* TM_OPTIMIZE_SPEED */
