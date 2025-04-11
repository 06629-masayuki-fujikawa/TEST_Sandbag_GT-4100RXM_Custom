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
 * Description: Simple heap allocation
 *
 * Filename: trsheap.c
 * Author: Odile
 * Date Created: 7/01/98
 * $Source: source/trsheap.c $
 *
 * Modification History
 * $Revision: 6.0.2.4 $
 * $Date: 2010/04/02 03:02:51JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */ 

#include <trsocket.h>

#ifdef TM_USE_SHEAP
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/* local functions */
static ttRawMemoryPtr tfSheapGetFreeListBlock(ttRawMemoryPtrPtr listPtrPtr, 
                                              tt32Bit           sizeLongs);
static void tfSheapInsertInFreeList(ttRawMemoryPtr memoryBlockPtr);

/* local macros */
#define TM_HIGH_BIT_32BIT_SET TM_UL(0x80000000)

#if TM_SHEAP_NUM_PAGE > 1
#define TM_SHEAP_PAGE_BOUNDARY
#endif /* TM_SHEAP_NUM_PAGE > 1 */

#ifdef TM_ERROR_CHECKING
/* 
 * add room (2 longs == 8-bytes) to store the size of the allocated block 
 * before and after 
 */
#define TM_SHEAP_ALLOC_OVERHEAD 2
#else /* ! TM_ERROR_CHECKING */
/* 
 * add room (1 long == 4-bytes) to store the size of the allocated block before.
 */
#define TM_SHEAP_ALLOC_OVERHEAD 1
#endif /* !TM_ERROR_CHECKING */

#ifndef TM_SHEAP_MIN_BLOCK_SIZE
/* in bytes */
#ifdef TM_ERROR_CHECKING
#define TM_SHEAP_MIN_BLOCK_SIZE 8
#else /* !TM_ERROR_CHECKING */
#define TM_SHEAP_MIN_BLOCK_SIZE 4
#endif /* !TM_ERROR_CHECKING */
#endif /* TM_SHEAP_MIN_BLOCK_SIZE */

/* If user did not define either FIRST or BEST fit, pick best fit */
#ifndef TM_SHEAP_FIRST_FIT
#ifndef TM_SHEAP_BEST_FIT
#define TM_SHEAP_BEST_FIT
#endif /* !TM_SHEAP_BEST_FIT */
#endif /* !TM_SHEAP_FIRST_FIT */

/* If user defined both FIRST or BEST fit, pick best fit */
#ifdef TM_SHEAP_FIRST_FIT
#ifdef TM_SHEAP_BEST_FIT
#undef TM_SHEAP_FIRST_FIT
#endif /* TM_SHEAP_BEST_FIT */
#endif /* TM_SHEAP_FIRST_FIT */

#define TM_8BIT_AFTER  (tt8Bit)1
#define TM_8BIT_BEFORE (tt8Bit)2

#define tm_sheap_size_to_queue_ptr(sizePtr) \
    (ttRawMemoryPtr)(ttVoidPtr)(sizePtr + 1)
#define tm_sheap_queue_to_size_ptr(memoryBlockPtr) \
    ((tt32BitPtr)(((tt32BitPtr)(ttVoidPtr)memoryBlockPtr) - 1))
#ifdef TM_ERROR_CHECKING
#define tm_sheap_queue_to_end_size_ptr(memoryBlockPtr, longs) \
    ((tt32BitPtr)(((tt32BitPtr)(ttVoidPtr)memoryBlockPtr) + longs))
#define tm_sheap_queue_to_end_size(memoryBlockPtr, longs) \
    (*(tm_sheap_queue_to_end_size_ptr(memoryBlockPtr, longs)))
#define tm_sheap_size_to_end_size_ptr(sizePtr, longs) \
    (sizePtr + longs + 1)
#define tm_sheap_end_size(sizePtr, longs) \
    (*(tm_sheap_size_to_end_size_ptr(sizePtr, longs)))
#endif /* TM_ERROR_CHECKING */
            
/*
 * Simple heap
 */
#ifdef TM_DYNAMIC_CREATE_SHEAP
/* Array of pointers to pages */
tt32BitPtr tlSheap[TM_SHEAP_NUM_PAGE];
#else /* !TM_DYNAMIC_CREATE_SHEAP */
static tt32Bit tlSheap[TM_SHEAP_NUM_PAGE]
                           [TM_SHEAP_SIZE/sizeof(tt32Bit)/TM_SHEAP_NUM_PAGE]; 
#endif /* !TM_DYNAMIC_CREATE_SHEAP */

/*
 * Use this module when an RTOS does not provide a heap.
 *
 * To limit fragmentation, we have two lists:
 *  + a list of "big" buffers, the free list. It contains free pages, and 
 *    buffers bigger than 4k+4 bytes that have been previously used and freed.
 *    This list is referenced by tvSheapFreeListPtr.
 *  + a list of "fragmented" buffers, containing buffers smaller than 4k+4
 *    bytes available for use. A buffer in the fragmented buffers list, is 
 *    a block of memory smaller than or equal to 4k+4 located at the end of an
 *    allocated block that was left over from a buffer allocation.
 *    This list is referenced by tvSheapFragmentListPtr.
 *
 * At start up all newly created pages are queued to the free list.
 *
 */

/* 
 * Create the simple heap. If TM_DYNAMIC_CREATE_SHEAP is #define'd, the
 * simple heap is dynamically allocated by the user, otherwise it is
 * implemented as a static array. Queue all the pages to the free list,
 * in order of allocation.
 */
