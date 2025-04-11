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
 * Description: Red-Black Tree Support for sockets
 * Sockets are sorted in the Red-Black tree by Port Number
 * and the Layer 4 Protocol for connection-less sockets.
 * For connection-oriented sockets, sockets are also sorted by remote
 * IP address and remote IP port.
 *
 * Filename: trsockrb.c
 * Author: Paul
 * Date Created: 11/10/97
 * $Source: source/trsockrb.c $
 *
 * Modification History
 * $Revision: 6.0.2.15 $
 * $Date: 2012/03/17 09:11:26JST $
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
 * Local macros
 */

#ifdef TM_USE_SOCKET_RB_TREE
#define TM_RB_RED   1
#define TM_RB_BLACK 0
#endif /* TM_USE_SOCKET_RB_TREE */

#ifdef TM_USE_IPDUAL
/* Red-black tree key generation with address family.
 * Note: the address family is positioned in the least significant byte
 * of the key so that sockets that differ only in address family will be
 * adjacent in a red-black tree. */
#define TM_RB_KEYGEN(port, proto, family) \
    (((tt32Bit)(proto) << 24) | ((tt32Bit)(port) << 8) | (tt32Bit)(family))
/* Omit the address family from the key to relax the search criteria */
#define TM_RB_KEYRELAX(key) ((tt32Bit)(key) & ~0xFFUL)
#else /* !TM_USE_IPDUAL */
/* Red-black tree key generation without address family */
#define TM_RB_KEYGEN(port, proto, family) \
    (((tt32Bit)(proto) << 24) | (tt32Bit)(port))
#endif /* !TM_USE_IPDUAL */

#ifdef TM_USE_TCP
/* TCP lookup table, connection-less search Types */
/* Local address port search flags */
#define TM_LOCAL_ANY_ADDR             1  /* Any local address */
#define TM_LOCAL_V4_MAPPED            2  /* V4 mapped IPV6 address */
#define TM_LOCAL_WILD_PORT            4  /* Wild port */
#define TM_CL_ANY_ADDR_V4     \
        (TM_SOC_TREE_CL_FLAG | TM_LOCAL_ANY_ADDR | TM_LOCAL_V4_MAPPED)
#define TM_CL_ANY_ADDR        \
        (TM_SOC_TREE_CL_FLAG | TM_LOCAL_ANY_ADDR)
#define TM_CL_WILD_PORT    \
        (TM_SOC_TREE_CL_FLAG | TM_LOCAL_WILD_PORT)
#define TM_CL_ANY_ADDR_V4_WILD_PORT \
        (TM_CL_ANY_ADDR_V4 | TM_LOCAL_WILD_PORT)
#define TM_CL_ANY_ADDR_WILD_PORT    \
        (TM_CL_ANY_ADDR | TM_LOCAL_WILD_PORT)
#endif /* TM_USE_TCP */

#ifdef TM_USE_SOCKET_RB_TREE
#define tm_socket_delete(socketPtr, index, searchIndex)        \
        tm_context(tvSocketPortSearchMembers[searchIndex])--;  \
        tfSocketRbDelete(socketPtr, index)
#ifdef TM_USE_TCP
#define tm_socket_insert(socketPtr, sockTuplePtr)              \
        tfSocketRbInsert(socketPtr, sockTuplePtr)
#else /* !TM_USE_TCP */
#define tm_socket_insert(socketPtr, sockTuplePtr)              \
        tfSocketRbInsert(socketPtr)
#endif /* !TM_USE_TCP */
#else /* !TM__USE_SOCKET_RB_TREE */
#ifdef TM_USE_TCP
#define tm_socket_insert(socketPtr, sockTuplePtr)                             \
        tfSocketListInsert(socketPtr, sockTuplePtr)
#else /* !TM_USE_TCP */
#define tm_socket_insert(socketPtr, sockTuplePtr)                             \
        tfSocketListInsert(socketPtr)
#endif /* !TM_USE_TCP */
#ifdef TM_USE_SOCKET_HASH_LIST
#define tm_socket_delete(socketPtr, index, searchIndex)                       \
    tm_context(tvSocketPortSearchMembers[searchIndex])--;                     \
    (void)tfListRemove(                                                       \
             &(tm_context(tvSocketTableListPtr[index][socketPtr->socHash])),  \
             &(socketPtr->socTableNode));                                     \
    if (tm_context(tvSocketTableListPtr                                       \
        [index][socketPtr->socHash]).listCount == 0)                          \
    {                                                                         \
        tm_context(tvSocketTableListHashBucketFilled[index])--;               \
    }
#else /* !TM_USE_SOCKET_HASH_LIST */
#define tm_socket_delete(socketPtr, index, searchIndex)                       \
    tm_context(tvSocketPortSearchMembers[searchIndex])--;                     \
    (void)tfListRemove(&(tm_context(tvSocketTableList[index])),               \
                       &(socketPtr->socTableNode));
#endif /* !TM_USE_SOCKET_HASH_LIST */
#endif /* !TM_USE_SOCKET_RB_TREE */

#define tm_32bit_cmp(x1, x2)    \
        (((x1) == (x2)) ? TM_STRCMP_EQUAL : (((x1) < (x2)) ? TM_STRCMP_LESS : TM_STRCMP_GREATER))

#ifdef TM_USE_TCP
#define tm_tcpvect_alloc() (ttTcpVectPtr)tm_recycle_alloc( \
                                         sizeof(ttTcpVect),    \
                                         TM_RECY_TCPVEC)
#endif /* TM_USE_TCP */
#define tm_soc_alloc()      (ttSocketEntryPtr)tm_recycle_alloc( \
                                         sizeof(ttSocketEntry), \
                                         TM_RECY_SOCKET)

/*
 * Local types.
 */
/*
 * Local functions
 */

#ifdef TM_USE_SOCKET_RB_TREE
/* non TCP tree search */
static ttSocketEntryPtr tfSocketRbClLookup(tt32Bit    redBlackKey);
#ifdef TM_USE_TCP
/* TCP tree search */
static ttSocketEntryPtr tfSocketRbLookup(ttSockTuplePtr sockTuplePtr);
#endif /* TM_USE_TCP */

static void tfSocketRbLeftRotate(ttSocketEntryPtrPtr  socketRootPtrPtr,
                                 ttSocketEntryPtr     socketPtr);
                          
static void tfSocketRbRightRotate(ttSocketEntryPtrPtr socketRootPtrPtr,
                                  ttSocketEntryPtr    socketPtr);
                           
static ttSocketEntryPtr tfSocketRbInsert(ttSocketEntryPtr    socketPtr
#ifdef TM_USE_TCP
                                       , ttSockTuplePtr      sockTuplePtr
#endif /* TM_USE_TCP */
                                         );
                     
static void tfSocketRbDelete(ttSocketEntryPtr         socketPtr,
                             int                      index);

#else /* !TM_USE_SOCKET_RB_TREE */
static ttSocketEntryPtr tfSocketListInsert(
    ttSocketEntryPtr    socketPtr
#ifdef TM_USE_TCP
  , ttSockTuplePtr      sockTuplePtr
#endif /* TM_USE_TCP */
    );

#ifdef TM_USE_TCP
static int tfSocketTupleEqualCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam);
#endif /* TM_USE_TCP */

static int tfSocketRBKeyEqualCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam);

#if defined(TM_USE_SOCKET_WALK) || defined(TM_USE_STOP_TRECK)
static int tfSocketWalkCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam);
#endif /* TM_USE_SOCKET_WALK || TM_USE_STOP_TRECK */
#endif /* !TM_USE_SOCKET_RB_TREE */

#ifdef TM_USE_REUSEADDR_LIST
static ttSocketEntryPtr tfSocketReuseMatch(
#if (defined(TM_USE_STRONG_ESL) || defined(TM_IGMP) || defined(TM_6_USE_MLD))
                                     ttDeviceEntryPtr      devPtr,
#endif /* TM_USE_STRONG_ESL || TM_IGMP || TM_6_USE_MLD */
                                     ttSocketEntryPtr      reuseAddrSocketPtr,
                                     ttSockTuplePtr        sockTuplePtr,
                                     tt16Bit               flag );
#endif /* TM_USE_REUSEADDR_LIST */


/*
 * Allocate a new Socket Node. Make sure that we do not exceed the
 * maximum number of open sockets.
 */
ttSocketEntryPtr tfSocketAllocate ( tt8Bit   protocol,
                                    ttIntPtr errorCodePtr )
{
    ttSocketEntryPtr newSocketPtr;
#ifdef TM_USE_TCP
    ttTcpVectPtr     newTcpVectPtr;
#endif /* TM_USE_TCP */
    int              errorCode;

/* LOCK the tvNumberOpenSockets variable */
    tm_call_lock_wait(&tm_context(tvSocketOpenLock));
    errorCode = TM_ENOBUFS;
    if (   (tm_context(tvNumberOpenSockets) < tm_context(tvMaxNumberSockets))
#ifdef TM_USE_TCP
        && ((protocol != (tt8Bit)IP_PROTOTCP)
            || (tm_context(tvTcpVectAllocCount)
                < tm_context(tvMaxTcpVectAllocCount)))
#endif /* TM_USE_TCP */
       )
/* If array is not full */
    {
        if (protocol == (tt8Bit)IP_PROTOTCP)
        {
#ifdef TM_USE_TCP
            newTcpVectPtr = tm_tcpvect_alloc();
            if (newTcpVectPtr == (ttTcpVectPtr)0)
            {
#ifdef TM_ERROR_CHECKING
                 tfKernelWarning("tfSocketAllocate",
                   "Unable to Allocate Memory for TCP socket from Kernel/OS");
#endif /* TM_ERROR_CHECKING */
                 newSocketPtr = (ttSocketEntryPtr)0;
                 goto socketAllocateFinish;
            }
            tfTcpVectInit(newTcpVectPtr);
            newSocketPtr = &(newTcpVectPtr->tcpsSocketEntry);
/* increase the number of allocated TCP sockets */
            tm_context(tvTcpVectAllocCount)++;
#else /* !TM_USE_TCP */
             newSocketPtr = (ttSocketEntryPtr)0;
             errorCode = TM_EPROTONOSUPPORT;
             goto socketAllocateFinish;
#endif /* !TM_USE_TCP */
        }
        else
        {
            newSocketPtr = tm_soc_alloc();
            if (newSocketPtr == TM_SOCKET_ENTRY_NULL_PTR)
            {
#ifdef TM_ERROR_CHECKING
                 tfKernelWarning("tfSocketAllocate",
                 "Unable to Allocate Memory for Socket Entry from Kernel/OS");
#endif /* TM_ERROR_CHECKING */
                 goto socketAllocateFinish;
            }
            newSocketPtr->socRecvCopyFraction =
                                            tm_context(tvDefRecvCopyFraction);
            newSocketPtr->socMaxRecvQueueDgrams =
                tm_context(tvDefRecvQueueDgrams);
            newSocketPtr->socMaxSendQueueDgrams =
                tm_context(tvDefSendQueueDgrams);
#ifdef TM_USE_IGMPV3
            tfListInit(&newSocketPtr->socIgmpInfoList);
#endif /* TM_USE_IGMPV3 */
        }
        newSocketPtr->socOwnerCount = 1;
#ifdef TM_SNMP_CACHE
/* Initialize SNMPD/NSTAT cache index */
        newSocketPtr->socSnmpCacheEntry.sceTableIndex = TM_SNMPC_NO_INDEX;
#endif /* TM_SNMP_CACHE */
        newSocketPtr->socProtocolNumber = protocol;
/* Not in socket array yet */
        newSocketPtr->socIndex = TM_SOC_NO_INDEX;
/* Load up the socket with defaults */
        newSocketPtr->socMaxRecvQueueBytes = tm_context(tvDefRecvQueueBytes);
        newSocketPtr->socMaxSendQueueBytes = tm_context(tvDefSendQueueBytes);
        newSocketPtr->socLowSendQueueBytes =
                           tm_send_low_water(tm_context(tvDefSendQueueBytes));
        newSocketPtr->socLowRecvQueueBytes =
                           tm_recv_low_water(tm_context(tvDefRecvQueueBytes));
#ifdef TM_USE_IPV4
        newSocketPtr->socIpTtl = tm_context(tvIpDefTtl);
        newSocketPtr->socIpMcastTtl = (tt8Bit)TM_IP_DEF_MULTICAST_TTL;
        newSocketPtr->socIpTos = tm_context(tvIpDefTos);
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
        newSocketPtr->soc6HopLimit = -1;
        newSocketPtr->soc6McastHopLimit = -1;
        newSocketPtr->soc6FlowInfo = TM_UL(0);
#ifdef TM_6_USE_RAW_SOCKET
        newSocketPtr->soc6RawChecksumOffset = -1;
#endif /* TM_6_USE_RAW_SOCKET */
#endif /* TM_USE_IPV6 */
        newSocketPtr->socOptions = tm_context(tvSocDefOptions);

/* If we cannot block, make sure that the socket is non blocking */
        if (tvPendPostEnabled == TM_8BIT_ZERO)
        {
            newSocketPtr->socFlags = TM_SOCF_NONBLOCKING;
        }
        tm_context(tvNumberOpenSockets)++;
        errorCode = TM_ENOERROR;
        tm_call_lock_wait(&newSocketPtr->socLockEntry);
    }
    else
    {
        errorCode = TM_EMFILE;
        newSocketPtr = (ttSocketEntryPtr)0;
    }
socketAllocateFinish:
    tm_call_unlock(&tm_context(tvSocketOpenLock));
    *errorCodePtr = errorCode;
    return (newSocketPtr);
}

/*
 * We are either closing a pending connection socket, or a user socket.
 * Decrese the number of open sockets.
 */
void tfSocketDecreaseOpenSockets (void)
{
/* LOCK the tvNumberOpenSockets variable */
    tm_call_lock_wait(&tm_context(tvSocketOpenLock));
    tm_context(tvNumberOpenSockets)--;
/* UNLOCK number of open sockets lock */
    tm_call_unlock(&tm_context(tvSocketOpenLock));
    return;
}

/*
 * Generate a random unique port number and insert into tree
 */
