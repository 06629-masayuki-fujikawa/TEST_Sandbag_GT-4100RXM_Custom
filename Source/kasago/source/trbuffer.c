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
 * Description: Heap and Buffer Allocation
 *
 * Filename: trbuffer.c
 * Author: Paul
 * Date Created: 11/10/97
 * $Source: source/trbuffer.c $
 *
 * Modification History
 * $Revision: 6.0.2.7 $
 * $Date: 2010/08/24 08:15:23JST $
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

#define TM_RAW_BUFFER_NULL_PTR      (ttRawBufferPtr)0

#ifdef TM_USE_DYNAMIC_MEMORY

#ifdef TM_USE_SHEAP
#ifdef TM_ERROR_CHECKING
/*
 * add room (2 longs == 8-bytes) to store the size of the allocated block
 * before and after
 */
#define TM_SHEAP_ALLOC_OVERHEAD 2 * sizeof(tt32Bit)
#else /* ! TM_ERROR_CHECKING */
/*
 * add room (1 long == 4-bytes) to store the size of the allocated block before.
 */
#define TM_SHEAP_ALLOC_OVERHEAD sizeof(tt32Bit)
#endif /* !TM_ERROR_CHECKING */
#else /* !TM_USE_SHEAP */
#define TM_SHEAP_ALLOC_OVERHEAD 0
#endif /* !TM_USE_SHEAP */

#define TM_MEM_BLOCK_ALLOC_OVERHEAD sizeof(tt32Bit)
#define TM_MEM_ALLOC_OVERHEAD   \
    TM_MEM_BLOCK_ALLOC_OVERHEAD + TM_SHEAP_ALLOC_OVERHEAD
#define TM_RECY_ALLOC_OVERHEAD  TM_SHEAP_ALLOC_OVERHEAD

/*
 * local macros. Only used for dynamic memory management.
 */

#define TM_RAW_MEMORY_NULL_PTR      (ttRawMemoryPtr)0
#define TM_RAW_MEMORY_NULL_PTRPTR   (ttRawMemoryPtrPtr)0

#define TM_MEM_SHIFT      7
/* (4096) >> TM_MEM_SHITF (we recycle buffers up to 4096 bytes) */
#define TM_MEM_LOOKUP_MAX  \
                ( (sizeof(tlMemoryIndexMap)/sizeof(tlMemoryIndexMap[0])) - 1 )

/*
 * local types. Only used for dynamic memory management.
 */
#ifdef TM_MULTIPLE_CONTEXT
struct tsMinMaxVal
{
    int minVal;
    int maxVal;
};
#endif /* TM_MULTIPLE_CONTEXT */

/*
 * Constant local variables.
 */
static const int  TM_CONST_QLF tlMemoryIndexMap[] =
{
/* 0 */ 2,
/* 1 */ 3,
/* 2 */ 4,
/* 3 */ 4,
/* 4 */ 5,
/* 5 */ 5,
/* 6 */ 5,
/* 7 */ 5,
/* 8 */ 6,
/* 9 */ 6,
/* 10 */6,
/* 11 */6,
/* 12 */6,
/* 13 */6,
/* 14 */6,
/* 15 */6,
/* 16 */7,
/* 17 */7,
/* 18 */7,
/* 19 */7,
/* 20 */7,
/* 21 */7,
/* 22 */7,
/* 23 */7,
/* 24 */7,
/* 25 */7,
/* 26 */7,
/* 27 */7,
/* 28 */7,
/* 29 */7,
/* 30 */7,
/* 31 */7,
/* Extra list for buffers which sizes are between 1, 32 */
/* 32 */0,
/* Extra list for buffers which sizes are between 33, 64 */
/* 33 */1
};

static const unsigned  TM_CONST_QLF tlBufQSize[] =
{
/* Extra list for buffers which sizes are between 1, 32 */
/* 0 */ TM_BUF_QX_SIZE,
/* Extra list for buffers which sizes are between 33, 64 */
/* 1 */ TM_BUF_Q0_SIZE,
/* 2 */ TM_BUF_Q1_SIZE,
/* 3 */ TM_BUF_Q2_SIZE,
/* 4 */ TM_BUF_Q3_SIZE,
/* 5 */ TM_BUF_Q4_SIZE,
/* 6 */ TM_BUF_Q5_SIZE,
/* 7 */ TM_BUF_Q6_SIZE
};

static const unsigned  TM_CONST_QLF tlRcyListSize[] =
{
/* TM_RECY_TIMER   0 */ /* Timers */
/* 0 */  sizeof(ttTimer),
/* TM_RECY_PACKET  1 */ /* ttPacket's */
/* 1 */ sizeof(ttPacket),
/* TM_RECY_RTE     2 */ /* routing entries */
/* 2 */ sizeof(ttRteEntry),
/* TM_RECY_SOCKET  3 */ /* non TCP sockets */
/* 3 */ sizeof(ttSocketEntry),
#ifdef TM_USE_TCP
/* TM_RECY_TCPVEC  4 */ /* TCP vectors/sockets */
/* 4 */ sizeof(ttTcpVect)
#endif /* TM_USE_TCP */
};
#endif /* TM_USE_DYNAMIC_MEMORY */

#ifdef TM_MULTIPLE_CONTEXT
/* Local functions */
#if defined(TM_USE_DYNAMIC_MEMORY) || defined(TM_USE_SHEAP)
static int tfFreeDynamicMemoryCB (
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam );
#endif /* TM_USE_DYNAMIC_MEMORY || TM_USE_SHEAP */

#ifdef TM_USE_DYNAMIC_MEMORY
static int tfFreeRangeDynamicMemoryCB (
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam );
#endif /* TM_USE_DYNAMIC_MEMORY */
#endif /* TM_MULTIPLE_CONTEXT */



/*
 * Allocate a shared data buffer with a packet to be used for sending
 * and receiving network data
 */
ttPacketPtr tfGetSharedBuffer(int hdrSize, ttPktLen dataSize, tt16Bit flag)
{
    ttPacketPtr     packetPtr;
    ttSharedDataPtr pktShrDataPtr;
    tt8BitPtr       pktDataPtr;
#ifndef TM_USER_PACKET_DATA_ALLOC
    tt32BitPtr      blockSizePtr; /* Pointer to size of allocated block */
#endif /* TM_USER_PACKET_DATA_ALLOC */
    ttPktLen        blockSize; /* Block size available to the user */
    ttPktLen        totalSize;
    ttPktLen        paddingSize;
    ttPktLen        trailerSize;

/* Get a packet */
    packetPtr = (ttPacketPtr)tm_packet_alloc(sizeof(ttPacket));
    if (packetPtr == TM_PACKET_NULL_PTR)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfGetSharedBuffer","No Kernel Memory");
#endif /* TM_ERROR_CHECKING */
        goto getSharedBufferFinish;
    }
/*
 * Need to call tfGetRawBuffer to get block size stored (for tfSendAppend(),
 * and TCP header append). If user replaces tfGetRawBuffer, then the
 * blockSize initialization below should be reviewed, and changed to
 * bufferSize + paddingSize, if user has no way to kow the allocated block size.
 *
 * This also allocates enough room for the packet trailer (by default, no
 * trailer is used)
 */
    paddingSize = 0; /* for minimum Ethernet length */
    trailerSize = 0; /* for IPSEC/device driver */
    if (!(flag & (TM_BUF_USER_DATA | TM_BUF_USER_DEVICE_DATA)))
/*
 * Not pointing to user buffer, i.e. data will actually be stored past the
 * ttSharedData structure.
 */
    {
        if ((hdrSize != 0) &&  (dataSize < (ttPktLen)TM_ETHER_IP_MIN_PACKET))
/*
 * Packet contains the header and size does not meet the minimum ethernet
 * requirements
 */
        {
/*
 * Add padding to avoid having to allocate an extra ttPacket/ttSharedData
 * later on.
 */
            paddingSize = TM_ETHER_IP_MIN_PACKET - dataSize;
        }
        trailerSize =   tm_context(tvPakPktSendTrailerSize)
                      + TM_PAK_IP_DEF_MAX_TRAILER_LEN;
    }

/* Allow room for headers */
    totalSize =    (ttPktLen)tm_packed_byte_count(hdrSize)
                +  (ttPktLen)tm_packed_byte_count(dataSize)
                +  paddingSize
                +  trailerSize;
/* Add room for Shared data structure + alignment ahead of data */
    totalSize =   tm_cache_align_roundup(totalSize)
                + sizeof(ttSharedData)
                + (TM_PACKET_BUF_CACHE_ALIGN - 1);

#ifdef TM_USER_PACKET_DATA_ALLOC
/* the user has specified their own allocation function for packet data
   buffers, we must protect the allocation with a lock because the allocation
   function (i.e. malloc) may not be reentrant */
    tm_lock_wait(&tm_global(tvPacketDataAllocLock));

    pktShrDataPtr = (ttSharedDataPtr)tm_alloc_packet_data_buffer(totalSize);

    tm_unlock(&tm_global(tvPacketDataAllocLock));
#else /* !TM_USER_PACKET_DATA_ALLOC */
    pktShrDataPtr = (ttSharedDataPtr)tm_get_raw_buffer(totalSize);