tt32BitPtr tfSheapCreate(void)
{
    tt32BitPtr sheapPtr;
#ifdef TM_DYNAMIC_CREATE_SHEAP 
    tt32Bit     pageSize;
#endif /* TM_DYNAMIC_CREATE_SHEAP */
    tt32Bit     freeLongs;
    int         i;

#ifdef TM_DYNAMIC_CREATE_SHEAP 
/* get the pages */
    pageSize = tm_global(tvSheapPageSize);
    i = 0;
#ifdef TM_SHEAP_PAGE_BOUNDARY
    for ( ; i < TM_SHEAP_NUM_PAGE; i++)
#endif /* TM_SHEAP_PAGE_BOUNDARY */
    {
        sheapPtr = tfKernelSheapCreate(&pageSize);
        if (sheapPtr == TM_32BIT_NULL_PTR)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelError( "tfSheapCreate",
       "Failed creating simple heap, Try undefining TM_DYNAMIC_CREATE_SHEAP.");
#endif /* TM_ERROR_CHECKING */
/* Severe Error */
            tm_thread_stop;
        }
        if ((((ttUserPtrCastToInt) sheapPtr) &
             (ttUserPtrCastToInt)(sizeof(tt32Bit) - 1)) != 0)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning( "tfSheapCreate",
                  "Start of page must be aligned on a 32-bit boundary." );
#endif /* TM_ERROR_CHECKING */
            tm_thread_stop; /* Severe Error */
        }
#ifdef TM_SHEAP_PAGE_BOUNDARY
        if (i != 0)
        {
            if (pageSize != tm_global(tvSheapPageSize))
            {
#ifdef TM_ERROR_CHECKING
                tfKernelError("tfSheapCreate",
                      "tfKernelSheapCreate() must create equal-sized pages");
#endif /* TM_ERROR_CHECKING */
                sheapPtr = TM_32BIT_NULL_PTR;
                tm_thread_stop; /* Severe Error */
            }
        }
        else
#endif /* TM_SHEAP_PAGE_BOUNDARY */
        {
/* Update simple heap page size */
            tm_global(tvSheapPageSize) = pageSize;
        }
        tlSheap[i] = sheapPtr;
    }
#endif /* TM_DYNAMIC_CREATE_SHEAP */

#ifndef TM_SHEAP_PAGE_BOUNDARY
    i = 0;
#else /* TM_SHEAP_PAGE_BOUNDARY */
/* Loop backwards so that first page end up at the beginning of the free list */
    for (i = (TM_SHEAP_NUM_PAGE - 1); i >= 0; i--)
#endif /* TM_SHEAP_PAGE_BOUNDARY */
    {
        sheapPtr = tlSheap[i];
        freeLongs = (tm_global(tvSheapPageSize) / sizeof(tt32Bit))
                    - TM_SHEAP_ALLOC_OVERHEAD;
#ifdef TM_SHEAP_MARK_MEMORY
/* Mark memory that has never been allocated */
        tm_memset((ttCharPtr)(ttVoidPtr)(sheapPtr + 1), 0xFE,
                  freeLongs * sizeof(tt32Bit));
#endif /* TM_SHEAP_MARK_MEMORY */
/* Free block size */
        *sheapPtr = freeLongs; 
#ifdef TM_ERROR_CHECKING
/* Mark end of free block */
        tm_sheap_end_size(sheapPtr, freeLongs) = freeLongs;
#endif /* TM_ERROR_CHECKING */
/* Put each page in free list */
        tfSheapInsertInFreeList(tm_sheap_size_to_queue_ptr(sheapPtr));
    }
#ifdef TM_SHEAP_PAGE_BOUNDARY
    sheapPtr = tlSheap[0]; /* compiler warning */
#endif /* TM_SHEAP_PAGE_BOUNDARY */
    return sheapPtr;
}

/*
 * Allocate a memory block of size from the kernel
 */
/* 
 * How tfSheapMalloc works:
 * 1. If the block size is greater that 4k+4 bytes, try to take the buffer
 *    from the free list (list of blocks of size > 4k+4). 
 *      #ifdef TM_SHEAP_OPTIMIZE_FREE_SPEED
 *      If there is nothing big enough in this list, try and concatenate all
 *      freed blocks in the heap and insert all free marked blocks in the heap 
 *      back into the appropriate lists, and try one more time the search 
 *      on the free list.
 *      #endif TM_SHEAP_OPTIMIZE_FREE_SPEED
 * 2. If the block is smaller than 4k+4 bytes, try to get it first
 *    from the fragment list (list of blocks of size <= 4k+4). If there is 
 *    nothing big enough in this list, try try to tap into the free buffer 
 *    list. 
 *      #ifdef TM_SHEAP_OPTIMIZE_FREE_SPEED
 *      If there is nothing available there either, try and insert all free
 *      marked blocks in the heap back into the appropriate lists, and try one
 *      more time to search the 2 lists in the same order.
 *      #endif TM_SHEAP_OPTIMIZE_FREE_SPEED
 * 3. Blocks taken from the fragment list or the free list are re-fragmented
 *    when possible and the remaining free block is inserted into the 
 *    appropriate queue (free list if bigger than 4k+4, fragment list
 *    otherwise).
 * 4. If TM_USE_DYNAMIC_MEMORY is used, Treck will not free 4096(+4) 
 *    (and under) bytes blocks. It instead recycles them internally. 
 *    However when there is no more memory, (because the memory is too 
 *    fragmented), then Treck will free all its recycled buffers back
 *    to the simple heap, i.e. if tfSheapMalloc() fails, 
 *    tfBufferDoubleMalloc() calls tfFreeDynamicMemory() that will attempt to
 *    mark all freed blocks in the heap in one swoop trying to concatenate 
 *    adjacent blocks, and then insert them in the free lists.
 */