ttIpPort tfRandPortInsert (ttSocketEntryPtr socketPtr,
                           ttSockTuplePtr   sockTuplePtr,
                           tt16Bit          flag)
{
    tt32Bit         randomNumber;
    unsigned int    i;
    unsigned int    portRange;
    int             tableIndex;
    int             searchIndex;
    int             retCode;
    ttIpPort        ipPort;
    tt16Bit         hostPort;

/* Assume failure */
    retCode = TM_SOC_RB_ALREADY;
    i = 0;
#ifdef TM_USE_TCP
    tableIndex = TM_SOTB_TCP_CON; /* TCP look up table */
    if (tm_16bit_one_bit_set(flag, TM_SOC_TREE_CO_FLAG))
#define TM_PORT_FIRST_NOT_WELL_KNOWN    1024
    {
/* Connected TCP search */
        searchIndex = TM_SOTB_TCP_CON;
        portRange = (unsigned int)(
                              (unsigned int)0xFFFF
                            - (unsigned int)TM_PORT_FIRST_NOT_WELL_KNOWN);
    }
    else
#endif /* TM_USE_TCP */
    {
#define TM_PORT_HIGH_RANGE  0x8000
/*
 * Random listening ports must ALWAYS be in the range 0x8000 - 0xfffe 
 * (32768 - 65534)
 */
#ifdef TM_USE_TCP
        if (tm_16bit_one_bit_set(flag, TM_SOC_TREE_CL_FLAG))
        {
/* Non connected TCP search */
            searchIndex = TM_SOTB_NUM;
        }
        else
#endif /* TM_USE_TCP */
        {
            tableIndex = TM_SOTB_NON_CON;
/* Non TCP search */
            searchIndex = TM_SOTB_NON_CON;
        }
        portRange = (unsigned int)(   (unsigned int)0xFFFF
                                   - (unsigned int)TM_PORT_HIGH_RANGE);
    }
    if (     (tm_context(tvSocketPortSearchHighLoopCount[searchIndex]) >
                                                                   (tt16Bit)1)
         &&  (tm_context(tvSocketPortSearchMembers[searchIndex]) >= (tt32Bit)
                     tm_context(tvSocketThresholdSequentialPort[tableIndex])))
    {
/*
 * If we had a port conflict, and we have reached the threshold, switch to
 * sequential port selection.
 */
        hostPort = tm_context(tvSocketPortSearchLastPort[searchIndex]);
        hostPort++;
    }
    else
    {
        randomNumber = tfGetRandom();
        hostPort = (tt16Bit)randomNumber;
    }
    do
    {
#ifdef TM_USE_TCP
        if (tm_16bit_one_bit_set(flag, TM_SOC_TREE_CO_FLAG))
        {
/* Make sure that port is at least 1024 */
            if (hostPort < (ttIpPort)TM_PORT_FIRST_NOT_WELL_KNOWN)
            {
                hostPort = TM_PORT_FIRST_NOT_WELL_KNOWN;
            }
        }
        else
#endif /* TM_USE_TCP */
        {
#ifdef TM_32BIT_DSP
        /*everything is 32bit, so mask off leftmost 2 bytes*/
            hostPort = (ttIpPort)( (hostPort | TM_PORT_HIGH_RANGE) & 0xFFFF );
#else /* TM_32BIT_DSP */
            hostPort = (ttIpPort)( hostPort | TM_PORT_HIGH_RANGE );
#endif /* TM_32BIT_DSP */
        }
        tm_htons(hostPort, ipPort);

        if (    (hostPort != TM_WILD_PORT)
             || ((tm_16bit_one_bit_set(flag, TM_SOC_TREE_CO_FLAG))) )
/* TM_WILD_PORT (65535) is reserved (used to listen on all ports) */
        {
            sockTuplePtr->sotLocalIpPort = ipPort;
            retCode = tfSocketTreeInsert(socketPtr, sockTuplePtr, flag);
            if (retCode == TM_SOC_RB_OKAY)
            {
/* Remember port */
                tm_context(tvSocketPortSearchLastPort[searchIndex]) = hostPort;
                break;
            }
        }
        hostPort++;
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
    } while (i++ < portRange);
/* PRQA L:L1 */
    if (retCode != TM_SOC_RB_OKAY)
    {
/* Did not find a random port number */
        ipPort = (ttIpPort)0;
    }
    else
    {
/* Remember highest loop count (for diagnosis) */
        i++;
        if (i > tm_context(tvSocketPortSearchHighLoopCount[searchIndex]))
        {
            tm_context(tvSocketPortSearchHighLoopCount[searchIndex]) =
                                                                 (tt16Bit)i;
        }
    }
    return (ipPort);
}

/* 
 * Insert a locked socket entry into the tree or
 * if there is a local IP address conflict and REUSEADDDR is allowed,
 * Insert into the list of reuseaddr sockets
 * On success, check out the socket (for tree ownership). 
 */
int tfSocketTreeInsert( ttSocketEntryPtr        socketPtr,
                        ttSockTuplePtr          sockTuplePtr,
                        tt16Bit                 flag)
{
    tt32Bit             otherTemp;
    tt32Bit             portTemp;
    ttSocketEntryPtr    reuseAddrSocketPtr;
#ifdef TM_USE_REUSEADDR_LIST
    ttSocketEntryPtr    nextReuseAddrSocketPtr;
#endif /* TM_USE_REUSEADDR_LIST */
    tt16Bit             coFlag;
    int                 retCode;

/* Save the connection oriented, or connection-less flag */
    coFlag = (tt16Bit)(flag & TM_SOC_TREE_ALL_FLAG);
/*
 * The tree insertion
 *      1. must be either connectionless or connection-oriented,
 *         for TCP, or non flavored for non TCP.
 * and  2. cannot be both connectionless and
 *         connection-oriented.
 * and  3. cannot have already taken place.
 */
    tm_assert(
     tfSocketTreeInsert,
     (    (   (    tm_16bit_one_bit_set(coFlag, TM_SOC_TREE_TCP_FLAG)
                && (((int)socketPtr->socProtocolNumber) == IPPROTO_TCP))
           || (    (coFlag == TM_16BIT_ZERO) 
                && ((int)socketPtr->socProtocolNumber != IPPROTO_TCP) ) )
       && (!tm_16bit_all_bits_set(coFlag, TM_SOC_TREE_TCP_FLAG))
       && tm_16bit_bits_not_set(socketPtr->socFlags, TM_SOCF_IN_TREE)) );
#if defined(TM_USE_REUSEPORT) && defined(TM_USE_TCP)
    if (    (tm_16bit_one_bit_set(flag, SO_REUSEPORT))
         && (flag & TM_SOC_TREE_CO_FLAG) != 0)
/* New socket has SO_REUSEPORT set, and TCP connected flag set */
/*
 * Allow REUSEPORT:
 * 1. For non connected/connecting TCP sockets.
 * 2. for non TCP sockets
 */
    {
        retCode = TM_SOC_RB_ALREADY; /* failure */
    }
    else
#endif /* TM_USE_REUSEPORT && TM_USE_TCP */
    {
        retCode = TM_SOC_RB_OKAY; /* Assume success */
#ifdef TM_USE_TCP
        if (((int)socketPtr->socProtocolNumber) != IPPROTO_TCP)
#endif /* TM_USE_TCP */
        {
/*    
 * Its neccessary for the protocol and port number to be 32-bit values,
 * in order for some DOS real mode compilers to work correctly
 */
            otherTemp = socketPtr->socProtocolNumber;
            portTemp = sockTuplePtr->sotLocalIpPort;
            socketPtr->socRedBlackKey = TM_RB_KEYGEN(
                                            portTemp,
                                            otherTemp,
                                            socketPtr->socProtocolFamily );
        }

/* LOCK the root, if not already locked */
        if (tm_16bit_bits_not_set(flag, TM_SOC_TREE_LOCKED_FLAG))
        {
            tm_lock_wait(&tm_context(tvSocketRootLock));
        }
/* Store the connection oriented, or connection-less flag + the no match flag */
        socketPtr->socCoFlags = (tt16Bit)(coFlag |
                                          (tt16Bit)
                                           (flag & TM_SOC_TREE_NO_MATCH_FLAG));
        reuseAddrSocketPtr = tm_socket_insert(socketPtr, sockTuplePtr);
        if (reuseAddrSocketPtr != (ttSocketEntryPtr)0)
        {
#ifdef TM_USE_REUSEADDR_LIST
#ifndef TM_USE_STRONG_ESL
            if (tm_16bit_one_bit_set(coFlag, TM_SOC_TREE_CO_FLAG))
/*
 * TCP connected socket, we already checked on the local address. So
 * there is a match on the 4-tuple.
 */
            {
/* Failed to insert */
                retCode = TM_SOC_RB_ALREADY;
            } else
#endif /* !TM_USE_STRONG_ESL */
/* If there is an inserted socket at the local port number already */
            if (    tm_16bit_one_bit_set(
                                    (flag & reuseAddrSocketPtr->socOptions),
                                    (SO_REUSEADDR | SO_REUSEPORT) )
               )
/*
 * Both new socket, and old socket have either SO_REUSEADDR set
 * or SO_REUSEPORT set.
 */
            {
/*
 * If we allow reuse of address, insert our socket in the queue of reuse
 * address socket entries. Make sure nobody else uses the same local
 * IP address, unless SO_REUSEPORT is set on both the new and matched sockets.
 */
                nextReuseAddrSocketPtr = reuseAddrSocketPtr;
                do
                {
                    if (
/* TCP connected tree we already have a match on the local IP address */
                         tm_16bit_one_bit_set(coFlag, TM_SOC_TREE_CO_FLAG) ||
/* Check that there is no socket with the same local IP address in the tree */
#ifdef TM_USE_IPV6
                         tm_6_ip_match(
                              &(nextReuseAddrSocketPtr->socOurIfIpAddress),
                              &(sockTuplePtr->sotLocalIpAddress))
#else /* ! TM_USE_IPV6 */
                         tm_ip_match(
                                nextReuseAddrSocketPtr->socOurIfIpAddress,
                                sockTuplePtr->sotLocalIpAddress)
#endif /* ! TM_USE_IPV6 */
                       )
                    {
/* Socket with the same local IP address.*/
#ifdef TM_USE_STRONG_ESL
                        if (   tm_8bit_bits_not_set(socketPtr->socFlags2,
                                                    TM_SOCF2_BINDTODEVICE)
                            || tm_8bit_bits_not_set(
                                            nextReuseAddrSocketPtr->socFlags2,
                                            TM_SOCF2_BINDTODEVICE)
                            || ( socketPtr->socRteCacheStruct.rtcDevPtr ==
                                 nextReuseAddrSocketPtr->
                                                 socRteCacheStruct.rtcDevPtr)
                           )
/* And bound to the same device, or one not bound */
#endif /* TM_USE_STRONG_ESL */
/* Cannot insert this one. */
                        {
#ifdef TM_USE_REUSEPORT
                            if ( tm_16bit_bits_not_set(
                                  (flag & nextReuseAddrSocketPtr->socOptions),
                                  SO_REUSEPORT) )
/*
 * Allow local address match only if both sockets have the SO_REUSEPORT 
 * option
 */
#endif /* TM_USE_REUSEPORT */
                            {
                                retCode = TM_SOC_RB_ALREADY;
                            }
                            break;
                        }
                    }
                    nextReuseAddrSocketPtr =
                                  nextReuseAddrSocketPtr->socReuseAddrNextPtr;
                } while (nextReuseAddrSocketPtr != (ttSocketEntryPtr)0);
                if (retCode == TM_SOC_RB_OKAY)
/* 
 * SO_REUSEADDR with no socket matching on the local IP address in the tree, 
 * or SO_REUSEPORT for non connected TCP socket
 * or SO_REUSEPORT for UDP socket with multicast address
 */
                {
/* Insert socket in the reuseaddr list */
                    nextReuseAddrSocketPtr =
                                    reuseAddrSocketPtr->socReuseAddrNextPtr;
                    socketPtr->socReuseAddrNextPtr = nextReuseAddrSocketPtr;
                    socketPtr->socReuseAddrPrevPtr = reuseAddrSocketPtr;
                    reuseAddrSocketPtr->socReuseAddrNextPtr = socketPtr;
                    if (nextReuseAddrSocketPtr != (ttSocketEntryPtr)0)
                    {
                        nextReuseAddrSocketPtr->socReuseAddrPrevPtr =
                                                               socketPtr;
                    }
/* Entry is in a reuse address list */
                    socketPtr->socInReuseAddrList = TM_8BIT_YES;
                }
            }
            else
#endif /* TM_USE_REUSEADDR_LIST */
            {
                retCode = TM_SOC_RB_ALREADY;
            }
        }
        if (retCode == TM_SOC_RB_OKAY)
        {
/* Increase socket owner count */
            tm_socket_checkout(socketPtr);
/* Entry is inserted in the tree */
            socketPtr->socFlags |= TM_SOCF_IN_TREE;
            if (sockTuplePtr != &socketPtr->socTuple)
/* Copy the tuple if not already done */
            {
                tm_bcopy(sockTuplePtr, &socketPtr->socTuple,
                         sizeof(*sockTuplePtr));
            }
#ifdef TM_SNMP_CACHE
            tfSnmpdCacheInsertSocket(socketPtr);
#endif /* TM_SNMP_CACHE */
        }
        if ( tm_16bit_bits_not_set( flag,   TM_SOC_TREE_LOCKED_FLAG
                                          | TM_SOC_TREE_KEEP_LOCK_FLAG ) )
        {
/* UNLOCK the root, if locked in this routine. */
            tm_unlock(&tm_context(tvSocketRootLock));
        }
    }
    return(retCode);
}

#ifdef TM_USE_REUSEADDR_LIST
/*
 * Check out the socket that matches the local IP address, in the passed reuse
 * address list. 
 * 1. Lock the tree
 * 2. If entry has been removed from the tree, set return value to null.
 * 3. else In a loop until we reach the end of the reuseaddr list.
 *   . Check if the passed socket matches. If so lock it
 *     and check it out, and break out of the loop.
 *   . Otherwise get the next socket in the reuse address list.
 * 4. Unlock the tree.
 * 5. Return socketPtr.
 *
 * Parameters
 * socketPtr    pointer to a socket at the head of a reuse address list
 * Return
 * Pointer to the next socket in the reuseaddress list that matches
 * the local IP address. 
 */
ttSocketEntryPtr tfSocketCheckOutReused (
#if (defined(TM_USE_STRONG_ESL) || defined(TM_IGMP) || defined(TM_6_USE_MLD))
                                        ttDeviceEntryPtr       devPtr,
#endif /* TM_USE_STRONG_ESL || TM_IGMP || TM_6_USE_MLD */
                                        ttSocketEntryPtr       socketPtr,
                                        ttSockTuplePtr         sockTuplePtr,
                                        tt16Bit                flag)
{
/* LOCK the root */
    tm_lock_wait(&tm_context(tvSocketRootLock));
/*
 * Entry is still in the tree (i.e. nobody removed it from the tree while we
 * acquired the lock)
 */
    if (tm_16bit_bits_not_set(socketPtr->socFlags, TM_SOCF_IN_TREE))
    {
/* Cannot use the entry, since it has been removed from the tree */
        socketPtr = (ttSocketEntryPtr)0; 
    }
    else
    {
        socketPtr = tfSocketReuseMatch(
#if (defined(TM_USE_STRONG_ESL) || defined(TM_IGMP) || defined(TM_6_USE_MLD))
                                        devPtr,
#endif /* TM_USE_STRONG_ESL || TM_IGMP || TM_6_USE_MLD */
                                        socketPtr,
                                        sockTuplePtr,
                                        flag);
        if (socketPtr != (ttSocketEntryPtr)0)
        {
/* socket has the reuseOption set. Lock it, and check it out. */
            tm_lock_wait(&socketPtr->socLockEntry);
            tm_socket_checkout(socketPtr);
        }
    }
/* UNLOCK the root */
    tm_unlock(&tm_context(tvSocketRootLock));
    return socketPtr;
}
#endif /* TM_USE_REUSEADDR_LIST */

/*
 * Re-insert a socket that is already in the tree, at a different
 * location in the tree. (Used by bind(), connect(), setsockopt with
 * local IP address change.)
 */
int tfSocketTreeReInsert(
    ttSocketEntryPtr    socketPtr,
    ttSockTuplePtr      sockTuplePtr,
    tt16Bit             flag)
{
    int              retCode;
    int              reInsert;
    ttIpPort         localPortNumber;
    tt16Bit          savCoFlag;

    retCode = TM_SOC_RB_ALREADY;
    if (tm_16bit_one_bit_set(socketPtr->socFlags, TM_SOCF_IN_TREE))
    {
/* tfSocketTreeDelete will lock the tree, and keep it locked */
        reInsert = tfSocketTreeDelete(socketPtr, TM_SOC_TREE_KEEP_LOCK_FLAG);
        if (tm_16bit_one_bit_set(socketPtr->socFlags, TM_SOCF_NO_MORE_SEND_DATA ))
        {
/* User closed the socket while we were trying to remove it from the tree */
            retCode = TM_SOC_RB_OKAY;
        }
        else
        {
            socketPtr->socReuseAddrPrevPtr = TM_SOCKET_ENTRY_NULL_PTR; 
            socketPtr->socReuseAddrNextPtr = TM_SOCKET_ENTRY_NULL_PTR; 

            savCoFlag = socketPtr->socCoFlags;
            flag = (tt16Bit)(flag | TM_SOC_TREE_LOCKED_FLAG);
/* Try and insert at new location */
            retCode = tfSocketTreeInsert(socketPtr, sockTuplePtr, flag);
            if (    (retCode != TM_SOC_RB_OKAY) 
                 && tm_16bit_one_bit_set(flag, TM_SOC_TREE_RANDOM_PORT) )
            {
/* If we failed, and it is OK to try and use a new random port number, do so */
                localPortNumber = tfRandPortInsert(socketPtr, sockTuplePtr, flag);
                if ( localPortNumber != TM_16BIT_ZERO )
                {
                    retCode = TM_SOC_RB_OKAY;
                }
            }
            if (    (retCode != TM_SOC_RB_OKAY)
                 && (reInsert == TM_SOC_RB_OKAY) )
            {
/* Re-insert in the tree at the previous position */
                flag = (tt16Bit)(   savCoFlag
                                  | TM_SOC_TREE_LOCKED_FLAG
                                  | (tt16Bit)
                                       (  socketPtr->socOptions
                                         & (   SO_REUSEADDR
#ifdef TM_USE_REUSEPORT
                                             | SO_REUSEPORT
#endif /* TM_USE_REUSEPORT */
                                           ))
                                );
                (void)tfSocketTreeInsert(socketPtr, sockTuplePtr, flag);
            }
        }
/* UNLOCK the root (locked in tfSocketTreeDelete()) */
        tm_call_unlock(&tm_context(tvSocketRootLock));
    }
    else
    {
/* Try and insert at new location */
        if (tm_16bit_one_bit_set(flag, TM_SOC_TREE_RANDOM_PORT))
        {
            localPortNumber = tfRandPortInsert(socketPtr, sockTuplePtr, flag);
            if ( localPortNumber != TM_16BIT_ZERO )
            {
                retCode = TM_SOC_RB_OKAY;
            }
        }
        else
        {
            retCode = tfSocketTreeInsert(socketPtr, sockTuplePtr, flag);
        }
    }
    return (retCode);
}

