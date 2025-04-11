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
 * Description: BSD Sockets Interface   (accept)
 *
 * Filename: traccept.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/traccept.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:42:38JST $
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
#ifdef TM_LOCK_NEEDED
#include <trglobal.h>
#endif /* TM_LOCK_NEEDED */
#if defined(TM_ERROR_CHECKING) && defined(TM_USE_BSD_DOMAIN)
#include <trglobal.h>
#endif /* TM_ERROR_CHECKING && TM_USE_BSD_DOMAIN */

/*
 * Get listening socket entry pointer (whose index is
 * socketDescriptor) from socket array.
 * Get an accepted socket from the queue of connection
 * requests of the listening socket, pending if
 * none is there and listening socket is non blocking.
 * If found, increase socBackLog of listening socket
 * Insert in the socket array. If no space available,
 * reset connection (tfTcpAbort())
 */
int accept(int                    listenSocketDescriptor,
           struct sockaddr TM_FAR *peerAddressPtr,
           int TM_FAR             *addressLengthPtr)
{
#ifdef TM_USE_TCP
    ttTcpVectPtr                listenTcpVectPtr;
    ttTcpVectPtr                tcpVectPtr;
    ttSocketEntryPtr            socketPtr;
    ttSockAddrPtrUnion          peerAddressInPtr;
#endif /* TM_USE_TCP */
    ttSocketEntryPtr            listenSocketEntryPtr;
#ifdef TM_USE_BSD_DOMAIN
    int                         af;
#endif /* TM_USE_BSD_DOMAIN */
    int                         index;
    int                         errorCode;
    int                         retCode;
    int                         addressLength;
#ifdef TM_USE_TCP
    tt8Bit                      protocol;
    tt8Bit                      foundConReq;
#endif /* TM_USE_TCP */

    errorCode = TM_ENOERROR;
    index = TM_SOCKET_ERROR; /* no accept socket descriptor yet */
#ifdef TM_USE_TCP
    foundConReq = TM_8BIT_ZERO;
#endif /* TM_USE_TCP */

    if (addressLengthPtr == (int TM_FAR *) 0)
    {
        addressLength = 0;
    }
    else
    {
        addressLength = *addressLengthPtr;
    }

/*
 * Map from socket descriptor to a locked socket entry pointer and verify
 * that the address length is at least what we expect for this address type.
 */    
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_ANY;
#endif /* TM_USE_BSD_DOMAIN */
    listenSocketEntryPtr = tfSocketCheckAddrLenLock(listenSocketDescriptor,
                                                    peerAddressPtr,
                                                    addressLength,
                                                    &errorCode
#ifdef TM_USE_BSD_DOMAIN
                                                  , &af
#endif /* TM_USE_BSD_DOMAIN */
                                                    );
#ifdef TM_USE_BSD_DOMAIN
    if (af == TM_BSD_FAMILY_NONINET) 
    {
        tm_assert(accept, (listenSocketEntryPtr != (ttSocketEntryPtr)0));
        retCode = tfBsdAccept( (ttVoidPtr)listenSocketEntryPtr, peerAddressPtr,
                               addressLengthPtr);
    }
    else
#endif /* !TM_USE_BSD_DOMAIN */

    {
        if (listenSocketEntryPtr != (ttSocketEntryPtr)0)
        {
#ifdef TM_USE_TCP
            protocol = listenSocketEntryPtr->socProtocolNumber;
/* accept only supported for TCP */
            if (protocol == IP_PROTOTCP)
            {
                listenTcpVectPtr = (ttTcpVectPtr)
                                        ((ttVoidPtr)listenSocketEntryPtr);
                tcpVectPtr = listenTcpVectPtr;
                socketPtr = &(tcpVectPtr->tcpsSocketEntry);
                do
                {
                    if ( tm_16bit_all_bits_set(
                             listenSocketEntryPtr->socFlags,
                             TM_SOCF_LISTENING|TM_SOCF_IN_TREE|TM_SOCF_OPEN ) )
                    {
/* Listen socket is in the tree and in the array */
/*
 * Get a socket entry that has an established connection and has not been
 * open yet (no accept call on it yet) in the list of fully connected
 * connection requests ready to be accepted off the listening socket.
 */
                        tcpVectPtr = 
                          listenTcpVectPtr->tcpsAcceptQueue.tcpConReqNextPtr;
                        socketPtr = &(tcpVectPtr->tcpsSocketEntry);
                        if (tcpVectPtr != listenTcpVectPtr)
                        {
/* Lock queued socket */
                            tm_call_lock_wait(&socketPtr->socLockEntry);
/*
 * Check out, to get ownersip, remove the accepted socket from the pending
 * queue.
 */
                            tm_socket_checkout(socketPtr);
                            tfTcpRemoveConReq(tcpVectPtr,
                                              TM_TCP_ACCEPT_QUEUE,
                                              0);
                            foundConReq = TM_8BIT_YES;
                        }
/* End of Get an established non accepted yet socket Entry */
                        if (foundConReq != TM_8BIT_ZERO)
                        {
                            break; /* do while */
                        }
                        if (tm_16bit_one_bit_set(
                                listenSocketEntryPtr->socFlags,
                                TM_SOCF_NONBLOCKING))
                        {
                            errorCode = TM_EWOULDBLOCK;
                        }
                        else
                        {
                            errorCode = tm_pend(
                                &listenSocketEntryPtr->socRecvPendEntry,
                                &listenSocketEntryPtr->socLockEntry );
                        }
                    }
                    else
                    {
                        errorCode = TM_EPERM;
                    }
                }
                while (    (tcpVectPtr == listenTcpVectPtr)
                        && (errorCode == TM_ENOERROR) );
        
                if (foundConReq != TM_8BIT_ZERO)
                {
/*
 * Unlock the listening socket, to prevent possible deadlock
 * when we lock the socket array in tfSocketArrayInsert().
 */
                    tm_call_unlock(&(listenSocketEntryPtr->socLockEntry));
/* listening socket is unlocked */
                    index = tfSocketArrayInsert(socketPtr);
                    if ((unsigned int)index != TM_SOC_NO_INDEX)
                    {
                        if ( tm_16bit_one_bit_set(
                                               tcpVectPtr->tcpsFlags3,
                                               TM_TCPF3_LISTEN_RECV_WINDOW ) )
                        {
/* Use real recv window (if the user had modified the pre-accept one) */
                            socketPtr->socMaxRecvQueueBytes =
                                            tcpVectPtr->tcpsMaxRecvQueueBytes;
                        }
                        if (peerAddressPtr != TM_SOCKADDR_NULL_PTR)
                        {
                            peerAddressInPtr.sockPtr = peerAddressPtr;

#ifdef TM_USE_IPV6
                            if (socketPtr->socProtocolFamily == AF_INET6)
                            {
                                *addressLengthPtr = sizeof(struct sockaddr_in6);
/* put the address in the format that the user expects */
                                tf6SetSockAddrFromIpAddr(
                                    &(socketPtr->socPeerIpAddress),
                                    TM_DEV_NULL_PTR, peerAddressInPtr.sockIn6Ptr);

/* set the other parts of sockaddr_in6 that haven't been set yet */
                                peerAddressInPtr.sockIn6Ptr->sin6_port =      
                                    socketPtr->socPeerLayer4Port;
                            }
                            else
#endif /* TM_USE_IPV6 */
                            {
#ifdef TM_USE_IPV4
/* copy the peeer IP address and port */
                                *addressLengthPtr = sizeof(struct sockaddr_in);
                                peerAddressInPtr.sockInPtr->sin_len =
                                    sizeof(struct sockaddr_in);
                                peerAddressInPtr.sockInPtr->sin_family = AF_INET;
                                peerAddressInPtr.sockInPtr->sin_port =      
                                    socketPtr->socPeerLayer4Port;
                                tm_ip_copy(
                                    tm_4_ip_addr(socketPtr->socPeerIpAddress),
                                    peerAddressInPtr.sockInPtr->sin_addr.s_addr );
#endif /* TM_USE_IPV4 */
                            }
                        }
                    }
                    else
                    {
                        errorCode = TM_EMFILE;
/* No user to notify */
                        (void)tfTcpAbort(tcpVectPtr, 0);
                    }
/* unlock the accepted socket and check it in */
                    tm_socket_checkin_call_unlock(socketPtr);
/* Relock the listening socket (it is unlocked at the bottom) */
                    tm_call_lock_wait(&(listenSocketEntryPtr)->socLockEntry);
                }
                else
                {
                    if (errorCode == TM_ENOERROR)
                    {
                        errorCode = TM_EWOULDBLOCK;
                    }
                }
            }
            else
#endif /* TM_USE_TCP */
            {
                errorCode = TM_EOPNOTSUPP;
            }
        }
/*
 *      else  
 *          errorCode set above in tfSocketCheckAndLock 
 */    
/* Returns index if no error, otherwise returns errorCode */
        retCode = tfSocketReturn(listenSocketEntryPtr, listenSocketDescriptor,
                                 errorCode, index);
    }
    return retCode;
}
