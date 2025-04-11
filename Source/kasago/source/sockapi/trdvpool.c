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
 * Description: Interface Pre-allocated Receive Pool functions
 *
 * Filename: trdvpool.c
 * Author: Odile
 * Date Created: 02/19/00
 * $Source: source/sockapi/trdvpool.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:03JST $
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
#if (defined(TM_USE_KERNEL_CRITICAL_STATUS) || defined(TM_ERROR_CHECKING))
#include <trglobal.h>
#endif /* TM_USE_KERNEL_CRITICAL_STATUS || TM_ERROR_CHECKING */

/* Define compile option for unreferenced macro */
#ifdef TM_LINT
LINT_UNREF_MACRO(TM_POOL_BUF_EMPTY)
#endif /* TM_LINT */
/*
 * Local macros 
 */

#define TM_POOL_FULL_INDEX       0  /* Index of pool of full size buffers */
#define TM_POOL_SMALL_INDEX      1  /* Index of pool of small size buffers */
/* No buffer allocated */
#define TM_POOL_BUF_EMPTY        (tt16Bit)0x00
/* Buffer ready to be used by chip */
#define TM_POOL_BUF_CHIP_OWNS    (tt16Bit)0x01
/* Buffer ready to be received (owned by the recv thread) */
#define TM_POOL_BUF_CPU_OWNS     (tt16Bit)0x02
/* A buffer has been allocated */
#define TM_POOL_BUF_ALLOC_FLAG   (tt16Bit) \
                                (TM_POOL_BUF_CHIP_OWNS | TM_POOL_BUF_CPU_OWNS)
/* Buffer belongs to small buffer ring */
#define TM_POOL_BUF_SMALL        (tt16Bit)0x10

/* A buffer is allocated (at that ring slot) */
#define tm_pool_buf_is_allocated(poolBufFlags) \
                ((poolBufFlags) & TM_POOL_BUF_ALLOC_FLAG)

/*
 * Local types
 */

/* Structure corresponding to one entry in a ring of allocated buffers */
typedef struct tsPoolEntry
{
    struct tsPoolEntry TM_FAR * poolNextPtr;
/* pointer to pre-allocated buffer */
    ttCharPtr                   poolBufPtr;
/* pre-allocated buffer handle */
    ttUserBuffer                poolBufHandle;
/*
 * Possible values for poolBufFlags:
 * 0                             No pre-allocated buffer
 * TM_POOL_BUF_CHIP_OWNS         Ethernet chip owns. Can copy data in ISR.
 * TM_POOL_BUF_CPU_OWNS          CPU owns. Data ready to be received.
 * TM_POOL_BUF_SMALL             buffer belongs to small buffer size pool
 */
    tt16Bit                     poolBufFlags;  /* Buffer flag */
    tt16Bit                     poolBufLength; /* Buffer length */
} ttPoolEntry;

typedef ttPoolEntry TM_FAR * ttPoolEntryPtr;

/* Structure corresponding to one Ring of allocated buffers */
typedef struct tsPoolAlloc
{
    ttPoolEntry    allocRing[1]; /* Variable length */
} ttPoolAlloc;

typedef ttPoolAlloc TM_FAR * ttPoolAllocPtr;


/* Head pointer to recv pool */
typedef struct tsPoolHead
{
    tt16Bit         headAllocBufferSize[2]; /* buffer sizes (full & small) */
/* Number of slots (in full & small pools)*/
    tt16Bit         headAllocSlots[2];
/* Pointer to ring of buffers (in full & small pools)*/
    ttPoolAllocPtr  headAllocPtr[2];
    tt16Bit         headAllocIsrIndex[2]; /* Isr Get index in alloc buffers */
/* If set, allocate new buffer in tfPoolReceive routine */
    tt16Bit         headAllocInLineFlag;
/* Alignnment allocation requirement */
    tt16Bit         headAllocAlignment;
/* Queue of packets ready to be received by the Treck stack */
    ttPoolEntryPtr  headRecvQueueFirstPtr;
/*
 * Point to last element in the Queue of packets ready to be received by
 * the Treck stack.
 */
    ttPoolEntryPtr  headRecvQueueLastPtr;
} ttPoolHead;

