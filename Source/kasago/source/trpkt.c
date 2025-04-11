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
 * Description: Common packet processing functions
 *
 * Filename: trpkt.c
 * Author: Odile
 * Date Created: 12/04/97
 * $Source: source/trpkt.c $
 *
 * Modification History
 * $Revision: 6.0.2.6 $
 * $Date: 2011/01/23 15:03:31JST $
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

/* constant declarations and enumerations */

/* type definitions */

/* local function prototypes */

/* local variable definitions */

/* static const tables */

/* macro definitions */

#ifndef TM_CUSTOM_OCS
/*
 * tfPacketChecksum() Function Description
 * This function performs checksum on (IP, ICMP, IGMP, UDP and TCP)
 * packets. Note that we cannot assume that data is contiguous. We follow
 * the chain of ttPacket (using the pktNextPtr), and compute the checksum
 * on each contiguous data area. We try and unroll the loop as much as we can.
 * This version handles odd byte lengths in inner links, and odd linkDataPtr.
 *
 * Parameter       Description
 * packetPtr       Pointer to a ttPacket. packetPtr->pktLinkDataPtr
 *                 points to the data to be checksummed.
 * length          Length of data to be checksumed.
 * pshHdrPtr       Pointer to pseudo header
 * pshLen          length of pseudo-header
 * Returns
 * Value   Meaning
 * 16bit one's complement checksum
 */

