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
 * Description: Treck Extentions to the BSD interface
 *
 * Filename: trsockex.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/trsockex.c $
 *
 * Modification History
 * $Revision: 6.0.2.4 $
 * $Date: 2010/08/26 04:21:17JST $
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
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
#include <trssl.h>
#endif /* (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER)) */

#ifdef TM_USE_MAPPED_CRYLIB
#if defined (TM_PUBKEY_USE_RSA) || defined (TM_PUBKEY_USE_DSA)
#include <trcrylib.h>
#include <tropnssl.h>
#include <trpki.h> 
#endif /* TM_PUBKEY_USE_RSA || TM_PUBKEY_USE_DSA */
#include <trcrymap.h>
#endif /* TM_USE_MAPPED_CRYLIB */

/*
 * Local macros
 */
/* Supress unreferenced macro error for PC-LINT */
#ifdef TM_LINT
LINT_UNREF_MACRO(TM_BASE_USER_PORT)
#endif /* TM_LINT */

/*
 * Tap points for the different vectors
 * are chosen to give the highest level of entropy
 * in generating a sequence of random numbers
 */
#define TM_RAND_V1T1 4
#define TM_RAND_V1T2 15
#define TM_RAND_V2T1 2
#define TM_RAND_V2T2 11

#define TM_BASE_USER_PORT IPPORT_RESERVED

/*
 * Local functions
 */

/* Make sure that seed is not 0, or 0xFFFF */
static unsigned short tfRandCheck(unsigned short seed);

#ifdef TM_USE_TCP
#ifdef TM_USE_STOP_TRECK
static int tfTcpTmWtCBClose (ttNodePtr nodePtr, ttGenericUnion genParm);
#endif /* TM_USE_STOP_TRECK */
#endif /* TM_USE_TCP */

#ifdef TM_USE_STOP_TRECK
static int tfSocketTreeCBDelete(ttSocketEntryPtr socketPtr, ttVoidPtr voidPtr);
#endif /* TM_USE_STOP_TRECK */

/*
 * Initialize 
 */
void tfSocketInit(tt32Bit maxSockets)
{
    unsigned int allocSize;
#if defined(TM_USE_TCP) || (!(defined(TM_USE_SOCKET_RB_TREE)) && defined(TM_USE_SOCKET_HASH_LIST))
    unsigned int maxHashIndex;
    unsigned int hashIndex; /* hash bucket index */
#endif /* TM_USE_TCP || (!TM_USE_SOCKET_RB_TREE && TM_USE_SOCKET_HASH_LIST) */
#if defined(TM_USE_SOCKET_HASH_LIST) || !defined(TM_USE_SOCKET_RB_TREE)
    int          i; /* socket table index */
#endif /* TM_USE_SOCKET_HASH_LIST || !TM_USE_SOCKET_RB_TREE */

/* Socket array (indexed by socket descriptors) */
    if (maxSockets >= (tt32Bit)TM_SOC_NO_INDEX)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelError("tfSocketInit",
                      "FATAL: cannot have more than TM_SOC_NO_INDEX sockets");
#endif
        tm_thread_stop;
    }
#ifdef TM_ERROR_CHECKING
    if (maxSockets > (tt32Bit)FD_SETSIZE)
    {
        tfKernelWarning("tfSocketInit",
    "WARNING: max number sockets exceed FD_SETSIZE, do not use FD_SET, or re-define FD_SETSIZE");
    }
#ifdef TM_USE_TCP
    if ((maxSockets - tm_context(tvMaxTcpVectAllocCount)) >= (tt32Bit)
                tm_context(tvSocketThresholdSequentialPort[TM_SOTB_NON_CON]))
    {
        tfKernelWarning("tfSocketInit",
    "WARNING: The threshold of concurrent non TCP connections when port randomization is turned off is lower than the maximum number of non TCP sockets");
    }
    if (tm_context(tvMaxTcpVectAllocCount) >= (tt32Bit)
                tm_context(tvSocketThresholdSequentialPort[TM_SOTB_TCP_CON]))
    {
        tfKernelWarning("tfSocketInit",
    "WARNING: The threshold of concurrent TCP connections when port randomization is turned off is lower than the maximum number of TCP sockets");
    }
#endif /* TM_USE_TCP */
#endif /* TM_ERROR_CHECKING */
#ifdef TM_USE_BSD_DOMAIN
    allocSize = (unsigned int)( maxSockets * sizeof(ttAFSocket) );
    tm_context(tvSocketArray) =
                    (ttAFSocketPtr)tm_kernel_malloc(allocSize);
    if (tm_context(tvSocketArray) == (ttAFSocketPtr)0)
#else /* !TM_USE_BSD_DOMAIN */
    allocSize = (unsigned int)( maxSockets * sizeof(ttSocketEntryPtr) );
    tm_context(tvSocketArray) =
                    (ttSocketEntryPtrPtr)tm_kernel_malloc(allocSize);
    if (tm_context(tvSocketArray) == TM_SOCKET_ENTRY_NULL_PTRPTR)
#endif /* !TM_USE_BSD_DOMAIN */
    {
#ifdef TM_ERROR_CHECKING
        tfKernelError("tfSocketInit","FATAL: Unable to malloc socket table");
#endif
        tm_thread_stop;
    }
    else
    {
/* Make sure that the socket entry pointers are null in socket array */
        tm_bzero((ttVoidPtr)tm_context(tvSocketArray), allocSize);
    }
    allocSize = (unsigned int)(maxSockets*sizeof(tt16Bit));
    tm_context(tvSocketErrorArray) = (tt16BitPtr)
                                         tm_kernel_malloc(allocSize);
    if (tm_context(tvSocketErrorArray) == (tt16BitPtr)0)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelError("tfSocketInit",
                      "FATAL: Unable to malloc socket error table");
#endif
        tm_thread_stop;
    }
    else
    {
/* Make sure that the socket entry pointers are null in socket array */
        tm_bzero((ttVoidPtr)tm_context(tvSocketErrorArray), allocSize);
    }
#ifdef TM_USE_SOCKET_HASH_LIST
/* Initalize socket hash tables and list(s) */
    for (i = 0; i < TM_SOTB_NUM; i++)
/* For each socket table */
    {
#ifdef TM_USE_SOCKET_RB_TREE
/* Allocate the hash table */
        allocSize = (unsigned int)(  sizeof(ttSocketEntryPtr)
                                   * tm_context(tvSocketTableListHashSize[i]));
        tm_context(tvSocketTableRootPtrPtr[i]) =
                (ttSocketEntryPtrPtr)(ttVoidPtr)tm_kernel_malloc(allocSize);
        if (tm_context(tvSocketTableRootPtrPtr[i]) != (ttSocketEntryPtrPtr)0)
        {
            tm_bzero(tm_context(tvSocketTableRootPtrPtr[i]), allocSize);
        }
#ifdef TM_ERROR_CHECKING
        else
        {
            tfKernelError("tfSocketTableInit",
                  "FATAL: Unable to malloc socket RB tree lookup hash table");
            tm_thread_stop;
        }
#endif /* TM_ERROR_CHECKING */
#else /* !TM_USE_SOCKET_RB_TREE */
/* Allocate the hash table */
        tm_context(tvSocketTableListPtr[i]) = (ttListPtr)(ttVoidPtr)
            tm_kernel_malloc(   sizeof(ttList)
                              * tm_context(tvSocketTableListHashSize[i]));
        if (tm_context(tvSocketTableListPtr[i]) != (ttListPtr)0)
        {
            maxHashIndex = tm_context(tvSocketTableListHashSize[i]);
            for (hashIndex = TM_UL(0); hashIndex < maxHashIndex; hashIndex ++)
/* Initialize hash bucket list */
            {
                tfListInit(&(tm_context(tvSocketTableListPtr[i][hashIndex])));
            }
        }
#ifdef TM_ERROR_CHECKING
        else
        {
            tfKernelError("tfSocketTableInit",
                      "FATAL: Unable to malloc socket list lookup hash table");
            tm_thread_stop;
        }
#endif /* TM_ERROR_CHECKING */
#endif /* !TM_USE_SOCKET_RB_TREE */
    }
#else /* !TM_USE_SOCKET_HASH_LIST */
#ifndef TM_USE_SOCKET_RB_TREE
    for (i = 0; i < TM_SOTB_NUM; i++)
    {
/* Initialize each socket table list */
        tfListInit(&tm_context(tvSocketTableList[i]));
    }
#endif /* !TM_USE_SOCKET_RB_TREE */
#endif /* !TM_USE_SOCKET_HASH_LIST */
#ifdef TM_USE_TCP
/* Time wait TCP vector hash table */
    tm_context(tvTcpTmWtTableListPtr) = (ttListPtr)(ttVoidPtr)
            tm_kernel_malloc(   sizeof(ttList)
                              * tm_context(tvTcpTmWtTableHashSize));
    if (tm_context(tvTcpTmWtTableListPtr) != (ttListPtr)0)
    {
        maxHashIndex = tm_context(tvTcpTmWtTableHashSize);
        for (hashIndex = TM_UL(0); hashIndex < maxHashIndex; hashIndex++)
/* Initialize hash bucket list */
        {
            tfListInit(&(tm_context(tvTcpTmWtTableListPtr[hashIndex])));
        }
    }
#endif /* TM_USE_TCP */
}

#ifdef TM_USE_STOP_TRECK
static int tfSocketTreeCBDelete(ttSocketEntryPtr socketPtr, ttVoidPtr voidPtr)
{
    TM_UNREF_IN_ARG(voidPtr);
    tm_call_lock_wait(&(socketPtr->socLockEntry));
#ifdef TM_USE_TCP
    if (((int)socketPtr->socProtocolNumber) == IP_PROTOTCP)
    {
        tfSocketTreeDelete(socketPtr,
                       TM_SOC_TREE_LOCKED_FLAG | TM_SOC_TREE_KEEP_LOCK_FLAG);
        socketPtr->socFlags2 |= TM_SOCF2_UNINITIALIZING;
        (void)tfTcpAbort((ttTcpVectPtr)socketPtr, TM_ESHUTDOWN);
    }
    else
#endif /* TM_USE_TCP */
    {
        tfKernelError("tfSocketTreeCBDelete",
                      "Non TCP socket still in the tree");
    }
    tm_call_unlock(&socketPtr->socLockEntry);
    return TM_ENOENT; /* keep walking */
}