ttVoidPtr tfSheapMalloc(unsigned int size)
{
    ttRawMemoryPtrPtr listPtrPtr;
    tt32Bit           freeBufLongs;
    tt32Bit           allocLongs;
    tt32Bit           sizeLongs;
    tt32BitPtr        memorySizePtr;
    ttRawMemoryPtr    memoryBlockPtr;
    ttRawMemoryPtr    memLeftFromPagePtr;
#ifdef TM_SHEAP_OPTIMIZE_FREE_SPEED
    ttVoidPtr         nullMemPtr;
#endif /* TM_SHEAP_OPTIMIZE_FREE_SPEED */
    int               tries;
    int               i;
#ifdef TM_SHEAP_OPTIMIZE_FREE_SPEED
    int               concatenateBlocks;
#endif /* TM_SHEAP_OPTIMIZE_FREE_SPEED */

    tm_memory_log1("SHEAP ALLOC %u\n", size);

    freeBufLongs = (tt32Bit)0;
    tm_lock_wait_no_log(&tm_global(tvSheapLock));
    memoryBlockPtr = (ttRawMemoryPtr)0;
    memorySizePtr = (tt32BitPtr)0;
/* Simple heap will allocate at least TM_SHEAP_MIN_BLOCK_SIZE */
#if TM_SHEAP_MIN_BLOCK_SIZE > 4 
    if (size < TM_SHEAP_MIN_BLOCK_SIZE)
    {
        size = TM_SHEAP_MIN_BLOCK_SIZE;
    }
#endif /* TM_SHEAP_MIN_BLOCK_SIZE */

#ifdef TM_DSP
    sizeLongs = (size + sizeof(tt32Bit) - 1) / sizeof(tt32Bit);
#else /* TM_DSP */
    if ((size & TM_ROUND_MASK_SIZE) != size)
    {
/* 
 * Round up size to a multiple of 4 bytes, if not already so. 
 */
        size = (unsigned)( (size + TM_ROUND_SIZE) & TM_ROUND_MASK_SIZE );
    }

/*
 * Convert number of bytes to number of longs.
 */
    sizeLongs = size / sizeof(tt32Bit);
#endif /* TM_DSP */

/*
 * Number of longs to be allocated plus overhead to store the size.
 */
    allocLongs = (tt32Bit)(sizeLongs + TM_SHEAP_ALLOC_OVERHEAD);
    if ( sizeLongs > ((TM_BUF_Q6_SIZE / sizeof(tt32Bit)) + 1) )
    {
/*
 * If size is greater than 4096 (+TM_MEM_BLOCK_ALLOC_OVERHEAD), try and find
 * a free buffer from the simple heap list of free buffers (they all have a
 * size bigger than 4096 (+TM_MEM_BLOCK_ALLOC_OVERHEAD) bytes in that free 
 * list). Try and find a best fit.
 */
        tries = 1;
        listPtrPtr = &tm_global(tvSheapFreeListPtr);
    }
    else
    {
/*
 * Size is lower than 4K + 4. First, try to get a block from the
 * fragmented list.
 */
        tries = 2;
        listPtrPtr = &tm_global(tvSheapFragmentListPtr);
    }
#ifdef TM_SHEAP_OPTIMIZE_FREE_SPEED
    concatenateBlocks = 2;
    do
#endif /* TM_SHEAP_OPTIMIZE_FREE_SPEED */
    {
        for (i = 0; i < tries; i++)
        {
            memoryBlockPtr = tfSheapGetFreeListBlock(listPtrPtr, sizeLongs);
            if (memoryBlockPtr != (ttRawMemoryPtr)0)
            {
/* Try to fragment it and  put the data back into the fragment
 * list if possible.
 */
                memorySizePtr = tm_sheap_queue_to_size_ptr(memoryBlockPtr);
                freeBufLongs = *memorySizePtr;
                if (freeBufLongs > (allocLongs))
                {
                    freeBufLongs -= allocLongs;
#if TM_SHEAP_MIN_BLOCK_SIZE > 4 
                    if (freeBufLongs >=
                                    TM_SHEAP_MIN_BLOCK_SIZE / sizeof(tt32Bit))
#endif /* TM_SHEAP_MIN_BLOCK_SIZE */
                    {
/* Write the new size of the buffer (not including space to store the size) */
                        *memorySizePtr = sizeLongs;
#ifdef TM_ERROR_CHECKING
                        tm_sheap_end_size(memorySizePtr, sizeLongs) = sizeLongs;
#endif /* TM_ERROR_CHECKING */
/* Release left over buffer */
                        memLeftFromPagePtr = (ttRawMemoryPtr)( 
                                         (tt32BitPtr)(ttVoidPtr)memoryBlockPtr
                                       + allocLongs );
                        *(tm_sheap_queue_to_size_ptr(memLeftFromPagePtr)) = 
                                                                  freeBufLongs;
#ifdef TM_ERROR_CHECKING
                        tm_sheap_queue_to_end_size(memLeftFromPagePtr,
                                                   freeBufLongs) = 
                                                                  freeBufLongs;
#endif /* TM_ERROR_CHECKING */
                        tfSheapInsertInFreeList(memLeftFromPagePtr);
                    }
                }
                break;
            }
            listPtrPtr = &tm_global(tvSheapFreeListPtr);
        }
#ifdef TM_SHEAP_OPTIMIZE_FREE_SPEED
/* 
 * Call tfSheapCollect(). It will try and concatenate all adjacent freed 
 * blocks, and put them on the free or fragment list.
 */
        if (memoryBlockPtr != (ttRawMemoryPtr)0)
        {
            concatenateBlocks = 0;
        }
        else
        {
            if (concatenateBlocks > 1)
            {
/* null Pointer to pass to tfSheapCollect() (in lieu of recycled lists) */
                nullMemPtr = (ttVoidPtr)0;
                tfSheapCollect( 
#ifdef TM_USE_DYNAMIC_MEMORY
                                  (ttRawMemoryPtrPtr)&nullMemPtr
                                , (ttRcylPtrPtr)&nullMemPtr
#ifdef TM_LOCK_NEEDED
                                , TM_8BIT_ZERO /* already locked */
#endif /* TM_LOCK_NEEDED */
#else /* !TM_USE_DYNAMIC_MEMORY */
#ifdef TM_LOCK_NEEDED
                                  TM_8BIT_ZERO /* already locked */
#endif /* TM_LOCK_NEEDED */
#endif /* !TM_USE_DYNAMIC_MEMORY */
                              );
            }
            concatenateBlocks--;
        }
#endif /* TM_SHEAP_OPTIMIZE_FREE_SPEED */
    }
#ifdef TM_SHEAP_OPTIMIZE_FREE_SPEED
    while (concatenateBlocks != 0);
#endif /* TM_SHEAP_OPTIMIZE_FREE_SPEED */
    if (memorySizePtr != (tt32BitPtr)0)
    {
        tm_global(tvSheapUsedLongs) += *memorySizePtr + TM_SHEAP_ALLOC_OVERHEAD;
        tm_assert(tvSheapMalloc, 
         tm_global(tvSheapUsedLongs) 
      <= (TM_SHEAP_NUM_PAGE * tm_global(tvSheapPageSize)) / sizeof (tt32Bit));
    }
#ifdef TM_ERROR_CHECKING
    else
    {
/*
 * Could not find any buffers in the lists. 
 * is the requested size greater than a page size?
 */
        if (allocLongs > (tm_global(tvSheapPageSize) / sizeof(tt32Bit)))
        {
            tfKernelError("tfSheapMalloc",
                    "size requested too high. Increase the memory page size.");
        }
#ifndef TM_USE_DYNAMIC_MEMORY
        else
        {
            tfKernelWarning("tfSheapMalloc","Ran out of Simple Heap");
        }
#endif /* !TM_USE_DYNAMIC_MEMORY */
    }
#endif /* TM_ERROR_CHECKING */
    tm_unlock_no_log(&tm_global(tvSheapLock));
#ifdef TM_SHEAP_MARK_MEMORY
/* Mark allocated memory */
    if (memoryBlockPtr != (ttRawMemoryPtr)0)
    {
        tm_memset((ttCharPtr)(ttVoidPtr)memoryBlockPtr, 0xA0, size);
    }
#endif /* TM_SHEAP_MARK_MEMORY */
    tm_memory_log2("SHEAP ALLOC %u 0x%x\n", size, memoryBlockPtr);
    return ( (ttVoidPtr)memoryBlockPtr );
}