tt16Bit tfPacketChecksum (
    ttPacketPtr packetPtr,
    ttPktLen    totalDataLength,
    tt16BitPtr  pshHdrPtr,
    tt16Bit     pshLen )
{
    tt16BitPtr      link16BitDataPtr;
    tt32Bit         sum;
    ttPktLen        linkLength;
#ifdef TM_DSP
    unsigned int    offset;
    unsigned int    swapNextSum;
    tt32Bit         linkSum;
#else /* !TM_DSP */
    union
    {
        tt16Bit  oddWord;
        tt8Bit   oddChar[2];
    }               oddByte16Bit;
    tt8Bit          sumByteSwapped;
#endif /* !TM_DSP */
#ifdef TM_DEV_SEND_OFFLOAD
    tt8Bit          partialChecksum;
#endif /* TM_DEV_SEND_OFFLOAD */

    sum = TM_UL(0);
/* 
 * The pseudo header length must be a either 12, or 40, 
 * if pshHdrPtr is non null 
 */
    tm_assert( tfPacketChecksum,
               (    (pshLen == TM_16BIT_ZERO) 
                 || (    (pshHdrPtr != TM_16BIT_NULL_PTR)
                      && ((pshLen == 40) || (pshLen == 12)) ) ) );
/* sum in the pseudo-header, if necessary */
    if (pshLen != 0)
    {
/* unroll the Pseudo header checksum */
/* 6 words (12 bytes) */
#ifdef TM_32BIT_DSP
        sum  = (pshHdrPtr[0] >> 16) & 0xffff; sum += pshHdrPtr[0] & 0xffff;
        sum += (pshHdrPtr[1] >> 16) & 0xffff; sum += pshHdrPtr[1] & 0xffff;
        sum += (pshHdrPtr[2] >> 16) & 0xffff; sum += pshHdrPtr[2] & 0xffff;
#else /* !TM_32BIT_DSP */
/* Should work OK for 16-bit DSP's */
        sum = pshHdrPtr[0]; sum += pshHdrPtr[1]; sum += pshHdrPtr[2];
        sum += pshHdrPtr[3]; sum += pshHdrPtr[4]; sum += pshHdrPtr[5];
#endif /* TM_32BIT_DSP */
#ifdef TM_USE_IPV6
        if (pshLen == 40)
        {
/* 14 extra words (28 bytes) */
#ifdef TM_32BIT_DSP
            sum += (pshHdrPtr[3] >> 16) & 0xffff; sum += pshHdrPtr[3] & 0xffff;
            sum += (pshHdrPtr[4] >> 16) & 0xffff; sum += pshHdrPtr[4] & 0xffff;
            sum += (pshHdrPtr[5] >> 16) & 0xffff; sum += pshHdrPtr[5] & 0xffff;
            sum += (pshHdrPtr[6] >> 16) & 0xffff; sum += pshHdrPtr[6] & 0xffff;
            sum += (pshHdrPtr[7] >> 16) & 0xffff; sum += pshHdrPtr[7] & 0xffff;
            sum += (pshHdrPtr[8] >> 16) & 0xffff; sum += pshHdrPtr[8] & 0xffff;
            sum += (pshHdrPtr[9] >> 16) & 0xffff; sum += pshHdrPtr[9] & 0xffff;
#else /* !TM_32BIT_DSP */
/* Should work OK for 16-bit DSP's */
            sum += pshHdrPtr[6]; sum += pshHdrPtr[7]; sum += pshHdrPtr[8];
            sum += pshHdrPtr[9]; sum += pshHdrPtr[10]; sum += pshHdrPtr[11];
            sum += pshHdrPtr[12]; sum += pshHdrPtr[13]; sum += pshHdrPtr[14];
            sum += pshHdrPtr[15]; sum += pshHdrPtr[16]; sum += pshHdrPtr[17];
            sum += pshHdrPtr[18]; sum += pshHdrPtr[19];
#endif /* TM_32BIT_DSP */
        }
#endif /* TM_USE_IPV6 */
    }
#ifdef TM_DEV_SEND_OFFLOAD
/* packetPtr could be null if checksum is offloaded. */
    if (packetPtr == TM_PACKET_NULL_PTR)
    {
        partialChecksum = TM_8BIT_YES;
    }
    else
#endif /* TM_DEV_SEND_OFFLOAD */
    {
#ifdef TM_DEV_SEND_OFFLOAD
        partialChecksum = TM_8BIT_ZERO;
#endif /* TM_DEV_SEND_OFFLOAD */
#ifdef TM_DSP
        offset      = 0;
        swapNextSum = 0;
#else /* !TM_DSP */
        sumByteSwapped = TM_8BIT_ZERO;
#endif /* !TM_DSP */
        linkLength = (ttPktLen)0;
        do
        {
/* Point to data in this link */
            link16BitDataPtr = (tt16BitPtr)packetPtr->pktLinkDataPtr;
#ifndef TM_DSP
/* Loop through scattered data */
            if (linkLength == (ttPktLen)0)
            {
                linkLength = packetPtr->pktLinkDataLength;
            }
            else
            {
/* previous link had odd byte length, store 2nd byte to make a word */
                oddByte16Bit.oddChar[1] = *((tt8BitPtr)link16BitDataPtr);
/* add to sum */
                sum = sum + oddByte16Bit.oddWord;
/* Update pointer, and length. Pointer now is on odd byte boundary */
                link16BitDataPtr = (tt16BitPtr)((tt8BitPtr)link16BitDataPtr + 1);
                linkLength = packetPtr->pktLinkDataLength - 1;
                totalDataLength--;
            }
            if (linkLength > totalDataLength)
            {
/* Do not span more than asked */
                linkLength = totalDataLength;
            }
/* left over data to be checksumed after this link */
            totalDataLength -= linkLength;
/* Move pointer to even byte boundary */
            if ( (tm_is_odd((ttUserPtrCastToInt)
                            link16BitDataPtr)) && (linkLength != 0) )
            {
/* save odd byte for later addition */
                oddByte16Bit.oddChar[0] = *((tt8BitPtr)link16BitDataPtr);
/* Update pointer, and length. Pointer now is on even byte boundary */
                link16BitDataPtr =
                                   (tt16BitPtr)((tt8BitPtr)link16BitDataPtr + 1);
                linkLength--;
/* Byte swap the sum, to collect odd bytes with odd bytes */
                tm_byte_swap_sum(sum);
                sumByteSwapped = TM_8BIT_YES;
            }
            while (linkLength >= 32)
            {
/*  Unroll 16 words in one loop pass */
                sum += link16BitDataPtr[0]; sum += link16BitDataPtr[1];
                sum += link16BitDataPtr[2]; sum += link16BitDataPtr[3];
                sum += link16BitDataPtr[4]; sum += link16BitDataPtr[5];
                sum += link16BitDataPtr[6]; sum += link16BitDataPtr[7];
                sum += link16BitDataPtr[8]; sum += link16BitDataPtr[9];
                sum += link16BitDataPtr[10]; sum += link16BitDataPtr[11];
                sum += link16BitDataPtr[12]; sum += link16BitDataPtr[13];
                sum += link16BitDataPtr[14]; sum += link16BitDataPtr[15];
                link16BitDataPtr += 16;
                linkLength -= 32;
                if (sum & TM_UL(0x80000000))
/* If high order bit set. Fold 32 bit sum into 16 bit */
                {
                    tm_fold_sum(sum);
                }
            }
            while (linkLength >= 8)
            {
/*  Unroll 4 words in one loop pass */
                sum += link16BitDataPtr[0]; sum += link16BitDataPtr[1];
                sum += link16BitDataPtr[2]; sum += link16BitDataPtr[3];
                link16BitDataPtr += 4;
                linkLength -= 8;
            }
            while (linkLength > 1)
            {
/* 1 word at a time */
                sum += *link16BitDataPtr++;
                linkLength -= 2;
            }
            if (sumByteSwapped != TM_8BIT_ZERO)
            {
/*
 * We want to add the saved even byte, so byte swap the sum to start
 * collecting the even bytes.
 */
                tm_byte_swap_sum(sum);
/* toggle byte swapping */
                sumByteSwapped = TM_8BIT_ZERO;
                if (linkLength != 0)
/* extra byte at the end. We can pair it with our saved even byte */
                {
                    oddByte16Bit.oddChar[1] = *((tt8BitPtr)link16BitDataPtr);
/* add to sum */
                    sum = sum + oddByte16Bit.oddWord;
                    linkLength = 0;
                }
                else
                {
/* remember that we have our even byte stored in oddChar[0] */
                    linkLength = 1;
                }
            }
            else
            {
                if (linkLength != 0)
                {
/* Odd byte length. Save the character to add it later on */
                    oddByte16Bit.oddChar[0] = *((tt8BitPtr)link16BitDataPtr);
                }
            }
#else /* TM_DSP */
            if (linkLength == (ttPktLen)0)
            {
                linkLength = packetPtr->pktLinkDataLength;
            }
            if (linkLength > totalDataLength)
            {
/* Do not span more than asked */
                linkLength = totalDataLength;
            }
/* left over data to be checksumed after this link */
            totalDataLength -= linkLength;

            linkSum = 0;

            offset += linkLength;

/* if the source byte offset is non-zero, we need to checksum the odd-bytes
   first to align the rest of the checksum to a word boundary */
            switch(packetPtr->pktLinkDataByteOffset)
            {
            case 1:
                if (linkLength >= 1)
                {
                    swapNextSum = !swapNextSum;
#ifdef TM_16BIT_DSP        
/* checksum the 2nd byte in this 16-bit word, byte-swapped */
                    linkSum += *link16BitDataPtr & 0x00ff;
                    link16BitDataPtr++; /* advance to next word to checksum */
#endif /* TM_16BIT_DSP */
#ifdef TM_32BIT_DSP
/* checksum the 2nd byte in this 32-bit word, byte-swapped */
                    linkSum += (*link16BitDataPtr >> 16) & 0x00ff;
#endif /* TM_32BIT_DSP */
                    linkLength--;
                }
#ifdef TM_16BIT_DSP        
                break;
#endif /* TM_16BIT_DSP */
/* !!! NOTE: 32-BIT DSP FALL-THROUGH TO NEXT CASE !!! */
#ifdef TM_32BIT_DSP
            case 2:
                if (linkLength >= 2)
                {
/* checksum the 2nd word in this 32-bit word */
                    linkSum += *link16BitDataPtr & 0xffff;
                    link16BitDataPtr++; /* advance to next word to checksum */
                    linkLength -= 2;
                }
                else if (linkLength == 1)
                {
                    linkSum += *link16BitDataPtr & 0xff00;
                    linkLength = 0;
                }
                break;
            case 3:
                if (linkLength >= 1)
                {
                    swapNextSum = !swapNextSum;
/* checksum the last byte in this 32-bit word, byte-swapped */
                    linkSum += *link16BitDataPtr & 0x00ff;
                    link16BitDataPtr++; /* advance to next word to checksum */
                    linkLength--;
                }
                break;
#endif /* TM_32BIT_DSP */
            default:
                break;
            }

#ifdef TM_16BIT_DSP        
            while (linkLength >= 2)
            {
                linkSum += *link16BitDataPtr & 0xffff;
                link16BitDataPtr++;
                linkLength -= 2;
            }

            if (linkLength == 1)
            {
                linkSum += *link16BitDataPtr & 0xff00;
                linkLength = 0;
            }
#endif /* TM_16BIT_DSP */

#ifdef TM_32BIT_DSP
            while (linkLength >= 4)
            {
                linkSum += (*link16BitDataPtr >> 16) & 0xffff;
                linkSum += *link16BitDataPtr & 0xffff;
                link16BitDataPtr++;
                linkLength -= 4;
            }

            switch (linkLength)
            {
                case 1:
                    linkSum += (*link16BitDataPtr >> 16) & 0xff00;
                    break;

                case 2:
                    linkSum += (*link16BitDataPtr >> 16) & 0xffff;
                    break;
                        
                case 3:
                    linkSum += (*link16BitDataPtr >> 16) & 0xffff;
                    linkSum += *link16BitDataPtr & 0xff00;
                    break;
            }
            linkLength = 0;
#endif /* TM_32BIT_DSP */

/* Common checksum code for C3x & C5x DSP: */
            
/*
 * If the last link that was summed was odd in length, swap the sum for
 * the current link before adding to total, so that the bytes properly
 * line up.
 */
            if (swapNextSum)
            {
                tm_add_carries_sum(linkSum);
                linkSum = ((linkSum << 8) & 0xff00) | ((linkSum >> 8) & 0x00ff);
            }

            sum += linkSum;

            if (offset & 1)
            {
                swapNextSum = 1;
            }
            else
            {
                swapNextSum = 0;
            }
          
#endif /* TM_DSP */

/* check this before computing next packetPtr */
            if (totalDataLength == (ttPktLen)0)
            {
/* break out of the loop. Do not check 0 length after the loop */
                goto outFold;
            }
            packetPtr = (ttPacketPtr)packetPtr->pktLinkNextPtr;
        }
        while (packetPtr != (ttPacketPtr)0);
#ifdef TM_ERROR_CHECKING
/* double check that all data has been checksumed */
#ifdef TM_LINT
LINT_BOOLEAN_BEGIN
#endif /* TM_LINT */

        if (totalDataLength != (ttPktLen)0)
#ifdef TM_LINT
LINT_BOOLEAN_END
#endif /* TM_LINT */

        {
            tfKernelError("tfPacketChecksum",
                "Data in Packet chain smaller than length to be checksumed");
        }
#endif
outFold:
#ifdef TM_DSP
        ;
#else /* !TM_DSP */
        if (linkLength != TM_UL(0))
        {
/*  Add left-over byte */
            oddByte16Bit.oddChar[1] = 0;
            sum = sum + oddByte16Bit.oddWord;
        }
#endif /* !TM_DSP */
    } /* endif (packetPtr != (ttPacketPtr)0) */
/*  Fold 32-bit sum to 16 bits, adding possible carries from fold */
    tm_add_carries_sum(sum);
#ifdef TM_DEV_SEND_OFFLOAD
    if (partialChecksum == TM_8BIT_ZERO)
#endif /* TM_DEV_SEND_OFFLOAD */
    {
#ifdef TM_32BIT_DSP
        sum = (~sum) & 0xffff;
#else /* !TM_32BIT_DSP */
        sum = ~sum;
#endif /* TM_32BIT_DSP */
    }
    return (tt16Bit)sum;
}
#endif /* THe C Version */