TM_PROTO_EXTERN void tfSocketDeInit(
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                )
{
#ifdef TM_USE_TCP
    ttGenericUnion      genParm;
    ttListPtr           listPtr;
#endif /* TM_USE_TCP */
    ttSocketEntryPtr    socketPtr;
    unsigned int        sockIndex;
#ifdef TM_USE_TCP
    unsigned int        maxHashIndex; /* maximum hash bucket index */
    unsigned int        hashIndex; /* hash bucket index */
#endif /* TM_USE_TCP */
#ifdef TM_USE_SOCKET_HASH_LIST
    int                 i; /* socket table index */
#endif /* TM_USE_SOCKET_HASH_LIST */

/* Close all the sockets without delay */
#ifdef TM_USE_BSD_DOMAIN
    if (tm_context_var(tvSocketArray) != (ttAFSocketPtr)0)
#else /* !TM_USE_BSD_DOMAIN */
    if (tm_context_var(tvSocketArray) != TM_SOCKET_ENTRY_NULL_PTRPTR)
#endif /* !TM_USE_BSD_DOMAIN */
    {
        tm_call_lock_wait(&tm_context_var(tvSocketArrayLock));
        for ( sockIndex = 0;
              (tt32Bit)sockIndex < tm_context_var(tvMaxNumberSockets);
              sockIndex++ )
        {
#ifdef TM_USE_BSD_DOMAIN
            if ( (tm_context_var(tvSocketArray)[sockIndex].afsAF) !=
                                                         TM_BSD_FAMILY_INET )
            {
                socketPtr = TM_SOCKET_ENTRY_NULL_PTR;
            }
            else
            {
                socketPtr = (ttSocketEntryPtr)
                      (tm_context_var(tvSocketArray)[sockIndex].afsSocketPtr);
            }
#else /* !TM_USE_BSD_DOMAIN */
            socketPtr = tm_context_var(tvSocketArray)[sockIndex];
#endif /* !TM_USE_BSD_DOMAIN */
            if (socketPtr != TM_SOCKET_ENTRY_NULL_PTR)
            {
/* Immediate close */
                tm_call_lock_wait(&(socketPtr->socLockEntry));
                socketPtr->socOptions |= SO_LINGER;
                socketPtr->socFlags2 |= TM_SOCF2_UNINITIALIZING; 
                socketPtr->socLingerTime = TM_16BIT_ZERO;
                tm_call_unlock(&socketPtr->socLockEntry);
                tm_call_unlock(&tm_context_var(tvSocketArrayLock));
                tfClose(sockIndex);
                tm_call_lock_wait(&tm_context_var(tvSocketArrayLock));
            }
        }
        tm_call_unlock(&tm_context_var(tvSocketArrayLock));
        tm_kernel_free(tm_context_var(tvSocketArray));
/* Free socket array */
#ifdef TM_USE_BSD_DOMAIN
        tm_context_var(tvSocketArray) = (ttAFSocketPtr)0;
#else /* !TM_USE_BSD_DOMAIN */
        tm_context_var(tvSocketArray) = TM_SOCKET_ENTRY_NULL_PTRPTR;
#endif /* !TM_USE_BSD_DOMAIN */
    }
/* Free socket error array */
    if (tm_context_var(tvSocketErrorArray) != (tt16BitPtr)0)
    {
        tm_kernel_free(tm_context_var(tvSocketErrorArray));
        tm_context_var(tvSocketErrorArray) = (tt16BitPtr)0;
    }
/* Reset all sockets/TCP vectors */
    tfSocketWalk(tfSocketTreeCBDelete, (ttVoidPtr)0);
#ifdef TM_USE_SOCKET_HASH_LIST
/* Free socket tree/table hash lists */
    for (i = 0; i < TM_SOTB_NUM; i++)
/* For each socket table */
    {
#ifdef TM_USE_SOCKET_RB_TREE
/* Two socket tree root hash lists */
        if (tm_context_var(tvSocketTableRootPtrPtr[i]) !=
                                                       (ttSocketEntryPtrPtr)0)
        {
            tm_kernel_free(tm_context_var(tvSocketTableRootPtrPtr[i]));
            tm_context_var(tvSocketTableRootPtrPtr)[i] = (ttSocketEntryPtrPtr)0;
        }
#else /* !TM_USE_SOCKET_RB_TREE */
/* Two socket table hash lists */
        if (tm_context_var(tvSocketTableListPtr[i]) != (ttListPtr)0)
        {
            tm_kernel_free(tm_context_var(tvSocketTableListPtr[i]));
            tm_context_var(tvSocketTableListPtr[i]) = (ttListPtr)0;
        }
#endif /* !TM_USE_SOCKET_RB_TREE */
    }
#endif /* TM_USE_SOCKET_HASH_LIST */
#ifdef TM_USE_TCP
/* Time wait TCP vector hash table */
    tm_call_lock_wait(&tm_context_var(tvTcpTmWtTableLock));
    if (tm_context_var(tvTcpTmWtTableListPtr) != (ttListPtr)0)
    {
        maxHashIndex = tm_context_var(tvTcpTmWtTableHashSize);
        for (hashIndex = TM_UL(0); hashIndex < maxHashIndex; hashIndex++)
        {
            listPtr = &(tm_context_var(tvTcpTmWtTableListPtr)[hashIndex]);
            genParm.genIntParm = 0;
            tfListWalk(listPtr, tfTcpTmWtCBClose, genParm);
        }
        tm_kernel_free(tm_context_var(tvTcpTmWtTableListPtr));
    }
    tm_call_unlock(&tm_context_var(tvTcpTmWtTableLock));
#endif /* TM_USE_TCP */
}

#ifdef TM_USE_TCP
static int tfTcpTmWtCBClose (ttNodePtr nodePtr, ttGenericUnion genParm)
{
    TM_UNREF_IN_ARG(genParm);

    tfTcpTmWtVectClose((ttTcpTmWtVectPtr)nodePtr);
    return TM_8BIT_NO;
}
#endif /* TM_USE_TCP */
#endif /* TM_USE_STOP_TRECK */


void tfSocketErrorRecord (int socketDescriptor, int errorCode)
{
    if ((tt32Bit)socketDescriptor >= tm_context(tvMaxNumberSockets))
    {
        tm_context(tvErrorCode) = errorCode;
    }
    else
    {
        tm_context(tvSocketErrorArray)[(unsigned int)socketDescriptor] =
                                                         (tt16Bit)errorCode;
    }
}

/*
 * Primitive socket send routine used by sendto and send
 * for any supported protocol
 */
int tfSocketSend(ttSocketEntryPtr socketPtr,
                 ttPacketPtr      packetPtr, 
                 ttPktLen         bufferLength,
                 int              flags)
{
#ifdef TM_USE_TCP
    ttTcpVectPtr    tcpVectPtr;
#endif /* TM_USE_TCP */
    ttSharedDataPtr pktShrDataPtr;
    int             protocolNumber;
    int             errorCode;
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
#ifdef TM_USE_TCP
    ttPacketPtr     lastPacketPtr;
#endif /* TM_USE_TCP */
    int             sslFlags;
    tt8Bit          needQueue;
#endif /* (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER)) */

#ifndef TM_USE_TCP
/* Avoid compiler warning about unreferenced formal parameter */
    TM_UNREF_IN_ARG(flags);
#endif /* !TM_USE_TCP */
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
    sslFlags = 0;
#endif /* (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER)) */
    errorCode = TM_ENOERROR;
#ifdef TM_6_USE_MIP_RO
    if ((tm_context(tv6MipRoEnable) == TM_8BIT_NO)
#ifdef TM_6_USE_MIP_MN
        || (tm_16bit_one_bit_set(
                socketPtr->socOptions, TM_6_IPO_MN_USE_HA_TUNNEL))
#endif /* TM_6_USE_MIP_MN */
       )
    {
/* this socket does not support route optimization, or route optimization
   is globally disabled, so bypass route optimization for this packet */
        packetPtr->pktFlags |= TM_6_MIP_BYPASS_RO_FLAG;
    }
#endif /* TM_6_USE_MIP_RO */

#ifdef TM_USE_TCP
    tcpVectPtr = (ttTcpVectPtr)((ttVoidPtr)socketPtr);
#endif /* TM_USE_TCP */
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
    if (packetPtr->pktUserFlags & TM_PKTF_USER_OWNS)
    {
        sslFlags |= TM_SSLSF_PKT_USER_OWNS;
    }
#endif /* TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */
/* User no longer owns the zero copy buffer */
    tm_16bit_clr_bit( packetPtr->pktUserFlags,
                      TM_PKTF_USER_OWNS );
    protocolNumber = socketPtr->socProtocolNumber;
#if defined(TM_USE_QOS) && defined(TM_USE_IPV4)
    if (!packetPtr->pktIpTosSet)
    {
        packetPtr->pktIpTos = socketPtr->socIpTos;
        packetPtr->pktIpTosSet = 1;
    }
#endif /* TM_USE_QOS && TM_USE_IPV4 */
/*
 * Decrease the amount of space left in the send queue by the bytes
 * copied in.
 */
#ifdef TM_USE_TCP
    if (protocolNumber == IP_PROTOTCP)
    {
        if (flags & MSG_OOB)
        {
/* 
 * Let SSL know that there is an OOB message, in which case SSL has to process
 * the data immediately. 
 */
            packetPtr->pktFlags |= MSG_OOB;
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
            sslFlags |= TM_SSLSF_MSG_OOB;
#endif /* TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */
        }
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
        needQueue = TM_8BIT_YES;
        if ( tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                                  TM_SSLF_SEND_ENABLED)
           )
        {
            sslFlags |= TM_SSLSF_APPLICATION;
            errorCode = tfSslSend(socketPtr,
                                  tcpVectPtr->tcpsSslConnStatePtr,
                                  &packetPtr,
                                  &bufferLength,
                                  sslFlags);
/* 
 * The return of tfSslSend can be either:
 * TM_ENOERROR: we need to queue the data here.
 * TM_ENOBUFS: could not allocate a packet, packetPtr is null. Cannot queue.
 * TM_EWOULDBLOCK: data is queued, do not queue here (default).
 */
            if (errorCode != TM_ENOERROR)
            {
/* TM_EWOULDBLOCK, TM_ENOBUS */
                needQueue = TM_8BIT_NO;
                if (errorCode == TM_EWOULDBLOCK)
                {
/* packet is queued in SSL buffer for later transmission */
                    errorCode = TM_ENOERROR;
                }
            }
        }
        else if ( tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                                       TM_SSLF_HANDSHK_FAILURE)
                )
        {
/* if handshake failure happens, we should set this error to prevent sending */
            tfFreePacket(packetPtr, TM_SOCKET_LOCKED);
            needQueue = TM_8BIT_NO;
            errorCode = TM_ENOTCONN;
        }
        if (needQueue != TM_8BIT_NO)
