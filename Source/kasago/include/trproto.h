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
 * Description:     Function Prototypes
 * Filename:        trproto.h
 * Author:          Odile & Paul
 * Date Created:    9/22/97
 * $Source: include/trproto.h $
 *
 * Modification History
 * $Revision: 6.0.2.11 $
 * $Date: 2011/11/30 01:08:19JST $
 * $Author: pcarney $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _TRPROTO_H_

#define _TRPROTO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Global Treck stack initialization (no context initialization).
 */
TM_PROTO_EXTERN void tfInitTreckMultipleContext(void);

/*
 * First per context Treck Init function. Initializes context global variables
 * settable by the user.
 */
TM_PROTO_EXTERN void tfContextGlobalVarsInit(void);

/*
 * Second per context Treck Init function. Initializes context variables
 * non settable by the user, and calls all other modules init routines.
 */
TM_PROTO_EXTERN void tfContextInit(void);

/*
 * Initialize the Treck simple heap.
 */
TM_PROTO_EXTERN tt32BitPtr tfSheapCreate(void);

#ifdef TM_USE_STOP_TRECK
/*
 * Initialize the Treck simple heap.
 */
TM_PROTO_EXTERN void tfSheapDeInit(void);
#endif /* TM_USE_STOP_TRECK */

/*
 * Enable the Loopback Interface
 */
TM_PROTO_EXTERN int tfUseLoopBack(void);

/*
 * Memory allocation routines
 */
/*
 * Memory leak detection initialization 
 */
#ifdef TM_ERROR_CHECKING
#ifdef TM_USE_ANSI_LINE_FILE
#if defined(TM_KERNEL_VISUAL_X86) || defined(TM_KERNEL_WIN32_X86)
TM_PROTO_EXTERN void tfMemoryLeakInit(void);
#endif /* defined(TM_KERNEL_VISUAL_X86) || defined(TM_KERNEL_WIN32_X86) */
#endif /*  TM_USE_ANSI_LINE_FILE */
#endif /* TM_ERROR_CHECKING */

/*
 * Heap allocator collects all freed blocks, concatenating adjacent blocks,
 * and put them on the free lists.
 */
#ifdef TM_USE_DYNAMIC_MEMORY
TM_PROTO_EXTERN void tfSheapCollect(  ttRawMemoryPtrPtr rawMemListPtr
                                    , ttRcylPtrPtr      rcylListPtr
#ifdef TM_LOCK_NEEDED
                                    , tt8Bit            needLock
#endif /* TM_LOCK_NEEDED */
                                    );
#else /* !TM_USE_DYNAMIC_MEMORY */
#ifdef TM_LOCK_NEEDED
TM_PROTO_EXTERN void tfSheapCollect(tt8Bit needLock);
#else /* !TM_LOCK_NEEDED */
TM_PROTO_EXTERN void tfSheapCollect(void);
#endif /* !TM_LOCK_NEEDED */
#endif /* !TM_USE_DYNAMIC_MEMORY */

/*
 * Always Add 4 bytes to the Allocation request
 */
/* Get a Raw Memory Block from the list.  If none available then malloc it */
#if (       defined(TM_ERROR_CHECKING)      \
         && defined(TM_USE_ANSI_LINE_FILE)  \
         && (defined(TM_KERNEL_VISUAL_X86) || defined(TM_KERNEL_WIN32_X86) \
             || defined(TM_MEMORY_LOGGING)))
TM_PROTO_EXTERN ttRawBufferPtr tfGetRawBuffer(
    ttPktLen size, char TM_FAR * filename, int lineNumber);
#else
TM_PROTO_EXTERN ttRawBufferPtr tfGetRawBuffer(ttPktLen size);
#endif

/* Return a Raw Memory block to the list */
TM_PROTO_EXTERN void tfFreeRawBuffer(ttRawBufferPtr memoryBlockPtr);

#ifdef TM_USE_DYNAMIC_MEMORY
/* Given an allocation size, determine the size of the memory queue
   that it fits in. */
TM_PROTO_EXTERN ttPktLen tfGetMemoryQueueSize(ttPktLen size);
#endif /* TM_USE_DYNAMIC_MEMORY */

/* Used to Get a single packet/shared data Buffer */
TM_PROTO_EXTERN ttPacketPtr tfGetSharedBuffer( int      hdrSize,
                                               ttPktLen dataSize,
                                               tt16Bit  flag );

/* Used to Free a single packet/shared data Buffer */
TM_PROTO_EXTERN void tfFreeSharedBuffer( ttPacketPtr packetPtr,
                                         tt8Bit socketLockFlag );

/* Used to Duplicate a single packet/shared data Buffer */
TM_PROTO_EXTERN ttPacketPtr tfDuplicateSharedBuffer(ttPacketPtr packetPtr);

/*
 * Used to make data contiguous starting at packetPtr->pktLinkDataPtr for the
 * next length bytes on a scattered buffer.
 * packetPtr always points to the beginning of a scattered buffer.
 * length bytes data straddles between the end of the first packetPtr, and
 * the next ones in the link.
 * Returns pointer to same packetPtr, if there was room to copy the data at
 * the end of the first link.
 * Returns newly allocated packetPtr, if there was not enough room and a new
 * link had to be allocated to replace the first link.
 */
TM_PROTO_EXTERN ttPacketPtr tfContiguousSharedBuffer( ttPacketPtr packetPtr,
                                                      int         length );


/* Used to Duplicate a packet/shared data list (part of one datagram) */
TM_PROTO_EXTERN ttPacketPtr tfDuplicatePacket(ttPacketPtr packetPtr);

/* Used to copy a packet/shared data list (part of one datagram) */
TM_PROTO_EXTERN void tfCopyPacket( ttPacketPtr srcPacketPtr,
                                   ttPacketPtr destPacketPtr );

/* Used to Free a packet/shared data list (part of one datagram) */
TM_PROTO_EXTERN void tfFreePacket( ttPacketPtr packetPtr,
                                   tt8Bit socketLockFlag );

/* Used to Free a chain of packet/shared data list */
TM_PROTO_EXTERN void tfFreeChainPacket( ttPacketPtr packetPtr,
                                        tt8Bit      socketLockFlag );

/* Allocate memory, and zero it */
TM_PROTO_EXTERN ttVoidPtr tfBufferCalloc(unsigned int size);

/*
 * Creates or Uses Recycled Timer Block and Puts into Active List
 * This also starts the timer since it is in the active list
 * When Execute is run and the timer has expired the timer is deleted
 * and put into the recycle list
 */
TM_PROTO_EXTERN ttTimerPtr tfTimerAdd( ttTmCBFuncPtr  callBackFunctionPtr,
                                       ttGenericUnion userParm1,
                                       ttGenericUnion userParm2,
                                       tt32Bit        timerMilliSec,
                                       tt8Bit         timerFlags );

/*
 * Same as tfTimerAdd, but also allow the user to specify a cleanup
 * function to be called by the timer execute just before the timer is freed.
 * That allows delaying freeing memory that the timer can access until that
 * time.
 */
TM_PROTO_EXTERN ttTimerPtr tfTimerAddExt (
                                       ttTimerPtr     preAllocTimerPtr,
                                       ttTmCBFuncPtr  callBackFunctionPtr,
                                       ttTmCUFuncPtr  cleanUpFunctionPtr,
                                       ttGenericUnion userParm1,
                                       ttGenericUnion userParm2,
                                       tt32Bit        timerMilliSec,
                                       tt8Bit         timerFlags );

/*
 * Re-initialize an automatic timer with a new interval of call back.
 * If timer was suspended, it is re-activated.
 */
TM_PROTO_EXTERN void tfTimerNewTime(ttTimerPtr tmrPtr, tt32Bit timeParam);

/* Resume a timer that was previously suspended */
TM_PROTO_EXTERN void tfTimerResume(ttTimerPtr tmrPtr);

/* Suspend a running timer */
TM_PROTO_EXTERN void tfTimerSuspend(ttTimerPtr timerPtr);

/*
 * stop/unsuspend & delete a timer
 */
TM_PROTO_EXTERN void tfTimerRemove(ttTimerPtr timerBlockPtr);

/*
 * stop/unsupend & delete a timer. Called from another thread
 * besides the timer.
 */
TM_PROTO_EXTERN void tfTimerLockedRemove(ttTimerPtr timerBlockPtr);

/*
 * Compute time remaining for a running/suspended timer
 * Called with a lock on that guarantees that the timer won't be removed
 * during the call.
 */
TM_PROTO_EXTERN tt32Bit tfTimerMsecLeft(ttTimerPtr timerPtr);

TM_PROTO_EXTERN void tfTimerDemoCallBack (ttVoidPtr timerBlockPtr,
                                          ttGenericUnion userParm1,
                                          ttGenericUnion userParm2);
#ifdef TM_USE_STOP_TRECK
TM_PROTO_EXTERN void tfTimerDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                );
#endif /* TM_USE_STOP_TRECK */

/*
 * Pend/Post Macros to suspend and resume a task/thread/process
 */
TM_PROTO_EXTERN ttCountSemPtr tfCreatePendEntry(ttPendEntryPtr pendEntryPtr);
TM_PROTO_EXTERN int tfPend( ttPendEntryPtr pendEntryPtr,
                            ttLockEntryPtr lockEntryPtr
#ifdef TM_USE_PEND_TIMEOUT
                          , tt32Bit        tickTimeout /* in ticks */
#endif /* TM_USE_PEND_TIMEOUT */
                            );
TM_PROTO_EXTERN ttVoid tfPost( ttPendEntryPtr pendEntryPtr,
                               ttLockEntryPtr lockEntryPtr,
                               int            errorCode );

/*
 * If allocated list of counting semaphores is empty,
 * Allocate a list of TM_COUNT_SEM_BLOCK_COUNT counting semaphores.
 * If getSemaphore is set, remove a counting semaphore from the
 * list and return it.
 */
TM_PROTO_EXTERN ttCountSemPtr tfCountSemAlloc(tt8Bit getSemaphore);

#ifdef TM_USE_STOP_TRECK
/* Delete and Free list of counting semaphores for one context */
TM_PROTO_EXTERN int tfCountSemDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                );
#endif /* TM_USE_STOP_TRECK */

/*
 * Recycle a counting semaphore (used either for suspend/resume, or locking)
 */
TM_PROTO_EXTERN void tfRecycleSemaphore(ttCountSemPtr semaphorePtr);

/*
 * Check and Lock a LockEntry
 */
TM_PROTO_EXTERN int tfLock(ttLockEntryPtr entryToLock, tt8Bit flag);

/*
 * Unlock a LockEntry
 */
TM_PROTO_EXTERN void tfUnLock(ttLockEntryPtr entryToUnlock);

#if defined(TM_USE_SHEAP) || defined(TM_USER_PACKET_DATA_ALLOC)
TM_PROTO_EXTERN int tfLockCountSemGlobalCreate(int index);
#ifdef TM_USE_STOP_TRECK
TM_PROTO_EXTERN void tfLockCountSemGlobalDelete(int index);
#endif /* TM_USE_STOP_TRECK */
#endif /* defined(TM_USE_SHEAP) || defined(TM_USER_PACKET_DATA_ALLOC) */

#ifdef TM_USE_STOP_TRECK
/*
 * Free list of pre-allocated select pend entries for one context.
 */
TM_PROTO_EXTERN void tfSelectDeInit(
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                );
#endif /* TM_USE_STOP_TRECK */

/*
 * Allocate entity of size bytes; check recycle list first for a free entry.
 */
TM_PROTO_EXTERN ttVoidPtr tfRecycleAlloc(int   recycleIndex);

/*
 * Recycle rcylPtr entity on *rcylListPtrPtr recycle list
 */
/*
 * Original:
 * TM_PROTO_EXTERN void tfRecycleFree(ttRcylPtr rcylPtr, int recycleIndex);
 */
TM_PROTO_EXTERN void tfRecycleFree(ttVoidPtr rcylPtr, int recycleIndex);

/*
 * Socket / Protocol Interface
 */

/* Initialization routine */
TM_PROTO_EXTERN void tfSocketInit(tt32Bit maxNumberSockets);

/* Initialize tvVector1/tvVector2 with seed1/seed2 */
TM_PROTO_EXTERN void tfRandSeed(unsigned short seed1, unsigned short seed2);
/* Generate new values for tvVector1, tvVector2 */
TM_PROTO_EXTERN void tfRandNew(void);
/* Add new seed to old seed */
TM_PROTO_EXTERN void tfRandAdd(tt16BitPtr oldSeed1Ptr,
                               tt16BitPtr oldSeed2Ptr,
                               tt16Bit    newSeed1,
                               tt16Bit    newSeed2);
/*
 * Generate a 32 Bit Random Number
 */
TM_PROTO_EXTERN tt32Bit tfGetRandom(void);
TM_PROTO_EXTERN tt8Bit tfGetRandomByte(void);
TM_PROTO_EXTERN void tfGetRandomBytes(tt8BitPtr  rPtr, int size);
TM_PROTO_EXTERN ttS32Bit tfGetRandomDitter(ttS32Bit   ceiling);


/*
 * Allocate a socket Entry
 */
TM_PROTO_EXTERN ttSocketEntryPtr tfSocketAllocate ( tt8Bit   protocol,
                                                    ttIntPtr errorCodePtr );

/*
 * Return a 32-bit hash given a key, its length, and the hash table size.
 */
TM_PROTO_EXTERN tt32Bit tfSocketHash(ttCharPtr    keyPtr,
                                     int          keyLen,
                                     unsigned int tableHashSize);

/*
 * Insert a socket into the tree
 */
TM_PROTO_EXTERN int tfSocketTreeInsert( ttSocketEntryPtr socketEntryPtr,
                                        ttSockTuplePtr   sockTuplePtr,
                                        tt16Bit          flag );

/*
 * Insert a socket into the tree with a random port number
 */
TM_PROTO_EXTERN ttIpPort tfRandPortInsert( ttSocketEntryPtr socketEntryPtr,
                                           ttSockTuplePtr   sockTuplePtr,
                                           tt16Bit          flag );

/*
 * Re-insert a socket that is already in the tree, at a different
 * location in the tree. (Used by bind(), connect(), setsockopt with
 * local IP address change.)
 */
TM_PROTO_EXTERN int tfSocketTreeReInsert(
    ttSocketEntryPtr socketEntryPtr,
    ttSockTuplePtr   sockTuplePtr,
    tt16Bit          flag );

/*
 * Remove a socket from the tree it belongs to.
 */
TM_PROTO_EXTERN int tfSocketTreeDelete( ttSocketEntryPtr socketEntryPtr,
                                        tt16Bit flag );

/* Insert a locked socket entry into the socket array */
TM_PROTO_EXTERN int tfSocketArrayInsert(ttSocketEntryPtr socketEntryPtr);

/* Remove socket entry from the array */
TM_PROTO_EXTERN void tfSocketArrayDelete(ttSocketEntryPtr socketEntryPtr);

/*
 * General send primitive (used by all send calls)
 */
TM_PROTO_EXTERN int tfSocketSend( ttSocketEntryPtr socketEntryPtr,
                                  ttPacketPtr      packetPtr,
                                  ttPktLen         bufferLength,
                                  int              flags );

TM_PROTO_EXTERN ttSocketEntryPtr tfSocketLookup(
    ttTupleDevPtr           tupleDevPtr,
    tt16Bit                 flag );

#if (defined(TM_IGMP) || defined(TM_6_USE_MLD))
TM_PROTO_EXTERN tt8Bit tfSocketMcastMember(
                                       ttSocketEntryPtr       socketEntryPtr
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
                                      );
#endif /* TM_IGMP || TM_6_USE_MLD */

#if defined(TM_USE_SOCKET_WALK) || defined(TM_USE_STOP_TRECK)
/*
 * Walk down the socket tree, calling
 * (*processFuncPtr)(socketEntryPtr, voidPtr) for each socket entry
 * in the tree.
 */