typedef ttPoolHead TM_FAR * ttPoolHeadPtr;

/*
 * Local functions
 */

int tfPoolRefill(ttUserInterface interfaceHandle);

/*
 * Allocate an empty ring store pointer in devEntryPtr. Call
 * tfRefillPool() to allocate buffers in the ring.
 */
int tfPoolCreate ( ttUserInterface interfaceHandle,
                   int             numberFullSizeBuffers,
                   int             numberSmallSizeBuffers,
                   int             fullBufferSize,
                   int             smallBufferSize,
                   int             alignment,
/* Indicates whether to re-allocate in line */
                   int             flag )
{
    ttDeviceEntryPtr    devEntryPtr;
    ttPoolHeadPtr       poolHeadPtr;
    int                 errorCode;
    int                 fullAllocPoolSize;
    int                 allocSize;

    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
    poolHeadPtr = (ttPoolHeadPtr)devEntryPtr->devPoolHeadPtr;
    if (    ( numberFullSizeBuffers <= 0 )
         || ( fullBufferSize <= 0 )
         || (    tm_ll_is_enet(devEntryPtr)
#ifdef TM_USE_IPV4
                 && (fullBufferSize < 
                     ((int)(devEntryPtr->devMtu) + TM_ETHER_HEADER_SIZE + 4))
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
                 && (fullBufferSize < 
                     ((int)(devEntryPtr->dev6Mtu) + TM_ETHER_HEADER_SIZE + 4))
#endif /* TM_USE_IPV6 */
             )
         || ( numberSmallSizeBuffers < 0 )
         || ( smallBufferSize < 0 )
         || ( (smallBufferSize == 0) && (numberSmallSizeBuffers != 0) )
         || ( alignment < 0 )
         || ( alignment > 64 )
         || ( (flag != TM_POOL_REFILL_IN_LINE) && (flag != 0) ) )
    {
        errorCode = TM_EINVAL;
        goto poolCreateExit;
    }
    if (devEntryPtr->devFlag & TM_DEV_OPENED)
    {
        errorCode = TM_EPERM;
        goto poolCreateExit;
    }
    if (poolHeadPtr != (ttPoolHeadPtr)0)
    {
        errorCode = TM_EALREADY;
        goto poolCreateExit;
    }
/*
 * Allocate and initialize empty ring
 */
    fullAllocPoolSize = (int)sizeof(ttPoolEntry) * numberFullSizeBuffers;
    allocSize =  (int)sizeof(ttPoolHead) + fullAllocPoolSize;
    if (numberSmallSizeBuffers != 0)
    {
        allocSize =    allocSize +
                    + ((int)sizeof(ttPoolEntry) * numberSmallSizeBuffers);
    }
    poolHeadPtr = (ttPoolHeadPtr)tm_get_raw_buffer((unsigned int)allocSize);
    if (poolHeadPtr == (ttPoolHeadPtr)0)
    {
        errorCode = TM_ENOBUFS;
        goto poolCreateExit;
    }
    tm_bzero(poolHeadPtr, allocSize);
    poolHeadPtr->headAllocPtr[0] =  (ttPoolAllocPtr)
                                                (   ((ttCharPtr)poolHeadPtr)
                                                  + sizeof(ttPoolHead) );
    if (numberSmallSizeBuffers != 0)
    {
        poolHeadPtr->headAllocPtr[1] =  (ttPoolAllocPtr)
                               (   ((ttCharPtr)poolHeadPtr->headAllocPtr[0])
                                 + fullAllocPoolSize );
    }
    poolHeadPtr->headAllocSlots[TM_POOL_FULL_INDEX] =
                                              (tt16Bit)numberFullSizeBuffers;
    poolHeadPtr->headAllocSlots[TM_POOL_SMALL_INDEX] =
                                              (tt16Bit)numberSmallSizeBuffers;
    poolHeadPtr->headAllocBufferSize[TM_POOL_FULL_INDEX] =
                                              (tt16Bit)fullBufferSize;
    poolHeadPtr->headAllocBufferSize[TM_POOL_SMALL_INDEX] =
                                              (tt16Bit)smallBufferSize;
    poolHeadPtr->headAllocAlignment = (tt16Bit)alignment;
    poolHeadPtr->headAllocInLineFlag = (tt16Bit)flag;
    devEntryPtr->devPoolHeadPtr = (ttVoidPtr)poolHeadPtr;
    devEntryPtr->devRefillPoolFuncPtr = tfPoolRefill;

/*
 * pre-allocate receive buffers
 */
    errorCode = tfPoolRefill(interfaceHandle);
    if (errorCode == TM_EINPROGRESS)
    {
        errorCode = TM_ENOBUFS;
    }
    if (errorCode == TM_ENOBUFS)
    {
        (void)tfPoolDelete(interfaceHandle);
    }
poolCreateExit:
    return errorCode;
}