#endif /* !TM_USER_PACKET_DATA_ALLOC */

    if (pktShrDataPtr == TM_SHARED_DATA_NULL_PTR)
    {
        tm_recycle_free(packetPtr, TM_RECY_PACKET);
        packetPtr = TM_PACKET_NULL_PTR;
    }
    else
    {
/* Initialize shared data structure */
        tm_bzero(pktShrDataPtr, sizeof(ttSharedData));
        pktShrDataPtr->dataOwnerCount = 1;
        pktShrDataPtr->dataFlags = flag;
        pktShrDataPtr->dataUserDataLength = dataSize;
/*
 * Block size. If user does not use tfGetRawBuffer() then should replace this
 * initialization with blockSize = bufferSize + paddingSize. This would
 * disable attempt to append to this buffer though.
 */
#ifdef TM_USER_PACKET_DATA_ALLOC
        blockSize = totalSize;
#else /* TM_USER_PACKET_DATA_ALLOC */
        blockSizePtr = (tt32BitPtr)(ttVoidPtr)pktShrDataPtr;
/* tfGetRawBuffer() stores allocation size just before the beginning of data */
        blockSize = (ttPktLen)(*(blockSizePtr-1));
#endif /* !TM_USER_PACKET_DATA_ALLOC */
/* Point to end of data */
        pktDataPtr = (tt8BitPtr)pktShrDataPtr + blockSize;
        pktShrDataPtr->dataBufLastPtr = (tt8BitPtr)
                (  tm_cache_align_rounddown_ptr(pktDataPtr)
/* Reserved room for the trailer: */
                 - trailerSize);
/* Point to beginning of data */
        pktDataPtr = (tt8BitPtr)pktShrDataPtr + sizeof(ttSharedData);
        pktShrDataPtr->dataBufFirstPtr =
                                tm_cache_align_roundup_ptr(pktDataPtr);
#ifdef TM_USE_IPV6
/* Initialize empty ancillary list */
        tfListInit(&pktShrDataPtr->data6AncHdrList);
#endif /* TM_USE_IPV6 */
/*
 * packetPtr points to shared data, and is used for queuing data.
 * Initialize ttPacket fields.
 */
        tm_bzero(packetPtr, sizeof(ttPacket));
        packetPtr->pktSharedDataPtr = pktShrDataPtr;
        packetPtr->pktLinkDataPtr =   pktShrDataPtr->dataBufFirstPtr
                                    + (unsigned)tm_packed_byte_count(hdrSize);
#ifdef TM_DSP
        packetPtr->pktLinkDataByteOffset = 0;
#endif /* TM_DSP */
        packetPtr->pktLinkDataLength = dataSize;
        packetPtr->pktChainDataLength = dataSize;
    }
getSharedBufferFinish:
    return(packetPtr);
}

#if (  defined(TM_USE_DRV_SCAT_RECV) || defined(TM_USE_IPV6) \
    || defined(TM_USE_FILTERING_CONTIGUOUS_DATA) \
    || defined(TM_USE_IGMP_FILTERING) )
/*
 * tfContiguousSharedBuffer function description.
 *
 * Function called if macro tm_pkt_hdr_is_not_cont() is true, i.e. if
 * packetPtr->pktLinkExtraCount > 0 && packetPtr->pktLinkDataLength < length.
 *
 * Used to make data contiguous starting at packetPtr->pktLinkDataPtr for the
 * next length bytes on a scattered buffer.
 *
 * packetPtr always points to the beginning of a scattered buffer.
 * length bytes data straddles between the end of the first packetPtr, and
 * the next ones in the link.
 *
 * Returns pointer to same packetPtr, if there was room to copy the data at
 * the end of the first link.
 * Returns pointer to the same packetPtr, that points to a newly allocated
 * shared data buffer, if there was not enough room and a new
 * link had to be allocated to replace the first link. In that case we
 * swap the ttPacket structures, to keep the original ttPacket.
 *
 * Parameters:
 *  Value          Meaning
 *   packetPtr      pointer to the beginning of a scattered buffer.
 *   length         number of bytes that need to be contiguous starting
 *                  at packetPtr->pktLinkDataPtr.
 *
 * Returns:
 *  Value                  Meaning
 *   non null packetPtr     successful.
 *   null packetPtr         Failed to allocate a new packetPtr.
 *
 */