TM_PROTO_EXTERN int tfSocketWalk( ttIntSocFuncPtr processFuncPtr,
                                  ttVoidPtr       voidPtr );
#endif /* TM_USE_SOCKET_WALK || TM_USE_STOP_TRECK */

#ifdef TM_USE_REUSEADDR_LIST
/*
 * Check out the socket that has matching IP address,
 * in the socket reuse address list. 
 */
TM_PROTO_EXTERN ttSocketEntryPtr tfSocketCheckOutReused(
#if (defined(TM_USE_STRONG_ESL) || defined(TM_IGMP) || defined(TM_6_USE_MLD))
                                        ttDeviceEntryPtr       devPtr,
#endif /* TM_USE_STRONG_ESL || TM_IGMP || TM_6_USE_MLD */
                                        ttSocketEntryPtr       socketPtr,
                                        ttSockTuplePtr         sockTuplePtr,
                                        tt16Bit                flag);
#endif /* TM_USE_REUSEADDR_LIST */

/*
 * Record error to the socket, and call back user error function
 * if any.
 */
TM_PROTO_EXTERN void tfSocketErrorRecord(int socIndex, int error);

/* Conversion from socket descriptor to socket entry pointer */
TM_PROTO_EXTERN ttSocketEntryPtr tfSocketCheckValidLock( int socketDescriptor
#ifdef TM_USE_BSD_DOMAIN
                                                        , int TM_FAR * af
#endif /* TM_USE_BSD_DOMAIN */
                                                       );

/* Same as tfSocketCheckValidLock, but also validates sockaddr parameter */
TM_PROTO_EXTERN ttSocketEntryPtr tfSocketCheckAddrLock(
          int                      socketDescriptor,
    const struct sockaddr TM_FAR * addrPtr,
          int                      addrLen,
          int TM_FAR *             errorCodePtr
#ifdef TM_USE_BSD_DOMAIN
        , int TM_FAR * af
#endif /* TM_USE_BSD_DOMAIN */
          );

TM_PROTO_EXTERN ttSocketEntryPtr tfSocketCheckAddrLenLock(
          int                      socketDescriptor,
    const struct sockaddr TM_FAR * addrPtr,
          int                      addrLen,
          int TM_FAR *             errorCodePtr
#ifdef TM_USE_BSD_DOMAIN
        , int TM_FAR *             af
#endif /* TM_USE_BSD_DOMAIN */
          );

TM_PROTO_EXTERN int tfSocketCopyDest(
         ttSocketEntryPtr          socketEntryPtr,
  const struct sockaddr TM_FAR *   toAddressPtr
#ifdef TM_4_USE_SCOPE_ID
       , ttPacketPtr               packetPtr
#endif /* TM_4_USE_SCOPE_ID */
          );

/*
 * Socket call return, checkin and unlock. If error code is non zero,
 * register it for the user, and return TM_SOCKET_ERROR, otherwise
 * return retCode.
 */
TM_PROTO_EXTERN int tfSocketReturn( ttSocketEntryPtr socketEntryPtr,
                                    int              socketDescriptor,
                                    int              errorCode,
                                    int              retCode );

/*
 * Socket call return, checkin and unlock. If error code is non zero,
 * register it for the user, and return TM_SOCKET_ERROR, otherwise
 * return retCode + copy our IP address, or peer address depending on the
 * address flag value.
 */
TM_PROTO_EXTERN int tfSocketReturnAddr(
                                    ttSocketEntryPtr socketEntryPtr,
                                    int              socketDescriptor,
                                    struct sockaddr TM_FAR *myAddressPtr,
                                    int              errorCode,
                                    int              retCode,
                                    tt8Bit           addressFlag);

/*
 * Reset all the sockets single routing cache entries of type flag (either
 * indirect or ARP).
 */
TM_PROTO_EXTERN void tfSocketResetCaches (tt16Bit           flag,
                                          int               addressFamilyFlag
#ifdef TM_LOCK_NEEDED
                                        , tt8Bit            lockFlag
#endif /* TM_LOCK_NEEDED */
                                         );

/*
 * Do socket notification (for incoming packet, send complete, connect
 * complete, accept complete, reset, error, remote close, close complete)
 */
TM_PROTO_EXTERN void tfSocketNotify( ttSocketEntryPtr socketEntryPtr,
                                     int              selectFlags,
                                     int              socCBFlags,
                                     int              errorCode );
/*
 * Notify from the register socket CB functions, if the socket has
 * already incoming data, or EOF from the remote.
 */
TM_PROTO_EXTERN void tfSockCBNotify(ttSocketEntryPtr socketEntryPtr);

#ifdef TM_USE_TCP_INCOMING_CB
/*
 * Notify the user of an incoming packet destined for the user's
 * TCP socket. The user must register a callback function on the
 * socket with tfRegisterSocketPacketCB().
 */
TM_PROTO_EXTERN void tfSockCBPacketNotify(
    ttSocketEntryPtr    socketEntryPtr,
    ttPacketPtr         packetPtr );
#endif /* TM_USE_TCP_INCOMING_CB */


/*
 * Incoming Data from TCP/UDP/ICMP
 * packetPtr->pktChainDataLength gives the total length of incoming user
 * data.
 * packetPtr->pktLinkDataLength gives user data length in one element of
 * chain of scattered data
 * packetPtr->pktLinkDataPtr = UserData for one element of chain of
 * scattered data.
 * UDP datagram is to be inserted in receive queue using the
 * pktChainNextPtr, pktChainPrevPtr (boundaries for a single datagram) of
 * the first ttPacket element.
 * TCP data is to be inserted in the receive queue using the pktLinkNextPtr
 * of the last ttPacket element and pktLinkPrevPtr of the first ttPacket
 * element.
 */
TM_PROTO_EXTERN int tfSocketIncomingPacket(ttSocketEntryPtr incomingSocketPtr,
                                           ttPacketPtr      packetPtr);

TM_PROTO_EXTERN void tfSocketFree(ttSocketEntryPtr socketEntryPtr);

/* Decrease the number of open sockets */
TM_PROTO_EXTERN void tfSocketDecreaseOpenSockets(void);

/* Reset a protocol socket cache. Called when socket is closed. */
TM_PROTO_EXTERN void tfSocketFlushProtocolCache(
                                            ttSocketEntryPtr socketEntryPtr );
#ifdef TM_USE_STOP_TRECK
TM_PROTO_EXTERN void tfSocketDeInit(
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                );
#endif /* TM_USE_STOP_TRECK */
/*
 * Append small packets to the end of a previous buffers. Used
 * only by TCP send().
 */
TM_PROTO_EXTERN ttPktLen tfSendAppend(
                                        ttSocketEntryPtr    socketEntryPtr,
                                        const char TM_FAR * bufferPtr,
                                        ttPktLen            bufferLength,
#ifdef TM_USE_QOS
                                        int                 tos,
#endif /* TM_USE_QOS */
                                        int                 flags);

/* Common code between bind(), and tfBindNoCheck() */
TM_PROTO_EXTERN int tfBindFlag(
                            int                            socketDescriptor,
                            const struct sockaddr TM_FAR * localAddrPtr,
                            int                            localAddressLength,
                            int                            flag );

/*
 * Allocate link of ttShared/ttPacket structures to point to user
 * scattered data buffers.
 */
TM_PROTO_EXTERN int tfLinkScatDataToHeader(
                                        ttPacketPtrPtr     headPacketPtrPtr,
                                        ttUserBlockPtr     userBlockPtr,
                                        int                userBlockCount,
                                        ttUserFreeFuncPtr  userFreeFunction,
                                        int                ipHdrIncl );

/*
 * Free scattered user buffers.
 */
TM_PROTO_EXTERN void tfFreeScatData( ttPacketPtr        headPacketPtr,
                                     ttUserBlockPtr     userBlockPtr,
                                     int                userBlockCount,
                                     ttUserFreeFuncPtr  userFreeFunction );

/* md4 */
TM_PROTO_EXTERN void tfMd4Init(ttVoidPtr contextPtr);
TM_PROTO_EXTERN void tfMd4Update(ttVoidPtr      contextPtr,
                                 ttConst8BitPtr dataPtr,
                                 ttPktLen       length);
TM_PROTO_EXTERN void tfMd4Final(tt8BitPtr       digestPtr,
                                ttVoidPtr       contextPtr);

/*
 * copyright RSA
 * (see copyright in trmd5.c)
 */
TM_PROTO_EXTERN void tfMd5Init(ttVoidPtr mdContextPtr);
TM_PROTO_EXTERN void tfMd5Update(ttVoidPtr      mdContextPtr,
                                 tt8BitPtr      inBufPtr,
                                 ttPktLen       inLen,
                                 ttPktLen       offset);
TM_PROTO_EXTERN void tfMd5Final(tt8BitPtr outDigest,
                                ttVoidPtr mdContextPtr);
TM_PROTO_EXTERN void tfHmacMd5(tt8BitPtr    dataPtr, 
                               int          dataLen,
                               tt8BitPtr    hashKeyPtr,
                               tt8BitPtr    digestPtr,
                               int          digestLen);
TM_PROTO_EXTERN void tfHmacSha1(tt8BitPtr    dataPtr, 
                                int          dataLen,
                                tt8BitPtr    hashKeyPtr,
                                tt8BitPtr    digestPtr,
                                int          digestLen);
TM_PROTO_EXTERN void tfMd5Checksum(tt8BitPtr    dataPtr, 
                                   int          dataLen,
                                   tt8BitPtr    digestPtr,
                                   int          digestLen);
TM_PROTO_EXTERN void tfSha1Checksum(tt8BitPtr   dataPtr, 
                                   int          dataLen,
                                   tt8BitPtr    digestPtr,
                                   int          digestLen);

TM_PROTO_EXTERN void tfSha1Init(ttVoidPtr      shaContextPtr);
TM_PROTO_EXTERN void tfSha1Update(ttVoidPtr    shaContextPtr,
                                  tt8BitPtr    inBufPtr,
                                  ttPktLen     count,
                                  ttPktLen     offset);
TM_PROTO_EXTERN void tfSha1Final(tt8BitPtr     outputPtr,
                                 ttVoidPtr     shaContextPtr);

TM_PROTO_EXTERN void tfSha256Init(ttVoidPtr      sha2ContextPtr);
TM_PROTO_EXTERN void tfSha256Update(ttVoidPtr    sha2ContextPtr, 
                                  tt8BitPtr    buffer, 
                                  ttPktLen     count, 
                                  ttPktLen     offset);
TM_PROTO_EXTERN void tfSha256Final(tt8BitPtr     outputPtr,
                                  ttVoidPtr     sha2ContextPtr);

TM_PROTO_EXTERN void tfSha384Init(ttVoidPtr      sha2ContextPtr);
TM_PROTO_EXTERN void tfSha384Update(ttVoidPtr    sha2ContextPtr, 
                                  tt8BitPtr    buffer, 
                                  ttPktLen     count, 
                                  ttPktLen     offset);
TM_PROTO_EXTERN void tfSha384Final(tt8BitPtr     outputPtr,
                                  ttVoidPtr     sha2ContextPtr);
TM_PROTO_EXTERN void tfSha512Init(ttVoidPtr      sha2ContextPtr);
TM_PROTO_EXTERN void tfSha512Update(ttVoidPtr    sha2ContextPtr, 
                                  tt8BitPtr    buffer, 
                                  ttPktLen     count, 
                                  ttPktLen     offset);
TM_PROTO_EXTERN void tfSha512Final(tt8BitPtr     outputPtr,
                                  ttVoidPtr     sha2ContextPtr);

TM_PROTO_EXTERN void tfAesXcbcInit(ttVoidPtr      xcbcContextPtr);
TM_PROTO_EXTERN void tfAesXcbcUpdate(ttVoidPtr    xcbcContextPtr, 
                                     tt8BitPtr    buffer, 
                                     ttPktLen     count, 
                                     ttPktLen     offset);
TM_PROTO_EXTERN void tfAesXcbcFinal(tt8BitPtr     outputPtr,
                                    ttVoidPtr     xcbcContextPtr);

#ifdef TM_USE_RIPEMD
/* RIPEMD160 */
TM_PROTO_EXTERN void tfRmd160Init(ttVoidPtr   rmd160ContextPtr);
TM_PROTO_EXTERN void tfRmd160Update(ttVoidPtr rmd160ContextPtr,
                                    tt8BitPtr inBufPtr,
                                    ttPktLen  count,
                                    ttPktLen  offset);
TM_PROTO_EXTERN void tfRmd160Final(tt8BitPtr    outputPtr,
                                   ttVoidPtr    rmd160ContextPtr);
#endif /* TM_USE_RIPEMD*/
#ifdef TM_USE_MD2
TM_PROTO_EXTERN void tfMd2Init(ttVoidPtr md2ContextPtr);
TM_PROTO_EXTERN void tfMd2Update(ttVoidPtr      md2ContextPtr,
                                 tt8BitPtr      inBufPtr,
                                 ttPktLen       inLen,
                                 ttPktLen       offset);
TM_PROTO_EXTERN void tfMd2Final(tt8BitPtr outDigest,
                                ttVoidPtr md2ContextPtr);
#endif /* TM_USE_MD2 */
TM_PROTO_EXTERN void tfArc4EncryptMessage(ttConst8BitPtr    srcPtr,
                                          ttPktLen          srcLength,
                                          tt8BitPtr         keyPtr,
                                          ttPktLen          keyLength,
                                          tt8BitPtr         destPtr);

/*
 * TCP Entry Points
 */
/* Initialize TCP code */
TM_PROTO_EXTERN void tfTcpInit(void);
/* Initialize TCP state vector */
TM_PROTO_EXTERN void tfTcpVectInit(ttTcpVectPtr tcpVectPtr);
/* Allocate and check out a closed TCP vector */
TM_PROTO_EXTERN ttTcpVectPtr tfTcpClosedVectGet(void);
/* Free a closed TCP vector */
TM_PROTO_EXTERN void tfTcpClosedVectFree(ttTcpVectPtr tcpVectPtr);
/* Connect/listen */
TM_PROTO_EXTERN int tfTcpOpen(ttTcpVectPtr tcpVectPtr, tt8Bit event);
/* Accept a connection */
TM_PROTO_EXTERN int tfTcpAccept(ttTcpVectPtr listenTcpVectPtr);
/*
 * Let TCP know that window is larger by copyCount. Update receive queue size.
 * Update out of band mark. Try and check if we should send a window update
 * but only if flags is set to do so.
 */
TM_PROTO_EXTERN void tfTcpRecvCmplt( ttTcpVectPtr tcpVectPtr,
                                     ttPktLen     copyCount,
                                     int          flags );
/* Check whether it is ok to receive data (according to TCP state) */
TM_PROTO_EXTERN int tfTcpRecv(ttTcpVectPtr tcpVectPtr);
/* Tcp User Receive out of band data */
TM_PROTO_EXTERN int tfTcpRecvOobData( ttTcpVectPtr  tcpVectPtr,
                                      char TM_FAR * bufferPtr,
                                      int           flags );
/* Tcp User Close */
TM_PROTO_EXTERN int tfTcpClose(ttTcpVectPtr tcpVectPtr);
/* TCP Incoming packet (from the IP layer) */
TM_PROTO_EXTERN void tfTcpIncomingPacket(ttPacketPtr packetPtr);

#ifdef TM_USE_TCP_IO
/*
 * Prototypes called from trtcpio.c
 */
/* Tcp State machine */
TM_PROTO_EXTERN int tfTcpStateMachine(ttTcpVectPtr tcpVectPtr,
                                      ttPacketPtr  packetPtr,
                                      tt8Bit       event);
/*
 * Get a TCP header from the connection TCP header recycle list. If
 * the connection TCP header recycle list is empty get a new TCP header
 * from memory.
 */