/*
 * Free a block of memory:
 * Decrease number of used bytes.
 * #ifdef TM_SHEAP_OPTIMIZE_FREE_SPEED
 * Mark the memory as freed, so that it can be concatenated
 * later on, and put back in a one of the 2 free list of buffers, when
 * the heap allocator runs out of memory to allocate from the free list, and 
 * from the fragment list.
 * #else !TM_SHEAP_OPTIMIZE_FREE_SPEED
 * Search the fragment list, and the free list to find an adjacent block
 * that has already been freed, and concatenate to a bigger block.
 * If memory block size is above 4096(+4) put it in the free list of
 * blocks of size bigger than 4096 (+4), otherwise put it in the fragment
 * list.
 * #endif !TM_SHEAP_OPTIMIZE_FREE_SPEED
 */
void tfSheapFree(ttVoidPtr blockPtr)
{
#ifdef TM_SHEAP_OPTIMIZE_FREE_SPEED
    tt32BitPtr  memorySizePtr;
    tt32Bit     freeLongs;
    tt32Bit     returnedLongs;

/* Pointer to size of block */
    memorySizePtr = tm_sheap_queue_to_size_ptr(blockPtr); 
    freeLongs = *memorySizePtr;
#ifdef TM_ERROR_CHECKING
/* Check that sizes before and after the block match */
    tm_assert(tfSheapFree, 
              freeLongs == tm_sheap_end_size(memorySizePtr, freeLongs));
#endif /* TM_ERROR_CHECKING */
#ifdef TM_SHEAP_MARK_MEMORY
/* Mark freed memory that had been previously allocated */
    tm_memset(blockPtr, 0xFA, freeLongs * sizeof(tt32Bit));
#endif /* TM_SHEAP_MARK_MEMORY */
    returnedLongs = freeLongs + TM_SHEAP_ALLOC_OVERHEAD;
    tm_lock_wait_no_log(&tm_global(tvSheapLock));
    tm_assert(tvSheapFree, 
                      returnedLongs
                  <=  tm_global(tvSheapUsedLongs));
/* Decrease the number of used bytes */
    tm_global(tvSheapUsedLongs) -= returnedLongs;
/* Mark memory as freed */
    *memorySizePtr = TM_HIGH_BIT_32BIT_SET | freeLongs;
    tm_unlock_no_log(&tm_global(tvSheapLock));
    return;
#else /* !TM_SHEAP_OPTIMIZE_FREE_SPEED */
    ttRawMemoryPtr    memoryBlockPtr;
    tt32BitPtr        memorySizePtr;
    ttRawMemoryPtr    prevMemoryBlockPtr;
    ttRawMemoryPtr    nextMemoryBlockPtr;
    tt32BitPtr        afterSizePtr;
    ttRawMemoryPtr    afterMemoryBlockPtr;
    ttRawMemoryPtrPtr listPtrPtr;
    tt32Bit           returnedLongs;
    tt32Bit           freeLongs;
    tt32Bit           loopNumberOfLongs;
#ifdef TM_ERROR_CHECKING
    tt32Bit           nextNumberOfLongs; 
#endif /* TM_ERROR_CHECKING */
    int               i;
    tt8Bit            concatMatch;
    tt8Bit            concatCheck;

    tm_memory_log1("SHEAP FREE 0x%x\n", blockPtr);

    memoryBlockPtr = (ttRawMemoryPtr)blockPtr;
/* Pointer to size of block */
    memorySizePtr = tm_sheap_queue_to_size_ptr(memoryBlockPtr); 
    freeLongs = *memorySizePtr;
#ifdef TM_ERROR_CHECKING
/* Check that sizes before and after the block match */
    tm_assert(tfSheapFree, 
              freeLongs == tm_sheap_end_size(memorySizePtr, freeLongs));
#endif /* TM_ERROR_CHECKING */
#ifdef TM_SHEAP_MARK_MEMORY
/* Mark freed memory that had been previously allocated */
    tm_memset(blockPtr, 0xFA, freeLongs * sizeof(tt32Bit));  
#endif /* TM_SHEAP_MARK_MEMORY */
    returnedLongs = (freeLongs + TM_SHEAP_ALLOC_OVERHEAD);
/* First try and concatenate to a free block to make a bigger block */
    afterSizePtr = memorySizePtr + returnedLongs;
    concatMatch = TM_8BIT_ZERO; /* no match yet */
/* check for a free block before and after our block being freed */
    concatCheck = TM_8BIT_AFTER | TM_8BIT_BEFORE; 
    tm_lock_wait_no_log(&tm_global(tvSheapLock));
    tm_assert(tvSheapFree,    (returnedLongs)
                           <= tm_global(tvSheapUsedLongs) );
    tm_global(tvSheapUsedLongs) -= returnedLongs;
    listPtrPtr = &tm_global(tvSheapFreeListPtr);
#ifdef TM_SHEAP_PAGE_BOUNDARY
    for (i = 0; (i < TM_SHEAP_NUM_PAGE) && (concatCheck); i++)
    {
        if (tlSheap[i] == memorySizePtr)
        {
/* 
 * Do not concatenate with a block before our block
 * (because we are at the beginning of a page) 
 */
            /* at beginning of page */
            tm_8bit_clr_bit( concatCheck, TM_8BIT_BEFORE );
        }
        if (tlSheap[i] + (tm_global(tvSheapPageSize) / sizeof(tt32Bit)) ==
                                                                 afterSizePtr)
/* Do nt assume that pages are contiguous */
        {
/* 
 * Do not concatenate with a block after our block
 * (because we are at the end of a page) 
 */
            /* at end of page */
            tm_8bit_clr_bit( concatCheck, TM_8BIT_AFTER );
        }
    }
#endif /* TM_SHEAP_PAGE_BOUNDARY */
    i = 0;
/* point to memory block (past the size) */
    afterMemoryBlockPtr = tm_sheap_size_to_queue_ptr(afterSizePtr);
#ifdef TM_SHEAP_PAGE_BOUNDARY
    if (concatCheck)
#endif /* TM_SHEAP_PAGE_BOUNDARY */
    {
        do
        {
            nextMemoryBlockPtr = *listPtrPtr;
            prevMemoryBlockPtr = (ttRawMemoryPtr)0;
            while (    (nextMemoryBlockPtr != (ttRawMemoryPtr)0)
                    && (concatCheck) )
            {
#ifdef TM_ERROR_CHECKING
                nextNumberOfLongs = 
                         *(tm_sheap_queue_to_size_ptr(nextMemoryBlockPtr));
                tm_assert(tfSheapFree, 
                  nextNumberOfLongs == 
                         tm_sheap_queue_to_end_size(nextMemoryBlockPtr,
                                                    nextNumberOfLongs));
#endif /* TM_ERROR_CHECKING */
                if (concatCheck & TM_8BIT_AFTER)
                {
/* Try and concatenate with a block after our block */
                    if (nextMemoryBlockPtr == afterMemoryBlockPtr)
                    {
/* Update size here (because of the before match below) */
                        freeLongs =   
                            freeLongs
                           + *(tm_sheap_queue_to_size_ptr(nextMemoryBlockPtr))
                           + TM_SHEAP_ALLOC_OVERHEAD;
                        *memorySizePtr = freeLongs;
#ifdef TM_ERROR_CHECKING
                        tm_sheap_end_size(memorySizePtr, freeLongs) = 
                                                                    freeLongs;
#endif /* TM_ERROR_CHECKING */
/* Can only find one match after our block. No need to check anymore */
                        concatCheck &= ~TM_8BIT_AFTER;
/* block removed from the free list below */
                        concatMatch |= TM_8BIT_AFTER; /* match */
                    }
                }
                if (concatCheck & TM_8BIT_BEFORE)
                {
/* Try and concatenate with a block before our block */
                    loopNumberOfLongs = 
                            *(tm_sheap_queue_to_size_ptr(nextMemoryBlockPtr))
                          + TM_SHEAP_ALLOC_OVERHEAD;
                    if ( (ttRawMemoryPtr)(
                                     (tt32BitPtr)(ttVoidPtr)nextMemoryBlockPtr 
                                   + loopNumberOfLongs )
                      == memoryBlockPtr )
                    {
/* Update size, before updating the pointer */
                        freeLongs = loopNumberOfLongs + freeLongs;
/* Point to new beginning of block. */
                        memoryBlockPtr = nextMemoryBlockPtr;
                        memorySizePtr = 
                                tm_sheap_queue_to_size_ptr(memoryBlockPtr);
/* Upate size of new block */
                        *(memorySizePtr) = freeLongs;
#ifdef TM_ERROR_CHECKING
                        tm_sheap_end_size(memorySizePtr, freeLongs) = freeLongs;
#endif /* TM_ERROR_CHECKING */
/* Can only find one match before our block. No need to check anymore */
                        tm_8bit_clr_bit( concatCheck, TM_8BIT_BEFORE );
/* block removed from the free list below */
                        concatMatch |= TM_8BIT_BEFORE; /* match */
                    }
                }
                if (concatMatch)
                {
/* Common code for a match: remove a concatenated block from the free list */
                    if (prevMemoryBlockPtr == (ttRawMemoryPtr)0)
                    {
                        *listPtrPtr = nextMemoryBlockPtr->rawNextPtr;
                    }
                    else
                    {
                        prevMemoryBlockPtr->rawNextPtr =
                                  nextMemoryBlockPtr->rawNextPtr;
                    }
/* Reset for next check in the loop */
                    concatMatch = TM_8BIT_ZERO;
                }
                else
                {
                    prevMemoryBlockPtr = nextMemoryBlockPtr;
                }
                nextMemoryBlockPtr = nextMemoryBlockPtr->rawNextPtr;
            }
            i++;
            listPtrPtr = &tm_global(tvSheapFragmentListPtr);
        } while (i < 2);
    }
/* end of trying to make a bigger block */
/* Queue to free list */
    tfSheapInsertInFreeList(memoryBlockPtr);
    tm_unlock_no_log(&tm_global(tvSheapLock));
    return;
#endif /* TM_SHEAP_OPTIMIZE_FREE_SPEED */
}