/*
 * tfIpUpdateChecksum() Function Description:
 * tfIpUpdateChecksum updates the IP checksum using the old checksum and
 * the difference between the old field value and new field value (used for
 * example when ttl is decreased). Eqn. 2 from RFC 1624 is used, and
 * we check for a -0 result and change it back to 0 to satisfy the
 * boundary condition mentionned in RFC1624.
 * (For a ttl update, this is more efficient than using Eqn 4 or Eqn 3
 * from RFC1624, since the difference is a constant in a ttl update.)
 * Note that, despite what RFC1141 suggests, no byte swapping is required
 * for little Endian machine (See RFC 1071 Sec. 2 B for details).
 * Yields New checksum after computation.
 * Parameters:
 * ipHptr points to IP header
 * diff:  (tt32Bit)old+(tt32Bit)~new or (tt32Bit)old - (tt32Bit)new
 *
 * Note: diff is between values in network byte order
 *
 * To use Eqn.4 instead, we would need to pass the old and new values
 * instead of the diff and the algorithm would be as follows:
 * Eqn. 4 (RFC1624) HC' = HC - ~m - m':
 *   sum = (tt32Bit)iphPtr->iphChecksum - ((~oldValue)&0xFFFF) - newValue;
 *  Fold 32-bit sum to 16 bits:
 *   sum = (sum & 0xFFFFUL) + ((sum>>16)&0xFFFFUL);
 * End of Eqn. 4 algorithm
 * (There is no need to check for 0xFFFF when we use Eqn.4)
 */