TM_PROTO_EXTERN ttPacketPtr tfTcpGetTcpHeader(ttTcpVectPtr tcpVectPtr);
/*
 * free ACKed buffers, update send queue, update RTT (since we
 * are advancing the left edge of the send window) and post on socket
 * send pend queue
 */
TM_PROTO_EXTERN void tfTcpSendQueueFree(ttTcpVectPtr tcpVectPtr,
                                        tt32Bit      bytesAcked,
                                        ttS32Bit     rtt,
                                        tt8Bit       computeRtoFlag);
/* Check if we need to update the send window variables */
TM_PROTO_EXTERN void tfTcpUpdateSendWindow(ttTcpVectPtr tcpVectPtr,
                                           tt32Bit      seqNo,
                                           tt32Bit      segAck,
                                           tt32Bit      unscaledWnd);
#ifdef TM_TCP_SACK
/*
 * Insert a SEL ACK block in TCP state vector queue of SEL ACK blocks
 * corresponding to either the SEL ACK blocks that we send in SEL ACK 
 * options (sackIndex == TM_TCP_SACK_SEND_INDEX), or corresponding to
 * the SEL ACK options that we receive 
 * (sackIndex == TM_TCP_SACK_RECV_INDEX).
 */
TM_PROTO_EXTERN  void tfTcpSackQueueBlock(
                                 ttTcpVectPtr tcpVectPtr,
                                 ttSqn        leftEdgeSegSeqNo,
                                 ttSqn        rightEdgeSegSeqNo,
                                 int          sackBlockIndex );
/*
 * Dequeue, and free a range of SEL ACK block in TCP state vector queue 
 * of SEL ACK blocks corresponding to either the SEL ACK blocks that we 
 * send in SEL ACK options (sackIndex == TM_TCP_SACK_SEND_INDEX), or 
 * corresponding to the SEL ACK blocks that we receive 
 * (sackIndex == TM_TCP_SACK_RECV_INDEX).
 */
TM_PROTO_EXTERN void tfTcpSackDeQueueBlocks(
                                    ttTcpVectPtr tcpVectPtr,
                                    ttSqn        leftEdgeSeqNo,
                                    ttSqn        rightEdgeSeqNo,
                                    int          sackBlockIndex );
#endif /* TM_TCP_SACK */
#ifdef TM_ERROR_CHECKING
TM_PROTO_EXTERN void tfTcpTimerSuspend (ttTimerPtr timerPtr);
/* Re-initialize an automatic timer with a new interval of call back */
TM_PROTO_EXTERN void tfTcpTimerNewTime(ttTimerPtr timerPtr, tt32Bit newTime);
#endif /* TM_ERROR_CHECKING */
/* Compute pseudo header checksum */
TM_PROTO_EXTERN tt16Bit tfPseudoHeaderChecksum(tt16BitPtr  pshPtr,
                                               tt16Bit     pshLength);
/* Add the window probe timer */
TM_PROTO_EXTERN void tfTcpAddProbeTimer (ttTcpVectPtr tcpVectPtr);
#endif /* TM_USE_TCP_IO */

/* Set Tcp option */
TM_PROTO_EXTERN int tfTcpSetOption(       ttTcpVectPtr  tcpVectPtr,
                                          int           optionName,
                                    const char TM_FAR * optionValuePtr,
                                          int           optionLength );
/* Get Tcp option */
TM_PROTO_EXTERN int tfTcpGetOption( ttTcpVectPtr  tcpVectPtr,
                                    int           optionName,
                                    char TM_FAR * optionValuePtr,
                                    int  TM_FAR * optionLengthPtr );


/* Send TCP packet from the send queue */
TM_PROTO_EXTERN int tfTcpSendPacket(ttTcpVectPtr tcpVectPtr);
/* Tcp User Abort */
TM_PROTO_EXTERN int tfTcpAbort(ttTcpVectPtr tcpVectPtr, int error);
/* Called from ICMP to report an error to TCP layer */
TM_PROTO_EXTERN void tfTcpIcmpErrInput ( ttTcpVectPtr    tcpVectPtr,
                                         ttTlHeaderPtr   tlPtr,
                                         int             errorCode,
                                         tt8Bit          icmpCode );
/* Purge all TCP headers from the TCP header recycle queue */
TM_PROTO_EXTERN void tfTcpPurgeTcpHeaders(ttTcpVectPtr tcpVectPtr);
/*
 * Read Shutdown:
 * User cannot recv any more data.
 * Flush receive, reassembly queues and SEL ACK block entries.
 */
TM_PROTO_EXTERN void tfTcpShutRead(ttTcpVectPtr tcpVectPtr);
/*
 * Write Shutdown:
 * Send a FIN on the TCP connection if appropriate. User can no longer
 * send data.
 */
TM_PROTO_EXTERN int  tfTcpShutWrite(ttTcpVectPtr tcpVectPtr);
/*
 * Remove a connection request from the listening queue of non accepted
 * connection requests.
 * QueueIndex indicates which queue: half connected, or fully connected.
 * Closing indicates whether the socket is being closed.
 */
TM_PROTO_EXTERN void tfTcpRemoveConReq(ttTcpVectPtr tcpVectPtr,
                                       int          queueIndex,
                                       int          closing);

/* TCP Time Wait table Table lookup */
TM_PROTO_EXTERN ttTcpTmWtVectPtr tfTcpTmWtLookup(ttTupleDevPtr tupleDevPtr);

TM_PROTO_EXTERN void tfTcpTmWtTsInit(ttTcpTmWtVectPtr tcpTmWtVectPtr,
                                     ttTcpVectPtr     tcpVectPtr);

/* Process incoming packet targeting a TCP vector in the time wait state. */
TM_PROTO_EXTERN ttPacketPtr tfTcpTmWtProcess(ttTcpTmWtVectPtr tcpTmWtVectPtr,
                                             ttTcpVectPtr     tcpVectPtr,
                                             ttPacketPtr      packetPtr,
                                             tt8Bit           event);

/* Time Wait TCP Vector Close function */
TM_PROTO_EXTERN void tfTcpTmWtVectClose(ttTcpTmWtVectPtr tcpTmWtVectPtr);

#ifdef TM_USE_TCP_REXMIT_CONTROL
TM_PROTO_EXTERN int tfTcpControlRexmitTimer(ttTcpVectPtr tcpVectPtr,
                                            int          fxn);
#endif /* TM_USE_TCP_REXMIT_CONTROL */

TM_PROTO_EXTERN int tfTcpSetMssWnd(ttTcpVectPtr tcpVectPtr);

#ifdef TM_USE_SOCKET_IMPORT
TM_PROTO_EXTERN void tfTcpEstablishSocImp(ttTcpVectPtr tcpVectPtr);
#endif /* TM_USE_SOCKET_IMPORT */

/* SLIP common routine: Initialize SLIP state vector */
TM_PROTO_EXTERN int tfSlipVectInit(ttDeviceEntryPtr devEntryPtr);

#ifdef TM_USE_IPV4
/*
 * BOOTP/DHCP common routines
 */
/* Open a BOOTP cliend UDP socket, and register a receive call back */
TM_PROTO_EXTERN int tfBtOpenSocket(ttBtEntryPtr btEntryPtr);

/* Close the BOOT/DHCP socket. Device unlocked/relocked there. */
TM_PROTO_EXTERN int tfBtCloseSocket(ttBtEntryPtr btEntryPtr);


/*
 * Called when tfConfigInterface is called (type == TM_BT_CONF), or when a
 * user wants to get a user DHCP IP address (type == TM_BT_USER). Allocate
 * a boot entry for boot information storage. call tfBtOpenSocket().
 * bootIndex is either DHCP, or BOOTP.
 */
TM_PROTO_EXTERN int tfBtInit( ttDeviceEntryPtr        devEntryPtr,
                              ttUserDhcpNotifyFuncPtr dhcpNotifyFuncPtr,
                              int                     index,
                              tt8Bit                  type,
                              tt8Bit                  bootIndex );

/*
 * Called from tfBtInit(), or tfDhcpCommonSet(), or tfBtSetOption()
 * Allocate a boot entry for boot information storage, if not already
 * allocated.
 */
TM_PROTO_EXTERN ttBtEntryPtr tfBtAlloc ( ttDeviceEntryPtr        devEntryPtr,
                                         int                     index,
                                         tt8Bit                  type,
                                         tt8Bit                  bootIndex );

/*
 * Map from device entry, index, type to pointer to a btEntry pointer
 */
TM_PROTO_EXTERN ttBtEntryPtr tfBtMap( ttDeviceEntryPtr devEntryPtr,
                                      int              index,
                                      tt8Bit           type,
                                      tt8Bit           bootIndex );

/* Allocate a packet and fill in default values for BOOTP/DHCP */
TM_PROTO_EXTERN ttPacketPtr tfBtGetFillHeader(
                                      ttBtEntryPtr     btEntryPtr,
                                      int              bootHeaderSize,
                                      ttTmCBFuncPtr    retryTimerFuncPtr );

/* Send a BOOTP/DHCP packet to provided destIpAddress on the network */
TM_PROTO_EXTERN int         tfBtSendPacket( ttPacketPtr      packetPtr,
                                            tt4IpAddress      destIpAddress );

/* Finish configuration. Notify user of error status */
TM_PROTO_EXTERN int         tfBtFinishConfig( ttBtEntryPtr     btEntrPtr,
                                              int              errorCode );

/* Remove the exponential back off timer */
TM_PROTO_EXTERN void        tfBtRemoveRetryTimer(ttBtEntryPtr btEntryPtr);

/* Set a custom DHCP/Bootp option */
TM_PROTO_EXTERN int tfBtSetOption(
                  ttUserInterface interfaceHandle,
                  int             index,
                  ttUser8Bit      bootOption,
                  ttUserCharPtr   dataPtr,
                  ttUser8Bit      dataLen,
                  ttUser8Bit      indexType,
                  ttUser8Bit      bootIndex);

/* Retrieve a specific DHCP/Bootp option sent by the server */
TM_PROTO_EXTERN int tfBtGetOption(
                  ttUserInterface  interfaceHandle,
                  int              index,
                  ttUser8Bit       bootOption,
                  ttUserCharPtr    dataPtr,
                  ttUser8BitPtr    dataLenPtr,
                  ttUser8Bit       indexType,
                  ttUser8Bit       bootIndex);

/* Called by tfListWalk to find an option in the unknown options list */
TM_NEARCALL TM_PROTO_EXTERN int tfBtFindCustOpt(ttNodePtr      nodePtr,
                                    ttGenericUnion genParam1);

/* Initialize user BOOT entry name pointers: btuDhcpRxHostNamePtr,
 * btuDomainName, btuBootSname, btuBootFileName, and btuServerFqdn. */
TM_PROTO_EXTERN void tfBtInitUserEntryNames(ttBtEntryPtr btEntryPtr);
/* Initialize user BOOT entry */
TM_PROTO_EXTERN void tfBtInitUserEntry (ttBtEntryPtr btEntryPtr);
/* Discard acquired names provided by the server */
TM_PROTO_EXTERN void tfBtDiscardServerNames (ttBtEntryPtr btEntryPtr);

/* Copy from BOOT header to user entry fields */
TM_PROTO_EXTERN int tfBtBootCopy(ttBtEntryPtr       btEntryPtr,
                                 tt8BitPtr          srcPtr,
                                 int                srcOffset,
                                 int                optionLength, /* unpacked */
                                 int                maxSize, /* packed */
                                 int                index);
/*
 * Bootp Entry points.
 */
TM_PROTO_EXTERN int tfBootpStart( ttDeviceEntryPtr deviceEntryPtr,
                                  int              index,
                                  tt8Bit           type );

TM_PROTO_EXTERN int tfBootpStop( ttDeviceEntryPtr deviceEntryPtr,
                                 int              index,
                                 tt8Bit           type );

TM_NEARCALL TM_PROTO_EXTERN void tfBootpRecvCB( ttPacketPtr  packetPtr,
                                    ttBtEntryPtr btEntryPtr );

/*
 * Dhcp entry points
 */
TM_PROTO_EXTERN int tfDhcpStart( ttDeviceEntryPtr deviceEntryPtr,
                                 int              index,
                                 tt8Bit           type );
TM_PROTO_EXTERN int tfDhcpStop( ttDeviceEntryPtr deviceEntryPtr,
                                int              index,
                                tt8Bit           type );
TM_PROTO_EXTERN void tfDhcpRecvCB( ttPacketPtr  packetPtr,
                                   ttBtEntryPtr btEntryPtr );
TM_PROTO_EXTERN int tfDhcpRenewLease(ttUserInterface interfaceHandle,
                                     int             mHomeIndex,
                                     ttUser32Bit     timeoutMilliSecs,
                                     unsigned char   retries);
TM_PROTO_EXTERN int tfDhcpRequestOffer(ttBtEntryPtr btEntryPtr);
TM_PROTO_EXTERN int tfDhcpDeleteOffers( ttBtEntryPtr    btEntryPtr,
                                        tt4IpAddress    serverId,
                                        int             operation );
#endif /* TM_USE_IPV4 */

/*
 * UDP Entry Points
 */
/* Reset the incoming socket cache if it matches the closing socket */
TM_PROTO_EXTERN void tfUdpClose(ttSocketEntryPtr socketEntryPtr);

TM_PROTO_EXTERN int tfUdpSendPacket( ttSocketEntryPtr socketPtr,
                                     ttPacketPtr      packetPtr );

TM_PROTO_EXTERN void tfUdpIncomingPacket(ttPacketPtr packetPtr);

#ifdef TM_USE_IPV4
/*
 * Raw socket entry points
 */
TM_PROTO_EXTERN void tf4RawIncomingPacket(ttPacketPtr packetPtr);
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV4
/*
 * IP Entry Points
 */
/* Incoming IP Packet */
TM_PROTO_EXTERN void tfIpIncomingPacket(ttPacketPtr               packetPtr
#ifdef TM_USE_IPSEC_TASK
                                        , ttIpsecTaskListEntryPtr varsPtr
#endif /* TM_USE_IPSEC_TASK */
                                        );

/* Sending IP packet */
#ifdef TM_USE_IPSEC    
TM_PROTO_EXTERN int tfIpSendPacket(
    ttPacketPtr               packetPtr,
    ttLockEntryPtr            lockEntryPtr,
    ttPktLenPtr               ipsecMtuAdjustPtr
#ifdef TM_USE_IPSEC_TASK
    , ttIpsecTaskListEntryPtr varsPtr
#endif /* TM_USE_IPSEC_TASK */
     );
#else /* ! TM_USE_IPSEC */
TM_PROTO_EXTERN int tfIpSendPacket( ttPacketPtr    packetPtr,
                                    ttLockEntryPtr lockEntryPtr );
#endif /* ! TM_USE_IPSEC */

/* Reassembling IP Packets */
TM_PROTO_EXTERN ttPacketPtr tfIpReassemblePacket(ttPacketPtr packetPtr);

/* Fragment IP packets */
TM_PROTO_EXTERN int tfIpFragmentPacket( ttPacketPtr packetPtr,
                                        ttPktLen    devMtu );

#ifdef TM_USE_STOP_TRECK
TM_PROTO_EXTERN void tfIpFragDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                );
#endif /* TM_USE_STOP_TRECK */

/* Get and cache the routing entry corresponding to ipDestAddr in rtcPtr */
TM_PROTO_EXTERN int tfIpCacheRte(
    ttRtCacheEntryPtr rtcPtr,
    tt4IpAddress      ipDestAddr
    );

/* IP header checksum for 20 bytes */
TM_PROTO_EXTERN tt16Bit tfIpHdr5Checksum(tt16BitPtr ipHeaderPtr);

/* Get the next Ip header option */
TM_PROTO_EXTERN tt8BitPtr tfIpGetNextOpt (
    tt8BitPtr optPtr,
    tt8BitPtr optDataLenPtr,
    tt8BitPtr optValPtr,
    tt8BitPtr optLenPtr,
    tt8BitPtr icmpParamPtr,
    tt8Bit    ipHdrLen);
