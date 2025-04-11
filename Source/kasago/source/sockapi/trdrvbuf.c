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
 * Description: Get a User Driver Buffer
 *
 * Filename: trdrvbuf.c
 * Author: Odile
 * Date Created: 10/22/99
 * $Source: source/sockapi/trdrvbuf.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:02JST $
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

/*
 * tfGetDriverBuffer() Function Description
 * This function allocates a Treck buffer to be used by a user device driver.
 *
 * Parameter       Description
 * userBufferPtr   Pointer to the user buffer handle to be filled in by
 *                 this routine.
 * length          Length of the data that the user need to copy
 * alignment       Buffer pointer will be aligned on a multiple of this value
 * Returns
 * Value        Meaning
 * dataPtr      Pointer to the allocated buffer
 */
char TM_FAR * tfGetDriverBuffer ( ttUserBufferPtr userBufferPtr,
                                  int             length,
                                  int             alignment )
{
    ttPacketPtr     packetPtr;
    char TM_FAR   * dataPtr;
    int             alignmentByteCount;

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

/*
 * Allocate enough room ahead of the packet to expand backwards, in case
 * we need to forward the packet through another interface.
 * Allocate datalength plus enough room to adjust for alignment ahead
 * of the buffer.
 */
    packetPtr = tfGetSharedBuffer(TM_LL_MAX_HDR_LEN + alignmentByteCount,
                                  (ttPktLen)length,
                                  TM_16BIT_ZERO);
    if (packetPtr != TM_PACKET_NULL_PTR)
    {
        packetPtr->pktUserFlags = TM_PKTF_USER_DEV_OWNS;
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
        dataPtr = (char TM_FAR *)packetPtr->pktLinkDataPtr;
        *userBufferPtr = (ttUserBuffer)packetPtr;
    }
    else
    {
        *userBufferPtr = (ttUserBuffer)0;
        dataPtr = (char TM_FAR *)0;
    }
    return dataPtr;
}