/*
 * Access ring pointer in devEntry.
 * Free all currently allocated buffers + the ring.
 */
int tfPoolDelete (ttUserInterface interfaceHandle)
{
    ttDeviceEntryPtr    devEntryPtr;
    ttPoolHeadPtr       poolHeadPtr;
    ttPoolAllocPtr      poolAllocPtr;
    ttPoolEntryPtr      poolEntryPtr;
    int                 poolIndex;
    int                 slotCount;
    int                 bufCount;
    int                 errorCode;

    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
    poolHeadPtr = (ttPoolHeadPtr)devEntryPtr->devPoolHeadPtr;
    devEntryPtr->devRefillPoolFuncPtr = (ttDevRefillPoolFuncPtr)0;
    if (poolHeadPtr != (ttPoolHeadPtr)0)
    {
        errorCode = TM_ENOERROR;
        tm_kernel_set_critical;
        devEntryPtr->devPoolHeadPtr = (ttVoidPtr)0;
        tm_kernel_release_critical;
        for ( poolIndex = TM_POOL_SMALL_INDEX;
              poolIndex >= TM_POOL_FULL_INDEX;
              poolIndex-- )
        {
            slotCount = poolHeadPtr->headAllocSlots[poolIndex];
            poolAllocPtr = poolHeadPtr->headAllocPtr[poolIndex];
            for( bufCount = 0; bufCount < slotCount; bufCount++)
            {
                poolEntryPtr = &poolAllocPtr->allocRing[bufCount];
                if (poolEntryPtr->poolBufFlags & TM_POOL_BUF_ALLOC_FLAG)
                {
                    (void)tfFreeDriverBuffer( poolEntryPtr->poolBufHandle );
                }
            }
        }
        tm_free_raw_buffer((ttRawBufferPtr)poolHeadPtr);
    }
    else
    {
        errorCode = TM_EALREADY;
    }
    return errorCode;
}

/*
 * Access ring pointer in devEntry.
 * Allocate buffers in the ring.
 */