#endif /* TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */

        {
/*
 * For TCP, a socket entry pointer should not be kept in the shared
 * data area to update the send queue when the packet is freed up,
 * since this work will be done instead by the TCP code when an ACK arrives.
 */
            socketPtr->socSendQueueBytes += (tt32Bit)bufferLength;
/* Queue the data */
            if (socketPtr->socSendQueueNextPtr == TM_PACKET_NULL_PTR)
            {
/* This is the only packet on the send queue */
                socketPtr->socSendQueueNextPtr = packetPtr;
            }
            else
            {
/* Put it at the end of the list */
/* The assumption is that the user only sends one link at a time */
                socketPtr->socSendQueueLastPtr->pktLinkNextPtr = 
                                                    (ttVoidPtr)packetPtr;
            }
            socketPtr->socSendQueueLastPtr = packetPtr;
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
            if(packetPtr->pktLinkNextPtr != (ttPacketPtr)0)
            {
                lastPacketPtr = packetPtr->pktLinkNextPtr;
                while(lastPacketPtr->pktLinkNextPtr != (ttPacketPtr)0)
                {
                    lastPacketPtr = lastPacketPtr->pktLinkNextPtr;
                }
                socketPtr->socSendQueueLastPtr = lastPacketPtr;
            }
#endif /* TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */
            if (flags & MSG_OOB)
            {
/* Update SND.UP to point to last byte of urgent data (RFC1122). */
                tcpVectPtr->tcpsSndUp = tcpVectPtr->tcpsSndUna +
                                    socketPtr->socSendQueueBytes - 1;
                tcpVectPtr->tcpsFlags2 |= TM_TCPF2_PROBE_OOB;
            }
            if (    (tcpVectPtr->tcpsState == TM_TCPS_ESTABLISHED)
                 || (tcpVectPtr->tcpsState == TM_TCPS_CLOSE_WAIT) )
            {
                errorCode = tfTcpSendPacket(tcpVectPtr);
            }
            else
            {
/* Just queue the packet for later transmission */
                errorCode = TM_ENOERROR; 
            }
        }
    }
    else /* UDP, ICMP, IGMP */
#endif /* TM_USE_TCP */
    {
        socketPtr->socSendQueueBytes += (tt32Bit)bufferLength;
        socketPtr->socSendQueueDgrams++;

/*
 * We don't queue for UDP or ICMP, rather we update the send queue bytes count
 * and initialize a pointer to the socket from the shared data.
 * When the shared data is freed up, the freeing code will update
 * the send queue bytes count.
 */
        pktShrDataPtr = packetPtr->pktSharedDataPtr;
/* checkout the socket entry because the packet->data has a copy of it */
        tm_socket_checkout(socketPtr);
        pktShrDataPtr->dataFlags |= TM_BUF_USER_SEND;
        pktShrDataPtr->dataSocketEntryPtr = socketPtr;
        if (protocolNumber == IP_PROTOUDP)
        {
            errorCode = tfUdpSendPacket(socketPtr, packetPtr);
        }
        else
        {
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
            if (socketPtr->socProtocolFamily == PF_INET6)
            {
#endif /* TM_USE_IPV4 */
/* IPv4 mapped addresses are not valid when using an IPV6 raw socket */
                if (!tm_6_addr_is_ipv4_mapped(
                        &socketPtr->socPeerIpAddress))
                {
                    errorCode = tf6RawSendPacket(socketPtr, packetPtr);
                }
                else
#ifdef TM_USE_IPV4
                {
                    errorCode = tfRawSendPacket(socketPtr, packetPtr);
                }
#else /* ! TM_USE_IPV4 */
                {
                    errorCode = TM_EAFNOSUPPORT;
                }
#endif /* ! TM_USE_IPV4 */
#ifdef TM_USE_IPV4
            }
            else
#endif /* TM_USE_IPV4 */
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
            {
                errorCode = tfRawSendPacket(socketPtr, packetPtr);
            }
#endif /* TM_USE_IPV4 */
        }
    }
    return (errorCode);
}

void tfSocketNotify(ttSocketEntryPtr socketPtr,
                    int              selectFlags,
                    int              socCBFlags,
                    int              errorCode)
{
    ttUserSocketCBFuncPtr       socketCBFuncPtr;
    ttUserSocketCBParamFuncPtr  socketCBParamFuncPtr;
    ttVoidPtr                   socketApiEntryPtr;
    int                         socketCBParamOn;
    int                         socketDescriptor;
    int                         cbFlags;

#ifdef TM_USE_TCP_REXMIT_TEST
    if (tm_context(tvSocketDisableRecv) == TM_8BIT_YES)
    {
        selectFlags &= ~TM_SELECT_READ;
        socCBFlags &= ~TM_CB_RECV;
    }
#endif /* TM_USE_TCP_REXMIT_TEST */

    if ( socketPtr->socIndex != TM_SOC_NO_INDEX )
    {
        if (errorCode != TM_ENOERROR)
        {
            if (selectFlags & TM_SELECT_READ)
            {
/* Read error */
                socketPtr->socRecvError = (tt16Bit)errorCode;
            }
            if (selectFlags & TM_SELECT_WRITE)
            {
/* Write error */
                socketPtr->socSendError = (tt16Bit)errorCode;
            }
        }
        cbFlags = socCBFlags & (int)socketPtr->socCBFlags;
        if (cbFlags != 0)
/* What the user registered for */
        {
            socketDescriptor = (int)socketPtr->socIndex;
            socketCBParamOn =
                        ((int)socketPtr->socFlags) & TM_SOCF_CB_PARAM_ON;
            if (socketCBParamOn == 0)
            {
                socketCBFuncPtr = socketPtr->socUserCBFuncPtr;
#ifdef TM_ERROR_CHECKING
                if (socketCBFuncPtr == (ttUserSocketCBFuncPtr)0)
                {
                    tfKernelError("tfSocketNotify",
                                  "Null CB function with non zero CB flags");
                }
#endif /* TM_ERROR_CHECKING */
/* for compiler */
                socketCBParamFuncPtr = (ttUserSocketCBParamFuncPtr)0;
                socketApiEntryPtr = (ttVoidPtr)0;
            }
            else
            {
                socketCBParamFuncPtr = socketPtr->socUserCBParamFuncPtr;
                socketApiEntryPtr = socketPtr->socApiEntryPtr;
#ifdef TM_ERROR_CHECKING
                if (socketCBParamFuncPtr == (ttUserSocketCBParamFuncPtr)0)
                {
                    tfKernelError("tfSocketNotify",
                             "Null CB param function with non zero CB flags");
                }
#endif /* TM_ERROR_CHECKING */
                socketCBFuncPtr = (ttUserSocketCBFuncPtr)0; /* for compiler */
            }
            tm_unlock(&socketPtr->socLockEntry);
            if (socketCBParamOn)
            {
#ifdef TM_LINT
LINT_NULL_PTR_BEGIN(socketCBParamFuncPtr)
#endif /* TM_LINT */
                (*socketCBParamFuncPtr)( socketDescriptor,
                                         socketApiEntryPtr,
                                         cbFlags );
#ifdef TM_LINT
LINT_NULL_PTR_END(socketCBParamFuncPtr)
#endif /* TM_LINT */
            }
            else
            {
#ifdef TM_LINT
LINT_NULL_PTR_BEGIN(socketCBFuncPtr)
#endif /* TM_LINT */
                (*socketCBFuncPtr)(socketDescriptor, cbFlags);
#ifdef TM_LINT
LINT_NULL_PTR_END(socketCBFuncPtr)
#endif /* TM_LINT */
            }
            tm_lock_wait(&socketPtr->socLockEntry);
        }
    }
#ifdef TM_PEND_POST_NEEDED
/*
 * check for select notification 
 */
    if (    (socketPtr->socIndex != TM_SOC_NO_INDEX)
         && (socketPtr->socSelectFlags & selectFlags)
         && (socketPtr->socSelectPendEntryPtr != TM_PEND_ENTRY_NULL_PTR) )
    {
/*
 * Post with no error, since a read error or write error registers as
 * a select read or write event
 */
        tm_post(socketPtr->socSelectPendEntryPtr, &socketPtr->socLockEntry, 0);
    }
/*
 * Check for read notification. We recheck the socIndex since socket was
 * possibly unlocked/locked
 */
#ifdef TM_USE_SSL
    if (socCBFlags & TM_CB_SSL_HANDSHK_PROCESS)
    {
/*
 * If user is pending in recv/zrecv, or send/zsend, post so that the handshake
 * is processed inside those APIs.
 */
        selectFlags |= (TM_SELECT_READ | TM_SELECT_WRITE);
    }
#endif /* TM_USE_SSL */
    if (   (socketPtr->socIndex != TM_SOC_NO_INDEX)
        && (selectFlags & TM_SELECT_READ) )
    {
/* Post on any non select pending read thread */
         tm_post(&socketPtr->socRecvPendEntry, &socketPtr->socLockEntry,
                 errorCode);
    }
/*
 * Check for write notification. We recheck the socIndex since socket was
 * possibly unlocked/locked
 */
    if (   (socketPtr->socIndex != TM_SOC_NO_INDEX)
        && (selectFlags & TM_SELECT_WRITE) )
    {
/* Post on any non select pending write thread */
        tm_post(&socketPtr->socSendPendEntry, &socketPtr->socLockEntry,
                errorCode);
    }
#else /* !TM_PEND_POST_NEEDED */
    TM_UNREF_IN_ARG(selectFlags);
#endif /* TM_PEND_POST_NEEDED */
}

/*
 * Process an incoming packet
 * Returns bytes queued or success or
 * TM_ERROR when socket is not found
 */