/*
 * Insert a block that is being freed in the appropriate list 
 * (free list for blocks > 4k+4, fragment list for blocks <=  4k+4).
 */
static void tfSheapInsertInFreeList(ttRawMemoryPtr memoryBlockPtr)
{
    ttRawMemoryPtrPtr   listPtrPtr;
    ttRawMemoryPtr      firstMemoryBlockPtr;

    if ( (*(tm_sheap_queue_to_size_ptr(memoryBlockPtr))) > 
                        ((TM_BUF_Q6_SIZE / sizeof(tt32Bit)) + 1) )
    {
/*
 * If size is greater than 4096 (+TM_MEM_BLOCK_ALLOC_OVERHEAD), insert
 * free buffer in the simple heap list of free buffers (they all have a
 * size bigger than 4096 (+TM_MEM_BLOCK_ALLOC_OVERHEAD) bytes in that free 
 * list).
 */
        listPtrPtr = &tm_global(tvSheapFreeListPtr);
    }
    else
    {
        listPtrPtr = &tm_global(tvSheapFragmentListPtr);
    }
    firstMemoryBlockPtr = *listPtrPtr;
    *listPtrPtr = memoryBlockPtr;
    memoryBlockPtr->rawNextPtr = firstMemoryBlockPtr;
    return;
}