int tfPoolRefill (ttUserInterface interfaceHandle)
{
    ttDeviceEntryPtr devEntryPtr;
    ttPoolHeadPtr    poolHeadPtr;
    ttPoolAllocPtr   poolAllocPtr;
    ttPoolEntryPtr   poolEntryPtr;
    tt16Bit          bufAllocFlag;
    tt16Bit          poolEntryAllocFlag;
    int              bufCount;
    int              slotCount;
    int              alignment;
    int              size;
    int              progress;
    int              poolIndex;
    int              errorCode;

    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
    poolHeadPtr = (ttPoolHeadPtr)devEntryPtr->devPoolHeadPtr;
    errorCode = TM_ENOERROR;
    progress = 0;
    if (poolHeadPtr != (ttPoolHeadPtr)0)
    {
        alignment = (int)poolHeadPtr->headAllocAlignment;
        for ( poolIndex = TM_POOL_SMALL_INDEX;
              poolIndex >= TM_POOL_FULL_INDEX;
              poolIndex-- )
        {
            slotCount = poolHeadPtr->headAllocSlots[poolIndex];
            if (slotCount != 0)
            {
                poolAllocPtr = poolHeadPtr->headAllocPtr[poolIndex];
                size = poolHeadPtr->headAllocBufferSize[poolIndex];
                bufAllocFlag = TM_POOL_BUF_CHIP_OWNS;
                if (poolIndex == TM_POOL_SMALL_INDEX)
                {
                    bufAllocFlag = TM_POOL_BUF_SMALL | TM_POOL_BUF_CHIP_OWNS;
                }
                for( bufCount = 0; bufCount < slotCount; bufCount++)
                {
                    poolEntryPtr = &poolAllocPtr->allocRing[bufCount];
/*
 * Need protection, since rinBufFlags read/set by the ISR.
 */
                    tm_kernel_set_critical;
                    poolEntryAllocFlag = poolEntryPtr->poolBufFlags;
                    tm_kernel_release_critical;
                    if (!tm_pool_buf_is_allocated(poolEntryAllocFlag))
                    {
#ifdef TM_USE_DCACHE_MANAGEMENT
/* 
 * If the user has requested that the stack handle cache coherency, use the
 * cache-safe buffer allocator
 */
                        if (devEntryPtr->devCacheInvalFuncPtr !=
                            (ttDevCacheMgmtFuncPtr)0)
                        {
                            poolEntryPtr->poolBufPtr =
                                    tfGetCacheSafeDrvBuf(
                                        interfaceHandle,
                                        &poolEntryPtr->poolBufHandle,
                                        size
#ifndef TM_NEED_ETHER_32BIT_ALIGNMENT
                                        + TM_ETHER_IP_ALIGN
#endif /* !TM_NEED_ETHER_32BIT_ALIGNMENT */
                                        ,
                                        alignment,
                                        0);
                        }
                        else
#endif /* TM_USE_DCACHE_MANAGEMENT */
                        {
                            poolEntryPtr->poolBufPtr =
                                    tfGetDriverBuffer(
                                        &poolEntryPtr->poolBufHandle,
                                        size
#ifndef TM_NEED_ETHER_32BIT_ALIGNMENT
                                        + TM_ETHER_IP_ALIGN
#endif /* !TM_NEED_ETHER_32BIT_ALIGNMENT */
                                        ,
                                        alignment);
                        }
                        if (poolEntryPtr->poolBufPtr == (ttCharPtr)0)
                        {
                            errorCode = TM_ENOBUFS;
                            break;
                        }
#ifndef TM_NEED_ETHER_32BIT_ALIGNMENT
#ifndef TM_USE_DCACHE_MANAGEMENT
                        poolEntryPtr->poolBufPtr += TM_ETHER_IP_ALIGN;
#endif /* TM_USE_DCACHE_MANAGEMENT */
#endif /* !TM_NEED_ETHER_32BIT_ALIGNMENT */
                        poolEntryPtr->poolBufLength = (tt16Bit)size;
                        tm_kernel_set_critical;
                        poolEntryPtr->poolBufFlags = bufAllocFlag;
                        tm_kernel_release_critical;
                        progress++;
                    }
                }
            }
            if (errorCode != TM_ENOERROR)
            {
                break;
            }
        }
    }
    if (errorCode != TM_ENOERROR)
    {
        if (progress)
        {
            errorCode = TM_EINPROGRESS;
        }
    }
    return errorCode;
}

/*
 * Access ring pointer in devEntry. Get next received buffer in queue.
 * Fill in parameters.
 * TM_DEV_OKAY if there exists a recv buffer, otherwise TM_DEV_ERROR 
 */