ttPacketPtr tfContiguousSharedBuffer( ttPacketPtr packetPtr,
                                      int         length )
{
    ttPacketPtr         swapPacketPtr;
    ttSharedDataPtr     oldSharedDataPtr;
    ttSharedDataPtr     newSharedDataPtr;
#ifdef TM_USE_IPV6
    tt8BitPtr           newLinkDataPtr;
#endif /* TM_USE_IPV6 */
    ttPacketPtr         freePacketPtr;
    ttPacketPtr         nextPacketPtr;
    ttPacketPtr         savNextPacketPtr;
    tt8BitPtr           copyDataPtr;
    ttPktLen            allocExtraBytes;
    ttPktLen            bufferSize;
    ttPktLen            appendBytes;
    ttPktLen            copyBytes;
    ttPktLen            allocHeaderBytes;
    ttPktLen            packedCopyBytes;
#ifdef TM_DSP
    ttPktLen            copyOffset;
    ttPktLen            dataOffset;
#endif /* TM_DSP */
    tt8Bit              flag;

    tm_assert( tfContiguousSharedBuffer,
               (    (packetPtr != (ttPacketPtr)0
                 && (length != 0) ) ) );

    tm_assert( tfContiguousSharedBuffer,
                   ( packetPtr->pktLinkExtraCount > 0)
               &&  ( packetPtr->pktLinkDataLength < (ttPktLen)length) );

    oldSharedDataPtr = (ttSharedDataPtr)0;
    newSharedDataPtr = (ttSharedDataPtr)0;
#ifdef TM_USE_IPV6
    newLinkDataPtr = (tt8BitPtr)0;
#endif /* TM_USE_IPV6 */
    copyDataPtr = (tt8BitPtr)0;

    if ((ttPktLen)length > packetPtr->pktChainDataLength)
/*  Make sure length is within the scattered buffer */
    {
        length = (int)packetPtr->pktChainDataLength;
    }
    appendBytes = (ttPktLen)length - packetPtr->pktLinkDataLength;
    tm_assert( tfContiguousSharedBuffer,
               (   packetPtr->pktSharedDataPtr->dataBufLastPtr
                 > packetPtr->pktLinkDataPtr ) );
    bufferSize = (ttPktLen)
        (tm_byte_count( packetPtr->pktSharedDataPtr->dataBufLastPtr
                        - packetPtr->pktLinkDataPtr )
#ifdef TM_DSP
         - packetPtr->pktLinkDataByteOffset
#endif /* TM_DSP */
         );
/* Temporary assert */
    tm_assert(tfContiguousSharedBuffer,
              ( bufferSize >= packetPtr->pktLinkDataLength ) );
/* No need to free by default. */
    freePacketPtr = (ttPacketPtr)0;
#ifdef TM_LOCK_NEEDED
    if (tm_16bit_one_bit_set(packetPtr->pktUserFlags,
                             TM_PKTF_USER_SOCKET_LOCKED))
    {
        flag = TM_SOCKET_LOCKED;
    }
    else
#endif /* TM_LOCK_NEEDED */
    {
        flag = TM_SOCKET_UNLOCKED;
    }
/* Next piece of the scattered data. */
    nextPacketPtr = (ttPacketPtr)packetPtr->pktLinkNextPtr;
#ifndef TESTING
    if (bufferSize < (ttPktLen)length)
#endif /* TESTING */
    {
/*
 * We do not have enough room to make that data contiguous. We have no choice
 * but to allocate a new packet, and copy the data from the first link, and
 * then free it.
 */
        packedCopyBytes = (ttPktLen)(ttUserPtrCastToInt)
            (packetPtr->pktLinkDataPtr
             - packetPtr->pktSharedDataPtr->dataBufFirstPtr);
        copyBytes = tm_byte_count(packedCopyBytes);
#ifndef TM_DSP
        if (    (packetPtr->pktSharedDataPtr->dataFlags
                                & TM_BUF_USER_DEVICE_DATA)
             && ( (  ((ttUserPtrCastToInt)packetPtr->pktLinkDataPtr)
                   & (ttUserPtrCastToInt)(TM_ROUND_PTR)) == 0 )
             && ( (copyBytes & TM_ROUND_SIZE) != 0 )
           )
        {
/* If original pointer is 4-byte aligned, so should the new one */
            allocHeaderBytes =
            (ttPktLen)( (copyBytes + 
                        (unsigned)TM_ROUND_SIZE) & TM_ROUND_MASK_SIZE );
        }
        else
#endif /* !TM_DSP */
        {
            allocHeaderBytes = copyBytes;
        }
        allocExtraBytes = (ttPktLen)length
#ifdef TM_DSP
/* We only computed the header(s) size up to the word boundary */
                          + packetPtr->pktLinkDataByteOffset
#endif /* TM_DSP */
                            ;
        if (allocExtraBytes < TM_RECV_SCAT_MIN_INCR_BUF)
        {
            allocExtraBytes = TM_RECV_SCAT_MIN_INCR_BUF;
        }
        swapPacketPtr = tfGetSharedBuffer( (int)allocHeaderBytes,
                                           allocExtraBytes,
                                           TM_16BIT_ZERO );
        if (swapPacketPtr == (ttPacketPtr)0)
/* We failed to allocate */
        {
/* Terminate the loop below */
            appendBytes = 0;
/* Free the packet.  */
            freePacketPtr = packetPtr;
/* Return NULL */
            packetPtr = (ttPacketPtr)0;
        }
        else
        {
            tm_16bit_clr_bit( packetPtr->pktSharedDataPtr->dataFlags,
                              TM_BUF_TCP_HDR_BLOCK );
            copyDataPtr = swapPacketPtr->pktSharedDataPtr->dataBufFirstPtr;
#ifndef TM_DSP
            if (allocHeaderBytes != copyBytes)
            {
                copyDataPtr += (allocHeaderBytes - copyBytes);
            }
#endif /* !TM_DSP */
/* Copy the data from the too short shared buffer to the newly allocated one */
            tm_bcopy( packetPtr->pktSharedDataPtr->dataBufFirstPtr,
                      copyDataPtr,
                        packedCopyBytes
/* For DSP will copy the potential extra bytes not counted in packedCopyBytes */
                      + tm_packed_byte_count(packetPtr->pktLinkDataLength) );
/* Copy ttSharedData fields from the old shared buffer to the new one */
            newSharedDataPtr = swapPacketPtr->pktSharedDataPtr;
            oldSharedDataPtr = packetPtr->pktSharedDataPtr;
            newSharedDataPtr->dataPeerPortNumber =
                oldSharedDataPtr->dataPeerPortNumber;
            newSharedDataPtr->dataLocalPortNumber =
                oldSharedDataPtr->dataLocalPortNumber;
            tm_ip_copy(oldSharedDataPtr->dataSrcIpAddress,
                       newSharedDataPtr ->dataSrcIpAddress);
#ifdef TM_USE_IPV6
            if (oldSharedDataPtr->data6PhysAddrLength != 0)
            {
                newSharedDataPtr->data6PhysAddrLength =
                                          oldSharedDataPtr->data6PhysAddrLength;
                tm_bcopy(oldSharedDataPtr->data6PhysAddr,
                         newSharedDataPtr->data6PhysAddr,
                         oldSharedDataPtr->data6PhysAddrLength);
            }
            newLinkDataPtr = copyDataPtr + packedCopyBytes;
            tm_6_ip_copy_structs( oldSharedDataPtr->dataSpecDestIpAddress,
                                  newSharedDataPtr->dataSpecDestIpAddress);
/* Update pointer to IPv6 header to point to new shared data IPv6 header*/
            if (packetPtr->pkt6RxIphPtr != (tt6IpHeaderPtr)0)
            {
                swapPacketPtr->pkt6RxIphPtr = (tt6IpHeaderPtr)
                 ((tt8BitPtr)(   newLinkDataPtr
                               - (   packetPtr->pktLinkDataPtr
                                   - ((tt8BitPtr)packetPtr->pkt6RxIphPtr)) ));
            }
/* update linkDataPtr in the new ttSharedData */
            packetPtr->pktLinkDataPtr = newLinkDataPtr;
#else /* !TM_USE_IPV6 */
            tm_ether_copy(oldSharedDataPtr->dataEthernetAddress,
                          newSharedDataPtr->dataEthernetAddress);
            tm_ip_copy( oldSharedDataPtr->dataSpecDestIpAddress,
                        newSharedDataPtr->dataSpecDestIpAddress );
/* update linkDataPtr in the new ttSharedData */
            packetPtr->pktLinkDataPtr =  copyDataPtr
                                       + packedCopyBytes;
#endif /* TM_USE_IPV6 */
/* swap pktSharedDataPtr */
            swapPacketPtr->pktSharedDataPtr = oldSharedDataPtr;
            packetPtr->pktSharedDataPtr = newSharedDataPtr;
/*
 * Free the swapped first link since we were able to allocate a new link.
 */
            freePacketPtr = swapPacketPtr;
        }
    }
/*
 * We have enough room at the end of the first ttSharedData, copy
 * from second link to first link the necessary number of bytes
 */
    while (appendBytes != 0)
    {
        tm_assert( tfContiguousSharedBuffer,
                   nextPacketPtr != (ttPacketPtr)0 );
        copyBytes = appendBytes;
        if (copyBytes > nextPacketPtr->pktLinkDataLength)
        {
            copyBytes = nextPacketPtr->pktLinkDataLength;
        }
#ifdef TM_DSP
        copyOffset =   packetPtr->pktLinkDataLength
                     + packetPtr->pktLinkDataByteOffset;
        tfMemCopyOffset((int *)nextPacketPtr->pktLinkDataPtr,
                        nextPacketPtr->pktLinkDataByteOffset,
                        (int *)(packetPtr->pktLinkDataPtr
                                + tm_packed_byte_len(copyOffset)),
                        copyOffset % TM_DSP_BYTES_PER_WORD,
                        copyBytes);
#else /* !TM_DSP */
        tm_bcopy( nextPacketPtr->pktLinkDataPtr,
                  packetPtr->pktLinkDataPtr + packetPtr->pktLinkDataLength,
                  (unsigned)copyBytes );
#endif /* !TM_DSP */
        packetPtr->pktLinkDataLength += copyBytes;
        appendBytes -= copyBytes;
        savNextPacketPtr = nextPacketPtr;
/* Point to next before freeing */
        nextPacketPtr = (ttPacketPtr)nextPacketPtr->pktLinkNextPtr;
        if (copyBytes < savNextPacketPtr->pktLinkDataLength)
/* Update pointer, and length in next link if we did a partial copy */
        {
#ifdef TM_DSP
            dataOffset = (unsigned int)
                       (savNextPacketPtr->pktLinkDataByteOffset + copyBytes);
            savNextPacketPtr->pktLinkDataPtr +=
                                    tm_packed_byte_len(dataOffset);
            savNextPacketPtr->pktLinkDataByteOffset =
                                    ((int)dataOffset) % TM_DSP_BYTES_PER_WORD;
#else /* !TM_DSP */
            savNextPacketPtr->pktLinkDataPtr += copyBytes;
#endif /* TM_DSP */
            savNextPacketPtr->pktLinkDataLength -= copyBytes;
        }
        else
        {
/* Free the next link, if we copied the whole next link */
            packetPtr->pktLinkNextPtr = (ttVoidPtr)nextPacketPtr;
            packetPtr->pktLinkExtraCount--;
            tfFreeSharedBuffer(savNextPacketPtr, flag);
        }
    }
    if (freePacketPtr != (ttPacketPtr)0)
    {
        tfFreePacket(freePacketPtr, flag);
    }
#ifdef TM_ERROR_CHECKING
    if (packetPtr != (ttPacketPtr)0)
    {
        tm_assert(tfContiguousSharedBuffer,
                  packetPtr->pktLinkExtraCount >= 0);
    }
#endif /* TM_ERROR_CHECKING */
    return packetPtr;
}
#endif /*    TM_USE_DRV_SCAT_RECV || TM_USE_IPV6
          || TM_USE_FILTERING_CONTIGUOUS_DATA
          || TM_USE_IGMP_FILTERING */

/*
 * Duplicate the packet/shared data Pair
 */
ttPacketPtr tfDuplicateSharedBuffer(ttPacketPtr packetPtr)
{
    ttPacketPtr     newPacketPtr;

#ifdef TM_ERROR_CHECKING
    if (packetPtr == TM_PACKET_NULL_PTR)
    {
        tfKernelError("tfDuplicateSharedBuffer", "Null packet pointer");
        newPacketPtr = TM_PACKET_NULL_PTR;
        goto duplicateSharedBufferFinish;
    }
#endif /* TM_ERROR_CHECKING */

/* Get a packet from the packet queue or the kernel */
    newPacketPtr = (ttPacketPtr)tm_packet_alloc(sizeof(ttPacket));
    if (newPacketPtr == TM_PACKET_NULL_PTR)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfDuplicateSharedBuffer","No Kernel Memory");
#endif /* TM_ERROR_CHECKING */
        goto duplicateSharedBufferFinish;
    }
/*
 * Increment the Owner Count for this shared data block
 * And do it in a critical section
 */
    tm_incr_owner_count(packetPtr->pktSharedDataPtr->dataOwnerCount);
/* Copy the packet pointing to the shared buffer */
    tm_bcopy(packetPtr, newPacketPtr, sizeof(ttPacket));
/* newPacketPtr pktChainNextPtr, linkNextPtr, and linkExtraCount are zero */
    newPacketPtr->pktChainNextPtr = TM_PACKET_NULL_PTR;
    newPacketPtr->pktLinkNextPtr = (ttVoidPtr)0;
    newPacketPtr->pktLinkExtraCount = 0;
/* Indicate that this is a duplicated packet */
    newPacketPtr->pktUserFlags = (tt16Bit) (   newPacketPtr->pktUserFlags
                                             | TM_PKTF_USER_DUPLICATED);
/* Indicate that the original packet has been duplicated */
    packetPtr->pktUserFlags = (tt16Bit) (   packetPtr->pktUserFlags
                                          | TM_PKTF_USER_DUPLICATED);
#ifdef TM_USE_IPSEC
#ifdef TM_USE_IKE
/*  newPacketPtr  pktIkeQueuedTimerPtr  is zero */
    newPacketPtr->pktIkeQueuedTimerPtr = (ttTimerPtr)0;
#endif /* TM_USE_IKE */
    packetPtr->pktSalistPtr = (ttVoidPtr)0;
    packetPtr->pktSelectorPtr = (ttVoidPtr)0;
#endif /* TM_USE_IPSEC */
duplicateSharedBufferFinish:
    return(newPacketPtr);
}

/*
 * Duplicate the whole chain of packets
 * and increment the owner count of the shared data
 * that they point to
 */
ttPacketPtr tfDuplicatePacket(ttPacketPtr packetPtr)
{
    ttPacketPtr prevNewPacketPtr;
    ttPacketPtr retPacketPtr;
    ttPacketPtr newPacketPtr;

/* Initialize prev and ret packet pointers to NULL */
    prevNewPacketPtr = TM_PACKET_NULL_PTR;
    retPacketPtr = TM_PACKET_NULL_PTR;
    while (packetPtr != TM_PACKET_NULL_PTR)
    {
/* Duplicate a packet */
        newPacketPtr = tfDuplicateSharedBuffer(packetPtr);
        if (newPacketPtr == TM_PACKET_NULL_PTR)
        {
/* No memory to complete the duplicate, so free the chains we allocated */
/* Loop and put back all the packets created thus far */
            while (retPacketPtr != TM_PACKET_NULL_PTR)
            {
                newPacketPtr = (ttPacketPtr)retPacketPtr->pktLinkNextPtr;
                tfFreeSharedBuffer(retPacketPtr, TM_SOCKET_UNLOCKED);
                retPacketPtr = newPacketPtr;
            }
            goto duplicatePacketFinish;
        }
        if (retPacketPtr == TM_PACKET_NULL_PTR)
        {
            retPacketPtr = newPacketPtr;
        }
        else
        {
            retPacketPtr->pktLinkExtraCount++;
        }
        if (prevNewPacketPtr != TM_PACKET_NULL_PTR)
        {
            prevNewPacketPtr->pktLinkNextPtr = (ttVoidPtr)newPacketPtr;
        }
        prevNewPacketPtr = newPacketPtr;
        packetPtr = (ttPacketPtr)packetPtr->pktLinkNextPtr;
    }
/* Exit point for abnormal terminations */
duplicatePacketFinish:
    return (retPacketPtr);
}