#ifdef TM_LINT
LINT_NULL_PTR_BEGIN(curLastPacketPtr)
#endif /* TM_LINT */
int tfSocketIncomingPacket(ttSocketEntryPtr  socketPtr,
                            ttPacketPtr      packetPtr)
{
#ifdef TM_USE_TCP
    ttTcpVectPtr            tcpVectPtr;
#endif /* TM_USE_TCP */
    ttSharedDataPtr         pktShrDataPtr;
    ttPacketPtr             lastPacketPtr;
    ttPacketPtr             curLastPacketPtr;
    ttPacketPtr             newPacketPtr;
#ifdef TM_ARP_UPDATE_ON_RECV
    ttRteEntryPtr           rtePtr;
#endif /* TM_ARP_UPDATE_ON_RECV */
    tt32Bit                 bytesAvailableInQueue;
    ttPktLen                blockSize;
#ifdef TM_USE_TCP
    ttPktLen                leftOverBytes;
#endif /* TM_USE_TCP */
    ttPktLen                bufferLength;
#ifdef TM_DSP
    unsigned int            destOffset;
#endif /* TM_DSP */
#ifdef TM_USE_IPV6
#ifdef TM_ARP_UPDATE_ON_RECV
    int                     match;
#endif /* TM_ARP_UPDATE_ON_RECV */
#endif /* TM_USE_IPV6 */
    int                     errorCode;
#ifdef TM_USE_TCP
    tt8Bit                  protocol;
#endif /* TM_USE_TCP */
    tt8Bit                  needFree;

#ifdef TM_USE_TCP
    tcpVectPtr = (ttTcpVectPtr)((ttVoidPtr)socketPtr);
    protocol = socketPtr->socProtocolNumber;
#endif /* TM_USE_TCP */
    needFree = TM_8BIT_ZERO;
    errorCode = TM_ENOERROR;

#if (defined(TM_6_USE_MIP_RO) && defined(TM_6_USE_MIP_MN))
/* When Mobile IPv6 Mobile Node functionality is enabled, when we receive a
   packet from a correspondent node on one of our registered home addresses
   that was not route optimized, if no route optimization BUL entry exists
   for that correspondent node we may attempt to create one.
   ([ISSUE158].R11.7.2:100, [ISSUE250].R11.3.5:15) */
/* if the socket supports route optimization: */
    if (tm_16bit_bits_not_set(
            socketPtr->socOptions, TM_6_IPO_MN_USE_HA_TUNNEL))
    {
/* check to see if we received this packet on one of our home addresses,
   and if so, then attempt to start route optimization with the peer */
        tf6MnStartRoIncomingPacket(packetPtr);
    }
#endif /* TM_6_USE_MIP_RO and TM_6_USE_MIP_MN */

/* Is the socket ready for data */
#ifdef TM_USE_TCP
    if (socketPtr->socProtocolNumber == IP_PROTOTCP)
/* TCP protocol */
    {
        bytesAvailableInQueue = tm_tcp_soc_recv_left(socketPtr, tcpVectPtr);
#ifdef TM_ERROR_CHECKING
        if (bytesAvailableInQueue < packetPtr->pktChainDataLength)
        {
/* This should not happen, since we trim the data in TCP */
            tfKernelError("tfSocketIncomingPacket",
                          "Data improperly trimmed in TCP");
        }
#endif /* TM_ERROR_CHECKING */
/* Clean up empty first link (due to retransmission, or reassembly, or FIN) */
        if (packetPtr->pktLinkDataLength == 0)
        {
            lastPacketPtr = (ttPacketPtr)packetPtr->pktLinkNextPtr;
            if (lastPacketPtr != TM_PACKET_NULL_PTR)
            {
                lastPacketPtr->pktChainDataLength =
                                                packetPtr->pktChainDataLength;
                tfFreeSharedBuffer(packetPtr, TM_SOCKET_LOCKED);
                packetPtr = lastPacketPtr;
            }
            else
            {
                needFree = TM_8BIT_YES;
/* Empty packet. Probably FIN */
                goto socketIncomingExit;
            }
        }
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
        if ( tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                                  TM_SSLF_RECV_ENABLED) )
        {
/* 
 * tfSsIncomingPacket will just append to the SSL incoming queue. 
 * It will return TM_EWOULDBLOCK if there is no data ready for the user,
 * TM_ENOERROR otherwise.
 */
            errorCode = tfSslIncomingPacket(tcpVectPtr, packetPtr);
            goto socketIncomingExit;
        }
#endif /* (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER)) */
    }
    else
#endif /* TM_USE_TCP */
/* Datagram protocol */
    {
        bytesAvailableInQueue = tm_soc_recv_left(socketPtr);
/* Check to make sure that we have room for the packet */
        if ((bytesAvailableInQueue < packetPtr->pktChainDataLength)
            || (socketPtr->socRecvQueueDgrams
                >= socketPtr->socMaxRecvQueueDgrams))
        { 
/* Datagram Protocol. */
            needFree = TM_8BIT_YES;
            errorCode = TM_ENOBUFS;
            goto socketIncomingExit;
        }

#ifdef TM_ARP_UPDATE_ON_RECV
/*
 * If this socket has an associated ARP entry, refresh the ARP cache entry.
 * This prevents the stack from sending ARP requests every 10 minutes
 * on an active ARP entry, i.e. an ARP entry for a destination IP address 
 * that the stack is actively sending to, which is unnecessary if
 * there is incoming traffic from the peer, and if this check is enabled. 
 * Non-TCP traffic (UDP, ICMP, etc) is covered here.  ARP cache entries
 * are renewed due to TCP traffic in tfTcpIncomingPacket.
 */
        rtePtr = socketPtr->socRteCacheStruct.rtcRtePtr;
        if (    (rtePtr != TM_RTE_NULL_PTR) 
             && tm_16bit_one_bit_set( rtePtr->rteFlags2, TM_RTE2_UP ))
        {
            if ( tm_16bit_one_bit_set (rtePtr->rteFlags, TM_RTE_INDIRECT) )
            {
                rtePtr = socketPtr->socRteCacheStruct.rtcRteClonedPtr;
            }
#ifdef TM_USE_IPDUAL
/*
 * In dual mode, the 6-4 tunnel default gateway does not point to any ARP
 * entry
 */
            if (rtePtr != TM_RTE_NULL_PTR)
#endif /* TM_USE_IPDUAL */
            {
                if (    tm_16bit_all_bits_set( rtePtr->rteFlags,
                                               (   TM_RTE_ARP | TM_RTE_CLONED 
                                                   | TM_RTE_LINK_LAYER) )
/* Don't modify static ARP entries */
                     && (    (rtePtr->rteTtl != TM_RTE_INF) 
#ifdef TM_USE_IPV6
                          || (rtePtr->rte6HSNudState != TM_6_NUD_IPV4_STATE)
#endif /* TM_USE_IPV6 */
                        )
                   )
                {
#ifdef TM_USE_IPV6
#ifdef TM_ARP_UPDATE_ON_RECV
                    tm_phys_addr_match(
                        packetPtr->pkt6PhysAddr,
                        rtePtr->rteHSPhysAddr,
                        rtePtr->rteHSPhysAddrLen,
                        match);
#endif /* TM_ARP_UPDATE_ON_RECV */
#endif /* TM_USE_IPV6 */
                
                    if (
/* 
 * Verify that the source Ethernet address of the incoming packet matches that
 * of the ARP cache entry.
 */
#ifdef TM_USE_IPV6
                        (match == rtePtr->rteHSPhysAddrLen)
#else /* ! TM_USE_IPV6 */
                        (tm_ether_match(rtePtr->rteHSEnetAdd,
                                        packetPtr->pktEthernetAddress))
#endif /* ! TM_USE_IPV6 */
                        )
                    {
/* Use a critical section to prevent the timer from removing the entry */
                        tm_kernel_set_critical;
#ifdef TM_USE_IPV6
                        if (rtePtr->rte6HSNudState == TM_6_NUD_IPV4_STATE)
                        {
#endif /* TM_USE_IPV6 */
                            rtePtr->rteTtl = tm_context(tvArpTimeout);
#ifdef TM_USE_IPV6
                        }
                        rtePtr->rteHSLastReachedTickCount = tvTime;
#endif /* TM_USE_IPV6 */
                        tm_kernel_release_critical;
                    }
                }
            }
        }
#endif /* TM_ARP_UPDATE_ON_RECV */

        socketPtr->socRecvQueueDgrams++;
    }
/* Update the queue Count */
    socketPtr->socRecvQueueBytes += (tt32Bit)packetPtr->pktChainDataLength;
/* Queue packet at the end */
    lastPacketPtr = packetPtr;
/* Queue the data in the receive queue */
    blockSize = 0; /* current buffer block size. Compute it below if needed */
#ifdef TM_USE_TCP
    if (protocol == IPPROTO_TCP)
    {
        if (packetPtr->pktChainDataLength != packetPtr->pktLinkDataLength)
        {
/* Walk the chain to find last packet */
            while (lastPacketPtr->pktLinkNextPtr != (ttVoidPtr)0)
            {
                lastPacketPtr = (ttPacketPtr)lastPacketPtr->pktLinkNextPtr;
            }
        }
    } /* tcp only */
#endif /* TM_USE_TCP */
    if (socketPtr->socReceiveQueueNextPtr == TM_PACKET_NULL_PTR)
    {
/* First Packet in the receive queue */
        socketPtr->socReceiveQueueNextPtr = packetPtr;
    }
    else
    {
        curLastPacketPtr = socketPtr->socReceiveQueueLastPtr;
#ifdef TM_ERROR_CHECKING
        if (curLastPacketPtr == TM_PACKET_NULL_PTR)
        {
            tfKernelError("tfSocketIncoming", "recv queue corrupted");
        }
#endif /* TM_ERROR_CHECKING */
/* current packet block size */
        if ( tm_16bit_bits_not_set(
                                curLastPacketPtr->pktSharedDataPtr->dataFlags,
                                TM_BUF_USER_DEVICE_DATA) )
        {
            pktShrDataPtr = packetPtr->pktSharedDataPtr;
            blockSize = (ttPktLen)
                         (tm_byte_count(   pktShrDataPtr->dataBufLastPtr 
                                         - pktShrDataPtr->dataBufFirstPtr ));
        }
#ifdef TM_USE_TCP
        if (protocol == IP_PROTOTCP)
        {
            if (blockSize != 0)
            {
                bufferLength = packetPtr->pktLinkDataLength;
/*
 * If packet utilization is below threshold, current packet is not
 * fragmented and TM_TCP_PACKET is not set. (Note that if packet utilization
 * is small, the packet is probably not fragmented, but we just double check).
 */
                if (((bufferLength * (socketPtr->socRecvCopyFraction))
                     <= blockSize )
                    && ( lastPacketPtr == packetPtr )
                    && tm_16bit_bits_not_set(
                        tcpVectPtr->tcpsFlags, TM_TCPF_PACKET))
                {
/* compute block size of previous packet */
                    leftOverBytes = (ttPktLen)
                        (tm_byte_count(
                            curLastPacketPtr->pktSharedDataPtr->dataBufLastPtr
                            - curLastPacketPtr->pktLinkDataPtr ));
                    leftOverBytes = (ttPktLen)
                        (   (leftOverBytes
                             - curLastPacketPtr->pktLinkDataLength)
#ifdef TM_DSP
                             - curLastPacketPtr->pktLinkDataByteOffset
#endif /* TM_DSP */
                            );
                    if (bufferLength > leftOverBytes)
                    {
/* partial copy */
                        bufferLength = leftOverBytes;
                    }
                    else
                    {
/* Our packet fits in */
                        needFree = TM_8BIT_YES;
                    }
                    if (bufferLength != 0)
                    {
#ifdef TM_DSP
                        destOffset = 
                            (curLastPacketPtr->pktLinkDataLength
                             + curLastPacketPtr->pktLinkDataByteOffset);
                        tfMemCopyOffset(
                            (int *) packetPtr->pktLinkDataPtr,
                            packetPtr->pktLinkDataByteOffset,
/* destination word pointer for copy */
                            (int *) (curLastPacketPtr->pktLinkDataPtr 
                                     + tm_packed_byte_len(destOffset)),
/* destination 8-bit byte offset for copy */
                            destOffset % TM_DSP_BYTES_PER_WORD,
                            bufferLength );
#else /* ! TM_DSP */
                        tm_bcopy(
                            packetPtr->pktLinkDataPtr,
                            curLastPacketPtr->pktLinkDataPtr 
                             + curLastPacketPtr->pktLinkDataLength,
                            (unsigned)bufferLength);
#endif /* ! TM_DSP */
                        curLastPacketPtr->pktLinkDataLength += bufferLength;
                        packetPtr->pktLinkDataLength -= bufferLength;
#ifdef TM_DSP
                        destOffset =  bufferLength
                                    + packetPtr->pktLinkDataByteOffset;
                        packetPtr->pktLinkDataPtr += 
                            (unsigned)tm_packed_byte_len(destOffset);
/* keep track of the 8-bit byte index (into the word) of the new start of
   data in this buffer */
                        packetPtr->pktLinkDataByteOffset =
                            (destOffset) % TM_DSP_BYTES_PER_WORD;
#else /* ! TM_DSP */
                        packetPtr->pktLinkDataPtr += bufferLength;
#endif /* ! TM_DSP */
                    }
                }
            }
            if (needFree == TM_8BIT_ZERO)
            {
/*
 * We are queueing the incoming buffer to the receive queue.
 * (For TCP we just use the link data pointer)
 */
                curLastPacketPtr->pktLinkNextPtr = (ttVoidPtr)packetPtr;
            }
            else
            {
/* Data has been copied */
                lastPacketPtr = curLastPacketPtr;
            }
        }
        else /* !TCP */
#endif /* TM_USE_TCP */
        {
/* 
 * Packet oriented protocol. Update the fields for this one link.
 * We need to preserve the datagram boundary so we use the
 * chain.
 */
            if (blockSize != 0)
            {
                bufferLength = packetPtr->pktChainDataLength;
/* If length is below fraction usage. */
                if (    (bufferLength * (socketPtr->socRecvCopyFraction))
                     <= blockSize )
                {
                    newPacketPtr = tfGetSharedBuffer(TM_MAX_SEND_HEADERS_SIZE,
                                                     bufferLength,
                                                     TM_16BIT_ZERO);
                    if (newPacketPtr != TM_PACKET_NULL_PTR)
                    {
                        needFree = TM_8BIT_YES;
                        tfCopyPacket(packetPtr, newPacketPtr);
                        lastPacketPtr = newPacketPtr;
                    }
                }
            }
            curLastPacketPtr->pktChainNextPtr = lastPacketPtr;
        }
    }