static ttRawMemoryPtr tfSheapGetFreeListBlock(
                            ttRawMemoryPtrPtr listPtr, tt32Bit sizeLongs)
{
#ifdef TM_SHEAP_FIRST_FIT
    ttRawMemoryPtr  memoryBlockPtr;
    ttRawMemoryPtr  prevMemoryBlockPtr;
    ttRawMemoryPtr  firstFitMemoryBlockPtr;
    tt32Bit         freeLongs;

    firstFitMemoryBlockPtr = (ttRawMemoryPtr)0; /* return value */
/* Not needed (for compiler warning) */
    prevMemoryBlockPtr = (ttRawMemoryPtr)0;
/*
 * Pointer to first buffer in the free list of buffers. 
 */
    memoryBlockPtr = *listPtr;
    if ( memoryBlockPtr != (ttRawMemoryPtr)0 )
/*
 * If there is any buffer in the free list of buffers.
 */
    {
        prevMemoryBlockPtr = (ttRawMemoryPtr)0;
        do
        {
/* Size of current buffer in the passed simple heap free list */
            freeLongs = *(tm_sheap_queue_to_size_ptr(memoryBlockPtr));
#ifdef TM_ERROR_CHECKING
            tm_assert(tfSheapGetFreeListBlock,
                        (freeLongs) ==
                        tm_sheap_queue_to_end_size(memoryBlockPtr, freeLongs));
#endif /* TM_ERROR_CHECKING */
            if ( sizeLongs <= freeLongs )
/* If size does not exceed current free buffer size */
            {
                firstFitMemoryBlockPtr = memoryBlockPtr;
/* First match, no need to look any further */
                if (prevMemoryBlockPtr == (ttRawMemoryPtr)0)
                {
                    *listPtr = firstFitMemoryBlockPtr->rawNextPtr;
                }
                else
                {
                    prevMemoryBlockPtr->rawNextPtr = 
                                 firstFitMemoryBlockPtr->rawNextPtr;
                }
                break;
            }
            prevMemoryBlockPtr = memoryBlockPtr;
            memoryBlockPtr     = memoryBlockPtr->rawNextPtr;
        }
        while (memoryBlockPtr != (ttRawMemoryPtr)0);
    }
    return firstFitMemoryBlockPtr;
#endif /* TM_SHEAP_FIRST_FIT */
#ifdef TM_SHEAP_BEST_FIT
    ttRawMemoryPtr  memoryBlockPtr;
    ttRawMemoryPtr  prevMemoryBlockPtr;
    ttRawMemoryPtr  bestFitMemoryBlockPtr;
    ttRawMemoryPtr  bestFitPrevMemoryBlockPtr;
    tt32Bit         freeLongs;
    tt32Bit         bestFitLongs;

    bestFitMemoryBlockPtr = (ttRawMemoryPtr)0; /* return value */
/* Not needed (for compiler warning) */
    bestFitPrevMemoryBlockPtr = (ttRawMemoryPtr)0;
    bestFitLongs = TM_UL(0);
/*
 * Pointer to first buffer in the passed simple heap free list 
 */
    memoryBlockPtr = *listPtr;
    if ( memoryBlockPtr != (ttRawMemoryPtr)0 )
/*
 * If there is any buffer in the passed simple free list.
 */
    {
        prevMemoryBlockPtr = (ttRawMemoryPtr)0;
        do
        {
/* Size of current buffer in the simple heap free list */
            freeLongs = *(tm_sheap_queue_to_size_ptr(memoryBlockPtr));
#ifdef TM_ERROR_CHECKING
            tm_assert(tfSheapGetFreeListBlock,
                        (freeLongs) ==
                        tm_sheap_queue_to_end_size(memoryBlockPtr, freeLongs));
#endif /* TM_ERROR_CHECKING */
            if ( sizeLongs <= freeLongs )
/* If size does not exceed current free buffer size */
            {
                if ( (freeLongs < bestFitLongs) || (bestFitLongs == TM_UL(0)) )
/* check whether this is the best fit so far */
                {
                    bestFitLongs = freeLongs; /* new Best Fit size */
                    bestFitMemoryBlockPtr = memoryBlockPtr;
                    bestFitPrevMemoryBlockPtr = prevMemoryBlockPtr;
                    if (sizeLongs == freeLongs)
                    {
/* If we match exactly, no need to look any further */
                        break;
                    }
                }
            }
            prevMemoryBlockPtr = memoryBlockPtr;
            memoryBlockPtr     = memoryBlockPtr->rawNextPtr;
        }
        while (memoryBlockPtr != (ttRawMemoryPtr)0);

        if ( bestFitLongs != TM_UL(0) )
        {
            if (bestFitPrevMemoryBlockPtr == (ttRawMemoryPtr)0)
            {
                *listPtr = bestFitMemoryBlockPtr->rawNextPtr;
            }
            else
            {
                bestFitPrevMemoryBlockPtr->rawNextPtr =
                           bestFitMemoryBlockPtr->rawNextPtr;
            }
        }
    }
    return bestFitMemoryBlockPtr;
#endif /* TM_SHEAP_BEST_FIT */
}