#endif /* TM_USE_IPV4 */

/* TCP/IP packet checksum */
TM_PROTO_EXTERN tt16Bit tfPacketChecksum(
    ttPacketPtr packetPtr,
    ttPktLen    length,
    tt16BitPtr  pshHdrPtr,
    tt16Bit     pshLen );

/* Update checksum when one 16bit value changes in the header. Diff is the
 * difference between old value and new value
 */
TM_PROTO_EXTERN tt16Bit tfIpUpdateChecksum(tt16Bit oldSum, tt32Bit diff);

#ifdef TM_DEV_RECV_OFFLOAD
/*
 * Compute checksum on passed checksumPtr, and remove it from the hardware
 * computed checksum passed in in the ttDevRecvOffload structure.
 */
TM_PROTO_EXTERN void tfDevoRecvPeelChecksum( ttPacketPtr    packetPtr,
                                             tt16BitPtr     checksumPtr,
                                             tt16Bit        length );
/*
 * Trim any excess checksum computed by the hardware on the data beyond
 * the end of the packet.
 */
TM_PROTO_EXTERN void tfDevoRecvTrimChecksum( ttPacketPtr packetPtr,
                                             tt16Bit     hdrLength,
                                             ttPktLen    pktLength );
/*
 * Compute checksum on passed pshPtr, and add it to the hardware computed
 * checksum (which has had the IP header checksum removed already in
 * tfDevoRecvPeelChecksum) passed in the ttDevRecvOffload structure.
 * Remove checksum on trailing data if any.
 */
TM_PROTO_EXTERN tt16Bit tfDevoRecvPacketChecksum(
                                    ttDevRecvOffloadPtr pktDevRecvOffloadPtr,
                                    tt16BitPtr  pshPtr,
                                    tt16Bit     pshLength );
#endif /* TM_DEV_RECV_OFFLOAD */

#ifdef TM_USE_IPV4
/*
 * If ipAddr is multicast or directed broadcast on device/mhome, map
 * to multicast/broadcast ethernet address. Returns
 * TM_ETHER_UCAST if ipAddr is a unicast address
 * TM_ETHER_BROAD if mapped to Ethernet Broadcast addrress
 * TM_ETHER_MCAST if mapped to Ethernet multicast address
 */
TM_PROTO_EXTERN int tfIpBroadMcastToLan( tt4IpAddress     ipAddr,
                                         ttDeviceEntryPtr devEntryPtr,
                                         tt16Bit          mhomeIndex,
                                         tt8BitPtr        etherPtr );

/*
 * Given a destination address, find the outgoing interface/route and
 * fill in the IP header TOS, Source and Destination IP addresses.
 * Return 0 on success, error value on failure
 */
TM_PROTO_EXTERN int tfIpDestToPacket(
    ttPacketPtr       packetPtr,
#ifdef TM_4_USE_SCOPE_ID
    tt6ConstIpAddressPtr srcV4MappedIpAddrPtr,
    tt6ConstIpAddressPtr peerV4MappedIpAddrPtr,
#else /* ! TM_4_USE_SCOPE_ID */
    tt4IpAddress      srcIpAddress,
    tt4IpAddress      peerIpAddress,
#endif /* ! TM_4_USE_SCOPE_ID */
    tt8Bit            ipTos,
    ttRtCacheEntryPtr rtcPtr,
    ttIpHeaderPtr     iphPtr );
#endif /* TM_USE_IPV4 */

/*
 * Reset non socket IP send cache, and IP forward cache of type flag
 * (either indirect or ARP).
 */
TM_PROTO_EXTERN void tfIpResetCaches(tt16Bit           flag,
                                     int               addressFamilyFlag
#ifdef TM_LOCK_NEEDED
                                   , tt8Bit            lockFlag
#endif /* TM_LOCK_NEEDED */
                                    );

#ifdef TM_SINGLE_INTERFACE_HOME
/*
 * Check whether the single interface is configured. Return pointer to
 * the device.
 */
TM_PROTO_EXTERN ttDeviceEntryPtr tfIfaceConfig(tt16Bit flag);
#else /* TM_SINGLE_INTERFACE_HOME */

#ifdef TM_USE_IPV4
/* Get multi-home index of directed broadcast match for a given interface */
TM_PROTO_EXTERN int tfMhomeDBroadMatch( ttDeviceEntryPtr   devEntryPtr,
                                        tt4IpAddress       ipAddress,
                                        tt16BitPtr         ifaceMhomePtr );

/* Get multi-home index of network match for a given interface */
TM_PROTO_EXTERN int tfMhomeNetMatch( ttDeviceEntryPtr      devEntryPtr,
                                     tt4IpAddress          ipAddress,
                                     tt16BitPtr            ifaceMhomePtr );

/* Get multi-home index of first configured multihome for a given interface */
TM_NEARCALL TM_PROTO_EXTERN int tfMhomeAnyConf( ttDeviceEntryPtr       devEntryPtr,
                                    tt4IpAddress           ipAddr,
                                    tt16BitPtr             ifaceMhomePtr );

/* Get multi-home index of address match for a given interface/IP address */
TM_PROTO_EXTERN int tfMhomeAddrMatch( ttDeviceEntryPtr     devEntryPtr,
                                      tt4IpAddress         ipAddress,
                                      tt16BitPtr           ifaceMhomePtr );

/* Get Device/multi-home index of address match for a given IP address */
TM_PROTO_EXTERN ttDeviceEntryPtr tfIfaceMatch(
                                      tt4IpAddress         ipAddr,
                                      ttMhomeFunctPtr      mHomeMatchFuncPtr,
                                      tt16BitPtr           ifaceMhomePtr );
#endif /* TM_USE_IPV4 */
#endif /* TM_SINGLE_INTERFACE_HOME */


/*
 * Update chain with a new smaller length 'newLength':
 * Update chain data length with new length. Free all packets
 * at the end of the chain that no longer belong to the chain.
 * Update link data length of the last link.
 */
TM_PROTO_EXTERN void tfPacketTailTrim( ttPacketPtr packetPtr,
                                       ttPktLen    newLength,
                                       tt8Bit      socketLockFlag );

/*
 * Append a new link of size addLength. Zero it.
 */
TM_PROTO_EXTERN int tfPacketTailAdd(ttPacketPtr packetPtr, int addLength);

/*
 * Trim head pointer by trimLen. Free shared buffers that are trimmed
 * all the way, but keep the first one around, since it contains packet
 * information, and could contain a network header. Update all lengths,
 * and pointer fields.
 */
TM_PROTO_EXTERN void tfPacketHeadTrim( ttPacketPtr packetPtr,
                                       ttPktLen    trimLen );

/* test if the current shared buffer has enough space to hold
 * new header size and|or tail size, if not, add shared buffer to
 * the front |end of the shared data.
 * returns new packet pointer if header changes
 */
TM_PROTO_EXTERN int tfPktHeadTailAdd(ttPacketPtrPtr packetPtrPtr,
                                        ttPktLen       copyHead,
                                        ttPktLen       needHead,
                                        ttPktLen       needTail);


#ifdef TM_USE_IPV4
/*
 * ICMP Entry Points
 */
/* ICMP error packet generation */
TM_PROTO_EXTERN void tfIcmpErrPacket( ttPacketPtr      packetPtr,
                                      tt8Bit           icmpType,
                                      tt8Bit           icmpCode,
                                      tt32Bit          icmpParam,
                                      ttDeviceEntryPtr outDevEntryPtr,
                                      tt4IpAddress     ipAddr );

/* Incoming ICMP packet from the network */
TM_PROTO_EXTERN void tfIcmpIncomingPacket(ttPacketPtr packetPtr);

/* Creating an ICMP packet internally */
TM_PROTO_EXTERN int tfIcmpCreatePacket( ttDeviceEntryPtr   devEntryPtr,
                                        tt16Bit            mhomeIndex,
                                        tt16Bit            flags,
                                        tt4IpAddress       IpSrcAddr,
                                        tt4IpAddress       IpDestAddr,
                                        tt8Bit             icmpType,
                                        tt8Bit             icmpCode,
                                        tt4IpAddress       ipAddr );

/*
 * User call to send data on a raw socket. If raw socket is of type ICMP
 * for example, ICMP header filled by the user.
 * Destination IP address stored in ttPacket. (If source known, stored in
 * ttPacket)
 */
TM_PROTO_EXTERN int tfRawSendPacket( ttSocketEntryPtr      sockPtr,
                                     ttPacketPtr           packetPtr );

/*
 * IGMP entry points
 */
/* Initialize IGMP variables */
TM_PROTO_EXTERN void tfIgmpInit(void);

/*
 * Per interface IGMP initialization, i.e joining 224.0.0.1, 224.0.0.2, and
 * initializing default IGMPv3 QQI, and IGMPv3 robustness variable for the
 * device.
 */
TM_PROTO_EXTERN void tfIgmpDevInit( ttDeviceEntryPtr devEntryPtr,
                                    tt16Bit          mhomeIndex );

/*
 * Per interface IGMP un-initialization, i.e un-joining 224.0.0.1,
 * 224.0.0.2
 */
TM_PROTO_EXTERN void tfIgmpDevClose( ttDeviceEntryPtr devEntryPtr );

#ifdef TM_USE_STOP_TRECK
TM_PROTO_EXTERN void tfIgmpDeInit( ttDeviceEntryPtr devEntryPtr );
#endif /* TM_USE_STOP_TRECK */

/* IGMP IPPROTO_IP level setsockopt() calls */
TM_PROTO_EXTERN int tfIgmpSetSockOpt( ttSocketEntryPtr   socketEntryPtr,
                                      int                optionName,
                                      ttConstCharPtr     optionValuePtr,
                                      int                optionLength );
/* IGMP IPPROTO_IP level getsockopt() calls */
TM_PROTO_EXTERN int tfIgmpGetSockOpt( ttSocketEntryPtr   socketEntryPtr,
                                      int                optionName,
                                      ttConstCharPtr     optionValuePtr,
                                      ttIntPtr           optionLengthPtr );

/* IGMP user APIs common parameters' check and initialization */
TM_PROTO_EXTERN int tfIgmpUserParameters(
                                      ttIpMreqSrcPtr     imrIntPtr,
                                      ttSocketEntryPtr   socketEntryPtr, 
                                      tt4IpAddress       multiAddr,
                                      tt4IpAddress       interfaceAddr,
#ifdef TM_USE_IGMPV3
                                struct in_addr TM_FAR  * srcAddrPtr,
                                      int                numSrc,
                                      int                fmode,
#endif /* TM_USE_IGMPV3 */
                                      int                optionName);

/* Common code for membership setsockopt */
TM_PROTO_EXTERN int tfIgmpSetSockMembership (ttIpMreqSrcPtr   imrIntPtr);
/* cache multicast group join on socket + IGMP Join */
TM_PROTO_EXTERN int tfIgmpSocketCacheJoin (ttIpMreqSrcPtr imrIntPtr);

/* MCAST group in socket cache and source address not filtered? */
TM_PROTO_EXTERN tt8Bit tfIgmpSocketCacheMatch(
    ttSocketEntryPtr socketEntryPtr,
    ttDeviceEntryPtr devEntryPtr,
    tt4IpAddress     groupAddress
#ifdef TM_USE_IGMPV3
   ,tt4IpAddress     srcAddress
#endif /* TM_USE_IGMPV3 */
    );

/*
 * Find out if multicast group has been joined on a given interface
 * and if source address is allowed.
 */
TM_PROTO_EXTERN int tfIgmpMember( ttDeviceEntryPtr devEntryPtr,
                                  tt4IpAddress     multiCastGroup
#ifdef TM_USE_IGMPV3
                                , tt4IpAddress     srcAddress
#endif /* TM_USE_IGMPV3 */
#ifndef TM_SINGLE_INTERFACE_HOME
                                , tt16BitPtr       mhomeIndexPtr
#endif /* !TM_SINGLE_INTERFACE_HOME */
                                );

/* IGMP protocol incoming packet */
TM_PROTO_EXTERN void tfIgmpIncomingPacket(ttPacketPtr packetPtr);

#ifdef TM_IGMP
/* Get socket mcast cache entry corresponding to multicast group on socket */
TM_PROTO_EXTERN ttSockIgmpInfoPtr tfIgmpSocketCacheGet(
    ttIpMreqSrcPtr imrIntPtr);
#endif /* TM_IGMP */

/* Leave all multicast groups joined on this socket. Free up cache area. */
TM_PROTO_EXTERN void tfIgmpSocketCachePurge(ttSocketEntryPtr socketEntryPtr);
/*
 * Return from join. Make sure to purge socket IGMP info cache if socket
 * had been closed.
 */
TM_PROTO_EXTERN void tfIgmpSocketJoinReturn(ttSocketEntryPtr socketEntryPtr);

#ifdef TM_USE_IGMPV3
/* Common code for setipv4sourcefilter and tfIoctl with SIOCSIPMSFILTER */
TM_PROTO_EXTERN int tfComSetipv4sourcefilter(
    ttSocketEntryPtr        socketEntryPtr,
    struct in_addr          interfaceAddr,
    struct in_addr          group,
    ttUser32Bit             fmode,
    ttUser32Bit             numsrc,
    struct in_addr TM_FAR * slist);
/* Common code for getipv4sourcefilter and tfIoctl with SIOCGIPMSFILTER */
TM_PROTO_EXTERN int tfComGetipv4sourcefilter(
    ttSocketEntryPtr        socketEntryPtr,
    struct in_addr          interfaceAddr,
    struct in_addr          group,
    ttUser32BitPtr          fmodePtr,
    ttUser32BitPtr          numsrcPtr,
    struct in_addr TM_FAR * slist);
TM_PROTO_EXTERN int tfIgmpSocketCacheAddSource (
    ttSockIgmpInfoPtr   sockIgmpInfoPtr,
    ttIpMreqSrcPtr      imrIntPtr);
/* Tell IGMP of source list change, possibly sending a state change report */
TM_PROTO_EXTERN int tfIgmpChangeSource(
    ttIpMreqSrcPtr  imrIntPtr,
    ttSourceListPtr oldSrcListPtr,
    int             deltaEmptyExclJoin);
/* Copy one source list, moving allocated sources */
TM_PROTO_EXTERN void tfIgmpMoveSourceList(
    ttSourceListPtr oldSrcListPtr,
    ttSourceListPtr newSrcListPtr);
/*
 * Restore previously backed up source list if an error occured during
 * processing, otherwise free the backed up source list.
 */
void tfIgmpRestoreSourceList(
    ttSourceListPtr backupSrcListPtr,
    ttSourceListPtr srcListPtr,
    int             errorCode);
#endif /* TM_USE_IGMPV3 */

/*
 * ARP Entry Points
 */
/* Incoming Arp Message */
TM_PROTO_EXTERN int tfArpIncomingPacket(ttPacketPtr  packetPtr);
/* Arp resolution */
TM_NEARCALL TM_PROTO_EXTERN int tfArpResolve(ttPacketPtr packetPtr);
/* Free all matching packets waiting for ARP resolution on the device. */
TM_NEARCALL TM_PROTO_EXTERN void tfArpResolveClean(ttUserInterface    interfaceHandle,
                                       tt4IpAddress       intfIpAddr,
                                       tt4IpAddress       intfNetMask);
/* Send an Arp Request */
TM_NEARCALL TM_PROTO_EXTERN int tfArpSendRequest( ttVoidPtr     deviceEntryPtr,
                                      tt16Bit       mhomeIndex,
                                      tt4IpAddress  ipAddress,
                                      ttRteEntryPtr rteEntryPtr );
/* Find a proxy entry corresponding to ipAddress */
TM_PROTO_EXTERN ttProxyEntryPtr tfArpFindProxyEntry(tt4IpAddress ipAddress);

TM_NEARCALL TM_PROTO_EXTERN int tfEtherMcast( ttVoidPtr        interfaceHandle,
                                  tt4IpAddress     groupAddress,
                                  int              command );