/*
 * Free a packet/shared buffer combination
 * Only free the shared data when the owner count is 0
 */
void tfFreeSharedBuffer(ttPacketPtr packetPtr, tt8Bit lockFlag)
{
    tt8Bit           ownerCount;
    tt16Bit          dataFlags;
    ttSharedDataPtr  pktShrDataPtr;
    ttSocketEntryPtr socketEntryPtr;
    ttTcpVectPtr     tcpVectPtr;
    ttDeviceEntryPtr devEntryPtr;
    tt8Bit           recycled;

#ifdef TM_ERROR_CHECKING
    if (packetPtr == TM_PACKET_NULL_PTR)
    {
        tfKernelError("tfFreeSharedBuffer", "Null packet pointer");
    }
    else
#endif /* TM_ERROR_CHECKING */
    {
        recycled = TM_8BIT_ZERO;
        pktShrDataPtr = packetPtr->pktSharedDataPtr;
        dataFlags = pktShrDataPtr->dataFlags;
#ifdef TM_ERROR_CHECKING
        if (pktShrDataPtr->dataOwnerCount == 0)
        {
            tfKernelError("tfFreeSharedBuffer",
                          "Attempt to free more that alloc a buffer");
        }
#endif /* TM_ERROR_CHECKING */
        tm_decr_owner_count(ownerCount,pktShrDataPtr->dataOwnerCount);
        if (ownerCount == 0)
/* Free shared data area */
        {
#ifdef TM_USE_IPV6
/* free ancillary data, if there is any */
            tfListFree(&pktShrDataPtr->data6AncHdrList);
#endif /* TM_USE_IPV6 */

/*
 * Check to see if the user data block came from the user device.
 */
            if (dataFlags & TM_BUF_USER_DEVICE_DATA)
            {
/* It did so call the user to free it */
                devEntryPtr = pktShrDataPtr->dataDeviceEntryPtr;
/* The device should not be NULL but check anyway */
                if (devEntryPtr != TM_DEV_NULL_PTR)
                {
/*
 * If the user has passed us a buffer but not a free routine then it is an
 * error
 */
                    if (devEntryPtr->devFreeRecvFuncPtr !=
                                                    (ttDevFreeRecvFuncPtr)0)
                    {
/*
 * Make the user device driver free function re-entrant by setting the
 * device driver lock.
 */
                        tm_lock_wait(&(devEntryPtr->devDriverLockEntry));
/* User buffer data pointer saved in dataDeviceUserDataPtr */
                        (void)(*(devEntryPtr->devFreeRecvFuncPtr))(
                                    (ttUserInterface)devEntryPtr,
                                    pktShrDataPtr->dataDeviceUserDataPtr);
                        tm_unlock(&(devEntryPtr->devDriverLockEntry));
                    }
#ifdef TM_ERROR_CHECKING
                    else
                    {
                        tfKernelError("tfFreeSharedBuffer",
                                      "No Free Recv Routine");
                    }
#endif /* TM_ERROR_CHECKING */
                }
#ifdef TM_ERROR_CHECKING
                else
                {
                    tfKernelError("tfFreeSharedBuffer",
                      "User device data, but device Entry Not Initialized");
                }
#endif /* TM_ERROR_CHECKING */
            }
            socketEntryPtr = pktShrDataPtr->dataSocketEntryPtr;
/*
 * If this is a TCP recycle header, or a non TCP send buffer.
 */
            if ( dataFlags & (  TM_BUF_TCP_HDR_BLOCK
                              | TM_BUF_SOCKETENTRYPTR
                              | TM_BUF_USER_SEND) ) /* non TCP send buffer */
            {
#ifdef TM_ERROR_CHECKING
                if (socketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR)
                {
                    tfKernelError("tfFreeSharedBuffer",
                           "Null socketEntryPtr when it should be non null");
                }
                else
#endif /* TM_ERROR_CHECKING */
                {
/*
 * If this is a TCP recycle header, or a non TCP send buffer,
 * socketEntryPtr actually points to a socket entry. We need to lock the
 * socket if the user called us with the socket unlocked.
 */
                    if (lockFlag == TM_SOCKET_UNLOCKED)
                    {
/* Lock to update socket queue and then for posting and checkin */
                        tm_lock_wait(&(socketEntryPtr->socLockEntry));
                    }
                    tcpVectPtr = (ttTcpVectPtr)((ttVoidPtr)socketEntryPtr);
                    if (    (dataFlags & TM_BUF_TCP_HDR_BLOCK)
/* If it is a tcp recycle header */
                         && (tm_16bit_one_bit_set( tcpVectPtr->tcpsFlags,
                                                     TM_BUF_TCP_HDR_BLOCK))
/*
 * and if recycling is still allowed (i.e state vector is not closed or in
 * TIME WAIT)
 */
                        && (tcpVectPtr->tcpsRecycleHdrCount <
                                         tcpVectPtr->tcpsMaxRecycleHdrCount)
/* and we have not exceeded maximum number of allowable recycled headers */
                       )
                    {
/*
 * Returns a TCP Header back to the recycle queue for the tcp state vector.
 * Set the recycle flag.
 */
                        pktShrDataPtr->dataFlags |= TM_BUF_HDR_RECYCLED;
/* Put it at the head of the queue */
                        packetPtr->pktLinkNextPtr = (ttVoidPtr)
                                              tcpVectPtr->tcpsRecycleHdrPtr;
                        tcpVectPtr->tcpsRecycleHdrPtr = packetPtr;
                        tcpVectPtr->tcpsRecycleHdrCount = (tt16Bit)
                                        (tcpVectPtr->tcpsRecycleHdrCount + 1);
                        pktShrDataPtr->dataOwnerCount = 1;
                        recycled = TM_8BIT_YES;
                    }
                    else
                    {
/*
 * Non TCP send buffer (TM_BUF_USER_SEND), or TCP header that cannot
 * be recycled (because of ARP or VJ compression), or because the state
 * vector is closed or in time wait, or because we have exceeded our quota.
 */
                        if (dataFlags & TM_BUF_USER_SEND)
                        {
/* non TCP send buffer */
                            socketEntryPtr->socSendQueueBytes -=
                                            pktShrDataPtr->dataUserDataLength;
                            socketEntryPtr->socSendQueueDgrams--;
                            if (    (    socketEntryPtr->socMaxSendQueueBytes
                                      -  socketEntryPtr->socSendQueueBytes )
                                 >= socketEntryPtr->socLowSendQueueBytes )
                            {
                                tfSocketNotify( socketEntryPtr,
                                                TM_SELECT_WRITE,
                                                TM_CB_WRITE_READY,
                                                0 );
                            }
                        }
                        socketEntryPtr->socOwnerCount--;
                    }
/*
 * If the user had called us with the socket unlocked, we need to unlock it
 * now
 */
                    if (lockFlag == TM_SOCKET_UNLOCKED)
                    {
                        if (socketEntryPtr->socOwnerCount == 0)
                        {
                            tfSocketFree(socketEntryPtr);
                        }
                        else
                        {
                            tm_unlock(&socketEntryPtr->socLockEntry);
                        }
                    }
                }
            }
#ifdef TM_ERROR_CHECKING
            else
            {
                if (socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR)
                {
                    tfKernelError("tfFreeSharedBuffer",
                        "non null dataSocketEntryPtr when it should be null");
                }
            }
#endif /* TM_ERROR_CHECKING */
/* If the shared data pointer is not being recycled, free it now */
            if (recycled == TM_8BIT_ZERO)
            {
                if (dataFlags & TM_BUF_USER_DATA)
                {
#ifdef TM_ERROR_CHECKING
                    if (    pktShrDataPtr->dataUserFreeFuncPtr
                         == (ttUserFreeFuncPtr)0 )
                    {
                        tfKernelError("tfFreeSharedBuffer",
                   "Cannot free user data because the free Func Ptr is null");
                    }
                    else
#endif /* TM_ERROR_CHECKING */
                    {
                        (void)(*(pktShrDataPtr->dataUserFreeFuncPtr))(
                                       pktShrDataPtr->dataDeviceUserDataPtr );
                    }
                }
/*
 * Free buffer:
 * Need to call tfFreeRawBuffer to point to beginning of allocated memory
 * block.
 */
#ifdef TM_USER_PACKET_DATA_ALLOC
/* the user has specified their own allocation function for packet data
   buffers, we must protect the allocation with a lock because the allocation
   function (i.e. malloc) may not be reentrant */
                tm_lock_wait(&tm_global(tvPacketDataAllocLock));

                tm_free_packet_data_buffer((ttRawBufferPtr)(pktShrDataPtr));

                tm_unlock(&tm_global(tvPacketDataAllocLock));
#else /* !TM_USER_PACKET_DATA_ALLOC */
                tm_free_raw_buffer(pktShrDataPtr);
#endif /* !TM_USER_PACKET_DATA_ALLOC */
            }
        }
/* no matter recycle or not, we need to free the SA list */
#ifdef TM_USE_IPSEC
/*
 * tvIpsecPtr could be freed at run time if user calls tfIpsecUninitialize.
 * So even if tvIpsecPtr is null, we still need to free memory allocated while
 * IPSEC was running.
 */
        if (    (packetPtr->pktSelectorPtr != (ttVoidPtr)0)
             || (packetPtr->pktSalistPtr != (ttVoidPtr)0)
             || (packetPtr->pktSadbRecordPtr != (ttVoidPtr)0)
#ifdef TM_USE_IKE
             || (packetPtr->pktIkeQueuedTimerPtr != (ttTimerPtr)0)
#endif /*TM_USE_IKE*/
           )
        {
            tfIpsecSalistSelectorClear(packetPtr);
        }
#if (defined(TM_USE_IPV4) && defined(TM_USE_IPSEC_HANDOFF_TASK))
        packetPtr->pktHdfEntryPtr = (ttVoidPtr)0;
#endif /* TM_USE_IPV4 && TM_USE_IPSEC_HANDOFF_TASK */
#endif /* TM_USE_IPSEC */

#ifdef TM_USE_IP_REASSEMBLY_TASK
/* If using the packet reassembly task, free any reassembly data */
        if (packetPtr->pktIrlEntryPtr)
        {
            tm_free_raw_buffer(packetPtr->pktIrlEntryPtr);
        }
#endif /* TM_USE_IP_REASSEMBLY_TASK */

/* If the packet structure is not being recycled, free it now */
        if (recycled == TM_8BIT_ZERO)
        {
            tm_recycle_free(packetPtr, TM_RECY_PACKET);
        }
    }
    return;
}