/*
 * Empty the lists pointed to by rawMemListPtr, rcylListPtr, simple heap
 * free list, and simple heap fragment list, marking each block in
 * those lists as freed (by setting the high bit in the size field).
 * Then search the entire simple heap for free blocks, concatenating
 * adjacent blocks, and inserting the free blocks in the appropriate 
 * lists (free list for blocks > 4k+4, fragment list for blocks <= 
 * 4k+4).
 */
#ifdef TM_USE_DYNAMIC_MEMORY
void tfSheapCollect(   ttRawMemoryPtrPtr rawMemListPtr
                     , ttRcylPtrPtr      rcylListPtr
#ifdef TM_LOCK_NEEDED
                     , tt8Bit            needLock
#endif /* TM_LOCK_NEEDED */
                   )
#else /* !TM_USE_DYNAMIC_MEMORY */
#ifdef TM_LOCK_NEEDED
TM_PROTO_EXTERN void tfSheapCollect(tt8Bit needLock)
#else /* !TM_LOCK_NEEDED */
TM_PROTO_EXTERN void tfSheapCollect(void)
#endif /* !TM_LOCK_NEEDED */
#endif /* !TM_USE_DYNAMIC_MEMORY */
{
#ifdef TM_USE_DYNAMIC_MEMORY
    ttRawMemoryPtrPtr memListPtr;
    int               offsetToSize;
#endif /* TM_USE_DYNAMIC_MEMORY */
    ttRawMemoryPtrPtr listPtrPtr;
    ttRawMemoryPtr    memoryBlockPtr;
    tt32BitPtr        sizePtr;
    tt32BitPtr        freeSizePtr;
    tt32BitPtr        simpleHeapPtr;
    tt32Bit           heapIndex;
#ifdef TM_SHEAP_PAGE_BOUNDARY
    tt32Bit           heapPageIndex;
#endif /* TM_SHEAP_PAGE_BOUNDARY */
    tt32Bit           sizeLongs;
    tt32Bit           allocLongs;
    tt32Bit           numberOfLongsInPage;
    int               i;

#ifdef TM_LOCK_NEEDED
    if (needLock)
    {
        tm_lock_wait_no_log(&tm_global(tvSheapLock));
    }
#endif /* TM_LOCK_NEEDED */

/* Array of dynamic memory queues */
#ifdef TM_USE_DYNAMIC_MEMORY
    memListPtr = rawMemListPtr; 
    offsetToSize = 2;
#endif /* TM_USE_DYNAMIC_MEMORY */

/* Point to simple heap fragment list */
    listPtrPtr = &tm_global(tvSheapFragmentListPtr);
    for (i = 0; i < 2; i++)
    {
#ifdef TM_USE_DYNAMIC_MEMORY
/* 
 * First collect from dynamic lists 
 * (array of buffer lists in first path (i == 0), array of recycled list in 
 *  second path (i == 1))
 */
        while (*memListPtr != (ttRawMemoryPtr)0)
        {
/* Beginning of a recycled list/memory queue */
            memoryBlockPtr = *memListPtr;
/*
 * Given a list of pointers to memory blocks to free, and an offset
 * to their sizes, mark each block as freed in the simple heap, i.e.
 * set the high bit in the size field.
 */

            while (memoryBlockPtr != (ttRawMemoryPtr)0)
            {
/* Point to beginning of memory block */
                freeSizePtr = (tt32BitPtr)memoryBlockPtr - offsetToSize;
/* Point to next memory block */
                memoryBlockPtr = memoryBlockPtr->rawNextPtr;
                tm_assert(tvSheapCollect, 
                             (*freeSizePtr + TM_SHEAP_ALLOC_OVERHEAD)
                          <= tm_global(tvSheapUsedLongs));
#ifdef TM_ERROR_CHECKING
                tm_assert(tfSheapCollect, 
                          (*freeSizePtr) ==
                             tm_sheap_end_size(freeSizePtr, *freeSizePtr));
#endif /* TM_ERROR_CHECKING */
/* Memory block freed back to the heap */
                tm_global(tvSheapUsedLongs) -= 
                                     (*freeSizePtr) + TM_SHEAP_ALLOC_OVERHEAD; 
/* Mark the freed block */
                *freeSizePtr = TM_HIGH_BIT_32BIT_SET | *freeSizePtr;
            }
/* point to next recycled list/memory queue */
            memListPtr++;
        }
/* Array of dynamic recycled lists */
        memListPtr = (ttRawMemoryPtrPtr)(ttVoidPtr)rcylListPtr;
        offsetToSize = 1;
#endif /* TM_USE_DYNAMIC_MEMORY */
/* 
 * Second collect from simple heap free lists. 
 * (fragment list in first path (i == 0), free list in second path (i == 1))
 */
/* Head of free or fragment list */
        memoryBlockPtr = *listPtrPtr;
        *listPtrPtr = (ttRawMemoryPtr)0; /* empty the free or fragment list */
/*
 * list of freed blocks in the fragment or free list.
 * Mark each block as freed in the simple heap, i.e. set the high bit in 
 * the size field.
 */
        while (memoryBlockPtr != (ttRawMemoryPtr)0)
        {
/* Point to beginning of kernel memory block */
            freeSizePtr = tm_sheap_queue_to_size_ptr(memoryBlockPtr);
/* Point to next, before freeing memory */
            memoryBlockPtr = memoryBlockPtr->rawNextPtr;
            *freeSizePtr = TM_HIGH_BIT_32BIT_SET | *freeSizePtr;
        }
/* Point to heap free list */
        listPtrPtr = &tm_global(tvSheapFreeListPtr);
    }
/* Go through the entire heap and concatenate what we can */
    heapIndex = 0; /* offset in current page */
#ifdef TM_SHEAP_PAGE_BOUNDARY
    heapPageIndex = 0; /* current page */
#endif /* TM_SHEAP_PAGE_BOUNDARY */
/* Start of first page */
    simpleHeapPtr = tlSheap[0];
    freeSizePtr = (tt32BitPtr)0;
    numberOfLongsInPage = tm_global(tvSheapPageSize)/(tt32Bit)sizeof(tt32Bit);
    while (    (heapIndex < numberOfLongsInPage) 
#ifdef TM_SHEAP_PAGE_BOUNDARY
            && (heapPageIndex < TM_SHEAP_NUM_PAGE) 
#endif /* TM_SHEAP_PAGE_BOUNDARY */
          )
    {
        sizePtr = &simpleHeapPtr[heapIndex];
        sizeLongs = *sizePtr; /* point to allocated block */
        if (sizeLongs & TM_HIGH_BIT_32BIT_SET)
        {
            sizeLongs &= ~TM_HIGH_BIT_32BIT_SET;
            *sizePtr = sizeLongs;
#ifdef TM_ERROR_CHECKING
            tm_assert(tfSheapCollect, 
                      (*sizePtr) ==
                         tm_sheap_end_size(sizePtr, sizeLongs));
#endif /* TM_ERROR_CHECKING */
            allocLongs = sizeLongs + TM_SHEAP_ALLOC_OVERHEAD;
            if (freeSizePtr == (tt32BitPtr)0)
            {
/* Start of a free block */
                freeSizePtr = sizePtr;
            }
            else
            {
/* Increase the size of the free block */
/* New size: */
                sizeLongs = allocLongs + *freeSizePtr;
                *(freeSizePtr) = sizeLongs;
#ifdef TM_ERROR_CHECKING
                tm_sheap_end_size(freeSizePtr, sizeLongs) = sizeLongs;
#endif /* TM_ERROR_CHECKING */
            }
        }
        else
        {
            allocLongs = sizeLongs + TM_SHEAP_ALLOC_OVERHEAD;
            if (freeSizePtr != (tt32BitPtr)0)
            {
                tfSheapInsertInFreeList(
                                     tm_sheap_size_to_queue_ptr(freeSizePtr));
/* Start new free block search */
                freeSizePtr = (tt32BitPtr)0;
            }
        }
        heapIndex += allocLongs;
#ifdef TM_SHEAP_PAGE_BOUNDARY
        if ( heapIndex >= numberOfLongsInPage )
        {
            tm_assert(tfSheapCollect, 
                      heapIndex == numberOfLongsInPage);
            if (freeSizePtr != (tt32BitPtr)0)
            {
                tfSheapInsertInFreeList(
                                     tm_sheap_size_to_queue_ptr(freeSizePtr));
                freeSizePtr = (tt32BitPtr)0;
            }
            heapIndex = 0;
            heapPageIndex++;
/* Start of next page */
            simpleHeapPtr = tlSheap[heapPageIndex];
        }
#endif /* TM_SHEAP_PAGE_BOUNDARY */
    }
    if (freeSizePtr != (tt32BitPtr)0)
    {
        tfSheapInsertInFreeList(tm_sheap_size_to_queue_ptr(freeSizePtr));
    }
#ifdef TM_LOCK_NEEDED
    if (needLock)
    {
        tm_unlock_no_log(&tm_global(tvSheapLock));
    }
#endif /* TM_LOCK_NEEDED */
    return;
}