int tfPoolReceive ( ttUserInterface        interfaceHandle,
                    char TM_FAR * TM_FAR * dataPtrPtr,
                    int  TM_FAR *          dataLengthPtr,
                    ttUserBufferPtr        bufHandlePtr )
{
    ttDeviceEntryPtr devEntryPtr;
    ttPoolHeadPtr    poolHeadPtr;
    ttPoolEntryPtr   poolEntryPtr;
    tt16Bit          bufAllocFlag;
    int              poolIndex;
    int              size;
    int              errorCode;

    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
    poolHeadPtr = (ttPoolHeadPtr)devEntryPtr->devPoolHeadPtr;
    errorCode = TM_DEV_ERROR; /* assume failure */
    if (poolHeadPtr != (ttPoolHeadPtr)0)
    {
/* Need protection to read this, since it could be set in the ISR */
        tm_kernel_set_critical;
        poolEntryPtr = poolHeadPtr->headRecvQueueFirstPtr;
        if ( poolEntryPtr != (ttPoolEntryPtr)0 )
/* Buffer In recv queue ready to be received by the stack */
        {
            poolHeadPtr->headRecvQueueFirstPtr = poolEntryPtr->poolNextPtr;
            if (poolHeadPtr->headRecvQueueLastPtr == poolEntryPtr)
            {
/* This was the last entry. Queue is now empty. */
                poolHeadPtr->headRecvQueueLastPtr = (ttPoolEntryPtr)0;
#ifdef TM_ERROR_CHECKING
                if (poolHeadPtr->headRecvQueueFirstPtr != (ttPoolEntryPtr)0)
                {
                    tfKernelError(
                        "tfPoolRecvBuffer", "Recv queue corrupted\n");
                }
#endif /* TM_ERROR_CHECKING */
            }
            tm_kernel_release_critical;
            poolEntryPtr->poolNextPtr = (ttPoolEntryPtr)0;
            *dataPtrPtr = poolEntryPtr->poolBufPtr;
            *dataLengthPtr = (int)poolEntryPtr->poolBufLength;
            *bufHandlePtr = poolEntryPtr->poolBufHandle;
            bufAllocFlag = 0;
            if (poolHeadPtr->headAllocInLineFlag)
            {
/*
 * If desired, refill this buffer now
 * Note that the ioctl still need to be called from the timer task
 * in case we fail to allocate memory here.
 */
                if (poolEntryPtr->poolBufFlags & TM_POOL_BUF_SMALL)
                {
                    poolIndex = TM_POOL_SMALL_INDEX;
                }
                else
                {
                    poolIndex = TM_POOL_FULL_INDEX;
                }
                size = poolHeadPtr->headAllocBufferSize[poolIndex];
#ifdef TM_USE_DCACHE_MANAGEMENT
/* 
 * If the user has requested that the stack handle cache coherency, use the
 * cache-safe buffer allocator
 */
                if (devEntryPtr->devCacheInvalFuncPtr !=
                    (ttDevCacheMgmtFuncPtr)0)
                {
                    poolEntryPtr->poolBufPtr =
                            tfGetCacheSafeDrvBuf(
                                    interfaceHandle,
                                    &poolEntryPtr->poolBufHandle,
                                    size
#ifndef TM_NEED_ETHER_32BIT_ALIGNMENT
                                    + TM_ETHER_IP_ALIGN
#endif /* !TM_NEED_ETHER_32BIT_ALIGNMENT */
                                    ,
                                    (int)poolHeadPtr->headAllocAlignment,
                                    0);
                }
                else
#endif /* TM_USE_DCACHE_MANAGEMENT */
                {
                    poolEntryPtr->poolBufPtr =
                            tfGetDriverBuffer(
                                    &poolEntryPtr->poolBufHandle,
                                    size
#ifndef TM_NEED_ETHER_32BIT_ALIGNMENT
                                    + TM_ETHER_IP_ALIGN
#endif /* !TM_NEED_ETHER_32BIT_ALIGNMENT */
                                    ,
                                    (int)poolHeadPtr->headAllocAlignment);
                }
                if (poolEntryPtr->poolBufPtr != (ttCharPtr)0)
                {
#ifndef TM_NEED_ETHER_32BIT_ALIGNMENT
#ifndef TM_USE_DCACHE_MANAGEMENT
                    poolEntryPtr->poolBufPtr += TM_ETHER_IP_ALIGN;
#endif /* TM_USE_DCACHE_MANAGEMENT */
#endif /* !TM_NEED_ETHER_32BIT_ALIGNMENT */
                    poolEntryPtr->poolBufLength = (tt16Bit)size;
/* set CHIP own buffer */
                    bufAllocFlag = TM_POOL_BUF_CHIP_OWNS;
                }
            }
            tm_kernel_set_critical;
            poolEntryPtr->poolBufFlags |= bufAllocFlag;
/* Reset CPU ownership (buffer will be freed by the stack) */
            tm_16bit_clr_bit( poolEntryPtr->poolBufFlags, 
                              TM_POOL_BUF_CPU_OWNS );
            tm_kernel_release_critical;
            errorCode = TM_DEV_OKAY;
        }
        else
        {
            tm_kernel_release_critical;
        }
    }
    return errorCode;
}