/* 
 * Lookup a socket entry by socket tuple
 * Returned it locked and checked out.
 */
ttSocketEntryPtr tfSocketLookup(
    ttTupleDevPtr       tupleDevPtr,
    tt16Bit             flag )
{
    ttSocketEntryPtr socketPtr;
    ttSocketEntryPtr reuseAddrSocketPtr;
    ttSockTuplePtr   sockTuplePtr;
#if (defined(TM_USE_STRONG_ESL) || defined(TM_IGMP) || defined(TM_6_USE_MLD))
    ttDeviceEntryPtr devPtr;
#endif /* TM_USE_STRONG_ESL || TM_IGMP || TM_6_USE_MLD */
#ifdef TM_USE_TCP
    ttSockTuplePtr   origSockTuplePtr;
    ttSockTuple      tuple;
#endif /* TM_USE_TCP */
#ifdef TM_USE_SOCKET_RB_TREE
    tt32Bit          redBlackKey;
#else /* !TM_USE_SOCKET_RB_TREE */
    ttGenericUnion   genParam;
#ifdef TM_USE_SOCKET_HASH_LIST
    tt32Bit          hash;
#endif /* TM_USE_SOCKET_HASH_LIST */
#endif /* !TM_USE_SOCKET_RB_TREE */
    tt32Bit          protocolTemp;
    tt32Bit          portTemp;
#ifdef TM_USE_TCP
    int              searchType;
#endif /* TM_USE_TCP */
    tt8Bit           protocol;
    tt8Bit           wildPortTry;
    tt8Bit           addressFamilyTemp;
#ifdef TM_USE_IPDUAL
    tt8Bit           dualAddressFamilyTry;
#endif /* TM_USE_IPDUAL */
#ifndef TM_USE_REUSEADDR_LIST
#if (defined(TM_IGMP) || defined(TM_6_USE_MLD))
    tt8Bit           mcastMember;
#endif /* TM_IGMP || TM_6_USE_MLD */
#endif /* !TM_USE_REUSEADDR_LIST */

/* Extract additional parameters from tupleDev */
    sockTuplePtr = &tupleDevPtr->tudTuple;
    protocol = tupleDevPtr->tudProtocolNumber;
#if (defined(TM_USE_STRONG_ESL) || defined(TM_IGMP) || defined(TM_6_USE_MLD))
    devPtr = tupleDevPtr->tudDevPtr;
#endif /* TM_USE_STRONG_ESL || TM_IGMP || TM_6_USE_MLD */

#ifdef TM_ERROR_CHECKING
    if ((flag & TM_SOC_TREE_TCP_FLAG) != 0)
    {
/*
 * Connection-oriented, connection less tree search, currently only TCP
 * is supported
 */
        tm_assert(tfSocketLookup, (((int) protocol) == IPPROTO_TCP));
    }
#endif /* TM_ERROR_CHECKING */

/* Assume failure */
    socketPtr = (ttSocketEntryPtr)0;
/* LOCK the root */
    tm_lock_wait(&tm_context(tvSocketRootLock));
#ifdef TM_USE_TCP
    if (protocol == (tt8Bit)IPPROTO_TCP)
    {
/* Save pointer to passed sockTuplePtr */
        origSockTuplePtr = sockTuplePtr;
        searchType = (flag & TM_SOC_TREE_ALL_FLAG);
        do
        {
            switch (searchType)
            {
            case TM_SOC_TREE_CL_FLAG:
/* Connection-less search. Only use local address/port for the search */
                tm_bzero(&tuple, sizeof(tuple));
/* Point to new tuple (with remote address/port zeroed out) */
                sockTuplePtr = &tuple;
/* Copy local port */
                tuple.sotLocalIpPort = origSockTuplePtr->sotLocalIpPort;
/* Fall Through */
            case TM_CL_WILD_PORT:
                if (searchType & TM_LOCAL_WILD_PORT)
                {
/* For wild port search, copy wild port */
                    tuple.sotLocalIpPort = TM_WILD_PORT;
                }
/* Copy original local address */
#ifdef TM_USE_IPV6
                tm_6_ip_copy_structs(origSockTuplePtr->sotLocalIpAddress,
                                     tuple.sotLocalIpAddress);
#else /* !TM_USE_IPV6 */
                tm_ip_copy(origSockTuplePtr->sotLocalIpAddress,
                           tuple.sotLocalIpAddress);
#endif /* !TM_USE_IPV6 */
                if (tm_16bit_bits_not_set(flag, TM_SOC_TREE_EXACT_MATCH))
                {
#ifdef TM_USE_IPDUAL
                    if (IN6_IS_ADDR_V4MAPPED(&(tuple.sotLocalIpAddress)))
                    {
/*
 * When a IPv4 packet is received in a dual stack, and the specific
 * destination IP address and port# match fails, then the IPv4
 * wildcard address and port# is tried,
 */
/* Next search will be for any IPv4 mapped IPv6 address at the same port */
                        searchType |= (TM_LOCAL_ANY_ADDR | TM_LOCAL_V4_MAPPED);
                     }
                     else
#endif /* TM_USE_IPDUAL */
                     {
/* Next search will be for any address at the same port */
                        searchType |= TM_LOCAL_ANY_ADDR;
                     }
                }
                else
                {
/* Done searching */
                    searchType = 0;
                }
                break;
#ifdef TM_USE_IPDUAL
            case TM_CL_ANY_ADDR_V4:
            case TM_CL_ANY_ADDR_V4_WILD_PORT:
/* Search for any IPv4 mapped IPv6 local address at local/wild port */
                tm_6_ip_copy_structs(in6addr_any, tuple.sotLocalIpAddress);
                tuple.sotLocalIpAddress.s6LAddr[2] = TM_IPV4_3_MAP;
/* Next search will be for any address at the same port */
                searchType &= ~TM_LOCAL_V4_MAPPED;
                break;
#endif /* TM_USE_IPDUAL */
            case TM_CL_ANY_ADDR:
            case TM_CL_ANY_ADDR_WILD_PORT:
/* Search for any local address at local port/wild port */
#ifdef TM_USE_IPV6
                tm_6_ip_copy_structs(in6addr_any, tuple.sotLocalIpAddress);
#else /* !TM_USE_IPV6 */
                tm_ip_copy(TM_IP_ZERO,
                           tuple.sotLocalIpAddress);
#endif /* !TM_USE_IPV6 */
                if (searchType & TM_LOCAL_WILD_PORT)
                {
/* Done searching */
                    searchType = 0;
                }
                else
                {
/* Next search will be for wild card port at local address */
                    searchType = TM_CL_WILD_PORT;
                }
                break;
            default:
/* TCP connected search. Keep both remote and local fields. */
/* Next search for non connected socket */
                searchType &= ~TM_SOC_TREE_CO_FLAG;
                break;
            }
#ifdef TM_USE_SOCKET_RB_TREE
            reuseAddrSocketPtr = tfSocketRbLookup(sockTuplePtr);
#else /* !TM_USE_SOCKET_RB_TREE */
            genParam.genVoidParmPtr = (ttVoidPtr)sockTuplePtr;
#ifdef TM_USE_SOCKET_HASH_LIST
            hash = tfSocketHash(
                    (ttCharPtr)genParam.genVoidParmPtr, sizeof(ttSockTuple),
                    tm_context(tvSocketTableListHashSize[TM_SOTB_TCP_CON]));
            reuseAddrSocketPtr = (ttSocketEntryPtr)(ttVoidPtr)
                  tfListWalk(
                    &(tm_context(tvSocketTableListPtr[TM_SOTB_TCP_CON][hash])),
                    tfSocketTupleEqualCB,
                    genParam);
#else /* !TM_USE_SOCKET_HASH_LIST */
            reuseAddrSocketPtr = (ttSocketEntryPtr)(ttVoidPtr)
                  tfListWalk(&(tm_context(tvSocketTableList[TM_SOTB_TCP_CON])),
                             tfSocketTupleEqualCB,
                             genParam);
#endif /* !TM_USE_SOCKET_HASH_LIST */
#endif /* !TM_USE_SOCKET_RB_TREE */
            if (reuseAddrSocketPtr != TM_SOCKET_ENTRY_NULL_PTR
#ifdef TM_USE_IPDUAL
            &&  !(  tm_16bit_one_bit_set(reuseAddrSocketPtr->soc6Flags,
                                                        TM_6_SOCF_V6ONLY)
                &&  (tupleDevPtr->tudAddressFamily == AF_INET) )
#endif /* TM_USE_IPDUAL */
                )
/*
 * local IP port, peer IP port, local address, remote address and protocol
 * match.
 * This is not an IPv4 packet being received by an IPv6-only socket.
 * Check device match.
 */
            {
#ifdef TM_USE_REUSEADDR_LIST
                do
                {
#endif /* TM_USE_REUSEADDR_LIST */
/* Check that the socket can be found (i.e. connected/connecting, or listening */
                    if (    ( tm_16bit_bits_not_set(
                                                reuseAddrSocketPtr->socCoFlags,
                                                TM_SOC_TREE_NO_MATCH_FLAG) )
#ifdef TM_USE_STRONG_ESL
/* and either not bound to a device or bound to the incoming device */
                         && (    (tm_8bit_bits_not_set(
                                                 reuseAddrSocketPtr->socFlags2,
                                                 TM_SOCF2_BINDTODEVICE))
                              || (reuseAddrSocketPtr
                                      ->socRteCacheStruct.rtcDevPtr == devPtr)
                            )
#endif /* TM_USE_STRONG_ESL */
                       )
                    {
                        socketPtr = reuseAddrSocketPtr;
#ifdef TM_USE_REUSEADDR_LIST
                        break; /* inner do while */
#endif /* TM_USE_REUSEADDR_LIST */
                    }
#ifdef TM_USE_REUSEADDR_LIST
                    else
                    {
                        reuseAddrSocketPtr =
                                     reuseAddrSocketPtr->socReuseAddrNextPtr;
                    }
                }
                while (reuseAddrSocketPtr != (ttSocketEntryPtr)0);
#endif /* TM_USE_REUSEADDR_LIST */
            }
        }
        while (searchType && (socketPtr == TM_SOCKET_ENTRY_NULL_PTR));
    }
    else
#endif /* TM_USE_TCP */
    {
/* non TCP sockets, connection-less search */

/*
 * Its neccessary for the protocol and port numbers to be 32-bit values
 * (hence the temporary variables) for some DOS real mode compilers
 * to work correctly
 */
        portTemp = (tt32Bit)sockTuplePtr->sotLocalIpPort;
        protocolTemp = (tt32Bit)protocol;
#ifndef TM_USE_IPDUAL
/* In single address family case, just initialize AF once. */
        addressFamilyTemp = tupleDevPtr->tudAddressFamily;
#endif /* TM_USE_IPDUAL */
        do
/* Port loop */
        {
            wildPortTry = TM_8BIT_ZERO;
#ifdef TM_USE_IPDUAL
/* In dual AF case, initialize in the port loop */
            addressFamilyTemp = tupleDevPtr->tudAddressFamily;
            do
/* Address family loop */
            {
                dualAddressFamilyTry = TM_8BIT_ZERO;
#endif /* TM_USE_IPDUAL */
#ifdef TM_USE_SOCKET_RB_TREE
                redBlackKey = TM_RB_KEYGEN( portTemp,
                                            protocolTemp,
                                            addressFamilyTemp );
                reuseAddrSocketPtr = tfSocketRbClLookup(redBlackKey);
#else /* !TM_USE_SOCKET_RB_TREE */
                genParam.gen32bitParm = TM_RB_KEYGEN(
                                            portTemp,
                                            protocolTemp,
                                            addressFamilyTemp);
#ifdef TM_USE_SOCKET_HASH_LIST
                hash = tfSocketHash(
                        (ttCharPtr)(ttVoidPtr)&(genParam.gen32bitParm),
                        sizeof(tt32Bit),
                        tm_context(tvSocketTableListHashSize[TM_SOTB_NON_CON]));
                reuseAddrSocketPtr = (ttSocketEntryPtr)(ttVoidPtr)
                      tfListWalk(
                        &(tm_context(tvSocketTableListPtr[TM_SOTB_NON_CON][hash])),
                        tfSocketRBKeyEqualCB,
                        genParam);
#else /* !TM_USE_SOCKET_HASH_LIST */
                reuseAddrSocketPtr = (ttSocketEntryPtr)(ttVoidPtr)
                      tfListWalk(
                        &(tm_context(tvSocketTableList[TM_SOTB_NON_CON])),
                        tfSocketRBKeyEqualCB,
                        genParam);
#endif /* !TM_USE_SOCKET_HASH_LIST */
#endif /* !TM_USE_SOCKET_RB_TREE */
                if (reuseAddrSocketPtr != TM_SOCKET_ENTRY_NULL_PTR
#ifdef TM_USE_IPDUAL
/* IPv6-only sockets do not support IPv4 */
                &&  !(  tm_16bit_one_bit_set(reuseAddrSocketPtr->soc6Flags,
                                                            TM_6_SOCF_V6ONLY)
                    &&  (tupleDevPtr->tudAddressFamily == AF_INET) )
#endif /* TM_USE_IPDUAL */
                    )
/*
 * local IP port, peer IP port, local address, remote address and protocol
 * match.
 * This is not an IPv4 tuple query on an IPv6-only socket.
 * Check device match.
 */

                {
/*
 * We found a match on the port.
 * This is not an IPv4 packet being received by an IPv6-only socket.
 * Try and find a matched socket for localIpAddress.
 */
#ifdef TM_USE_REUSEADDR_LIST
                    socketPtr = tfSocketReuseMatch(
#if (defined(TM_USE_STRONG_ESL) || defined(TM_IGMP) || defined(TM_6_USE_MLD))
                                                    devPtr,
#endif /* TM_USE_STRONG_ESL || TM_IGMP || TM_6_USE_MLD */
                                                    reuseAddrSocketPtr,
                                                    sockTuplePtr,
                                                    flag);
#else /* !TM_USE_REUSEADDR_LIST */
                    if ( tm_16bit_bits_not_set(reuseAddrSocketPtr->socCoFlags,
                                               TM_SOC_TREE_NO_MATCH_FLAG) )
                    {
#ifdef TM_USE_STRONG_ESL
                        if (  (tm_8bit_bits_not_set(reuseAddrSocketPtr->socFlags2,
                                                    TM_SOCF2_BINDTODEVICE))
                            || (reuseAddrSocketPtr
                                          ->socRteCacheStruct.rtcDevPtr == devPtr)
                           )
#endif /* TM_USE_STRONG_ESL */
                        {
                            if (
/* make sure we check remote values for connected sockets */
                                    (!(reuseAddrSocketPtr->socFlags
                                                             & TM_SOCF_CONNECTED))
                                 || ( (   sockTuplePtr->sotRemoteIpPort
                                       == reuseAddrSocketPtr->socPeerLayer4Port) &&
#ifdef TM_USE_IPV6
                                      (tm_6_ip_match(
                                          &(sockTuplePtr->sotRemoteIpAddress),
                                          &(reuseAddrSocketPtr->socPeerIpAddress)))
#else /* !TM_USE_IPV6 */
                                      (tm_ip_match(
                                            sockTuplePtr->sotRemoteIpAddress,
                                            reuseAddrSocketPtr->socPeerIpAddress))
#endif /* !TM_USE_IPV6 */
                                    )
                               )
                            {
                                if (
#ifdef TM_USE_IPV6
                                        tm_6_ip_match(
                                          &(reuseAddrSocketPtr->socOurIfIpAddress),
                                          &(sockTuplePtr->sotLocalIpAddress))
#else /* !TM_USE_IPV6 */
                                        tm_ip_match(
                                          reuseAddrSocketPtr->socOurIfIpAddress,
                                          sockTuplePtr->sotLocalIpAddress)
#endif /* !TM_USE_IPV6 */
                                )
                                {
/* Found it */
                                    socketPtr = reuseAddrSocketPtr;
                                }
/*
 * If we are not looking for an exact match.
 */
                                else if ( tm_16bit_bits_not_set(
                                                flag, TM_SOC_TREE_EXACT_MATCH) )
                                {
                                    if (
#ifdef TM_USE_IPV6
                                         tm_6_ip_zero(
                                             &reuseAddrSocketPtr->socOurIfIpAddress)
#else /* !TM_USE_IPV6 */
                                         tm_ip_zero(
                                            reuseAddrSocketPtr->socOurIfIpAddress)
#endif /* !TM_USE_IPV6 */
                                    )
                                    {
/*
 * Use socket with zero source IP address since we did not
 * find a better match (i.e. exact match).
 */
                                        socketPtr = reuseAddrSocketPtr;
                                    }
#ifdef TM_USE_IPDUAL
                                    else if (    IN6_IS_ADDR_V4MAPPED(
                                                &(sockTuplePtr->sotLocalIpAddress))
                                              && tm_6_addr_ipv4_mapped_zero(
                                           &(reuseAddrSocketPtr->socOurIfIpAddress))
                                            )
                                    {
/* When a IPv4 packet is received, the stack first tries to find a match on the
   specific destination IP address and port#, if that fails then the IPv4
   wildcard address and port#, if that fails then the IPv6 wildcard address
   and port#. */
/* use the IPv4 wildcard address as a default in case we do not find an
   exact match on the local IP address */
                                        socketPtr = reuseAddrSocketPtr;
                                    }
#endif /* TM_USE_IPDUAL */
                                    else if (tm_16bit_one_bit_set(flag,
                                                  TM_SOC_TREE_M_B_CAST_MATCH))
                                    {
/*
 * For multicast, or broadcast destination IP address:
 * use any socket (non connection oriented sockets only).
 */
#ifdef TM_USE_IPDUAL
                                        if (!IN6_IS_ADDR_V4MAPPED(
                                         &(reuseAddrSocketPtr->socOurIfIpAddress)))
/* Local IPv6 address */
#endif /* TM_USE_IPDUAL */
                                        {
                                            socketPtr = reuseAddrSocketPtr;
                                        }
#ifdef TM_USE_IPDUAL
                                        else if (IN6_IS_ADDR_V4MAPPED(
                                             &(sockTuplePtr->sotLocalIpAddress)))
                                        {
/* Both IPv4 addresses. */
                                            socketPtr = reuseAddrSocketPtr;
                                        }
#endif /* TM_USE_IPDUAL */
                                    }
                                }
#if (defined(TM_IGMP) || defined(TM_6_USE_MLD))
                                if (    (tm_16bit_one_bit_set(flag,
                                                              TM_SOC_TREE_MCAST_MATCH))
                                     && (socketPtr != (ttSocketEntryPtr)0)
                                   )
/* Check filters */
                                {
                                    mcastMember = tfSocketMcastMember(socketPtr
                                                , devPtr
#ifdef TM_USE_IPV6
                                                , &(sockTuplePtr->sotLocalIpAddress)
#else /* !TM_USE_IPV6 */
                                                , sockTuplePtr->sotLocalIpAddress
#endif /* !TM_USE_IPV6 */
#if (defined(TM_USE_IGMPV3) || defined(TM_6_USE_MLDV2))
#ifdef TM_USE_IPV6
                                                , &(sockTuplePtr->sotRemoteIpAddress)
#else /* !TM_USE_IPV6 */
                                                , sockTuplePtr->sotRemoteIpAddress
#endif /* !TM_USE_IPV6 */
#endif /* TM_USE_IGMPV3 || TM_6_USE_MLDV2 */
#ifdef TM_LOCK_NEEDED
                                                , TM_8BIT_NO
#endif /* TM_LOCK_NEEDED */
                                                );
                                    if (!mcastMember)
                                    {
                                        socketPtr = (ttSocketEntryPtr)0;
                                    }
                                }
#endif /* TM_IGMP || TM_6_USE_MLD */
                            }
                        }
                    }
#endif /* !TM_USE_REUSEADDR_LIST */
                }
#ifdef TM_USE_IPDUAL
/* If the search failed to find an IPv4 socket, try to find an IPv6 socket
 * that may support IPv4-mapped addresses. */
                if (    (socketPtr == TM_SOCKET_ENTRY_NULL_PTR)
                     && (addressFamilyTemp != AF_INET6) )
                {
                    addressFamilyTemp = AF_INET6;
                    dualAddressFamilyTry = TM_8BIT_YES;
                }
#endif /* TM_USE_IPDUAL */
#ifdef TM_USE_IPDUAL
            } while (dualAddressFamilyTry != TM_8BIT_ZERO);
#endif /* TM_USE_IPDUAL */
            if (    (socketPtr == TM_SOCKET_ENTRY_NULL_PTR)
                 && (portTemp != TM_WILD_PORT) )
            {
/*
 * Socket not found. Check for wild port listener.
 */
                portTemp = TM_WILD_PORT; /* 32-bit */
                wildPortTry = TM_8BIT_YES;
            }
        } while (wildPortTry != TM_8BIT_ZERO);
    }
    if (socketPtr != (ttSocketEntryPtr)0)
    {
        tm_lock_wait(&socketPtr->socLockEntry);
/*
 * Check out the socket, in case socket need to be unlocked/locked.
 * (Receiving thread owns the socket.)
 */
        tm_socket_checkout(socketPtr);
    }
	if( addressFamilyTemp ){
		;
	}
/* UNLOCK the root */
    tm_unlock(&tm_context(tvSocketRootLock));
/* Return LOCKED and checked out socket */
    return (socketPtr);
}

