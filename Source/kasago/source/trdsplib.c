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
 * Description: Common functions to support DSPs with 16-bit and 32-bit "char"
 *
 * Filename: trdsplib.c
 * Author: Ed Remmell
 * Date Created:
 * $Source: source/trdsplib.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:47:18JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * Include
 */
#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_DSP

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/* constant declarations and enumerations */

/* type definitions */

/* local function prototypes */

/* local variable definitions */

/* static const tables */

/* macro definitions */


/****************************************************************************
* FUNCTION: tfByteByByteCopy
*
* PURPOSE: Copy bytes on DSP platform which is not byte-addressable.
*
* PARAMETERS:
*   srcAddr:
*       Word-aligned pointer to source buffer for copy.
*   srcOffset:
*       Byte offset of start of data in source buffer to copy.
*   destAddr:
*       Word-aligned pointer to destination buffer for copy.
*   destOffset:
*       Byte offset of start of where to copy data to in destination buffer.
*   length:
*       Number of bytes to copy from source buffer to destination buffer.
*       
*
* RETURNS:
*   Nothing
*
* NOTES:
*   On a 32-bit DSP like the TI C3x, the "char" datatype is 32-bits long. On
*   a 16-bit DSP like the TI C5x, the "char" datatype is 16-bits long.
*
****************************************************************************/
void tfByteByByteCopy(int* srcAddr, int srcOffset, int* destAddr,
                      int destOffset, int length) 
{
    unsigned int srcByte;
    int          index;

    for (index = 0; index < length; index++)
    {
/* manipulate src and dest words to implement the copy*/

/* 1.) Retrieve the srcByte: shift contents of the srcAddr (word) to the right
 *   by (3-n)*8 (bits), where n is the srcOffset.  We implement the *8 using
 *   << 3. This makes the target byte the lsbyte.  Finally, mask off the
 *   leftmost 3 bytes. We end up with 0x000000XX, where XX is the target byte
 */
        srcByte = 
            ((*srcAddr) >> ((TM_DSP_ROUND_PTR - srcOffset) << 3)) & 0xff;

/* 2.) Zero out the destByte within the destAddr: take 0xff and left shift it by
 *   (3-n)*8 (bits), where n is the destOffset.  Then take the complement of
 *   this number.  Finally, AND it with the contents of the destAddr. This
 *   zeros out our destByte: 0xXX00XXXX (with 1 as our offset from the left)
 */
        (*destAddr) &= ~(0xff << ((TM_DSP_ROUND_PTR - destOffset) << 3));

/* 3.) Shift srcByte to same offset as the destByte: left shift srcByte by
 *   (3-n)*8 (bits), where n is the destOffset.  Then OR this result with the
 *   contents of the destAddr, which gives us a new word in destAddr with only
 *   the correct byte changed.  0x00XX0000 | 0x12005678 = 0x12XX5678
 */
        (*destAddr) |= srcByte << ((TM_DSP_ROUND_PTR - destOffset) << 3);

/* increment the src and dest offsets. If they exceed 3, 
 * reset offsets to 0 and increment the addresses
 */
        srcOffset++;
        if ( srcOffset > (TM_DSP_BYTES_PER_WORD - 1) )
        {
            srcOffset = 0;
            srcAddr++;
        }

        destOffset++;
        if ( destOffset > (TM_DSP_BYTES_PER_WORD - 1) )
        {
            destOffset = 0;
            destAddr++;
        }
    }
}

/* srcAddr and destAddr are memory addresses
 * srcOffset and destOffset are byte offsets (0-3)
 * length is number of 8-bit bytes to be copied
 */