tt16Bit tfIpUpdateChecksum (tt16Bit oldSum, tt32Bit diff)
{
    tt32Bit sum;

    sum = oldSum + diff;
/*  Fold 32-bit sum to 16 bits to add possible carry */
    tm_fold_sum(sum);
/* No possible carry from above addition */
/* Check for erroneous -0 (0xFFFF) result */
    if ((tt16Bit)sum == (tt16Bit)0xFFFF)
    {
        return (tt16Bit)0;
    }
    else
    {
        return (tt16Bit)sum;
    }
}

/*
 * Update chain with a new smaller length 'newLength':
 * Update chain data length with new length. Free all packets
 * at the end of the chain that no longer belong to the chain, but do
 * not free the first link.
 * Update link data length of the last link.
 */
void tfPacketTailTrim (ttPacketPtr packetPtr,
                       ttPktLen    newLength,
                       tt8Bit      socketLockFlag)
{
    ttPacketPtr lastPacketPtr;
#ifdef TM_USE_DRV_SCAT_RECV
    ttPacketPtr endPacketPtr;
#endif /* TM_USE_DRV_SCAT_RECV */

#ifdef TM_ERROR_CHECKING
    if (    (newLength == (ttPktLen)0)
         || (newLength > packetPtr->pktChainDataLength)
         || (packetPtr == TM_PACKET_NULL_PTR) )
    {
        tfKernelError("tfPacketTailTrim",
                "attempting to trim all data, or trim no data, or no packet");
    }
#else /* !TM_ERROR_CHECKING */
#endif /* TM_ERROR_CHECKING */
/* Update chain data length (new smaller length) */
#ifdef TM_LINT
LINT_NULL_PTR_BEGIN(packetPtr)
#endif /* TM_LINT */

    packetPtr->pktChainDataLength = newLength;
#ifdef TM_LINT
LINT_NULL_PTR_END(packetPtr)
#endif /* TM_LINT */

    lastPacketPtr = packetPtr;

/* Fetch last link of trimmed packet */
#ifdef TM_LINT
LINT_NULL_PTR_BEGIN(lastPacketPtr)
#endif /* TM_LINT */

    while ( lastPacketPtr->pktLinkDataLength < newLength )
    {
        newLength -= lastPacketPtr->pktLinkDataLength;
        lastPacketPtr = (ttPacketPtr)lastPacketPtr->pktLinkNextPtr;
#ifdef TM_ERROR_CHECKING
        if (lastPacketPtr == TM_PACKET_NULL_PTR)
        {
            tfKernelError("tfPacketTailTrim", "trimming corrupted link");
        }
#endif /* TM_ERROR_CHECKING */
    }
/* Update last link data length */
    lastPacketPtr->pktLinkDataLength = newLength;
/* Free all packets that do not belong to the chain */
    if (lastPacketPtr->pktLinkNextPtr != (ttVoidPtr)0)
    {
#ifdef TM_USE_DRV_SCAT_RECV
/* Update the pktLinkExtraCount */   
        endPacketPtr = lastPacketPtr->pktLinkNextPtr;
        do
        {
            packetPtr->pktLinkExtraCount--;
            endPacketPtr = (ttPacketPtr)endPacketPtr->pktLinkNextPtr;
        } 
        while (endPacketPtr != (ttPacketPtr)0);
#endif /* TM_USE_DRV_SCAT_RECV */
        tfFreePacket( (ttPacketPtr)lastPacketPtr->pktLinkNextPtr,
                      socketLockFlag );
        lastPacketPtr->pktLinkNextPtr = (ttVoidPtr)0;
    }
#ifdef TM_LINT
LINT_NULL_PTR_END(lastPacketPtr)
#endif /* TM_LINT */

}

/*
 * Append a new link of size addLength. Zero it.
 */
int tfPacketTailAdd (ttPacketPtr packetPtr, int addLength)
{
    ttPacketPtr lastPacketPtr;
    ttPacketPtr newPacketPtr;
    int         retCode;

    newPacketPtr = tfGetSharedBuffer(0, (ttPktLen)addLength, TM_16BIT_ZERO);
    if (newPacketPtr != (ttPacketPtr)0)
    {
        retCode = TM_ENOERROR;
        packetPtr->pktLinkExtraCount++;
        packetPtr->pktChainDataLength += (ttPktLen)addLength;
        lastPacketPtr = packetPtr;
/* Fetch last entry */
        while (lastPacketPtr->pktLinkNextPtr != (ttVoidPtr)0)
        {
            lastPacketPtr = (ttPacketPtr)lastPacketPtr->pktLinkNextPtr;
        }
        lastPacketPtr->pktLinkNextPtr = newPacketPtr;
        tm_bzero(newPacketPtr->pktLinkDataPtr,
                 tm_packed_byte_count(addLength));
    }
    else
    {
        retCode = TM_ENOBUFS;
    }
    return retCode;
}