/*
 * Walk the whole packet link
 * and free all the packet/shared buffers
 */
void tfFreePacket(ttPacketPtr packetPtr, tt8Bit lockFlag)
{
    ttPacketPtr nextPacketPtr;

    while (packetPtr != TM_PACKET_NULL_PTR)
    {
        nextPacketPtr = (ttPacketPtr)packetPtr->pktLinkNextPtr;
        tfFreeSharedBuffer(packetPtr, lockFlag);
        packetPtr = nextPacketPtr;
    }
}

/*
 * Walk the packet chain (queue of packets), and free all the packets
 */
void tfFreeChainPacket(ttPacketPtr packetPtr, tt8Bit lockFlag)
{
    ttPacketPtr nxtPacketPtr;

    while (packetPtr != TM_PACKET_NULL_PTR)
    {
         nxtPacketPtr = packetPtr->pktChainNextPtr;
         tfFreePacket(packetPtr, lockFlag);
         packetPtr = nxtPacketPtr;
    }
}

/*
 * Copy the whole packet link.
 */
void tfCopyPacket (ttPacketPtr srcPacketPtr, ttPacketPtr destPacketPtr)
{
    ttPktLen        copyOffset;

/* Copy Source IP address (IP address of the sender) (for recvfrom()) */
    tm_ip_copy(srcPacketPtr->pktSharedDataPtr->dataSrcIpAddress,
               destPacketPtr->pktSharedDataPtr->dataSrcIpAddress);
/* Copy Source port (IP port of the sender) (for recvfrom()) */
    destPacketPtr->pktSharedDataPtr->dataPeerPortNumber =
        srcPacketPtr->pktSharedDataPtr->dataPeerPortNumber;
    destPacketPtr->pktSharedDataPtr->dataLocalPortNumber =
        srcPacketPtr->pktSharedDataPtr->dataLocalPortNumber;
/* Copy Destination IP address (IP destination address ) (for tfRecvFromTo) */
    tm_ip_copy(srcPacketPtr->pktSharedDataPtr->dataDestIpAddress,
               destPacketPtr->pktSharedDataPtr->dataDestIpAddress);
/* Copy over the flags field */
    destPacketPtr->pktFlags |=
            (srcPacketPtr->pktFlags & (  TM_LL_UNICAST_FLAG
                                       | TM_LL_BROADCAST_FLAG
                                       | TM_LL_MULTICAST_FLAG
                                       | TM_IP_UNICAST_FLAG
                                       | TM_IP_BROADCAST_FLAG
                                       | TM_IP_MULTICAST_FLAG));

#ifdef TM_USE_IPV6
/* Copy Destination IP address (our IP address, for multihoming) (for RIP) */
    tm_6_ip_copy_structs(
        srcPacketPtr->pktSharedDataPtr->dataSpecDestIpAddress,
        destPacketPtr->pktSharedDataPtr->dataSpecDestIpAddress);
/* Copy pointer to IPv6 header */
    destPacketPtr->pkt6RxIphPtr = srcPacketPtr->pkt6RxIphPtr;
    if (destPacketPtr->pkt6RxIphPtr != (tt6IpHeaderPtr)0)
    {
/* Offset of pointer to IPv6 header in old shared data */
        copyOffset = (ttPktLen)(ttUserPtrCastToInt)
            (srcPacketPtr->pktLinkDataPtr
             - ((tt8BitPtr)srcPacketPtr->pkt6RxIphPtr));
/* Pointer to IPv6 header in new shared data at the correct offset */
        destPacketPtr->pkt6RxIphPtr = (tt6IpHeaderPtr)
                ((tt8BitPtr)(destPacketPtr->pktLinkDataPtr - copyOffset));
/* Copy header in new shared data */
        tm_bcopy(srcPacketPtr->pkt6RxIphPtr,
                 destPacketPtr->pkt6RxIphPtr,
                 TM_6PAK_IP_MIN_HDR_LEN);
    }
/* Copy pktNetworkLayer */
    destPacketPtr->pktNetworkLayer = srcPacketPtr->pktNetworkLayer;
#else /* ! TM_USE_IPV6 */
/* Copy Destination IP address (our IP address, for multihoming) (for RIP) */
    tm_ip_copy(srcPacketPtr->pktSharedDataPtr->dataSpecDestIpAddress,
               destPacketPtr->pktSharedDataPtr->dataSpecDestIpAddress);
#endif /* ! TM_USE_IPV6 */
/* copy Ethernet address (for tfZeroCopyRecvFromToInterface() */
    tm_ether_copy(srcPacketPtr->pktEthernetAddress,
                  destPacketPtr->pktEthernetAddress);
/* Copy interface packet came from (for BOOTP/DHCP/RIP) */
    destPacketPtr->pktDeviceEntryPtr = srcPacketPtr->pktDeviceEntryPtr;
#ifndef TM_LOOP_TO_DRIVER
    destPacketPtr->pktOrigDevPtr = srcPacketPtr->pktOrigDevPtr;
#endif /* !TM_LOOP_TO_DRIVER */
#ifdef TM_DSP
    copyOffset = destPacketPtr->pktLinkDataByteOffset;
#else /* !TM_DSP */
    copyOffset = (ttPktLen)0;
#endif /* !TM_DSP */
    do
    {
/* Copy this data area to the user buffer */
#ifdef TM_DSP
        tfMemCopyOffset((int *) srcPacketPtr->pktLinkDataPtr,
                srcPacketPtr->pktLinkDataByteOffset,
                (int*) (destPacketPtr->pktLinkDataPtr
                        + tm_packed_byte_len(copyOffset)),
                copyOffset % TM_DSP_BYTES_PER_WORD,
                srcPacketPtr->pktLinkDataLength);
#else /* !TM_DSP */
        tm_bcopy(srcPacketPtr->pktLinkDataPtr,
                 destPacketPtr->pktLinkDataPtr + copyOffset,
                 srcPacketPtr->pktLinkDataLength);
#endif /* !TM_DSP */
        copyOffset += srcPacketPtr->pktLinkDataLength;
        srcPacketPtr = (ttPacketPtr)srcPacketPtr->pktLinkNextPtr;
    } while (srcPacketPtr != TM_PACKET_NULL_PTR);
}

#ifdef TM_ERROR_CHECKING
#ifdef TM_USE_ANSI_LINE_FILE
#if defined(TM_KERNEL_VISUAL_X86) || defined(TM_KERNEL_WIN32_X86)
/*
 * Local dynamic memory management variables initialization.
 * Only needed for dynamic memory management.
 */
void tfMemoryLeakInit(void)
{
#ifdef TM_MULTIPLE_CONTEXT
    char   fileNameString[24];
#endif /* TM_MULTIPLE_CONTEXT */

#ifdef TM_MULTIPLE_CONTEXT
    tm_bcopy("memoryleak", fileNameString, 10);
    tfUlongDecimalToString(&fileNameString[10],
                           tm_context(tvContextId));
    tm_strcat(fileNameString, ".dat");
    tm_context(tvMemoryLeakDetector) = fopen(fileNameString, "wb");
#else /* !TM_MULTIPLE_CONTEXT */
    tm_context(tvMemoryLeakDetector) = fopen("memoryleak.dat", "wb");
#endif /* !TM_MULTIPLE_CONTEXT */
    tm_assert(tfMemoryLeakInit, tm_context(tvMemoryLeakDetector) != (FILE*)0);
    return;
}
#endif /* defined(TM_KERNEL_VISUAL_X86) || defined(TM_KERNEL_WIN32_X86) */
#endif /*  TM_USE_ANSI_LINE_FILE */
#endif /* TM_ERROR_CHECKING */

#ifdef TM_USE_DYNAMIC_MEMORY
/* Given an allocation size, determine the size of the memory queue
   that it fits in. */