int tfMemCopyOffset(
    int * srcAddr, int srcOffset, int * destAddr, int destOffset, int length)
{
/* Number of full words to be copied*/
    int numWords;

/*
 * Make sure offsets are in range 0-3 on a 32-bit DSP,
 * and in range 0-2 on a 16-Bit DSP.
 */
    if (srcOffset >= TM_DSP_BYTES_PER_WORD)
    {
        srcAddr = srcAddr + ((srcOffset) >> TM_DSP_SHIFT);
        srcOffset = srcOffset % TM_DSP_BYTES_PER_WORD;
    }
    if (destOffset >= TM_DSP_BYTES_PER_WORD)
    {
        destAddr = destAddr + ((destOffset) >> TM_DSP_SHIFT);
        destOffset = destOffset % TM_DSP_BYTES_PER_WORD;
    }
    numWords = tm_packed_byte_len(length);
    if(srcOffset == destOffset)
    {
/* see if end of block is right aligned*/
        if(!((length + srcOffset) & TM_DSP_ROUND_PTR))
        {
/* only full words to be copied*/
            if(srcOffset == 0)
            {
                tm_bcopy(srcAddr, destAddr, numWords);
                return 0;
            }

/* first word is incomplete, but rest are full*/
/* use byte-by-byte copy for the first partial word*/
            tfByteByByteCopy(srcAddr, srcOffset, destAddr, destOffset,
                             TM_DSP_BYTES_PER_WORD - srcOffset);

/* use tm_bcopy to move the remaining full words*/
            tm_bcopy(++srcAddr, ++destAddr, numWords);
            return 0;
        }

/* see if beginning of block is word aligned*/
        if(srcOffset == 0)
        {
/* use tm_bcopy to move the first full words*/
            tm_bcopy(srcAddr, destAddr, numWords);

/* use byte-by-byte copy for the last partial word
 *   length to be copied = total num of bytes - (num full words * 4)
 */
            tfByteByByteCopy(srcAddr+numWords, 0, destAddr+numWords, 0,
                             length - tm_byte_count(numWords));
        }
        else
        {
/* use byte-by-byte copy for first partial word*/
            tfByteByByteCopy(srcAddr, srcOffset, destAddr, destOffset,
                             TM_DSP_BYTES_PER_WORD - srcOffset);

/* redefine numWords:
 *   first take off the first partial word
 *      :srcOffset - 4  clears the first word
 *      add this to total num of bytes to get bytes in rest of block
 *      then divide by 4 (to get whole 32bit words)
 */
            numWords = tm_packed_byte_len(
                length + srcOffset - TM_DSP_BYTES_PER_WORD);

/* use tm_bcopy for middle block*/
            tm_bcopy(++srcAddr, ++destAddr, numWords);

/* use byte-by-byte copy for last partial word*/
            tfByteByByteCopy( srcAddr+numWords,
                              srcOffset,
                              destAddr+numWords,
                              destOffset,
                              length + srcOffset - TM_DSP_BYTES_PER_WORD
                                     - tm_byte_count(numWords) );
        }
    }
    else
    {
        tfByteByByteCopy(srcAddr, srcOffset, destAddr, destOffset, length);
        return 0;
    }

    return 0;
}

void tfBcopyToPacked(const void * sourcePtr,
                     void *       destPtr,
                     unsigned int length,
                     unsigned int dstOffset)
{
    unsigned int   srcIndex;
    unsigned int   inWord;
    unsigned int   outWord;
    unsigned int * sourceWordPtr;
    unsigned int * destWordPtr;

    sourceWordPtr = (unsigned int *) sourcePtr;
    destWordPtr   = (unsigned int *) destPtr;

    for (srcIndex=0;srcIndex < length; srcIndex++)
    {

/* Read current source and destination words */
        inWord   = sourceWordPtr[srcIndex];
        outWord  = *destWordPtr;

/* Clear and set current byte in destination word */
        outWord &= ~(0xff << ((TM_DSP_ROUND_PTR - dstOffset) << 3));
        outWord |= (inWord << ((TM_DSP_ROUND_PTR - dstOffset) << 3));
        *destWordPtr = outWord;

/* 
 * Move to next destination byte.  If we've reached the end of a word, move
 * on to the next one and reset our offset count.
 */
        dstOffset++;
        if (dstOffset >= TM_DSP_BYTES_PER_WORD)
        {
            dstOffset = 0;
            destWordPtr++;
        }

    }

}

void tfBcopyToUnpacked(const void * sourcePtr,
                       int          srcOffset,
                       void *       destPtr,
                       unsigned int length)
{
    unsigned int   dstIndex;
    unsigned int   inWord;
    unsigned int   outWord;
    unsigned int * sourceWordPtr;
    unsigned int * destWordPtr;

    sourceWordPtr = (unsigned int *) sourcePtr;
    destWordPtr   = (unsigned int *) destPtr;

    for (dstIndex=0; dstIndex < length; dstIndex++)
    {

/* Get current byte from the current source word, and set the dest word */
        inWord  = *sourceWordPtr;
        outWord = (inWord >> ((TM_DSP_ROUND_PTR - srcOffset) << 3)) & 0xff;
        destWordPtr[dstIndex] = outWord;

/* 
 * Move to next source byte.  If we've reached the end of the word, move to next
 * one and reset offset count.
 */
        srcOffset++;
        if (srcOffset >= TM_DSP_BYTES_PER_WORD)
        {
            srcOffset = 0;
            sourceWordPtr++;
        }
    }


}
#else /* ! TM_DSP */

LINT_UNUSED_HEADER

/* To allow link for builds when TM_DSP is not defined */
int tvDspLibDummy = 0;
#endif /* ! TM_DSP */

/***************** End Of File *****************/