/*
 * Trim head pointer by trimLen. Free shared buffers that are trimmed
 * all the way, but keep the first one around, since it contains packet
 * information, and could contain a network header. Update all lengths,
 * and pointer fields.
 */
void tfPacketHeadTrim (ttPacketPtr firstPacketPtr, ttPktLen trimLen)
{
    ttPacketPtr  dumpPacketPtr;
    ttPacketPtr  nextPacketPtr;
#ifdef TM_DSP
    unsigned int dataOffset;
#endif /* TM_DSP */

/* update total length */
    firstPacketPtr->pktChainDataLength -= trimLen;
    nextPacketPtr = firstPacketPtr;
    while (trimLen != TM_32BIT_ZERO)
    {
        if (nextPacketPtr->pktLinkDataLength > trimLen)
        {
/*
 * If trim amount is less than size of next. Update next and break out
 * of the loop
 */
#ifdef TM_DSP
            dataOffset = (unsigned int)
                           (nextPacketPtr->pktLinkDataByteOffset + trimLen);
            nextPacketPtr->pktLinkDataPtr += 
                                    tm_packed_byte_len(dataOffset);
            nextPacketPtr->pktLinkDataByteOffset = 
                                    ((int)dataOffset) % TM_DSP_BYTES_PER_WORD;
#else /* !TM_DSP */
            nextPacketPtr->pktLinkDataPtr += trimLen;
#endif /* TM_DSP */
            nextPacketPtr->pktLinkDataLength -= trimLen;
            break;
        }
        else
        {
/*
 * If trim amount is more than size of next, update trim amount, free
 * next unless it is the first packet, and move to next packet.
 */
            trimLen -= nextPacketPtr->pktLinkDataLength;
            dumpPacketPtr = nextPacketPtr; /* save copy to old */
            nextPacketPtr = (ttPacketPtr)nextPacketPtr->pktLinkNextPtr;
            if (dumpPacketPtr != firstPacketPtr)
            {
#ifdef TM_USE_DRV_SCAT_RECV
                firstPacketPtr->pktLinkExtraCount--;
#endif /* TM_USE_DRV_SCAT_RECV */
                tfFreeSharedBuffer(dumpPacketPtr, TM_SOCKET_UNLOCKED);
            }
            else
            {
                firstPacketPtr->pktLinkDataLength = 0;
            }
            if (nextPacketPtr == TM_PACKET_NULL_PTR)
            {
/* If there is no next shared buffer, break */
                break;
            }
        }
    }
    if (nextPacketPtr != firstPacketPtr)
    {
/* Update pointer field */
        firstPacketPtr->pktLinkNextPtr = (ttVoidPtr)nextPacketPtr;
    }
}

/*
 * tfPacketHeadTailAdd
 * Function Description
 * Check if the current packetPtr has enough space to hold the needHead, 
 * and needTail. If it has not enough head space, another packetPtr be
 * allocated in front of the current one. The headers of the current
 * packetPtr will be copied to the new one too.
 * If it has not enough tail space, another packetPtr be allocated
 * and linked to the end.
 */

int tfPktHeadTailAdd(ttPacketPtrPtr   packetPtrPtr, 
                     ttPktLen         copyHead,
                     ttPktLen         needHead,
                     ttPktLen         needTail) 