/* Update last packet in the receive queue */
    socketPtr->socReceiveQueueLastPtr = lastPacketPtr;
socketIncomingExit:
    if (needFree != TM_8BIT_ZERO)
    {
        tfFreePacket(packetPtr, TM_SOCKET_LOCKED);
    }
    return errorCode;
}
#ifdef TM_LINT
LINT_NULL_PTR_END(curLastPacketPtr)
#endif /* TM_LINT */

    
/*
 * Check to see if a socket is valid and the length of the sockaddr is
 * correct for the specified address family;  if everything is valid, then
 * lock it. If a NULL socket entry pointer is returned, the return code is
 * stored in *errorCodePtr.
 *
 * NOTE: Much of this code is duplicated in tfSocketCheckValidLock, which
 *       is used in cases where a sockaddr structure does not need to be
 *       validated.
 */
ttSocketEntryPtr tfSocketCheckAddrLenLock(
          int                      socketDescriptor,
    const struct sockaddr TM_FAR * addrPtr,
          int                      addrLen,
          int TM_FAR *             errorCodePtr
#ifdef TM_USE_BSD_DOMAIN
        , int TM_FAR *             afPtr
#endif /* TM_USE_BSD_DOMAIN */
        )
{
    ttSocketEntryPtr    socketPtr;
#ifdef TM_USE_BSD_DOMAIN
    ttAFSocketPtr       afSocketPtr; /* socket pointer with address family */
#endif /* TM_USE_BSD_DOMAIN */
    unsigned int        sockIndex;
#ifdef TM_USE_BSD_DOMAIN
    int                 af;
#endif /* TM_USE_BSD_DOMAIN */
    int                 errorCode;

    socketPtr = TM_SOCKET_ENTRY_NULL_PTR;
    errorCode = TM_ENOERROR;
    
/* PARAMETER CHECK */
    if ((tt32Bit)socketDescriptor >= tm_context(tvMaxNumberSockets))
    {
        errorCode = TM_EBADF;
        goto checkSocketFinish;
    }
    
/* LOCK the array to get the entry */
    tm_lock_wait(&tm_context(tvSocketArrayLock));
    sockIndex = (unsigned int)socketDescriptor;
#ifdef TM_USE_BSD_DOMAIN
    af = *afPtr;
    afSocketPtr = &(tm_context(tvSocketArray)[sockIndex]);
    *afPtr = afSocketPtr->afsAF;
    socketPtr = (ttSocketEntryPtr)(afSocketPtr->afsSocketPtr);
#else /* !TM_USE_BSD_DOMAIN */
    socketPtr = tm_context(tvSocketArray)[sockIndex];
#endif /* !TM_USE_BSD_DOMAIN */

/* Check the socket descriptor to see if it is valid */
    if ( (socketPtr == TM_SOCKET_ENTRY_NULL_PTR) )
    {
        errorCode = TM_EBADF;
        goto checkSocketUnlockFinish;
    }

#ifdef TM_USE_BSD_DOMAIN
    if (af & *afPtr) /* there is a match on the socket type of domain */
#endif /* TM_USE_BSD_DOMAIN */
    {
#ifdef TM_USE_BSD_DOMAIN
        if (*afPtr == TM_BSD_FAMILY_NONINET)
        {
/* Get a non inet socket lock */
            errorCode = tfBsdGetSocketLock((ttVoidPtr)socketPtr);
        }
        else
#endif /* TM_USE_BSD_DOMAIN */
        {
/* LOCK the socket entry */
            tm_lock_wait(&(socketPtr->socLockEntry));
            if (tm_16bit_bits_not_set(socketPtr->socFlags, TM_SOCF_OPEN))
            {
/* Socket in the process of being closed */
                tm_call_unlock(&socketPtr->socLockEntry);
                errorCode = TM_EBADF;
            }
            else
            {
                if (addrPtr != (const struct sockaddr TM_FAR *) 0)
                {
#ifdef TM_USE_IPV4
                    if (    (socketPtr->socProtocolFamily == AF_INET)
                         && (addrLen < (int)sizeof(struct sockaddr_in)) )
                    {
                        errorCode = TM_EINVAL;
                    }
#endif /* TM_USE_IPV4 */
    
#ifdef TM_USE_IPV6
                    if (    (socketPtr->socProtocolFamily == AF_INET6)
                            && (addrLen < (int)sizeof(struct sockaddr_in6)) )
                    {
                        errorCode = TM_EINVAL;
                    }
#endif /* TM_USE_IPV6 */
                }
    
                if (errorCode == TM_ENOERROR)
                {
                    tm_socket_checkout(socketPtr);
                }
                else
                {
                    tm_call_unlock(&socketPtr->socLockEntry);
                }
            }
        }
    }
#ifdef TM_USE_BSD_DOMAIN
    else
    {
/* 
 * Not allowed to get that type of domain socket. Set errorCode to 
 * TM_EOPNOTSUPP
 */
        errorCode = TM_EOPNOTSUPP;
    }
#endif /* TM_USE_BSD_DOMAIN */

checkSocketUnlockFinish:
/* UNLOCK the array */
    tm_unlock(&tm_context(tvSocketArrayLock));
checkSocketFinish:

    if (errorCode != TM_ENOERROR)
    {
        *errorCodePtr = errorCode;
        socketPtr = TM_SOCKET_ENTRY_NULL_PTR;
#ifdef TM_USE_BSD_DOMAIN
/* Use INET socket return */
        *afPtr = TM_BSD_FAMILY_INET;
#endif /* TM_USE_BSD_DOMAIN */
    }
    return (socketPtr);
}

/*
 * Check to see if a socket is valid, the address family matches the
 * socket type and the length of the sockaddr is correct for the specified
 * address family;  if everything is valid, then lock it.
 * If a NULL socket entry pointer is returned, the return code is stored in
 * *errorCodePtr.
 *
 * NOTE: Much of this code is duplicated in tfSocketCheckValidLock, which
 *       is used in cases where a sockaddr structure does not need to be
 *       validated.
 */
ttSocketEntryPtr tfSocketCheckAddrLock(
          int                      socketDescriptor,
    const struct sockaddr TM_FAR * addrPtr,
          int                      addrLen,
          int TM_FAR *             errorCodePtr
#ifdef TM_USE_BSD_DOMAIN
        , int TM_FAR *             afPtr
#endif /* TM_USE_BSD_DOMAIN */
        )
{
    ttSocketEntryPtr    socketPtr;
    unsigned int        sockIndex;
    int                 errorCode;
#ifdef TM_USE_IPV6
    struct sockaddr_in6 TM_FAR * ip6SockaddrPtr;
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_BSD_DOMAIN
    ttAFSocketPtr       afSocketPtr;
    int                 af;
#endif /* TM_USE_BSD_DOMAIN */

    socketPtr = TM_SOCKET_ENTRY_NULL_PTR;
    errorCode = TM_ENOERROR;
    
/* PARAMETER CHECK */
    if ((tt32Bit)socketDescriptor >= tm_context(tvMaxNumberSockets))
    {
        errorCode = TM_EBADF;
        goto checkSocketFinish;
    }
    sockIndex = (unsigned int)socketDescriptor;
/* LOCK the array to get the entry */
    tm_lock_wait(&tm_context(tvSocketArrayLock));
#ifdef TM_USE_BSD_DOMAIN
    af = *afPtr;
    afSocketPtr = &(tm_context(tvSocketArray)[sockIndex]);
    *afPtr = afSocketPtr->afsAF;
    socketPtr = (ttSocketEntryPtr)(afSocketPtr->afsSocketPtr);
#else /* !TM_USE_BSD_DOMAIN */
    socketPtr = tm_context(tvSocketArray)[sockIndex];
#endif /* !TM_USE_BSD_DOMAIN */

/* Check the socket descriptor to see if it is valid */
    if ( (socketPtr == TM_SOCKET_ENTRY_NULL_PTR) )
    {
        errorCode = TM_EBADF;
        goto checkSocketUnlockFinish;
    }

#ifdef TM_USE_BSD_DOMAIN
    if (af & *afPtr) /* there is a match on the socket type of domain */
#endif /* TM_USE_BSD_DOMAIN */
    {
#ifdef TM_USE_BSD_DOMAIN
        if (*afPtr == TM_BSD_FAMILY_NONINET)
        {
/* Get a non inet socket lock */
            errorCode = tfBsdGetSocketLock((ttVoidPtr)socketPtr);
        }
        else
#endif /* TM_USE_BSD_DOMAIN */
        {
/* LOCK the socket entry */
            tm_lock_wait(&(socketPtr->socLockEntry));
            if (tm_16bit_bits_not_set(socketPtr->socFlags, TM_SOCF_OPEN))
            {
/* Socket in the process of being closed */
                tm_call_unlock(&socketPtr->socLockEntry);
                errorCode = TM_EBADF;
            }
            else
            {
                if (addrPtr != (const struct sockaddr TM_FAR *) 0)
                {
/* Verify that the address type matches the protocol type of this socket. */
                    if (((tt8Bit) addrPtr->sa_family)
                        != socketPtr->socProtocolFamily)
                    {
                        errorCode = TM_EAFNOSUPPORT;
                    }
    
                    if (errorCode == TM_ENOERROR)
                    {
#ifdef TM_USE_IPV4
                        if (    (socketPtr->socProtocolFamily == AF_INET)
                             && (addrLen < (int)sizeof(struct sockaddr_in)) )
                        {
                            errorCode = TM_EINVAL;
                        }
#endif /* TM_USE_IPV4 */
    
#ifdef TM_USE_IPV6
                        if (socketPtr->socProtocolFamily == AF_INET6)
                        {
                            if (addrLen < (int)sizeof(struct sockaddr_in6))
                            {
                                errorCode = TM_EINVAL;
                            }
                            else
                            {
                                ip6SockaddrPtr =
                                        (struct sockaddr_in6 TM_FAR *)addrPtr;
#ifdef TM_USE_IPV4
/* Filter IPv4 packets for sockets that are flagged as IPv6 only */
                                if (    tm_16bit_one_bit_set(
                                                    socketPtr->soc6Flags,
                                                    TM_6_SOCF_V6ONLY)
                                    &&  IN6_IS_ADDR_V4MAPPED(
                                            &(ip6SockaddrPtr->sin6_addr)) )
                                {
                                    errorCode = TM_EAFNOSUPPORT;
                                }
#else /* !TM_USE_IPV4 */
/* IPv4-mapped IPv6 addresses are not supported in IPv6-only mode */
                                if (IN6_IS_ADDR_V4MAPPED(
                                            &(ip6SockaddrPtr->sin6_addr)))
                                {
/* IPv4 protocol is not available in IPv6-only mode */
                                    errorCode = TM_ENOPROTOOPT;
                                }
#endif /* !TM_USE_IPV4 */
                            }
                        }
#endif /* TM_USE_IPV6 */
                    }
                }
    
                if (errorCode == TM_ENOERROR)
                {
                    tm_socket_checkout(socketPtr);
                }
                else
                {
                    tm_call_unlock(&socketPtr->socLockEntry);
                }
            }
        }
    }
#ifdef TM_USE_BSD_DOMAIN
    else
    {
/* 
 * Not allowed to get that type of domain socket. Set errorCode to 
 * TM_EOPNOTSUPP
 */
        errorCode = TM_EOPNOTSUPP;
    }
#endif /* TM_USE_BSD_DOMAIN */
checkSocketUnlockFinish:
/* UNLOCK the array */
    tm_unlock(&tm_context(tvSocketArrayLock));
checkSocketFinish:

    if (errorCode != TM_ENOERROR)
    {
        *errorCodePtr = errorCode;
        socketPtr = TM_SOCKET_ENTRY_NULL_PTR;
#ifdef TM_USE_BSD_DOMAIN
/* Use INET socket return */
        *afPtr = TM_BSD_FAMILY_INET;
#endif /* TM_USE_BSD_DOMAIN */
    }
    return (socketPtr);
}

    
/*
 * Check to see if a socket is valid, if so then lock it
 *
 * NOTE: Much of this code is duplicated in tfSocketCheckAndLock, which
 *       is used in cases where a sockaddr structure does need to be
 *       validated.
 */