/* Get a pre-allocated buffer. Called from an ISR */
ttCharPtr tfPoolIsrGetBuffer (ttUserInterface interfaceHandle, int size)
{
    ttDeviceEntryPtr devEntryPtr;
    ttPoolHeadPtr    poolHeadPtr;
    ttPoolEntryPtr   poolEntryPtr;
    ttCharPtr        dataPtr;
    int              slotCount;
    int              poolIndex;
    int              isrIndex;

    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
    poolHeadPtr = (ttPoolHeadPtr)devEntryPtr->devPoolHeadPtr;
    dataPtr = (ttCharPtr)0;
    if ((poolHeadPtr != (ttPoolHeadPtr)0) && (size > 0))
    {
        if ( (tt16Bit) size <= 
             poolHeadPtr->headAllocBufferSize[TM_POOL_SMALL_INDEX])
        {
            poolIndex = TM_POOL_SMALL_INDEX;
        }
        else
        {
            poolIndex = TM_POOL_FULL_INDEX;
        }
        slotCount = poolHeadPtr->headAllocSlots[poolIndex];
        isrIndex = poolHeadPtr->headAllocIsrIndex[poolIndex];
        poolEntryPtr =
                   &poolHeadPtr->headAllocPtr[poolIndex]->allocRing[isrIndex];
/* 
 * If there is a buffer that has been allocated
 */
        if ( poolEntryPtr->poolBufFlags & TM_POOL_BUF_CHIP_OWNS )
        {
            poolEntryPtr->poolBufLength = (unsigned short)size;
/* Get pointer to current buffer entry */
            dataPtr = (char TM_FAR *)poolEntryPtr->poolBufPtr;
            if (isrIndex < slotCount - 1)
            {
                isrIndex++;
            }
            else
            {
                isrIndex = 0;
            }
            poolHeadPtr->headAllocIsrIndex[poolIndex] = (tt16Bit)isrIndex;
/* Buffer now owned by the CPU. Set CPU_OWNS, reset CHIP_OWNS */
            poolEntryPtr->poolBufFlags |= TM_POOL_BUF_CPU_OWNS;
            tm_16bit_clr_bit( poolEntryPtr->poolBufFlags,
                              TM_POOL_BUF_CHIP_OWNS );
            if (poolHeadPtr->headRecvQueueLastPtr != (ttPoolEntryPtr)0)
            {
/* Recv queue non empty. Insert at end */
                poolHeadPtr->headRecvQueueLastPtr->poolNextPtr = poolEntryPtr;
            }
            else
            {
#ifdef TM_ERROR_CHECKING
                if (poolHeadPtr->headRecvQueueFirstPtr != (ttPoolEntryPtr)0)
                {
                    tfKernelError("tfPoolIsrGetBuffer",
                                  "Recv queue corrupted\n");
                }
#endif /* TM_ERROR_CHECKING */
/* Recv queue empty. Insert at head */
                poolHeadPtr->headRecvQueueFirstPtr = poolEntryPtr;
            }
/* Update last pointer */
            poolHeadPtr->headRecvQueueLastPtr = poolEntryPtr;
        }

    }
    return dataPtr;
}