/* if new header packetPtr is allocated, how much 
 * need to copy to the new location this may include IP, IPv6 
 * extentions ...
 */
{
    ttPacketPtr      newPacketPtr;
    ttPacketPtr      packetPtr;
    ttSocketEntryPtr socketEntryPtr;
#ifdef TM_USE_IPV6
    ttListPtr        listPtr;
#endif /* TM_USE_IPV6 */
    ttPktLen         availHdrLen; /* how much we can provide */
    ttPktLen         availableTail;
    ttPktLen         tailSize;
    int              errorCode;
    tt16Bit          flags;
    tt16Bit          userFlags;

#ifdef TM_DSP
/* tfPktHeadTailAdd should only be called with an even number of long (32-bit)
   words.  On DSP platforms, calling this routine with an odd number of bytes
   will cause this routine to behave incorrectly, so make sure that no one
   is doing this. */
    tm_assert(tfPktHeadTailAdd,
              (copyHead % TM_DSP_BYTES_PER_WORD) == 0);
    tm_assert(tfPktHeadTailAdd,
              (needHead % TM_DSP_BYTES_PER_WORD) == 0);
    tm_assert(tfPktHeadTailAdd,
              (needTail % TM_DSP_BYTES_PER_WORD) == 0);
#endif /* TM_DSP */

    socketEntryPtr = (ttSocketEntryPtr) 0;


    if ( (packetPtrPtr == (ttPacketPtr *)0) ||
         (*packetPtrPtr == (ttPacketPtr)0))
    {
        errorCode = TM_EINVAL;
        goto HTADD_RETURN;
    }
    
    packetPtr = *packetPtrPtr;
    
    socketEntryPtr = packetPtr->pktSharedDataPtr->dataSocketEntryPtr;

    needHead = (ttPktLen)(needHead + TM_LL_MAX_HDR_LEN);
/*1 get the free space left ahead of our data */
    availHdrLen = (ttPktLen)
            tm_byte_count(    packetPtr->pktLinkDataPtr
                           -  packetPtr->pktSharedDataPtr->dataBufFirstPtr );
        
/*2 if necessory, allocate a new head shared data */
    if (availHdrLen < needHead)
    { 
/* 
 * additional needHead bytes should be in this newly allocated buffer, 
 * plus how many bytes we need to copy from the old buffer.
 */
        newPacketPtr = tfGetSharedBuffer( (int)(needHead),
                                          (ttPktLen)copyHead, 
                                          TM_16BIT_ZERO );
        if(newPacketPtr == TM_PACKET_NULL_PTR)
        {
            errorCode = TM_ENOMEM;
            goto HTADD_RETURN;
        }
            
/* TCP can no longer recycle this header */
        tm_16bit_clr_bit( packetPtr->pktSharedDataPtr->dataFlags,
                          TM_BUF_TCP_HDR_BLOCK );
        flags = packetPtr->pktSharedDataPtr->dataFlags;

        newPacketPtr->pktLinkNextPtr = packetPtr;
/* link extra count */
        newPacketPtr->pktLinkExtraCount = 
                 packetPtr->pktLinkExtraCount + 1;
        newPacketPtr->pktChainDataLength = 
                 packetPtr->pktChainDataLength;
/* since this is for header use only, we re-initialize
 * linkdataPtr from the buffer end 
 */
        newPacketPtr->pktLinkDataPtr = 
                newPacketPtr->pktSharedDataPtr->dataBufLastPtr
              - tm_packed_byte_count(copyHead);
/* newPacketPtr->pktLinkDataLength already initialized with copyHead */
            
/* copy the copyHead to this new buffer */
        tm_bcopy(packetPtr->pktLinkDataPtr, 
                 newPacketPtr->pktLinkDataPtr, 
                 tm_packed_byte_count(copyHead));
        newPacketPtr->pktIpHdrLen = (tt8Bit)copyHead;
/* Trim copied header from the linked packet */
        packetPtr->pktLinkDataPtr += tm_packed_byte_count(copyHead);
        packetPtr->pktLinkDataLength -= copyHead;
            
        newPacketPtr->pktSharedDataPtr->dataRtePtr = 
                packetPtr->pktSharedDataPtr->dataRtePtr;
        newPacketPtr->pktSharedDataPtr->dataRteClonedPtr = 
                packetPtr->pktSharedDataPtr->dataRteClonedPtr;
        newPacketPtr->pktFlags = packetPtr->pktFlags;
        newPacketPtr->pktFlags2 = packetPtr->pktFlags2;
        newPacketPtr->pktUserFlags = packetPtr->pktUserFlags;
        newPacketPtr->pktDeviceEntryPtr = packetPtr->pktDeviceEntryPtr;
#ifndef TM_LOOP_TO_DRIVER
        newPacketPtr->pktOrigDevPtr = packetPtr->pktOrigDevPtr;
#endif /* !TM_LOOP_TO_DRIVER */
/* what else need to copy ? */
/* we don't clear TM_ETHERNET_ADDRESS_FLAG, instead, we copy the old
 * packet's pktEthernetAddress
 */
        tm_ether_copy(packetPtr->pktEthernetAddress,
                      newPacketPtr->pktEthernetAddress);
        newPacketPtr->pktSharedDataPtr->dataDeviceEntryPtr =
                 packetPtr->pktSharedDataPtr->dataDeviceEntryPtr; 
        newPacketPtr->pktMhomeIndex = packetPtr->pktMhomeIndex;
        newPacketPtr->pktSharedDataPtr->dataSocketEntryPtr = 
                       socketEntryPtr;
/* Not a device driver data buffer */
        tm_16bit_clr_bit( flags, TM_BUF_USER_DEVICE_DATA);
        newPacketPtr->pktSharedDataPtr->dataFlags = flags;
        if(    ((flags & TM_BUF_USER_SEND)!= (unsigned)0) 
            && (socketEntryPtr != (ttSocketEntryPtr)0) )
        {
            tm_16bit_clr_bit( packetPtr->pktSharedDataPtr->dataFlags,
                              TM_BUF_USER_SEND );
/* otherwise, socSendQueueBytes will be deducted twice*/
            packetPtr->pktSharedDataPtr->dataSocketEntryPtr = 0;
/* Do not increase socOwnerCount below */
            socketEntryPtr = TM_SOCKET_ENTRY_NULL_PTR;
        }
        if (socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR)
        {
/* Socket lock might not be on */
            tm_kernel_set_critical;
            socketEntryPtr->socOwnerCount++;
            tm_kernel_release_critical;
        }
        newPacketPtr->pktSharedDataPtr->dataUserDataLength =
                packetPtr->pktSharedDataPtr->dataUserDataLength;

#ifdef TM_USE_IPV6
/* Move the ancillary data list to the new shared Data */
        listPtr = &(packetPtr->pktSharedDataPtr->data6AncHdrList);
        if (listPtr->listCount != 0)
        {
            tfListReplace(
              &(listPtr->listHeadNode),
              &(newPacketPtr->pktSharedDataPtr->data6AncHdrList.listHeadNode));
            newPacketPtr->pktSharedDataPtr->data6AncHdrList.listCount =
                   listPtr->listCount;
        }
        else
        {
            tfListInit(&(newPacketPtr->pktSharedDataPtr->data6AncHdrList));
        }
        tfListInit(listPtr);
#endif /* TM_USE_IPV6 */

        newPacketPtr->pktNetworkLayer = packetPtr->pktNetworkLayer;

#ifdef TM_USE_IPSEC
        newPacketPtr->pktSalistPtr = packetPtr->pktSalistPtr;
        packetPtr->pktSalistPtr = (ttVoidPtr)0;
        newPacketPtr->pktSelectorPtr = packetPtr->pktSelectorPtr;
        packetPtr->pktSelectorPtr = (ttVoidPtr)0;
        newPacketPtr->pktIpsecLevel = packetPtr->pktIpsecLevel;
#if (TM_VERSION > 30)
        newPacketPtr->pktUnFragLen = packetPtr->pktUnFragLen;
        packetPtr->pktUnFragLen = 0;
#endif /* (TM_VERSION > 30) */
#ifdef TM_LOCK_NEEDED
        newPacketPtr->pktSendLockEntryPtr = 
                    packetPtr->pktSendLockEntryPtr;
        packetPtr->pktSendLockEntryPtr = (ttLockEntryPtr)0;
#endif /* TM_LOCK_NEEDED */
        newPacketPtr->pktRouteCachePtr = packetPtr->pktRouteCachePtr;
        packetPtr->pktRouteCachePtr = (ttRtCacheEntryPtr)0;
#endif /* TM_USE_IPSEC */
          
        *packetPtrPtr = newPacketPtr;
    }
    
/* 3 get the availableTail at end of the link */
    if (needTail != (ttPktLen)0)
    {
/* Check flag before walking the linked list of scattered buffers  */
        if (tm_16bit_one_bit_set(packetPtr->pktFlags2, TM_PF2_TRAILING_DATA))
        {
/* Unsent data at the end of the last scattered buffer. */
            userFlags = TM_PF2_TCP_TRAILER;
        }
        else
        {
            userFlags = (tt16Bit)0;
        }
/* Get last scattered buffer in the linked list */
        while(packetPtr->pktLinkNextPtr)
        {
            packetPtr = packetPtr->pktLinkNextPtr;
        }   
        if (    (userFlags == (tt16Bit)0)
             && (tm_16bit_one_bit_set(packetPtr->pktFlags2, TM_PF2_TRAILING_DATA)))
        {
/* Unsent data at the end of the last scattered buffer. */
            userFlags = TM_PF2_TCP_TRAILER;
        }

        if (userFlags != (tt16Bit)0)
        {
/* 
 * For TCP, with scattered data, we can't add the ESP trailer
 * at the end of the data if there is still unsent data in the
 * last scattered buffer.  We must append a separate shared packet to 
 * add the trailer/padding.
 */
            availableTail = 0;
        }
        else
        {
/* Get available space at the end of the last scattered buffer */
            availableTail = (ttPktLen)
                  tm_byte_count(packetPtr->pktSharedDataPtr->dataBufLastPtr - 
                              packetPtr->pktLinkDataPtr)
#ifdef TM_DSP
                - packetPtr->pktLinkDataByteOffset
#endif /* TM_DSP */
                - packetPtr->pktLinkDataLength;
        }
        
/*4 if necessory, allocate a new tail buffer */
        
        if(availableTail < needTail )
        {
            tailSize = needTail - availableTail;
#ifdef TM_USE_IPSEC 
/* for IPsec, we need to make sure that the new tail buffer
 * is bigger than TM_ESP_AH_TRAILER_SIZE, which means we
 * want the Integrity Check Value to be contiguous.
 */
            if(tailSize < TM_ESP_AH_TRAILER_SIZE)
            {
                tailSize = TM_ESP_AH_TRAILER_SIZE;
            }
#endif /* TM_USE_IPSEC*/
            newPacketPtr = tfGetSharedBuffer(0, tailSize, TM_16BIT_ZERO);
/* we need to save the ICV data together */
            if (newPacketPtr == TM_PACKET_NULL_PTR)
            {
                errorCode = TM_ENOMEM;
                goto HTADD_RETURN;
            }
            packetPtr->pktLinkNextPtr = newPacketPtr;
            newPacketPtr->pktLinkDataLength = 0; 
            newPacketPtr->pktFlags2 = userFlags;
/* link extra count */
            (*packetPtrPtr)->pktLinkExtraCount++;

/* no padding or tailPtr yet 
 * just append a new buffer, don't change
 * the linkdataleng or chained data length
 */
        }
    }
    errorCode = TM_ENOERROR;
HTADD_RETURN:
    return errorCode;
}