ttSocketEntryPtr tfSocketCheckValidLock(  int socketDescriptor
#ifdef TM_USE_BSD_DOMAIN
                                        , int TM_FAR * afPtr
#endif /* TM_USE_BSD_DOMAIN */
                                       )
{
    ttSocketEntryPtr socketPtr;
#ifdef TM_USE_BSD_DOMAIN
    ttAFSocketPtr    afSocketPtr; /* socket pointer with address family */
#endif /* TM_USE_BSD_DOMAIN */
    unsigned int        sockIndex;
#ifdef TM_USE_BSD_DOMAIN
    int              errorCode;
#endif /* TM_USE_BSD_DOMAIN */

/* PARAMETER CHECK */
    if ((tt32Bit)socketDescriptor >= tm_context(tvMaxNumberSockets))
    {
/* Use INET socket return */
#ifdef TM_USE_BSD_DOMAIN
        *afPtr = TM_BSD_FAMILY_INET;
#endif /* TM_USE_BSD_DOMAIN */
        socketPtr = TM_SOCKET_ENTRY_NULL_PTR;
        goto checkSocketFinish;
    }
    sockIndex = (unsigned int)socketDescriptor;
/* LOCK the array to get the entry */
    tm_lock_wait(&tm_context(tvSocketArrayLock));
#ifdef TM_USE_BSD_DOMAIN
    afSocketPtr = &(tm_context(tvSocketArray)[sockIndex]);
    if ( ((*afPtr) & (afSocketPtr->afsAF)) != 0 )
    {
        socketPtr = (ttSocketEntryPtr)(afSocketPtr->afsSocketPtr);
    }
    else
    {
        socketPtr = TM_SOCKET_ENTRY_NULL_PTR;
    }
    *afPtr = afSocketPtr->afsAF;
#else /* !TM_USE_BSD_DOMAIN */
    socketPtr = tm_context(tvSocketArray)[sockIndex];
#endif /* !TM_USE_BSD_DOMAIN */

/* Check the socket descriptor to see if it is valid */
    if (socketPtr == TM_SOCKET_ENTRY_NULL_PTR)
    {
/* Do not change *afPtr */
        goto checkSocketUnlockFinish;
    }

/* LOCK the socket entry */
#ifdef TM_USE_BSD_DOMAIN
    if (*afPtr == TM_BSD_FAMILY_NONINET)
    {
        errorCode = tfBsdGetSocketLock((ttVoidPtr)socketPtr);
        if (errorCode != TM_ENOERROR)
        {
            socketPtr = TM_SOCKET_ENTRY_NULL_PTR;
/* Use INET socket return */
            *afPtr = TM_BSD_FAMILY_INET;
        }
    }
    else
#endif /* TM_USE_BSD_DOMAIN */
    {
        tm_lock_wait(&(socketPtr->socLockEntry));
        if (tm_16bit_bits_not_set(socketPtr->socFlags, TM_SOCF_OPEN))
        {
/* Socket in the process of being closed */
            tm_call_unlock(&socketPtr->socLockEntry);
            socketPtr = TM_SOCKET_ENTRY_NULL_PTR;
        }
        else
        {
            tm_socket_checkout(socketPtr);
        }
    }
checkSocketUnlockFinish:
/* UNLOCK the array */
    tm_unlock(&tm_context(tvSocketArrayLock));
checkSocketFinish:
    return (socketPtr);
}

int tfSocketCopyDest (  ttSocketEntryPtr                socketPtr
                      , const struct sockaddr TM_FAR *  toAddressPtr
#ifdef TM_4_USE_SCOPE_ID
                      , ttPacketPtr                     packetPtr
#endif /* TM_4_USE_SCOPE_ID */
                     )
{
    ttSockAddrPtrUnion  toSockAddr;
    ttSockTuplePtr      sockTuplePtr;
    int                 errorCode;

    toSockAddr.sockPtr = (struct sockaddr TM_FAR *)toAddressPtr; 
    sockTuplePtr = &(socketPtr->socTuple);
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
    if (toSockAddr.sockPtr->sa_family == AF_INET6)
#endif /* TM_USE_IPV4 */
    {
        tm_6_ip_copy_structs(
                toSockAddr.sockIn6Ptr->sin6_addr,
                sockTuplePtr->sotRemoteIpAddress);
#ifdef TM_4_USE_SCOPE_ID
        if (tm_6_addr_is_ipv4_mapped(&sockTuplePtr->sotRemoteIpAddress))
        {
            tm_4_addr_add_scope(
                    &sockTuplePtr->sotRemoteIpAddress,
                    toSockAddr.sockIn6Ptr->sin6_scope_id);
        }
        else
#endif /* TM_4_USE_SCOPE_ID */
        {
            tm_6_addr_add_scope(&sockTuplePtr->sotRemoteIpAddress,
                                toSockAddr.sockIn6Ptr->sin6_scope_id);
        }
    }
#ifdef TM_USE_IPV4
    else
    {
        tm_6_addr_to_ipv4_mapped(
                (tt4IpAddress)(toSockAddr.sockInPtr->sin_addr.s_addr),
                &sockTuplePtr->sotRemoteIpAddress);
#ifdef TM_4_USE_SCOPE_ID
        if (    (packetPtr != (ttPacketPtr)0)
             && tm_16bit_one_bit_set(packetPtr->pktFlags,
                                     TM_OUTPUT_DEVICE_FLAG))
        {
/* we know the output device, so use it to embed the scope ID if the IPv4
   address is link-local scope */
            tm_4_addr_add_scope(
                &(sockTuplePtr->sotRemoteIpAddress),
                packetPtr->pktDeviceEntryPtr->devIndex);
        }
#endif /* TM_4_USE_SCOPE_ID */
    }
#endif /* TM_USE_IPV4 */
#else /* ! TM_USE_IPV6 */
    tm_ip_copy((tt4IpAddress)(toSockAddr.sockInPtr->sin_addr.s_addr),
                sockTuplePtr->sotRemoteIpAddress);
#endif /* ! TM_USE_IPV6 */
    sockTuplePtr->sotRemoteIpPort=(ttIpPort)
                   (toSockAddr.sockInPtr->sin_port);
    if (tm_16bit_bits_not_set(socketPtr->socFlags,
                              TM_SOCF_IN_TREE))
    {
        errorCode = tfSocketTreeReInsert(socketPtr,
                                         sockTuplePtr,
                                         (tt16Bit)(  socketPtr->socCoFlags
                                                   | TM_SOC_TREE_RANDOM_PORT));
        if (errorCode != TM_SOC_RB_OKAY)
        {
            errorCode = TM_EMFILE;
        }
    }
    else
    {
        errorCode = TM_ENOERROR;
    }
    return errorCode;
}

/*
 * Common exit code for socket function calls.
 * 1. Checkin and unlock socket if socket entry pointer non null
 * 2. If errorCode non zero, register it in socket user error number,
 *    and return TM_SOCKET_ERROR, so that the user can check it,
 *    otherwise return retCode parameter to the user.
 */
int tfSocketReturn(ttSocketEntryPtr socketPtr,
                   int              socketDescriptor,
                   int              errorCode,
                   int              retCode)
{
    if (errorCode != TM_ENOERROR)
    {
/* Record the socket error for socketDescriptor */
        tfSocketErrorRecord(socketDescriptor, errorCode);
        retCode = TM_SOCKET_ERROR;
    }
    if (socketPtr != TM_SOCKET_ENTRY_NULL_PTR)
    {
        tm_socket_checkin_unlock(socketPtr);
    }
    return (retCode);
}