ttPktLen tfGetMemoryQueueSize(ttPktLen size)
{
    ttPktLen                lookupIndex;

/*
 * Round up to the nearest Round Size byte boundary
 */
#ifndef TM_DSP
    size = (ttPktLen)( (size + (unsigned)TM_ROUND_SIZE) & TM_ROUND_MASK_SIZE );
#endif /* TM_DSP */

/* Determine which memory queue to get the block from */
    lookupIndex = (ttPktLen)( (size - 1) >> TM_MEM_SHIFT );
    if (lookupIndex < TM_MEM_LOOKUP_MAX - 1)
    {
        if (size <= TM_BUF_Q0_SIZE)
        {
/* Look at the end of the lookup table */
            if (size <= TM_BUF_QX_SIZE)
            {
                lookupIndex = TM_MEM_LOOKUP_MAX - 1; /* 32 */
            }
            else
            {
                lookupIndex = TM_MEM_LOOKUP_MAX; /* 33 */
            }
        }
        lookupIndex = (ttPktLen)tlMemoryIndexMap[lookupIndex];
/* size within block sizes that we manage in our queues */
        size = (ttPktLen)tlBufQSize[lookupIndex];
    }
    return size;
}
#endif /* TM_USE_DYNAMIC_MEMORY */

/*
 * Allocate a raw buffer from the queues or the Kernel
 * Only needed for dynamic memory management.
 */
#if (       defined(TM_ERROR_CHECKING)      \
         && defined(TM_USE_ANSI_LINE_FILE)  \
         && (defined(TM_KERNEL_VISUAL_X86) || defined(TM_KERNEL_WIN32_X86) \
             || defined(TM_MEMORY_LOGGING)))
ttRawBufferPtr tfGetRawBuffer(
    ttPktLen size, char TM_FAR * filename, int lineNumber)
#else
ttRawBufferPtr tfGetRawBuffer(ttPktLen size)
#endif
{
    ttRawBufferPtr          memoryBlockPtr;

#ifdef TM_USE_DYNAMIC_MEMORY
    ttRawMemoryPtrPtr       memoryQueuePtrPtr;
#ifdef TM_LOCK_NEEDED
    ttLockEntryPtr          lockEntryPtr;
#endif /* TM_LOCK_NEEDED */
    ttPktLen                lookupIndex;
#endif /* TM_USE_DYNAMIC_MEMORY */

/* Zero out our return pointer */
    memoryBlockPtr = TM_RAW_BUFFER_NULL_PTR;

    if (size == 0)
    {
        goto getRawBufferFinish;
    }

/*
 * Round up to the nearest Round Size byte boundary
 */
#ifndef TM_DSP
    size = (ttPktLen)( (size + (unsigned)TM_ROUND_SIZE) & TM_ROUND_MASK_SIZE );
#endif /* TM_DSP */

#ifdef TM_USE_DYNAMIC_MEMORY
/* Determine which memory queue to get the block from */
    lookupIndex = (ttPktLen)( (size - 1) >> TM_MEM_SHIFT );
    if (lookupIndex < TM_MEM_LOOKUP_MAX - 1)
    {
        if (size <= TM_BUF_Q0_SIZE)
        {
/* Look at the end of the lookup table */
            if (size <= TM_BUF_QX_SIZE)
            {
                lookupIndex = TM_MEM_LOOKUP_MAX - 1; /* 32 */
            }
            else
            {
                lookupIndex = TM_MEM_LOOKUP_MAX; /* 33 */
            }
        }
        lookupIndex = (ttPktLen)tlMemoryIndexMap[lookupIndex];
/* size within block sizes that we manage in our queues */
        size = (ttPktLen)tlBufQSize[lookupIndex];
        memoryQueuePtrPtr = &(tm_context(tvBufQListPtr[lookupIndex]));
        tm_lock_entry_init(lockEntryPtr, &tm_context(tvDynMemLockEntry));
/* LOCK the queue */
        tm_lock_wait(lockEntryPtr);
/* Check to see if we have anything on this queue */
        if (*memoryQueuePtrPtr != TM_RAW_MEMORY_NULL_PTR)
        {
            memoryBlockPtr = (ttRawBufferPtr)(*memoryQueuePtrPtr);
            *memoryQueuePtrPtr = (*(memoryQueuePtrPtr))->rawNextPtr;
            tm_context(tvMemRecycledBytes) -= size + TM_MEM_ALLOC_OVERHEAD;
            ((ttRawMemoryPtr)memoryBlockPtr)->rawNextPtr =
                                        TM_RAW_MEMORY_NULL_PTR;
/*
 * Make sure that memory is not corrupted. Buffer should be preceded by
 * its size.
 */
            tm_assert(tfGetRawBuffer, *(((tt32BitPtr)memoryBlockPtr) - 1)
                                      == size);
        }
/* UNLOCK the queue */
        tm_unlock(lockEntryPtr);
    }
    else
    {
/* Bigger than we keep track of so get it from the kernel/OS */
        memoryQueuePtrPtr = TM_RAW_MEMORY_NULL_PTRPTR;
    }
/* Did we get memory from the queue? */
    if (memoryBlockPtr == TM_RAW_BUFFER_NULL_PTR)
#endif /* TM_USE_DYNAMIC_MEMORY */
    {
/* Get it from the kernel */
        memoryBlockPtr = (ttRawBufferPtr)tm_kernel_malloc( (unsigned int)
                                               ( size + sizeof(tt32Bit) ) );
        if (memoryBlockPtr == TM_RAW_BUFFER_NULL_PTR)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfGetRawBuffer","No Kernel Memory");
#endif /* TM_ERROR_CHECKING */
            goto getRawBufferFinish;
        }
        else
        {

#ifndef TM_DSP
#ifdef TM_ERROR_CHECKING
/* If pointer is not on a 4 bytes boundary */
            if (((ttUserPtrCastToInt) memoryBlockPtr)
                & (ttUserPtrCastToInt)(TM_ROUND_PTR))
            {
                tfKernelWarning("tfGetRawBuffer",
            "tm_kernel_malloc did not return a pointer on a 4-byte boundary");
            }
#endif /* TM_ERROR_CHECKING */
#endif /* TM_DSP */

/*
 * Put the size in the first 32 bits of the memory block
 * and move the pointer
 */
            *((tt32BitPtr)memoryBlockPtr) = (tt32Bit)size;
            memoryBlockPtr = (tt8BitPtr)memoryBlockPtr + sizeof(tt32Bit);
        }
    }
getRawBufferFinish:
#ifdef TM_ERROR_CHECKING
    if (memoryBlockPtr != TM_RAW_BUFFER_NULL_PTR)
    {
        tm_kernel_set_critical;
        (tm_context(tvGetRawCount))++;
        tm_kernel_release_critical;
#ifdef TM_USE_ANSI_LINE_FILE
#if defined(TM_KERNEL_VISUAL_X86) || defined(TM_KERNEL_WIN32_X86)
        tm_call_lock_wait(&tm_context(tvMemoryLeakLock));
        fprintf(tm_context(tvMemoryLeakDetector),
                "GetBuffer, %d, %ld, 0x%p, %d, %s, line, %d\n",
                tm_context(tvGetRawCount), tvTime, memoryBlockPtr, size,
                filename, lineNumber);
        fflush(tm_context(tvMemoryLeakDetector));
        tm_call_unlock(&tm_context(tvMemoryLeakLock));
#else /* !(defined(TM_KERNEL_VISUAL_X86) || defined(TM_KERNEL_WIN32_X86)) */
#ifdef TM_MEMORY_LOGGING
/* NOTE: when using TM_MEMORY_LOGGING, avoid doing any operation
   that requires Treck memory allocation when walking the log buffer. For
   example, have your log walk function call the RTOS printf() function to
   output log messages, since that does not use any Treck memory. */
        tm_call_lock_wait(&tm_context(tvMemoryLeakLock));
        tm_memory_log6(
            "GetBuffer, %d, %ld, 0x%p, %d, %s, line, %d\n",
            tm_context(tvGetRawCount), tvTime, memoryBlockPtr, size,
            filename, lineNumber);
        tm_call_unlock(&tm_context(tvMemoryLeakLock));
#endif /* TM_MEMORY_LOGGING */
#endif /* !(defined(TM_KERNEL_VISUAL_X86) || defined(TM_KERNEL_WIN32_X86)) */
#endif /*  TM_USE_ANSI_LINE_FILE */
    }
#endif /* TM_ERROR_CHECKING */
    return memoryBlockPtr;
}

/*
 * Free a buffer back to the queues or the kernel
 * Only needed for dynamic memory management.
 */
void tfFreeRawBuffer (ttRawBufferPtr memoryBlockPtr)
{
    tt32BitPtr           sizePtr;
#ifdef TM_ERROR_CHECKING
#if (defined(TM_KERNEL_VISUAL_X86) || defined(TM_KERNEL_WIN32_X86) \
     || defined(TM_MEMORY_LOGGING))
    int                  size;
#endif /* Win32 or TM_MEMORY_LOGGING */
#endif /* TM_ERROR_CHECKING */
#ifdef TM_USE_DYNAMIC_MEMORY
    ttRawMemoryPtrPtr    memoryQueuePtrPtr;
#ifdef TM_LOCK_NEEDED
    ttLockEntryPtr       lockEntryPtr;
#endif /* TM_LOCK_NEEDED */
    int                  index;
#endif /* TM_USE_DYNAMIC_MEMORY */

    sizePtr = (tt32BitPtr)memoryBlockPtr;
    sizePtr--;
#ifdef TM_ERROR_CHECKING
#if (defined(TM_KERNEL_VISUAL_X86) || defined(TM_KERNEL_WIN32_X86) \
     || defined(TM_MEMORY_LOGGING))
    size = (int)(*sizePtr);
#endif /* Win32 or TM_MEMORY_LOGGING */
#endif /* TM_ERROR_CHECKING */