#ifdef TM_USE_STOP_TRECK
void tfSheapDeInit(void)
{
#ifdef TM_ERROR_CHECKING
    ttRawMemoryPtr    memoryBlockPtr;
    tt32Bit           freeLongs;
#endif /* TM_ERROR_CHECKING */
#ifdef TM_DYNAMIC_CREATE_SHEAP 
    int               i;
#endif /* TM_DYNAMIC_CREATE_SHEAP */

#ifdef TM_ERROR_CHECKING
    if (tm_global(tvSheapFragmentListPtr) != (ttRawMemoryPtr)0)
    {
        tfKernelError( "tfSheapDeInit",
       "Freed Memory fragmented!!");
/* Severe Error */
        tm_thread_stop;
    }
    if (tm_global(tvSheapFreeListPtr) != (ttRawMemoryPtr)0)
    {
        freeLongs = 0;
        memoryBlockPtr = tm_global(tvSheapFreeListPtr);
        while (memoryBlockPtr != (ttRawMemoryPtr)0)
        {
            freeLongs += *(tm_sheap_queue_to_size_ptr(memoryBlockPtr));
            memoryBlockPtr = memoryBlockPtr->rawNextPtr;
        }
        if ( freeLongs !=
                ((tm_global(tvSheapPageSize) / sizeof(tt32Bit))
                 - TM_SHEAP_ALLOC_OVERHEAD) * TM_SHEAP_NUM_PAGE )
        {
            tfKernelError( "tfSheapDeInit",
           "Freed Memory in free list subset of all memory");
/* Severe Error */
            tm_thread_stop;
        }
    }
#endif /* TM_ERROR_CHECKING */
    tm_global(tvSheapFreeListPtr) = (ttRawMemoryPtr)0;
    tm_global(tvSheapFragmentListPtr) = (ttRawMemoryPtr)0;
#ifdef TM_DYNAMIC_CREATE_SHEAP 
    i = 0;
#ifdef TM_SHEAP_PAGE_BOUNDARY
    for ( ; i < TM_SHEAP_NUM_PAGE; i++)
#endif /* TM_SHEAP_PAGE_BOUNDARY */
    {
        tfKernelSheapFree(tlSheap[i]);
    }
#endif /* TM_DYNAMIC_CREATE_SHEAP */

    return;
}
#endif /* TMP_USE_STOP_TRECK */
#else /* !TM_USE_SHEAP */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_SHEAP is not defined */
int tlSheapDummy = 0;

#endif /* !TM_USE_SHEAP */