/****************************************************************************
* FUNCTION: tfSocketReturnAddr
*
* PURPOSE: Common exit code for socket function calls that return our local
*   address.
*       1. Checkin and unlock socket if socket entry pointer non null
*       2. If errorCode non zero, register it in socket user error number,
*          and return TM_SOCKET_ERROR, so that the user can check it,
*          otherwise return retCode parameter to the user.
*
* PARAMETERS:
*   None
*
* RETURNS:
*   Nothing
*
* NOTES:
*
****************************************************************************/
int tfSocketReturnAddr(
    ttSocketEntryPtr         socketPtr,
    int                      socketDescriptor,
    struct sockaddr TM_FAR * myAddressPtr,
    int                      errorCode,
    int                      retCode,
    tt8Bit                   flag)
{
    ttSockAddrPtrUnion  internetAddressPtrU;
#ifdef TM_USE_IPV6
#ifndef TM_SINGLE_INTERFACE
    u_long             tempUlong;
#endif /* TM_SINGLE_INTERFACE */
    tt6IpAddressPtr     ip6AddrPtr;
    struct sockaddr_in6 TM_FAR * sock6AddrPtr;
#endif /* TM_USE_IPV6 */

    internetAddressPtrU.sockPtr = myAddressPtr;

    if (errorCode == TM_ENOERROR)
    {
        if (socketPtr != (ttSocketEntryPtr)0)
        {
#ifdef TM_USE_IPV6
            if (socketPtr->socProtocolFamily == AF_INET6)
            {
/* populate the sockaddr_in6 from the information that is stored on the
   socket. */
                sock6AddrPtr = internetAddressPtrU.sockIn6Ptr;
                tm_bzero((ttVoidPtr) sock6AddrPtr, sizeof(struct sockaddr_in6));
                if (flag == TM_ADDR_PEER_FLAG)
                {
                    ip6AddrPtr = &(socketPtr->socPeerIpAddress);
                    internetAddressPtrU.sockInPtr->sin_port = 
                                            socketPtr->socPeerLayer4Port;
                }
                else
                {
                    ip6AddrPtr = &(socketPtr->socOurIfIpAddress);
                    internetAddressPtrU.sockInPtr->sin_port = 
                                            socketPtr->socOurLayer4Port;
                }
                tm_6_ip_copy_dststruct(
                    ip6AddrPtr, sock6AddrPtr->sin6_addr);
                sock6AddrPtr->sin6_len = sizeof(struct sockaddr_in6);

                sock6AddrPtr->sin6_family = AF_INET6;
#ifndef TM_SINGLE_INTERFACE
                if (IN6_IS_ADDR_LINKLOCAL(ip6AddrPtr))
                {
/* set the scope ID from the IPv6 address. NOTE: the IPv6 address does not
   always have an embedded scope ID. For example, IPv6 addresses stored on
   the interface are stored without the embedded scope ID, to make it easier
   to compare them against the destination address of a received packet in
   the receive path. In this case, a pointer to the incoming interface
   must be passed to this function, so that the scope ID can be obtained
   from the interface. */
                    tm_6_get_embedded_scope_id(ip6AddrPtr, tempUlong);
                    sock6AddrPtr->sin6_scope_id = tempUlong;

/* removed the embedded scope ID in the local scope address we return to the
   user */
                    tm_6_embed_scope_id(
                        &(sock6AddrPtr->sin6_addr), TM_UL(0));
                }
#ifdef TM_4_USE_SCOPE_ID
                else if (
                    tm_6_addr_is_ipv4_mapped(ip6AddrPtr)
                    && IN4_IS_ADDR_LINKLOCAL(tm_6_ip_mapped_6to4(ip6AddrPtr)))
                {
                    tm_4_get_embedded_scope_id(ip6AddrPtr, tempUlong);
                    sock6AddrPtr->sin6_scope_id = tempUlong;
                }
#endif /* TM_4_USE_SCOPE_ID */
#endif /* !TM_SINGLE_INTERFACE */
            }
#ifdef TM_USE_IPV4
            else
#endif /* dual */
#endif /* TM_USE_IPV6 */
            {
/* AF_INET */
#ifdef TM_USE_IPV4
                internetAddressPtrU.sockInPtr->sin_len = 
                                                    sizeof(struct sockaddr_in);
                internetAddressPtrU.sockInPtr->sin_family = AF_INET;

/* Load up the saddr */
                if (flag == TM_ADDR_PEER_FLAG)
                {
                    tm_ip_copy(tm_4_ip_addr(socketPtr->socPeerIpAddress),
                           internetAddressPtrU.sockInPtr->sin_addr.s_addr);
                    internetAddressPtrU.sockInPtr->sin_port = 
                                            socketPtr->socPeerLayer4Port;
                }
                else
                {
                    internetAddressPtrU.sockInPtr->sin_port = 
                                            socketPtr->socOurLayer4Port;
                    tm_ip_copy(tm_4_ip_addr(socketPtr->socOurIfIpAddress),
                            internetAddressPtrU.sockInPtr->sin_addr.s_addr);
                }
#endif /* TM_USE_IPV4 */
            }
        }
    }
    else /* errorCode != TM_ENOERROR */
    {
/* Record the socket error for socketDescriptor */
        tfSocketErrorRecord(socketDescriptor, errorCode);
        retCode = TM_SOCKET_ERROR;
    }
    if (socketPtr != TM_SOCKET_ENTRY_NULL_PTR)
    {
        tm_socket_checkin_unlock(socketPtr);
    }
    return (retCode);
}

/* Insert a locked socket entry into the socket array */
int tfSocketArrayInsert(ttSocketEntryPtr socketPtr)
{
    unsigned int    sockIndex;
    unsigned int    firstIndex;
    int             numberSearches;
    tt32Bit         lastIndex;

/* 
 * Unlock the socket to prevent deadlock with the tvSocketArrayLock. 
 * 1. If tfSocketArrayInsert is called from socket(), then this is safe, as
 *    nobody can access that socketPtr yet.
 * 2. If tfSocketArrayInsert is called from accept(), then this is safe, as
 *    we remove the connection request from the listening socket connection
 *    request queue (i.e. tfSocketArrayInsert cannot be called twice), and 
 *    we increase the owner count in accept() (i.e. socket cannot be freed). 
 *    Also tfSocketArrayDelete() could not be called, since that could only
 *    happen if the user issues a tfClose and the user has no access to this
 *    socket yet, since it has not been inserted in the socket array yet.
 */
    tm_call_unlock(&socketPtr->socLockEntry);
/* LOCK the socket array */
    tm_call_lock_wait(&tm_context(tvSocketArrayLock));

/* Find the first empty slot. Search from last used slot. */
    firstIndex = (unsigned int)tm_context(tvSocketSearchIndex);
    lastIndex = tm_context(tvMaxNumberSockets);
    numberSearches = 0;
    do
    {
        for ( sockIndex = firstIndex ;
              (    ( (tt32Bit)sockIndex < lastIndex )
#ifdef TM_USE_BSD_DOMAIN
                && (    tm_context(tvSocketArray)[sockIndex].afsSocketPtr
                     != (ttVoidPtr)0 )
#else /* !TM_USE_BSD_DOMAIN */
                && (    tm_context(tvSocketArray)[sockIndex]
                     != TM_SOCKET_ENTRY_NULL_PTR )
#endif /* !TM_USE_BSD_DOMAIN */
              ) ;
              sockIndex++ )
        {
            ;
        }
        if ((tt32Bit)sockIndex != lastIndex)
        {
/* Found an empty slot. */
            break;
        }
/* Wrap around situation. Search from beginning of the array. */
        firstIndex = 0;
        lastIndex = tm_context(tvSocketSearchIndex);
        numberSearches++;
    }
    while (numberSearches < 2);

/* Relock the socket (unlocked prior to locking the socket array) */
    tm_call_lock_wait(&socketPtr->socLockEntry);
    if ((tt32Bit)sockIndex != lastIndex)
    {
/* Start next search at sockIndex + 1 */
        tm_context(tvSocketSearchIndex) = (tt32Bit)((tt32Bit)sockIndex + 1);
        if (tm_context(tvSocketSearchIndex) == tm_context(tvMaxNumberSockets))
/* Wrap around */
        {
            tm_context(tvSocketSearchIndex) = TM_UL(0);
        }
#ifdef TM_USE_BSD_DOMAIN
        tm_context(tvSocketArray)[sockIndex].afsSocketPtr = 
                                                    (ttVoidPtr)socketPtr;
        tm_context(tvSocketArray)[sockIndex].afsAF = TM_BSD_FAMILY_INET;
#else /* !TM_USE_BSD_DOMAIN */
        tm_context(tvSocketArray)[sockIndex] = socketPtr;
#endif /* !TM_USE_BSD_DOMAIN */
        socketPtr->socIndex = sockIndex;
/* Socket is opened */
        socketPtr->socFlags |= TM_SOCF_OPEN;
/* Increase ownership for array insertion */
        socketPtr->socOwnerCount++;
    }
    else
    {
        sockIndex = TM_SOC_NO_INDEX;
    }
/* UNLOCK the socket array */
    tm_call_unlock(&tm_context(tvSocketArrayLock));
    return (int)sockIndex;
}
  
/* Remove socket entry from the array */
void tfSocketArrayDelete (ttSocketEntryPtr socketPtr)
{
   unsigned int  sockIndex;
/*
 * Mark it as deleted to prevent critical windows, since we are
 * unlocking the entry below to prevent deadlock with array lock
 */
    if (tm_16bit_one_bit_set(socketPtr->socFlags, TM_SOCF_OPEN))
    {
        tm_16bit_clr_bit( socketPtr->socFlags,
                          (TM_SOCF_OPEN|TM_SOCF_CLOSING) );
        sockIndex = socketPtr->socIndex;
/* Reset the socIndex (socket is closed to the user) */
        socketPtr->socIndex = TM_SOC_NO_INDEX;
/* Unlock socket entry to prevent deadlock with the array */
        tm_call_unlock(&socketPtr->socLockEntry);
/* Lock the array */
        tm_call_lock_wait(&tm_context(tvSocketArrayLock));
        if (tm_context(tvSocketArray) != TM_SOCKET_ENTRY_NULL_PTRPTR)
        {
/* Reset the socket array slot (socket is closed to the user) */
#ifdef TM_USE_BSD_DOMAIN
            tm_context(tvSocketArray)[sockIndex].afsSocketPtr = (ttVoidPtr)0;
            tm_context(tvSocketArray)[sockIndex].afsAF = 0;
#else /* !TM_USE_BSD_DOMAIN */
            tm_context(tvSocketArray)[sockIndex] = TM_SOCKET_ENTRY_NULL_PTR;
#endif /* !TM_USE_BSD_DOMAIN */
        }
/* UNLOCK the array */
        tm_call_unlock(&tm_context(tvSocketArrayLock));
        tfSocketDecreaseOpenSockets();
/* Re-lock the socketentry */
        tm_call_lock_wait(&socketPtr->socLockEntry);
/* Reset routing entry cache */
        tfRtCacheUnGet(&socketPtr->socRteCacheStruct);
/* for array ownership */
        socketPtr->socOwnerCount--;
    }
}

/* 
 * Seed the Random Port Number Generator
 */
