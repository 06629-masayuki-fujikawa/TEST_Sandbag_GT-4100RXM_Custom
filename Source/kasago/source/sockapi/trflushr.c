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
 * Description: tfFlushRecvQ API 
 * Filename: trflushr.c
 * Author: Odile
 * Date Created: 02/19/02
 * $Source: source/sockapi/trflushr.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:07JST $
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
 * User call to flush the socket receive queue. 
 */
int tfFlushRecvQ(int socketDescriptor)
{
    ttSocketEntryPtr    socketEntryPtr;
    ttPacketPtr         packetPtr;
#ifdef TM_USE_BSD_DOMAIN
    int                 af;
#endif /* TM_USE_BSD_DOMAIN */
    int                 errorCode;

    errorCode = TM_ENOERROR;
/* Parameter check */
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_INET;
#endif /* TM_USE_BSD_DOMAIN */
    socketEntryPtr = tfSocketCheckValidLock(  socketDescriptor
#ifdef TM_USE_BSD_DOMAIN
                                            , &af
#endif /* TM_USE_BSD_DOMAIN */
                                           );

    if (socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR)
    {
/* Remove queue of packets from the receive queue */
        packetPtr = socketEntryPtr->socReceiveQueueNextPtr;
        socketEntryPtr->socReceiveQueueNextPtr = TM_PACKET_NULL_PTR;
        socketEntryPtr->socReceiveQueueLastPtr = TM_PACKET_NULL_PTR;
        socketEntryPtr->socRecvQueueBytes = 0;
        socketEntryPtr->socRecvQueueDgrams = TM_16BIT_ZERO;
/* Free all the packets in the receive queue */
        tfFreeChainPacket(packetPtr, TM_SOCKET_LOCKED);
    }
#ifdef TM_USE_BSD_DOMAIN
    else
    {
        if (af == TM_BSD_FAMILY_NONINET)
        {
            errorCode = TM_EOPNOTSUPP;
        }
    }
#endif /* TM_USE_BSD_DOMAIN */
    return tfSocketReturn(socketEntryPtr, socketDescriptor,
                          errorCode, TM_ENOERROR);
}