#ifdef TM_USE_REUSEADDR_LIST
static ttSocketEntryPtr tfSocketReuseMatch(
#if (defined(TM_USE_STRONG_ESL) || defined(TM_IGMP) || defined(TM_6_USE_MLD))
                                     ttDeviceEntryPtr   devPtr,
#endif /* TM_USE_STRONG_ESL || TM_IGMP || TM_6_USE_MLD */
                                     ttSocketEntryPtr   reuseAddrSocketPtr,
                                     ttSockTuplePtr     sockTuplePtr,
                                     tt16Bit            flag )
{
    ttSocketEntryPtr socketPtr;
#if (defined(TM_IGMP) || defined(TM_6_USE_MLD))
    tt8Bit           mcastMember;
#endif /* TM_IGMP || TM_6_USE_MLD */

    socketPtr = (ttSocketEntryPtr)0;
/* Try and find a matched socket for localIpAddress */
    while (reuseAddrSocketPtr != TM_SOCKET_ENTRY_NULL_PTR)
    {
        if ( tm_16bit_bits_not_set(reuseAddrSocketPtr->socCoFlags,
                                   TM_SOC_TREE_NO_MATCH_FLAG) )
/* socket can be found as a match */
        {
#ifdef TM_USE_STRONG_ESL
            if (  (tm_8bit_bits_not_set(reuseAddrSocketPtr->socFlags2,
                                        TM_SOCF2_BINDTODEVICE))
                || (reuseAddrSocketPtr->socRteCacheStruct.rtcDevPtr == devPtr)
               )
#endif /* TM_USE_STRONG_ESL */
            {
                if (
/* make sure we check remote values for connected sockets */
                        (!(reuseAddrSocketPtr->socFlags & TM_SOCF_CONNECTED))
                     || (  (sockTuplePtr->sotRemoteIpPort
                            == reuseAddrSocketPtr->socPeerLayer4Port) &&
#ifdef TM_USE_IPV6
                           (tm_6_ip_match(
                                      &(sockTuplePtr->sotRemoteIpAddress),
                                      &(reuseAddrSocketPtr->socPeerIpAddress)))
#else /* !TM_USE_IPV6 */
                           (tm_ip_match(sockTuplePtr->sotRemoteIpAddress,
                                        reuseAddrSocketPtr->socPeerIpAddress))
#endif /* !TM_USE_IPV6 */
                        )
                   )
                {
#if (defined(TM_IGMP) || defined(TM_6_USE_MLD))
                    if (tm_16bit_one_bit_set(flag, TM_SOC_TREE_MCAST_MATCH))
/* Check filters */
                    {
                        mcastMember = tfSocketMcastMember(reuseAddrSocketPtr
                                            , devPtr
#ifdef TM_USE_IPV6
                                            , &(sockTuplePtr->sotLocalIpAddress)
#else /* !TM_USE_IPV6 */
                                            , sockTuplePtr->sotLocalIpAddress
#endif /* !TM_USE_IPV6 */
#if (defined(TM_USE_IGMPV3) || defined(TM_6_USE_MLDV2))
#ifdef TM_USE_IPV6
                                            , &(sockTuplePtr->sotRemoteIpAddress)
#else /* !TM_USE_IPV6 */
                                            , sockTuplePtr->sotRemoteIpAddress
#endif /* !TM_USE_IPV6 */
#endif /* TM_USE_IGMPV3 || TM_6_USE_MLDV2 */
#ifdef TM_LOCK_NEEDED
                                           , TM_8BIT_NO
#endif /* TM_LOCK_NEEDED */
                                           );
                        if (!mcastMember)
                        {
                            reuseAddrSocketPtr =
                                reuseAddrSocketPtr->socReuseAddrNextPtr;
/* Filtered. Skip this socket */
                            continue;
                        }
                    }
#endif /* TM_IGMP || TM_6_USE_MLD */
#ifdef TM_USE_IPV6
                    if ( tm_6_ip_match(&reuseAddrSocketPtr->socOurIfIpAddress,
                                       &(sockTuplePtr->sotLocalIpAddress)) )
#else /* !TM_USE_IPV6 */
                    if ( tm_ip_match(reuseAddrSocketPtr->socOurIfIpAddress,
                                     sockTuplePtr->sotLocalIpAddress) )
#endif /* !TM_USE_IPV6 */
                    {
/* Found it in the list of reuse address sockets */
                        socketPtr = reuseAddrSocketPtr;
                        break;
                    }
/*
 * If there is a socket without any IP address set, and we are not looking
 * for an exact match.
 */
                    else if (tm_16bit_bits_not_set(
                                               flag, TM_SOC_TREE_EXACT_MATCH))
                    {
                        if (
#ifdef TM_USE_IPV6
                             tm_6_ip_zero(
                                    &(reuseAddrSocketPtr->socOurIfIpAddress))
#else /* !TM_USE_IPV6 */
                             tm_ip_zero(
                                    reuseAddrSocketPtr->socOurIfIpAddress)
#endif /* !TM_USE_IPV6 */
                           )
                        {
#ifdef TM_USE_IPDUAL /* dual */
                            if (socketPtr == TM_SOCKET_ENTRY_NULL_PTR)
#endif /* TM_USE_IPDUAL */
                            {
/* use the IPv6 wildcard address as a default in case we do not find a
   better match */
                                socketPtr = reuseAddrSocketPtr;
                                if (tm_16bit_one_bit_set(
                                            flag, TM_SOC_TREE_M_B_CAST_MATCH))
                                {
/* For multicast/broadcast use all matching sockets */
                                    break;
                                }
                            }
                        }
#ifdef TM_USE_IPDUAL
                        else if (    IN6_IS_ADDR_V4MAPPED(
                                           &(sockTuplePtr->sotLocalIpAddress))
                                  && tm_6_addr_ipv4_mapped_zero(
                                      &reuseAddrSocketPtr->socOurIfIpAddress))
                        {
/* When a IPv4 packet is received, the stack first tries to find a match on the
   specific destination IP address and port#, if that fails then the IPv4
   wildcard address and port#, if that fails then the IPv6 wildcard address
   and port#. */
/* use the IPv4 wildcard address as a default in case we do not find an
   exact match on the local IP address */
                            socketPtr = reuseAddrSocketPtr;
                            if (tm_16bit_one_bit_set(
                                        flag, TM_SOC_TREE_M_B_CAST_MATCH))
                            {
/* For multicast/broadcast use all matching sockets */
                                break;
                            }
                        }
#endif /* TM_USE_IPDUAL  */
                        else if (  (socketPtr == TM_SOCKET_ENTRY_NULL_PTR) &&
                                   (tm_16bit_one_bit_set(
                                        flag, TM_SOC_TREE_M_B_CAST_MATCH)) &&
                                   (reuseAddrSocketPtr->socCoFlags 
                                                            == TM_16BIT_ZERO)
                                )
                        {
/* 
 * For multicast, or broadcast destination IP address:
 * use any socket (non connection oriented sockets only).
 */
#ifdef TM_USE_IPDUAL
                            if (!IN6_IS_ADDR_V4MAPPED(
                                    &reuseAddrSocketPtr->socOurIfIpAddress))
/* Local IPv6 address */
#endif /* TM_USE_IPDUAL */
                            {
                                socketPtr = reuseAddrSocketPtr;
                                break;
                            }
#ifdef TM_USE_IPDUAL
                            else if (IN6_IS_ADDR_V4MAPPED(
                                           &(sockTuplePtr->sotLocalIpAddress)))
                            {
/* Both IPv4 addresses. */
                                socketPtr = reuseAddrSocketPtr;
                                break;
                            }
#endif /* TM_USE_IPDUAL */
                        }
                    }
                }
            }
        }
        reuseAddrSocketPtr =
                reuseAddrSocketPtr->socReuseAddrNextPtr;
    }
    return socketPtr;
}
#endif /* TM_USE_REUSEADDR_LIST */


/* 
 * Delete a socket entry from the tree or from the list of reuse addresses.
 * Parameter socketPtr has been locked and checked out prior to the call
 */
