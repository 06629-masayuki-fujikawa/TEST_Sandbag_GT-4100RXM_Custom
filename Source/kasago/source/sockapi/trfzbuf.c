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
 * Description: BSD Sockets Interface (tfFreeZeroCopyBuffer)
 *
 * Filename: trfzbuf.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trfzbuf.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:10JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


/*
 * Include
 */
#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>

/* 
 * Free a Zero Copy Buffer
 */
int tfFreeZeroCopyBuffer (ttUserMessage bufferHandle)
{
    int retCode;

/* PARAMETER CHECK */
    if (    (bufferHandle != (ttUserMessage)0)
         && ( (   ((ttPacketPtr)bufferHandle)->pktUserFlags
                & TM_PKTF_USER_OWNS ) ) )
    {
        tm_16bit_clr_bit( ((ttPacketPtr)bufferHandle)->pktUserFlags,
                          TM_PKTF_USER_OWNS );
/* Parameter is non null, and user owns the packet */
        tfFreePacket((ttPacketPtr)bufferHandle, TM_SOCKET_UNLOCKED);
        retCode = TM_ENOERROR;
    }
    else
    {
/* Parameter is null, or user did not own the packet */
        retCode = TM_SOCKET_ERROR;
    }
    return retCode;
}