#ifdef TM_DEV_RECV_OFFLOAD
/*
 * Compute checksum on passed checksumPtr, and remove it from the hardware
 * computed checksum passed in in the pktDevRecvOffload structure.
 */
void tfDevoRecvPeelChecksum( ttPacketPtr packetPtr, 
                             tt16BitPtr  checksumPtr,
                             tt16Bit     length )
{
    ttDevRecvOffloadPtr pktDevRecvOffloadPtr;
    tt16Bit             checksumLength;
    tt32Bit             sum;

    pktDevRecvOffloadPtr = (ttDevRecvOffloadPtr)packetPtr->pktDevOffloadPtr;
    if (pktDevRecvOffloadPtr->devoRecvXsumOffset == TM_16BIT_ZERO)
    {
/* Back out the header from the checksum */
        sum = (tt32Bit)0;
        checksumLength = length;
        while (checksumLength > 1)
        {
            sum += *checksumPtr++;
            checksumLength -= 2;
        }
        sum = (tt32Bit)(  ~(pktDevRecvOffloadPtr->devoRecvXsum)
                         - sum);
        if ( sum == (tt32Bit) 0 )
        {
            pktDevRecvOffloadPtr->devoRecvXsum = (tt16Bit)0;
        }
        else
        {
            tm_add_carries_sum(sum);
            pktDevRecvOffloadPtr->devoRecvXsum = (tt16Bit)~sum;
        }
        pktDevRecvOffloadPtr->devoRecvXsumLength = (tt16Bit) 
            (pktDevRecvOffloadPtr->devoRecvXsumLength - length);
    }
    else
    {
        if (    pktDevRecvOffloadPtr->devoRecvXsumOffset 
             >= length )
        {
/* Offset from beginning of next layer */
            pktDevRecvOffloadPtr->devoRecvXsumOffset = (tt16Bit)
                (pktDevRecvOffloadPtr->devoRecvXsumOffset - length);
        }
        else
        {
/* Something wrong with the checksum offset. Ignore harware computed checksum */
            pktDevRecvOffloadPtr->devoRFlags &= ~ 
                                             TM_DEVOR_PACKET_RECV_CHKSUM_COMPUTED;
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfDevoRecvPeelChecksum", 
                           "Bad offset in hardware computed checksum, ignored");
#endif /* TM_ERROR_CHECKING */
        }
    }
}