int tfSocketTreeDelete ( ttSocketEntryPtr socketPtr, tt16Bit flag )
{
#ifdef TM_USE_REUSEADDR_LIST
    ttSocketEntryPtr    reuseAddrSocketPtr;
    ttSocketEntryPtr    prevReuseAddrSocketPtr;
    ttSocketEntryPtr    nextReuseAddrSocketPtr;
#endif /* TM_USE_REUSEADDR_LIST */
    int                 index;
    int                 searchIndex;
    int                 retCode;

    if (tm_16bit_bits_not_set(flag, TM_SOC_TREE_LOCKED_FLAG))
    {
/* Unlock socket entry to prevent dead lock with socketrootlock */
        tm_call_unlock(&socketPtr->socLockEntry);
/* LOCK the root */
        tm_call_lock_wait(&tm_context(tvSocketRootLock));
/* Relock the socket entry */
        tm_call_lock_wait(&socketPtr->socLockEntry);
    }
    if (tm_16bit_one_bit_set(socketPtr->socFlags, TM_SOCF_IN_TREE))
    {
#ifdef TM_USE_TCP
        if (((int)socketPtr->socProtocolNumber) == IP_PROTOTCP)
        {
            index = TM_SOTB_TCP_CON;
            if (tm_16bit_one_bit_set(socketPtr->socCoFlags,
                                     TM_SOC_TREE_CO_FLAG))
            {
/* Connected TCP search */
                searchIndex = TM_SOTB_TCP_CON;
            }
            else
            {
/* Non connected TCP search */
                searchIndex = TM_SOTB_NUM;
            }
        }
        else
#endif /* TM_USE_TCP */
        {
            index = TM_SOTB_NON_CON;
/* Non TCP search */
            searchIndex = TM_SOTB_NON_CON;
        }
#ifdef TM_USE_REUSEADDR_LIST
        if (socketPtr->socInReuseAddrList == TM_8BIT_NO)
        {
/*
 * If our socket entry is in the tree remove it. If the list of socket entries
 * sharing the same local IP address is non empty, insert the first entry
 * in the tree
 */
            reuseAddrSocketPtr = socketPtr->socReuseAddrNextPtr;
            tm_socket_delete(socketPtr, index, searchIndex);
            if (reuseAddrSocketPtr != TM_SOCKET_ENTRY_NULL_PTR)
            {
                reuseAddrSocketPtr->socInReuseAddrList = TM_8BIT_NO;
                (void)tm_socket_insert(reuseAddrSocketPtr,
                                       &(reuseAddrSocketPtr->socTuple));
            }
        }
        else
        {
/*
 * If entry is in the list of reuse address entries, remove it from that
 * list.
 */
            prevReuseAddrSocketPtr = socketPtr->socReuseAddrPrevPtr;
            nextReuseAddrSocketPtr = socketPtr->socReuseAddrNextPtr;
#ifdef TM_ERROR_CHECKING
            if (prevReuseAddrSocketPtr == (ttSocketEntryPtr)0)
            {
                tfKernelError("tfSocketTreeDelete",
                              "Corrupted reuseaddress list");
            }
            else
#endif /* TM_ERROR_CHECKING */
            {
                prevReuseAddrSocketPtr->socReuseAddrNextPtr =
                    nextReuseAddrSocketPtr;
            }
            if (nextReuseAddrSocketPtr != (ttSocketEntryPtr)0)
            {
                nextReuseAddrSocketPtr->socReuseAddrPrevPtr =
                    prevReuseAddrSocketPtr;
            }
        }
        socketPtr->socInReuseAddrList = TM_8BIT_NO;
#else /* !TM_USE_REUSEADDR_LIST */
        tm_socket_delete(socketPtr, index, searchIndex);
#endif /* TM_USE_REUSEADDR_LIST */
        tm_16bit_clr_bit( socketPtr->socFlags, TM_SOCF_IN_TREE );
        retCode = TM_SOC_RB_OKAY; /* removed from the tree */
#ifdef TM_SNMP_CACHE
        if (socketPtr->socSnmpCacheEntry.sceNode.nodeNextPtr != (ttVoidPtr)0)
        {
            tfSnmpdCacheDeleteSocket(socketPtr);
        }
#endif /* TM_SNMP_CACHE */
        socketPtr->socOwnerCount--;
    }
    else
    {
        retCode = TM_SOC_RB_NOT_FOUND; /* not in the tree */
    }
    if ( tm_16bit_bits_not_set(flag,   TM_SOC_TREE_LOCKED_FLAG
                                     | TM_SOC_TREE_KEEP_LOCK_FLAG ) )
    {
/*
 * UNLOCK the root, if either tree was not locked prior to calling
 * this routine, or if caller does not want the tree to be kept locked upon
 * return.
 */
        tm_call_unlock(&tm_context(tvSocketRootLock));
    }
    return(retCode);
}

/*
 * The following are the primitive functions for access into
 * the red-black tree
 * They are left as primitives so the user can opt to remove them
 * by not defining TM_USE_SOCKET_RB_TREE
 */

#ifdef TM_USE_SOCKET_RB_TREE

#ifdef TM_USE_TCP
/*
 * Lookup a TCP socket in the red-black tree
 */
static ttSocketEntryPtr tfSocketRbLookup (ttSockTuplePtr sockTuplePtr)
{
    ttSocketEntryPtr socketPtr;
    ttSocketEntryPtr retSocketPtr;
#ifdef TM_USE_SOCKET_HASH_LIST
    tt32Bit          hash;
#endif /* !TM_USE_SOCKET_HASH_LIST */
    int              retCode;

    retSocketPtr = TM_SOCKET_ENTRY_NULL_PTR;
#ifdef TM_USE_SOCKET_HASH_LIST
    hash = tfSocketHash(
                    (ttCharPtr)(ttVoidPtr)sockTuplePtr,
                    sizeof(ttSockTuple),
                    tm_context(tvSocketTableListHashSize[TM_SOTB_TCP_CON]));
    socketPtr = tm_context(tvSocketTableRootPtrPtr[TM_SOTB_TCP_CON])[hash];
#else /* !TM_USE_SOCKET_HASH_LIST */
    socketPtr = tm_context(tvSocketTableRootPtr[TM_SOTB_TCP_CON]);
#endif /* !TM_USE_SOCKET_HASH_LIST */
/* TCP search. */
    while (socketPtr != TM_SOCKET_ENTRY_NULL_PTR)
    {
        retCode = tm_tupleptr_cmp(&(socketPtr->socTuple), sockTuplePtr);
        if (retCode == TM_STRCMP_EQUAL)
        {
            retSocketPtr = socketPtr;
            break;
        }
        else
        {
            if (retCode <= TM_STRCMP_LESS)
            {
/* If we are less, then the socket must be to the right */
                socketPtr = socketPtr->socRbRightSocketPtr;
            }
            else
            {
/* We are greater, then the socket must be to the left */
                socketPtr = socketPtr->socRbLeftSocketPtr;
            }
        }
    }
    return retSocketPtr;
}
#endif /* TM_USE_TCP */

/*
 * Lookup a non TCP socket in the red-black tree.
 *
 * For dual IPv4/IPv6 systems that are not using a socket hash table, there
 * is a performance enhancement in the form of a "relaxed" key comparison.
 * When an exact match is not found, this function can return a close
 * match, ignoring address family. This will save an extra walk through the
 * tree with an alternate key that is based on a different address family.
 * This is possible only because the address family occupies the least
 * significant byte in the key, so sockets that differ only in address family
 * will be adjacent in the tree.
 * If a socket hash table is used, sockets that differ only in address family
 * may hash to a different tree, so an extra walk will probably be required.
 */
static ttSocketEntryPtr tfSocketRbClLookup (tt32Bit redBlackKey)
{
    ttSocketEntryPtr socketPtr;
    ttSocketEntryPtr retSocketPtr;
    tt32Bit          socketRedBlackKey;
#ifdef TM_USE_SOCKET_HASH_LIST
    tt32Bit          hash;
#else /* !TM_USE_SOCKET_HASH_LIST */
#ifdef TM_USE_IPDUAL
    tt32Bit          redBlackKeyRelaxed;
#endif /* TM_USE_IPDUAL */
#endif /* !TM_USE_SOCKET_HASH_LIST */

    retSocketPtr = TM_SOCKET_ENTRY_NULL_PTR;
#ifdef TM_USE_SOCKET_HASH_LIST
    hash = tfSocketHash(
                    (ttCharPtr)(ttVoidPtr)&redBlackKey,
                    sizeof(tt32Bit),
                    tm_context(tvSocketTableListHashSize[TM_SOTB_NON_CON]));
    socketPtr = tm_context(tvSocketTableRootPtrPtr[TM_SOTB_NON_CON])
                                                  [hash];
#else /* !TM_USE_SOCKET_HASH_LIST */
    socketPtr = tm_context(tvSocketTableRootPtr[TM_SOTB_NON_CON]);
#ifdef TM_USE_IPDUAL
    redBlackKeyRelaxed = TM_RB_KEYRELAX(redBlackKey);
#endif /* TM_USE_IPDUAL */
#endif /* !TM_USE_SOCKET_HASH_LIST */
/* TCP Connection-less, non TCP sockets search */
    while (socketPtr != TM_SOCKET_ENTRY_NULL_PTR)
    {
        socketRedBlackKey = socketPtr->socRedBlackKey;
        if (socketRedBlackKey == redBlackKey)
        {
/* local IP port, protocol and address family match */
            retSocketPtr = socketPtr;
/* Exit while loop */
            break;
        }
#if defined(TM_USE_IPDUAL) && !defined(TM_USE_SOCKET_HASH_LIST)
        if (TM_RB_KEYRELAX(socketRedBlackKey) == redBlackKeyRelaxed)
        {
/* local IP port, and protocol match; return this socket pointer unless we
 * find a better fit */
            retSocketPtr = socketPtr;
        }
#endif /* TM_USE_IPDUAL && !TM_USE_SOCKET_HASH_LIST */
        if (socketRedBlackKey < redBlackKey)
        {
/* If we are less, then the socket must be to the right */
            socketPtr = socketPtr->socRbRightSocketPtr;
        }
        else
        {
/* We are greater, then the socket must be to the left */
            socketPtr = socketPtr->socRbLeftSocketPtr;
        }
    }
    return retSocketPtr;
}

/*
 * Transform two socket entries by performing a rotate left
 */
static void tfSocketRbLeftRotate (ttSocketEntryPtrPtr socketRootPtrPtr,
                                  ttSocketEntryPtr    socketPtr)
{
    ttSocketEntryPtr tempSocketEntryPtr;

/* Since we are rotating left we need this entry and its right entry */
    tempSocketEntryPtr = socketPtr->socRbRightSocketPtr;
/* turn tempSocketEntry's left subtree into socketEntry's right subtree */
    socketPtr->socRbRightSocketPtr = tempSocketEntryPtr->socRbLeftSocketPtr;
    if (tempSocketEntryPtr->socRbLeftSocketPtr != TM_SOCKET_ENTRY_NULL_PTR)
    {
        tempSocketEntryPtr->socRbLeftSocketPtr->socRbParentSocketPtr =
                                                            socketPtr;
    }
/* Link socketEntry's parent to tempSocketEntry */
    tempSocketEntryPtr->socRbParentSocketPtr =
                                        socketPtr->socRbParentSocketPtr;
    if (socketPtr->socRbParentSocketPtr == TM_SOCKET_ENTRY_NULL_PTR)
    {
        *socketRootPtrPtr = tempSocketEntryPtr;
    }
    else
    {
        if(socketPtr == socketPtr->socRbParentSocketPtr->socRbLeftSocketPtr)
        {
            socketPtr->socRbParentSocketPtr->socRbLeftSocketPtr =
                                                         tempSocketEntryPtr;
        }
        else
        {
            socketPtr->socRbParentSocketPtr->socRbRightSocketPtr =
                                                         tempSocketEntryPtr;
        }
    }
/* Put socketEntry to tempSocketEntry's left */
    tempSocketEntryPtr->socRbLeftSocketPtr = socketPtr;
    socketPtr->socRbParentSocketPtr = tempSocketEntryPtr;
}

/*
 * Transform two socket entries by performing a rotate right
 */
static void tfSocketRbRightRotate (ttSocketEntryPtrPtr socketRootPtrPtr,
                                   ttSocketEntryPtr    socketPtr )
{
    ttSocketEntryPtr tempSocketEntryPtr;

/* Since we are rotating right we need this entry and its left entry */
    tempSocketEntryPtr = socketPtr->socRbLeftSocketPtr;
/* turn tempSocketEntry's right subtree into socketEntry's left subtree */
    socketPtr->socRbLeftSocketPtr = tempSocketEntryPtr->socRbRightSocketPtr;
    if (tempSocketEntryPtr->socRbRightSocketPtr != TM_SOCKET_ENTRY_NULL_PTR)
    {
        tempSocketEntryPtr->socRbRightSocketPtr->socRbParentSocketPtr =
                                                            socketPtr;
    }
/* Link socketEntry's parent to tempSocketEntry */
    tempSocketEntryPtr->socRbParentSocketPtr =
                                        socketPtr->socRbParentSocketPtr;
    if (socketPtr->socRbParentSocketPtr == TM_SOCKET_ENTRY_NULL_PTR)
    {
        *socketRootPtrPtr = tempSocketEntryPtr;
    }
    else
    {
        if(    socketPtr
            == socketPtr->socRbParentSocketPtr->socRbRightSocketPtr)
        {
            socketPtr->socRbParentSocketPtr->socRbRightSocketPtr =
                                                        tempSocketEntryPtr;
        }
        else
        {
            socketPtr->socRbParentSocketPtr->socRbLeftSocketPtr =
                                                        tempSocketEntryPtr;
        }
    }
/* Put socketEntry to tempSocketEntry's Right */
    tempSocketEntryPtr->socRbRightSocketPtr = socketPtr;
    socketPtr->socRbParentSocketPtr = tempSocketEntryPtr;
}

/*
 * Insert a socket entry into the red-black tree
 */