#ifdef TM_USE_DYNAMIC_MEMORY

    switch (*sizePtr)
    {
        case TM_BUF_QX_SIZE:
            index = 0;
            break;
        case TM_BUF_Q0_SIZE:
            index = 1;
            break;
        case TM_BUF_Q1_SIZE:
            index = 2;
            break;
        case TM_BUF_Q2_SIZE:
            index = 3;
            break;
        case TM_BUF_Q3_SIZE:
            index = 4;
            break;
        case TM_BUF_Q4_SIZE:
            index = 5;
            break;
        case TM_BUF_Q5_SIZE:
            index = 6;
            break;
        case TM_BUF_Q6_SIZE:
            index = 7;
            break;
        default:
/* Bigger than we keep track of */
            index = -1;
            break;
    }
    if (index == -1)
    {
/* Bigger than we keep track of so give it back to the kernel/OS */
        memoryQueuePtrPtr = TM_RAW_MEMORY_NULL_PTRPTR;
        tm_kernel_free(sizePtr);
    }
    else
/* One of our queues */
    {
        memoryQueuePtrPtr = &tm_context(tvBufQListPtr[index]);
/* Put it to the head of the queue */
/* LOCK the queue */
        tm_lock_entry_init(lockEntryPtr, &tm_context(tvDynMemLockEntry));
        tm_lock_wait(lockEntryPtr);
        ((ttRawMemoryPtr)memoryBlockPtr)->rawNextPtr = *memoryQueuePtrPtr;
        *memoryQueuePtrPtr = (ttRawMemoryPtr)memoryBlockPtr;
        tm_context(tvMemRecycledBytes) += *sizePtr + TM_MEM_ALLOC_OVERHEAD;
/* UNLOCK the queue */
        tm_unlock(lockEntryPtr);
    }

#else /* !TM_USE_DYNAMIC_MEMORY */

    tm_kernel_free(sizePtr);

#endif /* TM_USE_DYNAMIC_MEMORY */

#ifdef TM_ERROR_CHECKING
    tm_kernel_set_critical;
    (tm_context(tvFreeRawCount))++;
    tm_kernel_release_critical;
#ifdef TM_USE_ANSI_LINE_FILE
#if defined(TM_KERNEL_VISUAL_X86) || defined(TM_KERNEL_WIN32_X86)
    fprintf(tm_context(tvMemoryLeakDetector),
            "FreeBuffer, %d, %ld, 0x%p, %d, ", tm_context(tvFreeRawCount),
            tvTime, memoryBlockPtr, size);
#else /* !( defined(TM_KERNEL_VISUAL_X86) || defined(TM_KERNEL_WIN32_X86)) */
#ifdef TM_MEMORY_LOGGING
    (void)tm_sprintf(
        (char *) tm_context(tvAssertTempBuf),
        "FreeBuffer, %d, %ld, 0x%p, %d, ", tm_context(tvFreeRawCount),
        tvTime, memoryBlockPtr, size);
#endif /* TM_MEMORY_LOGGING */
#endif /* !( defined(TM_KERNEL_VISUAL_X86) || defined(TM_KERNEL_WIN32_X86)) */
#endif /*  TM_USE_ANSI_LINE_FILE */
#endif /* TM_ERROR_CHECKING */
    return;
}


#ifdef TM_USE_DYNAMIC_MEMORY
/*
 * Insert into appropriate recycle queue for later re-use.
 * Only needed for dynamic memory management.
 */
void tfRecycleFree ( ttVoidPtr rcylVoidPtr, int recycleIndex )
{
    ttRcylPtr        rcylPtr;
    ttRcylPtrPtr     rcylListPtrPtr;
#ifdef TM_LOCK_NEEDED
    ttLockEntryPtr   lockEntryPtr;
#endif /* TM_LOCK_NEEDED */

    tm_memory_log2("RECY FREE %u 0x%p\n", recycleIndex, rcylVoidPtr);

    rcylPtr = (ttRcylPtr)rcylVoidPtr;
    tm_lock_entry_init(lockEntryPtr, &tm_context(tvDynMemLockEntry));
/* LOCK the recycle list */
    tm_lock_wait(lockEntryPtr);
    rcylListPtrPtr = &(tm_context(tvRcyListPtr[recycleIndex]));

/* Insert at head of recycle queue. */
    rcylPtr->rcylNextPtr = *rcylListPtrPtr;
    *rcylListPtrPtr = rcylPtr;
    tm_context(tvMemRecycledBytes) +=
                       tlRcyListSize[recycleIndex] + TM_RECY_ALLOC_OVERHEAD;
/* UNLOCK Recycle list */
    tm_unlock(lockEntryPtr);
}
#endif /* TM_USE_DYNAMIC_MEMORY */

#ifdef TM_USE_DYNAMIC_MEMORY
/*
 * Allocate a block of memory, by checking the appropriate recycle
 * queue for a free recycled element.
 * Only needed for dynamic memory management.
 */
ttVoidPtr tfRecycleAlloc ( int recycleIndex )
{
    ttRcylPtr        rcylPtr;
    ttRcylPtrPtr     rcylListPtrPtr;
    unsigned         size;
#ifdef TM_LOCK_NEEDED
    ttLockEntryPtr   lockEntryPtr;
#endif /* TM_LOCK_NEEDED */

    tm_memory_log1("RECY ALLOC %u\n", recycleIndex);

    tm_lock_entry_init(lockEntryPtr, &tm_context(tvDynMemLockEntry));
/* LOCK the recycle queue */
    tm_lock_wait(lockEntryPtr);
/* Get recycle list corresponding to recycleIndex */
    rcylListPtrPtr = &(tm_context(tvRcyListPtr[recycleIndex]));
/* Get size of allocated buffers on the recycle list */
    size = tlRcyListSize[recycleIndex];

    rcylPtr = *rcylListPtrPtr;
/* Check if there are entries on the recycle queue */
    if (rcylPtr != TM_RCYL_NULL_PTR)
    {
/* Remove the first entry from the recycle queue */
        *rcylListPtrPtr = rcylPtr->rcylNextPtr;
        tm_context(tvMemRecycledBytes) -=
                            size + TM_RECY_ALLOC_OVERHEAD;
/* UNLOCK the recycle queue */
        tm_unlock(lockEntryPtr);
    }
    else
    {
/* UNLOCK the recycle queue */
        tm_unlock(lockEntryPtr);
/* Go to the Kernel and get an entry block */
        rcylPtr = (ttRcylPtr)tm_kernel_malloc(size);
        if (rcylPtr == TM_RCYL_NULL_PTR)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfRecyleAlloc",
                            "Could not allocate memory from system");
#endif /* TM_ERROR_CHECKING */
             goto outRcylAlloc;
        }
    }
    if (recycleIndex != TM_RECY_PACKET)
/*
 * ttPacket buffers do not need to be zeroed out, since all the fields
 * are being re-initialized after the allocation in tfGetSharedBuffer(),
 * or tfDuplicateSharedBuffer().
 */
    {
        tm_bzero((ttVoidPtr)rcylPtr, size);
    }
outRcylAlloc:
    tm_memory_log2("RECY ALLOC %u 0x%p\n", recycleIndex, rcylPtr);
    return (ttVoidPtr)rcylPtr;

}

/*
 * Last call before tfKernelMalloc() so that we can free dynamic memory
 * if memory is so fragmented that we cannot find any free block.
 */
ttVoidPtr tfBufferDoubleMalloc(unsigned size)
{
    ttVoidPtr memPtr;

    memPtr = tm_kernel_single_malloc(size);
    if (memPtr == (ttVoidPtr)0)
    {
        tfFreeDynamicMemory();
        memPtr = tm_kernel_single_malloc(size);
#ifdef TM_MULTIPLE_CONTEXT
        if (memPtr == (ttVoidPtr)0)
        {
/* Free dynamic memory in all contexts */
            tfFreeAllDynamicMemory();
            memPtr = tm_kernel_single_malloc(size);
        }
#endif /* TM_MULTIPLE_CONTEXT */
#if defined(TM_USE_SHEAP) && defined(TM_ERROR_CHECKING)
        if (memPtr == (ttVoidPtr)0)
        {
            tfKernelWarning("tfBufferDoubleMalloc","Ran out of Simple Heap");
        }
#endif /* TM_USE_SHEAP && TM_ERROR_CHECKING */
    }
    return memPtr;
}

#else /* !TM_USE_DYNAMIC_MEMORY */

/*
 * Allocate a block of memory, and initialize it to zero.
 * Only needed if dynamic memory management is NOT used.
 */
ttVoidPtr tfBufferCalloc (unsigned int size)
{
    ttVoidPtr memPtr;

    memPtr = tm_kernel_malloc(size);
    if (memPtr != (ttVoidPtr)0)
    {
        tm_bzero(memPtr, size);
    }
    return memPtr;
}
#endif /* !TM_USE_DYNAMIC_MEMORY */

/*
 * Free all memory allocated dynamically by the Treck internal memory
 * management system. Called by the user, when the user does not
 * want to use the treck stack anymore, and wants all currently unused
 * memory to be returned to the user's system.
 * Only needed for dynamic memory management.
 * Also called by the stack when stack runs out of memory.
 */