/*
 * tfPoolIsrFreeBuffer function description
 * Called from recv ISR, if last packet returned by tfPoolIsrGetBuffer(),
 * queued to be processed by the tfRecvInterface, needs to be
 * freed, i.e. removed from the pool recv queue, and put back
 * in the ring of free buffers to be used by the chip.
 * Parameters:
 * devEntryPtr  Interface handle
 * size         size of buffer just allocated
 *
 */
void tfPoolIsrFreeBuffer( ttUserInterface interfaceHandle,
                          int             dataSize )
{
    ttDeviceEntryPtr    devEntryPtr;
    ttPoolHeadPtr       poolHeadPtr;
    ttPoolEntryPtr      poolEntryPtr;
    ttPoolEntryPtr      prevPoolPtr;
    ttPoolEntryPtr      curPoolPtr;
    int                 poolIndex;
    int                 isrIndex;
    int                 slotCount;

    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
    poolHeadPtr = (ttPoolHeadPtr)devEntryPtr->devPoolHeadPtr;
/* Buffer is at the end of the pool entry queue */
    poolEntryPtr = poolHeadPtr->headRecvQueueLastPtr;
/*
 * Remove last entry from list of received packet ready to be processed
 */
    curPoolPtr = poolHeadPtr->headRecvQueueFirstPtr;
    if (curPoolPtr != (ttPoolEntryPtr)0)
    {
        if (poolEntryPtr == curPoolPtr)
/* Last and First match: One entry in the list only */
        {
            poolHeadPtr->headRecvQueueFirstPtr = (ttPoolEntryPtr)0;
            poolHeadPtr->headRecvQueueLastPtr = (ttPoolEntryPtr)0;
        }
        else
        {
            prevPoolPtr = curPoolPtr;
            curPoolPtr = curPoolPtr->poolNextPtr;
            while (curPoolPtr != (ttPoolEntryPtr)0)
/* Find point of deletion */
            {
                if (curPoolPtr == poolEntryPtr)
                {
/* Delete from end of queue */
                    prevPoolPtr->poolNextPtr = (ttPoolEntryPtr)0;
                    poolHeadPtr->headRecvQueueLastPtr = prevPoolPtr;
                    break;
                }
                prevPoolPtr = curPoolPtr;
                curPoolPtr = curPoolPtr->poolNextPtr;
            }
        }
/*
 * Recycle the buffer back into the ISR alloc ring
 */
/* Buffer now owned by the CHIP. Set CHIP_OWNS, reset CPU_OWNS */
        poolEntryPtr->poolBufFlags |= TM_POOL_BUF_CHIP_OWNS;
        tm_16bit_clr_bit( poolEntryPtr->poolBufFlags, TM_POOL_BUF_CPU_OWNS );
        if ( (tt16Bit) dataSize <=
             poolHeadPtr->headAllocBufferSize[TM_POOL_SMALL_INDEX])
        {
            poolIndex = TM_POOL_SMALL_INDEX;
        }
        else
        {
            poolIndex = TM_POOL_FULL_INDEX;
        }
        slotCount = poolHeadPtr->headAllocSlots[poolIndex];
        isrIndex = poolHeadPtr->headAllocIsrIndex[poolIndex];
        if (isrIndex > 0)
        {
            isrIndex--;
        }
        else
        {
            isrIndex = slotCount - 1;
        }
        poolHeadPtr->headAllocIsrIndex[poolIndex] = (tt16Bit)isrIndex;
#ifdef TM_USE_INDRT
        if (    (devEntryPtr->devIndrtPosRecvIsr == devEntryPtr->devRecvIsrCount)
/* Following condition should always be true: */
             && (devEntryPtr->devRecvIsrCount != 0) )
/*
 * If we are freeing an INDRT Packet, make sure tfRecvInterface() will
 * not free it twice
 */
        {
            devEntryPtr->devIndrtPosRecvIsr = 0;
            devEntryPtr->devRecvIsrCount--;
        }
#endif /* TM_USE_INDRT */
    }
}
