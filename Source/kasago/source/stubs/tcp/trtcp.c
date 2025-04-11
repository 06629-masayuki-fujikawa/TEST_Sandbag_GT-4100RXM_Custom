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
 * Description:     TCP stub functions
 *
 * Filename:        stubs/trtcp.c
 * Author:          Jason & Odile
 * Date Created:    7/13/98
 * $Source: source/stubs/tcp/trtcp.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:46:26JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

void tfTcpInit(void)
{
}

void tfTcpVectInit(ttTcpVectPtr tcpVectPtr)
{
    TM_UNREF_IN_ARG(tcpVectPtr);
}

int tfTcpOpen(ttTcpVectPtr tcpVectPtr, tt8Bit event)
{
    TM_UNREF_IN_ARG(tcpVectPtr);
    TM_UNREF_IN_ARG(event);
    return TM_EPROTONOSUPPORT;
}

int tfTcpAccept(ttTcpVectPtr     listenTcpVectPtr)
{
    TM_UNREF_IN_ARG(listenTcpVectPtr);
    return TM_EPROTONOSUPPORT;
}

void tfTcpRecvCmplt(ttTcpVectPtr tcpVectPtr, ttPktLen copyCount, int flags)
{
    TM_UNREF_IN_ARG(tcpVectPtr);
    TM_UNREF_IN_ARG(copyCount);
    TM_UNREF_IN_ARG(flags);
}


int tfTcpRecv(ttTcpVectPtr tcpVectPtr)
{
    TM_UNREF_IN_ARG(tcpVectPtr);
    return TM_EPROTONOSUPPORT;
}

int tfTcpRecvOobData( ttTcpVectPtr  tcpVectPtr,
                      char TM_FAR * bufferPtr,
                      int           flags)
{
    TM_UNREF_IN_ARG(tcpVectPtr);
    TM_UNREF_IN_ARG(bufferPtr);
    TM_UNREF_IN_ARG(flags);
    return TM_EPROTONOSUPPORT;
}

void tfTcpShutRead(ttTcpVectPtr tcpVectPtr)
{
    TM_UNREF_IN_ARG(tcpVectPtr);
}

int tfTcpShutWrite(ttTcpVectPtr tcpVectPtr)
{
    TM_UNREF_IN_ARG(tcpVectPtr);
    return TM_EPROTONOSUPPORT;
}

int tfTcpClose(ttTcpVectPtr tcpVectPtr)
{
    TM_UNREF_IN_ARG(tcpVectPtr);
    return TM_EPROTONOSUPPORT;
}

void tfTcpIncomingPacket(ttPacketPtr packetPtr)
{
    tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
}

int tfTcpSendPacket(ttTcpVectPtr tcpVectPtr)
{
#define socketEntryPtr ((ttSocketEntryPtr)tcpVectPtr)
    TM_UNREF_IN_ARG(tcpVectPtr);
/* Free all the packets in the send queue */
    tfFreePacket( socketEntryPtr->socSendQueueNextPtr,
                  TM_SOCKET_LOCKED);
    socketEntryPtr->socSendQueueBytes = 0;
    socketEntryPtr->socSendQueueNextPtr = TM_PACKET_NULL_PTR;
    socketEntryPtr->socSendQueueLastPtr = TM_PACKET_NULL_PTR;
    return TM_EPROTONOSUPPORT;
#undef socketEntryPtr
}

int tfTcpAbort(ttTcpVectPtr tcpVectPtr, int error)
{
    TM_UNREF_IN_ARG(tcpVectPtr);
    TM_UNREF_IN_ARG(error);
    return TM_EPROTONOSUPPORT;
}

void tfTcpIcmpErrInput(ttTcpVectPtr tcpVectPtr, int error)
{
    TM_UNREF_IN_ARG(tcpVectPtr);
    TM_UNREF_IN_ARG(error);
}

void tfTcpPurgeTcpHeaders(ttTcpVectPtr tcpVectPtr)
{
    TM_UNREF_IN_ARG(tcpVectPtr);
}


void tfTcpSocketFlushRQ (ttTcpVectPtr tcpVectPtr)
{
    TM_UNREF_IN_ARG(tcpVectPtr);
}

void tfTcpRemoveConReq (ttTcpVectPtr tcpVectPtr, int closing)
{
    TM_UNREF_IN_ARG(tcpVectPtr);
    TM_UNREF_IN_ARG(closing);
    return;
}
