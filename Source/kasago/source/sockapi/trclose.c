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
 * Description: BSD Sockets Interface (tfClose)
 *
 * Filename: trclose.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trclose.c $
 *
 * Modification History
 * $Revision: 6.0.2.3 $
 * $Date: 2010/02/26 13:10:05JST $
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
#include <trglobal.h>


/* 
 * User call to close a socket
 */
int tfClose(int socketDescriptor)
{
    ttSocketEntryPtr            socketEntryPtr;
#ifdef TM_LOCK_NEEDED
    ttLockEntryPtr              cacheLockPtr;
#endif /* TM_LOCK_NEEDED */
    ttSocketEntryPtr            cacheSocketPtr;
    ttSocketEntryPtrPtr         cacheSocketPtrPtr;
    ttPacketPtr                 packetPtr;
#ifdef TM_USE_IPSEC
    ttIpsecEntryPtr             gIpsecPtr;
    ttSASockHashListPtr         salistPtr;
    ttSASockHashListPtr         prevSalistPtr;
#endif /* TM_USE_IPSEC */
    tt8Bit                      protocol;
    int                         errorCode;
    int                         retCode;
#ifdef TM_USE_TCP
    int                         abortTheConnection;
#endif /* TM_USE_TCP */
#ifdef TM_USE_BSD_DOMAIN
    int                         af;
#endif /* TM_USE_BSD_DOMAIN */
#if (defined(TM_6_USE_MIP_MN) && defined(TM_USE_IKE))
    int                         imk;
#endif /* TM_6_USE_MIP_MN && TM_USE_IKE*/
#ifdef TM_USE_IPSEC
    unsigned int                cacheIndex;
    unsigned int                socketIndex;
#endif /* TM_USE_IPSEC */
#ifdef TM_6_USE_RAW_SOCKET
    tt8Bit                      protoFamily;
#endif /* TM_6_USE_RAW_SOCKET */

/* No more user access */
    errorCode = TM_ENOERROR;

#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_ANY;
#endif /* TM_USE_BSD_DOMAIN */
/* Parameter check */
    socketEntryPtr = tfSocketCheckValidLock( socketDescriptor
#ifdef TM_USE_BSD_DOMAIN
                                            , &af
#endif /* TM_USE_BSD_DOMAIN */
                                            );

#ifdef TM_USE_BSD_DOMAIN
    if (af == TM_BSD_FAMILY_NONINET) 
    {
        tm_assert(close, socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR);
        retCode = tfBsdClose((ttVoidPtr)socketEntryPtr);
    }
    else
#endif /* TM_USE_BSD_DOMAIN */
    {
        if (socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR)
        {
#ifdef TM_USE_IPSEC
            socketIndex = socketEntryPtr->socIndex;
#endif /* TM_USE_IPSEC */

/* socketEntryPtr non nil, socket is not closed yet */
            protocol = socketEntryPtr->socProtocolNumber;
#ifdef TM_6_USE_RAW_SOCKET
            protoFamily = socketEntryPtr->socProtocolFamily;
#endif /* TM_6_USE_RAW_SOCKET */
#ifdef TM_USE_TCP
/* Check if tfClose is really a TCP abort call */
            if (    (protocol == IP_PROTOTCP)
                 && (tm_16bit_one_bit_set( socketEntryPtr->socOptions, 
                                           SO_LINGER) )
                 && (socketEntryPtr->socLingerTime == TM_16BIT_ZERO) )
/* If linger option on with timeout of 0, we abort the connection */
            {
                abortTheConnection = 1;
            }
            else
            {
                abortTheConnection = 0;
            }
#endif /* TM_USE_TCP */
/* 
 * If socket is not in the process of closing already, and if the socket is 
 * not closed.
 * (Both TM_SOCF_CLOSING and TM_SOCF_OPEN give us re-entrancy protection for
 *  close.)
 */
            if (    ( tm_16bit_bits_not_set(socketEntryPtr->socFlags,
                                            TM_SOCF_CLOSING))
                 && (tm_16bit_one_bit_set(socketEntryPtr->socFlags,
                                          TM_SOCF_OPEN)) )
            {
/* Socket is closing. No more data can be put into send queue */
                socketEntryPtr->socFlags |=   TM_SOCF_CLOSING
                                            | TM_SOCF_NO_MORE_SEND_DATA;
#ifdef TM_6_USE_MLD
                if (socketEntryPtr->soc6MldInfoArray != TM_6_SOC_MLD_INFO_NULL_PTR)
                {
/* Socket unlocked and relocked */
                    tf6MldSocketCachePurge(socketEntryPtr
#ifdef TM_6_USE_MLDV2
                                           ,
                                           TM_6_MODE_IS_INCLUDE,
                                           TM_6_SRC_ADDR_ENTRY_NULL_PTR
#endif /*TM_6_USE_MLDV2*/
                                           );
                }
#endif /* TM_6_USE_MLD */
#ifdef TM_IGMP
#ifdef TM_USE_IGMPV3
                if (socketEntryPtr->socIgmpInfoList.listCount != 0)
#else /* !TM_USE_IGMPV3 */
                if (socketEntryPtr->socIgmpInfoArray != TM_SOC_IGMP_INFO_NULL_PTR)
#endif /* !TM_USE_IGMPV3 */
                {
/* Socket unlocked and relocked */
                    tfIgmpSocketCachePurge(socketEntryPtr);
                }
#endif /* TM_IGMP */
/*
 * Notify any pending read/write/select that socket is closing (in case
 * close is issued from another task).
 */
                tfSocketNotify(socketEntryPtr, TM_SELECT_READ|TM_SELECT_WRITE,
                                               0,
                                               TM_ESHUTDOWN);
#ifdef TM_USE_TCP
                if (protocol == IP_PROTOTCP)
                {
/*
 * Let TCP deal with
 * 1. Linger/non-blocking
 * 2. Sending a FIN if TCP state allows it.
 * 3. tfSocketTreeDelete (when TCP state transitions to CLOSED),
 * 4. tfSocketArrayDelete, and freeing outstanding receive and
 *    send packets, and close CB function (when TCP state transitions to
 *    either FIN_WAIT2 with no retransmit data, or TIME_WAIT or CLOSED)
 */
                    if (abortTheConnection == 0)
                    {
                        errorCode = 
                         tfTcpClose((ttTcpVectPtr)((ttVoidPtr)socketEntryPtr));
                        if (errorCode == TM_ESHUTDOWN)
                        {
/* Do not generate an error if shutdown has been called */
                            errorCode = TM_ENOERROR;
                        }
                    }
                }
                else
#endif /* TM_USE_TCP */
                {
#ifdef TM_ERROR_CHECKING
                    if (socketEntryPtr->socOwnerCount < 3)
                    {
                        if (  tm_16bit_one_bit_set(socketEntryPtr->socFlags,
                                                   TM_SOCF_IN_TREE)
                             || (socketEntryPtr->socOwnerCount < 2) )
                        {
                            tfKernelError("tfClose",
                                          "socket has wrong ownership");
                        }
                    }
#endif /* TM_ERROR_CHECKING */
/*
 * remove from socket array. No longer accessible to the user. 
 */
                    tfSocketArrayDelete(socketEntryPtr);
/* Remove entry from the tree */
                    retCode = tfSocketTreeDelete(socketEntryPtr, TM_16BIT_ZERO);

                    if (retCode == TM_SOC_RB_OKAY)
                    {
/*
 * Reset an non TCP incoming protocol socket cache if it matches the
 * closing socket:
 * . Find protocol socket entry cache.
 * . If there is a protocol socket entry cache:
 *    .. Unlock socketEntryPtr (to avoid deadlock with protocol cache
 *       lock)
 *    .. Lock the cache.
 *    .. If there is a match:
 *       ... reset the cache
 *       ... update socket owner count with socket lock on (locking, unlocking
 *           socketEntryPtr
 *       ... unlock the cache 
 *    .. Relock socketEntryPtr.
 */
/* Need to unlock to avoid deadlock with cache lock */
                        switch (protocol)
                        {
                            case IPPROTO_UDP:
#ifdef TM_DISABLE_UDP_CACHE
                                cacheSocketPtrPtr = (ttSocketEntryPtrPtr)0; 
#ifdef TM_LOCK_NEEDED
                                cacheLockPtr = (ttLockEntryPtr)0;
#endif /* TM_LOCK_NEEDED */
#else /* !TM_DISABLE_UDP_CACHE */
/* UDP cache lock */
#ifdef TM_LOCK_NEEDED
                                cacheLockPtr = &tm_context(tvUdpCacheLock);
#endif /* TM_LOCK_NEEDED */
                                cacheSocketPtrPtr = &tm_context(tvUdpSocketPtr);
#endif /* !TM_DISABLE_UDP_CACHE */
                                break;

                            default:
#ifdef TM_6_USE_RAW_SOCKET
                                if (protoFamily == PF_INET6)
                                {
/* Raw socket cache lock */
#ifdef TM_LOCK_NEEDED
                                    cacheLockPtr = &tm_context(tv6RawCacheLock);
#endif /* TM_LOCK_NEEDED */
                                    cacheSocketPtrPtr =
                                        &tm_context(tv6RawSocketPtr);

                                }
                                else
#endif /* TM_6_USE_RAW_SOCKET */
                                {
/* Raw socket cache lock */
#ifdef TM_LOCK_NEEDED
                                    cacheLockPtr = &tm_context(tvRawCacheLock);
#endif /* TM_LOCK_NEEDED */
                                    cacheSocketPtrPtr =
                                        &tm_context(tvRawSocketPtr);
                                }
                                break;
        
                        }

#ifdef TM_DISABLE_UDP_CACHE
                        if (cacheSocketPtrPtr != (ttSocketEntryPtrPtr)0)
#endif /* TM_DISABLE_UDP_CACHE */
                        {
                            tm_call_unlock(&socketEntryPtr->socLockEntry);
                            tm_call_lock_wait(cacheLockPtr);
                            cacheSocketPtr = *cacheSocketPtrPtr;
                            if (socketEntryPtr == cacheSocketPtr)
                            {
/*
 * Socketentry is unlocked. Since the closing thread owns the socket, we do
 * not have to worry about checkout and checkin.
 */
                                tm_call_lock_wait(&socketEntryPtr->socLockEntry);
                                socketEntryPtr->socOwnerCount--;
                                tm_call_unlock(&socketEntryPtr->socLockEntry);
/* Reset the cache socket pointer */
                                *cacheSocketPtrPtr = TM_SOCKET_ENTRY_NULL_PTR;
                            }
                            tm_call_unlock(cacheLockPtr);
/* Re-lock the entry */
                            tm_call_lock_wait(&socketEntryPtr->socLockEntry);
                        }

                    }
/* socket entry no longer accessible from UDP/ICMP */
/* Remove queue of packets from the receive queue */
                    packetPtr = socketEntryPtr->socReceiveQueueNextPtr;
                    socketEntryPtr->socReceiveQueueNextPtr =
                                                       TM_PACKET_NULL_PTR;
/* Free all the packets in the receive queue */
                    tfFreeChainPacket(packetPtr, TM_SOCKET_LOCKED);
                }
            }
            else
            {
/* tfClose has already been called */
                errorCode = TM_EALREADY;
            }
#ifdef TM_USE_TCP
            if (abortTheConnection != 0)
            {
/*
 * Let TCP deal with
 * 1. Sending a RESET if TCP state allows it.
 * 2. tfSocketTreeDelete
 * 3. tfSocketArrayDelete, and freeing outstanding receive and
 *    send packets, and close CB function.
 * Note that even if tfClose had been called before, we allow the
 * user to abort the connection.
 */
                errorCode = tfTcpAbort( 
                                   (ttTcpVectPtr)((ttVoidPtr)socketEntryPtr),
                                   TM_ECONNABORTED );
            }
#endif /* TM_USE_TCP */
        }
        else
        {
#ifdef TM_USE_IPSEC
            socketIndex = TM_SOC_NO_INDEX;
#endif /* TM_USE_IPSEC */
            errorCode = TM_EBADF;
        }

#ifdef TM_USE_IPSEC
        tm_call_lock_wait(&(tm_context(tvIpsecLockEntry)));
        gIpsecPtr = (ttIpsecEntryPtr)tm_context(tvIpsecPtr);
        if (gIpsecPtr && (socketEntryPtr != (ttSocketEntryPtr)0))
        {

            prevSalistPtr = (ttSASockHashListPtr)0;
/* for TCP socket, we queued the SA for this socket, now need clearing */
            if( socketEntryPtr->socProtocolNumber == IP_PROTOTCP && 
                socketIndex != TM_SOC_NO_INDEX)
            {
                cacheIndex = socketIndex % TM_IPSEC_SOCKSA_CACHE_SIZE;
                salistPtr = gIpsecPtr->ipsecSocketCache[cacheIndex];
                while( (salistPtr != (ttSASockHashListPtr)0) &&
                       (salistPtr->sshSock != socketIndex) )
                {
                    prevSalistPtr = salistPtr;
                    salistPtr = salistPtr->sshHashNextPtr;
                }
                if (salistPtr != (ttSASockHashListPtr)0)
                {
/* we found it */
                    if (prevSalistPtr == (ttSASockHashListPtr)0)
                    {
                        gIpsecPtr->ipsecSocketCache[cacheIndex] = 
                        salistPtr->sshHashNextPtr;
                    }
                    else
                    {
                        prevSalistPtr->sshHashNextPtr = 
                                                    salistPtr->sshHashNextPtr;
                    }
                    tfSadbFreeSocketHashList(salistPtr);
                }
            }
/* 
 * If MN & IKE is defined, we need to set the IKE socket descriptor
 * back to -1 to indicate that this home address has no IKE running
 */
#if (defined(TM_6_USE_MIP_MN) && defined(TM_USE_IKE))
            {
                for(imk = 0; 
                    imk < TM_6_MAX_LOCAL_MHOME; 
                    imk ++)
                {
                    if(tm_context(tv6MnVect).mns6IkeSd[imk] == 
                            socketDescriptor)
                    {
                        tm_context(tv6MnVect).mns6IkeSd[imk] = 
                                 TM_SOCKET_ERROR;
/* There could be only one home address uses this socket number */
                        break;
                    }
                }
            }
#endif /* TM_6_USE_MIP_MN && TM_USE_IKE*/
        }
        tm_call_unlock(&(tm_context(tvIpsecLockEntry)));
#endif /* TM_USE_IPSEC */

        retCode = tfSocketReturn(socketEntryPtr, socketDescriptor,
                                 errorCode, TM_ENOERROR);
    }
    return retCode;
}