static ttSocketEntryPtr tfSocketRbInsert (ttSocketEntryPtr    socketPtr
#ifdef TM_USE_TCP
                                        , ttSockTuplePtr      sockTuplePtr
#endif /* TM_USE_TCP */
                                         )
{
    ttSocketEntryPtrPtr socketRootPtrPtr;
    ttSocketEntryPtr    tempSocketPtr;
    ttSocketEntryPtr    grandParentSocketPtr;
    ttSocketEntryPtr    retSocketPtr;
#ifdef TM_USE_SOCKET_HASH_LIST
    ttCharPtr           keyPtr;
#endif /* TM_USE_SOCKET_HASH_LIST */
    tt32Bit             redBlackKey;
#ifdef TM_USE_SOCKET_HASH_LIST
    unsigned int        depth;
    int                 keyLen;
#endif /* TM_USE_SOCKET_HASH_LIST */
    int                 index;
    int                 searchIndex;
    int                 errorCode;
    int                 retCode;
    
    errorCode = TM_SOC_RB_OKAY;

#ifdef TM_USE_SOCKET_HASH_LIST
    depth = 1;
#endif /* TM_USE_SOCKET_HASH_LIST */
/* BUG ID 11-632: initialize left and right child pointers to NULL */
    socketPtr->socRbLeftSocketPtr = TM_SOCKET_ENTRY_NULL_PTR;
    socketPtr->socRbRightSocketPtr = TM_SOCKET_ENTRY_NULL_PTR;
    socketPtr->socRbParentSocketPtr = TM_SOCKET_ENTRY_NULL_PTR;

    retSocketPtr = (ttSocketEntryPtr)0;
#ifdef TM_USE_TCP
    if (((int)socketPtr->socProtocolNumber) == IP_PROTOTCP)
    {
        redBlackKey = TM_32BIT_ZERO; /* compiler warning */
        index = TM_SOTB_TCP_CON;
        if (tm_16bit_one_bit_set(socketPtr->socCoFlags,
                                 TM_SOC_TREE_CO_FLAG))
        {
/* Connected TCP search */
            searchIndex = TM_SOTB_TCP_CON;
        }
        else
        {
/* Non connected TCP search */
            searchIndex = TM_SOTB_NUM;
        }
#ifdef TM_USE_SOCKET_HASH_LIST
        keyPtr = (ttCharPtr)(ttVoidPtr)sockTuplePtr;
        keyLen = sizeof(ttSockTuple);
#endif /* TM_USE_SOCKET_HASH_LIST */
    }
    else
#endif /* TM_USE_TCP */
    {
        index = TM_SOTB_NON_CON;
/* Non TCP search */
        searchIndex = TM_SOTB_NON_CON;
        redBlackKey = socketPtr->socRedBlackKey;
#ifdef TM_USE_SOCKET_HASH_LIST
        keyPtr = (ttCharPtr)(ttVoidPtr)&(redBlackKey);
        keyLen = sizeof(tt32Bit);
#endif /* TM_USE_SOCKET_HASH_LIST */
    }
#ifdef TM_USE_SOCKET_HASH_LIST
    socketPtr->socHash = tfSocketHash(
                                keyPtr, keyLen,
                                tm_context(tvSocketTableListHashSize[index]));
    socketRootPtrPtr = &(tm_context(tvSocketTableRootPtrPtr[index])
                                                        [socketPtr->socHash]);
#else /* !TM_USE_SOCKET_HASH_LIST */
    socketRootPtrPtr = &tm_context(tvSocketTableRootPtr[index]);
#endif /* !TM_USE_SOCKET_HASH_LIST */
    tempSocketPtr = *socketRootPtrPtr;
    while (tempSocketPtr != TM_SOCKET_ENTRY_NULL_PTR)
    {
/* Compare socketPtr, with tempSocketPtr:
 * retCode ==
 * . TM_STRCMP_EQUAL
 *       tempSocketPtr and socketPtr are equal
 * . TM_STRCMP_LESS
 *       tempSocketPtr is less than socketPtr
 * . TM_STRCMP_GREATER
 *       tempSocketPtr is greater than socketPtr
 */
#ifdef TM_USE_TCP
        if (index == TM_SOTB_TCP_CON)
        {
            retCode = tm_tupleptr_cmp(&tempSocketPtr->socTuple,
                                      sockTuplePtr);
        }
        else
#endif /* TM_USE_TCP */
        {
            retCode = tm_32bit_cmp(tempSocketPtr->socRedBlackKey,
                                   redBlackKey);
        }
#ifdef TM_USE_SOCKET_HASH_LIST
        depth++;
#endif /* TM_USE_SOCKET_HASH_LIST */
        if (retCode == TM_STRCMP_EQUAL)
        {
/* Already Exists and it should not! */
            errorCode = TM_SOC_RB_ALREADY;
            retSocketPtr = tempSocketPtr;
            break;
        }
        if (retCode <= TM_STRCMP_LESS)
        {
/* If we are less, then the socket must go to the right */
            if (tempSocketPtr->socRbRightSocketPtr
                                                == TM_SOCKET_ENTRY_NULL_PTR)
            {
                tempSocketPtr->socRbRightSocketPtr = socketPtr;
                socketPtr->socRbParentSocketPtr = tempSocketPtr;
/* It is inserted so lets get out of the loop */
                break;
            }
            else
            {
/* Continue to look down the tree */
                tempSocketPtr = tempSocketPtr->socRbRightSocketPtr;
            }
        }
        else
        {
/* We are greater, then the socket must go to the left */
            if (tempSocketPtr->socRbLeftSocketPtr == TM_SOCKET_ENTRY_NULL_PTR)
            {
                tempSocketPtr->socRbLeftSocketPtr = socketPtr;
                socketPtr->socRbParentSocketPtr = tempSocketPtr;
/* It is inserted so lets get out of the loop */
                break;
            }
            else
            {
/* Continue to look down the tree */
                tempSocketPtr = tempSocketPtr->socRbLeftSocketPtr;
            }
        }
    }
    if (errorCode == TM_SOC_RB_OKAY)
    {
#ifdef TM_USE_SOCKET_HASH_LIST
        if (   depth
             > tm_context(tvSocketTableListHighDepth[index]))
        {
/* Remember the highest list depth for diagnosis */
            tm_context(tvSocketTableListHighDepth[index]) =
                                             depth;
        }
#endif /* TM_USE_SOCKET_HASH_LIST */
        if (tempSocketPtr == TM_SOCKET_ENTRY_NULL_PTR)
        {
/* Empty Tree */
            *socketRootPtrPtr = socketPtr;
        }
        socketPtr->socRbTreeColor = TM_RB_RED;
        while(socketPtr != *socketRootPtrPtr)
        {
            if (socketPtr->socRbParentSocketPtr->socRbTreeColor
                                                             == TM_RB_BLACK)
            {
/* This Entry now has Red-Black Qualities */
                break;
            }    
            grandParentSocketPtr =
                socketPtr->socRbParentSocketPtr->socRbParentSocketPtr;
            
            if (grandParentSocketPtr == TM_SOCKET_ENTRY_NULL_PTR)
            {
                break;
            }
            
            if (socketPtr->socRbParentSocketPtr ==
                grandParentSocketPtr->socRbLeftSocketPtr)
            {
                tempSocketPtr =
                    grandParentSocketPtr->socRbRightSocketPtr;
                if  (    (tempSocketPtr != TM_SOCKET_ENTRY_NULL_PTR)
                      && (tempSocketPtr->socRbTreeColor == TM_RB_RED) )
                {
                    socketPtr->socRbParentSocketPtr->socRbTreeColor =
                        TM_RB_BLACK;
                    tempSocketPtr->socRbTreeColor = TM_RB_BLACK;
                    grandParentSocketPtr->socRbTreeColor = TM_RB_RED;
                    socketPtr = grandParentSocketPtr;
                }
                else
                {
                    if (socketPtr ==
                        socketPtr->socRbParentSocketPtr->socRbRightSocketPtr)
                    {
                        socketPtr = socketPtr->socRbParentSocketPtr;
/* Rotate left */
                        tfSocketRbLeftRotate(socketRootPtrPtr,socketPtr);
                    }
                    socketPtr->socRbParentSocketPtr->socRbTreeColor =
                        TM_RB_BLACK;
                    socketPtr->socRbParentSocketPtr->
                        socRbParentSocketPtr->socRbTreeColor = TM_RB_RED;
/* Rotate Right */
                    tfSocketRbRightRotate(
                      socketRootPtrPtr,
                      socketPtr->socRbParentSocketPtr->socRbParentSocketPtr);
                }
            }
            else
            {
                tempSocketPtr = grandParentSocketPtr->socRbLeftSocketPtr;
                if (    (tempSocketPtr != TM_SOCKET_ENTRY_NULL_PTR)
                     && (tempSocketPtr->socRbTreeColor == TM_RB_RED) )
                {
                    socketPtr->socRbParentSocketPtr->socRbTreeColor =
                        TM_RB_BLACK;
                    tempSocketPtr->socRbTreeColor = TM_RB_BLACK;
                    grandParentSocketPtr->socRbTreeColor = TM_RB_RED;
                    socketPtr = grandParentSocketPtr;
                }
                else
                {
                    if (socketPtr ==
                        socketPtr->socRbParentSocketPtr->socRbLeftSocketPtr)
                    {
                        socketPtr = socketPtr->socRbParentSocketPtr;
/* Rotate Right */
                        tfSocketRbRightRotate(socketRootPtrPtr,socketPtr);
                    }
                    socketPtr->socRbParentSocketPtr->socRbTreeColor =
                        TM_RB_BLACK;
                    socketPtr->socRbParentSocketPtr->
                        socRbParentSocketPtr->socRbTreeColor = TM_RB_RED;
/* Rotate Left */
                    tfSocketRbLeftRotate(
                      socketRootPtrPtr,
                      socketPtr->socRbParentSocketPtr->socRbParentSocketPtr);
                }
            }
        }
/* Root always black */
        (*socketRootPtrPtr)->socRbTreeColor = TM_RB_BLACK;
/* Number of sockets in the tree */
        tm_context(tvSocketPortSearchMembers[searchIndex])++;
    }
    return(retSocketPtr);
}

/*
 * Socket Entry Delete
 */
static void tfSocketRbDelete (ttSocketEntryPtr socketPtr, int index)
{
    ttSocketEntryPtrPtr socketRootPtrPtr;
    ttSocketEntryPtr    wSocketEntryPtr;
    ttSocketEntryPtr    ySocketEntryPtr;
    ttSocketEntryPtr    xSocketEntryPtr;
    static ttSocketEntry       nullSocketEntry;
    static ttSocketEntry       newNullSocketEntry;
    tt8Bit              tempLeft;
    tt8Bit              tempRight;
    tt8Bit              isLeft;
    tt8Bit              yRbTreeColor;
    
#ifdef TM_USE_SOCKET_HASH_LIST
    socketRootPtrPtr = &(tm_context(tvSocketTableRootPtrPtr[index])
                                                        [socketPtr->socHash]);
#else /* !TM_USE_SOCKET_HASH_LIST */
    socketRootPtrPtr = &tm_context(tvSocketTableRootPtr[index]);
#endif /* !TM_USE_SOCKET_HASH_LIST */
    isLeft = 0;
    xSocketEntryPtr = TM_SOCKET_ENTRY_NULL_PTR;
    ySocketEntryPtr = TM_SOCKET_ENTRY_NULL_PTR;
    
/* ECR 03/06/01: redid entire delete algorithm */
/* If ( isSentinel(leftChild(node)) or isSentinel(rightChild(node)) ) */
    if ( (socketPtr->socRbLeftSocketPtr == TM_SOCKET_ENTRY_NULL_PTR)
         || (socketPtr->socRbRightSocketPtr == TM_SOCKET_ENTRY_NULL_PTR))
    {
/* { y = node } */
        ySocketEntryPtr = socketPtr;
    }
    else
    {
/* Else y = Tree_Successor(node) */
/* Neither child is NULL so search for the replacement */
        ySocketEntryPtr = socketPtr->socRbRightSocketPtr;
/* We want the minimum of the tree to the right */
        while (ySocketEntryPtr->socRbLeftSocketPtr !=
               TM_SOCKET_ENTRY_NULL_PTR)
        {
/* Go down the left side of this tree */
            ySocketEntryPtr = ySocketEntryPtr->socRbLeftSocketPtr;
        }
    }
    
/* x is y's only child */
/* If ( !isSentinel(leftChild(y)) ) */
    if (ySocketEntryPtr->socRbLeftSocketPtr != TM_SOCKET_ENTRY_NULL_PTR)
    {
/*  x = leftChild(y) */
        xSocketEntryPtr = ySocketEntryPtr->socRbLeftSocketPtr;
    }
    else
    {
/* Else x = rightChild(y) */
        xSocketEntryPtr = ySocketEntryPtr->socRbRightSocketPtr;
    }
    
/* remove y from the parent chain */
/* parent(x) = parent(y) */
    if (xSocketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR)
    {
        if ((ySocketEntryPtr != socketPtr) &&
            (ySocketEntryPtr->socRbParentSocketPtr == socketPtr))
        {
/* SPECIAL CASE: if y != node and parent(y) == node, then parent(x) should
   be set to y, since y replaces parent(y) in this case */
            xSocketEntryPtr->socRbParentSocketPtr = ySocketEntryPtr;
        }
        else
        {
            xSocketEntryPtr->socRbParentSocketPtr =
                ySocketEntryPtr->socRbParentSocketPtr;
        }
    }
    else
    {
        if ((ySocketEntryPtr != socketPtr) &&
            (ySocketEntryPtr->socRbParentSocketPtr == socketPtr))
        {
/* SPECIAL CASE: if y != node and parent(y) == node, then parent(x) should
   be set to y, since y replaces parent(y) in this case */
            newNullSocketEntry.socRbParentSocketPtr = ySocketEntryPtr;
        }
        else
        {
            newNullSocketEntry.socRbParentSocketPtr =
                ySocketEntryPtr->socRbParentSocketPtr;
        }
    }
    
/* if (isSentinel(parent(y))) */
    if (ySocketEntryPtr->socRbParentSocketPtr == TM_SOCKET_ENTRY_NULL_PTR)
    {
/* { root(Tree) = x} */
        *socketRootPtrPtr = xSocketEntryPtr;
    }
    else
    {
/*  If (y == leftChild(parent(y))) */
        if (ySocketEntryPtr->socRbParentSocketPtr->socRbLeftSocketPtr ==
            ySocketEntryPtr)
        {
/*      leftChild(parent(y)) = x */
            ySocketEntryPtr->socRbParentSocketPtr->socRbLeftSocketPtr =
                xSocketEntryPtr;
            isLeft = 1;
        }
        else
        {
/*  Else rightChild(parent(y)) = x */
            ySocketEntryPtr->socRbParentSocketPtr->socRbRightSocketPtr =
                xSocketEntryPtr;
        }
    }
    
    yRbTreeColor = ySocketEntryPtr->socRbTreeColor;
/* if (y != node) */
    if (ySocketEntryPtr != socketPtr)
    {
/*  key(node) = key(y) */
/*  delete node from the tree, and replace it with y */
        if (socketPtr->socRbParentSocketPtr != TM_SOCKET_ENTRY_NULL_PTR)
        {
            if (socketPtr->socRbParentSocketPtr->socRbLeftSocketPtr ==
                socketPtr)
            {
                socketPtr->socRbParentSocketPtr->socRbLeftSocketPtr =
                    ySocketEntryPtr;
            }
            else
            {
                socketPtr->socRbParentSocketPtr->socRbRightSocketPtr =
                    ySocketEntryPtr;
            }
        }
        else
        {
/* y becomes the new root of the tree */
            *socketRootPtrPtr = ySocketEntryPtr;
        }
        
/* link y to the deleted node's parent and children */
        ySocketEntryPtr->socRbParentSocketPtr = 
            socketPtr->socRbParentSocketPtr;
        ySocketEntryPtr->socRbLeftSocketPtr =
            socketPtr->socRbLeftSocketPtr;
        if (ySocketEntryPtr->socRbLeftSocketPtr != TM_SOCKET_ENTRY_NULL_PTR)
        {
            ySocketEntryPtr->socRbLeftSocketPtr->socRbParentSocketPtr =
                ySocketEntryPtr;
        }
        ySocketEntryPtr->socRbRightSocketPtr =
            socketPtr->socRbRightSocketPtr;
        if (ySocketEntryPtr->socRbRightSocketPtr != TM_SOCKET_ENTRY_NULL_PTR)
        {
            ySocketEntryPtr->socRbRightSocketPtr->socRbParentSocketPtr =
                ySocketEntryPtr;
        }
        
/* give y the same color as the deleted node it replaces */
        ySocketEntryPtr->socRbTreeColor = socketPtr->socRbTreeColor;
    }
    
    if ( (*socketRootPtrPtr != TM_SOCKET_ENTRY_NULL_PTR)
         && (yRbTreeColor == TM_RB_BLACK) )
    {
/* Fix up the tree */
        wSocketEntryPtr = TM_SOCKET_ENTRY_NULL_PTR;
        if (xSocketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR)
        {
            xSocketEntryPtr = &newNullSocketEntry;
            newNullSocketEntry.socRbRightSocketPtr = 0;
            newNullSocketEntry.socRbLeftSocketPtr = 0;
            newNullSocketEntry.socRbTreeColor = TM_RB_BLACK;
        }
        
        while ( (xSocketEntryPtr != *socketRootPtrPtr)
                && (xSocketEntryPtr->socRbTreeColor == TM_RB_BLACK) )
        {
            if ((xSocketEntryPtr
                 == xSocketEntryPtr->socRbParentSocketPtr->socRbLeftSocketPtr)
                || isLeft)
            {
/* We're a left child */
                isLeft = 0;
                wSocketEntryPtr = xSocketEntryPtr->
                    socRbParentSocketPtr->socRbRightSocketPtr;
                
                if (wSocketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR)
                {
                    wSocketEntryPtr = &nullSocketEntry;
                    nullSocketEntry.socRbParentSocketPtr = 
                        xSocketEntryPtr->socRbParentSocketPtr;
                    nullSocketEntry.socRbRightSocketPtr = 0;
                    nullSocketEntry.socRbLeftSocketPtr = 0;
                    nullSocketEntry.socRbTreeColor = TM_RB_BLACK;
                }
                
/* CASE #1 - our sibling is red */       
                if (wSocketEntryPtr->socRbTreeColor == TM_RB_RED)
                {
                    wSocketEntryPtr->socRbTreeColor = TM_RB_BLACK;
                    xSocketEntryPtr->socRbParentSocketPtr->
                        socRbTreeColor = TM_RB_RED;
                    tfSocketRbLeftRotate(
                        socketRootPtrPtr,
                        xSocketEntryPtr->socRbParentSocketPtr);
                    wSocketEntryPtr = xSocketEntryPtr->
                        socRbParentSocketPtr->socRbRightSocketPtr;
                    
                    if (wSocketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR)
                    {
                        wSocketEntryPtr = &nullSocketEntry;
                        nullSocketEntry.socRbParentSocketPtr = 
                            xSocketEntryPtr->socRbParentSocketPtr;
                        nullSocketEntry.socRbTreeColor = TM_RB_BLACK;
                        nullSocketEntry.socRbRightSocketPtr = 0;
                        nullSocketEntry.socRbLeftSocketPtr = 0;
                    }
                }

/* CASE #2 - our sibling is black and its children are black */
                
/* sentinel (NULL) nodes are always black */
                tempLeft = TM_RB_BLACK;
                tempRight = TM_RB_BLACK;
                if ( wSocketEntryPtr->socRbLeftSocketPtr
                     != TM_SOCKET_ENTRY_NULL_PTR )
                {
                    tempLeft = wSocketEntryPtr->socRbLeftSocketPtr->
                        socRbTreeColor;
                }
                if ( wSocketEntryPtr->socRbRightSocketPtr
                     != TM_SOCKET_ENTRY_NULL_PTR )
                {
                    tempRight = wSocketEntryPtr->socRbRightSocketPtr->
                        socRbTreeColor;
                }
                
                if ( ( tempLeft == TM_RB_BLACK )
                     && ( tempRight == TM_RB_BLACK ))
                {
                    wSocketEntryPtr->socRbTreeColor = TM_RB_RED;
                    
/* ECR 03/05/01: always want to traverse up the tree, even if this node has
   no children. */
                    xSocketEntryPtr = xSocketEntryPtr->socRbParentSocketPtr;
                }
                
/*
 * CASE #3 - our sibling is black, its left child is red and its right child
 * is black
 */
                else
                {
                    if (tempRight == TM_RB_BLACK)
                    {
                        wSocketEntryPtr->socRbLeftSocketPtr->
                            socRbTreeColor = TM_RB_BLACK;
                        wSocketEntryPtr->socRbTreeColor = TM_RB_RED;
                        tfSocketRbRightRotate(socketRootPtrPtr,
                                              wSocketEntryPtr);
                        wSocketEntryPtr = xSocketEntryPtr->
                            socRbParentSocketPtr->socRbRightSocketPtr;
                        if ( wSocketEntryPtr
                             == TM_SOCKET_ENTRY_NULL_PTR )
                        {
                            wSocketEntryPtr = &nullSocketEntry;
                            nullSocketEntry.socRbParentSocketPtr = 
                                xSocketEntryPtr->socRbParentSocketPtr;
                            nullSocketEntry.socRbTreeColor = TM_RB_BLACK;
                            nullSocketEntry.socRbRightSocketPtr = 0;
                            nullSocketEntry.socRbLeftSocketPtr = 0;
                        }
                    }
                    
/* CASE #4 - our sibling is black and its right child is red */
/* ECR 03/05/01: fixed */
                    wSocketEntryPtr->socRbTreeColor =
                        xSocketEntryPtr->
                        socRbParentSocketPtr->socRbTreeColor;
                    xSocketEntryPtr->socRbParentSocketPtr->
                        socRbTreeColor = TM_RB_BLACK;
                    if ( wSocketEntryPtr->socRbRightSocketPtr
                         != TM_SOCKET_ENTRY_NULL_PTR )
                    {
                        wSocketEntryPtr->socRbRightSocketPtr->
                            socRbTreeColor = TM_RB_BLACK;
                    }
                    tfSocketRbLeftRotate(
                        socketRootPtrPtr,
                        xSocketEntryPtr->socRbParentSocketPtr);
                    xSocketEntryPtr = *socketRootPtrPtr;
                }
            }
            else
            {
/* We're a right child */
                wSocketEntryPtr = xSocketEntryPtr->
                    socRbParentSocketPtr->socRbLeftSocketPtr;
                
/* No sibling! */
                if (wSocketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR)
                {
                    wSocketEntryPtr = &nullSocketEntry;
                    nullSocketEntry.socRbParentSocketPtr = 
                        xSocketEntryPtr->socRbParentSocketPtr;
                    nullSocketEntry.socRbRightSocketPtr = 0;
                    nullSocketEntry.socRbLeftSocketPtr = 0;
                    nullSocketEntry.socRbTreeColor = TM_RB_BLACK;
                }                    
                
/* CASE #1 - our sibling is red */       
                if (wSocketEntryPtr->socRbTreeColor == TM_RB_RED)
                {
                    wSocketEntryPtr->socRbTreeColor = TM_RB_BLACK;
                    xSocketEntryPtr->socRbParentSocketPtr->
                        socRbTreeColor = TM_RB_RED;
                    tfSocketRbRightRotate(
                        socketRootPtrPtr,
                        xSocketEntryPtr->socRbParentSocketPtr);
                    wSocketEntryPtr = xSocketEntryPtr->
                        socRbParentSocketPtr->socRbLeftSocketPtr;
                    
                    if (wSocketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR)
                    {
                        wSocketEntryPtr = &nullSocketEntry;
                        nullSocketEntry.socRbParentSocketPtr = 
                            xSocketEntryPtr->socRbParentSocketPtr;
                        nullSocketEntry.socRbTreeColor = TM_RB_BLACK;
                        nullSocketEntry.socRbRightSocketPtr = 0;
                        nullSocketEntry.socRbLeftSocketPtr = 0;
                    }
                }
                
/* CASE #2 - our sibling is black and its children are black */
/* ECR 03/05/01: sentinel (NULL) nodes are always black */
                tempLeft = TM_RB_BLACK;
                tempRight = TM_RB_BLACK;
                if ( wSocketEntryPtr->socRbLeftSocketPtr
                     != TM_SOCKET_ENTRY_NULL_PTR )
                {
                    tempLeft = wSocketEntryPtr->socRbLeftSocketPtr->
                        socRbTreeColor;
                }
                if ( wSocketEntryPtr->socRbRightSocketPtr
                     != TM_SOCKET_ENTRY_NULL_PTR )
                {
                    tempRight = wSocketEntryPtr->socRbRightSocketPtr->
                        socRbTreeColor;
                }
                
                if ( (tempRight == TM_RB_BLACK) && (tempLeft == TM_RB_BLACK) )
                {
                    xSocketEntryPtr = xSocketEntryPtr->
                        socRbParentSocketPtr;
                    wSocketEntryPtr->socRbTreeColor = TM_RB_RED;
                }
/*
 * CASE #3 - our sibling is black, its left child is red and its right child
 * is black
 */
                else
                {
                    if (tempLeft == TM_RB_BLACK )
                    {
                        wSocketEntryPtr->socRbRightSocketPtr->
                            socRbTreeColor = TM_RB_BLACK;
                        wSocketEntryPtr->socRbTreeColor = TM_RB_RED;
                        tfSocketRbLeftRotate(socketRootPtrPtr,
                                             wSocketEntryPtr);
                        wSocketEntryPtr = xSocketEntryPtr->
                            socRbParentSocketPtr->socRbLeftSocketPtr;
                        if ( wSocketEntryPtr
                             == TM_SOCKET_ENTRY_NULL_PTR )
                        {
                            wSocketEntryPtr = &nullSocketEntry;
                            nullSocketEntry.socRbParentSocketPtr = 
                                xSocketEntryPtr->socRbParentSocketPtr;
                            nullSocketEntry.socRbTreeColor = TM_RB_BLACK;
                            nullSocketEntry.socRbRightSocketPtr = 0;
                            nullSocketEntry.socRbLeftSocketPtr = 0;
                        }                    
                    }
                    
/* CASE #4 - our sibling is black and its right child is red */
/* ECR 03/05/01: fixed */
                    wSocketEntryPtr->socRbTreeColor =
                        xSocketEntryPtr->
                        socRbParentSocketPtr->socRbTreeColor;
                    xSocketEntryPtr->socRbParentSocketPtr->
                        socRbTreeColor = TM_RB_BLACK;
                    if ( wSocketEntryPtr->socRbLeftSocketPtr
                         != TM_SOCKET_ENTRY_NULL_PTR )
                    {
                        wSocketEntryPtr->socRbLeftSocketPtr->
                            socRbTreeColor = TM_RB_BLACK;
                    }
                    tfSocketRbRightRotate(
                        socketRootPtrPtr,
                        xSocketEntryPtr->socRbParentSocketPtr);
                    xSocketEntryPtr = *socketRootPtrPtr;
                }
            }
        }
        
/* ECR 03/05/01: fixed */
        xSocketEntryPtr->socRbTreeColor = TM_RB_BLACK;
    }
    return;
}