#endif /* TM_USE_IPV4 */

/* Ethernet/PPPoE link layer recv function */
TM_PROTO_EXTERN int tfEtherRecv(void TM_FAR *interfaceId,
                                void TM_FAR *bufferHandle);
TM_PROTO_EXTERN int tfEtherSetMcastAddr( ttDeviceEntryPtr devEntryPtr,
                                         ttEnetAddress    enetAddress,
                                         tt8Bit           specGroupCmd,
                                         int              command);

/* common functions between 802.3 and Ethernet link-layers */
TM_NEARCALL TM_PROTO_EXTERN int tfEtherCommonClose(void TM_FAR *interfaceId);

#if defined(TM_USE_IPV4) || defined(TM_6_USE_DAD)
TM_PROTO_EXTERN tt8Bit tfEtherMatchOurs(
    ttDeviceEntryPtr devPtr,
    tt8BitPtr        enetAddressPtr);
#endif /* TM_USE_IPV4 || TM_6_USE_DAD */
    
TM_PROTO_EXTERN void tfNudSetReachable( ttRteEntryPtr rteEntryPtr );

/*
 * Device Entry Points (most of them are in trsocket.h).
 */

/*
 * Add interface to the routing table, and set its configuration flag.
 */
TM_PROTO_EXTERN int tfDeviceStart( ttDeviceEntryPtr devEntryPtr,
                                   tt16Bit          multiHomeIndex,
                                   int              protocolFamily );

/*
 * Remove all entries associated with devEntryPtr from the routing table
 * and close link layer and device
 */
TM_PROTO_EXTERN int tfDeviceClose (ttDeviceEntryPtr devEntryPtr);

/*
 * Remove a configured interface from the routing table, and reset its
 * configuration flag.
 */
TM_PROTO_EXTERN int tfRemoveInterface( ttDeviceEntryPtr devEntryPtr,
                                       tt16Bit          mhomeIndex,
                                       int              protocolFamily );

/* Send an ARP or IP packet to the interface */
TM_PROTO_EXTERN int tfDeviceSend(ttPacketPtr packetPtr);

/*
 * Send one packet to the device. This is the link post send function
 * used by Ethernet, Loop back, or Transparent link layer. Packet has
 * already been queued by tfDeviceSend().
 * Function called from tfDeviceSend() if no Treck transmit task is used,
 * or from tfXmitInterface() if a Treck transmit task is used.
 */
TM_NEARCALL TM_PROTO_EXTERN int tfDeviceSendOnePacket(ttPacketPtr packetPtr);

/*
 * Function called from tfDeviceSend() if no Treck transmit task is used,
 * or from tfXmitInterface() if a Treck transmit task is used,
 * when the link post send function returns an error.
 * If no device transmit queue is used, or if the errorCode is TM_ENOBUFS
 * (i.e the device transmit queue logic dropped a whole packet),
 * it dequeues and frees the last packet queued to the device send queue.
 * It updates the interface SNMP MIB out error variables.
 */
TM_PROTO_EXTERN int tfDeviceSendFailed( ttDeviceEntryPtr devEntryPtr,
                                        ttPacketPtr      packetPtr,
                                        int              errorCode,
                                        tt8BitPtr        needFreePacketPtr );

/* Unlock device driver. Check if we need to call tfSendCompleteInterface() */
TM_PROTO_EXTERN void tfDeviceUnlockSendComplete(ttDeviceEntryPtr devEntryPtr);

#ifdef TM_USE_STOP_TRECK
#define TM_DEVF_UNINITIALIZING  1

TM_PROTO_EXTERN void tfDeviceListClose(int flags);
TM_PROTO_EXTERN void tfDeviceListFree(
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                                     );
#endif /* TM_USE_STOP_TRECK */

/* Common code to open a non point to point Link Layer device */
TM_NEARCALL TM_PROTO_EXTERN int tfLinkOpen( void  TM_FAR * interfaceId,
                                tt16Bit        multiHomeIndex,
                                int            protocolFamily);

/* Null link-layer functions */
TM_PROTO_EXTERN int tfNullLinkClose(void TM_FAR * interfaceId);
TM_PROTO_EXTERN int tfNullLinkSend(ttPacketPtr packetPtr);
TM_PROTO_EXTERN int tfNullLinkRecv(
    void TM_FAR *interfaceId, void TM_FAR *bufferHandle);
TM_PROTO_EXTERN int tfNullLinkIoctl(int operation);
TM_PROTO_EXTERN int tfNullLinkError(int type);

/*
 * Interface Loopback Driver entry points with non scattered recv and
 * multiple scattered send calls.
 */
TM_PROTO_EXTERN int tfIntfDrvSend( ttUserInterface  interfaceHandle,
                                   char     TM_FAR  *dataPtr,
                                   int              dataLength,
                                   int              flag );

TM_PROTO_EXTERN int tfIntfDrvRecv ( ttUserInterface    interfaceHandle,
                                    ttCharPtr TM_FAR  *dataPtrPtr,
                                    ttIntPtr           dataSizePtr,
                                    ttUserBufferPtr    packetPtrPtr
#ifdef TM_DEV_RECV_OFFLOAD
                                  , ttDevRecvOffloadPtr recvOffloadPtr
#endif /* TM_DEV_RECV_OFFLOAD */
                                   );

TM_PROTO_EXTERN int tfIntfDrvGetPhyAddr( ttUserInterface interfaceHandle,
                                         char TM_FAR *physicalAddress );

TM_PROTO_EXTERN int tfIntfDrvIoctl( ttUserInterface interfaceHandle, int flag,
                                    void TM_FAR * optionPtr, int optionLen );


/*
 * Wait for an event to occur on an interface. Used by
 * tfWaitReceiveInterface, tfWaitSentInterface, and tfWaitXmitInterface
 */
TM_PROTO_EXTERN int tfWaitEventInterface(ttEventEntryPtr eventEntryPtr);


/* Check whether a user device handle is valid */
TM_PROTO_EXTERN int tfValidInterface(ttDeviceEntryPtr checkDevEntryPtr);

#ifdef TM_USE_IPV4
/*
 * Check whether a user device handle and multi home index are valid,
 * and that the device is configured for that multi home index
 */
TM_PROTO_EXTERN int tfValidConfigInterface (
                                           ttDeviceEntryPtr checkDevEntryPtr,
                                           tt16Bit          mHomeIndex );
#endif /* TM_USE_IPV4 */

/*
 * Check whether an event (recv, xmit, or send on the interface) has
 * occured. Common function used by tfCheckReceiveInterface,
 * tfCheckXmitInterface, or tfCheckSentInterface.
 */
TM_PROTO_EXTERN int tfCheckEventInterface(ttEventEntryPtr eventEntryPtr);

#ifdef TM_USE_IPV4
/* Return device destination IP address for PT2PT */
TM_PROTO_EXTERN tt4IpAddress tfDeviceDestIpAddress(
                                           ttDeviceEntryPtr devEntryPtr );
#endif /* TM_USE_IPV4 */

/* Check whether a user link layer handle is valid */
TM_PROTO_EXTERN int tfValidLinkLayer(ttLinkLayerEntryPtr linkLayerHandle);

/*
 * Router Entry Points
 */
/* Init routine */
TM_PROTO_EXTERN void tfRtInit(void);

#ifdef TM_USE_STOP_TRECK
/* Free all router/arp entries + route tables */
TM_PROTO_EXTERN void tfRtDeInit(
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                );
#endif /* TM_USE_STOP_TRECK */

#ifdef TM_LITTLE_ENDIAN
TM_PROTO_EXTERN tt16Bit tfRtBitOffset( tt4IpAddress    key,
                                       tt4IpAddress    ipaddr,
                                       tt4IpAddressPtr bitMaskPtr );
#endif /* TM_LITTLE_ENDIAN */

/*
 * Update routing entries for a given device with a new MTU received in a
 * router advertisement, or set by the user.  Called with the tree unlocked.
 */   
TM_PROTO_EXTERN void tfRtUpdateDevMtu(ttDeviceEntryPtr devEntryPtr,
                                      tt16Bit          newMtu,
                                      tt16Bit          af);

#ifdef TM_USE_IPV4
/* Called by tfConfigInterface to add a local route */
TM_PROTO_EXTERN int tf4RtAddLocal( ttDeviceEntryPtr      devEntryPtr,
/* local destination network for Ethernet, Remote IP address for SLIP/PPP */
                                  tt4IpAddress          destination,
                                  tt4IpAddress          myNetMask,
/* IP address of the interface */
                                  tt4IpAddress          myIpAddress,
/* Multihome index of the interface */
                                  tt16Bit               myMultiHomeIndex );

/*
 * Called by tfDelDefaultGateway, or by tfDelStaticRoute, or by
 * tfUncongigInterface to delete a route
 */
TM_PROTO_EXTERN int tf4RtDelRoute(
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr    devPtr,
#endif /* TM_USE_STRONG_ESL */
    tt4IpAddress        destIpAddress,
    tt4IpAddress        destNetMask,
    tt16Bit             rteCreateFlag );

/* Get IPv4 default gateway IP address */
TM_PROTO_EXTERN int tf4RtGetDefault(
#ifdef TM_USE_STRONG_ESL
                             ttDeviceEntryPtr    devPtr,
#endif /* TM_USE_STRONG_ESL */
                             tt4IpAddressPtr     gatewayIpAddrPtr,
                             ttIntPtr            hopsPtr);

/*
 * Called by tfAddRoute(), BOOTP/DHCP, MOBILIP, ICMP, SNMP to lock the
 * routing table, allocate a new router entry and insert it in the
 * routing table.
 */
TM_PROTO_EXTERN int tf4RtAddRoute(
 /* route device */
                                 ttDeviceEntryPtr       devEntryPtr,
/* Destination Ip address */
                                 tt4IpAddress           ipAddr,
/* destination network address */
                                 tt4IpAddress           netMask,
/* gateway */
                                 tt4IpAddress           gwayAddr,
/* Tag for RIP */
                                 tt16Bit                tag,
/* multihome index */
                                 tt16Bit                multiHomeIndex,
/* metrics for non local routes */
                                 int                    hops,
                                 tt32Bit                ttl,
/* flag to indicate owner local/static/redirect/RIP */
                                 tt16Bit                flags,
                                 tt8Bit                 needLock);

TM_PROTO_EXTERN int tf4RtAddHost(
 /* route device */
                                 ttDeviceEntryPtr       devEntryPtr,
/* Destination Ip address */
                                 tt4IpAddress           ipAddr,
/* physical address */
                                 tt8BitPtr              physAddrPtr,
/* physical address len */
                                 tt8Bit                 physAddrLen,
/* multihome index */
                                 tt16Bit                multiHomeIndex,
/* metrics for non local routes */
                                 tt32Bit                ttl,
/* flag to indicate cloned/static/link-layer/mcast */
                                 tt16Bit                flags );

/* Called by RIP when a RIP entry has been received */
TM_PROTO_EXTERN void tfRtRip( ttDeviceEntryPtr devEntryPtr,
                              tt16Bit          mhomeIndex,
                              tt4IpAddress     destIpAddr,
                              tt4IpAddress     destNetMask,
                              tt4IpAddress     nextHop,
                              int              metric,
                              tt16Bit          ripTag );

/* Called by ICMP when a redirect has been received */
TM_PROTO_EXTERN void tf4RtRedirect( ttDeviceEntryPtr devEntryPtr,
                                    tt16Bit          mhomeIndex,
                                    tt4IpAddress     dest,
                                    tt4IpAddress     gateway,
                                    tt4IpAddress     src );
/* Called by ICMP when an echo reply has been received */
TM_PROTO_EXTERN void tfRtEchoReply(
#ifdef TM_USE_STRONG_ESL
                                    ttDeviceEntryPtr devEntryPtr,
#endif /* TM_USE_STRONG_ESL */
                                    tt4IpAddress     ipAddress);
/*
 * Called by ICMP when an UNREACH for fragment arrives.
 * Also called by the user tfDisablePathMtuDisc()/tfNgDisablePathMtuDisc()
 * when the user wants to disable PATH MTU discovery on a host route.
 */
TM_PROTO_EXTERN int tfRtNextHopMtu(
#ifdef TM_USE_STRONG_ESL
                                    ttDeviceEntryPtr devEntryPtr,
#endif /* TM_USE_STRONG_ESL */
                                    tt4IpAddress     ipAddress,
                                    tt16Bit          nextHopMtu,
                                    ttIpHeaderPtr    iphPtr );

#endif /* TM_USE_IPV4 */

#if (defined(TM_USE_IPV6) && defined(TM_USE_IPV4))
/*
 * Function used in dual mode, by IPV4 APIs.
 */
/* Called by tfAddDefGateway() */
TM_PROTO_EXTERN int  tf4RtAddDefGw( ttDeviceEntryPtr   devEntryPtr,
                                    tt4IpAddress       gatewayIpAddress
#ifdef TM_USE_STRONG_ESL
                                  , int                hops
#endif /* TM_USE_STRONG_ESL */
                                  );

/* Called by tfDelDefGateway() */
TM_PROTO_EXTERN int  tf4RtDelDefGw(
#ifdef TM_USE_STRONG_ESL
                                    ttDeviceEntryPtr   devEntryPtr,
#endif /* TM_USE_STRONG_ESL */
                                    tt4IpAddress       gatewayIpAddress
                                  );

/* Insert an empty IPv4 default gateway */
TM_PROTO_EXTERN int tf4RtInitInsertEmptyDefaultGateway(
#ifdef TM_USE_STRONG_ESL
                                    ttDeviceEntryPtr   devEntryPtr
#else /* !TM_USE_STRONG_ESL */
                                    void
#endif /* !TM_USE_STRONG_ESL */
                                                  );
#endif /* TM_USE_IPV6 && TM_USE_IPV4 */

/*
 * Called by the IP forward function and all send functions to get an
 * outgoing route entry pointer for a given destination address. Destination
 * IP address is in routing cache structure. Resulting routing entry is
 * stored in cache, if no error. We also cache the ARP entry if the routing
 * entry is indirect.
 * Return value is 0 on success, error value otherwise.
 */
TM_PROTO_EXTERN int tfRtGet(ttRtCacheEntryPtr rtCachePtr);

/*
 * Decrease ownership count of route when done with it. Recycle the entry if
 * both ownership is zero and route is no longer in the tree.
 */
TM_PROTO_EXTERN void tfRtUnGet(ttRteEntryPtr rtePtr);

/*
 * Called when a routing cache need to be emptied.
 * If one bit of the passed rteFlag is set on the rteFlag of the cached
 * routing entry, we release ownership of the cached routing entry
 * and cached corresponding ARP entry (cached if routing entry is indirect).
 * Called by the socket interface when the user closes the socket
 * to release the socket routing cache entry, or by tfSocketResetCaches()
 * or tfIpResetCaches() (both called by tfRtResetCaches in the router code,
 * when all the routing cache entries in the system need to be reset,
 * because a either a new route has been added, or because we need some room
 * in the ARP cache).
 */
TM_PROTO_EXTERN void tfRtCacheReset(ttRtCacheEntryPtr rtcPtr,
                                    tt16Bit           rteFlag);

/*
 * Same as tfRtCacheReset with rteFlag set at 0xFFFF. Called when a routing
 * cache need to be emptied. We release ownership of the cached routing entry
 * and cached corresponding ARP entry (cached if routing entry is indirect).
 */
TM_PROTO_EXTERN void tfRtCacheUnGet(ttRtCacheEntryPtr rtcPtr);

#ifdef TM_USE_ARP_FLUSH
/*
 * Called by tfArpFlush() to flush the ARP table for the passed address
 * family/families
 */
TM_PROTO_EXTERN void tfRtArpFlush(int af
#ifdef TM_LOCK_NEEDED
                                , tt8Bit lockFlag
#endif /* TM_LOCK_NEEDED */
                                 );
#endif /* TM_USE_ARP_FLUSH */