int tfFreeDynamicMemory (void)
{
#ifdef TM_USE_DYNAMIC_MEMORY
#ifdef TM_USE_SHEAP
    ttRawMemoryPtr              memPtrList[TM_BUF_Q_NUM + 1];
    ttRcylPtr                   rcylPtrList[TM_RECY_NUM + 1];
    int                         index;
    int                         ptrIndex;
#ifndef TM_SHEAP_OPTIMIZE_FREE_SPEED
    tt8Bit                      freedMemory;
#endif /* !TM_SHEAP_OPTIMIZE_FREE_SPEED */
#ifdef TM_LOCK_NEEDED
    ttLockEntryPtr  lockEntryPtr;
#endif /* TM_LOCK_NEEDED */

#ifndef TM_SHEAP_OPTIMIZE_FREE_SPEED
    freedMemory = TM_8BIT_NO;
#endif /* !TM_SHEAP_OPTIMIZE_FREE_SPEED */
/* Empty the Memory and recycle queues */
    ptrIndex = 0;
    tm_lock_entry_init(lockEntryPtr, &tm_context(tvDynMemLockEntry));
    tm_lock_wait_no_log(lockEntryPtr);
/* Empty the memory queues */
    for ( index = 0; index < TM_BUF_Q_NUM; index++)
    {
        memPtrList[ptrIndex] = tm_context(tvBufQListPtr)[index];
        if (memPtrList[ptrIndex] != (ttRawMemoryPtr)0)
        {
            ptrIndex++;
        }
/* Empty list */
        tm_context(tvBufQListPtr)[index] = TM_RAW_MEMORY_NULL_PTR;
    }
#ifndef TM_SHEAP_OPTIMIZE_FREE_SPEED
    if (ptrIndex > 0)
    {
        freedMemory = TM_8BIT_YES;
    }
#endif /* !TM_SHEAP_OPTIMIZE_FREE_SPEED */
    memPtrList[ptrIndex] =  TM_RAW_MEMORY_NULL_PTR; /* null terminated */
    ptrIndex = 0;
/* Empty the recycle lists */
    for ( index = 0; index < TM_RECY_NUM; index++)
    {
        rcylPtrList[ptrIndex] = tm_context(tvRcyListPtr[index]);
        if (rcylPtrList[ptrIndex] != (ttRcylPtr)0)
        {
            ptrIndex++;
        }
/* Empty list */
        tm_context(tvRcyListPtr[index]) = (ttRcylPtr)0;
    }
#ifndef TM_SHEAP_OPTIMIZE_FREE_SPEED
    if (!freedMemory && ptrIndex > 0)
    {
        freedMemory = TM_8BIT_YES;
    }
#endif /* !TM_SHEAP_OPTIMIZE_FREE_SPEED */
    rcylPtrList[ptrIndex] =  (ttRcylPtr)0; /* null terminated */
#ifndef TM_SHEAP_OPTIMIZE_FREE_SPEED
    if (freedMemory)
#endif /* !TM_SHEAP_OPTIMIZE_FREE_SPEED */
    {
/* Free all memory queues, and recycled lists in the simple heap */
        tfSheapCollect(  memPtrList
                       , rcylPtrList
#ifdef TM_LOCK_NEEDED
                       , TM_8BIT_YES
#endif /* TM_LOCK_NEEDED */
                       );
        tm_context(tvMemRecycledBytes) = TM_UL(0);
    }
/* UnLock dynamic memory lists */
    tm_unlock_no_log(lockEntryPtr);
    return TM_ENOERROR;
#else /* !TM_USE_SHEAP */
    return tfFreeRangeDynamicMemory(0, -1);
#endif /* !TM_USE_SHEAP */
#else /* !TM_USE_DYNAMIC_MEMORY */
#ifdef TM_USE_SHEAP
/* Garbage collection */
    tfSheapCollect(
#ifdef TM_LOCK_NEEDED
                   TM_8BIT_YES
#endif /* TM_LOCK_NEEDED */
                   );
    return TM_ENOERROR;
#else /* !TM_USE_SHEAP */
    return TM_ENOENT;
#endif /* !TM_USE_SHEAP */
#endif /* !TM_USE_DYNAMIC_MEMORY */
}

/*
 * Free memory allocated dynamically by the Treck internal memory
 * management system. Called by the user, when the user wants to free
 * some memory to avoid out of memory conditions.
 * Memory will be freed only in queue sizes that are between the
 * minimum and the maximum sizes. (-1 means infinity, i.e. no limit)
 * Only needed for dynamic memory management.
 */
int tfFreeRangeDynamicMemory (int minSize, int maxSize)
{
#ifdef TM_USE_DYNAMIC_MEMORY
    ttRawMemoryPtr              memoryPtr;
    tt8BitPtr                   dumpMemoryPtr;
    ttRcylPtr                   recyclePtr;
    ttRcylPtr                   dumpRecyclePtr;
#ifdef TM_LOCK_NEEDED
    ttLockEntryPtr              lockEntryPtr;
#endif /* TM_LOCK_NEEDED */
    unsigned int                size;
    int                         index;

/* Lock dynamic memory lists */
    tm_lock_entry_init(lockEntryPtr, &tm_context(tvDynMemLockEntry));
    tm_lock_wait_no_log(lockEntryPtr);
/* Free the Memory queues */
    for ( index = 0; index < TM_BUF_Q_NUM; index++)
    {
/* Free the memory queues whose sizes are in the given range */
        size = tlBufQSize[index];
        if (    ((unsigned int)minSize <= size)
             && ((maxSize == -1) || ((unsigned int)maxSize >= size)) )
        {
            memoryPtr = tm_context(tvBufQListPtr)[index];
            while (memoryPtr != (ttRawMemoryPtr)0)
            {
/* Point to beginning of kernel memory block */
                dumpMemoryPtr = (tt8BitPtr)memoryPtr - sizeof(tt32Bit);
/* Point to next, before freeing memory */
                memoryPtr = memoryPtr->rawNextPtr;
                tm_kernel_free(dumpMemoryPtr);
                tm_context(tvMemRecycledBytes) -= size + TM_MEM_ALLOC_OVERHEAD;
            }
/* Empty list */
            tm_context(tvBufQListPtr)[index] = TM_RAW_MEMORY_NULL_PTR;
        }
    }
/* Free the recycle lists */
    for ( index = 0; index < TM_RECY_NUM; index++)
    {
/* Free the recycle lists whose sizes are in the given range */
        size = tlRcyListSize[index];
        if (   ((unsigned int)minSize <= size)
            && (    (maxSize == -1)
                 || ((unsigned int)maxSize >= size) ) )
        {
            recyclePtr = tm_context(tvRcyListPtr[index]);
            while (recyclePtr != (ttRcylPtr)0)
            {
                dumpRecyclePtr = recyclePtr;
/* Point to next, before freeing memory */
                recyclePtr = recyclePtr->rcylNextPtr;
                tm_kernel_free(dumpRecyclePtr);
                tm_context(tvMemRecycledBytes) -= size + TM_RECY_ALLOC_OVERHEAD;
            }
/* Empty list */
            tm_context(tvRcyListPtr[index]) = (ttRcylPtr)0;
        }
    }
/* UnLock dynamic memory lists */
    tm_unlock_no_log(lockEntryPtr);
    return TM_ENOERROR;

#else /* !TM_USE_DYNAMIC_MEMORY */

    TM_UNREF_IN_ARG(minSize);
    TM_UNREF_IN_ARG(maxSize);

    return TM_ENOENT;

#endif /* TM_USE_DYNAMIC_MEMORY */
}

#ifdef TM_MULTIPLE_CONTEXT

#if defined(TM_USE_DYNAMIC_MEMORY) || defined(TM_USE_SHEAP)
/* Call back routine to free dynamic memory for one context */
static int tfFreeDynamicMemoryCB (
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam )
{
    TM_UNREF_IN_ARG(genParam);
    tfSetCurrentContext((ttUserContext)nodePtr);
    tfFreeDynamicMemory();
    return 0; /* Keep walking */
}
#endif /* TM_USE_DYNAMIC_MEMORY || TM_USE_SHEAP */

#ifdef TM_USE_DYNAMIC_MEMORY
/* Call back routine to free range dynamic memory for one context */
static int tfFreeRangeDynamicMemoryCB (
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam )
{
    struct tsMinMaxVal TM_FAR * minMaxValPtr;

    tfSetCurrentContext((ttUserContext)nodePtr);
    minMaxValPtr = (struct tsMinMaxVal TM_FAR *)(genParam.genVoidParmPtr);
    tfFreeRangeDynamicMemory(minMaxValPtr->minVal, minMaxValPtr->maxVal);
    return 0; /* Keep walking */
}
#endif /* TM_USE_DYNAMIC_MEMORY */

/* Free dynamic memory in all contexts */
int tfFreeAllDynamicMemory (void)
{
#if defined(TM_USE_DYNAMIC_MEMORY) || defined(TM_USE_SHEAP)
    ttGenericUnion     genParam; /* unused */
    ttUserContext      currentContext;

    currentContext = tfGetCurrentContext();
    genParam.genIntParm = 0; /* compiler warning */
    tfListWalk(&tm_global(tvContextHead),
               tfFreeDynamicMemoryCB,
               genParam);
    tfSetCurrentContext(currentContext);
    return TM_ENOERROR;
#else /* !TM_USE_DYNAMIC_MEMORY && !TM_USE_SHEAP */
    return TM_ENOENT;
#endif /* TM_USE_DYNAMIC_MEMORY */
}

/* Free a range of dynamic memory in all contexts */
int tfFreeAllRangeDynamicMemory (int minSize, int maxSize)
{
#ifdef TM_USE_DYNAMIC_MEMORY
    ttUserContext      currentContext;
    ttGenericUnion     genParam;
    struct tsMinMaxVal minMaxStruct;

    minMaxStruct.minVal = minSize;
    minMaxStruct.maxVal = maxSize;
    genParam.genVoidParmPtr = (ttVoidPtr)&minMaxStruct;
    currentContext = tfGetCurrentContext();
    tfListWalk(&tm_global(tvContextHead),
               tfFreeRangeDynamicMemoryCB,
               genParam);
    tfSetCurrentContext(currentContext);
    return TM_ENOERROR;
#else /* !TM_USE_DYNAMIC_MEMORY */
    TM_UNREF_IN_ARG(minSize);
    TM_UNREF_IN_ARG(maxSize);
    return TM_ENOENT;
#endif /* TM_USE_DYNAMIC_MEMORY */
}
#endif /* TM_MULTIPLE_CONTEXT */