#if defined(TM_USE_SOCKET_WALK) || defined(TM_USE_STOP_TRECK)
/*
 * tfSocketWalk function description:
 * Walk down the tree, and for each socket entry call the tree entry
 * processing function. We stop when we have walked down the whole tree,
 * or when the processing function returns with no error, whichever
 * comes first.
 *
 * 1) Initialize errorCode to TM_ENOENT
 * 2) Lock the tree
 * 3) In an outer loop from the current tree root, while errorCode is
 *    not TM_ENOERROR:
 * 4) In an inner loop from the current root, we process the leftmost
 *    socket entries while there is any, and while the processing function
 *    does not return the TM_ENOERROR error code. If the processing
 *    function returns the TM_ENOERROR code, we break out of both loops.
 * 5) After having processed the leftmost socket entry, we process the
 *    sockets in the reuseaddress list if any.
 * 6) Then we get the next subtree, by trying to find the next unprocessed
 *    right subtree (going up the tree while we are a right child, or there
 *    is no right child).
 * 7) Unlock the tree
 */
int tfSocketWalk ( ttIntSocFuncPtr  processFuncPtr,
                   ttVoidPtr        voidPtr )
{
    ttSocketEntryPtr socketPtr;
    ttSocketEntryPtr nextSocketEntryPtr;
#ifdef TM_USE_REUSEADDR_LIST
    ttSocketEntryPtr reuseAddrSocketPtr;
    ttSocketEntryPtr nextReuseAddrSocketPtr;
#endif /* TM_USE_REUSEADDR_LIST */
#ifdef TM_USE_SOCKET_HASH_LIST
    unsigned int     maxHashIndex;
    unsigned int     hashIndex;
#endif /* TM_USE_SOCKET_HASH_LIST */
    int              index;
    int              errorCode;

    errorCode = TM_ENOENT;

    tm_call_lock_wait(&tm_context(tvSocketRootLock));
    for (index = 0;
         (index < TM_SOTB_NUM) && (errorCode != TM_ENOERROR);
         index++)
    {
#ifdef TM_USE_SOCKET_HASH_LIST
        maxHashIndex = tm_context(tvSocketTableListHashSize[index]);
        for (hashIndex = TM_UL(0);
             (hashIndex < maxHashIndex) && (errorCode != TM_ENOERROR);
             hashIndex++)
#endif /* TM_USE_SOCKET_HASH_LIST */
        {
#ifdef TM_USE_SOCKET_HASH_LIST
            socketPtr = tm_context(tvSocketTableRootPtrPtr[index])[hashIndex];
#else /* !TM_USE_SOCKET_HASH_LIST */
            socketPtr = tm_context(tvSocketTableRootPtr[index]);
#endif /* !TM_USE_SOCKET_HASH_LIST */
/* Start at the root. */
            while (socketPtr != (ttSocketEntryPtr)0)
/* Need to check because of possible null empty tree */
            {
                nextSocketEntryPtr = socketPtr;
                do
/* Process all socketEntry, going down the tree following the left links. */
                {
                    socketPtr = nextSocketEntryPtr;
                    nextSocketEntryPtr = socketPtr->socRbLeftSocketPtr;
                    errorCode = (*processFuncPtr)(socketPtr, voidPtr);
#ifdef TM_USE_REUSEADDR_LIST
                    reuseAddrSocketPtr = socketPtr->socReuseAddrNextPtr;
                    while (    (reuseAddrSocketPtr != (ttSocketEntryPtr)0)
                            && (errorCode != TM_ENOERROR) )
/* Process all sockets in reuse address list */
                    {
                        nextReuseAddrSocketPtr =
                                      reuseAddrSocketPtr->socReuseAddrNextPtr;
                        errorCode = (*processFuncPtr)( reuseAddrSocketPtr,
                                                       voidPtr );
                        reuseAddrSocketPtr = nextReuseAddrSocketPtr;
                    }
#endif /* TM_USE_REUSEADDR_LIST */
                }
                while (    (nextSocketEntryPtr != (ttSocketEntryPtr)0)
                        && (errorCode != TM_ENOERROR) );

                if (errorCode != TM_ENOERROR)
                {
/* At the bottom left */
                    nextSocketEntryPtr = socketPtr;
/* Go up the tree while we are a right child, or there is no right child */
                    while (    (nextSocketEntryPtr != (ttSocketEntryPtr)0)
                            && (    (    nextSocketEntryPtr->socRbRightSocketPtr
                                      == socketPtr )
                                 || ( nextSocketEntryPtr->socRbRightSocketPtr
                                      == (ttSocketEntryPtr)0 ) ) )
                    {
                        socketPtr = nextSocketEntryPtr;
                        nextSocketEntryPtr =
                                    nextSocketEntryPtr->socRbParentSocketPtr;
                    }
                    if (nextSocketEntryPtr != (ttSocketEntryPtr)0)
                    {
/*
 * Walk down the subtree starting at the first not yet processed
 * right child. We know that nextSocketEntryPtr->socRbRightSocketPtr is
 * non null because of the check in the while loop above.
 */
                        socketPtr = nextSocketEntryPtr->socRbRightSocketPtr;
                    }
                    else
                    {
                        socketPtr = (ttSocketEntryPtr)0; /* done */
                    }
                }
            }
        }
    }
    tm_call_unlock(&tm_context(tvSocketRootLock));
    return errorCode;
}
#endif /* TM_USE_SOCKET_WALK || TM_USE_STOP_TRECK */

#else /* !TM_USE_SOCKET_RB_TREE */

#ifdef TM_USE_TCP
static int tfSocketTupleEqualCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam)
{
    ttSockTuplePtr      sockTuplePtr;
    int                 retCode;
    
    sockTuplePtr = &((ttSocketEntryPtr)(ttVoidPtr)nodePtr)->socTuple;
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
    retCode = tm_tupleptr_equal(sockTuplePtr,
                                (ttSockTuplePtr)genParam.genVoidParmPtr);
/* PRQA L:L1 */
    return retCode;
}
#endif /* TM_USE_TCP */

static int tfSocketRBKeyEqualCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam)
{
    int                 retCode;
    
    retCode = (     ((ttSocketEntryPtr)(ttVoidPtr)nodePtr)->socRedBlackKey
                ==  genParam.gen32bitParm);
    return retCode;
}

