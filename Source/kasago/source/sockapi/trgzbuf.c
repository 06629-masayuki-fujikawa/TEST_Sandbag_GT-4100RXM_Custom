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
 * Description: BSD Sockets Interface (tfGetZeroCopyBuffer)
 *
 * Filename: trgzbuf.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trgzbuf.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:28JST $
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

ttUserMessage tfGetZeroCopyBuffer( int size,
                                   char TM_FAR * TM_FAR * dataPtrPtr);

ttUserMessage tfGetAlignedZeroCopyBuffer( int size,
                                          int alignment,
                                          char TM_FAR * TM_FAR * dataPtrPtr);


/* 
 * Get a Message Buffer for Zero Copy
 */
ttUserMessage tfGetZeroCopyBuffer( int size,
                                   char TM_FAR * TM_FAR * dataPtrPtr)
{
    return tfGetAlignedZeroCopyBuffer( size, 0, dataPtrPtr);
}


/* 
 * Get an Aligned Message Buffer for Zero Copy
 */
ttUserMessage tfGetAlignedZeroCopyBuffer( int size,
                                          int alignment,
                                          char TM_FAR * TM_FAR * dataPtrPtr)
{
    ttPacketPtr packetPtr;
    int         alignmentByteCount;

#ifdef TM_ERROR_CHECKING
/* Parameter check */
    if (alignment < 0)
    {
        alignment = 0;
    }
#endif /* TM_ERROR_CHECKING */

#ifdef TM_DSP
#ifdef TM_32BIT_DSP
/* make sure we allocate enough bytes for the specified address alignment for
   a target CPU that has 32-bit words */
    alignmentByteCount = alignment << 2;
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
/* make sure we allocate enough bytes for the specified address alignment for
   a target CPU that has 16-bit words */
    alignmentByteCount = alignment << 1;
#endif /* TM_16BIT_DSP */
#else /* ! TM_DSP */
    alignmentByteCount = alignment;
#endif /* ! TM_DSP */

    packetPtr = TM_PACKET_NULL_PTR;
    if (dataPtrPtr == (char TM_FAR * TM_FAR *)0)
    {
        goto zeroCopyExit;
    }
    if (size == 0)
    {
        *dataPtrPtr=(char TM_FAR *)0;
        goto zeroCopyExit;
    }
/* Allocate enough room to adjust for alignment ahead of the buffer. */
    packetPtr=tfGetSharedBuffer(
                      TM_MAX_SEND_TCP_HEADERS_SIZE + alignmentByteCount,
                      (ttPktLen)size,
                      TM_16BIT_ZERO);
    if (packetPtr != TM_PACKET_NULL_PTR)
    {
/* User owns the packet and is allowed to free it */
        packetPtr->pktUserFlags = TM_PKTF_USER_OWNS;
        if (alignment > 0)
        {
            if (((ttUserPtrCastToInt)packetPtr->pktLinkDataPtr)
                & (ttUserPtrCastToInt)(alignment - 1) )
            {
/* Align the data area (according to alignment) */
                packetPtr->pktLinkDataPtr = (tt8BitPtr)
                    (    ((ttUserPtrCastToInt)packetPtr->pktLinkDataPtr)
                         & ~((ttUserPtrCastToInt)(alignment - 1)) );
            }
        }
        *dataPtrPtr=(char TM_FAR *)packetPtr->pktLinkDataPtr;
    }
    else
    {
        *dataPtrPtr=(char TM_FAR *)0;
    }
zeroCopyExit:
    return((ttUserMessage)packetPtr);
}