/* 
 * Trim any excess checksum computed by the hardware on the data beyond 
 * the end of the packet.
 * (i.e. Remove checksum on trailing data if any.)
 */
void tfDevoRecvTrimChecksum( ttPacketPtr packetPtr, 
                             tt16Bit     hdrLength,
                             ttPktLen    pktLength )
{
    ttDevRecvOffloadPtr pktDevRecvOffloadPtr;
    tt8BitPtr           savPktLinkDataPtr;
    ttPktLen            savPktLinkDataLength;
    ttPktLen            hwLength;
    ttPktLen            trim;
    ttPktLen            dataLength;
    tt32Bit             sum;
    tt16Bit             checksum;

    pktDevRecvOffloadPtr = (ttDevRecvOffloadPtr)packetPtr->pktDevOffloadPtr;
    if (    (pktDevRecvOffloadPtr != (ttDevRecvOffloadPtr)0)
         && (   pktDevRecvOffloadPtr->devoRFlags 
              & TM_DEVOR_PACKET_RECV_CHKSUM_COMPUTED ) )
    {

        dataLength = (ttPktLen)(pktLength - hdrLength);
/* 
 * devoRecvXsumOffset is relative to peeled off data (in our case 
 * IP header + Ethernet header), i.e. devRecvXsum... correspond to ULP 
 * already (data above IP header).
 */
        hwLength = (ttPktLen)(   pktDevRecvOffloadPtr->devoRecvXsumOffset 
                               + pktDevRecvOffloadPtr->devoRecvXsumLength);
        if ( hwLength > dataLength)
        {
            trim = (ttPktLen)(hwLength - dataLength);
#ifdef TM_USE_DRV_SCAT_RECV
/* Fetch last link of trimmed packet */
            while ( packetPtr->pktLinkDataLength < dataLength )
            {
                dataLength -= packetPtr->pktLinkDataLength;
                packetPtr = (ttPacketPtr)packetPtr->pktLinkNextPtr;
#ifdef TM_ERROR_CHECKING
                if (packetPtr == TM_PACKET_NULL_PTR)
                {
                    tfKernelError("tfDevoRecvTrimChecksum",
                                  "corrupted link");
                }
#endif /* TM_ERROR_CHECKING */
            }
#endif /* TM_USE_DRV_SCAT_RECV */
            savPktLinkDataPtr = packetPtr->pktLinkDataPtr;
            savPktLinkDataLength = packetPtr->pktLinkDataLength;
            packetPtr->pktLinkDataPtr += pktLength;
            packetPtr->pktLinkDataLength -= pktLength;
/* 
 * Back out execess data from the checksum. tfPacketChecksum handles odd size
 * length checksum, and odd pktLinkDataPtr.
 */
            checksum = tfPacketChecksum(
                        packetPtr, trim, (tt16BitPtr)0, (ttPktLen)0);
            packetPtr->pktLinkDataPtr = savPktLinkDataPtr;
            packetPtr->pktLinkDataLength = savPktLinkDataLength;
            sum = (tt32Bit)(   ~(pktDevRecvOffloadPtr->devoRecvXsum)
                             - ~checksum );
            if ( sum == (tt32Bit) 0 )
            {
                pktDevRecvOffloadPtr->devoRecvXsum = (tt16Bit)0;
            }
            else
            {
                tm_add_carries_sum(sum);
                pktDevRecvOffloadPtr->devoRecvXsum = (tt16Bit)~sum;
            }
            pktDevRecvOffloadPtr->devoRecvXsumLength = (tt16Bit) 
                (pktDevRecvOffloadPtr->devoRecvXsumLength - trim);
        }
    }
    return;
}

/*
 * Compute checksum on passed pshPtr, and add it to the hardware computed
 * checksum (which has had the IP header checksum removed already in
 * tfDevoRecvPeelChecksum) passed in the ttDevRecvOffload structure.
 */
tt16Bit tfDevoRecvPacketChecksum( ttDevRecvOffloadPtr pktDevRecvOffloadPtr, 
                                  tt16BitPtr          pshPtr,
                                  tt16Bit             pshLength )
{
    tt32Bit             sum;
    tt16Bit             checksum;

/* Checksum only the pseudo header */
    sum = (tt32Bit)0;
    while (pshLength > 1)
    {
        sum += *pshPtr++;
        pshLength -= 2;
    }
    checksum = (tt16Bit)(~(pktDevRecvOffloadPtr->devoRecvXsum));
    sum = checksum + sum;
    tm_add_carries_sum(sum);
    checksum = (tt16Bit)~sum;
    return checksum;
}
#endif /* TM_DEV_RECV_OFFLOAD */

/***************** End Of File *****************/