static ttSocketEntryPtr tfSocketListInsert (ttSocketEntryPtr    socketPtr
#ifdef TM_USE_TCP
                                          , ttSockTuplePtr      sockTuplePtr
#endif /* TM_USE_TCP */
                                           )
{
    ttSocketEntryPtr    lookupSocketEntryPtr;
    ttListCBFuncPtr     listCBFuncPtr;
    ttGenericUnion      genParam;
    ttListPtr           listPtr;
#ifdef TM_USE_SOCKET_HASH_LIST
    ttCharPtr           keyPtr;
    int                 keyLen;
#endif /* TM_USE_SOCKET_HASH_LIST */
    int                 index;
    int                 searchIndex;

/* Look the socket up in the list to make sure we don't add duplicates */
#ifdef TM_USE_TCP
    if (((int)socketPtr->socProtocolNumber) == IP_PROTOTCP)
    {
        if (tm_16bit_one_bit_set(socketPtr->socCoFlags,
                                 TM_SOC_TREE_CO_FLAG))
        {
/* Connected TCP search */
            searchIndex = TM_SOTB_TCP_CON;
        }
        else
        {
/* Non connected TCP search */
            searchIndex = TM_SOTB_NUM;
        }
        index = TM_SOTB_TCP_CON; /* TCP sockets list */
        genParam.genVoidParmPtr = (ttVoidPtr)sockTuplePtr;
        listCBFuncPtr = tfSocketTupleEqualCB;
#ifdef TM_USE_SOCKET_HASH_LIST
        keyPtr = (ttCharPtr)(ttVoidPtr)sockTuplePtr;
        keyLen = sizeof(ttSockTuple);
#endif /* TM_USE_SOCKET_HASH_LIST */
    }
    else
#endif /* TM_USE_TCP */
    {
/* non TCP search */
        searchIndex = TM_SOTB_NON_CON;
        index = TM_SOTB_NON_CON; /* Non TCP sockets list */
        genParam.gen32bitParm = socketPtr->socRedBlackKey;
        listCBFuncPtr = tfSocketRBKeyEqualCB;
#ifdef TM_USE_SOCKET_HASH_LIST
        keyPtr = (ttCharPtr)(ttVoidPtr)&(socketPtr->socRedBlackKey);
        keyLen = sizeof(tt32Bit);
#endif /* TM_USE_SOCKET_HASH_LIST */
    }
#ifdef TM_USE_SOCKET_HASH_LIST
/* Compute and save hash */
    socketPtr->socHash = tfSocketHash(
                                keyPtr, keyLen,
                                tm_context(tvSocketTableListHashSize[index]));
    listPtr = &(tm_context(tvSocketTableListPtr[index])
                                        [socketPtr->socHash]);
#else /* !TM_USE_SOCKET_HASH_LIST */
    listPtr = &(tm_context(tvSocketTableList[index]));
#endif /* !TM_USE_SOCKET_HASH_LIST */
    lookupSocketEntryPtr = (ttSocketEntryPtr)(ttVoidPtr)
             tfListWalk(listPtr,
                        listCBFuncPtr,
                        genParam);
    if (lookupSocketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR)
    {
/* Does not exist so add it to the beginning of the list */
        tfListAddToHead(listPtr,
                        &(socketPtr->socTableNode));
/* Number of sockets in the tree */
        tm_context(tvSocketPortSearchMembers[searchIndex])++;
#ifdef TM_USE_SOCKET_HASH_LIST
        if (listPtr->listCount == 1)
        {
/* Remember the number of non empty hash buckets (for diagnosis) */
            tm_context(tvSocketTableListHashBucketFilled[index])++;
        }
        if (   listPtr->listCount
             > tm_context(tvSocketTableListHighDepth[index]))
        {
/* Remember the highest list depth for diagnosis */
            tm_context(tvSocketTableListHighDepth[index]) =
                                             listPtr->listCount;
        }
#endif /* TM_USE_SOCKET_HASH_LIST */
    }
    return lookupSocketEntryPtr;
}

#if defined(TM_USE_SOCKET_WALK) || defined(TM_USE_STOP_TRECK)
struct tsSocketWalk
{
    ttIntSocFuncPtr  sowProcessFuncPtr;
    ttVoidPtr        sowVoidPtr;
};

typedef struct tsSocketWalk TM_FAR * ttSocketWalkPtr;

/*
 * tfSocketWalk function description:
 * Lock the socket tree.
 * Walk down the socket tree and call the process function for
 * each entry in the socket tree, as long as the process function
 * returns an error (to get the next entry in the tree). If the
 * process function returns TM_ENOERROR, it means that we are done,
 * and we can return, instead of fetching the next entry.
 * Unlock the socket tree.
 * Return the process function error code.
 */
int tfSocketWalk ( ttIntSocFuncPtr  processFuncPtr,
                   ttVoidPtr        voidPtr )
{
    struct tsSocketWalk socketWalkParm;
    ttNodePtr           nodePtr;
    ttGenericUnion      genParam;
#ifdef TM_USE_SOCKET_HASH_LIST
    unsigned int        maxHashIndex;
    unsigned int        hashIndex;
#endif /* TM_USE_SOCKET_HASH_LIST */
    int                 index;
    int                 errorCode;

    errorCode = TM_ENOENT;
    tm_call_lock_wait(&tm_context(tvSocketRootLock));
/* User process function, and void parameter */
    socketWalkParm.sowProcessFuncPtr = processFuncPtr;
    socketWalkParm.sowVoidPtr = voidPtr;
    genParam.genVoidParmPtr = (ttVoidPtr)&socketWalkParm;
    for (index = 0; index < TM_SOTB_NUM; index++)
    {
#ifdef TM_USE_SOCKET_HASH_LIST
        maxHashIndex = tm_context(tvSocketTableListHashSize[index]);
        for (hashIndex = TM_UL(0); hashIndex < maxHashIndex; hashIndex++)
#endif /* TM_USE_SOCKET_HASH_LIST */
        {
            nodePtr = tfListWalk(
#ifdef TM_USE_SOCKET_HASH_LIST
                         &(tm_context(tvSocketTableListPtr[index][hashIndex])),
#else /* !TM_USE_SOCKET_HASH_LIST */
                         &(tm_context(tvSocketTableList[index])),
#endif /* !TM_USE_SOCKET_HASH_LIST */
                         tfSocketWalkCB,
                         genParam);
            if (nodePtr != (ttNodePtr)0)
            {
                errorCode = TM_ENOERROR; /* User returned TM_ENOERROR */
#ifdef TM_USE_SOCKET_HASH_LIST
                break; /* stop the walk, inner for loop */
#endif /* TM_USE_SOCKET_HASH_LIST */
            }
        }
        if (errorCode == TM_ENOERROR)
        {
            break; /* stop the walk, outer for loop */
        }
    }
    tm_call_unlock(&tm_context(tvSocketRootLock));
    return errorCode;
}

static int tfSocketWalkCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam)
{
    ttSocketEntryPtr socketPtr;
/* User process function, and void parameter */
    ttIntSocFuncPtr  processFuncPtr;
    ttVoidPtr        voidPtr;
#ifdef TM_USE_REUSEADDR_LIST
    ttSocketEntryPtr reuseAddrSocketPtr;
    ttSocketEntryPtr nextReuseAddrSocketPtr;
#endif /* TM_USE_REUSEADDR_LIST */
    int              errorCode;

    processFuncPtr =  ((ttSocketWalkPtr)genParam.genVoidParmPtr)
                                                    ->sowProcessFuncPtr;
    voidPtr =  ((ttSocketWalkPtr)genParam.genVoidParmPtr)->sowVoidPtr;
    socketPtr = (ttSocketEntryPtr)(ttVoidPtr)nodePtr;
    errorCode = (*processFuncPtr)(socketPtr, voidPtr);
#ifdef TM_USE_REUSEADDR_LIST
    reuseAddrSocketPtr = socketPtr->socReuseAddrNextPtr;
    while (    (reuseAddrSocketPtr != (ttSocketEntryPtr)0)
            && (errorCode != TM_ENOERROR) )
/* Process all sockets in reuse address list */
    {
        nextReuseAddrSocketPtr = reuseAddrSocketPtr->socReuseAddrNextPtr;
        errorCode = (*processFuncPtr)( reuseAddrSocketPtr,
                                       voidPtr );
        reuseAddrSocketPtr = nextReuseAddrSocketPtr;
    }
#endif /* TM_USE_REUSEADDR_LIST */
/* Stop if user returns TM_ENOERROR ((errorCode == TM_ENOERROR) is true) */
    return (errorCode == TM_ENOERROR);
}
#endif /* TM_USE_SOCKET_WALK || TM_USE_STOP_TRECK */
#endif /* !TM_USE_SOCKET_RB_TREE */

/* Pick one of the 2 hashes algorithms (SDBM, or Bernstein's):
 * The SDBM hash algorithm is a little bit more CPU intensive than the
 * Bernstein one.
 * However our tests showed that the SDBM hash algorithm yields a slightly
 * better distribution, so the SDBM hash algorithm is selected by default.
 */
#define TM_USE_SDBM_HASH
/* #define TM_USE_BERNSTEIN_HASH_XOR */
/*
 * Hash a key and return an index into the hash list specified by
 * index.
 */
tt32Bit tfSocketHash (ttCharPtr      keyPtr,
                      int            keyLen,
                      unsigned int   tableHashSize)
{
    tt32Bit hash;
    int     byte;
    int     i;

#ifdef TM_USE_SDBM_HASH
/* sdbm hash function is a good general hashing fuction with good
 * distribution.
 */
    hash = 0;
    for (i = 0; i < keyLen; i++)
    {
        byte = keyPtr[i];
/* hash = hash * 65599 + byte */
        hash = (tt32Bit)byte + (hash << 6) + (hash << 16) - hash;
    }
#endif /* TM_USE_SDBM_HASH */
#ifdef TM_USE_BERNSTEIN_HASH_XOR
/*
 * Bernstein's Hash function: 
 * Very powerful hash function (written by Daniel J. Bernstein.)
 * (Updated version below XOR's the byte instead of adding it. Our tests
 * showed that the XOR version yields a better distribution than the
 * ADD version.)
 */
    hash = 5381;
    for (i = 0; i < keyLen; i++)
    {
        byte = keyPtr[i];
/* hash = hash*33 ^ byte */
        hash = ((hash << 5) + hash) ^ byte;
    }
#endif /* TM_USE_BERNSTEIN_HASH_XOR */
/* Make sure hash bucket is within the allocated hash table */
    hash = hash % (tt32Bit)tableHashSize;
    return hash;
}

#if (defined(TM_IGMP) || defined(TM_6_USE_MLD))
tt8Bit tfSocketMcastMember(  ttSocketEntryPtr      socketEntryPtr
                            ,ttDeviceEntryPtr      devPtr
#ifdef TM_USE_IPV6
                            ,tt6ConstIpAddressPtr  localIpAddrPtr
#if (defined(TM_USE_IGMPV3) || defined(TM_6_USE_MLDV2))
                            ,tt6ConstIpAddressPtr  remoteIpAddrPtr
#endif /* TM_USE_IGMPV3 || TM_6_USE_MLDV2 */
#else /* !TM_USE_IPV6 */
                            ,tt4IpAddress          localIpAddress
#ifdef TM_USE_IGMPV3
                            ,tt4IpAddress          remoteIpAddress
#endif /* TM_USE_IGMPV3 */
#endif /* !TM_USE_IPV6 */
#ifdef TM_LOCK_NEEDED
                            ,tt8Bit                locked
#endif /* TM_LOCK_NEEDED */
                          )
{
    tt8Bit isMember;
#ifdef TM_6_USE_MLDV2
    tt6IpAddress    remoteIpAddr;
    int             index;
    int             srcIndex;
#endif /* TM_6_USE_MLDV2 */

    isMember = TM_8BIT_YES; /* Assume socket has joined */
#ifdef TM_LOCK_NEEDED
    if (!locked)
    {
        tm_lock_wait(&(socketEntryPtr->socLockEntry));
    }
#endif /* TM_LOCK_NEEDED */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPDUAL
    if (!(IN6_IS_ADDR_V4MAPPED(localIpAddrPtr)))
#endif /* TM_USE_IPDUAL */
    {
#ifdef TM_6_USE_MLD
        if (!(tm_6_is_addr_all_nodes(localIpAddrPtr)))
        {
#ifdef TM_6_USE_MULTICAST_LOOP
            if (devPtr == tm_context(tvLoopbackDevPtr))
            {
/* We are sending the packet to ourselves */
                isMember = TM_8BIT_YES;
            }
            else
#endif /* TM_6_USE_MULTICAST_LOOP */
            {
#ifdef TM_6_USE_MLDV2
                tm_6_ip_copy( remoteIpAddrPtr, &remoteIpAddr);
                tm_6_dev_unscope_addr(&remoteIpAddr);
#endif /*TM_6_USE_MLDV2*/
                isMember = tf6MldSocketCacheMatch(socketEntryPtr, devPtr,
                                              (tt6IpAddressPtr)localIpAddrPtr
#ifdef TM_6_USE_MLDV2
                                              ,
                                              &index
#endif /*TM_6_USE_MLDV2*/
                                              );
#ifdef TM_6_USE_MLDV2
                if(isMember == TM_8BIT_YES)
                {
                    isMember = TM_8BIT_NO;
/* match on both multicast group address and multicast device */
                    srcIndex = tfMldv2CacheGetSource(
                                    socketEntryPtr->soc6MldInfoArray[index].
                                            sourceAddrEntryPtr,
                                    &remoteIpAddr);
                    if (   (    (srcIndex == -1)
                                && (socketEntryPtr->soc6MldInfoArray[index].
                                            smi6FilterMode
                                                == TM_6_MODE_IS_EXCLUDE) )
/* Not in the source list, and mode is exclude */
                            || (    (srcIndex != -1)
                                && (socketEntryPtr->soc6MldInfoArray[index].
                                        smi6FilterMode
                                                == TM_6_MODE_IS_INCLUDE) )
/* or in the source list, and mode is include */
                        )
/* Multcast incoming traffic allowed from this peer address */
                    {
                        isMember = TM_8BIT_YES;
                    }
                }
#endif /* TM_6_USE_MLDV2 */
            }
        }
#endif /* TM_6_USE_MLD */
    }
#if defined(TM_USE_IPV4) && defined(TM_IGMP)
    else
    {
        if ( !(tm_ip_match(tm_4_ip_addr(*localIpAddrPtr),
                           TM_IP_MCAST_ALL_HOSTS )) )
        {
            isMember = tfIgmpSocketCacheMatch(socketEntryPtr, devPtr,
                                              tm_4_ip_addr(*localIpAddrPtr)
#ifdef TM_USE_IGMPV3
                                             ,tm_4_ip_addr(*remoteIpAddrPtr)
#endif /* TM_USE_IGMPV3 */
                                              );
        }
    }
#endif /* defined(TM_USE_IPV4) && defined(TM_IGMP) */
#else /* !TM_USE_IPV6 */
#if defined(TM_USE_IPV4) && defined(TM_IGMP)
    if ( !(tm_ip_match(localIpAddress,
                       TM_IP_MCAST_ALL_HOSTS )) )
    {
        isMember = tfIgmpSocketCacheMatch(socketEntryPtr, devPtr,
                                          localIpAddress
#ifdef TM_USE_IGMPV3
                                         ,remoteIpAddress
#endif /* TM_USE_IGMPV3 */
                                           );
    }
#endif /* defined(TM_USE_IPV4) && defined(TM_IGMP) */
#endif /* !TM_USE_IPV6 */
#ifdef TM_LOCK_NEEDED
    if (!locked)
    {
        tm_unlock(&(socketEntryPtr->socLockEntry));
    }
#endif /* TM_LOCK_NEEDED */
    return isMember;
}
#endif /* TM_IGMP || TM_6_USE_MLD */
 
void tfSocketFree ( ttSocketEntryPtr socketPtr )
{
#ifdef TM_USE_DYNAMIC_MEMORY
    int index;
#endif /* TM_USE_DYNAMIC_MEMORY */

/* Unlock the socket */
    tm_call_unlock(&socketPtr->socLockEntry);
/* Free the socket. Put it in the protocol dependent recycle list */
#ifdef TM_USE_DYNAMIC_MEMORY
/* Assume non TCP socket */
    index = TM_RECY_SOCKET;
#endif /* TM_USE_DYNAMIC_MEMORY */
#ifdef TM_USE_TCP
    if (((int) socketPtr->socProtocolNumber) == IP_PROTOTCP)
    {
#ifdef TM_USE_DYNAMIC_MEMORY
/* TCP socket */
        index = TM_RECY_TCPVEC;
#endif /* TM_USE_DYNAMIC_MEMORY */
/* decrement the count of allocate TCP sockets */
        tm_call_lock_wait(&tm_context(tvSocketOpenLock));
        tm_context(tvTcpVectAllocCount)--;
        tm_call_unlock(&tm_context(tvSocketOpenLock));
    }
#endif /* TM_USE_TCP */
#ifdef TM_USE_DYNAMIC_MEMORY
    tm_recycle_free(socketPtr, index);
#else /* !TM_USE_DYNAMIC_MEMORY */
    tm_recycle_free(socketPtr, 0);
#endif /* !TM_USE_DYNAMIC_MEMORY */
}

/***************** End Of File *****************/