#ifdef TM_USE_STOP_TRECK
/*
 * Called by tfRtDeInit() to flush the routing and ARP tables for all
 * address families.
 */
TM_PROTO_EXTERN void tfRtFlush(int      af,
                               tt16Bit  flags
#ifdef TM_LOCK_NEEDED
                             , tt8Bit   lockFlag
#endif /* TM_LOCK_NEEDED */
                                 );
#endif /* TM_USE_STOP_TRECK */

/* Set physical address mapping to a routing table ARP Entry */
TM_PROTO_EXTERN int tfRtArpAddByRte (
    ttRteEntryPtr       rtePtr,
    tt8BitPtr           physAddrPtr,
    tt32Bit             ttl,
    tt8Bit              physAddrLen );

#ifdef TM_USE_IPV4
/*
 * Add/Set an ARP Entry to routing table for a given destination IP address/
 * Physical address. Returns 0 on success, error value on failure.
 */
TM_PROTO_EXTERN int tf4RtArpAddEntry(
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr    devPtr,
#endif /* TM_USE_STRONG_ESL */
    tt4IpAddress        ipAddress,
    tt8BitPtr           physAddrPtr,
    tt32Bit             ttl,
    tt8Bit              physAddrLen );
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
/*
 * Add/Set an ARP Entry to routing table for a given destination IP address/
 * Physical address. Returns 0 on success, error value on failure.
 */
TM_PROTO_EXTERN int tf6RtArpAddEntry(
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr    devPtr,
#endif /* TM_USE_STRONG_ESL */
    tt6IpAddressPtr     ipAddressPtr,
    tt8BitPtr           physAddrPtr,
    tt32Bit             ttl,
    tt8Bit              physAddrLen );
#endif /* TM_USE_IPV6 */


/*
 * Execute a user ARP entry command as set in arpmCommand field of
 * arpMappingPtr:
 * TM_ARP_GET_BY_PHYS:
 * Get an ARP Entry mapping in routing table using Physical address to
 * locate.
 * TM_ARP_GET_BY_IP:
 * Get an ARP Entry mapping in routing table using IP address to locate
 * TM_ARP_DEL_BY_IP:
 * Erase an ARP Entry mapping in routing table using IP address to locate
 * TM_ARP_DEL_BY_PHYS:
 * Erase an ARP Entry mapping in routing table using Physical address to
 * locate.
 * TM_ARP_SET_TTL
 * Change ttl of ARP entry.
 */
TM_PROTO_EXTERN int tfRtArpCommand(ttArpMappingPtr arpMappingPtr);

#ifdef TM_SNMP_MIB
/* SNMP Agent calls tfSnmpRtFindIpRouteDest to get a copy of a routing entry */
TM_PROTO_EXTERN int tfSnmpRtFindIpRouteDest(
#ifdef TM_USE_STRONG_ESL
                                            ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
                                            ttRteEntryPtr    ipRouteEntryPtr,
                                            tt4IpAddress     ipRouteDest);
#ifdef TM_USE_NG_MIB2
TM_PROTO_EXTERN int tfSnmpNgRtFindIpRouteDest(
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
    ttRteEntryPtr    ipRouteEntryPtr, ttIpAddressPtr  ipRouteDest);
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */

/*
 * Walk down the tree, and for each reached leaf call the routing entry
 * processing function
 */
TM_PROTO_EXTERN int tfRtTreeWalk( ttIntRtFuncPtr processRtePtr,
                                  ttVoidPtr      voidPtr,
                                  tt8Bit         needLock );

/* Print the content of the routing tree */
TM_PROTO_EXTERN void tfRtTreePrint (void);

/* remove gateway entries that has a certain gateway ip address */
TM_PROTO_EXTERN int tfRtRemoveGwEntry(ttIpAddressPtr ipAddrPtr);

/*
 * Given a desintation address, return output device.
 */
TM_PROTO_EXTERN ttUserInterface tfRtInterface(
                struct sockaddr_storage TM_FAR * peerPtr
                );

#ifdef TM_USE_IPV4
/* Update the multihome index for the IPv4 default gateway routing entry. */
TM_PROTO_EXTERN int tf4RtModifyDefGatewayMhome(
#ifdef TM_USE_STRONG_ESL
                                               ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
                                               tt16Bit          mhomeIndex);
#endif /* TM_USE_IPV4 */

#if defined(TM_USE_IKEV2) && defined(TM_IKEV2_REMOTE_SERVER) 
TM_PROTO_EXTERN int tfRtGetGwFromDest(
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr        devPtr,
#endif /* TM_USE_STRONG_ESL */
    const ttIpAddressPtr    destIpAddrPtr,
    const ttIpAddressPtr    netMaskPtr,
    ttIpAddressPtr          gatewayIpAddrPtr);
#endif /* defined(TM_USE_IKEV2) && defined(TM_IKEV2_REMOTE_SERVER) */

/* FTPD, FTP, NAT socket address to h1,h2,h3,h4,p1,p2 conversion */
TM_PROTO_EXTERN int tfFtpSockAddrToHxPxFormat( ttSockAddrInPtr   sockAddrPtr,
                                               ttCharPtr         bufferPtr );
/* FTPD, FTP, NAT h1,h2,h3,h4,p1,p2 to socket address conversion */
TM_PROTO_EXTERN int tfFtpHxPxFormatToSockAddr( ttCharPtr       bufferPtr,
                                               ttSockAddrInPtr sockAddrInPtr );
#ifdef TM_USE_STOP_TRECK
TM_PROTO_EXTERN void tfFtpdDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                );
#endif /* TM_USE_STOP_TRECK */

#ifdef TM_USE_STOP_TRECK
TM_PROTO_EXTERN void tfTftpDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                );
#endif /* TM_USE_STOP_TRECK */

#ifdef TM_USE_STOP_TRECK
TM_PROTO_EXTERN void tfTftpdDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                );
#endif /* TM_USE_STOP_TRECK */

#ifdef TM_USE_STOP_TRECK
TM_PROTO_EXTERN void tfTeldDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                );
#endif /* TM_USE_STOP_TRECK */

/*
 * convert unsigned long decimal from binary to ASCII. Returns pointer to end
 * of string containing the converted number.
 */
TM_PROTO_EXTERN ttCharPtr tfUlongDecimalToString(ttCharPtr setBufPtr,
                                                 tt32Bit   data);

/*
 * Common between FTP server and Telnet server.
 */
/*
 * Remove a server queue entry from either the run Q, or open connection
 * list as given by queueIndex, if entry is in the queue.
 */
TM_PROTO_EXTERN void tfServerRemoveFromQueue(
                                    ttServerQueueHeadPtr  servQueueHeadPtr,
                                    ttServerQueuePtr      servQueuePtr,
                                    int                   queueIndex );

/*
 * Insert a server queue entry in either the run Q, or open connection
 * list as given by queueIndex, if entry is not already in the queue.
 */
TM_PROTO_EXTERN void tfServerAppendToQueue(
                                    ttServerQueueHeadPtr  servQueueHeadPtr,
                                    ttServerQueuePtr      servQueuePtr,
                                    int                   queueIndex);

#ifdef TM_PPP_LQM
TM_PROTO_EXTERN void tfLqmEnable(
                        ttPppVectPtr    pppVectPtr,
                        tt32Bit         negotiatedLqrTimerHsec,
                        tt32Bit         peerLqrTimerHsec);

TM_PROTO_EXTERN void tfLqmDisable(
                        ttPppVectPtr    pppVectPtr);

TM_PROTO_EXTERN int tfLqmIsEnabled(
                        ttPppVectPtr    pppVectPtr);

/* called by PPP to process a Link-Quality-Report message */
TM_PROTO_EXTERN int tfLqmIncomingPacket(
                        ttPppVectPtr    pppVectPtr,
                        ttPacketPtr     packetPtr);
#endif /* TM_PPP_LQM */

/* Open a PPP CLIENT session */
TM_PROTO_EXTERN int tfPppClientOpen(void TM_FAR * interfaceId,
                                    tt16Bit       multihomeIndex,
                                    int           protocolFamily);

/* Open a PPP Server session */
TM_PROTO_EXTERN int tfPppServerOpen(void TM_FAR * interfaceId,
                                    tt16Bit       multihomeIndex,
                                    int           protocolFamily);

/* Close a PPP session */
TM_PROTO_EXTERN int tfPppClose(void TM_FAR * interfaceId);

/* User notification of a link event */
TM_PROTO_EXTERN void tfPppUserNotify(
                        ttDeviceEntryPtr devEntryPtr,
                        int              llEvent);

TM_PROTO_EXTERN int tfPppIsOpen(
                        ttPppVectPtr    pppVectPtr);

/* Common deviceSend routine when called from PPP */
TM_PROTO_EXTERN int tfPppDeviceSend(
                        ttPppVectPtr    pppVectPtr,
                        ttPacketPtr     packetPtr,
                        tt16Bit         pppProtocol);

/*
 * Allocate a PPP state vector
 */
TM_PROTO_EXTERN ttPppVectPtr tfPppVectAlloc(ttDeviceEntryPtr devEntryPtr);

#ifdef TM_USE_PPP_MSCHAP
TM_PROTO_EXTERN void tfMChapOldNtPwhashEncNewNtPwhash(
                              tt8BitPtr         newPasswordPtr,
                              tt32Bit           newPswdLength,
                              tt8BitPtr         oldPasswordPtr,
                              tt32Bit           oldPswdLength,
                              tt8BitPtr         encPwHashPtr);

TM_PROTO_EXTERN void tfMChapV1NewPwEncwithOldNtPwhash(
                              tt8BitPtr         newPasswordPtr,
                              tt32Bit           newPswdLength,
                              tt8BitPtr         oldPasswordPtr,
                              tt32Bit           oldPswdLength,
                              tt8BitPtr         encPwblockPtr);
TM_PROTO_EXTERN void tfMChapV1NtChallengeResponse(
                                tt8BitPtr  challenge,
                                tt8BitPtr  password,
                                tt32Bit    pswdLength,
                                tt8BitPtr  response);

#endif /* TM_USE_PPP_MSCHAP */


#ifdef TM_DSP
TM_PROTO_EXTERN void tfByteByByteCopy( int * srcAddr,
                                       int   srcOffset,
                                       int * destAddr,
                                       int   destOffset,
              int length);

TM_PROTO_EXTERN void tfPppByteCopy( int *        sourcePtr,
                                    unsigned int sourceOffset,
                                    int *        destPtr,
                                    unsigned int destOffset,
                                    unsigned int bytes);
TM_PROTO_EXTERN int tfMemCopyOffset( int * srcAddr,
                                     int   srcOffset,
                                     int * destAddr,
                                     int   destOffset,
                                     int   length);

TM_PROTO_EXTERN void tfBcopyToPacked( const void TM_FAR * source,
                                      void TM_FAR *       destination,
                                      unsigned int        packedLength,
                                      unsigned int        offset);

TM_PROTO_EXTERN void tfBcopyToUnpacked(const void * sourcePtr,
                                       int          srcOffset,
                                       void *       destPtr,
                                       unsigned int length);
#endif /* TM_DSP */

#ifdef TM_USE_IPV4
TM_PROTO_EXTERN int tfStartArpSend (
    ttDeviceEntryPtr devPtr,
    ttUserIpAddress  ipAddress,
    int              numberArpProbes,
    ttUser32Bit      arpProbeInterval,
    ttUser32Bit      timeout );
#endif /* TM_USE_IPV4 */

/* support for debug, error and trace logging to circular buffer */
TM_PROTO_EXTERN ttLogCtrlBlkPtr tfLogAllocBuf(
    tt16Bit maxLogMsgLen, tt16Bit numLogMsgs);
TM_PROTO_EXTERN void tfLogFreeBuf(
    ttLogCtrlBlkPtr lcbPtr);
TM_PROTO_EXTERN void tfLogStop(void);
TM_PROTO_EXTERN int tfLogMsg(
    ttLogCtrlBlkPtr lcbPtr, const char TM_FAR *pszMsgPrefix,
    const char TM_FAR *pszFormat, ...);
TM_PROTO_EXTERN int tfLogWalk( ttLogWalkFuncPtr funcPtr,
                               ttLogCtrlBlkPtr lcbPtr,
                               int msgSeqNo,
                               ttUserGenericUnion genParam);
/* Use tfLogWalkNoAlloc instead of tfLogWalk when TM_MEMORY_LOGGING is
   enabled. NOTE: when using TM_MEMORY_LOGGING, avoid doing any operation
   that requires Treck memory allocation when walking the log buffer. For
   example, have your log walk function call the RTOS printf() function to
   output log messages, since that does not use any Treck memory. */
TM_PROTO_EXTERN int tfLogWalkNoAlloc( ttLogWalkFuncPtr funcPtr,
                                      ttLogCtrlBlkPtr lcbPtr,
                                      ttLogCtrlBlkPtr lcbCopyPtr,
                                      int lcbCopyLen,
                                      int msgSeqNo,
                                      ttUserGenericUnion genParam);
#ifdef TM_USE_LOGGING_LEVELS
TM_PROTO_EXTERN int tfLogSetLevel(tt32Bit module, tt8Bit level, tt8Bit verbosity);
TM_PROTO_EXTERN int tfLogGetLevel(tt32Bit module, tt8BitPtr levelPtr, tt8BitPtr verbosityPtr);
TM_PROTO_EXTERN void tfEnhancedLogWrite(tt32Bit module, 
                                            tt8Bit level, 
                                            ttConstCharPtr msgPtr, 
                                            ...);
TM_PROTO_EXTERN void tfInitEnhancedLogging(void);
#endif /* TM_USE_LOGGING_LEVELS */
#if (defined(TM_OPTIMIZE_MANY_MHOMES) && !defined(TM_SINGLE_INTERFACE_HOME))
/* APIs for sorting IP addresses to improve performance of IP address lookup
   in the receive-path when many IP aliases are configured on a single
   interface. */

/* tfRemoveSortedIpAddr supports both IPv4 and IPv6 */
TM_PROTO_EXTERN void tfRemoveSortedIpAddr(
    ttDeviceEntryPtr removeDevEntryPtr, tt16Bit removeMHomeIndex,
    ttSortedIpAddrCachePtr ipAddrCachePtr);

#ifdef TM_USE_IPV4
TM_PROTO_EXTERN void tf4InsertSortedIpAddr(
    ttDeviceEntryPtr insertDevEntryPtr, tt16Bit insertMHomeIndex,
    ttSortedIpAddrCachePtr ipAddrCachePtr, tt8Bit dBroadFlag);
TM_PROTO_EXTERN ttIpAddrEntryPtr tf4LookupSortedIpAddr(
    tt4IpAddress ipAddr, int devIndex, ttSortedIpAddrCachePtr ipAddrCachePtr,
    tt8Bit dBroadFlag);
#endif /* TM_USE_IPV4 */
#endif /* TM_OPTIMIZE_MANY_MHOMES and not TM_SINGLE_INTERFACE_HOME */

/*
 * SNMP agent caching routines for ARP table cache, routing table cache,
 * UDP sockets table cache, and TCP vectors cache
 */
TM_PROTO_EXTERN void tfSnmpdCacheInsertRoute(ttRteEntryPtr rtePtr,
                                             int           cacheIndex);
TM_PROTO_EXTERN void tfSnmpdCacheDeleteRoute(ttRteEntryPtr rtePtr);
TM_PROTO_EXTERN void tfSnmpdCacheInsertSocket(ttSocketEntryPtr socketPtr);
TM_PROTO_EXTERN void tfSnmpdCacheDeleteSocket(ttSocketEntryPtr socketPtr);
TM_PROTO_EXTERN void tfSnmpdCacheInsertTmWtVect (ttTcpTmWtVectPtr tcpTmWtVectPtr);
TM_PROTO_EXTERN void tfSnmpdCacheDeleteTmWtVect (ttTcpTmWtVectPtr tcpTmWtVectPtr);
TM_PROTO_EXTERN int tfSnmpdCacheInsertPrefix(tt8Bit            addrFamily,
                                              ttDeviceEntryPtr  devPtr,
                                              tt16Bit           mHomeIndex);