void tfRandSeed (unsigned short seed1, unsigned short seed2)
{
/* Seed with tvTime */
    tfRandAdd(&seed1,
              &seed2,
              (tt16Bit)(tvTime & 0xFFFF),
              (tt16Bit)((tvTime >> 16) & 0xFFFF));
    seed1 = tfRandCheck(seed1);
    seed2 = tfRandCheck(seed2);
    tm_context(tvVector1) = seed1;
    tm_context(tvVector2) = seed2;
}


/*
 * Generate a random 32Bit Number
 */
tt32Bit tfGetRandom (void)
{
    tt32Bit result;

#ifdef TM_USE_KERNEL_RNG
    result = (tt32Bit) tfKernelGetRandom();
#elif (defined(TM_USE_MAPPED_CRYLIB))
    tfGetRandomBytes((tt8BitPtr)&result, sizeof(tt32Bit));
#else
    tfRandNew(); /* Generate new values for tvVector1, and tvVector2 */
    result = (tt32Bit)tm_context(tvVector1) & 0xFFFF;
    result |= ((tt32Bit)tm_context(tvVector2) & 0xFFFF) << 16;
#endif /* TM_USE_KERNEL_RNG */

    return (result);
}

/* Generate a non zero random 8bit byte */
tt8Bit tfGetRandomByte(void)
{
#ifndef TM_DSP
    union
    {
        tt32Bit  ran32Bit;
        tt8Bit   ranByte[4];
    }               random32Bit;
    int     i;
#else /* TM_DSP */
    tt32Bit random32Bit;
#endif /* TM_DSP */
    tt8Bit  randomByte;

#ifndef TM_DSP
    random32Bit.ran32Bit = tfGetRandom();
    i = sizeof(tt32Bit) - 1;
    do
    {
        randomByte = random32Bit.ranByte[i];
        if (randomByte != TM_8BIT_ZERO)
        {
            break;
        }
        i--;
    } while (i >= 0);
#else /* TM_DSP */
    random32Bit = tfGetRandom();
    randomByte = (tt8Bit)(random32Bit & 0xFF);
    if (randomByte == 0)
    {
        randomByte = 1;
    }
#endif /* TM_DSP */
    return randomByte;
}

/* Generate a random signed 32bit value given the ceiling. For example,
 * if ceiling is 10, then this function will generate random value between 
 * -10 and 10
 */
ttS32Bit tfGetRandomDitter(ttS32Bit   ceiling)
{
    ttS32Bit retVal;

    if (ceiling != (ttS32Bit)0)
    {
        retVal = (ttS32Bit)tfGetRandom();
        retVal = ((ttS32Bit)retVal) % ceiling;
    }
    else
    {
        retVal = (ttS32Bit)0;
    }
    return retVal;
}

/* Generate random byte stream*/
void tfGetRandomBytes(tt8BitPtr  rPtr, int size)
{
#ifdef TM_USE_MAPPED_CRYLIB
    tfMapGetRandomBytes(rPtr, size);
#else /* TM_USE_MAPPED_CRYLIB */
    tt32Bit     random;
    unsigned    copyBytes;
    
    while (size > 0)
    {
        random = tfGetRandom();
        copyBytes = tm_min(sizeof(tt32Bit), (unsigned)size);
        tm_bcopy(&random, rPtr, copyBytes);
        rPtr += copyBytes;
        size -= (int)copyBytes;
    }
#endif /* TM_USE_MAPPED_CRYLIB */
}

/*
 * tfSocketResetCaches function description:
 * Called by the router code with TM_RTE_INDIRECT, or TM_RTE_ARP rteFlag,
 * and a null rteEntryPtr parameter when the all the socket single routing
 * caches need to be reset, because a new route has been added
 * (TM_RTE_INDIRECT), or because there is no more room in the ARP cache
 * (TM_RTE_ARP).
 * lockFlag can have two possible values TM_LOC_WAIT or TM_LOC_NOWAIT.
 *
 * TM_LOC_WAIT
 *   If lockFlag is TM_LOCK_WAIT, then this function is called with the 
 *   routing tree unlocked, and not in the send/forward path. It is safe
 *   to lock the socket array and each socket.
 * TM_LOC_NOWAIT
 *   If lockFlag is TM_LOC_NOWAIT, then this function is called with the
 *   routing tree locked. This is also the case if this function is
 *   called in the send/forward path. If the routing tree is locked
 *   it is unsafe to lock the socket array or a socket if it is already locked.
 *   Also in the send/forward path, an outgoing routing cache (socket) could
 *   already be locked, resulting in a deadlock if we attempt to lock the
 *   same lock again.
 *   To prevent deadlocks, if lockFlag is set to TM_LOC_NOWAIT,
 *   we will not attempt to reset any socket entry routing cache if the 
 *   tvSocketArray is already locked. We will not attempt to reset a socket 
 *   entry routing cache, if that socket 
 *   entry is already locked.
 * Note on tvSocketArray lock:
 *  We cannot wait on the tvSocketArray lock, because 
 *   1. This function could be called with the routing tree locked.
 *   2. another application task could have locked the tvSocketArrayLock, and
 *      could be waiting on a socket lock.
 *   3. Another task (recv task, or application sending task) could have 
 *      locked the same socket lock, and then would be waiting on the routing
 *      tree lock.
 *  This would result in a deadlock if we were to wait on the socket array lock.
 * Note on socket entry Lock
 *  If lockFlag is TM_LOC_NOWAIT, we cannot wait on a socket entry lock for
 *  two different reasons:
 *  1. this function can be called from the send/forward path, with an outgoing 
 *  routing cache locked (socket or IP send, or IP forward), resulting in a 
 *  deadlock if we attempt to lock the same lock again. 
 *  2. this function could be called from the recv task with the routing
 *  tree locked, and an application send task could also lock a socket entry,
 *  and then lock the routing tree.
 */
void tfSocketResetCaches (  tt16Bit           rteFlag
                          , int               addressFamilyFlag
#ifdef TM_LOCK_NEEDED
                          , tt8Bit            lockFlag
#endif /* TM_LOCK_NEEDED */
                         )
{
    ttSocketEntryPtr    socketPtr;
    unsigned int        sockIndex;
    int                 af;

#ifdef TM_USE_STOP_TRECK
    if (tm_context(tvSocketArray) != 0)
#endif /* TM_USE_STOP_TRECK */
    {
#ifdef TM_LOCK_NEEDED
        if (    ( tvLockEnabled == TM_8BIT_ZERO )
             || (    tfLock(&tm_context(tvSocketArrayLock), lockFlag)
                  == TM_LOC_OKAY) )
#endif /* TM_LOCK_NEEDED */
        {
            for ( sockIndex = 0;
                  (tt32Bit)sockIndex < tm_context(tvMaxNumberSockets);
                  sockIndex++ )
            {
#ifdef TM_USE_BSD_DOMAIN
                if ( (tm_context(tvSocketArray)[sockIndex].afsAF) !=
                                                         TM_BSD_FAMILY_INET )
                {
                    socketPtr = TM_SOCKET_ENTRY_NULL_PTR;
                }
                else
                {
                    socketPtr = (ttSocketEntryPtr)
                          (tm_context(tvSocketArray)[sockIndex].afsSocketPtr);
                }
#else /* !TM_USE_BSD_DOMAIN */
                socketPtr = tm_context(tvSocketArray)[sockIndex];
#endif /* !TM_USE_BSD_DOMAIN */
                if (socketPtr != TM_SOCKET_ENTRY_NULL_PTR)
                {
                    if (socketPtr->socRteCacheStruct.rtcRtePtr
                                                       != TM_RTE_NULL_PTR)
                    {
                        af = socketPtr->socProtocolFamily;
#ifdef TM_USE_IPDUAL
/* 
 * A socket with address family AF_INET6 could use IPv4 addresses.
 * Change to AF_INET if we need to, i.e. if the socket routing cache
 * points a an IPv4 mapped IPv6 address, and we are looking to reset
 * IPv4 routing caces only.
 */
                        if (    (af == AF_INET6)
                             && (addressFamilyFlag == AF_INET))
/*
 * No match and socket address family is AF_INET6.
 * Check if we have an IPv4 mapped IPv6 address
 */
                        {
                            if (tm_6_addr_is_ipv4_mapped(
                            &socketPtr->socRteCacheStruct.rtcRtePtr->rteDest))
                            {
                                af = AF_INET;
                            }
                        }
#endif /* TM_USE_IPDUAL */
                        if (af & addressFamilyFlag)
                        {
#ifdef TM_LOCK_NEEDED
                            if (   ( tvLockEnabled == TM_8BIT_ZERO )
                                || (    tfLock(&socketPtr->socLockEntry,
                                               lockFlag)
                                     == TM_LOC_OKAY) )
#endif /* TM_LOCK_NEEDED */
                            {
                                tfRtCacheReset(&socketPtr->socRteCacheStruct,
                                               rteFlag);
                                tm_call_unlock(&(socketPtr->socLockEntry));
                            }
                        }
                    }
                }
            }
            tm_call_unlock(&tm_context(tvSocketArrayLock));
        }
    }
}

/*
 * Generate new values for tvVector1, tvVector2 .
 */
void tfRandNew (void)
{
    tm_context(tvVector2) <<= 1;
    tm_context(tvVector2) |= (
                     ( (tm_context(tvVector1) >> TM_RAND_V1T1) & 1 )
                   ^ ( (tm_context(tvVector1) >> TM_RAND_V1T2) & 1 ) );
    tm_context(tvVector1) <<= 1;
    tm_context(tvVector1) |= (
                     ( (tm_context(tvVector2) >> TM_RAND_V2T1) & 1 )
                   ^ ( (tm_context(tvVector2) >> TM_RAND_V2T2) & 1 ) );
}

/* 
 * Add new seed to old seed
 */
void tfRandAdd(tt16BitPtr oldSeed1Ptr,
               tt16BitPtr oldSeed2Ptr,
               tt16Bit    newSeed1,
               tt16Bit    newSeed2)
{
    newSeed1 = (tt16Bit)(*oldSeed1Ptr ^ newSeed1);
    newSeed2 = (tt16Bit)(*oldSeed2Ptr ^ newSeed2);
    newSeed2 <<= 1;
    newSeed2 |= (   ( (newSeed1 >> TM_RAND_V1T1) & 1 )
                 ^ ( (newSeed1 >> TM_RAND_V1T2) & 1 ) );
    newSeed1 <<= 1;
    newSeed1 |= (   ( (newSeed2 >> TM_RAND_V2T1) & 1 )
                 ^ ( (newSeed2 >> TM_RAND_V2T2) & 1 ) );
    *oldSeed1Ptr = newSeed1;
    *oldSeed2Ptr = newSeed2;
}

/*
 * Make sure that the seed number is not 0 or 0xFFFF
 */
static unsigned short tfRandCheck (unsigned short seed)
{
    if (seed == (unsigned short)0)
    {
        seed = (unsigned short)1;
    }
    else
    {
        if (seed == (unsigned short)0xFFFF)
        {
            seed = (unsigned short)0xFFFE;
        }
    }
    return seed;
}