TM_PROTO_EXTERN int tfSnmpdCacheDeletePrefix(tt8Bit            addrFamily,
                                              ttDeviceEntryPtr  devPtr,
                                              tt16Bit           mHomeIndex);

#ifdef TM_USE_IPV4
/* IPv4-specific functions supporting tfNg "Next Generation" APIs */
TM_PROTO_EXTERN int tf4ConfigInterface (
    ttUserInterface interfaceHandle,
    ttUserIpAddress ipAddress,
    ttUserIpAddress netMask,
    int             flags,
    int             buffersPerFrameCount,
    tt16Bit         mHomeIndex );

TM_PROTO_EXTERN int tf4UnConfigInterface (
    ttUserInterface interfaceHandle,
    tt16Bit         mHomeIndex );

/* NOTE: we don't yet have a tfNg function for tfAddInterfaceMhomeAddress */
TM_PROTO_EXTERN int tf4AddInterfaceMhomeAddress(
    ttUserInterface interfaceId,
    ttUserIpAddress ipAddress,
    tt16Bit mHomeIndex );
#endif /* TM_USE_IPV4 */


#ifdef TM_USE_NETSTAT
/* netstat related inernal functions */

/*
 * extract information from a routing entry for ARP
 * and store it in the ttNtArpEntry struct
 */
TM_PROTO_EXTERN int tfNtGetArpEntry(
    ttSnmpCacheEntryPtr     snmpCacheEntryPtr,
    ttNtEntryUPtr           ntEntryPtr);

/*
 * extract information from a routing entry
 * and store it in the ttNtRteEntry struct
 */
TM_PROTO_EXTERN int tfNtGetRteEntry(
    ttSnmpCacheEntryPtr     snmpCacheEntryPtr,
    ttNtEntryUPtr           ntEntryPtr);

/*
 * extract information from a socket entry
 * and store it in the ttNtTcpEntry struct
 */
TM_PROTO_EXTERN int tfNtGetTcpEntry(
    ttSnmpCacheEntryPtr     snmpCacheEntryPtr,
    ttNtEntryUPtr           ntEntryPtr);

/*
 * extract information from a socket entry
 * and store it in the ttNtUdpEntry struct
 */
TM_PROTO_EXTERN int tfNtGetUdpEntry(
    ttSnmpCacheEntryPtr     snmpCacheEntrPtr, 
    ttNtEntryUPtr           ntEntryPtr);

#ifdef TM_USE_IPSEC
TM_PROTO_EXTERN int tfNtGetSadbEntry(
    ttSadbRecordPtr         sadbRecordPtr,
    ttNtSadbEntryPtr        ntSadbEntryPtr);
TM_PROTO_EXTERN int tfNtGetIpsecSpdEntry(
    ttPolicyEntryPtr        plcyPtr,
    ttNtIpsecSpdEntryPtr    ntIpsecSpdEntryPtr);

#ifdef TM_USE_IPSEC_64BIT_KB_LIFETIMES
TM_PROTO_EXTERN void tfIpsecGlobalInitialize(void);
#endif /* TM_USE_IPSEC_64BIT_KB_LIFETIMES */

#endif /* TM_USE_IPSEC */


#ifdef TM_6_USE_MIP_CN
/*
 * extract information from a binding entry
 * and store it in the ttNtBindingEntry struct
 */
int tfNtGetBindingEntry(
        tt6BindingEntryPtr   bindingEntryPtr,
        tt6NtBindingEntryPtr ntBindingEntryPtr);
#endif /* TM_6_USE_MIP_CN */

/*
 * extract information from a devce entry
 * and store it in the ttNtDevEntry struct
 */
int tfNtGetDeviceEntry(
    ttDeviceEntryPtr    devPtr,
    ttNtDevEntryPtr     ntDevPtr);

#ifdef TM_USE_IPV4
/*
 * extract information from a NAT entry
 * and store it in the ttNtNatEntry struct
 */
void tfNtGetNatEntry(ttNatTriggerPtr    trigPtr,
                     ttNtNatEntryPtr    ntNatPtr);
#endif /* TM_USE_IPV4 */

/*
 * converts a HW address into a string in the format of FF:FF:...FF
 * returns the result string buffer if sucess,
 * returns NULL if input parameter is wrong */
TM_PROTO_EXTERN char TM_FAR * tfNtHwAddrToStr(
    ttUser8Bit TM_FAR * hwAddrPtr,
    int                 len,
    char TM_FAR *       buffer);

/* print route flags as a string */
TM_PROTO_EXTERN char TM_FAR * tfNtRteFlagToStr(
    ttUser16Bit     flags,
    char TM_FAR *   buffer);

TM_PROTO_EXTERN char TM_FAR * tfNtNatFlagToStr(
    ttUser8Bit      flags,
    char TM_FAR *   buffer);

/* print TCP state as a string */
TM_PROTO_EXTERN const char TM_FAR * tfNtGetTcpStateStr(ttUser8Bit state);

#ifdef TM_ERROR_CHECKING
#ifdef TM_DEBUG_LOGGING
TM_PROTO_EXTERN int tfxNtArpEntryCBLog(
    ttNtEntryUPtr       ntEntryUPtr,
    ttUserGenericUnion  genParam1,
    ttUserGenericUnion  genParam2);
TM_PROTO_EXTERN int tfxNtRteEntryCBLog(
    ttNtEntryUPtr       ntEntryUPtr,
    ttUserGenericUnion  genParam1,
    ttUserGenericUnion  genParam2);
TM_PROTO_EXTERN void tfxLogArpTable(const char * msg);
TM_PROTO_EXTERN void tfxLogRteTable(const char * msg);
TM_PROTO_EXTERN void tfxLogRteEntry(const char * msg, ttRteEntryPtr rtePtr);
#endif /* TM_ERROR_CHECKING */
#endif /* TM_DEBUG_LOGGING */
#endif /* TM_USE_NETSTAT */

#ifdef TM_USE_BSD_DOMAIN
/* BSD socket functions for multi domain support */

TM_PROTO_EXTERN int tfBsdSocket(
    int addressFamily,
    int socketType,
    int protocol);

TM_PROTO_EXTERN int tfBsdListen(
    ttVoidPtr   soVoidPtr,
    int         backLog);

TM_PROTO_EXTERN int tfBsdBind(
    ttVoidPtr                       soVoidPtr,
    const struct sockaddr TM_FAR *  localAddressPtr,
    int                             localAddressLength);

TM_PROTO_EXTERN int tfBsdSnd(
    ttVoidPtr     soVoidPtr,
    char TM_FAR * bufferPtr,
    int           bufferLength,
    int           flags);

TM_PROTO_EXTERN int tfBsdRecv(
    ttVoidPtr     soVoidPtr,
    char TM_FAR * bufferPtr,
    int           bufferLength,
    int           flags );

TM_PROTO_EXTERN int tfBsdSend(
    ttVoidPtr                       soVoidPtr,
    char TM_FAR *                   bufferPtr,
    int                             bufferLength,
    int                             flags );

TM_PROTO_EXTERN int tfBsdSendTo(
    ttVoidPtr                       soVoidPtr,
    char TM_FAR *                   bufferPtr,
    int                             bufferLength,
    int                             flags,
    const struct sockaddr TM_FAR *  toAddressPtr,
    int                             addressLength );

TM_PROTO_EXTERN int tfBsdRecvFrom(
    ttVoidPtr                soVoidPtr,
    char            TM_FAR * bufferPtr,
    int                      bufferLength,
    int                      flags,
    struct sockaddr TM_FAR * fromAddressPtr,
    int             TM_FAR * addressLengthPtr );

TM_PROTO_EXTERN int tfBsdReadv(
    ttVoidPtr             soVoidPtr,
    struct iovec TM_FAR * io,
    int                   iocount );

TM_PROTO_EXTERN int tfBsdWritev(
    ttVoidPtr             soVoidPtr,
    struct iovec TM_FAR * io,
    int                   iocount );


TM_PROTO_EXTERN int tfBsdAccept(
    ttVoidPtr                soVoidPtr,
    struct sockaddr TM_FAR * peerAddressPtr,
    int             TM_FAR * addressLengthPtr );

TM_PROTO_EXTERN int tfBsdSetSockOpt(
    ttVoidPtr           soVoidPtr,
    int                 levelOfLayer,
    int                 optionName,
    const char TM_FAR * optionValuePtr,
    int                 optionLength );

TM_PROTO_EXTERN int tfBsdGetSockOpt(
    ttVoidPtr     soVoidPtr,
    int           levelOfLayer,
    int           optionName,
    char TM_FAR * optionValuePtr,
    int  TM_FAR * optionLengthPtr );

TM_PROTO_EXTERN int tfBsdGetPeerName(
    ttVoidPtr                soVoidPtr,
    struct sockaddr TM_FAR * fromAddressPtr,
    int             TM_FAR * addressLengthPtr );

TM_PROTO_EXTERN int tfBsdGetSockName(
    ttVoidPtr                soVoidPtr,
    struct sockaddr TM_FAR * myAddressPtr,
    int             TM_FAR * addressLengthPtr );

TM_PROTO_EXTERN int tfBsdShutDown(
    ttVoidPtr soVoidPtr,
    int       howToShutdown);

TM_PROTO_EXTERN int tfBsdConnect(
    ttVoidPtr                       soVoidPtr,
    const struct sockaddr TM_FAR *  addressPtr,
    int                             addressLength );

TM_PROTO_EXTERN int tfBsdIoctl   (
    ttVoidPtr               soVoidPtr,
    u_long                  command,
    caddr_t                 data);

TM_PROTO_EXTERN int tfBsdClose   (ttVoidPtr soVoidPtr);

TM_PROTO_EXTERN int tfBsdAbort   (ttVoidPtr soVoidPtr);

/* select() will call the function */
TM_PROTO_EXTERN int tfBsdSelectCheckEvent (
                        ttVoidPtr           soVoidPtr,
                        ttPendEntryPtr      pendEntryPtr,
                        int                 checkMask );

TM_PROTO_EXTERN int  tfBsdGetSocketLock ( ttVoidPtr soVoidPtr);

TM_PROTO_EXTERN void tfBsdMbufInit(void);

#endif /* TM_USE_BSD_DOMAIN */

#ifdef TM_USE_SNIFF
/* open a file, append an ethernet frame into it, and close it */
TM_PROTO_EXTERN int tfPcapWriteFrame(
    tt16Bit linkLayerProtocol,
    char*   dataPtr,
    int     dataLen);
TM_PROTO_EXTERN int tfPcapWritePacket(ttPacketPtr packetPtr);
#endif /* TM_USE_SNIFF */

/* if _WIN32 is defined, print in the right location with good color,
 * else do nothing
 */
TM_PROTO_EXTERN int tfKernelPrint(
    int msgType, const char TM_FAR * format, ...);

/* Generic linked list, with count. */

/* Initilazing the list, must be called for a list before it is used */
TM_PROTO_EXTERN void tfListInit(ttListPtr listPtr);

/* Add node at the head of a list */
TM_PROTO_EXTERN void tfListAddToHead(ttListPtr listPtr, ttNodePtr nodePtr);

/* Add a node to the tail of a list */
TM_PROTO_EXTERN void tfListAddToTail(ttListPtr listPtr, ttNodePtr nodePtr);

#if defined(TM_MULTIPLE_CONTEXT) || defined(TM_6_USE_MIP_CN)
/* Check that nodePtr is in the list pointed to by listPtr */
TM_PROTO_EXTERN tt8Bit tfListMember(ttListPtr listPtr, ttNodePtr nodePtr);
#endif /* TM_MULTIPLE_CONTEXT || TM_6_USE_MIP_CN */

/* remove a node from the list */
TM_PROTO_EXTERN void tfListRemove(ttListPtr listPtr, ttNodePtr nodePtr);

/* move a node to the front and keep the order of the rest of the nodes */
TM_PROTO_EXTERN void tfListMoveToFront(ttListPtr listPtr, ttNodePtr nodePtr);

/* Replace a node with an new one */
TM_PROTO_EXTERN void tfListReplace(ttNodePtr oldNodePtr, ttNodePtr newNodePtr);

/*
 * Traverse the list, for each node in the list, call the provided call
 * back function listCBFuncPtr with genParam,
 *
 * use to find node with customized matching cretiria, enum list,
 * update list, etc...
 *
 * continues if listCBFuncPtr returns TM_8BIT_NO,
 * stops and return the current node if the listCBFuncPtr returns TM_8BIT_YES
 * return TM_NODE_NULL_PTR if the whole list is Traversed without getting
 * a TM_8BIT_YES from the call back function.
 */
TM_PROTO_EXTERN ttNodePtr tfListWalk(ttListPtr          listPtr,
                                     ttListCBFuncPtr    listCBFuncPtr,
                                     ttGenericUnion     genParam);

/* Free all the node in this list, you can only call tfListFree() if all the
 * nodes that have been allocated with tm_get_raw_buffer(), since tfListfree()
 * will call tm_free_raw_buffer for every node in the list.
 */
TM_PROTO_EXTERN void tfListFree(ttListPtr listPtr);

typedef void (TM_CODE_FAR * ttAuthNotifyFuncPtr)(
    ttDeviceEntryPtr devEntryPtr,
    int              event,
    int              remoteLocalFlag,
    tt8Bit           needLockFlag);

/*
 * List iterator support.
 */
TM_PROTO_EXTERN void tfIterateThis(ttIteratorPtr itPtr, ttListPtr listPtr);
TM_PROTO_EXTERN int tfIterateHasNext(ttIteratorPtr itPtr);
TM_PROTO_EXTERN ttVoidPtr tfIterateNext(ttIteratorPtr itPtr);


TM_PROTO_EXTERN int tfEapOpen(ttVoidPtr  eapVectParmPtr,
                              tt8Bit     authPeerFlag);
TM_PROTO_EXTERN void tfEapIncomingPacket(ttVoidPtr    eapVectParmPtr,
                                         ttPacketPtr  packetPtr);
TM_PROTO_EXTERN int tfEapInit(ttVoidPtrPtr         eapVectParmPtrPtr,
                              ttDeviceEntryPtr     devEntryPtr,
                              ttAuthNotifyFuncPtr  notifyFuncPtr);
TM_PROTO_EXTERN int tfEapClose(ttVoidPtr eapVectParmPtr);
TM_PROTO_EXTERN void tfEapDeInit(ttVoidPtrPtr eapVectParmPtrPtr);

#ifdef TM_USE_PPPOE
/* PPPoE incoming function called from tfEtherRecv() */
TM_PROTO_EXTERN void tfPppoeIncomingPacket(ttPacketPtr  packetPtr,
                                           tt16Bit      ethType);

/* Link layer notify function to be called by PPP on PPPoE interfaces */
TM_PROTO_EXTERN void tfPppoePppCB(ttUserInterface interfaceId,
                                  unsigned int flags);

/* PPP incoming function called from tfPppoeIncomingPacket() */
TM_PROTO_EXTERN void tfPppPppoeIncomingPacket(ttPacketPtr  packetPtr,
                                              tt16Bit      pppProtocol);
/*
 * Close a PPP connection immediately (without sending PPP Terminate
 * messages)
 */
TM_PROTO_EXTERN int tfPppPppoeTerminate (ttPppVectPtr pppVectPtr);
#endif /* TM_USE_PPPOE */

/*  Given the length of a buffer to hold a base64 encoded stream,
 *  compute the length of input buffer that can be accomodated.
 *  Note that this cannot be done acurately, since the actuall
 *  encode length of a buffer depends on the real content of the
 *  input buffer. This would be a estimation at the lower end.
 */
TM_PROTO_EXTERN int tfBase64InputLen(int outLen);

/*  Given a buffer and length of the buffer, compute the length of
 *  the Base64 encoded stream.
 */
TM_PROTO_EXTERN int tfBase64OutputLen(ttCharPtr inPtr, int inLen);

/*  BASE64 encoding routing */
TM_PROTO_EXTERN int tfBase64Encode(
    ttCharPtr inPtr, int inLen, ttCharPtr outPtr, int outLen);

/*  Calculate the length of the string created by decoding the given
 *  encoded string. */
TM_PROTO_EXTERN int tfBase64DecodedLen(ttCharPtr encodedPtr, int encodedLen);

/*  Decode the string in inPtr, saving the decoded text in outPtr. */
TM_PROTO_EXTERN int tfBase64Decode(ttCharPtr inPtr,
                                   int       inLen,
                                   ttCharPtr outPtr,
                                   int       outLen);

#ifdef TM_MULTIPLE_CONTEXT
/* Get a network ID that hasn't been used yet */
TM_PROTO_EXTERN ttUser32Bit tfNetworkGetUnusedId(void);

/* Associate a interface to an network, this is to simulate plugging
 * a interface into a certain network
 */
TM_PROTO_EXTERN int tfNetworkSetId(
    ttUserInterface interfaceHandle,
    ttUser32Bit     networkId);

#endif /* TM_MULTIPLE_CONTEXT */

/* Given the time lapsed since midnight 1970, Jan 1,
 * Get the time structure representation
 */
TM_PROTO_EXTERN int tfGetUtcTime (
    ttConst32BitPtr daysPtr,
    ttConst32BitPtr secondsPtr,
    ttTimePtr        timePtr);

/* Convert the time strcuture to a string of the specified format
 * valid formats are
 * TM_TIME_FORMAT_HTTP     0
 * TM_TIME_FORMAT_SMTP     1
 */
TM_PROTO_EXTERN ttCharPtr tfGetTimeString(
    ttTimePtr timePtr, ttCharPtr timeStrPtr,
    int timeStrLen, int format);

/* Init Global HTTP/HTTPD pointers to const tables */
TM_PROTO_EXTERN void tfHttpInitGlobalTablesPtr(void);

/* tfHttpdGetCurrentTime function description
 *   Get the current time in HTTP date header format specified in
 *   [RFC1945]R3.3:20: UT format
 */
TM_PROTO_EXTERN ttCharPtr tfHttpGetCurrentTime(
    ttCharPtr timeBufferPtr, int timeBufferLen);

/* tfHttpdGetChunkLen function description
 *   Given an HTTP chunk header, extract and save the length of the chunk
 */
TM_PROTO_EXTERN int tfHttpGetChunkLen(ttCharPtr  chunkLine,
                                      int        lineLen,
                                      tt32BitPtr valPtr);
#ifdef TM_USE_STOP_TRECK
TM_PROTO_EXTERN void tfHttpdDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                );
#endif /* TM_USE_STOP_TRECK */

/* Find the corresponding type index for a given header type string. */
TM_PROTO_EXTERN int tfHttpFindHeaderIndex(ttCharPtr   headerTypeStrPtr);

#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
TM_PROTO_EXTERN ttVoidPtr tfSslGetSessionUsingIndex(
    int        sessionNumber);
#endif /* TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */

#ifdef TM_USE_DOS_FS
/* We consider this directory our root, no files outside of this directory
 * is allowed to be acessed
 */
TM_PROTO_EXTERN void tfSetDosRootDir(ttConstCharPtr rootDirPtr);

/* Set the DOS root directory to be the current working directory */
TM_PROTO_EXTERN void tfInitDosFs(void);

#endif /* TM_USE_DOS_FS*/

#ifdef TM_USE_EMU_UNIX_FS
/* We consider this directory our root, no files outside of this directory
 * is allowed to be acessed
 */
TM_PROTO_EXTERN void tfSetEmuUnixRootDir(ttConstCharPtr rootDirPtr);

/* Set the DOS root directory to be the current working directory */
TM_PROTO_EXTERN void tfInitEmuUnixFs(void);

#endif /* TM_USE_EMU_UNIX_FS */

/* Check whether a file matches to one of the filters provided as
 * a list of ttFileFilter
 */
TM_PROTO_EXTERN int tfMatchFile (
    ttUserConstCharPtr  filePathPtr,
    ttListPtr           filterListPtr);

/*
 * Given a port mask, return a unique phys port.
 * (If no bit, or more than one bit is set, return -1 (for wild card))
 */
TM_PROTO_EXTERN int tfPortMaskToPhysPort(tt32Bit portMask);

#ifdef TM_USE_UPNP_DEVICE
/* tfUpnpInit does one-time UPnP initialization that needs to be done
   before the user application calls tfUpnpUserStart */
TM_PROTO_EXTERN void tfUpnpInit(void);

#ifdef TM_USE_STOP_TRECK
TM_PROTO_EXTERN void tfUpnpDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                );
#endif /* TM_USE_STOP_TRECK */

/* initialize SSDP */
TM_PROTO_EXTERN int tfSsdpInit(int maxNumSsdpEntries);
    
/* deinitialize SSDP: calls tfSsdpStopAdvertInterface() for all active SSDP
   interfaces and then frees up anything that was allocated in tfSsdpInit */
TM_PROTO_EXTERN void tfSsdpStop(void);

/** 
 * This function is called by tfRemoveInterface when a network interface
 * (or single multihomed address) is unconfigured. 
 * tfSsdpStopAdvertInterface stops sending all advertisements on the
 * specified interface and removes any instances corresponding to this
 * interface, multihome and address family from the list of active
 * SSDP instances.
 */
TM_PROTO_EXTERN int tfSsdpStopAdvertInterface(
    ttDeviceEntryPtr devEntryPtr, 
    int              mHomeIndex, 
    int              addrFamily);

/**
 * Creates a HTTPU message and returns a handle to a zero copy buffer
 * containing the new message and returns the length of the new message.
 * The message may then be sent immediately by calling tfZeroCopySend 
 * or queued for later transmission by calling tfSsdpNewTxRequest.  
 * Caller needs to make sure that the passed in msgType, headerType are
 * all valid values, headerValues array must have headerCount null 
 * terminated strings inside.
 */
TM_PROTO_EXTERN int tfHttpuCreateMsg(
    int                   msgType, 
    int                   headerType[], 
    ttConstCharPtr        headerValue[],
    int                   headerCount,
    ttUserMessage TM_FAR *retMsgHandlePtr,
    ttIntPtr              retMsgLengthPtr);

/** 
 * This function receives a HTTP message via UDP, parses the message and
 * returns information about the headers and the message type to the 
 * caller. Since this is an internal API, it does not validate the input
 * arguments, so the caller needs to make sure that all pointers are  
 * valid before calling this routine.
 */
TM_PROTO_EXTERN ttUserMessage tfHttpuRecvMsg(
    ttIntPtr        msgTypePtr,
    ttIntPtr        headerTypePtr,
    ttCharPtrPtr    headerValuePtrPtr,
    ttIntPtr        headerCountPtr,
    struct sockaddr_storage TM_FAR *fromSockAddrPtr,
    ttIntPtr        errorCodePtr);

/* Lock the UPnP state vector, and increase the owner count */
TM_PROTO_EXTERN ttUpnpVectPtr tfCkoutAndLockUpnpVect(void);

/* Unlock the UPnP state vector, decrement the owner count and free it if its
   owner count has reached 0 */
#ifdef TM_LOCK_NEEDED
TM_PROTO_EXTERN void tfCkinUpnpVect(int unlockFlag);
#else /* !TM_LOCK_NEEDED */
TM_PROTO_EXTERN void tfCkinUpnpVect(void);
#endif /* !TM_LOCK_NEEDED */

#endif /* TM_USE_UPNP_DEVICE */

#if (defined(TM_USE_UPNP_DEVICE) || defined(TM_USE_SOAP))
/* Convert UPnP state variable value to a utf-8 buffer */
TM_PROTO_EXTERN int tfUpnpStateVariable2UTF8(
    ttVoidPtr            valuePtr,
    int                  valueLength,
    ttUpnpTypecode       valueType,
    tt8BitPtrPtr         utf8StringPtrPtr,
    int TM_FAR *         utf8LenPtr,
    tt8BitPtr            newBufferPtr);

/* Search and Converts a decimal digit string into long value */
TM_PROTO_EXTERN int tfStr2Int(ttConstCharPtr strPtr, tt32BitPtr numberPtr);

/* SOAP action handler and SOAP success function prototypes */
int cg0Soap_SetTarget(
    ttSoapUserReqContextPtr soapUserReqContextPtr,
    struct sockaddr_storage *cpSockAddrPtr,
    ttUser8BitPtr /* boolean */ newTargetValue);
int cg0Success_SetTarget(
    ttSoapUserReqContextPtr soapUserReqContextPtr,
    int flags);
int cg0Soap_GetTarget(
    ttSoapUserReqContextPtr soapUserReqContextPtr,
    struct sockaddr_storage *cpSockAddrPtr);
int cg0Success_GetTarget(
    ttSoapUserReqContextPtr soapUserReqContextPtr,
    int flags,
    ttUser8BitPtr /* boolean */ RetTargetValue);
int cg0Soap_GetStatus(
    ttSoapUserReqContextPtr soapUserReqContextPtr,
    struct sockaddr_storage *cpSockAddrPtr);
int cg0Success_GetStatus(
    ttSoapUserReqContextPtr soapUserReqContextPtr,
    int flags,
    ttUser8BitPtr /* boolean */ ResultStatus);
int cg1Soap_TestAction(
    ttSoapUserReqContextPtr soapUserReqContextPtr,
    struct sockaddr_storage *cpSockAddrPtr,
    ttUser8Bit /* ui1 */ T_inArg_ui1,
    ttUser16Bit /* ui2 */ T_inArg_ui2,
    ttUser32Bit /* ui4 */ T_inArg_ui4,
    ttUserS8Bit /* i1 */ T_inArg_i1,
    ttUserS16Bit /* i2 */ T_inArg_i2,
    ttUserS32Bit /* i4 */ T_inArg_i4,
    ttUserS32Bit /* int */ T_inArg_int,
    ttUser8Bit /* char */ T_inArg_char,
    ttUser8BitPtr /* string */ T_inArg_string,
    ttUser8BitPtr /* date */ T_inArg_date,
    ttUser8BitPtr /* dateTime */ T_inArg_dateTime,
    ttUser8BitPtr /* dateTime.tz */ T_inArg_dateTime_tz,
    ttUser8BitPtr /* time */ T_inArg_time,
    ttUser8BitPtr /* time.tz */ T_inArg_time_tz,
    ttUser8BitPtr /* boolean */ T_inArg_boolean,
    ttUser8BitPtr /* bin.base64 */ T_inArg_bin_base64,
    ttUser8BitPtr /* bin.hex */ T_inArg_bin_hex,
    ttUser8BitPtr /* uri */ T_inArg_uri,
    ttUser8BitPtr /* uuid */ T_inArg_uuid);
int cg1Success_TestAction(
    ttSoapUserReqContextPtr soapUserReqContextPtr,
    int flags,
    ttUser8Bit /* ui1 */ T_outArg_ui1,
    ttUser16Bit /* ui2 */ T_outArg_ui2,
    ttUser32Bit /* ui4 */ T_outArg_ui4,
    ttUserS8Bit /* i1 */ T_outArg_i1,
    ttUserS16Bit /* i2 */ T_outArg_i2,
    ttUserS32Bit /* i4 */ T_outArg_i4,
    ttUserS32Bit /* int */ T_outArg_int,
    ttUser8Bit /* char */ T_outArg_char,
    ttUser8BitPtr /* string */ T_outArg_string,
    ttUser8BitPtr /* date */ T_outArg_date,
    ttUser8BitPtr /* dateTime */ T_outArg_dateTime,
    ttUser8BitPtr /* dateTime.tz */ T_outArg_dateTime_tz,
    ttUser8BitPtr /* time */ T_outArg_time,
    ttUser8BitPtr /* time.tz */ T_outArg_time_tz,
    ttUser8BitPtr /* boolean */ T_outArg_boolean,
    ttUser8BitPtr /* bin.base64 */ T_outArg_bin_base64,
    ttUser8BitPtr /* bin.hex */ T_outArg_bin_hex,
    ttUser8BitPtr /* uri */ T_outArg_uri,
    ttUser8BitPtr /* uuid */ T_outArg_uuid);
#endif /* TM_USE_UPNP_DEVICE || TM_USE_SOAP */

#ifdef TM_USE_SOAP
/* build a SOAP message */
TM_PROTO_EXTERN int tfSoapMsgConstruct(
    ttSoapReqContextPtr soapReqCxtPtr,
    ttGenericUnionPtr   argArray,
    int                 argCount,
    ttSoapMsgType       msgType);
    
/* send a SOAP message */
TM_PROTO_EXTERN int tfSoapMsgSend(ttSoapReqContextPtr soapReqCxtPtr);
/* SOAP handle a web server event */
TM_PROTO_EXTERN int tfSoapHttpdEventHandler(
    ttHttpdUserConHandle    conHandle, 
    ttHttpdUserEvent        event);
#endif /* TM_USE_SOAP */

#ifdef TM_USE_GENA
TM_PROTO_EXTERN void tfGenaHttpdEventHandler(
    ttHttpdUserConHandle    userConHandle, 
    ttHttpdUserEvent        event);
/* deinitialize GENA */
TM_PROTO_EXTERN void tfGenaStop(void);
#endif /* TM_USE_GENA */

#ifdef TM_USE_IPV6
#include <t6proto.h>
#endif /* TM_USE_IPV6 */

#ifdef TM_6_USE_DHCP

/* BOOT function for starting or stopping DHCPv6 */
TM_NEARCALL void tf6DhcpStartStop(ttDeviceEntryPtr devPtr, int ipv6Flags, int start);

/* For DAD to tell DHCPv6 that an duplicate address is detected */
void tf6DhcpDadFailed(ttDeviceEntryPtr devPtr, tt16Bit multiHomeIndex);

/* For tf6UnConfigInterface to tell DHCPv6 to release an address */
int tf6DhcpUnconfig(ttDeviceEntryPtr devPtr, tt16Bit multiHomeIndex);

#endif/* TM_6_USE_DHCP */

/* Expand compressed DNS label */
int tfDnsLabelToAscii( tt8BitPtr labelPtr,
                       tt8BitPtr asciiPtr,
                       tt8BitPtr pktDataPtr,
                       tt8Bit    labelType,
                       int       labelLen);
/* Find the length of the ASCII representation of a DNS label */
int tfDnsLabelAsciiLen( tt8BitPtr labelPtr );

/* Convert a domain name from ASCII to DNS-label format */
int tfDnsAsciiToLabel(ttConstCharPtr  hostnamePtr,
                      int             hostnameLen,
                      tt8BitPtr       labelPtr,
                      int             labelLen);

/*
 * This routine attempts to retrieve information about a hostname and is called 
 * once for each request from the user.  First, the cache is checked for valid 
 * entries; if any are found it is returned to the caller.  If not, a DNS query 
 * is generated and sent.
 */
TM_PROTO_EXTERN int tfDnsGenerateQuery(
                        ttConstCharPtr        hostnameStr,
                        tt16Bit               queryType,
                        int                   addrFamily,
                        ttDnsCacheEntryPtrPtr entryPtrPtr );

#ifdef TM_USE_STOP_TRECK
TM_PROTO_EXTERN void tfDnsDeInit(
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                );
#endif /* TM_USE_STOP_TRECK */




#if (defined(TM_USE_IPV6) && defined(TM_6_USE_DAD))
TM_PROTO_EXTERN int tf6FindAddrInDevList(ttDeviceEntryPtr   devPtr,
                                         tt6IpAddressPtr    tempAddr);
#endif /* (TM_USE_IPV6 && TM_6_USE_DAD) */
#ifdef __cplusplus
}
#endif

#endif /* _TRPROTO_H_ */
